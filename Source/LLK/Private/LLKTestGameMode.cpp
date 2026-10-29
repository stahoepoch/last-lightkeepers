#include "LLKTestGameMode.h"

#include "Characters/LLKCharacter.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "LLK.h"
#include "LLKGameState.h"
#include "Storm/LLKStormChargeSubsystem.h"
#include "Storm/LLKStormChargeSubsystem.h"
#include "LLKUnits.h"
#include "Harness/LLKStoreFloorBuilder.h"
#include "UI/LLKDebugHUD.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/**
	 * Engine primitives only. These live in the engine install, not in
	 * `Content/`, so using them adds nothing to the project and leaves
	 * nothing behind. If a future engine moves them, the harness degrades to
	 * invisible-but-solid rather than failing - which is why every use below
	 * is null-checked instead of assumed.
	 */
	UStaticMesh* LoadEngineCube()
	{
		return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	}

	AStaticMeshActor* SpawnBlock(UWorld* World, UStaticMesh* Mesh, const FVector& Location, const FVector& Scale)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator, Params);
		if (Actor == nullptr)
		{
			return nullptr;
		}

		// Movable: these are spawned into a running world and will never be
		// part of a built lighting scenario. Static would log a warning and
		// render unlit.
		Actor->SetMobility(EComponentMobility::Movable);
		if (UStaticMeshComponent* MeshComponent = Actor->GetStaticMeshComponent())
		{
			if (Mesh != nullptr)
			{
				MeshComponent->SetStaticMesh(Mesh);
			}
			MeshComponent->SetWorldScale3D(Scale);
			// Collision regardless of whether the mesh loaded. An invisible
			// floor is a bad harness; a floor you fall through is no harness.
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
		}
		return Actor;
	}
}

ALLKTestGameMode::ALLKTestGameMode()
{
	DefaultPawnClass = ALLKCharacter::StaticClass();

	// M1.7. Set here rather than left to the project's default HUD class,
	// because the whole point of the harness is that pressing Play needs no
	// setup - and a storm meter nobody can see is why the lantern read as a
	// cool light rather than a decision. Turn it off with `LLK.ShowStormHUD 0`.
	HUDClass = ALLKDebugHUD::StaticClass();

	// Without this the storm never reaches a client and the HUD draws
	// nothing there - which looks exactly like a night that does not
	// escalate. M1.9 is a two-machine test, so the harness has to be the
	// shape that test needs rather than the shape standalone play tolerates.
	GameStateClass = ALLKGameState::StaticClass();
}

bool ALLKTestGameMode::WorldAlreadyHas(TSubclassOf<AActor> ActorClass) const
{
	const UWorld* World = GetWorld();
	if (World == nullptr || ActorClass == nullptr)
	{
		return false;
	}

	for (TActorIterator<AActor> It(World, ActorClass); It; ++It)
	{
		return true;
	}
	return false;
}

void ALLKTestGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Before login, so the PlayerStart below exists in time to be chosen.
	BuildHarnessWorld();
}

void ALLKTestGameMode::BuildHarnessWorld()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	FVector HarnessOrigin = FVector::ZeroVector;
	bool bHasExistingGround = false;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		HarnessOrigin = It->GetActorLocation();

		FHitResult GroundHit;
		const FVector TraceStart = HarnessOrigin + FVector(0.f, 0.f, 10000.f);
		const FVector TraceEnd = HarnessOrigin - FVector(0.f, 0.f, 10000.f);
		bHasExistingGround = World->LineTraceSingleByChannel(
			GroundHit, TraceStart, TraceEnd, ECC_WorldStatic);
		break;
	}

	UStaticMesh* Cube = LoadEngineCube();
	if (Cube == nullptr)
	{
		UE_LOG(LogLLK, Warning,
			TEXT("LLKTestGameMode: /Engine/BasicShapes/Cube not found. The floor will be solid but invisible."));
	}

	// --- the floor -----------------------------------------------------
	// A starter map already has terrain. Adding a cube at the same elevation
	// makes the two surfaces fight for every pixel, which reads as a broken
	// texture. Build a floor only for genuinely blank maps.
	//
	// The engine cube is 100cm, so scale is extent/50. Its top surface sits at
	// the harness origin and the generated PlayerStart goes above it.
	const float Thickness = 50.f;
	if (!bHasExistingGround)
	{
		SpawnBlock(World, Cube,
			HarnessOrigin - FVector(0.f, 0.f, Thickness * 0.5f),
			FVector(FloorHalfExtent * 2.f / 100.f, FloorHalfExtent * 2.f / 100.f, Thickness / 100.f));
	}

	// **The pillar ring is gone.** It existed because a flat untextured plane
	// gives a first-person camera no motion cue, and M0.8's only question was
	// "did the keeper move?". A shop answers that better: walls at a known
	// distance, counters at hip height, columns to pass. A ring of pillars
	// standing outside the store would now be clutter around the thing it was
	// a stand-in for.

	// The ground floor, from the plan. Everything it spawns is an engine
	// primitive placed at runtime, so this still writes nothing to Content/
	// and still deletes itself when the authored store arrives.
	const LLKStoreFloor::FBuildResult Floor =
		LLKStoreFloor::BuildGroundFloor(World, Cube, HarnessOrigin, StoreFloorPlan);
	EnemySpawnPoints = Floor.EnemySpawnPoints;

	// --- light, and nothing persistent ---------------------------------
	// See the header. Two spawned actors in a running world; no project
	// lighting value is read or written, and DONTFUCKUPTHELIGHTS.md is
	// untouched by design.
	// **Dim on purpose, since 2026-10-29.** The first version lit this box
	// like an afternoon, and Chris went looking for shadows to see the lantern
	// in - which is the wrong way round for a game whose whole trade is
	// whether to turn a light on. A harness bright enough to make the floor
	// visible is also bright enough to make the lantern pointless, and the
	// lantern is the thing being tested.
	//
	// So: enough to read the floor and the pillars as shapes, not enough to
	// work by. Both values are properties rather than constants because "how
	// dark is dark" is the single hardest thing to settle by argument.
	if (!WorldAlreadyHas(ADirectionalLight::StaticClass()))
	{
		if (ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(
				FVector(0.f, 0.f, 1000.f), FRotator(-46.f, -30.f, 0.f)))
		{
			Sun->SetMobility(EComponentMobility::Movable);
			if (UDirectionalLightComponent* Component = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
			{
				// Kept so M2.3 can dim it when the night escalates. Weak, and
				// every use is null-checked, so a map that brought its own sun
				// simply gets no storm response rather than a crash.
				HarnessSun = Component;
				Component->SetIntensity(HarnessSunIntensity);
				// Cool and blue: the storm outside, so the keeper's warm
				// lantern reads as the one human light in the shot. The same
				// contrast M1.5's grey box will need and Chris owns there.
				Component->SetLightColor(FLinearColor(0.55f, 0.66f, 1.f));
			}
		}
	}
	if (!WorldAlreadyHas(ASkyLight::StaticClass()))
	{
		if (ASkyLight* Sky = World->SpawnActor<ASkyLight>())
		{
			if (USkyLightComponent* Component = Sky->GetLightComponent())
			{
				HarnessSky = Component;
				// ASkyLight has no SetMobility; the component carries it.
				Component->SetMobility(EComponentMobility::Movable);
				Component->SetIntensity(HarnessSkyIntensity);
				Component->SetLightColor(FLinearColor(0.4f, 0.45f, 0.55f));
			}
		}
	}

	// --- where the keeper starts ----------------------------------------
	// Only if the open map has none. A second PlayerStart would make the
	// spawn point depend on iteration order, which is the kind of thing that
	// looks like a bug in the character.
	if (!WorldAlreadyHas(APlayerStart::StaticClass()))
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		// Just inside the sealed street doors, facing into the shop, which is
		// where the builder put it. Spawning at the origin would drop the
		// keeper in the middle of the cosmetics counters.
		World->SpawnActor<APlayerStart>(Floor.SpawnPoint, FRotator::ZeroRotator, Params);
	}

	UE_LOG(LogLLK, Log,
		TEXT("LLKTestGameMode: harness world built - %.0fm slab under a %.0f x %.0f stud store, ")
		TEXT("%d enemy spawn point(s), sun %.2f / sky %.2f. Disposable; the authored store "
			"replaces all of it."),
		FloorHalfExtent * 2.f / 100.f,
		StoreFloorPlan.DepthStuds, StoreFloorPlan.WidthStuds,
		EnemySpawnPoints.Num(), HarnessSunIntensity, HarnessSkyIntensity);
}

void ALLKTestGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Bound here rather than in InitGame, where the harness world is built:
	// the subsystem exists by then, but binding before the lights have been
	// spawned means a crossing during startup would drive nothing.
	UWorld* World = GetWorld();
	ULLKStormChargeSubsystem* Storm =
		World != nullptr ? World->GetSubsystem<ULLKStormChargeSubsystem>() : nullptr;
	if (Storm == nullptr)
	{
		// No subsystem in this world. Not a warning - ShouldCreateSubsystem
		// gates deliberately, and a GameMode exists on the server only.
		return;
	}

	Storm->OnThresholdCrossed.AddDynamic(this, &ALLKTestGameMode::HandleThresholdCrossed);
	Storm->OnWardenConverged.AddDynamic(this, &ALLKTestGameMode::HandleWardenConverged);

	// Applied once at the start too, so a world that somehow begins mid-night
	// does not look calm until the next crossing.
	ApplyStormStage();
}

void ALLKTestGameMode::HandleThresholdCrossed(FName ThresholdId, float Charge)
{
	UE_LOG(LogLLK, Log,
		TEXT("LLKTestGameMode: the storm answers %s at charge %.0f - the harness sky darkens."),
		*ThresholdId.ToString(), Charge);
	ApplyStormStage();
}

void ALLKTestGameMode::HandleWardenConverged()
{
	UE_LOG(LogLLK, Log, TEXT("LLKTestGameMode: the Warden converges - the harness sky turns."));
	ApplyStormStage();
}

void ALLKTestGameMode::ApplyStormStage()
{
	const UWorld* World = GetWorld();
	const ULLKStormChargeSubsystem* Storm =
		World != nullptr ? World->GetSubsystem<ULLKStormChargeSubsystem>() : nullptr;
	if (Storm == nullptr)
	{
		return;
	}

	// Reads the derived stage, never the charge. The point of M2.3 is that
	// "how far into the night are we" has exactly one answer, and everything
	// that reacts asks the same question of it rather than picking its own
	// number out of the meter.
	const FLLKNightStage Stage = Storm->GetNightStage();

	// Multiplicative per stage, so the floor never reaches pure black: a
	// harness nobody can see teaches nothing, and the lantern still has to be
	// the thing that reveals the shop.
	float Dim = FMath::Pow(1.f - StormDimPerStage, static_cast<float>(Stage.Index));

	const float Through = Stage.Count > 0
		? static_cast<float>(Stage.Index) / static_cast<float>(Stage.Count)
		: 0.f;
	FLinearColor SkyColour =
		FMath::Lerp(FLinearColor(0.4f, 0.45f, 0.55f), StormSkyColour, Through);

	if (Stage.bWardenConverged)
	{
		// Row 7: the closing act, not a third patrol state - so it reads as a
		// different thing rather than one more step of the same dimming.
		Dim = FMath::Min(Dim, 1.f - WardenDim);
		SkyColour = WardenSkyColour;
	}

	if (UDirectionalLightComponent* Sun = HarnessSun.Get())
	{
		Sun->SetIntensity(HarnessSunIntensity * Dim);
	}
	if (USkyLightComponent* Sky = HarnessSky.Get())
	{
		Sky->SetIntensity(HarnessSkyIntensity * Dim);
		Sky->SetLightColor(SkyColour);
	}
}

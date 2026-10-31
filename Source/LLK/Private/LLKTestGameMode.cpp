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
#include "Components/BoxComponent.h"
#include "Enemies/LLKSweeper.h"
#include "GameFramework/PlayerStart.h"
#include "Run/LLKExtractionZone.h"
#include "LLK.h"
#include "LLKGameState.h"
#include "Storm/LLKZonePressureSubsystem.h"
#include "Storm/LLKZonePressureSubsystem.h"
#include "LLKUnits.h"
#include "Harness/LLKStoreFloorBuilder.h"
#include "UI/LLKDebugHUD.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/**
	 * Pass two, as one line in the console.
	 *
	 * Off by default because Chris's test order depends on it: pass one reads
	 * the zone and readout with nothing hunting, and an idle Sweeper would
	 * still chase a keeper inside 12 studs. `LLK.Harness.Sweeper 1` then makes
	 * the same harness pass two without a second map.
	 */
	static TAutoConsoleVariable<int32> CVarHarnessSweeper(
		TEXT("LLK.Harness.Sweeper"),
		0,
		TEXT("Spawn one Sweeper and a patrol route in the harness world. 0 = off (default)."),
		ECVF_Default);

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

	bool WorldAlreadyHasTag(const UWorld* World, const FName& Tag)
	{
		if (World == nullptr)
		{
			return false;
		}

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (It->ActorHasTag(Tag))
			{
				return true;
			}
		}
		return false;
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

	// An authored map carries its own floor, fixtures, lights, extraction
	// zone and patrol route. They are level actors, so every client loads them
	// natively. Building the harness over that map makes a server-only second
	// store, and can silently mix its patrol markers with authored ones.
	if (bHasExistingGround)
	{
		UE_LOG(LogLLK, Log,
			TEXT("LLKTestGameMode: authored ground detected; skipping disposable runtime harness."));
		return;
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

	// --- the dock, so the loop can be played rather than only tested -----
	//
	// **Added 2026-10-30 because the alternative was Chris placing cubes.**
	// Everything in M3 - the zone, the hold, the five readout lines,
	// hold-to-call, burden - had never run, and the only thing standing in the
	// way was one actor with one FName typed into it. The harness already
	// builds a floor, a spawn and a light rig at runtime; this is the same
	// trick applied to the one piece of geometry extraction needs.
	//
	// `Floor.ExtractionPoint` is where the builder already decided the way out
	// is - *"the way you came in is the way out"* - so the dock is at the
	// sealed street doors rather than somewhere this function invented. Nothing
	// is written to `Content/`: a spawned actor in a running world, deleted
	// with the rest of the harness the day the authored store arrives.
	if (!WorldAlreadyHas(ALLKExtractionZone::StaticClass()))
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Raised by the box's own half-height so the volume sits ON the floor
		// rather than half inside it, which is the first thing anybody placing
		// one by hand gets wrong.
		const float LiftCm = LLKUnits::DistanceFromStuds(4.f);
		if (ALLKExtractionZone* Dock = World->SpawnActor<ALLKExtractionZone>(
				Floor.ExtractionPoint + FVector(0.f, 0.f, LiftCm), FRotator::ZeroRotator, Params))
		{
			// `LoadingDock` because that is what `data/loot.json` authors, and
			// the zone reports an id nothing authored by name at BeginPlay.
			// The reliable exit, 20-second hold, payout 1.0.
			Dock->ExtractionPointId = TEXT("LoadingDock");

			// A bay mouth rather than a room: wide enough to stand in with a
			// crewmate, shallow enough that stepping out of it is a decision.
			// The real extents come with the authored dock.
			if (UBoxComponent* Trigger = Dock->GetTrigger())
			{
				Trigger->SetBoxExtent(FVector(
					LLKUnits::DistanceFromStuds(7.f),
					LLKUnits::DistanceFromStuds(7.f),
					LiftCm));
			}

			UE_LOG(LogLLK, Log,
				TEXT("LLKTestGameMode: extraction zone for LoadingDock at the street doors. ")
				TEXT("Walk in and hold the interact key."));
		}
	}

	// --- pass two, behind one console variable ---------------------------
	//
	// Chris's test order, 2026-10-30: *"Bare extraction first - one clean
	// call, pause, return, and secure. Then add the Sweeper to that exact same
	// dock bay immediately."* So the Sweeper is OFF by default and the same
	// harness becomes pass two with `LLK.Harness.Sweeper 1` - no second map,
	// which was the other half of his instruction: *"Don't make separate test
	// maps."*
	//
	// **It needs a navmesh to walk**, and this floor is spawned at runtime, so
	// a built navmesh cannot exist for it. Until `RuntimeGeneration=Dynamic`
	// is set - `Config/DefaultEngine.ini`, Codex's lane - a harness Sweeper
	// will notice, telegraph and stand still. That is still pass two's first
	// half: the announcement is the thing row 16 exists for.
	if (CVarHarnessSweeper.GetValueOnGameThread() != 0 && !EnemySpawnPoints.IsEmpty())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (World->SpawnActor<ALLKSweeper>(
				EnemySpawnPoints[0] + FVector(0.f, 0.f, LLKUnits::DistanceFromStuds(4.f)),
				FRotator::ZeroRotator, Params) != nullptr)
		{
			UE_LOG(LogLLK, Log,
				TEXT("LLKTestGameMode: one Sweeper at the service route. It notices a dark keeper ")
				TEXT("at 12 studs and a lit one at 38."));
		}

		// Patrol points at the builder's own enemy spawns, tagged for the pawn
		// to find. The pawn invents no route (row 78), so this is the harness
		// authoring one - and it is the authored positions rather than
		// generated ones, which is row 63's rule about machines appearing
		// somewhere a player can learn.
		if (!WorldAlreadyHasTag(World, ALLKSweeper::PatrolPointTag))
		{
			for (const FVector& Point : EnemySpawnPoints)
			{
				if (AActor* Marker = World->SpawnActor<AActor>(Point, FRotator::ZeroRotator, Params))
				{
					Marker->Tags.Add(ALLKSweeper::PatrolPointTag);
				}
			}
		}
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
	ULLKZonePressureSubsystem* Storm =
		World != nullptr ? World->GetSubsystem<ULLKZonePressureSubsystem>() : nullptr;
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

void ALLKTestGameMode::HandleThresholdCrossed(FName ThresholdId, float Pressure)
{
	UE_LOG(LogLLK, Log,
		TEXT("LLKTestGameMode: the storm answers %s at charge %.0f - the harness sky darkens."),
		*ThresholdId.ToString(), Pressure);
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
	const ULLKZonePressureSubsystem* Storm =
		World != nullptr ? World->GetSubsystem<ULLKZonePressureSubsystem>() : nullptr;
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

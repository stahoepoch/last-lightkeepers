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
#include "LLKUnits.h"
#include "Loot/LLKRelicComponent.h"
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

	// --- something to measure movement against --------------------------
	// Ringed at the perimeter, so a keeper walking in any direction passes
	// edges at a steady rate and speed differences are legible. The aisles
	// below are the near-range reference; these are the long-range one.
	const float Radius = FloorHalfExtent * 0.85f;
	for (int32 i = 0; i < ReferencePillars; ++i)
	{
		const float Angle = 2.f * PI * static_cast<float>(i) / FMath::Max(1, ReferencePillars);
		SpawnBlock(World, Cube,
			HarnessOrigin + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 150.f),
			FVector(1.f, 1.f, 3.f));
	}

	BuildStoreFloor(World, Cube, HarnessOrigin);

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
		World->SpawnActor<APlayerStart>(HarnessOrigin + FVector(0.f, 0.f, 120.f), FRotator::ZeroRotator, Params);
	}

	UE_LOG(LogLLK, Log,
		TEXT("LLKTestGameMode: harness world built - %.0fm floor, %d pillars, ")
		TEXT("sun %.2f / sky %.2f. Temporary; delete with M1.5's grey box."),
		FloorHalfExtent * 2.f / 100.f, ReferencePillars, HarnessSunIntensity, HarnessSkyIntensity);
}

void ALLKTestGameMode::SpawnLoot(UWorld* World, UStaticMesh* Cube, const FVector& Location,
	ELLKLootVisibility InVisibility, FName Id, const FString& DisplayName, int32 Value)
{
	// Small, because a relic is a thing you pick up rather than furniture.
	// 24cm cube: readable as an object on a shelf at a keeper's eye height.
	AStaticMeshActor* Actor = SpawnBlock(World, Cube, Location, FVector(0.24f, 0.24f, 0.24f));
	if (Actor == nullptr)
	{
		return;
	}

	// No collision on loot. A relic you bump into is a relic you knock off a
	// shelf, and physics on a test prop is a distraction from the thing being
	// tested.
	if (UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent())
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// The relic component replicates its legibility, which is useless on an
	// actor that does not replicate at all - and AStaticMeshActor does not by
	// default. Set here so that if Chris does try two machines on this
	// harness, a client sees what the server read rather than nothing, and
	// the result is about the mechanic instead of about the prop.
	Actor->SetReplicates(true);

	ULLKRelicComponent* Relic = NewObject<ULLKRelicComponent>(Actor, ULLKRelicComponent::StaticClass());
	if (Relic == nullptr)
	{
		return;
	}

	Relic->Visibility = InVisibility;
	Relic->RegisterComponent();
	Relic->SetRelicIdentity(Id, FText::FromString(DisplayName), Value);
}

void ALLKTestGameMode::BuildStoreFloor(UWorld* World, UStaticMesh* Cube, const FVector& Origin)
{
	// Everything here is in STUDS and converted once, row 61 - the same
	// authority the keeper's speeds and the lantern's radius go through. A
	// harness is exactly where raw centimetres creep in otherwise.
	const float AisleSpacing = LLKUnits::DistanceFromStuds(14.f);
	const float ShelfSpacing = LLKUnits::DistanceFromStuds(10.f);
	const float ShelfHeight = LLKUnits::DistanceFromStuds(6.f);
	const float ShelfLength = LLKUnits::DistanceFromStuds(8.f);
	const float ShelfDepth = LLKUnits::DistanceFromStuds(2.5f);

	// Aisle spacing against the lantern's 30-stud reach is the thing worth
	// looking at: at 14 studs apart, a lit keeper in one aisle spills light
	// into the next two, which is where "light reveals value and attracts
	// danger" stops being a sentence and becomes a thing you can see
	// happening to you.
	const float FirstAisle = -(ShelfAisles - 1) * 0.5f * AisleSpacing;
	const float FirstShelf = -(ShelvesPerAisle - 1) * 0.5f * ShelfSpacing;

	int32 RelicIndex = 0;
	static const TCHAR* RelicNames[] = {
		TEXT("Engraved pocket watch"),
		TEXT("Enamel brooch"),
		TEXT("Silver locket"),
		TEXT("Staff photograph, 1931"),
		TEXT("Account ledger"),
		TEXT("Auditorium programme"),
	};
	static const TCHAR* BaselineNames[] = {
		TEXT("Bandage"),
		TEXT("Spool of thread"),
		TEXT("Tin of nails"),
		TEXT("Coil of wiring"),
	};

	for (int32 Aisle = 0; Aisle < ShelfAisles; ++Aisle)
	{
		const float X = Origin.X + FirstAisle + Aisle * AisleSpacing;

		for (int32 Shelf = 0; Shelf < ShelvesPerAisle; ++Shelf)
		{
			const float Y = Origin.Y + FirstShelf + Shelf * ShelfSpacing;

			// The shelving unit. Long on Y, thin on X, so the aisles run
			// north-south and the gaps between them are the sightlines.
			SpawnBlock(World, Cube,
				FVector(X, Y, Origin.Z + ShelfHeight * 0.5f),
				FVector(ShelfDepth / 100.f, ShelfLength / 100.f, ShelfHeight / 100.f));

			// Loot on top, alternating so there is always something readable
			// in the dark to contrast the unreadable relics against - lesson
			// one of the ground floor is what lesson two needs to exist.
			const bool bRelicHere = ((Aisle + Shelf) % 3) == 0;
			const FVector LootAt(X, Y, Origin.Z + ShelfHeight + 12.f);

			if (bRelicHere && RelicIndex < UE_ARRAY_COUNT(RelicNames))
			{
				SpawnLoot(World, Cube, LootAt, ELLKLootVisibility::LanternOnly,
					FName(*FString::Printf(TEXT("Relic_%d"), RelicIndex)),
					RelicNames[RelicIndex], 300 + RelicIndex * 75);
				++RelicIndex;
			}
			else if (((Aisle * ShelvesPerAisle + Shelf) % 4) == 1)
			{
				const int32 Which = (Aisle + Shelf) % UE_ARRAY_COUNT(BaselineNames);
				SpawnLoot(World, Cube, LootAt, ELLKLootVisibility::Always,
					FName(*FString::Printf(TEXT("Baseline_%d_%d"), Aisle, Shelf)),
					BaselineNames[Which], 5);
			}
		}
	}

	// A counter line across one end. Not decoration either: it is a long
	// unbroken edge, which is the clearest thing to judge sprint against, and
	// it gives the floor an orientation so "which way did I come in" has an
	// answer.
	const float CounterY = Origin.Y + FirstShelf - ShelfSpacing * 1.6f;
	SpawnBlock(World, Cube,
		FVector(Origin.X, CounterY, Origin.Z + LLKUnits::DistanceFromStuds(1.6f)),
		FVector(LLKUnits::DistanceFromStuds(1.5f) / 100.f,
			LLKUnits::DistanceFromStuds(34.f) / 100.f,
			LLKUnits::DistanceFromStuds(3.2f) / 100.f));

	UE_LOG(LogLLK, Log,
		TEXT("LLKTestGameMode: store floor - %d aisles x %d shelves, %d relics, aisle spacing %.0fcm ")
		TEXT("against a %.0fcm lantern. Temporary; M1.5's grey box replaces all of it."),
		ShelfAisles, ShelvesPerAisle, RelicIndex, AisleSpacing,
		LLKUnits::DistanceFromStuds(30.f));
}

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
	// Ringed rather than scattered, so a keeper walking in any direction
	// passes edges at a steady rate and speed differences are legible.
	const float Radius = FloorHalfExtent * 0.6f;
	for (int32 i = 0; i < ReferencePillars; ++i)
	{
		const float Angle = 2.f * PI * static_cast<float>(i) / FMath::Max(1, ReferencePillars);
		SpawnBlock(World, Cube,
			HarnessOrigin + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 150.f),
			FVector(1.f, 1.f, 3.f));
	}

	// --- light, and nothing persistent ---------------------------------
	// See the header. Two spawned actors in a running world; no project
	// lighting value is read or written, and DONTFUCKUPTHELIGHTS.md is
	// untouched by design.
	if (!WorldAlreadyHas(ADirectionalLight::StaticClass()))
	{
		if (ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(
				FVector(0.f, 0.f, 1000.f), FRotator(-46.f, -30.f, 0.f)))
		{
			Sun->SetMobility(EComponentMobility::Movable);
			if (UDirectionalLightComponent* Component = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
			{
				Component->SetIntensity(3.f);
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
				Component->SetIntensity(1.f);
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
		TEXT("LLKTestGameMode: harness world built - %.0fm floor, %d pillars. ")
		TEXT("Temporary; delete with M1.5's grey box."),
		FloorHalfExtent * 2.f / 100.f, ReferencePillars);
}

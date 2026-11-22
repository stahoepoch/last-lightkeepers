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
#include "Enemies/LLKHearing.h"
#include "World/LLKPushFeel.h"
#include "World/LLKPushableFixture.h"
#include "Enemies/LLKSweeper.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/WorldSettings.h"
#include "Run/LLKExtractionZone.h"
#include "LLK.h"
#include "LLKGameState.h"
#include "Storm/LLKZonePressureSubsystem.h"
#include "Storm/LLKZonePressureSubsystem.h"
#include "LLKUnits.h"
#include "Lantern/LLKLanternFeel.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Lantern/LLKLanternPickup.h"
#include "Enemies/LLKSweeper.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Loot/LLKInteractorComponent.h"
#include "TimerManager.h"
#include "Harness/LLKMapReadiness.h"
#include "Harness/LLKStoreFloorBuilder.h"
#include "UI/LLKDebugHUD.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// A WorldSettings actor is part of the map package, not a World Partition
	// external actor.  It is therefore available during InitGame, before any
	// streamed actor can answer a content-based question.  The harness is
	// deliberately opt-in: a map must carry this tag to receive runtime
	// scaffolding.


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

	// Do not ask whether an authored map already has a floor, zone, patrol, or
	// relic here.  On a World Partition map those actors may not have streamed
	// during InitGame yet, which makes every one of those questions answer
	// false and produces a second, server-only store.  Map intent is stable at
	// this point: only an explicit WorldSettings tag enables the harness.
	const AWorldSettings* WorldSettings = World->GetWorldSettings();
	if (WorldSettings == nullptr || !WorldSettings->ActorHasTag(LLKMapReadiness::HarnessMapTag))
	{
		UE_LOG(LogLLK, Log,
			TEXT("LLKTestGameMode: no LLK.Harness tag on WorldSettings; skipping runtime harness."));
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

	UWorld* World = GetWorld();

	// **Next tick, not now, and scheduled above the early return below.**
	//
	// Two reasons, both load-bearing. The registry fills from each pickup's
	// BeginPlay, and actor BeginPlay order inside
	// AWorldSettings::NotifyBeginPlay is not specified - this GameMode is in
	// that same pass, so counting here could report zero in a map that has
	// one. A diagnostic that can be wrong about the one thing it exists to
	// report is worse than none at all; a frame of delay cannot be.
	//
	// And the Storm lookup below returns out of BeginPlay in any world
	// without that subsystem, so anything scheduled after it is not
	// scheduled in every world.
	if (World != nullptr)
	{
		World->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(
				this, &ALLKTestGameMode::ReportLanternPickups));
		World->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(
				this, &ALLKTestGameMode::ReportThreatReachability));
		// Next tick like its siblings, not inline: level actors are not all
		// spawned yet at this point, and an inline count would report zero
		// pushables in a map that has them.
		World->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(
				this, &ALLKTestGameMode::ReportPushables));
	}

	// Bound here rather than in InitGame, where the harness world is built:
	// the subsystem exists by then, but binding before the lights have been
	// spawned means a crossing during startup would drive nothing.
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

void ALLKTestGameMode::ReportLanternPickups()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// The same registry the interactor reads, so this cannot disagree with
	// what E can actually find. Asking a second source would make this a
	// report about a different world than the one being played.
	const TArray<ALLKLanternPickup*> Pickups =
		ALLKLanternPickup::GetLivePickups(World);
	const FString MapName = World->GetMapName();

	if (Pickups.Num() == 0)
	{
		// **A Warning on purpose, and unconditional on purpose.** Several
		// maps legitimately have no lantern - the harness floor, the route
		// and apparel passes - so this will speak in all of them. That is
		// the cost of it speaking in the one where somebody is standing
		// still pressing a key that cannot work. It does not name the map
		// that does have the lantern: a map name spelled into a log string
		// is a thing that goes stale silently, which is the exact failure
		// shape this project keeps finding in its own checks.
		UE_LOG(LogLLK, Warning,
			TEXT("No lantern pickup in %s. The keeper starts empty-handed, so "
				 "E will never offer one and F will refuse - this map has "
				 "nothing to find. If you expected a lantern here, it is "
				 "either a different map or it was never saved into this one."),
			*MapName);
		return;
	}

	// The keeper's own reach, read off the component that enforces it. A
	// number repeated here would be a number that goes stale the first time
	// somebody tunes the real one, and this check exists precisely to catch
	// a reach that cannot work.
	const APawn* Keeper = World->GetFirstPlayerController() != nullptr
		? World->GetFirstPlayerController()->GetPawn()
		: nullptr;
	const ULLKInteractorComponent* Interactor = Keeper != nullptr
		? Keeper->FindComponentByClass<ULLKInteractorComponent>()
		: nullptr;

	for (const ALLKLanternPickup* Pickup : Pickups)
	{
		// The interaction point, not the actor location - that is the point
		// the reach and the sight line are measured to, so it is the one
		// worth reading next to a complaint about not being able to take it.
		const FVector At = Pickup->GetInteractionPoint();
		UE_LOG(LogLLK, Log,
			TEXT("Lantern pickup in %s: %s, reached at (%.0f, %.0f, %.0f)%s."),
			*MapName, *Pickup->GetName(), At.X, At.Y, At.Z,
			Pickup->IsTaken() ? TEXT(" - already taken") : TEXT(""));

		if (Interactor == nullptr)
		{
			continue;
		}

		// **The check that would have caught the double-counted eye at
		// frame one.** Reach is measured eye-to-lantern, so the vertical
		// gap is a floor on the distance: no amount of walking closer can
		// get under it. If that gap alone exceeds the reach, the pickup is
		// unreachable from every position in the level and no amount of
		// playing will find that out - which is exactly what happened on
		// 2026-11-15, when the eye sat 189 cm above a lantern a 150 cm
		// reach was supposed to span.
		const FVector Eye =
			LLKLanternFeel::KeeperEyeWorldPosition(Keeper->GetActorLocation());
		const float VerticalGap = FMath::Abs(Eye.Z - At.Z);
		if (VerticalGap >= Interactor->LanternReachCm)
		{
			UE_LOG(LogLLK, Warning,
				TEXT("  and it cannot be reached from anywhere: the keeper's "
					 "eye is %.0f cm above it and the reach is %.0f cm. Reach "
					 "is measured eye-to-lantern, so the height difference is "
					 "a floor on the distance - walking closer cannot help."),
				VerticalGap, Interactor->LanternReachCm);
		}
	}
}

void ALLKTestGameMode::ReportPushables()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	int32 Found = 0;
	for (TActorIterator<ALLKPushableFixture> It(World); It; ++It)
	{
		const ALLKPushableFixture* Fixture = *It;
		if (Fixture == nullptr)
		{
			continue;
		}

		++Found;
		// Read off the actor rather than restated from the class defaults,
		// so an instance somebody retuned in the level reports its own
		// numbers and not the ones this file remembers.
		UE_LOG(LogLLK, Display,
			TEXT("%s is pushable: %.1f studs of rail, %.1f s end to end, and "
				 "%.0f studs loud while it moves (a sprint is 12.5, and this "
				 "machine's lit sight is 38)."),
			*Fixture->GetName(), Fixture->TravelStuds,
			LLKPushFeel::PushSpeedStuds > 0.f
				? Fixture->TravelStuds / LLKPushFeel::PushSpeedStuds : -1.f,
			LLKPushFeel::PushNoiseStuds);
	}

	if (Found == 0)
	{
		// **A Warning, and the point of the whole report.** The runtime, the
		// interface, the noise and both test layers can all be correct while
		// no level contains a single pushable - and that is this project's
		// recurring defect exactly: a correct system over an absent thing,
		// with a clean log. Nothing else in the build would say so.
		UE_LOG(LogLLK, Warning,
			TEXT("No pushable fixture in %s. The push verb exists in code and "
				 "there is nothing in this map to use it on, so no player can "
				 "discover it here. Place an ALLKPushableFixture and author "
				 "its LocalPushAxis and TravelStuds."),
			*World->GetMapName());
	}
}

void ALLKTestGameMode::ReportThreatReachability()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const APlayerController* PC = World->GetFirstPlayerController();
	const APawn* Keeper = PC != nullptr ? PC->GetPawn() : nullptr;
	if (Keeper == nullptr)
	{
		// No keeper yet is not a finding. Reporting one would make every
		// menu map and every dedicated-server boot look broken.
		return;
	}

	int32 Machines = 0;
	for (TActorIterator<ALLKSweeper> It(World); It; ++It)
	{
		++Machines;
	}
	if (Machines == 0)
	{
		UE_LOG(LogLLK, Log,
			TEXT("No machine in %s. Nothing in this map can threaten the "
				 "keeper, which is correct for a quiet map and wrong for a "
				 "chapter."), *World->GetMapName());
		return;
	}

	// **The instrument, before any verdict.** Without a navigation system
	// there is no path to find and no finding to report: every machine would
	// look unreachable for one reason that has nothing to do with any of
	// them. This project has twice reported a map defect that was a broken
	// query, so the distinction gets its own branch.
	const UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(World);
	if (Nav == nullptr)
	{
		UE_LOG(LogLLK, Warning,
			TEXT("%d machine(s) in %s and NO NAVIGATION SYSTEM, so none of "
				 "them can pursue anything. This is not a report that they "
				 "cannot reach the keeper - it is a report that nothing can "
				 "be asked."), Machines, *World->GetMapName());
		return;
	}

	const FVector KeeperAt = Keeper->GetActorLocation();

	for (TActorIterator<ALLKSweeper> It(World); It; ++It)
	{
		const ALLKSweeper* Machine = *It;
		if (Machine == nullptr)
		{
			continue;
		}

		const FVector MachineAt = Machine->GetActorLocation();
		const float Straight = FVector::Dist(MachineAt, KeeperAt);

		// **What this machine can HEAR from where it stands, reported before
		// the path, because hearing does not need one.** A machine with no
		// navmesh between it and the keeper can still be sent looking by a
		// sound, and the whole point of the sense is that it operates on the
		// keeper's gait rather than on the geometry.
		//
		// Reported affirmatively, and it is the navmesh lesson for the second
		// time: a keeper standing still is correctly silent, so a Sweeper with
		// working ears and a Sweeper with none produce the same empty log in
		// every automated boot - nobody moves in an unattended one. "Nothing
		// complained" is not evidence a sense exists.
		//
		// It reports the THRESHOLD rather than the model: the speed a keeper at
		// this distance would have to exceed to be heard from here, against
		// the gaits in `data/lantern.json` - crouch 3, walk 5, sprint 12.5.
		// One number, and a playtest can tell at a glance whether this machine
		// is reachable by a run, by a walk, or not at all.
		const float AudibleAboveStuds =
			Straight / LLKUnits::StudsToCm / LLKHearing::SecondsOfTravel;
		UE_LOG(LogLLK, Display,
			TEXT("%s has ears: from %.1f studs away it hears a keeper moving faster ")
			TEXT("than %.1f studs/s (crouch 3, walk 5, sprint 12.5). Standing still ")
			TEXT("is silent at any distance."),
			*Machine->GetName(), Straight / LLKUnits::StudsToCm, AudibleAboveStuds);

		// Its real speed, off the component that moves it, rather than a
		// number repeated here that would go stale the first time the roster
		// is retuned.
		float SpeedCms = 0.f;
		if (const UCharacterMovementComponent* Movement = Machine->GetCharacterMovement())
		{
			SpeedCms = Movement->MaxWalkSpeed;
		}

		double PathLength = 0.0;
		const ENavigationQueryResult::Type Result =
			UNavigationSystemV1::GetPathLength(World, MachineAt, KeeperAt, PathLength);

		switch (Result)
		{
		case ENavigationQueryResult::Success:
		{
			const float Detour = Straight > 1.f
				? static_cast<float>(PathLength) / Straight : 1.f;
			const float Seconds = SpeedCms > 1.f
				? static_cast<float>(PathLength) / SpeedCms : -1.f;
			UE_LOG(LogLLK, Log,
				TEXT("%s can reach the keeper: %.0f cm of path over %.0f cm "
					 "straight (%.2fx), %.1f s at %.0f cm/s."),
				*Machine->GetName(), PathLength, Straight, Detour, Seconds,
				SpeedCms);
			break;
		}
		case ENavigationQueryResult::Fail:
			// The one case that is about the map.
			UE_LOG(LogLLK, Warning,
				TEXT("%s CANNOT REACH THE KEEPER. %.0f cm apart in a straight "
					 "line and no path between them, so it can telegraph and "
					 "pursue forever without ever arriving. Check the navmesh "
					 "between them."),
				*Machine->GetName(), Straight);
			break;
		case ENavigationQueryResult::Error:
		case ENavigationQueryResult::Invalid:
		default:
		{
			// **An Error is a question, not an answer**, and left as one it
			// is the least useful line a log can carry: it names a failure
			// without naming a cause, which is how the unreachable crate
			// survived for a day. So the three things that produce it get
			// asked directly, and whichever is true gets said.
			// **IsNavigationBuilt is not the authority here, and trusting it
			// cost a wrong headline on 2026-11-21.** It reported YES for a
			// RecastNavMesh whose bounds were (0,0,0)-(0,0,0): it answers
			// "nothing is pending", not "tiles exist", and an unbuilt navmesh
			// has nothing pending precisely because nobody ever asked for it.
			// The fact that decides this is whether any navigation data has
			// non-empty bounds.
			const bool bNothingPending =
				Nav->IsNavigationBuilt(World->GetWorldSettings());

			int32 NavDataActors = 0;
			int32 NavDataWithTiles = 0;
			for (TActorIterator<ANavigationData> NavIt(World); NavIt; ++NavIt)
			{
				++NavDataActors;
				const FBox NavBounds = NavIt->GetBounds();
				if (NavBounds.IsValid != 0 && NavBounds.GetVolume() > 0.0)
				{
					++NavDataWithTiles;
				}
			}

			FNavLocation MachineOnMesh;
			const bool bMachineOnMesh = Nav->ProjectPointToNavigation(
				MachineAt, MachineOnMesh, FVector(400.f, 400.f, 600.f));
			FNavLocation KeeperOnMesh;
			const bool bKeeperOnMesh = Nav->ProjectPointToNavigation(
				KeeperAt, KeeperOnMesh, FVector(400.f, 400.f, 600.f));

			UE_LOG(LogLLK, Warning,
				TEXT("%s: path query returned %d, which is neither success nor "
					 "failure. Why: %d of %d navigation data actor(s) carry "
					 "any built tiles, machine projects onto the mesh %s, "
					 "keeper projects %s. %s"),
				*Machine->GetName(), static_cast<int32>(Result),
				NavDataWithTiles, NavDataActors,
				bMachineOnMesh ? TEXT("YES") : TEXT("NO"),
				bKeeperOnMesh ? TEXT("YES") : TEXT("NO"),
				(NavDataActors == 0)
					? TEXT("With no navigation data in the world nothing can "
						   "path at all, and the machine's pursuit is inert.")
					: (NavDataWithTiles == 0
						? TEXT("THE NAVMESH HAS NO BUILT TILES, so nothing in "
							   "this map can path anywhere and every machine "
							   "here can notice and telegraph and then never "
							   "arrive. Build paths and save the map.")
						: TEXT("Tiles exist somewhere, so the failure is about "
							   "these two positions rather than the map as a "
							   "whole.")));

			// **Where the mesh actually is.** "It does not project" is true
			// and unactionable; the bounds against the positions tell whoever
			// owns the map which volume to move and how far. Logged only on
			// the failing branch, so a healthy map stays quiet.
			if (!bMachineOnMesh || !bKeeperOnMesh)
			{
				UE_LOG(LogLLK, Warning,
					TEXT("  IsNavigationBuilt says %s, which is why it is not "
						 "the test used above"),
					bNothingPending ? TEXT("nothing pending") : TEXT("pending work"));
				UE_LOG(LogLLK, Warning,
					TEXT("  machine at (%.0f, %.0f, %.0f), keeper at "
						 "(%.0f, %.0f, %.0f)"),
					MachineAt.X, MachineAt.Y, MachineAt.Z,
					KeeperAt.X, KeeperAt.Y, KeeperAt.Z);

				for (TActorIterator<ANavigationData> NavIt(World); NavIt; ++NavIt)
				{
					const FBox Bounds = NavIt->GetBounds();
					UE_LOG(LogLLK, Warning,
						TEXT("  nav data %s covers (%.0f, %.0f, %.0f) to "
							 "(%.0f, %.0f, %.0f)%s"),
						*NavIt->GetName(),
						Bounds.Min.X, Bounds.Min.Y, Bounds.Min.Z,
						Bounds.Max.X, Bounds.Max.Y, Bounds.Max.Z,
						Bounds.IsValid == 0 ? TEXT(" - EMPTY BOUNDS") : TEXT(""));
				}

				int32 Volumes = 0;
				for (TActorIterator<ANavMeshBoundsVolume> VolIt(World); VolIt; ++VolIt)
				{
					++Volumes;
					FVector Origin = FVector::ZeroVector;
					FVector Extent = FVector::ZeroVector;
					VolIt->GetActorBounds(false, Origin, Extent);
					UE_LOG(LogLLK, Warning,
						TEXT("  bounds volume %s centred (%.0f, %.0f, %.0f) "
							 "extent (%.0f, %.0f, %.0f) - keeper is %s it"),
						*VolIt->GetName(), Origin.X, Origin.Y, Origin.Z,
						Extent.X, Extent.Y, Extent.Z,
						FBox(Origin - Extent, Origin + Extent).IsInside(KeeperAt)
							? TEXT("INSIDE") : TEXT("OUTSIDE"));
				}
				if (Volumes == 0)
				{
					UE_LOG(LogLLK, Warning,
						TEXT("  and there is no NavMeshBoundsVolume in the "
							 "level, so nothing tells Recast where to build."));
				}
			}
			break;
		}
		}
	}
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

#include "Harness/LLKMapReadiness.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/LightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/IConsoleManager.h"
#include "Harness/LLKStoreFloorPlan.h"
#include "Lantern/LLKLanternFeel.h"
#include "LLKUnits.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "NavigationData.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"

namespace LLKMapReadiness
{
	const FName EnemySpawnTag(TEXT("LLK.EnemySpawn"));
	const FName TestLaneOpenTag(TEXT("LLK.TestLane.Open"));
	const FName TestLaneBlockedTag(TEXT("LLK.TestLane.Blocked"));

	// Deferred rather than re-declared: LLKLanternFeel owns this number,
	// because the gameplay light anchor is derived from it and that is the
	// copy whose drift would be noticed last. See its header.
	const float EyeHeightCm = LLKLanternFeel::KeeperEyeHeightCm;

	namespace
	{
		/** How many directions the sightline sweep probes. */
		constexpr int32 SweepDirections = 32;

		/**
		 * How far each sweep ray goes, in studs.
		 *
		 * Longer than the limit it is checking, which is the whole point: a
		 * probe that stops at 34 studs can never report a 61-stud sightline,
		 * it can only report "34 and still going", and the number Chris needs
		 * in order to decide where a shelf goes is the actual one. 80 studs is
		 * most of the floor's 120-stud depth.
		 */
		constexpr float SweepReachStuds = 80.f;

		/**
		 * The range a sight break has to exist inside to count as cover.
		 *
		 * `Sweepers.aggroRadiusLit` is 38 studs - the distance a lit keeper is
		 * noticed from - so geometry further away than that does not help the
		 * keeper who is being hunted right now. A floor whose only occlusion
		 * is the back wall has no cover, however many walls it has.
		 */
		constexpr float CoverMattersWithinStuds = 38.f;

		/** Every actor carrying this tag, in level order. */
		TArray<const AActor*> ActorsTagged(const UWorld* World, const FName& Tag)
		{
			TArray<const AActor*> Found;
			if (World == nullptr)
			{
				return Found;
			}

			for (TActorIterator<AActor> It(World); It; ++It)
			{
				if (It->Tags.Contains(Tag))
				{
					Found.Add(*It);
				}
			}
			return Found;
		}
	}

	FLLKMapReport Evaluate(const FLLKMapObservations& Obs)
	{
		FLLKMapReport Report;

		const auto Add = [&Report](ELLKMapCheck Check, FString Detail)
		{
			FLLKMapFinding Finding;
			Finding.Check = Check;
			Finding.Detail = MoveTemp(Detail);
			Report.Findings.Add(MoveTemp(Finding));
		};

		// ---- the things everything else is measured from -----------------
		//
		// No spawn means the sweep, the floor trace and the nav coverage test
		// were all taken from the world origin, so reporting them would be
		// reporting measurements of nowhere. One real failure beats five
		// derived ones.
		if (!Obs.bHasPlayerSpawn)
		{
			Add(ELLKMapCheck::NoPlayerSpawn,
				TEXT("no APlayerStart in the level - everything else is measured from it"));

			if (Obs.EnemySpawnPoints <= 0)
			{
				Add(ELLKMapCheck::NoEnemySpawnPoints,
					FString::Printf(TEXT("no actors tagged %s"), *EnemySpawnTag.ToString()));
			}
			return Report;
		}

		if (Obs.EnemySpawnPoints <= 0)
		{
			Add(ELLKMapCheck::NoEnemySpawnPoints,
				FString::Printf(TEXT("no actors tagged %s"), *EnemySpawnTag.ToString()));
		}

		if (!Obs.bFloorUnderSpawn)
		{
			Add(ELLKMapCheck::FloorHasNoCollision,
				TEXT("a downward trace from above the keeper spawn hit nothing solid"));
		}

		if (Obs.VisualEnvironmentReachesClients.IsSet()
			&& !Obs.VisualEnvironmentReachesClients.GetValue())
		{
			Add(ELLKMapCheck::GeometryServerOnly,
				TEXT("playable geometry or lighting is runtime-spawned and non-replicated, "
					"with no placed equivalent - clients can render black"));
		}

		// ---- navigation ---------------------------------------------------
		if (Obs.NavBoundsVolumes <= 0)
		{
			Add(ELLKMapCheck::NavBoundsMissing,
				TEXT("no NavMeshBoundsVolume - without one no navmesh is ever built"));
		}
		else if (Obs.NavDataActors <= 0)
		{
			// Only worth saying when a volume exists. "No nav data" on a map
			// with no volume is the same failure said twice.
			Add(ELLKMapCheck::NavDataMissing,
				FString::Printf(
					TEXT("%d nav bounds volume(s) but no nav data actor - the level may not have been saved after the volume was added"),
					Obs.NavBoundsVolumes));
		}

		if (Obs.NavBoundsVolumes > 0 && Obs.SpawnPointsOutsideNavBounds > 0)
		{
			Add(ELLKMapCheck::NavBoundsDoesNotCoverSpawn,
				FString::Printf(TEXT("%d spawn point(s) outside every nav bounds volume, first at %s"),
					Obs.SpawnPointsOutsideNavBounds,
					*Obs.FirstSpawnPointOutsideNavBounds.ToCompactString()));
		}

		// The nav query half, as a chain rather than as four independent
		// questions. **Each answer invalidates the ones below it**, and
		// reporting them anyway is the "one failure as five" mistake: a route
		// query against an unbuilt navmesh and a projection of a spawn onto a
		// mesh that does not exist yet both produce a false that means
		// nothing. So the first real failure is the only one said.
		//
		// **The foundation is the first link, and it was missing until
		// 2026-10-29.** With no bounds volume or no nav data there is no mesh
		// to project onto, so `ProjectPointToNavigation` correctly answers
		// false and `SpawnOffNavmesh` fires on a map whose `PlayerStart` is in
		// exactly the right place. Worse, it is the more actionable-looking of
		// the two findings, so it is the one somebody acts on.
		//
		// The reason that slipped through is specific and worth keeping:
		// `UNavigationSystemV1::IsNavigationBuilt` returns **true** when there
		// is nowhere to build navigation (`NavigationSystem.cpp:2555`), so on a
		// map with no volume the navmesh reports itself built - vacuously, and
		// truthfully, since nothing was asked to generate. `NavigationBuilt`
		// therefore cannot carry this; only the counts can.
		const bool bNavFoundationMissing = Obs.NavBoundsVolumes <= 0 || Obs.NavDataActors <= 0;

		if (bNavFoundationMissing)
		{
			// NavBoundsMissing or NavDataMissing has already been reported
			// above. Everything below would be measuring against a mesh that
			// does not exist.
		}
		else if (Obs.NavigationBuilt.IsSet() && !Obs.NavigationBuilt.GetValue())
		{
			if (Obs.NavDataActors > 0)
			{
				Add(ELLKMapCheck::NavmeshNotBuilt,
					TEXT("nav data exists and has not finished generating - "
						"navmesh generation is asynchronous, so a check run too early sees this"));
			}
			// With no nav data actor at all, NavDataMissing above already
			// said it. Either way, nothing below could have been asked
			// meaningfully.
		}
		else if (Obs.SpawnOnNavmesh.IsSet() && !Obs.SpawnOnNavmesh.GetValue())
		{
			Add(ELLKMapCheck::SpawnOffNavmesh,
				TEXT("the keeper spawn does not project onto navigable space - "
					"usually a PlayerStart sunk into the floor, floating above it, or on a counter"));
		}
		else if (Obs.EnemySpawnsOffNavmesh.Get(0) > 0)
		{
			// Before the route, because every path TO an off-mesh point comes
			// back partial. Reporting the route instead would be true and
			// would describe a blocked building rather than a misplaced actor.
			Add(ELLKMapCheck::EnemySpawnOffNavmesh,
				FString::Printf(
					TEXT("%d enemy spawn point(s) do not project onto navigable space, first at %s"),
					Obs.EnemySpawnsOffNavmesh.GetValue(),
					*Obs.FirstEnemySpawnOffNavmesh.ToCompactString()));
		}
		else if (!Obs.RouteFromSpawnReaches.IsSet())
		{
			Add(ELLKMapCheck::RouteUnverified,
				TEXT("no navmesh path query ran - see the handoff in docs/MAP-READINESS.md"));
		}
		else if (!Obs.RouteFromSpawnReaches.GetValue())
		{
			Add(ELLKMapCheck::NoRouteFromSpawn,
				TEXT("at least one enemy spawn point cannot be walked to from the keeper spawn"));
		}

		// ---- occlusion and sightlines -------------------------------------
		//
		// Both of these need the sweep to have actually run. A map that was
		// never probed has zero blocked directions, which would read as "no
		// occluding geometry" and be a lie about an unmeasured floor.
		if (Obs.ProbedDirections > 0)
		{
			if (Obs.BlockedDirections <= 0)
			{
				Add(ELLKMapCheck::NoOccludingGeometry,
					FString::Printf(
						TEXT("none of %d directions from the keeper spawn hit anything within %.0f studs - perception degenerates to raw distance"),
						Obs.ProbedDirections, CoverMattersWithinStuds));
			}

			if (Obs.LongestSightlineStuds > Obs.MaxSightlineStuds)
			{
				Add(ELLKMapCheck::SightlineTooLong,
					FString::Printf(TEXT("longest sightline %.1f studs, limit %.1f"),
						Obs.LongestSightlineStuds, Obs.MaxSightlineStuds));
			}
		}

		// ---- the two lanes, which prove the trace can say both words ------
		if (Obs.OpenLaneEnds != 2)
		{
			Add(ELLKMapCheck::TestLaneOpenMissing,
				FString::Printf(TEXT("%d actor(s) tagged %s, need exactly 2"),
					Obs.OpenLaneEnds, *TestLaneOpenTag.ToString()));
		}
		else if (Obs.OpenLaneResult.Get(ELLKVisibility::Unknown) != ELLKVisibility::Clear)
		{
			Add(ELLKMapCheck::TestLaneOpenBlocked,
				TEXT("the open lane did not report Clear, so a Clear answer is unproven on this map"));
		}

		if (Obs.BlockedLaneEnds != 2)
		{
			Add(ELLKMapCheck::TestLaneBlockedMissing,
				FString::Printf(TEXT("%d actor(s) tagged %s, need exactly 2"),
					Obs.BlockedLaneEnds, *TestLaneBlockedTag.ToString()));
		}
		else if (Obs.BlockedLaneResult.Get(ELLKVisibility::Unknown) != ELLKVisibility::Blocked)
		{
			Add(ELLKMapCheck::TestLaneBlockedClear,
				TEXT("the blocked lane did not report Blocked, so a Blocked answer is unproven on this map"));
		}

		return Report;
	}

	FLLKMapObservations Observe(UWorld* World, const FLLKStoreFloorPlan& Plan)
	{
		FLLKMapObservations Obs;
		Obs.MaxSightlineStuds = Plan.MaxSightlineStuds;

		if (World == nullptr)
		{
			return Obs;
		}

		// ---- the spawn ----------------------------------------------------
		FVector KeeperSpawn = FVector::ZeroVector;
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			KeeperSpawn = It->GetActorLocation();
			Obs.bHasPlayerSpawn = true;
			break;
		}

		const TArray<const AActor*> EnemySpawns = ActorsTagged(World, EnemySpawnTag);
		Obs.EnemySpawnPoints = EnemySpawns.Num();

		if (!Obs.bHasPlayerSpawn)
		{
			// Everything below is measured from the spawn. Measuring it from
			// the origin would produce numbers that look like findings.
			return Obs;
		}

		const FVector Eye = FVector(KeeperSpawn.X, KeeperSpawn.Y, KeeperSpawn.Z + EyeHeightCm);

		FCollisionQueryParams Params(SCENE_QUERY_STAT(LLKMapReadiness), /*bTraceComplex=*/false);

		// ---- requirement 1: something solid underneath --------------------
		Obs.bFloorUnderSpawn = World->LineTraceTestByChannel(
			KeeperSpawn + FVector(0.f, 0.f, 200.f),
			KeeperSpawn - FVector(0.f, 0.f, 500.f),
			ECC_WorldStatic, Params);

		// ---- requirements 2, 3, 4: navigation -----------------------------
		// ---- requirement 15: the visual world reaches clients --------------
		//
		// A server cannot truthfully ask whether a client rendered this level.
		// This is consequently an intentionally narrow detector for the one
		// known bad shape: the harness created its floor, walls, sun and sky at
		// runtime from GameMode; none were replicated and clients saw black.
		// Placed actors carry RF_WasLoaded and load independently on every
		// client. Pawns are excluded so a keeper's replicated lantern cannot
		// make an otherwise empty world look valid.
		bool bFoundRuntimeVisual = false;
		bool bFoundPlacedVisual = false;
		bool bAllRuntimeVisualsReplicate = true;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor == nullptr || Actor->IsA<APawn>())
			{
				continue;
			}

			bool bIsWorldVisual = false;
			for (UActorComponent* Component : Actor->GetComponents())
			{
				if (const ULightComponent* Light = Cast<ULightComponent>(Component))
				{
					bIsWorldVisual = Light->IsRegistered();
				}
				else if (const UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
				{
					bIsWorldVisual = Primitive->IsRegistered()
						&& Primitive->GetCollisionObjectType() == ECC_WorldStatic
						&& Primitive->GetCollisionEnabled() != ECollisionEnabled::NoCollision;
				}

				if (bIsWorldVisual)
				{
					break;
				}
			}

			if (!bIsWorldVisual)
			{
				continue;
			}

			if (Actor->HasAnyFlags(RF_WasLoaded))
			{
				bFoundPlacedVisual = true;
			}
			else
			{
				bFoundRuntimeVisual = true;
				bAllRuntimeVisualsReplicate &= Actor->GetIsReplicated();
			}
		}

		if (bFoundRuntimeVisual)
		{
			Obs.VisualEnvironmentReachesClients = bFoundPlacedVisual || bAllRuntimeVisualsReplicate;
		}

		TArray<FBox> NavBounds;
		for (TActorIterator<ANavMeshBoundsVolume> It(World); It; ++It)
		{
			++Obs.NavBoundsVolumes;
			NavBounds.Add(It->GetComponentsBoundingBox(/*bNonColliding=*/true));
		}

		for (TActorIterator<ANavigationData> It(World); It; ++It)
		{
			++Obs.NavDataActors;
		}

		if (!NavBounds.IsEmpty())
		{
			TArray<FVector> MustBeCovered;
			MustBeCovered.Add(KeeperSpawn);
			for (const AActor* Spawn : EnemySpawns)
			{
				MustBeCovered.Add(Spawn->GetActorLocation());
			}

			for (const FVector& Point : MustBeCovered)
			{
				const bool bCovered = NavBounds.ContainsByPredicate(
					[&Point](const FBox& Box) { return Box.IsInsideOrOn(Point); });

				if (!bCovered)
				{
					if (Obs.SpawnPointsOutsideNavBounds == 0)
					{
						Obs.FirstSpawnPointOutsideNavBounds = Point;
					}
					++Obs.SpawnPointsOutsideNavBounds;
				}
			}
		}

		// NavigationSystem complete. The typed queries below
		// replace the former class-name workaround and leave unanswered checks
		// unset rather than calling them passes.
		// NavigationSystem's query APIs take a mutable UWorld*. Observe does
		// not mutate the map; the non-const parameter exists only because the
		// engine does not express that read-only contract in its API.
		if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World))
		{
			Obs.NavigationBuilt = NavSys->IsNavigationBuilt(World->GetWorldSettings());
			if (Obs.NavigationBuilt.Get(false))
			{
				FNavLocation ProjectedKeeperSpawn;
				// A 50cm horizontal / 100cm vertical tolerance accepts a normal
				// PlayerStart at floor height, but does not turn a spawn floating
				// metres above the floor into a false pass.
				const FVector ProjectionExtent(50.f, 50.f, 100.f);
				Obs.SpawnOnNavmesh = NavSys->ProjectPointToNavigation(
					KeeperSpawn, ProjectedKeeperSpawn, ProjectionExtent);

				if (Obs.SpawnOnNavmesh.Get(false) && !EnemySpawns.IsEmpty())
				{
					int32 OffNavmeshCount = 0;
					bool bEverySpawnReaches = true;
					for (const AActor* Spawn : EnemySpawns)
					{
						FNavLocation ProjectedEnemySpawn;
						if (!NavSys->ProjectPointToNavigation(
							Spawn->GetActorLocation(), ProjectedEnemySpawn, ProjectionExtent))
						{
							if (OffNavmeshCount == 0)
							{
								Obs.FirstEnemySpawnOffNavmesh = Spawn->GetActorLocation();
							}
							++OffNavmeshCount;
							bEverySpawnReaches = false;
							continue;
						}

						UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(
							World, ProjectedKeeperSpawn.Location, ProjectedEnemySpawn.Location);
						if (Path == nullptr || !Path->IsValid() || Path->IsPartial())
						{
							bEverySpawnReaches = false;
						}
					}
					Obs.EnemySpawnsOffNavmesh = OffNavmeshCount;
					Obs.RouteFromSpawnReaches = bEverySpawnReaches;
				}
			}
		}

		// ---- requirements 6 and 7: the sightline sweep --------------------
		const float ReachCm = LLKUnits::DistanceFromStuds(SweepReachStuds);
		const float CoverCm = LLKUnits::DistanceFromStuds(CoverMattersWithinStuds);

		for (int32 Step = 0; Step < SweepDirections; ++Step)
		{
			const float Yaw = 360.f * static_cast<float>(Step) / static_cast<float>(SweepDirections);
			const FVector Direction = FRotator(0.f, Yaw, 0.f).Vector();

			FHitResult Hit;
			const bool bHit = World->LineTraceSingleByChannel(
				Hit, Eye, Eye + Direction * ReachCm, ECC_Visibility, Params);

			++Obs.ProbedDirections;

			const float DistanceCm = bHit ? Hit.Distance : ReachCm;
			if (bHit && Hit.Distance <= CoverCm)
			{
				++Obs.BlockedDirections;
			}

			Obs.LongestSightlineStuds = FMath::Max(
				Obs.LongestSightlineStuds, DistanceCm / LLKUnits::StudsToCm);
		}

		// ---- requirements 8 and 9: the two lanes --------------------------
		const FLLKWorldVisibility Visibility(World);

		const auto TraceLane = [&Visibility](const TArray<const AActor*>& Ends) -> TOptional<ELLKVisibility>
		{
			if (Ends.Num() != 2)
			{
				// Not traced rather than traced and failed. A lane with one
				// end is a placement mistake, not a visibility result, and
				// Evaluate says so with a different code.
				return TOptional<ELLKVisibility>();
			}
			return Visibility.VisibilityBetween(
				Ends[0]->GetActorLocation(), Ends[1]->GetActorLocation());
		};

		const TArray<const AActor*> OpenEnds = ActorsTagged(World, TestLaneOpenTag);
		const TArray<const AActor*> BlockedEnds = ActorsTagged(World, TestLaneBlockedTag);
		Obs.OpenLaneEnds = OpenEnds.Num();
		Obs.BlockedLaneEnds = BlockedEnds.Num();
		Obs.OpenLaneResult = TraceLane(OpenEnds);
		Obs.BlockedLaneResult = TraceLane(BlockedEnds);

		return Obs;
	}

	FLLKMapReport Check(UWorld* World, const FLLKStoreFloorPlan& Plan)
	{
		return Evaluate(Observe(World, Plan));
	}

	FString NameOf(ELLKMapCheck Check)
	{
		switch (Check)
		{
		case ELLKMapCheck::NoPlayerSpawn:              return TEXT("NoPlayerSpawn");
		case ELLKMapCheck::NoEnemySpawnPoints:         return TEXT("NoEnemySpawnPoints");
		case ELLKMapCheck::FloorHasNoCollision:        return TEXT("FloorHasNoCollision");
		case ELLKMapCheck::GeometryServerOnly:         return TEXT("GeometryServerOnly");
		case ELLKMapCheck::NavBoundsMissing:           return TEXT("NavBoundsMissing");
		case ELLKMapCheck::NavDataMissing:             return TEXT("NavDataMissing");
		case ELLKMapCheck::NavmeshNotBuilt:            return TEXT("NavmeshNotBuilt");
		case ELLKMapCheck::SpawnOffNavmesh:            return TEXT("SpawnOffNavmesh");
		case ELLKMapCheck::EnemySpawnOffNavmesh:       return TEXT("EnemySpawnOffNavmesh");
		case ELLKMapCheck::NavBoundsDoesNotCoverSpawn: return TEXT("NavBoundsDoesNotCoverSpawn");
		case ELLKMapCheck::NoRouteFromSpawn:           return TEXT("NoRouteFromSpawn");
		case ELLKMapCheck::RouteUnverified:            return TEXT("RouteUnverified");
		case ELLKMapCheck::NoOccludingGeometry:        return TEXT("NoOccludingGeometry");
		case ELLKMapCheck::SightlineTooLong:           return TEXT("SightlineTooLong");
		case ELLKMapCheck::TestLaneOpenMissing:        return TEXT("TestLaneOpenMissing");
		case ELLKMapCheck::TestLaneOpenBlocked:        return TEXT("TestLaneOpenBlocked");
		case ELLKMapCheck::TestLaneBlockedMissing:     return TEXT("TestLaneBlockedMissing");
		case ELLKMapCheck::TestLaneBlockedClear:       return TEXT("TestLaneBlockedClear");
		}

		// No default above, so adding a value to the enum and forgetting it
		// here is a compiler warning rather than a log line reading "Unknown".
		return TEXT("Unnamed");
	}
}

bool FLLKMapReport::Has(ELLKMapCheck Check) const
{
	return Findings.ContainsByPredicate(
		[Check](const FLLKMapFinding& Finding) { return Finding.Check == Check; });
}

FString FLLKMapReport::Describe() const
{
	if (IsReady())
	{
		return TEXT("Map readiness: ready. All of docs/MAP-READINESS.md is satisfied.");
	}

	FString Out = FString::Printf(TEXT("Map readiness: %d finding(s)."), Findings.Num());
	for (const FLLKMapFinding& Finding : Findings)
	{
		Out += FString::Printf(TEXT("\n  %s - %s"),
			*LLKMapReadiness::NameOf(Finding.Check), *Finding.Detail);
	}
	return Out;
}

// ---- the console command ------------------------------------------------
//
// In the runtime module rather than in LLKEditor, which is Codex's lane
// (docs/LANES.md), and which would also mean the check could not run in a
// packaged build - where "the navmesh did not cook" is a real failure mode
// that never shows up in PIE.
static FAutoConsoleCommandWithWorldAndArgs GLLKCheckMap(
	TEXT("LLK.CheckMap"),
	TEXT("Reports what this map is missing before an enemy is put in it. Changes nothing. See docs/MAP-READINESS.md."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
		[](const TArray<FString>& /*Args*/, UWorld* World)
		{
			const FLLKStoreFloorPlan Plan;
			const FLLKMapReport Report = LLKMapReadiness::Check(World, Plan);

			// Display rather than Warning on a ready map, Warning when there
			// is something to fix: the point of running it is to be told, and
			// a warning that always fires stops being read.
			if (Report.IsReady())
			{
				UE_LOG(LogTemp, Display, TEXT("%s"), *Report.Describe());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s"), *Report.Describe());
			}
		}));

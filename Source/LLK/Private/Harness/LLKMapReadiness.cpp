#include "Harness/LLKMapReadiness.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/IConsoleManager.h"
#include "Harness/LLKStoreFloorPlan.h"
#include "LLKUnits.h"

namespace LLKMapReadiness
{
	const FName EnemySpawnTag(TEXT("LLK.EnemySpawn"));
	const FName TestLaneOpenTag(TEXT("LLK.TestLane.Open"));
	const FName TestLaneBlockedTag(TEXT("LLK.TestLane.Blocked"));

	// 88cm capsule half-height plus the camera's 64cm. See the header.
	const float EyeHeightCm = 152.f;

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

		/**
		 * Whether this actor is of a class with this name, anywhere up its
		 * chain.
		 *
		 * **Why a string and not a type.** `ANavMeshBoundsVolume` and
		 * `ARecastNavMesh` live in the engine's `NavigationSystem` module, and
		 * including their headers means adding that module to
		 * `Source/LLK/LLK.Build.cs`, which is Codex's lane (`docs/LANES.md`).
		 * The module is loaded by default at runtime, so the actors are there
		 * to find - it is only the compile-time link that is not mine to add.
		 *
		 * Walks the superclass chain so a Blueprint subclass of a nav volume,
		 * or a project-specific `ARecastNavMesh` subclass, still counts.
		 */
		bool IsClassNamed(const AActor* Actor, const TCHAR* ClassName)
		{
			if (Actor == nullptr)
			{
				return false;
			}

			for (const UClass* Class = Actor->GetClass(); Class != nullptr; Class = Class->GetSuperClass())
			{
				if (Class->GetName() == ClassName)
				{
					return true;
				}
			}
			return false;
		}

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

		if (!Obs.RouteFromSpawnReaches.IsSet())
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

	FLLKMapObservations Observe(const UWorld* World, const FLLKStoreFloorPlan& Plan)
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
		//
		// Class names rather than types - see IsClassNamed for why that is
		// not laziness.
		TArray<FBox> NavBounds;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (IsClassNamed(*It, TEXT("NavMeshBoundsVolume")))
			{
				++Obs.NavBoundsVolumes;
				NavBounds.Add(It->GetComponentsBoundingBox(/*bNonColliding=*/true));
			}
			else if (IsClassNamed(*It, TEXT("NavigationData")))
			{
				// ARecastNavMesh derives from ANavigationData, so matching the
				// base catches the Recast one and any other nav data type
				// without this file knowing which is in use.
				++Obs.NavDataActors;
			}
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

		// HANDOFF(Codex): NavigationSystem. With "NavigationSystem" in
		// Source/LLK/LLK.Build.cs this is where
		// UNavigationSystemV1::FindPathToLocationSynchronously goes, once per
		// enemy spawn, and RouteFromSpawnReaches stops being unset. Left unset
		// deliberately rather than defaulted - Evaluate reports
		// RouteUnverified, which is the honest answer and the annoying one.

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

	FLLKMapReport Check(const UWorld* World, const FLLKStoreFloorPlan& Plan)
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
		case ELLKMapCheck::NavBoundsMissing:           return TEXT("NavBoundsMissing");
		case ELLKMapCheck::NavDataMissing:             return TEXT("NavDataMissing");
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

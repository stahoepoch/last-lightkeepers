// The judgement half of the map check, with no map. M2.5 support.
//
// `LLKMapReadiness::Evaluate` is pure so that this file can hand it maps that
// could not exist: a floor with navmesh and no collision, a lane with three
// ends, a sweep that never ran. Those are the shapes a real map is in while
// somebody is halfway through setting it up, and they are exactly the shapes a
// check has to get right, because a check is only ever read when something is
// wrong.
//
// What would go wrong, which is what is asserted:
//
//   1. **Unmeasured reported as fine.** The failure that makes a checker worse
//      than no checker. A sweep that never ran has zero blocked directions,
//      which must NOT read as "no occluding geometry", and an unqueried route
//      must NOT read as a route.
//   2. **One failure reported as five.** No `PlayerStart` means the sweep, the
//      floor trace and the coverage test were all taken from the origin.
//      Reporting their derived nonsense buries the one thing to fix.
//   3. **The same failure said twice.** "No nav bounds volume" and "no nav
//      data" are one problem on a map with no volume.
//   4. **A missing lane read as a failing lane.** A lane with one end is a
//      placement mistake; a lane with two ends and the wrong answer is a trace
//      problem. Different codes, because they are different afternoons.
//   5. **A constant trace passing the lane pair.** The whole reason there are
//      two lanes: no single answer may satisfy both.
//   6. **A check with no name.** A finding whose code prints as "Unnamed" is a
//      finding nobody can search `docs/MAP-READINESS.md` for.

#include "Harness/LLKMapReadiness.h"

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// Named rather than anonymous, for the unity-build reason written out at the
// top of LLKNoticeTest.cpp's fixture namespace.
namespace LLKMapReadinessTestFixture
{
	/**
	 * A map that satisfies every requirement, including the route.
	 *
	 * Built here rather than loaded, so each test below can break exactly one
	 * thing and the finding it expects is the only one it gets. If this helper
	 * ever stops being ready, every test in the file fails at once and says
	 * so - which is the right failure, not a confusing one.
	 */
	FLLKMapObservations ReadyMap()
	{
		FLLKMapObservations Obs;
		Obs.bHasPlayerSpawn = true;
		Obs.EnemySpawnPoints = 3;
		Obs.bFloorUnderSpawn = true;
		Obs.VisualEnvironmentReachesClients = true;
		Obs.NavBoundsVolumes = 1;
		Obs.NavDataActors = 1;
		Obs.SpawnPointsOutsideNavBounds = 0;
		Obs.NavigationBuilt = true;
		Obs.SpawnOnNavmesh = true;
		Obs.EnemySpawnsOffNavmesh = 0;
		Obs.RouteFromSpawnReaches = true;
		Obs.ProbedDirections = 32;
		Obs.BlockedDirections = 11;
		Obs.LongestSightlineStuds = 30.f;
		Obs.MaxSightlineStuds = 34.f;
		Obs.OpenLaneEnds = 2;
		Obs.BlockedLaneEnds = 2;
		Obs.OpenLaneResult = ELLKVisibility::Clear;
		Obs.BlockedLaneResult = ELLKVisibility::Blocked;
		return Obs;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKMapReadinessTest,
	"LLK.Harness.MapReadiness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKMapReadinessTest::RunTest(const FString& Parameters)
{
	using namespace LLKMapReadinessTestFixture;

	// ---- the baseline, which everything else depends on ------------------
	{
		const FLLKMapReport Report = LLKMapReadiness::Evaluate(ReadyMap());
		TestTrue(TEXT("a map that satisfies every requirement is ready"), Report.IsReady());
		TestTrue(TEXT("and says so in one line"),
			Report.Describe().Contains(TEXT("ready")));
	}

	// ---- 1. unmeasured is never fine -------------------------------------
	{
		// Default-constructed: nothing was measured, because there was no
		// world to measure. This is what Observe returns for a null world.
		const FLLKMapReport Report = LLKMapReadiness::Evaluate(FLLKMapObservations());

		TestFalse(TEXT("FAILURE MODE 1: an unmeasured map is not ready"), Report.IsReady());
		TestTrue(TEXT("and the first thing it says is that there is no spawn"),
			Report.Has(ELLKMapCheck::NoPlayerSpawn));

		// FAILURE MODE 2: the derived nonsense is not reported. A sweep that
		// never ran found zero blocked directions and a zero-stud sightline,
		// and reporting either would be reporting a measurement of nowhere.
		TestFalse(TEXT("FAILURE MODE 2: it does not also claim there is no occluding geometry"),
			Report.Has(ELLKMapCheck::NoOccludingGeometry));
		TestFalse(TEXT("nor that a sightline measured from the origin is too long"),
			Report.Has(ELLKMapCheck::SightlineTooLong));
		TestFalse(TEXT("nor that nav bounds fail to cover a spawn that does not exist"),
			Report.Has(ELLKMapCheck::NavBoundsDoesNotCoverSpawn));
	}

	// ---- 1 again: a sweep that did not run is not a floor without cover --
	{
		// **This case exists because a planted bug survived without it.** The
		// unmeasured-map case above also asserts that no occlusion finding is
		// invented, but it never reaches the geometry rules at all - no
		// PlayerStart short-circuits first. So removing the
		// `ProbedDirections > 0` guard broke nothing any test could see, and
		// the protection that actually matters was untested.
		//
		// A map in exactly this state is real: a spawn exists, and the sweep
		// found nothing to probe because the floor was streamed out or the
		// trace channel is wrong. Zero blocked directions out of zero probed
		// is not a floor without cover; it is a floor nobody looked at.
		FLLKMapObservations Obs = ReadyMap();
		Obs.ProbedDirections = 0;
		Obs.BlockedDirections = 0;
		Obs.LongestSightlineStuds = 0.f;

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);
		TestFalse(TEXT("FAILURE MODE 1: a sweep that never ran does not report missing cover"),
			Report.Has(ELLKMapCheck::NoOccludingGeometry));
		TestFalse(TEXT("nor a 0-stud sightline as a passing one, by reporting nothing about it"),
			Report.Has(ELLKMapCheck::SightlineTooLong));
	}

	// ---- 1 again: an unqueried route is a finding, not a pass ------------
	{
		FLLKMapObservations Obs = ReadyMap();
		Obs.RouteFromSpawnReaches.Reset();

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);

		TestFalse(TEXT("FAILURE MODE 1: a map nobody checked the route on is not ready"),
			Report.IsReady());
		TestTrue(TEXT("and the finding is RouteUnverified"),
			Report.Has(ELLKMapCheck::RouteUnverified));
		TestFalse(TEXT("which is NOT the same as a route that was looked for and missing"),
			Report.Has(ELLKMapCheck::NoRouteFromSpawn));
		TestEqual(TEXT("and on an otherwise perfect map it is the only finding - "
			"this is the state every clean map is in until the NavigationSystem handoff lands"),
			Report.Findings.Num(), 1);
	}

	{
		FLLKMapObservations Obs = ReadyMap();
		Obs.RouteFromSpawnReaches = false;

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);
		TestTrue(TEXT("a route that was looked for and missing says so"),
			Report.Has(ELLKMapCheck::NoRouteFromSpawn));
		TestFalse(TEXT("and is not also reported as unverified"),
			Report.Has(ELLKMapCheck::RouteUnverified));
	}

	// ---- the navigation chain, which is four answers and one finding -----
	{
		// **The case a real integration produced and no test had.** With no
		// bounds volume there is no mesh, so ProjectPointToNavigation answers
		// false and the engine's IsNavigationBuilt answers TRUE - because
		// there was nowhere to build, which it reports as built. A chain that
		// starts at NavigationBuilt therefore sails past the real failure and
		// accuses a PlayerStart that is in exactly the right place.
		FLLKMapObservations Obs = ReadyMap();
		Obs.NavBoundsVolumes = 0;
		Obs.NavDataActors = 0;
		Obs.NavigationBuilt = true;
		Obs.SpawnOnNavmesh = false;
		Obs.RouteFromSpawnReaches = false;

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);

		TestTrue(TEXT("no bounds volume is the finding"),
			Report.Has(ELLKMapCheck::NavBoundsMissing));
		TestFalse(TEXT("and the spawn is NOT accused of being off a mesh that does not exist"),
			Report.Has(ELLKMapCheck::SpawnOffNavmesh));
		TestFalse(TEXT("nor the route, for the same reason"),
			Report.Has(ELLKMapCheck::NoRouteFromSpawn));
		TestEqual(TEXT("one finding, and it is the one with a fix attached"),
			Report.Findings.Num(), 1);
	}

	// ---- requirement 15: a server-only world is never client-ready -------
	{
		FLLKMapObservations Obs = ReadyMap();
		Obs.VisualEnvironmentReachesClients = false;

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);
		TestTrue(TEXT("FAILURE MODE 15: runtime-only non-replicated visuals are named"),
			Report.Has(ELLKMapCheck::GeometryServerOnly));
		TestEqual(TEXT("and the otherwise-ready map has exactly that one finding"),
			Report.Findings.Num(), 1);
	}

	{
		FLLKMapObservations Obs = ReadyMap();
		Obs.VisualEnvironmentReachesClients.Reset();

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);
		TestFalse(TEXT("an unmeasured client-visual boundary is not invented as a failure"),
			Report.Has(ELLKMapCheck::GeometryServerOnly));
	}

	{
		// The same hole one link along: a volume, no nav data actor. Nav
		// disabled in world settings reaches here too, and IsNavigationBuilt
		// answers true for that as well.
		FLLKMapObservations Obs = ReadyMap();
		Obs.NavDataActors = 0;
		Obs.NavigationBuilt = true;
		Obs.SpawnOnNavmesh = false;

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);
		TestTrue(TEXT("missing nav data is the finding"),
			Report.Has(ELLKMapCheck::NavDataMissing));
		TestFalse(TEXT("and the spawn projection is suppressed with it"),
			Report.Has(ELLKMapCheck::SpawnOffNavmesh));
		TestEqual(TEXT("one finding"), Report.Findings.Num(), 1);
	}

	{
		// An unbuilt navmesh, with the two questions below it answered
		// falsely as well. **Both of those falses are meaningless** - you
		// cannot project onto a mesh that has not generated and you cannot
		// path across it - and a check that reported all three would send
		// Chris to move a PlayerStart that is in exactly the right place.
		FLLKMapObservations Obs = ReadyMap();
		Obs.NavigationBuilt = false;
		Obs.SpawnOnNavmesh = false;
		Obs.RouteFromSpawnReaches = false;

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);

		TestTrue(TEXT("an unbuilt navmesh is reported"),
			Report.Has(ELLKMapCheck::NavmeshNotBuilt));
		TestFalse(TEXT("and the spawn projection below it is suppressed, because it "
			"could not have meant anything"), Report.Has(ELLKMapCheck::SpawnOffNavmesh));
		TestFalse(TEXT("and so is the route"), Report.Has(ELLKMapCheck::NoRouteFromSpawn));
		TestFalse(TEXT("which is not the same as calling the route unverified"),
			Report.Has(ELLKMapCheck::RouteUnverified));
		TestEqual(TEXT("one finding, so there is one thing to go and fix"),
			Report.Findings.Num(), 1);
	}

	{
		// No nav data actor at all AND not built. NavDataMissing already says
		// it; NavmeshNotBuilt would be the same failure in different words.
		FLLKMapObservations Obs = ReadyMap();
		Obs.NavDataActors = 0;
		Obs.NavigationBuilt = false;

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);
		TestTrue(TEXT("missing nav data is reported"),
			Report.Has(ELLKMapCheck::NavDataMissing));
		TestFalse(TEXT("and is not also reported as unbuilt"),
			Report.Has(ELLKMapCheck::NavmeshNotBuilt));
	}

	{
		// The handoff's acceptance criterion 4, asserted rather than trusted:
		// a PlayerStart raised into the air reports the spawn and says nothing
		// about the route.
		FLLKMapObservations Obs = ReadyMap();
		Obs.SpawnOnNavmesh = false;
		Obs.RouteFromSpawnReaches = false;

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);
		TestTrue(TEXT("a spawn off the navmesh is reported"),
			Report.Has(ELLKMapCheck::SpawnOffNavmesh));
		TestFalse(TEXT("and nothing is said about the route, which every query "
			"from an off-mesh start would have failed"),
			Report.Has(ELLKMapCheck::NoRouteFromSpawn));
		TestEqual(TEXT("one finding again"), Report.Findings.Num(), 1);
	}

	{
		// The box test and the projection are independent, and a map can fail
		// one while passing the other. This is the case that proves keeping
		// both was not duplication: the spawn is inside the volume's bounds
		// and still not on the mesh, which is a counter or a 30cm sink.
		FLLKMapObservations Obs = ReadyMap();
		Obs.SpawnPointsOutsideNavBounds = 0;
		Obs.SpawnOnNavmesh = false;

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);
		TestTrue(TEXT("a spawn inside the bounds box can still be off the mesh"),
			Report.Has(ELLKMapCheck::SpawnOffNavmesh));
		TestFalse(TEXT("and the box test correctly says nothing"),
			Report.Has(ELLKMapCheck::NavBoundsDoesNotCoverSpawn));
	}

	{
		// The state the real build is in right now: the module is linked, the
		// contract is complete, and the three queries are not written yet. All
		// three unset, one finding, and it is the honest one.
		FLLKMapObservations Obs = ReadyMap();
		Obs.NavigationBuilt.Reset();
		Obs.SpawnOnNavmesh.Reset();
		Obs.RouteFromSpawnReaches.Reset();

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);
		TestEqual(TEXT("three unanswered nav questions produce one finding, not three"),
			Report.Findings.Num(), 1);
		TestTrue(TEXT("and it is RouteUnverified"),
			Report.Has(ELLKMapCheck::RouteUnverified));
	}

	{
		// The distinction this code was added for. An off-mesh enemy spawn
		// makes every path to it partial, so before this the finding was
		// NoRouteFromSpawn - true, and about the wrong thing. One actor 30cm
		// into the floor is not a blocked building.
		FLLKMapObservations Obs = ReadyMap();
		Obs.EnemySpawnsOffNavmesh = 2;
		Obs.FirstEnemySpawnOffNavmesh = FVector(-900.f, 420.f, 15.f);
		Obs.RouteFromSpawnReaches = false;

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);

		TestTrue(TEXT("a misplaced enemy spawn is reported as a misplaced enemy spawn"),
			Report.Has(ELLKMapCheck::EnemySpawnOffNavmesh));
		TestFalse(TEXT("and not as a blocked route, which is the true-but-misleading answer"),
			Report.Has(ELLKMapCheck::NoRouteFromSpawn));
		TestTrue(TEXT("with the count and a position, so it is findable in the level"),
			Report.Describe().Contains(TEXT("2 enemy spawn"))
			&& Report.Describe().Contains(TEXT("-900")));
		TestEqual(TEXT("one finding"), Report.Findings.Num(), 1);
	}

	{
		// All three of the spawn questions are asked and answered well, and a
		// route is still genuinely blocked. This is the case that has to
		// survive: adding the new code must not swallow a real blockage.
		FLLKMapObservations Obs = ReadyMap();
		Obs.EnemySpawnsOffNavmesh = 0;
		Obs.RouteFromSpawnReaches = false;

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);
		TestTrue(TEXT("a genuinely blocked route still reports as one"),
			Report.Has(ELLKMapCheck::NoRouteFromSpawn));
		TestFalse(TEXT("and does not accuse the spawn points"),
			Report.Has(ELLKMapCheck::EnemySpawnOffNavmesh));
	}

	{
		// Unset, not zero. Nobody projected the enemy spawns, so the honest
		// finding is about the query that did not run.
		FLLKMapObservations Obs = ReadyMap();
		Obs.EnemySpawnsOffNavmesh.Reset();
		Obs.RouteFromSpawnReaches.Reset();

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);
		TestTrue(TEXT("an unasked enemy-spawn projection is RouteUnverified, not a pass"),
			Report.Has(ELLKMapCheck::RouteUnverified));
		TestFalse(TEXT("and unset is not read as zero off-mesh spawns being a failure"),
			Report.Has(ELLKMapCheck::EnemySpawnOffNavmesh));
	}

	// ---- 3. the same failure is not said twice ---------------------------
	{
		FLLKMapObservations Obs = ReadyMap();
		Obs.NavBoundsVolumes = 0;
		Obs.NavDataActors = 0;
		Obs.SpawnPointsOutsideNavBounds = 4;

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);

		TestTrue(TEXT("no nav bounds volume is reported"),
			Report.Has(ELLKMapCheck::NavBoundsMissing));
		TestFalse(TEXT("FAILURE MODE 3: and 'no nav data' is not piled on top of it"),
			Report.Has(ELLKMapCheck::NavDataMissing));
		TestFalse(TEXT("nor is coverage, which cannot fail against volumes that do not exist"),
			Report.Has(ELLKMapCheck::NavBoundsDoesNotCoverSpawn));
	}

	{
		FLLKMapObservations Obs = ReadyMap();
		Obs.NavDataActors = 0;

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);
		TestTrue(TEXT("a volume with no built nav data is its own finding"),
			Report.Has(ELLKMapCheck::NavDataMissing));
		TestFalse(TEXT("and does not claim the volume is missing"),
			Report.Has(ELLKMapCheck::NavBoundsMissing));
	}

	{
		FLLKMapObservations Obs = ReadyMap();
		Obs.SpawnPointsOutsideNavBounds = 2;
		Obs.FirstSpawnPointOutsideNavBounds = FVector(1234.f, -56.f, 78.f);

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);
		TestTrue(TEXT("a spawn point outside the nav bounds is reported"),
			Report.Has(ELLKMapCheck::NavBoundsDoesNotCoverSpawn));
		TestTrue(TEXT("and the detail carries the position, because a count alone "
			"does not tell Chris which end of the floor to extend the volume to"),
			Report.Describe().Contains(TEXT("1234")));
	}

	// ---- the geometry rules, at their boundaries -------------------------
	{
		FLLKMapObservations Obs = ReadyMap();
		Obs.BlockedDirections = 0;

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);
		TestTrue(TEXT("a sweep that ran and hit nothing within reach has no cover"),
			Report.Has(ELLKMapCheck::NoOccludingGeometry));
	}

	{
		FLLKMapObservations Obs = ReadyMap();
		Obs.LongestSightlineStuds = 34.f;
		TestTrue(TEXT("a sightline exactly at the limit is allowed"),
			!LLKMapReadiness::Evaluate(Obs).Has(ELLKMapCheck::SightlineTooLong));

		Obs.LongestSightlineStuds = 34.1f;
		const FLLKMapReport Over = LLKMapReadiness::Evaluate(Obs);
		TestTrue(TEXT("and a tenth of a stud past it is not"),
			Over.Has(ELLKMapCheck::SightlineTooLong));
		TestTrue(TEXT("with both numbers in the detail, so the fix is obvious"),
			Over.Describe().Contains(TEXT("34.1")) && Over.Describe().Contains(TEXT("34.0")));
	}

	{
		// The limit travels with the observations rather than being a constant
		// here, so retuning FLLKStoreFloorPlan::MaxSightlineStuds moves the
		// check and does not need this file edited.
		FLLKMapObservations Obs = ReadyMap();
		Obs.MaxSightlineStuds = 20.f;
		Obs.LongestSightlineStuds = 30.f;
		TestTrue(TEXT("the plan's limit is what the sightline is judged against"),
			LLKMapReadiness::Evaluate(Obs).Has(ELLKMapCheck::SightlineTooLong));
	}

	// ---- 4. a missing lane is not a failing lane ------------------------
	{
		FLLKMapObservations Obs = ReadyMap();
		Obs.OpenLaneEnds = 1;
		Obs.OpenLaneResult.Reset();

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);
		TestTrue(TEXT("FAILURE MODE 4: a lane with one end is reported as missing"),
			Report.Has(ELLKMapCheck::TestLaneOpenMissing));
		TestFalse(TEXT("and not as a lane that reported the wrong answer"),
			Report.Has(ELLKMapCheck::TestLaneOpenBlocked));
	}

	{
		FLLKMapObservations Obs = ReadyMap();
		Obs.BlockedLaneEnds = 3;

		TestTrue(TEXT("three ends is not a lane either"),
			LLKMapReadiness::Evaluate(Obs).Has(ELLKMapCheck::TestLaneBlockedMissing));
	}

	{
		FLLKMapObservations Obs = ReadyMap();
		Obs.OpenLaneResult = ELLKVisibility::Unknown;

		TestTrue(TEXT("an open lane that could not be answered does not prove a Clear "
			"answer is possible, so it fails"),
			LLKMapReadiness::Evaluate(Obs).Has(ELLKMapCheck::TestLaneOpenBlocked));
	}

	// ---- 5. no constant answer satisfies both lanes ---------------------
	{
		FLLKMapObservations AlwaysClear = ReadyMap();
		AlwaysClear.OpenLaneResult = ELLKVisibility::Clear;
		AlwaysClear.BlockedLaneResult = ELLKVisibility::Clear;

		FLLKMapObservations AlwaysBlocked = ReadyMap();
		AlwaysBlocked.OpenLaneResult = ELLKVisibility::Blocked;
		AlwaysBlocked.BlockedLaneResult = ELLKVisibility::Blocked;

		FLLKMapObservations AlwaysUnknown = ReadyMap();
		AlwaysUnknown.OpenLaneResult = ELLKVisibility::Unknown;
		AlwaysUnknown.BlockedLaneResult = ELLKVisibility::Unknown;

		TestTrue(TEXT("FAILURE MODE 5: a trace stuck on Clear fails the blocked lane"),
			LLKMapReadiness::Evaluate(AlwaysClear).Has(ELLKMapCheck::TestLaneBlockedClear));
		TestTrue(TEXT("a trace stuck on Blocked fails the open lane"),
			LLKMapReadiness::Evaluate(AlwaysBlocked).Has(ELLKMapCheck::TestLaneOpenBlocked));
		TestFalse(TEXT("neither is ready"),
			LLKMapReadiness::Evaluate(AlwaysClear).IsReady()
			|| LLKMapReadiness::Evaluate(AlwaysBlocked).IsReady()
			|| LLKMapReadiness::Evaluate(AlwaysUnknown).IsReady());
	}

	// ---- 6. every code has a name, and they are all different -----------
	{
		TSet<FString> Names;
		for (uint8 Value = 0; Value <= static_cast<uint8>(ELLKMapCheck::TestLaneBlockedClear); ++Value)
		{
			const FString Name = LLKMapReadiness::NameOf(static_cast<ELLKMapCheck>(Value));
			TestNotEqual(TEXT("FAILURE MODE 6: every check has a name a document can be "
				"searched for"), Name, FString(TEXT("Unnamed")));
			Names.Add(Name);
		}

		TestEqual(TEXT("and no two checks share one"),
			Names.Num(), static_cast<int32>(ELLKMapCheck::TestLaneBlockedClear) + 1);
	}

	// ---- and the one thing a missing floor must not do -------------------
	{
		// A map with a built navmesh and no collision under the spawn. Not a
		// contradiction: the volume and the nav data are authored, and the
		// floor's collision can be turned off on a single static mesh. This is
		// the state that makes every trace miss and every enemy look blind
		// while the navmesh looks perfectly healthy in the viewport.
		FLLKMapObservations Obs = ReadyMap();
		Obs.bFloorUnderSpawn = false;

		const FLLKMapReport Report = LLKMapReadiness::Evaluate(Obs);
		TestTrue(TEXT("a floor with no collision is reported even when the navmesh is fine"),
			Report.Has(ELLKMapCheck::FloorHasNoCollision));
		TestEqual(TEXT("and it is the only finding, so nothing distracts from it"),
			Report.Findings.Num(), 1);
	}

	return true;
}

#endif

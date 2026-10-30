// The wall half of noticing. M2.5 support.
//
// `LLK.Enemies.Perception` tests which sense reaches. This tests what happens
// when something is in the way, and like that file it is written against the
// ways it could go wrong rather than against the way it should work:
//
//   1. **A wall that does not stop a notice.** The failure everybody expects,
//      and the only one on this list that a playtest would catch.
//   2. **`Unknown` treated as clear.** A trace that cannot answer - no world,
//      a floor with no collision, geometry not built - becoming a notice means
//      enemies hunting through walls that do not exist yet, in exactly the
//      build where somebody forgot to set the map up.
//   3. **`Unknown` quietly treated as blocked.** The opposite mistake, and the
//      worse one to debug: the whole roster goes inert and looks like a design
//      problem rather than a missing NavMeshBoundsVolume. So it is its own
//      value, it does not notice, and `IsUnverified` says which it was.
//   4. **A null source defaulting to clear.** An assume-clear path is the one
//      that ships. It must be an error, and it must notice nothing.
//   5. **A wall stopping the Warden.** Row 7 and row 54 make its arrival
//      storm-driven. If a shelf broke it, the one inescapable thing in the
//      game would become a hiding puzzle.
//   6. **A trace run when nothing reached.** Not a correctness bug but a cost
//      one: a trace per enemy per keeper per tick for the common case, which
//      is the case that runs most. The counting source below makes "was it
//      asked" an assertion rather than a hope.
//   7. **Visibility inventing a sense.** A source that always answers `Clear`
//      must not make a keeper nobody could reach noticeable.

#include "Data/LLKEnemyDef.h"
#include "Enemies/LLKPerception.h"
#include "Enemies/LLKVisibility.h"
#include "LLKUnits.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// Named, not anonymous. A unity build merges this file with the other
// perception tests, and `KeeperAt` lives in three of them; two anonymous
// namespaces in one translation unit are one namespace. Adding this file is
// what made that collide, which means it was always going to.
namespace LLKNoticeTestFixture
{
	FLLKPerceivedKeeper KeeperAt(float Studs, float Detectability)
	{
		FLLKPerceivedKeeper Keeper;
		Keeper.Position = FVector(LLKUnits::DistanceFromStuds(Studs), 0.f, 0.f);
		Keeper.Detectability = Detectability;
		return Keeper;
	}

	FLLKPerceiver FacingForward()
	{
		FLLKPerceiver Perceiver;
		Perceiver.Position = FVector::ZeroVector;
		Perceiver.Facing = FVector::ForwardVector;
		return Perceiver;
	}

	/**
	 * A fixed answer that also counts how many times it was asked.
	 *
	 * The counting is the point for failure mode 6: "the Warden does not
	 * consult geometry" and "nothing is traced when no sense reaches" are
	 * both claims about a call that should not happen, and the only way to
	 * assert a call did not happen is to count calls.
	 */
	struct FCountingVisibility final : public ILLKVisibilitySource
	{
		explicit FCountingVisibility(ELLKVisibility InAnswer)
			: Answer(InAnswer)
		{
		}

		virtual ELLKVisibility VisibilityBetween(const FVector&, const FVector&) const override
		{
			++Asked;
			return Answer;
		}

		ELLKVisibility Answer = ELLKVisibility::Unknown;
		mutable int32 Asked = 0;
	};

	FLLKEnemyDef MakeSweeper()
	{
		FLLKEnemyDef Sweeper;
		Sweeper.Family = ELLKEnemyFamily::Sweepers;
		Sweeper.AggroRadiusDark = 12.f;
		Sweeper.AggroRadiusLit = 38.f;
		return Sweeper;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKNoticeTest,
	"LLK.Enemies.Notice",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKNoticeTest::RunTest(const FString& Parameters)
{
	using namespace LLKNoticeTestFixture;

	// Detectability, 0 to 1, since row 80. See LLKPerceptionTest for why the
	// derived brightness threshold could not express a hooded lantern.
	const float Lit = 1.f;
	const float Dark = 0.f;

	const FLLKEnemyDef Sweeper = MakeSweeper();
	const FLLKPerceiver At = FacingForward();

	// A keeper a Sweeper's lit proximity definitely reaches: 20 studs, inside
	// 38 and outside 12, so light is what put them in range.
	const FLLKPerceivedKeeper InReach = KeeperAt(20.f, Lit);

	// ---- 1. a wall stops the notice, and says so ------------------------
	{
		const FCountingVisibility Wall(ELLKVisibility::Blocked);
		const FLLKNotice Notice = LLKPerception::Notice(Sweeper, At, InReach, &Wall);

		TestFalse(TEXT("FAILURE MODE 1: a blocked line does not notice"), Notice.bNoticed);
		TestEqual(TEXT("and the sense that was stopped is still reported, because "
			"'why did it NOT notice me' is the map question"),
			Notice.Sense, ELLKNoticeSense::LitProximity);
		TestEqual(TEXT("and the wall's answer is carried out"),
			Notice.Visibility, ELLKVisibility::Blocked);
		TestFalse(TEXT("a wall is a verified answer, not an unverified one"),
			Notice.IsUnverified());
		TestEqual(TEXT("the source was asked exactly once"), Wall.Asked, 1);
	}

	// ---- the same enemy, the same keeper, a clear line -------------------
	{
		const FCountingVisibility Clear(ELLKVisibility::Clear);
		const FLLKNotice Notice = LLKPerception::Notice(Sweeper, At, InReach, &Clear);

		TestTrue(TEXT("a clear line notices"), Notice.bNoticed);
		TestEqual(TEXT("by proximity"), Notice.Sense, ELLKNoticeSense::LitProximity);
		TestFalse(TEXT("and is not unverified"), Notice.IsUnverified());
	}

	// ---- 2 and 3. Unknown is neither clear nor silently blocked ----------
	{
		const FCountingVisibility CannotTell(ELLKVisibility::Unknown);
		const FLLKNotice Notice = LLKPerception::Notice(Sweeper, At, InReach, &CannotTell);

		TestFalse(TEXT("FAILURE MODE 2: an unanswerable trace does not notice"), Notice.bNoticed);
		TestEqual(TEXT("FAILURE MODE 3: and it is reported as Unknown rather than as Blocked, "
			"so a readout can say unverified instead of safe"),
			Notice.Visibility, ELLKVisibility::Unknown);
		TestTrue(TEXT("which is exactly what IsUnverified is for"), Notice.IsUnverified());
	}

	// ---- 4. no source at all is an error, not a default ------------------
	{
		AddExpectedError(TEXT("no visibility source"), EAutomationExpectedErrorFlags::Contains, 1);

		const FLLKNotice Notice = LLKPerception::Notice(Sweeper, At, InReach, nullptr);

		TestFalse(TEXT("FAILURE MODE 4: an unwired source notices nothing"), Notice.bNoticed);
		TestEqual(TEXT("and does not pretend the line was clear"),
			Notice.Visibility, ELLKVisibility::Unknown);
		TestTrue(TEXT("and reads as unverified, which is what it is"), Notice.IsUnverified());
	}

	// ---- 5. walls do not stop the Warden --------------------------------
	{
		FLLKEnemyDef Warden;
		Warden.Family = ELLKEnemyFamily::Warden;
		// No radii, no cone, no scan, no wake. Row 7 and row 54: its arrival
		// is a meter, not a sense.

		const FCountingVisibility Wall(ELLKVisibility::Blocked);
		const FLLKNotice Notice = LLKPerception::Notice(
			Warden, At, KeeperAt(200.f, Dark), &Wall);

		TestTrue(TEXT("FAILURE MODE 5: the Warden notices through a wall, from 200 studs, in the dark"),
			Notice.bNoticed);
		TestEqual(TEXT("as storm authority rather than as a sense"),
			Notice.Sense, ELLKNoticeSense::StormAuthority);
		TestEqual(TEXT("the source was never asked"), Wall.Asked, 0);
		TestEqual(TEXT("so visibility is Unknown, which is the truth - nobody asked"),
			Notice.Visibility, ELLKVisibility::Unknown);
		TestFalse(TEXT("and that Unknown is NOT a map problem, so IsUnverified excludes it"),
			Notice.IsUnverified());
	}

	// ---- 6. nothing reached, so nothing was traced -----------------------
	{
		const FCountingVisibility Clear(ELLKVisibility::Clear);

		// The same keeper as the clear-line case above, gone dark: 20 studs is
		// outside a Sweeper's 12-stud dark radius, so no sense reaches.
		const FLLKNotice Notice = LLKPerception::Notice(
			Sweeper, At, KeeperAt(20.f, Dark), &Clear);

		TestEqual(TEXT("no sense reaches a dark keeper at 20 studs"),
			Notice.Sense, ELLKNoticeSense::None);
		TestFalse(TEXT("so nothing is noticed"), Notice.bNoticed);
		TestEqual(TEXT("FAILURE MODE 6: and no trace was run - there was nothing to occlude"),
			Clear.Asked, 0);
		TestFalse(TEXT("and None with no trace is not an unverified state"),
			Notice.IsUnverified());
	}

	// ---- 7. a clear line cannot invent a sense --------------------------
	{
		const FCountingVisibility AlwaysClear(ELLKVisibility::Clear);

		// 100 studs, lit, far outside every radius a Sweeper has.
		const FLLKNotice Notice = LLKPerception::Notice(
			Sweeper, At, KeeperAt(100.f, Lit), &AlwaysClear);

		TestFalse(TEXT("FAILURE MODE 7: a source that always answers Clear notices nobody "
			"it could not already reach"), Notice.bNoticed);
		TestEqual(TEXT("because the sense is computed first"),
			Notice.Sense, ELLKNoticeSense::None);
	}

	// ---- and the fixed source the rest of the project may use -----------
	{
		const FLLKFixedVisibility Blocked(ELLKVisibility::Blocked);
		TestEqual(TEXT("FLLKFixedVisibility answers what it was built with"),
			Blocked.VisibilityBetween(FVector::ZeroVector, FVector(1000.f, 0.f, 0.f)),
			ELLKVisibility::Blocked);

		// A world-backed source with no world. The real one, in the state the
		// tests and a commandlet see it in, and it must not answer Clear.
		const FLLKWorldVisibility NoWorld(nullptr);
		TestEqual(TEXT("and a world-less FLLKWorldVisibility answers Unknown, never Clear"),
			NoWorld.VisibilityBetween(FVector::ZeroVector, FVector(1000.f, 0.f, 0.f)),
			ELLKVisibility::Unknown);
	}

	return true;
}

#endif

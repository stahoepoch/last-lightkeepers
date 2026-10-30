// The extraction state machine, asserted as rules. M3.
//
// Row 74 settled who comes for you; this settles what the game does about it.
// The whole state machine is a pure function, so every rule below is checked
// with no world, no dock and no actor - which is the only reason it can be
// checked at all today, because the dock does not exist yet.
//
// **What is worth a test here**, in the order the rules matter:
//
//   1. **The call is refused for one named reason at a time.** Six refusals,
//      each reachable, each distinguishable. `ELLKMapCheck`'s discipline: a
//      refusal that only says no sends somebody reading code.
//   2. **The hold is a hold.** It cannot start during the siren, it cannot
//      advance with nobody on the route, and it cannot advance when nobody
//      measured the route - which is today's normal case and must never
//      secure a haul on its own.
//   3. **The hold pauses and never rewinds.** The tuning decision the header
//      flags. If Chris inverts it, this is the test that fails by name.
//   4. **Terminal is terminal.** Secured cannot be re-called, re-held or
//      un-secured by a later wipe; Failed cannot become Secured.
//   5. **The clock is not reachable from here.** Asserted as a signature
//      claim, the same way the lantern feel's third guarantee is: there is no
//      argument and no return value through which a pressure value could pass.
//   6. **Frame rate does not change the deal.** One 28-second step and 1680
//      sixtieths both secure a 20-second hold after an 8-second siren, which
//      is what carrying the leftover delta across the phase boundary buys.

#include "Run/LLKExtraction.h"

#include "Misc/AutomationTest.h"

#include <limits>

#if WITH_DEV_AUTOMATION_TESTS

namespace LLKExtractionTestFixture
{
	/** `loot.json`'s dock, so the test moves when Chris retunes it. */
	FLLKExtractionPointRules Dock()
	{
		FLLKExtractionPointRules Point;
		Point.Id = TEXT("LoadingDock");
		Point.HoldSeconds = 20.f;
		Point.PayoutModifier = 1.f;
		return Point;
	}

	/** The lift, which is the gated one. */
	FLLKExtractionPointRules Lift()
	{
		FLLKExtractionPointRules Point;
		Point.Id = TEXT("FreightLift");
		Point.HoldSeconds = 30.f;
		Point.PayoutModifier = 1.6f;
		return Point;
	}

	/** `storm-charge.json`'s `extractionCall`, minus the field row 73 retired. */
	FLLKExtractionAnnounce Announce()
	{
		FLLKExtractionAnnounce Call;
		Call.SirenSeconds = 8.f;
		Call.bConvergeAllPatrols = true;
		return Call;
	}

	FLLKExtractionInput Step(const float DeltaSeconds, const int32 KeepersOnRoute)
	{
		FLLKExtractionInput Input;
		Input.DeltaSeconds = DeltaSeconds;
		Input.KeepersOnBoardingRoute = KeepersOnRoute;
		return Input;
	}

	/** Nobody measured the route. The greybox's state, and it is not zero. */
	FLLKExtractionInput Unmeasured(const float DeltaSeconds)
	{
		FLLKExtractionInput Input;
		Input.DeltaSeconds = DeltaSeconds;
		return Input;
	}

	/** Calls at the dock and runs the siren out, so a test can start at the hold. */
	FLLKExtractionState Holding()
	{
		ELLKExtractionRefusal Refusal = ELLKExtractionRefusal::None;
		const FLLKExtractionState Called = LLKExtraction::Call(
			FLLKExtractionState(), Dock(), Announce(), true, false, Refusal);
		return LLKExtraction::Advance(Called, Step(8.f, 1)).State;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKExtractionTest,
	"LLK.Run.Extraction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKExtractionTest::RunTest(const FString& Parameters)
{
	using namespace LLKExtractionTestFixture;

	// ---- 1. the call, and its six refusals ------------------------------
	{
		ELLKExtractionRefusal Refusal = ELLKExtractionRefusal::AlreadyUnderway;
		const FLLKExtractionState Called = LLKExtraction::Call(
			FLLKExtractionState(), Dock(), Announce(), true, false, Refusal);

		TestEqual(TEXT("a legal call is not refused"),
			FString(LLKExtraction::NameOf(Refusal)), FString(TEXT("None")));
		TestEqual(TEXT("and it announces rather than holding"),
			FString(LLKExtraction::NameOf(Called.Phase)), FString(TEXT("Announcing")));
		TestEqual(TEXT("the siren is the authored eight seconds"),
			Called.SirenSecondsRemaining, 8.f);
		TestEqual(TEXT("the hold is captured at the call"), Called.HoldSecondsRequired, 20.f);
		TestEqual(TEXT("the exit is named"), Called.PointId.ToString(), FString(TEXT("LoadingDock")));
		TestEqual(TEXT("and the call is counted"), Called.CallCount, 1);

		// Nothing can be held yet. The rig is still on the road.
		TestEqual(TEXT("the hold does not start during the siren"),
			LLKExtraction::HoldFraction(Called), 0.f);

		// AlreadyUnderway.
		LLKExtraction::Call(Called, Dock(), Announce(), true, false, Refusal);
		TestEqual(TEXT("calling twice is refused as AlreadyUnderway"),
			FString(LLKExtraction::NameOf(Refusal)), FString(TEXT("AlreadyUnderway")));

		// NoSuchPoint.
		FLLKExtractionPointRules Nameless = Dock();
		Nameless.Id = NAME_None;
		LLKExtraction::Call(FLLKExtractionState(), Nameless, Announce(), true, false, Refusal);
		TestEqual(TEXT("an unnamed exit is refused as NoSuchPoint"),
			FString(LLKExtraction::NameOf(Refusal)), FString(TEXT("NoSuchPoint")));

		// NoHoldAuthored - `loot.json`'s "an exit that costs nothing is not a
		// decision", enforced rather than commented.
		FLLKExtractionPointRules Free = Dock();
		Free.HoldSeconds = 0.f;
		LLKExtraction::Call(FLLKExtractionState(), Free, Announce(), true, false, Refusal);
		TestEqual(TEXT("an exit with no hold is refused as NoHoldAuthored"),
			FString(LLKExtraction::NameOf(Refusal)), FString(TEXT("NoHoldAuthored")));

		Free.HoldSeconds = std::numeric_limits<float>::quiet_NaN();
		LLKExtraction::Call(FLLKExtractionState(), Free, Announce(), true, false, Refusal);
		TestEqual(TEXT("and so is a hold that is not a number"),
			FString(LLKExtraction::NameOf(Refusal)), FString(TEXT("NoHoldAuthored")));

		// RequirementNotMet - the lift without the breaker.
		LLKExtraction::Call(FLLKExtractionState(), Lift(), Announce(), false, false, Refusal);
		TestEqual(TEXT("the lift without its breaker is refused as RequirementNotMet"),
			FString(LLKExtraction::NameOf(Refusal)), FString(TEXT("RequirementNotMet")));

		// RunIsOver outranks everything, including a missing requirement:
		// telling a wiped crew that it needs a breaker would be true and
		// useless.
		LLKExtraction::Call(FLLKExtractionState(), Lift(), Announce(), false, true, Refusal);
		TestEqual(TEXT("a wiped crew is refused as RunIsOver, not RequirementNotMet"),
			FString(LLKExtraction::NameOf(Refusal)), FString(TEXT("RunIsOver")));
	}

	// ---- 2. the siren, then the hold -------------------------------------
	{
		ELLKExtractionRefusal Refusal = ELLKExtractionRefusal::None;
		FLLKExtractionState State = LLKExtraction::Call(
			FLLKExtractionState(), Dock(), Announce(), true, false, Refusal);

		// Seven seconds of siren: still announcing, hold untouched.
		FLLKExtractionStep Stepped = LLKExtraction::Advance(State, Step(7.f, 1));
		State = Stepped.State;
		TestEqual(TEXT("seven seconds in, the siren is still running"),
			FString(LLKExtraction::NameOf(State.Phase)), FString(TEXT("Announcing")));
		TestFalse(TEXT("and the rig has not arrived"), Stepped.bRigArrived);
		TestEqual(TEXT("the hold is still whole"), State.HoldSecondsRemaining, 20.f);

		// The eighth second ends the siren, and the leftover second starts the
		// hold rather than being dropped at the boundary.
		Stepped = LLKExtraction::Advance(State, Step(2.f, 1));
		State = Stepped.State;
		TestTrue(TEXT("the rig arrives when the siren runs out"), Stepped.bRigArrived);
		TestEqual(TEXT("and the hold begins"),
			FString(LLKExtraction::NameOf(State.Phase)), FString(TEXT("Holding")));
		TestNearlyEqual(TEXT("the leftover second is spent on the hold, not lost"),
			State.HoldSecondsRemaining, 19.f, 0.001f);
	}

	// ---- 3. the hold needs somebody standing on the route ----------------
	{
		FLLKExtractionState State = Holding();
		const float Before = State.HoldSecondsRemaining;

		// Nobody measured: the greybox. This must not advance, and it must
		// say so, because an extraction that silently never completes is
		// indistinguishable from a broken state machine.
		FLLKExtractionStep Stepped = LLKExtraction::Advance(State, Unmeasured(5.f));
		TestEqual(TEXT("an unmeasured route does not advance the hold"),
			Stepped.State.HoldSecondsRemaining, Before);
		TestTrue(TEXT("and reports the stall"), Stepped.bHoldStalled);
		TestFalse(TEXT("and certainly does not secure a haul"), Stepped.bSecured);

		// Measured and empty: the crew left the bay.
		Stepped = LLKExtraction::Advance(State, Step(5.f, 0));
		TestEqual(TEXT("an empty route does not advance the hold either"),
			Stepped.State.HoldSecondsRemaining, Before);
		TestTrue(TEXT("and reports the stall"), Stepped.bHoldStalled);

		// Somebody there: it moves.
		Stepped = LLKExtraction::Advance(State, Step(5.f, 1));
		TestNearlyEqual(TEXT("one keeper on the route advances the hold"),
			Stepped.State.HoldSecondsRemaining, Before - 5.f, 0.001f);
		TestFalse(TEXT("and that is not a stall"), Stepped.bHoldStalled);

		// Four keepers are not four times faster. The hold is the rig loading
		// and sealing, not work the crew does.
		const FLLKExtractionStep Crowd = LLKExtraction::Advance(State, Step(5.f, 4));
		TestEqual(TEXT("a whole crew on the route is no faster than one keeper"),
			Crowd.State.HoldSecondsRemaining, Stepped.State.HoldSecondsRemaining);
	}

	// ---- 4. the hold pauses and never rewinds ----------------------------
	//
	// The tuning decision, named so it fails by name if Chris inverts it.
	{
		FLLKExtractionState State = Holding();
		State = LLKExtraction::Advance(State, Step(15.f, 1)).State;
		TestNearlyEqual(TEXT("fifteen of twenty seconds held"),
			State.HoldSecondsRemaining, 5.f, 0.001f);

		// Driven off the route for ten seconds, which is longer than the hold
		// has left: progress survives it entirely.
		for (int32 Second = 0; Second < 10; ++Second)
		{
			State = LLKExtraction::Advance(State, Step(1.f, 0)).State;
		}
		TestNearlyEqual(TEXT("leaving the route costs time, never progress"),
			State.HoldSecondsRemaining, 5.f, 0.001f);
		TestEqual(TEXT("and it is still the same extraction"),
			FString(LLKExtraction::NameOf(State.Phase)), FString(TEXT("Holding")));

		// Back on, and the last five seconds finish it.
		const FLLKExtractionStep Finish = LLKExtraction::Advance(State, Step(5.f, 1));
		TestTrue(TEXT("returning to the route finishes the hold"), Finish.bSecured);
		TestEqual(TEXT("and the phase is Secured"),
			FString(LLKExtraction::NameOf(Finish.State.Phase)), FString(TEXT("Secured")));
		TestEqual(TEXT("the payout survives the whole thing"), Finish.State.PayoutModifier, 1.f);
		TestEqual(TEXT("and the readout reads one"),
			LLKExtraction::HoldFraction(Finish.State), 1.f);
	}

	// ---- 5. cancellation buys back the route and not the noise -----------
	{
		ELLKExtractionRefusal Refusal = ELLKExtractionRefusal::None;
		const FLLKExtractionState Called = LLKExtraction::Call(
			FLLKExtractionState(), Dock(), Announce(), true, false, Refusal);

		bool bCancelled = false;
		FLLKExtractionState State = LLKExtraction::Cancel(Called, bCancelled);
		TestTrue(TEXT("an announcing extraction can be called off"), bCancelled);
		TestEqual(TEXT("and it returns to Idle"),
			FString(LLKExtraction::NameOf(State.Phase)), FString(TEXT("Idle")));
		TestTrue(TEXT("with no exit"), State.PointId.IsNone());
		TestEqual(TEXT("the call is still on the record"), State.CallCount, 1);
		TestEqual(TEXT("and so is the cancel"), State.CancelCount, 1);

		// Mid-hold, with nineteen of twenty seconds done: cancelling loses all
		// of it. That is the decision, and it is why cancelling late is a real
		// choice rather than a free one.
		FLLKExtractionState Deep = Holding();
		Deep = LLKExtraction::Advance(Deep, Step(19.f, 1)).State;
		Deep = LLKExtraction::Cancel(Deep, bCancelled);
		TestTrue(TEXT("a hold can be called off too"), bCancelled);
		TestEqual(TEXT("and the hold is gone, not banked"), Deep.HoldSecondsRemaining, 0.f);

		// Calling again after a cancel is legal and counted, because each
		// siren is a fresh event in a building that listens.
		const FLLKExtractionState Again = LLKExtraction::Call(
			Deep, Dock(), Announce(), true, false, Refusal);
		TestEqual(TEXT("calling again after a cancel is allowed"),
			FString(LLKExtraction::NameOf(Refusal)), FString(TEXT("None")));
		TestEqual(TEXT("and it is the second call this run"), Again.CallCount, 2);
		TestEqual(TEXT("with the whole hold to do over"), Again.HoldSecondsRemaining, 20.f);

		// Idle has nothing to cancel, and a secured run cannot be cancelled.
		LLKExtraction::Cancel(FLLKExtractionState(), bCancelled);
		TestFalse(TEXT("there is nothing to cancel when nothing was called"), bCancelled);
	}

	// ---- 6. the wipe, and terminal meaning terminal ----------------------
	{
		// From the hold.
		FLLKExtractionInput Wipe;
		Wipe.DeltaSeconds = 0.1f;
		Wipe.KeepersOnBoardingRoute = 1;
		Wipe.bCrewWiped = true;

		const FLLKExtractionStep Lost = LLKExtraction::Advance(Holding(), Wipe);
		TestTrue(TEXT("a wipe mid-hold fails the extraction"), Lost.bFailed);
		TestEqual(TEXT("and the phase is Failed"),
			FString(LLKExtraction::NameOf(Lost.State.Phase)), FString(TEXT("Failed")));

		// From Idle, which reads oddly until said out loud: the run ended and
		// no extraction happened, which is what Failed means.
		const FLLKExtractionStep NeverCalled = LLKExtraction::Advance(FLLKExtractionState(), Wipe);
		TestTrue(TEXT("a wipe with nothing called is also a failed extraction"),
			NeverCalled.bFailed);

		// Failed is terminal in both directions.
		const FLLKExtractionStep AfterFail = LLKExtraction::Advance(Lost.State, Step(60.f, 4));
		TestEqual(TEXT("a failed extraction cannot be held back to life"),
			FString(LLKExtraction::NameOf(AfterFail.State.Phase)), FString(TEXT("Failed")));
		TestFalse(TEXT("and cannot secure"), AfterFail.bSecured);

		ELLKExtractionRefusal Refusal = ELLKExtractionRefusal::None;
		LLKExtraction::Call(Lost.State, Dock(), Announce(), true, false, Refusal);
		TestEqual(TEXT("and cannot be called again"),
			FString(LLKExtraction::NameOf(Refusal)), FString(TEXT("RunIsOver")));

		// Secured is terminal too, and a wipe afterwards does not take the
		// haul back: the crew is out, whatever happens to the building.
		FLLKExtractionState Out = Holding();
		Out = LLKExtraction::Advance(Out, Step(20.f, 1)).State;
		TestEqual(TEXT("twenty seconds held secures it"),
			FString(LLKExtraction::NameOf(Out.Phase)), FString(TEXT("Secured")));

		const FLLKExtractionStep AfterSecure = LLKExtraction::Advance(Out, Wipe);
		TestEqual(TEXT("a wipe after extraction does not un-extract anybody"),
			FString(LLKExtraction::NameOf(AfterSecure.State.Phase)), FString(TEXT("Secured")));
		TestFalse(TEXT("and does not fail"), AfterSecure.bFailed);

		LLKExtraction::Call(Out, Dock(), Announce(), true, false, Refusal);
		TestEqual(TEXT("and nobody extracts twice"),
			FString(LLKExtraction::NameOf(Refusal)), FString(TEXT("AlreadyExtracted")));
	}

	// ---- 7. hitches, pauses and garbage deltas ---------------------------
	{
		const FLLKExtractionState State = Holding();

		for (const float Delta : {0.f, -5.f, std::numeric_limits<float>::quiet_NaN(),
				std::numeric_limits<float>::infinity()})
		{
			const FLLKExtractionStep Stepped = LLKExtraction::Advance(State, Step(Delta, 1));
			TestEqual(TEXT("a zero, negative or non-finite step never moves the hold"),
				Stepped.State.HoldSecondsRemaining, State.HoldSecondsRemaining);
			TestFalse(TEXT("and never secures a haul"), Stepped.bSecured);
			TestEqual(TEXT("and leaves the phase alone"),
			FString(LLKExtraction::NameOf(Stepped.State.Phase)), FString(TEXT("Holding")));
		}

		// A real hitch - a two-second frame - is progress, not a rewind.
		const FLLKExtractionStep Hitch = LLKExtraction::Advance(State, Step(2.f, 1));
		TestNearlyEqual(TEXT("a long frame advances by its own length"),
			Hitch.State.HoldSecondsRemaining, State.HoldSecondsRemaining - 2.f, 0.001f);
	}

	// ---- 8. frame rate cannot change the deal ----------------------------
	//
	// Twenty-eight seconds is eight of siren plus twenty of hold. One step or
	// 1680 of them must both end in exactly the same place, which is what
	// carrying the leftover delta across the phase boundary is for.
	{
		ELLKExtractionRefusal Refusal = ELLKExtractionRefusal::None;
		const FLLKExtractionState Called = LLKExtraction::Call(
			FLLKExtractionState(), Dock(), Announce(), true, false, Refusal);

		const FLLKExtractionStep OneStep = LLKExtraction::Advance(Called, Step(28.f, 1));
		TestTrue(TEXT("twenty-eight seconds in one step extracts"), OneStep.bSecured);

		FLLKExtractionState Many = Called;
		int32 Frames = 0;
		while (Many.Phase != ELLKExtractionPhase::Secured && Frames < 10000)
		{
			Many = LLKExtraction::Advance(Many, Step(1.f / 60.f, 1)).State;
			++Frames;
		}
		TestEqual(TEXT("and so does the same time at sixty frames a second"),
			FString(LLKExtraction::NameOf(Many.Phase)), FString(TEXT("Secured")));
		TestTrue(TEXT("in the frames twenty-eight seconds actually is"),
			Frames >= 1679 && Frames <= 1681);
	}

	// ---- 9. the names are there, because a log reads them ----------------
	{
		TestEqual(TEXT("phases are named"),
			FString(LLKExtraction::NameOf(ELLKExtractionPhase::Holding)), FString(TEXT("Holding")));
		TestEqual(TEXT("and so are refusals"),
			FString(LLKExtraction::NameOf(ELLKExtractionRefusal::RequirementNotMet)),
			FString(TEXT("RequirementNotMet")));
		TestNotEqual(TEXT("and an unhandled value is not silently blank"),
			FString(LLKExtraction::NameOf(static_cast<ELLKExtractionPhase>(200))), FString());
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

// The storm charge rules, asserted in-engine. M1.8.
//
// `tests/balance.test.mjs` and `tests/pacing.mjs` already assert this
// arithmetic against `data/storm-charge.json` in Node. That is not the same
// thing as asserting it here, and the difference is the whole reason this
// file exists: the Node suite proves the DATA is coherent, and this proves
// the ENGINE computes the same curve from it. A C++ transcription that drifts
// from the JSON by one term would pass every existing check in the repo.
//
// Three things are worth the file:
//
//   1. **The anchor.** One lantern is exactly 2.5x. That number is locked by
//      the handoff and survived both curve changes; every minute target in
//      `storm-charge.json` is written against it.
//   2. **No keeper's light is free** (row 12). This is why Chris switched to
//      the diminishing curve on 2026-10-27: under `linearCapped` a fourth
//      lantern cost nothing, so a four-player run was always the shortest.
//      The test asserts the property, not the four numbers - a property
//      survives a retune, a number does not.
//   3. **The meter only ever rises** (row 6), including against a negative
//      spike, which is the one way a reduction could get in by accident.
//
// It needs no world: `Advance` takes the lit count as a parameter and never
// looks at `GetWorld()`, which is exactly what makes the real accrual path
// testable rather than a copy of it.

#include "Data/LLKStormChargeData.h"
#include "LLKStormTestListener.h"
#include "Misc/AutomationTest.h"
#include "Storm/LLKStormChargeSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKStormFlatRateTest,
	"LLK.Storm.FlatRate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKStormChargeAccrualTest,
	"LLK.Storm.Accrual",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKStormFlatRateTest::RunTest(const FString& Parameters)
{
	// **This test used to assert the lantern curve and now asserts its
	// absence.** Row 72, 2026-10-29: *"Light never changes that clock's
	// rate."* What was here was good work on a rule we retired - the 2.5x
	// one-lantern anchor, the diminishing series, the 4.75 asymptote - and
	// keeping any of it would have meant a green suite guarding a model the
	// design had abandoned. That is worse than no test, because it reads as
	// evidence.
	//
	// So the assertions below are the new rule stated as the old one's
	// negative: no count of lanterns, and no arrangement of them, changes how
	// fast the night arrives.
	ULLKStormChargeSubsystem* Clock = NewObject<ULLKStormChargeSubsystem>(GetTransientPackage());
	ULLKStormChargeData* Rules = ULLKStormChargeSubsystem::MakeFallbackRules(Clock);
	Rules->BaselineRatePerSecond = 1.f;
	Rules->Warden.ChargeValue = 700.f;
	Clock->SetRulesForTest(Rules);

	// 1. The rate is the baseline, and it is the baseline whatever the crew
	//    is doing. There is no longer an accessor that could say otherwise -
	//    GetCurrentMultiplier and GetCurrentWardenMultiplier are gone, so a
	//    reintroduced multiplier would have to be a new API and would show up
	//    in review rather than hiding in an existing call.
	TestEqual(TEXT("the rate is the authored baseline"),
		Clock->GetCurrentRate(), Rules->BaselineRatePerSecond);

	// 2. Sixty seconds is sixty units, and it is sixty units in the dark and
	//    sixty units with the whole crew lit. Run as two independent clocks
	//    rather than as one clock twice, so a sticky multiplier cannot be
	//    hidden by order of operations.
	ULLKStormChargeSubsystem* Dark = NewObject<ULLKStormChargeSubsystem>(GetTransientPackage());
	Dark->SetRulesForTest(Rules);
	ULLKStormChargeSubsystem* Lit = NewObject<ULLKStormChargeSubsystem>(GetTransientPackage());
	Lit->SetRulesForTest(Rules);

	for (int32 Second = 0; Second < 60; ++Second)
	{
		Dark->AdvanceForTest(1.f);
		Lit->AdvanceForTest(1.f);
	}

	TestNearlyEqual(TEXT("sixty seconds is sixty units"), Dark->GetCharge(), 60.f, 0.01f);
	TestNearlyEqual(TEXT("ROW 72: and the same sixty whatever the lanterns were doing"),
		Lit->GetCharge(), Dark->GetCharge(), 0.001f);

	// 3. The Warden arrives on the one clock, at a time arithmetic can state.
	//    700 at one per second is 700 seconds, from the clock rather than from
	//    a second meter that used to run beside it.
	TestNearlyEqual(TEXT("the Warden's arrival is a threshold on the one clock"),
		Dark->SecondsUntilWarden(), 700.f - 60.f, 0.01f);

	// 4. Row 6 is untouched: it still only rises.
	const float Before = Dark->GetCharge();
	Dark->AdvanceForTest(1.f);
	TestTrue(TEXT("ROW 6: the clock still only rises"), Dark->GetCharge() > Before);

	// 5. And the one thing that may still move it discretely. The extraction
	//    call is not light, so row 72 does not forbid it - see the open
	//    question recorded in docs/THE-LANTERN.md about whether it should.
	const float BeforeSpike = Dark->GetCharge();
	Dark->AddSpike(ELLKStormSpike::ExtractionCall);
	TestTrue(TEXT("the extraction call still moves the clock"),
		Dark->GetCharge() > BeforeSpike);

	return true;
}


bool FLLKStormChargeAccrualTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK storm charge accrual test reached RunTest."));

	ULLKStormChargeSubsystem* Storm = NewObject<ULLKStormChargeSubsystem>(GetTransientPackage());
	ULLKStormChargeData* Rules = ULLKStormChargeSubsystem::MakeFallbackRules(Storm);
	Storm->SetRulesForTest(Rules);

	// 1. It starts at zero and nothing has happened yet.
	TestEqual(TEXT("the meter starts at zero"), Storm->GetCharge(), 0.f);
	TestFalse(TEXT("the Warden has not converged"), Storm->HasWardenConverged());

	// 2. A dark crew: one charge unit is one second of darkness, which is the
	//    sentence `storm-charge.json` opens with.
	Storm->AdvanceForTest(10.f);
	TestNearlyEqual(TEXT("ten seconds is ten charge"), Storm->GetCharge(), 10.f, 0.001f);

	// 3. ROW 72: ten more seconds is ten more charge, and there is no third
	//    number. This section used to assert the 2.5x lit anchor and the
	//    Warden's 1.45x dampening - the same light at two different costs -
	//    and both are retired. What replaces them is the absence: the only
	//    thing that advances this clock is time.
	const float ChargeBefore = Storm->GetCharge();
	Storm->AdvanceForTest(10.f);
	TestNearlyEqual(TEXT("ROW 72: ten more seconds is ten more charge, not twenty-five"),
		Storm->GetCharge() - ChargeBefore, 10.f, 0.001f);

	// 4. ROW 6, the one that matters most. No step of any size may lower it.
	float Highest = Storm->GetCharge();
	for (int32 Step = 0; Step < 40; ++Step)
	{
		Storm->AdvanceForTest(1.f);
		TestTrue(TEXT("the meter never falls"), Storm->GetCharge() >= Highest);
		Highest = Storm->GetCharge();
	}

	// 5. A zero or negative delta is not a rewind. A paused or hitching frame
	//    must not run the night backwards.
	Storm->AdvanceForTest(0.f);
	TestEqual(TEXT("a zero-second step changes nothing"), Storm->GetCharge(), Highest);
	Storm->AdvanceForTest(-5.f);
	TestEqual(TEXT("a negative step changes nothing"), Storm->GetCharge(), Highest);

	// 6. A negative spike is refused outright rather than applied. This is
	//    the one route by which something could subtract from the meter -
	//    an extraction-call tuning someone later sets negative - and row 6
	//    is enforced here rather than trusted.
	Storm->AddSpikeAmountForTest(-500.f, ELLKStormSpike::ExtractionCall);
	TestEqual(TEXT("a negative spike is refused"), Storm->GetCharge(), Highest);

	// 7. A real spike rises. The extraction call's 180 is
	//    three minutes of darkness applied at once, deliberately large
	//    against thresholds of 270 and 480: calling it should move you a
	//    stage, so calling early and calling late are different decisions.
	// The amount now comes from the data rather than the caller, so this also
	// asserts the lookup instead of only the accrual.
	TestTrue(TEXT("the extraction call has an authored price"),
		Storm->GetSpikeAmount(ELLKStormSpike::ExtractionCall) > 0.f);
	Storm->AddSpike(ELLKStormSpike::ExtractionCall);
	TestTrue(TEXT("a real spike raises the meter"), Storm->GetCharge() > Highest);

	// 8. Thresholds fire once each, in order, and the Warden is not one of
	//    them (row 54). Drive the meter well past both thresholds and the
	//    Warden's value, then count.
	// Both delegates are dynamic and BlueprintAssignable - the things that
	// react to a threshold are presentation and spawning, some of it Chris's -
	// so they bind to a UFUNCTION rather than a lambda.
	// Drive past PatrolsAlert BEFORE the listener exists. This used to happen
	// by accident: the spike plus the lit-crew accrual above put the meter
	// past 270 on its own. At the flat rate it does not, so the ordering this
	// assertion depends on is now explicit rather than incidental.
	while (Storm->GetCharge() < 300.f)
	{
		Storm->AdvanceForTest(1.f);
	}

	ULLKStormTestListener* Listener = NewObject<ULLKStormTestListener>(GetTransientPackage());
	Storm->OnThresholdCrossed.AddDynamic(Listener, &ULLKStormTestListener::OnThresholdCrossed);
	Storm->OnWardenConverged.AddDynamic(Listener, &ULLKStormTestListener::OnWardenConverged);

	// One clock, so there is one honest path to all three events.
	for (int32 Second = 0; Second < 1200; ++Second)
	{
		Storm->AdvanceForTest(1.f);
	}

	// PatrolsAlert was already crossed before the lambda was attached - the
	// spike above put the meter past 270 - so it must NOT fire now. That is
	// the assertion: a crossing is announced once, at the crossing, and never
	// replayed to a listener that arrives late.
	TestEqual(TEXT("PatrolsAlert is not re-announced to a late listener"),
		Listener->CountFor(TEXT("PatrolsAlert")), 0);
	TestEqual(TEXT("PatrolsHunt fires exactly once"),
		Listener->CountFor(TEXT("PatrolsHunt")), 1);
	TestEqual(TEXT("the Warden converges exactly once"), Listener->WardenCount, 1);
	TestTrue(TEXT("and it has converged"), Storm->HasWardenConverged());

	// 9. Two thresholds, not three. Row 54 split the Warden out; anything
	//    reading the last threshold expecting the Warden reads the wrong one.
	TestEqual(TEXT("there are two thresholds and the Warden is not among them"),
		Rules->Thresholds.Num(), 2);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

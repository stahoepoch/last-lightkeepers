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
	FLLKStormChargeCurveTest,
	"LLK.Storm.LanternCurve",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKStormChargeAccrualTest,
	"LLK.Storm.Accrual",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKStormChargeCurveTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK storm charge curve test reached RunTest."));

	const ULLKStormChargeData* Rules = ULLKStormChargeSubsystem::MakeFallbackRules(GetTransientPackage());
	const FLLKLanternChargeRules& Lantern = Rules->Lantern;

	// The fallbacks must be the diminishing curve, or every assertion below
	// is testing the retired one by accident.
	TestEqual(TEXT("the declared fallback uses the diminishing curve"),
		Lantern.Curve, ELLKLanternCurve::Diminishing);

	// 1. A dark crew pays the baseline and no more. Row 6 means the meter
	//    still rises here - what must not happen is it rising faster.
	TestEqual(TEXT("no lanterns is exactly 1x"),
		ULLKStormChargeSubsystem::LanternMultiplier(Lantern, 0), 1.f);
	TestEqual(TEXT("a negative count cannot go below 1x"),
		ULLKStormChargeSubsystem::LanternMultiplier(Lantern, -3), 1.f);

	// 2. THE LOCKED ANCHOR. One lantern is 2.5x. It survived the switch from
	//    linearCapped to diminishing precisely because both curves preserve
	//    it, and every minute figure in storm-charge.json assumes it.
	TestNearlyEqual(TEXT("one lantern is the locked 2.5x anchor"),
		ULLKStormChargeSubsystem::LanternMultiplier(Lantern, 1), 2.5f, 0.001f);

	// 3. ROW 12: no keeper's light is free. Asserted as a property rather
	//    than as the numbers 3.40 / 3.94 / 4.26, so that a retune of
	//    perLanternBonus or diminishingDecay cannot make this pass while the
	//    design intent is broken.
	float Previous = ULLKStormChargeSubsystem::LanternMultiplier(Lantern, 1);
	float PreviousStep = Previous - 1.f;
	for (int32 Count = 2; Count <= 8; ++Count)
	{
		const float Current = ULLKStormChargeSubsystem::LanternMultiplier(Lantern, Count);
		const float Step = Current - Previous;

		TestTrue(*FString::Printf(TEXT("lantern %d costs something"), Count), Step > 0.f);
		TestTrue(*FString::Printf(TEXT("lantern %d costs less than lantern %d"), Count, Count - 1),
			Step < PreviousStep);

		Previous = Current;
		PreviousStep = Step;
	}

	// 4. The self-limit. `1 + bonus/(1 - decay)` is the asymptote - 4.75 with
	//    the current values - and the curve must approach it without reaching
	//    it, which is what makes maxMultiplier inert rather than merely
	//    unused.
	const float Asymptote = 1.f + Lantern.PerLanternBonus / (1.f - Lantern.DiminishingDecay);
	TestTrue(TEXT("even twenty lanterns stay under the self-limit"),
		ULLKStormChargeSubsystem::LanternMultiplier(Lantern, 20) < Asymptote);
	TestTrue(TEXT("the self-limit sits above the one-lantern anchor"), Asymptote > 2.5f);

	// 5. The retired curve is still real, row 12: switching back must never
	//    need a code change. This also records WHY it was retired - the
	//    clamp makes two different crew sizes cost the same.
	FLLKLanternChargeRules Linear = Lantern;
	Linear.Curve = ELLKLanternCurve::LinearCapped;
	TestNearlyEqual(TEXT("the retired curve keeps the same one-lantern anchor"),
		ULLKStormChargeSubsystem::LanternMultiplier(Linear, 1), 2.5f, 0.001f);
	TestEqual(TEXT("and clamps three and four lanterns to the same cost, which is why it was retired"),
		ULLKStormChargeSubsystem::LanternMultiplier(Linear, 3),
		ULLKStormChargeSubsystem::LanternMultiplier(Linear, 4));

	// 6. Row 54's dampening. At sensitivity 0 the Warden ignores light
	//    entirely; at 1 it behaves exactly as the old third threshold did.
	//    Those two ends are what make 0.3 a dial rather than a magic number.
	const float OneLantern = ULLKStormChargeSubsystem::LanternMultiplier(Lantern, 1);
	TestEqual(TEXT("at sensitivity 0 the Warden ignores light"),
		ULLKStormChargeSubsystem::WardenMultiplierFrom(OneLantern, 0.f), 1.f);
	TestEqual(TEXT("at sensitivity 1 the Warden feels light in full"),
		ULLKStormChargeSubsystem::WardenMultiplierFrom(OneLantern, 1.f), OneLantern);
	TestTrue(TEXT("at the tuned sensitivity light still costs something"),
		ULLKStormChargeSubsystem::WardenMultiplierFrom(OneLantern, Rules->Warden.LightSensitivity) > 1.f);
	TestTrue(TEXT("but less than it costs the thresholds"),
		ULLKStormChargeSubsystem::WardenMultiplierFrom(OneLantern, Rules->Warden.LightSensitivity) < OneLantern);

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
	TestEqual(TEXT("the Warden's meter starts at zero"), Storm->GetWardenCharge(), 0.f);
	TestFalse(TEXT("the Warden has not converged"), Storm->HasWardenConverged());

	// 2. A dark crew: one charge unit is one second of darkness, which is the
	//    sentence `storm-charge.json` opens with.
	Storm->AdvanceForTest(10.f, 0);
	TestNearlyEqual(TEXT("ten dark seconds is ten charge"), Storm->GetCharge(), 10.f, 0.001f);
	TestNearlyEqual(TEXT("and ten on the Warden's meter too"), Storm->GetWardenCharge(), 10.f, 0.001f);

	// 3. A lit keeper. Ten seconds at the locked 2.5x anchor is 25, and the
	//    Warden takes 1 + 0.3*(2.5-1) = 1.45x of the same ten seconds. This
	//    is row 54 in one assertion: the same light, two different costs.
	const float ChargeBefore = Storm->GetCharge();
	const float WardenBefore = Storm->GetWardenCharge();
	Storm->AdvanceForTest(10.f, 1);
	TestNearlyEqual(TEXT("ten lit seconds is 25 charge"), Storm->GetCharge() - ChargeBefore, 25.f, 0.001f);
	TestNearlyEqual(TEXT("but only 14.5 to the Warden"),
		Storm->GetWardenCharge() - WardenBefore, 14.5f, 0.001f);
	TestTrue(TEXT("so the Warden's meter now trails the thresholds'"),
		Storm->GetWardenCharge() < Storm->GetCharge());

	// 4. ROW 6, the one that matters most. No step of any size, with any
	//    lantern count, may lower either meter.
	float Highest = Storm->GetCharge();
	float HighestWarden = Storm->GetWardenCharge();
	for (int32 Step = 0; Step < 40; ++Step)
	{
		Storm->AdvanceForTest(1.f, Step % 5);
		TestTrue(TEXT("the meter never falls"), Storm->GetCharge() >= Highest);
		TestTrue(TEXT("the Warden's meter never falls"), Storm->GetWardenCharge() >= HighestWarden);
		Highest = Storm->GetCharge();
		HighestWarden = Storm->GetWardenCharge();
	}

	// 5. A zero or negative delta is not a rewind. A paused or hitching frame
	//    must not run the night backwards.
	Storm->AdvanceForTest(0.f, 4);
	TestEqual(TEXT("a zero-second step changes nothing"), Storm->GetCharge(), Highest);
	Storm->AdvanceForTest(-5.f, 4);
	TestEqual(TEXT("a negative step changes nothing"), Storm->GetCharge(), Highest);

	// 6. A negative spike is refused outright rather than applied. This is
	//    the one route by which something could subtract from the meter -
	//    an extraction-call tuning someone later sets negative - and row 6
	//    is enforced here rather than trusted.
	Storm->AddSpike(-500.f, TEXT("TestNegativeSpike"));
	TestEqual(TEXT("a negative spike is refused"), Storm->GetCharge(), Highest);

	// 7. A real spike rises, on both meters. The extraction call's 180 is
	//    three minutes of darkness applied at once, deliberately large
	//    against thresholds of 270 and 480: calling it should move you a
	//    stage, so calling early and calling late are different decisions.
	Storm->AddSpike(Rules->ExtractionCall.StormChargeSpike > 0.f
		? Rules->ExtractionCall.StormChargeSpike : 180.f, TEXT("ExtractionCall"));
	TestTrue(TEXT("a real spike raises the meter"), Storm->GetCharge() > Highest);

	// 8. Thresholds fire once each, in order, and the Warden is not one of
	//    them (row 54). Drive the meter well past both thresholds and the
	//    Warden's value, then count.
	// Both delegates are dynamic and BlueprintAssignable - the things that
	// react to a threshold are presentation and spawning, some of it Chris's -
	// so they bind to a UFUNCTION rather than a lambda.
	ULLKStormTestListener* Listener = NewObject<ULLKStormTestListener>(GetTransientPackage());
	Storm->OnThresholdCrossed.AddDynamic(Listener, &ULLKStormTestListener::OnThresholdCrossed);
	Storm->OnWardenConverged.AddDynamic(Listener, &ULLKStormTestListener::OnWardenConverged);

	// A dark crew, so the Warden's dampened meter rises at the same rate as
	// the thresholds' - the longest honest path to all three events.
	for (int32 Second = 0; Second < 1200; ++Second)
	{
		Storm->AdvanceForTest(1.f, 0);
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

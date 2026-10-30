// The projection the debug HUD shows. M1.7.
//
// The readout's primary number is "patrols hunt in 2:24", not "336 / 480",
// because time is what makes the lantern a decision and a climbing meter is
// two numbers with no stakes. That means the arithmetic behind it is not
// cosmetic: it is the sentence the player makes their decision on, and a
// projection that is wrong by a factor teaches the wrong lesson confidently.
//
// Three failure modes are worth the file, and none of them would crash:
//
//   1. **The counterfactual being backwards.** The HUD shows "2:24, and 0:58
//      if you light one lantern". If those were the wrong way round, the game
//      would teach that light BUYS time - the exact inversion of row 5, stated
//      by the interface in the player's own numbers.
//   2. **Dividing by a zero rate.** `baselineRatePerSecond` is tunable. At 0
//      the night does not advance, and "0:00" would say it arrives
//      immediately - the precise opposite.
//   3. **The Warden being projected off the wrong meter.** It accrues on its
//      own dampened one (row 54). Projecting it from the thresholds' charge
//      would be wrong by the whole of the dampening, and wrong in the
//      direction that makes a lit crew look safer than it is.

#include "Data/LLKStormChargeData.h"
#include "Misc/AutomationTest.h"
#include "Storm/LLKStormChargeSubsystem.h"
#include "UI/LLKDebugHUD.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKStormProjectionTest,
	"LLK.Storm.Projection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKStormProjectionTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK storm projection test reached RunTest."));

	ULLKStormChargeSubsystem* Storm = NewObject<ULLKStormChargeSubsystem>(GetTransientPackage());
	ULLKStormChargeData* Rules = ULLKStormChargeSubsystem::MakeFallbackRules(Storm);
	Storm->SetRulesForTest(Rules);

	// ---- 1. the projection itself -------------------------------------
	//
	// One charge unit is one second of darkness, so dark, the seconds to a
	// threshold are its value. PatrolsAlert is 270.
	TestNearlyEqual(TEXT("dark, PatrolsAlert is 270 seconds away"),
		Storm->SecondsUntilCharge(0.f, 270.f), 270.f, 0.01f);

	// ---- 2. ROW 72: the projection has no second answer ---------------
	//
	// **This section used to be the most important one in the file** and it
	// is now the opposite assertion. It checked the counterfactual's
	// DIRECTION: that one lantern projected as 108 seconds against darkness's
	// 270, that more lanterns always left less time, and that if light ever
	// projected as MORE time the HUD would be telling the player in their own
	// numbers that lighting up was free.
	//
	// Row 72 removed the counterfactual by removing its cause. There is one
	// rate, so there is one projection, and the thing to guard is that no
	// second answer creeps back in. A projection that varied with anything
	// but the authored baseline would now be the bug.
	const float Projected = Storm->SecondsUntilCharge(0.f, 270.f);
	TestNearlyEqual(TEXT("the projection is the threshold divided by the one rate"),
		Projected, 270.f / Rules->BaselineRatePerSecond, 0.01f);
	TestNearlyEqual(TEXT("ROW 72: and asking again gives the same answer, because "
		"nothing about the crew is an input to it"),
		Storm->SecondsUntilCharge(0.f, 270.f), Projected, 0.0001f);

	// ---- 3. a zero rate is never a division, and never "now" ----------
	ULLKStormChargeData* Stopped = ULLKStormChargeSubsystem::MakeFallbackRules(Storm);
	Stopped->BaselineRatePerSecond = 0.f;
	Storm->SetRulesForTest(Stopped);
	const float Never = Storm->SecondsUntilCharge(0.f, 270.f);
	TestTrue(TEXT("a zero rate projects as never, not as zero seconds"),
		Never >= TNumericLimits<float>::Max() * 0.5f);
	TestEqual(TEXT("and the readout renders it as a dash"),
		ALLKDebugHUD::FormatSeconds(Never), FString(TEXT("--:--")));
	Storm->SetRulesForTest(Rules);

	// ---- 4. the Warden comes off the one clock ------------------------
	//
	// Row 54 still gives it its own arrival VALUE. Row 72 took away its own
	// METER: this used to assert that a lit crew met the Warden at the
	// dampened 1.45x rate while the thresholds ran at 2.5x, which was the
	// whole point of there being two meters. With one clock the Warden's
	// 700 is 700 seconds, and the only thing worth asserting is that the
	// countdown and the threshold arithmetic are the same arithmetic.
	TestNearlyEqual(TEXT("the Warden is 700 seconds away"),
		Storm->SecondsUntilWarden(), 700.f, 0.01f);
	TestNearlyEqual(TEXT("ROW 72: by exactly the same calculation as any other threshold"),
		Storm->SecondsUntilWarden(),
		Storm->SecondsUntilCharge(0.f, Rules->Warden.ChargeValue), 0.0001f);

	// ---- 5. the next escalation, and the order it is found in ----------
	FName NextId;
	float Target = 0.f;
	float Current = 0.f;
	TestTrue(TEXT("a fresh night has something coming"),
		Storm->GetNextEscalation(NextId, Target, Current));
	TestEqual(TEXT("and it is the lowest threshold, not the first in the array"),
		NextId, FName(TEXT("PatrolsAlert")));
	TestEqual(TEXT("reported against the clock"), Current, Storm->GetCharge());

	// Cross both thresholds. The Warden must then be what is next. This used
	// to assert that it was reported against its OWN meter and that mixing
	// the two was a failure; row 72 removed the second meter, so the
	// assertion inverts: everything is reported against the one clock, and
	// the old failure mode is now unreachable by construction.
	Storm->AdvanceForTest(500.f);
	TestTrue(TEXT("with both thresholds crossed something is still coming"),
		Storm->GetNextEscalation(NextId, Target, Current));
	TestEqual(TEXT("and it is the Warden"), NextId, FName(TEXT("WardenConverges")));
	TestEqual(TEXT("reported against the one clock, like every other threshold"),
		Current, Storm->GetCharge());
	TestEqual(TEXT("against the Warden's own value"), Target, Rules->Warden.ChargeValue);

	// ---- 6. nothing left to send ---------------------------------------
	//
	// The one state a countdown must not render. Drive the Warden in and the
	// readout must stop claiming anything is on its way.
	Storm->AdvanceForTest(1200.f);
	TestTrue(TEXT("the Warden has converged"), Storm->HasWardenConverged());
	TestFalse(TEXT("and nothing is reported as still coming"),
		Storm->GetNextEscalation(NextId, Target, Current));

	// ---- 7. the formatter ----------------------------------------------
	TestEqual(TEXT("144 seconds reads as 2:24"), ALLKDebugHUD::FormatSeconds(144.f), FString(TEXT("2:24")));
	TestEqual(TEXT("58 seconds reads as 0:58"), ALLKDebugHUD::FormatSeconds(58.f), FString(TEXT("0:58")));
	TestEqual(TEXT("600 seconds reads as 10:00"), ALLKDebugHUD::FormatSeconds(600.f), FString(TEXT("10:00")));
	// Passed and never are different states; flattening both to 0:00 would
	// hide a stage that should have fired and did not.
	TestEqual(TEXT("an already-passed threshold reads as now, not 0:00"),
		ALLKDebugHUD::FormatSeconds(-12.f), FString(TEXT("now")));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

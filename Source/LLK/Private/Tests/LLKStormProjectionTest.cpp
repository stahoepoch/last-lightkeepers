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
		Storm->SecondsUntilCharge(0.f, 270.f, 0), 270.f, 0.01f);

	// Lit, at the locked 2.5x anchor, the same distance takes 108 seconds -
	// which is the `oneLanternSeconds: 108` already published in
	// storm-charge.json. The engine and the data agreeing on that number is
	// the point of this assertion.
	TestNearlyEqual(TEXT("one lantern makes it 108 seconds, as the data says"),
		Storm->SecondsUntilCharge(0.f, 270.f, 1), 108.f, 0.01f);

	// ---- 2. THE COUNTERFACTUAL'S DIRECTION ----------------------------
	//
	// The one that would teach the opposite of the game. If light ever
	// projected as MORE time, the HUD would tell the player, in their own
	// numbers, that lighting the lantern is free.
	for (int32 Lit = 1; Lit <= 4; ++Lit)
	{
		TestTrue(*FString::Printf(TEXT("%d lantern(s) leaves less time than darkness"), Lit),
			Storm->SecondsUntilCharge(0.f, 270.f, Lit) < Storm->SecondsUntilCharge(0.f, 270.f, 0));
		if (Lit > 1)
		{
			TestTrue(*FString::Printf(TEXT("and %d leaves less time than %d"), Lit, Lit - 1),
				Storm->SecondsUntilCharge(0.f, 270.f, Lit)
					< Storm->SecondsUntilCharge(0.f, 270.f, Lit - 1));
		}
	}

	// ---- 3. a zero rate is never a division, and never "now" ----------
	ULLKStormChargeData* Stopped = ULLKStormChargeSubsystem::MakeFallbackRules(Storm);
	Stopped->BaselineRatePerSecond = 0.f;
	Storm->SetRulesForTest(Stopped);
	const float Never = Storm->SecondsUntilCharge(0.f, 270.f, 0);
	TestTrue(TEXT("a zero rate projects as never, not as zero seconds"),
		Never >= TNumericLimits<float>::Max() * 0.5f);
	TestEqual(TEXT("and the readout renders it as a dash"),
		ALLKDebugHUD::FormatSeconds(Never), FString(TEXT("--:--")));
	Storm->SetRulesForTest(Rules);

	// ---- 4. the Warden comes off its own meter, row 54 -----------------
	//
	// Dark, both meters rise together, so the Warden's 700 is 700 seconds
	// away. Lit, the thresholds speed up by 2.5x and the Warden only by
	// 1.45x - so projecting it off the thresholds' rate would make a lit
	// crew look like it meets the Warden far sooner than it does.
	TestNearlyEqual(TEXT("dark, the Warden is 700 seconds away"),
		Storm->SecondsUntilWarden(0), 700.f, 0.01f);

	const float WardenLit = Storm->SecondsUntilWarden(1);
	const float ThresholdPaceLit = Storm->SecondsUntilCharge(0.f, 700.f, 1);
	TestTrue(TEXT("lit, the Warden still arrives sooner than it would in the dark"),
		WardenLit < 700.f);
	TestTrue(TEXT("but much later than the thresholds' pace would suggest"),
		WardenLit > ThresholdPaceLit * 1.5f);
	TestNearlyEqual(TEXT("specifically at the dampened 1.45x rate"),
		WardenLit, 700.f / 1.45f, 0.5f);

	// ---- 5. the next escalation, and the order it is found in ----------
	FName NextId;
	float Target = 0.f;
	float Current = 0.f;
	TestTrue(TEXT("a fresh night has something coming"),
		Storm->GetNextEscalation(NextId, Target, Current));
	TestEqual(TEXT("and it is the lowest threshold, not the first in the array"),
		NextId, FName(TEXT("PatrolsAlert")));
	TestEqual(TEXT("reported against the thresholds' own meter"), Current, Storm->GetCharge());

	// Cross both thresholds. The Warden must then be what is next, and it
	// must be reported against ITS meter - mixing the two is failure mode 3.
	Storm->AdvanceForTest(500.f, 0);
	TestTrue(TEXT("with both thresholds crossed something is still coming"),
		Storm->GetNextEscalation(NextId, Target, Current));
	TestEqual(TEXT("and it is the Warden"), NextId, FName(TEXT("WardenConverges")));
	TestEqual(TEXT("reported against the Warden's meter, not the thresholds'"),
		Current, Storm->GetWardenCharge());
	TestEqual(TEXT("against the Warden's own value"), Target, Rules->Warden.ChargeValue);

	// ---- 6. nothing left to send ---------------------------------------
	//
	// The one state a countdown must not render. Drive the Warden in and the
	// readout must stop claiming anything is on its way.
	Storm->AdvanceForTest(1200.f, 0);
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

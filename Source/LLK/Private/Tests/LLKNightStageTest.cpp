// Which stage the night is in, and whether both machines agree on it. M2.3.
//
// The stage is a derived view, so the interesting failures are not "does it
// compute" but "can it ever disagree with itself". In order of how badly each
// would go wrong:
//
//   1. **A client's stage differing from the server's.** The stage is NOT
//      replicated - it is computed on both sides from the charge and the
//      shared `ULLKStormChargeData`. That is the right design only if the two
//      sides provably land on the same stage, because a badge reading "calm"
//      over a countdown reading "patrols hunt in 0:00" is worse than no badge.
//      This is the same property `LLK.Storm.Snapshot` asserts for the
//      countdown, one level up.
//   2. **The stage going backwards.** Row 64 line one makes the stage the
//      building's permanent AUTHORITY, and row 6 makes the meter monotonic.
//      An authority that can be revoked is a different design, and an enemy
//      that de-escalates mid-chase is the bug that design was chosen to
//      prevent.
//   3. **Naming the wrong threshold.** `StageFrom` counts every threshold the
//      charge has passed and names the HIGHEST-valued one. Naming the
//      last-in-array one instead would be correct for the data as written and
//      silently wrong the day somebody sorts `storm-charge.json` descending -
//      so the descending case is constructed here rather than hoped about.
//   4. **A hardcoded stage count.** The index exists so a third threshold
//      costs no C++ (rows 12 and 33). A test that asserted exactly two
//      thresholds would reintroduce the coupling the index removed, so the
//      assertions below are about relationships and never about the number 2.
//   5. **Invented effect text.** The readout prints the authored `effect`
//      sentence. An unknown id must yield nothing rather than a placeholder,
//      because a badge with made-up consequence reads as authored design.

#include "Algo/Reverse.h"
#include "Data/LLKStormChargeData.h"
#include "Misc/AutomationTest.h"
#include "Storm/LLKNightStage.h"
#include "Storm/LLKStormChargeSubsystem.h"
#include "Storm/LLKStormSnapshot.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKNightStageTest,
	"LLK.Storm.NightStage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKNightStageTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK night stage test reached RunTest."));

	// ---- 0. no rules is calm, and distinguishable from a calm night -----
	//
	// Failure mode: a subsystem whose rules have not resolved reporting a
	// plausible stage. Count 0 is what tells the two apart.
	const FLLKNightStage Unresolved = ULLKStormChargeSubsystem::StageFrom(nullptr, 9999.f, false);
	TestTrue(TEXT("with no rules the stage is calm, however high the charge"), Unresolved.IsCalm());
	TestEqual(TEXT("and its count is zero, which is how a readout tells it apart"),
		static_cast<int32>(Unresolved.Count), 0);
	TestFalse(TEXT("and it authorises nothing"), Unresolved.IsAtLeast(1));

	// A default-constructed stage means the same thing, which matters because
	// that is what anything holding one before the first tick has.
	const FLLKNightStage Default;
	TestTrue(TEXT("a default stage is calm"), Default.IsCalm());
	TestFalse(TEXT("and is not final, which would mean the night was over"), Default.IsFinal());

	// ---- the real rules ------------------------------------------------
	ULLKStormChargeSubsystem* Server = NewObject<ULLKStormChargeSubsystem>(GetTransientPackage());
	ULLKStormChargeData* Rules = ULLKStormChargeSubsystem::MakeFallbackRules(Server);
	Server->SetRulesForTest(Rules);

	// Relationships, never the literal 2. Rows 12 and 33: a third threshold
	// must cost no code, so it must cost no test either.
	const int32 ThresholdCount = Rules->Thresholds.Num();
	if (!TestTrue(TEXT("the rules carry at least two thresholds to escalate through"),
			ThresholdCount >= 2))
	{
		return false;
	}

	// Sorted ascending here so "first" and "last" mean what the assertions
	// below assume, without assuming the DATA is sorted - which is exactly
	// the bug failure mode 3 is about.
	TArray<FLLKStormThreshold> Ascending = Rules->Thresholds;
	Ascending.Sort([](const FLLKStormThreshold& A, const FLLKStormThreshold& B)
		{ return A.Value < B.Value; });
	const FLLKStormThreshold& First = Ascending[0];
	const FLLKStormThreshold& Last = Ascending.Last();

	TestTrue(TEXT("and those two thresholds are at different charges"), Last.Value > First.Value);

	// ---- 1. the stage tracks the charge across every boundary ----------
	const FLLKNightStage AtZero = ULLKStormChargeSubsystem::StageFrom(Rules, 0.f, false);
	TestTrue(TEXT("a night that has not started is calm"), AtZero.IsCalm());
	TestEqual(TEXT("and knows how many thresholds it has to cross"),
		static_cast<int32>(AtZero.Count), ThresholdCount);
	TestTrue(TEXT("and names no threshold"), AtZero.Id.IsNone());

	// Just under the first boundary. Off-by-one here would escalate the whole
	// building a tick early, which is the kind of thing nobody notices and
	// everybody feels.
	const FLLKNightStage JustUnder =
		ULLKStormChargeSubsystem::StageFrom(Rules, First.Value - 0.01f, false);
	TestTrue(TEXT("one hundredth below the first threshold is still calm"), JustUnder.IsCalm());

	// Exactly on it. `AnnounceCrossings` fires at `Charge >= Value`, so the
	// stage must agree at exactly the boundary or the badge and the delegate
	// disagree for one tick.
	const FLLKNightStage AtFirst = ULLKStormChargeSubsystem::StageFrom(Rules, First.Value, false);
	TestEqual(TEXT("exactly on the first threshold the stage has advanced"),
		static_cast<int32>(AtFirst.Index), 1);
	TestEqual(TEXT("and names the threshold that was crossed"), AtFirst.Id, First.Id);
	TestFalse(TEXT("and it is no longer calm"), AtFirst.IsCalm());
	TestTrue(TEXT("and it authorises stage one"), AtFirst.IsAtLeast(1));
	TestFalse(TEXT("but not stage two"), AtFirst.IsAtLeast(2));

	// Between the two.
	const FLLKNightStage Between =
		ULLKStormChargeSubsystem::StageFrom(Rules, Last.Value - 0.01f, false);
	TestEqual(TEXT("below the last threshold the stage is still the earlier one"),
		Between.Id, First.Id);

	const FLLKNightStage AtLast = ULLKStormChargeSubsystem::StageFrom(Rules, Last.Value, false);
	TestEqual(TEXT("on the last threshold every threshold is crossed"),
		static_cast<int32>(AtLast.Index), ThresholdCount);
	TestEqual(TEXT("and the stage names the highest one"), AtLast.Id, Last.Id);

	// Every threshold crossed is still not the end of the night: row 54 keeps
	// the Warden on its own meter, so it has not arrived just because the
	// thresholds are done.
	TestFalse(TEXT("all thresholds crossed is NOT the end - the Warden is separate"),
		AtLast.IsFinal());

	const FLLKNightStage Converged =
		ULLKStormChargeSubsystem::StageFrom(Rules, Last.Value, true);
	TestTrue(TEXT("the Warden on top of every threshold is the end of the night"),
		Converged.IsFinal());

	// And the Warden can arrive while thresholds remain uncrossed, because the
	// two meters are tracked separately. The stage must not claim finality
	// then - "nothing left to send" would be a lie with a threshold pending.
	const FLLKNightStage EarlyWarden = ULLKStormChargeSubsystem::StageFrom(Rules, 0.f, true);
	TestFalse(TEXT("a Warden with thresholds still pending is not a finished night"),
		EarlyWarden.IsFinal());
	TestFalse(TEXT("and such a night is not calm either"), EarlyWarden.IsCalm());

	// ---- 2. FAILURE MODE 2: the stage never goes backwards -------------
	//
	// Driven through the real accrual path rather than by handing StageFrom
	// rising numbers, so this asserts the property of the SYSTEM and not of
	// the arithmetic. Row 6 keeps the charge monotonic; this is the claim that
	// the authority built on it inherits that.
	int32 HighestSeen = 0;
	bool bEverFell = false;
	for (int32 Step = 0; Step < 200; ++Step)
	{
		Server->AdvanceForTest(5.f);
		const int32 Index = static_cast<int32>(Server->GetNightStage().Index);
		if (Index < HighestSeen)
		{
			bEverFell = true;
		}
		HighestSeen = FMath::Max(HighestSeen, Index);
	}
	TestFalse(TEXT("across 1000 seconds of accrual the stage never fell"), bEverFell);
	TestTrue(TEXT("and it did actually escalate, so the loop proved something"),
		HighestSeen >= ThresholdCount);

	// ---- 1 (continued). THE AGREEMENT: a client derives the same stage --
	//
	// Failure mode 1, and the reason nothing about the stage is replicated. A
	// second subsystem with the same rules and no meters of its own is
	// exactly a client's situation.
	ULLKStormChargeSubsystem* Client = NewObject<ULLKStormChargeSubsystem>(GetTransientPackage());
	Client->SetRulesForTest(Rules);

	TestTrue(TEXT("the client's own meter is empty, as a client's is"),
		Client->GetCharge() <= 0.f);
	TestTrue(TEXT("so its own stage is calm while the server's is not"),
		Client->GetNightStage().IsCalm() && !Server->GetNightStage().IsCalm());

	const FLLKStormSnapshot Snapshot = Server->BuildSnapshot();
	const FLLKNightStage FromSnapshot = ULLKStormChargeSubsystem::StageFrom(
		Client->GetRules(), Snapshot.Charge, Snapshot.bWardenConverged);

	TestEqual(TEXT("but from the SNAPSHOT the client reaches the server's exact stage"),
		FromSnapshot, Server->GetNightStage());
	TestEqual(TEXT("including the threshold it names, which is what the HUD prints"),
		FromSnapshot.Id, Server->GetNightStage().Id);

	// ---- 3. FAILURE MODE 3: descending data still names the right stage -
	//
	// Nothing sorts `Thresholds`, and a data file written highest-first is
	// perfectly valid. Naming last-in-array would pass against the real data
	// and fail silently here.
	ULLKStormChargeData* Reversed = ULLKStormChargeSubsystem::MakeFallbackRules(Server);
	Reversed->Thresholds = Ascending;
	Algo::Reverse(Reversed->Thresholds);
	TestEqual(TEXT("the reversed rules really are reversed"),
		Reversed->Thresholds[0].Id, Last.Id);

	const FLLKNightStage FromReversed =
		ULLKStormChargeSubsystem::StageFrom(Reversed, First.Value, false);
	TestEqual(TEXT("with thresholds in descending order the count is still right"),
		static_cast<int32>(FromReversed.Index), 1);
	TestEqual(TEXT("and the stage names the highest CROSSED one, not the first listed"),
		FromReversed.Id, First.Id);

	// ---- the comparison a crossing is announced on ----------------------
	TestTrue(TEXT("calm is earlier than a crossed threshold"), AtZero.IsEarlierThan(AtFirst));
	TestFalse(TEXT("and a crossed threshold is not earlier than calm"),
		AtFirst.IsEarlierThan(AtZero));
	TestTrue(TEXT("the first threshold is earlier than the last"), AtFirst.IsEarlierThan(AtLast));
	TestFalse(TEXT("a stage is not earlier than itself"), AtFirst.IsEarlierThan(AtFirst));

	// The Warden is the closing act, not a third patrol state - so it ranks
	// after every threshold count rather than alongside one.
	TestTrue(TEXT("every threshold crossed is still earlier than the Warden arriving"),
		AtLast.IsEarlierThan(Converged));
	TestTrue(TEXT("and the Warden outranks a threshold count, whatever that count is"),
		AtZero.IsEarlierThan(EarlyWarden));

	// ---- 5. FAILURE MODE 5: the effect text is authored or absent -------
	const FString FirstEffect = Server->GetThresholdEffect(First.Id);
	TestEqual(TEXT("the effect text is the authored sentence, verbatim"),
		FirstEffect, First.Effect);
	TestTrue(TEXT("and the authored data actually has one to print"), !FirstEffect.IsEmpty());

	TestTrue(TEXT("an unknown threshold yields nothing, never a placeholder"),
		Server->GetThresholdEffect(TEXT("NoSuchThreshold")).IsEmpty());
	TestTrue(TEXT("and so does the calm stage's empty id"),
		Server->GetThresholdEffect(NAME_None).IsEmpty());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

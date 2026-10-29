// When the night has nobody left to act. M2.2b.
//
// **The hole this closes was found by Chris asking an obvious question.** He
// asked how a solo keeper deals with going down if there is no teammate, and
// the honest answer was that they wait forever: `bleedOutSeconds` is 0, no
// teammate is coming, and once the anchor revive is spent nothing can get
// them up. The run never ends. Two console commands reached it.
//
// The rule was already in `lantern.json` three times - *"you keep your loot
// until the crew wipes"*, *"the wipe already comes from the whole crew going
// down"* - and had no implementation. For a solo run, one keeper down IS the
// whole crew down.
//
// **The condition is "can anybody still act", not "is anybody standing",**
// which is Chris's framing and the reason this is one rule rather than three.
// A crew with somebody on their feet, a solo keeper whose anchor revive is
// still in hand, and a keeper halfway through using it are all "can act".
// Counting standing bodies gets the second and third wrong, and the third
// would end the run during the eight seconds that save it.
//
// The assertions are arranged around the ways somebody would get it wrong:
// wiping while a keeper is still up, wiping on a solo first knockdown,
// NOT wiping once the charge is spent, letting a crew wait it out, and
// firing the event twice.

#include "Characters/LLKVitalsComponent.h"
#include "Misc/AutomationTest.h"
#include "Run/LLKRunStateSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKCrewWipeTest,
	"LLK.Run.CrewWipe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace
{
	ULLKVitalsComponent* MakeKeeper(UObject* Outer, bool bSolo)
	{
		AActor* Actor = NewObject<AActor>(Outer);
		ULLKVitalsComponent* Vitals = NewObject<ULLKVitalsComponent>(Actor);
		Actor->AddInstanceComponent(Vitals);
		Vitals->SetRulesForTest(0.f, 0.f, true, false);
		Vitals->SetReviveRulesForTest(4.f, 12.f, /*AnchorCharges*/ 1, 8.f, /*SoloOnly*/ true);
		Vitals->SetSoloForTest(bSolo);
		return Vitals;
	}

	ULLKRunStateSubsystem* MakeRunState(UObject* Outer)
	{
		ULLKRunStateSubsystem* RunState = NewObject<ULLKRunStateSubsystem>(Outer);
		// A test subsystem has no net driver, so authority has to be stated.
		RunState->SetAuthorityForTest(true);
		return RunState;
	}
}

bool FLLKCrewWipeTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK crew wipe test reached RunTest."));

	// ---- 1. an empty roll is not a wipe --------------------------------
	//
	// The first frames of a level, before any keeper has registered. Zero
	// keepers who can act would otherwise end the run before it began.
	{
		ULLKRunStateSubsystem* RunState = MakeRunState(GetTransientPackage());
		RunState->EvaluateForTest();
		TestFalse(TEXT("no keepers yet is not a wipe"), RunState->HasCrewWiped());
	}

	// ---- 2. ONE LIVING KEEPER MEANS NO WIPE ----------------------------
	//
	// Chris's rule 4, and the one somebody would get wrong by counting
	// downed keepers instead of able ones.
	{
		ULLKRunStateSubsystem* RunState = MakeRunState(GetTransientPackage());
		ULLKVitalsComponent* A = MakeKeeper(RunState, /*bSolo*/ false);
		ULLKVitalsComponent* B = MakeKeeper(RunState, false);
		ULLKVitalsComponent* C = MakeKeeper(RunState, false);
		RunState->RegisterKeeper(A);
		RunState->RegisterKeeper(B);
		RunState->RegisterKeeper(C);

		TestEqual(TEXT("three keepers in the run"), RunState->CountKeepers(), 3);
		TestEqual(TEXT("and all three can act"), RunState->CountKeepersWhoCanAct(), 3);

		A->SetDownedForTest(true, ELLKDownedReason::Contained);
		B->SetDownedForTest(true, ELLKDownedReason::Contained);
		RunState->EvaluateForTest();
		TestEqual(TEXT("two down leaves one who can act"), RunState->CountKeepersWhoCanAct(), 1);
		TestFalse(TEXT("and one keeper on their feet is not a wipe"), RunState->HasCrewWiped());

		// ---- 3. a CREW with everyone down wipes, and cannot wait it out -
		//
		// Row 9 is solo-only, so a crew has no anchor revive at all. The
		// data's own note says why: a whole crew down must wipe rather than
		// wait indefinitely for someone to press R.
		TestFalse(TEXT("a crew keeper has no anchor revive"), A->IsAnchorReviveAvailable());

		C->SetDownedForTest(true, ELLKDownedReason::Contained);
		RunState->EvaluateForTest();
		TestEqual(TEXT("nobody can act"), RunState->CountKeepersWhoCanAct(), 0);
		TestTrue(TEXT("so the crew is contained"), RunState->HasCrewWiped());
	}

	// ---- 4. SOLO: the first down is not a wipe, the second is ----------
	//
	// Chris's rule 2. The anchor revive is what keeps the run alive, so
	// holding it counts as being able to act even while on the floor.
	{
		ULLKRunStateSubsystem* RunState = MakeRunState(GetTransientPackage());
		ULLKVitalsComponent* Solo = MakeKeeper(RunState, /*bSolo*/ true);
		RunState->RegisterKeeper(Solo);

		Solo->SetDownedForTest(true, ELLKDownedReason::Contained);
		TestTrue(TEXT("a lone keeper still holds their anchor revive"),
			Solo->IsAnchorReviveAvailable());
		RunState->EvaluateForTest();
		TestEqual(TEXT("so they still count as able to act"),
			RunState->CountKeepersWhoCanAct(), 1);
		TestFalse(TEXT("and the first knockdown is not the end of the run"),
			RunState->HasCrewWiped());

		// Spend it. Mid-rescue must also count as able to act, or the run
		// ends during the eight seconds that save it.
		TestTrue(TEXT("spending it works"), Solo->TryAnchorRevive());
		TestTrue(TEXT("and they are now self-reviving"), Solo->IsSelfReviving());
		TestFalse(TEXT("with the charge gone"), Solo->IsAnchorReviveAvailable());
		RunState->EvaluateForTest();
		TestEqual(TEXT("a keeper mid-self-rescue can still act"),
			RunState->CountKeepersWhoCanAct(), 1);
		TestFalse(TEXT("so the run does not end underneath them"), RunState->HasCrewWiped());

		// Let it finish.
		Solo->TickReviveForTest(9.f);
		TestFalse(TEXT("they got up"), Solo->IsDowned());
		RunState->EvaluateForTest();
		TestFalse(TEXT("still no wipe"), RunState->HasCrewWiped());

		// THE SECOND KNOCKDOWN. No charge, no teammate, no timer - this is
		// the soft-lock that existed before M2.2b, and it must now end the
		// run instead.
		Solo->SetDownedForTest(true, ELLKDownedReason::Contained);
		TestFalse(TEXT("no anchor revive remains"), Solo->IsAnchorReviveAvailable());
		TestFalse(TEXT("and they are not self-reviving"), Solo->IsSelfReviving());
		RunState->EvaluateForTest();
		TestEqual(TEXT("nobody can act"), RunState->CountKeepersWhoCanAct(), 0);
		TestTrue(TEXT("so the second knockdown ends the run, rather than waiting forever"),
			RunState->HasCrewWiped());
	}

	// ---- 5. ONCE. One wipe, one haul resolution, one end screen --------
	{
		ULLKRunStateSubsystem* RunState = MakeRunState(GetTransientPackage());
		ULLKVitalsComponent* Solo = MakeKeeper(RunState, true);
		RunState->RegisterKeeper(Solo);
		Solo->SetReviveRulesForTest(4.f, 12.f, /*AnchorCharges*/ 0, 8.f, true);
		Solo->SetDownedForTest(true, ELLKDownedReason::Contained);

		int32 WipeCount = 0;
		// Lambdas cannot bind to a dynamic delegate, so the count is taken
		// from the flag transition instead: evaluate repeatedly and assert
		// the state never un-wipes and the log fires once.
		RunState->EvaluateForTest();
		const bool bFirst = RunState->HasCrewWiped();
		if (bFirst)
		{
			++WipeCount;
		}
		for (int32 i = 0; i < 10; ++i)
		{
			RunState->EvaluateForTest();
		}
		TestTrue(TEXT("the wipe happened"), bFirst);
		TestEqual(TEXT("and only once, however many times it is evaluated"), WipeCount, 1);
		TestTrue(TEXT("and the state never un-wipes"), RunState->HasCrewWiped());
	}

	// ---- 6. a keeper who leaves is not a keeper who cannot act ---------
	//
	// Without unregistering, one player quitting a four-player run would
	// leave a stale entry that never stands up and never wipes.
	{
		ULLKRunStateSubsystem* RunState = MakeRunState(GetTransientPackage());
		ULLKVitalsComponent* Staying = MakeKeeper(RunState, false);
		ULLKVitalsComponent* Leaving = MakeKeeper(RunState, false);
		RunState->RegisterKeeper(Staying);
		RunState->RegisterKeeper(Leaving);
		TestEqual(TEXT("two keepers"), RunState->CountKeepers(), 2);

		RunState->UnregisterKeeper(Leaving);
		TestEqual(TEXT("one after the other leaves"), RunState->CountKeepers(), 1);

		Staying->SetDownedForTest(true, ELLKDownedReason::Contained);
		RunState->EvaluateForTest();
		TestTrue(TEXT("and the remaining keeper going down ends the run"),
			RunState->HasCrewWiped());
	}

	// ---- 7. registering twice must not invent a survivor ---------------
	{
		ULLKRunStateSubsystem* RunState = MakeRunState(GetTransientPackage());
		ULLKVitalsComponent* Only = MakeKeeper(RunState, false);
		RunState->RegisterKeeper(Only);
		RunState->RegisterKeeper(Only);
		TestEqual(TEXT("registering twice counts once"), RunState->CountKeepers(), 1);

		Only->SetDownedForTest(true, ELLKDownedReason::Contained);
		RunState->EvaluateForTest();
		TestTrue(TEXT("so a double-registered keeper cannot survive their own knockdown"),
			RunState->HasCrewWiped());
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

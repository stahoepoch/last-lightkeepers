// Getting back up. M2.2.
//
// M2.1 left a keeper on the floor with no way off it except a console
// command, which is the dangling-seam pattern this project keeps catching.
// This is the way off it, and there are two: a teammate's hold and, for a
// lone keeper, one self-rescue per run.
//
// **The teammate hold is where the design lives.** `revive.maxReviveDistance`
// is 12 studs and `revive.teammateHoldSeconds` is 4, and the rule that makes
// those matter is that range is checked EVERY TICK rather than once at the
// start. Row 59 asks for "everyone protects the carrier"; that is only a rule
// if walking away actually stops the rescue. Checked once, it would be
// decoration - start the hold, wander off, get the revive anyway.
//
// **The anchor revive is row 9 and it is solo-only in the hard sense.**
// `anchorRevive.soloOnly: true`, and the data's own note says why granting it
// to a crew breaks the other half of the same rule: "a whole crew down must
// wipe rather than wait indefinitely for someone to press R." So the test
// checks a crew does NOT get one, which is the half somebody would quietly
// relax to be generous.
//
// **And the price is not what its own key says.**
// `consumesFullLanternCharge: true` has nothing to consume -
// `charge.secondsOfLightPerFullCharge` is 0, light is unlimited, there is no
// fuel. `lantern.json` says the wording is stale itself. The real price is one
// charge per run plus a permanent 120 on the meter, so that is what is
// asserted; no fuel drain was invented to justify a key.

#include "Characters/LLKVitalsComponent.h"
#include "Lantern/LLKLanternComponent.h"
#include "Components/SceneComponent.h"
#include "LLKUnits.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKReviveTest,
	"LLK.Vitals.Revive",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace
{
	/** A keeper with a body, at a position. */
	struct FTestKeeper
	{
		AActor* Actor = nullptr;
		ULLKVitalsComponent* Vitals = nullptr;
	};

	FTestKeeper MakeKeeper(const FVector& Where)
	{
		FTestKeeper Keeper;
		Keeper.Actor = NewObject<AActor>(GetTransientPackage());

		// **A root component, because without one the actor has no position.**
		// `NewObject<AActor>` leaves RootComponent null, so `SetActorLocation`
		// silently does nothing and `GetActorLocation` returns the origin for
		// everybody - which made the first version of this test put both
		// keepers in the same place and then report that the range check was
		// broken. The check was right; the positions were never real.
		USceneComponent* Root = NewObject<USceneComponent>(Keeper.Actor);
		Keeper.Actor->SetRootComponent(Root);
		// Relative rather than world, since an unregistered component in no
		// world has no world transform to set.
		Root->SetRelativeLocation(Where);

		Keeper.Vitals = NewObject<ULLKVitalsComponent>(Keeper.Actor);
		Keeper.Actor->AddInstanceComponent(Keeper.Vitals);
		Keeper.Vitals->SetRulesForTest(0.f, 0.f, true, false);
		Keeper.Vitals->SetReviveRulesForTest(
			/*HoldSeconds*/ 4.f, /*MaxDistanceStuds*/ 12.f,
			/*AnchorCharges*/ 1, /*AnchorSeconds*/ 8.f, /*AnchorSoloOnly*/ true);
		return Keeper;
	}
}

bool FLLKReviveTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK revive test reached RunTest."));

	// Inside 12 studs of each other: 336cm apart would be exactly the limit,
	// so 200cm is comfortably in reach.
	FTestKeeper Fallen = MakeKeeper(FVector::ZeroVector);
	FTestKeeper Rescuer = MakeKeeper(FVector(200.f, 0.f, 0.f));

	// ---- 1. you cannot revive somebody who is standing ------------------
	Fallen.Vitals->BeginRevive(Rescuer.Actor);
	TestFalse(TEXT("a standing keeper is not being revived"), Fallen.Vitals->IsBeingRevived());

	Fallen.Vitals->SetDownedForTest(true, ELLKDownedReason::Contained);

	// ---- 2. the hold, and it takes the authored four seconds ------------
	Fallen.Vitals->BeginRevive(Rescuer.Actor);
	TestTrue(TEXT("the hold started"), Fallen.Vitals->IsBeingRevived());
	TestEqual(TEXT("and names the rescuer, so a HUD can say who"),
		Fallen.Vitals->GetRescuer(), Rescuer.Actor);

	Fallen.Vitals->TickReviveForTest(2.f);
	TestTrue(TEXT("halfway through, progress is visible"),
		Fallen.Vitals->GetReviveProgress() > 0.4f && Fallen.Vitals->GetReviveProgress() < 0.6f);
	TestTrue(TEXT("and they are still down at two seconds of four"), Fallen.Vitals->IsDowned());

	Fallen.Vitals->TickReviveForTest(2.1f);
	TestFalse(TEXT("four seconds of holding gets them up"), Fallen.Vitals->IsDowned());
	TestFalse(TEXT("and the hold is finished"), Fallen.Vitals->IsBeingRevived());

	// ---- 3. THE RULE THAT MATTERS: range is checked every tick ----------
	//
	// Checked once at the start, this would be decoration - begin the hold,
	// wander off, get the revive anyway. Row 59's "everyone protects the
	// carrier" is only a rule if leaving actually stops it.
	Fallen.Vitals->SetDownedForTest(true, ELLKDownedReason::Contained);
	Fallen.Vitals->BeginRevive(Rescuer.Actor);
	Fallen.Vitals->TickReviveForTest(1.f);
	TestTrue(TEXT("a hold is under way"), Fallen.Vitals->IsBeingRevived());

	// 12 studs is 336cm. Step to 400cm - out of reach.
	Rescuer.Actor->GetRootComponent()->SetRelativeLocation(FVector(400.f, 0.f, 0.f));
	Fallen.Vitals->TickReviveForTest(0.1f);
	TestFalse(TEXT("walking out of range cancels the hold"), Fallen.Vitals->IsBeingRevived());
	TestTrue(TEXT("and the keeper is still down"), Fallen.Vitals->IsDowned());
	TestEqual(TEXT("progress is lost, not banked"), Fallen.Vitals->GetReviveProgress(), 0.f);

	// Coming back does not resume where it left off. A rescue interrupted is
	// a rescue restarted, which is what makes the commitment real.
	Rescuer.Actor->GetRootComponent()->SetRelativeLocation(FVector(200.f, 0.f, 0.f));
	Fallen.Vitals->BeginRevive(Rescuer.Actor);
	Fallen.Vitals->TickReviveForTest(1.f);
	TestTrue(TEXT("restarting is possible"), Fallen.Vitals->IsBeingRevived());
	TestTrue(TEXT("but from the beginning, not from where it stopped"),
		Fallen.Vitals->GetReviveProgress() < 0.4f);

	// ---- 4. a rescuer who goes down stops rescuing ----------------------
	//
	// The co-op crisis compounding, which is the decision Chris's fourth
	// pillar is about: now there are two people on the floor and whoever is
	// left has a harder choice.
	Rescuer.Vitals->SetDownedForTest(true, ELLKDownedReason::Contained);
	Fallen.Vitals->TickReviveForTest(0.1f);
	TestFalse(TEXT("a rescuer who is contained mid-hold stops rescuing"),
		Fallen.Vitals->IsBeingRevived());
	TestTrue(TEXT("and the first keeper is still down"), Fallen.Vitals->IsDowned());
	Rescuer.Vitals->SetDownedForTest(false, ELLKDownedReason::Revived);

	// ---- 5. ROW 9. The anchor revive is solo only -----------------------
	//
	// The half somebody would quietly relax to be generous. The data's own
	// note: granting it to a crew breaks the other half of the same rule,
	// because a whole crew down must wipe rather than wait indefinitely for
	// someone to press R.
	FTestKeeper Solo = MakeKeeper(FVector::ZeroVector);
	Solo.Vitals->SetDownedForTest(true, ELLKDownedReason::Contained);

	Solo.Vitals->SetSoloForTest(false);
	TestFalse(TEXT("a keeper in a CREW gets no anchor revive"),
		Solo.Vitals->IsAnchorReviveAvailable());
	TestFalse(TEXT("and asking for one is refused"), Solo.Vitals->TryAnchorRevive());
	TestEqual(TEXT("so the charge is not spent"), Solo.Vitals->GetAnchorChargesLeft(), 1);

	Solo.Vitals->SetSoloForTest(true);
	TestTrue(TEXT("a LONE keeper has one"), Solo.Vitals->IsAnchorReviveAvailable());

	// ---- 6. one per run, and it takes the authored eight seconds --------
	TestTrue(TEXT("spending it succeeds"), Solo.Vitals->TryAnchorRevive());
	TestEqual(TEXT("and the charge is gone"), Solo.Vitals->GetAnchorChargesLeft(), 0);
	TestTrue(TEXT("they are still down while it runs"), Solo.Vitals->IsDowned());

	Solo.Vitals->TickReviveForTest(4.f);
	TestTrue(TEXT("still down at four seconds of eight"), Solo.Vitals->IsDowned());
	TestTrue(TEXT("with progress showing"), Solo.Vitals->GetReviveProgress() > 0.4f);

	Solo.Vitals->TickReviveForTest(4.1f);
	TestFalse(TEXT("eight seconds gets them up"), Solo.Vitals->IsDowned());

	// Once per run. Going down again does not refill it - that is what
	// `chargesPerRun: 1` means, and it is the real price of the mechanic.
	Solo.Vitals->SetDownedForTest(true, ELLKDownedReason::Contained);
	TestFalse(TEXT("there is no second anchor revive in the same run"),
		Solo.Vitals->IsAnchorReviveAvailable());
	TestFalse(TEXT("and asking again is refused"), Solo.Vitals->TryAnchorRevive());

	// ---- 7. the unowned case, as everywhere else -----------------------
	ULLKVitalsComponent* Orphan = NewObject<ULLKVitalsComponent>(GetTransientPackage());
	Orphan->SetReviveRulesForTest(4.f, 12.f, 1, 8.f, true);
	Orphan->SetSoloForTest(true);
	TestFalse(TEXT("an unowned keeper cannot anchor-revive itself"), Orphan->TryAnchorRevive());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

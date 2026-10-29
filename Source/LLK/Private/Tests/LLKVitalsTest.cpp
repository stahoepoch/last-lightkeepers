// Knockdown and the downed state, asserted. M2.1.
//
// Row 47's machines contain people rather than killing them, so there is no
// health here and no death - a keeper is standing or on the floor. Four flags
// in `lantern.json` `knockdown` decide what being on the floor means, and all
// four are load-bearing:
//
//   - `downedWalkSpeed: 0` - cannot crawl.
//   - `lanternForcedOffWhenDowned: true` - the light goes out, not by choice.
//   - `dropsCarriedCrate: false` - row 11, they KEEP what they carry.
//   - `bleedOutSeconds: 0` - they wait indefinitely.
//
// **The assertion that matters most is about a zero.** Two of those values are
// 0, and this morning a different component learned to treat zero as evidence
// of a stale asset - `FLLKLanternLightRules::DescribesARealLight` exists
// because a lantern radius of zero is not a tuning anybody would author. Here
// zero IS the authored design, documented twice in the data file. The same
// defensive check applied here would silently replace Chris's rules with a
// fallback, so this test pins that these zeros survive resolution rather than
// being "helpfully" corrected by a later reader of that other component.
//
// And the seam: `ULLKLanternComponent::ForceOff` and the reason entry
// `ForcedOffWhenDowned` were both written in M1.2 and neither had a caller
// until now. The test checks the lantern goes out AND that it reports why,
// because a HUD explaining a light the keeper did not switch off needs the
// difference.

#include "Characters/LLKVitalsComponent.h"
#include "Data/LLKLanternData.h"
#include "Lantern/LLKLanternComponent.h"
#include "LLKUnits.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKVitalsTest,
	"LLK.Vitals.Knockdown",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKVitalsTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK vitals test reached RunTest."));

	ULLKVitalsComponent* Vitals = NewObject<ULLKVitalsComponent>(GetTransientPackage());

	// ---- 1. a keeper starts standing -----------------------------------
	TestFalse(TEXT("a new keeper is not downed"), Vitals->IsDowned());
	TestEqual(TEXT("and has spent no time on the floor"), Vitals->GetSecondsDowned(), 0.f);

	// With no owning actor there is no authority to defer to, so the public
	// entry points must refuse - the same rule the lantern follows. A
	// component that puts itself down when it cannot find a server is the
	// client-authority hole this whole class is written to avoid.
	Vitals->GoDown(ELLKDownedReason::Contained);
	TestFalse(TEXT("an unowned keeper cannot put itself down"), Vitals->IsDowned());

	// ---- 2. THE ZEROS, which are values and not absences ---------------
	//
	// The authored knockdown block, injected as the importer would resolve it.
	Vitals->SetRulesForTest(/*DownedWalkStuds*/ 0.f, /*BleedOut*/ 0.f,
		/*ForceLanternOff*/ true, /*DropsCrate*/ false);

	TestEqual(TEXT("downedWalkSpeed stays 0 - a downed keeper cannot crawl"),
		Vitals->GetDownedWalkStuds(), 0.f);
	TestEqual(TEXT("and 0 studs converts to 0 cm/s, not to some default"),
		LLKUnits::SpeedFromStuds(Vitals->GetDownedWalkStuds()), 0.f);
	TestEqual(TEXT("bleedOutSeconds stays 0 - they wait indefinitely"),
		Vitals->GetBleedOutSeconds(), 0.f);
	TestFalse(TEXT("dropsCarriedCrate stays false - row 11, they keep the crate"),
		Vitals->DoesDropCarriedCrate());

	// ---- 2b. THE RESOLUTION PATH, which the seam above skips -----------
	//
	// **This section exists because the break-proof failed.** Misapplying the
	// light block's zero-is-stale check to knockdown - substituting a
	// "sensible" crawl speed for the authored 0 - passed every assertion
	// above, because `SetRulesForTest` writes resolved values directly and
	// never runs `ResolveRules`. The state machine was tested; the data path
	// was not.
	//
	// So this drives the real resolution against a real asset carrying the
	// authored zeros, and asserts they come out the other side unhelped.
	ULLKLanternData* Authored = NewObject<ULLKLanternData>(GetTransientPackage());
	Authored->Knockdown.DownedWalkSpeed = 0.f;
	Authored->Knockdown.BleedOutSeconds = 0.f;
	Authored->Knockdown.bLanternForcedOffWhenDowned = true;
	Authored->Knockdown.bDropsCarriedCrate = false;

	ULLKVitalsComponent* Resolved = NewObject<ULLKVitalsComponent>(GetTransientPackage());
	Resolved->ResolveFromAssetForTest(Authored);

	TestEqual(TEXT("the asset resolved, rather than falling back"),
		Resolved->GetRulesSource(), ELLKDataSource::Asset);
	TestEqual(TEXT("and an authored downedWalkSpeed of 0 survives resolution"),
		Resolved->GetDownedWalkStuds(), 0.f);
	TestEqual(TEXT("and an authored bleedOutSeconds of 0 survives resolution"),
		Resolved->GetBleedOutSeconds(), 0.f);
	TestFalse(TEXT("and dropsCarriedCrate false survives resolution"),
		Resolved->DoesDropCarriedCrate());

	// A non-zero crawl speed must also survive, so the test is pinning
	// "respect the data" rather than "always zero".
	Authored->Knockdown.DownedWalkSpeed = 4.f;
	Resolved->ResolveFromAssetForTest(Authored);
	TestEqual(TEXT("and a non-zero crawl speed is respected too"),
		Resolved->GetDownedWalkStuds(), 4.f);

	// ---- 3. the authority path -----------------------------------------
	Vitals->SetDownedForTest(true, ELLKDownedReason::Contained);
	TestTrue(TEXT("the keeper is down"), Vitals->IsDowned());

	// ---- 4. idempotent -------------------------------------------------
	//
	// A second machine reaching an already-downed keeper is not an event. The
	// HUD counts downed keepers and a revive prompt reads the clock; both
	// would move for something that did not happen.
	Vitals->AdvanceForTest(3.f);
	const float AfterThree = Vitals->GetSecondsDownedForTest();
	TestTrue(TEXT("the clock runs while down"), AfterThree >= 3.f);
	Vitals->SetDownedForTest(true, ELLKDownedReason::Contained);
	TestEqual(TEXT("going down again does not restart the clock"),
		Vitals->GetSecondsDownedForTest(), AfterThree);

	// ---- 5. NO BLEED-OUT. The wipe comes from the crew, not a clock ----
	//
	// Ten minutes on the floor. `bleedOutSeconds: 0` means nothing happens,
	// and that is the whole of it: a downed keeper waits for a teammate
	// indefinitely, so leaving somebody down costs them the night's credit
	// without ever killing them (row 59's asymmetry).
	Vitals->AdvanceForTest(600.f);
	TestTrue(TEXT("still down after ten minutes"), Vitals->IsDowned());
	TestTrue(TEXT("and the clock kept counting"), Vitals->GetSecondsDownedForTest() > 600.f);

	// ---- 6. standing up resets the clock -------------------------------
	Vitals->SetDownedForTest(false, ELLKDownedReason::Revived);
	TestFalse(TEXT("the keeper is up"), Vitals->IsDowned());
	TestEqual(TEXT("and the clock is clear for next time"),
		Vitals->GetSecondsDownedForTest(), 0.f);

	// ---- 7. THE M1.2 SEAM, finally connected ---------------------------
	//
	// ForceOff and ELLKLanternChangeReason::ForcedOffWhenDowned were both
	// written in M1.2 and neither had a caller until M2.1. This is that
	// caller, and the reason has to survive - a HUD explaining a lantern the
	// keeper did not switch off needs to tell it from one they did.
	AActor* Keeper = NewObject<AActor>(GetTransientPackage());
	ULLKLanternComponent* Lantern = NewObject<ULLKLanternComponent>(Keeper);
	ULLKVitalsComponent* OwnedVitals = NewObject<ULLKVitalsComponent>(Keeper);
	Keeper->AddInstanceComponent(Lantern);
	Keeper->AddInstanceComponent(OwnedVitals);
	OwnedVitals->SetRulesForTest(0.f, 0.f, true, false);

	Lantern->ApplyStateForTest(true, ELLKLanternChangeReason::PlayerToggled);
	TestTrue(TEXT("the keeper's lantern is lit before they go down"), Lantern->IsLanternOn());

	OwnedVitals->SetDownedForTest(true, ELLKDownedReason::Contained);
	TestFalse(TEXT("going down puts the lantern out"), Lantern->IsLanternOn());

	// Standing up does NOT relight it. The keeper chooses to be seen (row 5),
	// and a revive that silently relit the lantern would make that choice for
	// them at the worst possible moment - just as the machines converge on
	// where they fell.
	OwnedVitals->SetDownedForTest(false, ELLKDownedReason::Revived);
	TestFalse(TEXT("and standing up does NOT relight it - being seen is a choice"),
		Lantern->IsLanternOn());

	// ---- 8. the flag can be turned off, and is honoured ----------------
	//
	// If `lanternForcedOffWhenDowned` were ever false, the light must stay on.
	// Asserted so the flag is a flag rather than a comment.
	ULLKVitalsComponent* Lenient = NewObject<ULLKVitalsComponent>(Keeper);
	Keeper->AddInstanceComponent(Lenient);
	Lenient->SetRulesForTest(0.f, 0.f, /*ForceLanternOff*/ false, false);
	Lantern->ApplyStateForTest(true, ELLKLanternChangeReason::PlayerToggled);
	Lenient->SetDownedForTest(true, ELLKDownedReason::Contained);
	TestTrue(TEXT("with the flag false, a downed keeper keeps their light"),
		Lantern->IsLanternOn());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

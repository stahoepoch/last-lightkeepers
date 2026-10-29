// Being put down must never accelerate the night.
//
// **This is the single hardest constraint the 2026-10-29 research produced,
// and it exists before the code that could violate it.** M2.6 adds the first
// Sweeper, which is the first thing in the game able to knock a keeper down,
// and the obvious-looking implementation is to charge the storm for it.
//
// The reason not to is Creative Assembly's, from Alien: Isolation: **no
// behaviour unlock is ever keyed to a player death**, because escalation
// triggered by failure is punishment the player cannot diagnose. They reach
// for an explanation, find none, and conclude the game is cheating. Chris's
// own fifth replayability pillar says the same thing in his words - never
// secretly rubber-band, the player must always see why danger rose - so a
// knockdown that fed the meter would violate his design using his own
// escalation system.
//
// `Storm Charge` may rise from light, theft, depth, alarms, elapsed time and
// the extraction call. Every one of those is a thing the crew chose. Being
// contained is the thing that happens when the crew gets it wrong, and the
// night does not get to charge them twice for it.
//
// **The property is stronger than "no acceleration", and that is the
// interesting part.** A downed keeper's lantern is forced off (M2.1), which
// lowers the lit-lantern count, which lowers the multiplier. So containment
// actually *slows* the storm - and that is thematically exactly right: a
// contained keeper has stopped lighting the building up. The night loses
// interest in them. Nothing had to be designed for that; it falls out of two
// locked rules meeting.

#include "Characters/LLKVitalsComponent.h"
#include "Data/LLKStormChargeData.h"
#include "Lantern/LLKLanternComponent.h"
#include "Misc/AutomationTest.h"
#include "Storm/LLKStormChargeSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKKnockdownEscalationTest,
	"LLK.Storm.KnockdownDoesNotEscalate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKKnockdownEscalationTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK knockdown escalation test reached RunTest."));

	ULLKStormChargeSubsystem* Storm = NewObject<ULLKStormChargeSubsystem>(GetTransientPackage());
	ULLKStormChargeData* Rules = ULLKStormChargeSubsystem::MakeFallbackRules(Storm);
	Storm->SetRulesForTest(Rules);

	// A keeper with a lantern and a body, registered with the night.
	AActor* Keeper = NewObject<AActor>(GetTransientPackage());
	ULLKLanternComponent* Lantern = NewObject<ULLKLanternComponent>(Keeper);
	ULLKVitalsComponent* Vitals = NewObject<ULLKVitalsComponent>(Keeper);
	Keeper->AddInstanceComponent(Lantern);
	Keeper->AddInstanceComponent(Vitals);
	Vitals->SetRulesForTest(/*DownedWalkStuds*/ 0.f, /*BleedOut*/ 0.f,
		/*ForceLanternOff*/ true, /*DropsCrate*/ false);
	Storm->RegisterLantern(Lantern);

	// ---- 1. a dark keeper going down changes nothing --------------------
	Storm->AdvanceForTest(10.f, Storm->GetLitLanternCount());
	const float ChargeBefore = Storm->GetCharge();
	const float WardenBefore = Storm->GetWardenCharge();

	Vitals->SetDownedForTest(true, ELLKDownedReason::Contained);

	TestEqual(TEXT("going down does not move the thresholds' meter"),
		Storm->GetCharge(), ChargeBefore);
	TestEqual(TEXT("and does not move the Warden's meter"),
		Storm->GetWardenCharge(), WardenBefore);
	TestEqual(TEXT("and does not count as a spike of any size"),
		Storm->GetCharge() - ChargeBefore, 0.f);

	Vitals->SetDownedForTest(false, ELLKDownedReason::Revived);

	// ---- 2. lying there does not accelerate it either -------------------
	//
	// Time passes at the baseline rate because time passes, not because
	// somebody is on the floor. A downed keeper is not a tax.
	Vitals->SetDownedForTest(true, ELLKDownedReason::Contained);
	const float BeforeWaiting = Storm->GetCharge();
	Storm->AdvanceForTest(60.f, Storm->GetLitLanternCount());
	const float WhileDowned = Storm->GetCharge() - BeforeWaiting;

	Vitals->SetDownedForTest(false, ELLKDownedReason::Revived);
	const float BeforeStanding = Storm->GetCharge();
	Storm->AdvanceForTest(60.f, Storm->GetLitLanternCount());
	const float WhileStanding = Storm->GetCharge() - BeforeStanding;

	TestNearlyEqual(TEXT("sixty seconds on the floor costs exactly what sixty seconds standing costs"),
		WhileDowned, WhileStanding, 0.01f);

	// ---- 3. THE STRONGER PROPERTY: containment SLOWS the night ----------
	//
	// A downed keeper's lantern is forced off (M2.1), so the lit count drops
	// and the multiplier drops with it. Containment decelerates the storm.
	// Nothing was designed for that - it falls out of two locked rules
	// meeting, and it is thematically exact: a contained keeper has stopped
	// lighting the building up.
	Lantern->ApplyStateForTest(true, ELLKLanternChangeReason::PlayerToggled);
	TestEqual(TEXT("a standing keeper with a lit lantern counts as one lit lantern"),
		Storm->GetLitLanternCount(), 1);

	const float LitRate = Storm->GetCurrentRate();
	TestTrue(TEXT("and the night runs faster while they are lit"),
		LitRate > Rules->BaselineRatePerSecond);

	Vitals->SetDownedForTest(true, ELLKDownedReason::Contained);
	TestFalse(TEXT("going down puts their lantern out"), Lantern->IsLanternOn());
	TestEqual(TEXT("so the lit count falls"), Storm->GetLitLanternCount(), 0);

	const float DownedRate = Storm->GetCurrentRate();
	TestTrue(TEXT("and the night SLOWS when a keeper is contained, never speeds up"),
		DownedRate < LitRate);
	TestEqual(TEXT("back to the baseline, because nobody is lit"),
		DownedRate, Rules->BaselineRatePerSecond);

	// ---- 4. the one spike that IS allowed, and why ----------------------
	//
	// `anchorRevive.stormChargeSpike` is 120, and that is not a
	// contradiction. The anchor revive is a solo keeper CHOOSING to spend
	// their one self-rescue - an action with a price, like calling
	// extraction's 180. The rule is not "nothing costs meter", it is
	// "failure does not cost meter". Being contained is failure; deciding to
	// get back up on your own is a decision.
	TestTrue(TEXT("the anchor revive has a price, which is a decision and not a punishment"),
		Rules->AnchorRevive.StormChargeSpike > 0.f);
	TestTrue(TEXT("and the extraction call has one too"),
		Rules->ExtractionCall.StormChargeSpike > 0.f);

	// Both are reachable only through AddSpike, which logs its reason - so
	// every acceleration in the game can be traced to a named cause the crew
	// can argue about afterwards. That traceability is Chris's fifth pillar
	// made checkable.
	const float BeforeChosen = Storm->GetCharge();
	Storm->AddSpike(Rules->AnchorRevive.StormChargeSpike, TEXT("AnchorRevive"));
	TestTrue(TEXT("a chosen spike does raise the meter"), Storm->GetCharge() > BeforeChosen);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

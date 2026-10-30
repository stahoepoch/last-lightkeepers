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
// `Storm Pressure` may rise from light, theft, depth, alarms, elapsed time and
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
#include "Data/LLKZonePressureData.h"
#include "Lantern/LLKLanternComponent.h"
#include "Misc/AutomationTest.h"
#include "Storm/LLKZonePressureSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKKnockdownEscalationTest,
	"LLK.Storm.KnockdownDoesNotEscalate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKKnockdownEscalationTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK knockdown escalation test reached RunTest."));

	ULLKZonePressureSubsystem* Storm = NewObject<ULLKZonePressureSubsystem>(GetTransientPackage());
	ULLKZonePressureData* Rules = ULLKZonePressureSubsystem::MakeFallbackRules(Storm);
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
	Storm->AdvanceForTest(10.f);
	const float ChargeBefore = Storm->GetPressure();

	Vitals->SetDownedForTest(true, ELLKDownedReason::Contained);

	// Row 64 line three, and it is now easier to hold than it was: with one
	// clock that light does not touch, the number of ways a knockdown could
	// accidentally escalate the night went from two to one.
	TestEqual(TEXT("going down does not move the clock"),
		Storm->GetPressure(), ChargeBefore);
	TestEqual(TEXT("and does not count as a spike of any size"),
		Storm->GetPressure() - ChargeBefore, 0.f);

	Vitals->SetDownedForTest(false, ELLKDownedReason::Revived);

	// ---- 2. lying there does not accelerate it either -------------------
	//
	// Time passes at the baseline rate because time passes, not because
	// somebody is on the floor. A downed keeper is not a tax.
	Vitals->SetDownedForTest(true, ELLKDownedReason::Contained);
	const float BeforeWaiting = Storm->GetPressure();
	Storm->AdvanceForTest(60.f);
	const float WhileDowned = Storm->GetPressure() - BeforeWaiting;

	Vitals->SetDownedForTest(false, ELLKDownedReason::Revived);
	const float BeforeStanding = Storm->GetPressure();
	Storm->AdvanceForTest(60.f);
	const float WhileStanding = Storm->GetPressure() - BeforeStanding;

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

	// **Row 72 inverted this section, and it cost row 64 an emergent
	// property worth recording.** What used to be asserted here: a lit keeper
	// made the night run faster, and containment therefore SLOWED the night,
	// because a downed keeper's lantern is forced off and the lit count drops.
	// Row 64 called that "a property nobody designed" and liked it.
	//
	// It is gone, because it was a consequence of light driving the clock. The
	// replacement is duller and stronger: containment moves the clock in
	// NEITHER direction. Row 64 line three - escalation is never caused by
	// failing - used to need an argument about which way the emergent effect
	// pointed. Now it needs none, and there is no direction left for a future
	// change to get backwards.
	const float LitRate = Storm->GetCurrentRate();
	TestEqual(TEXT("ROW 72: a lit keeper does not make the night run faster"),
		LitRate, Rules->BaselineRatePerSecond);

	Vitals->SetDownedForTest(true, ELLKDownedReason::Contained);
	TestFalse(TEXT("going down still puts their lantern out"), Lantern->IsLanternOn());
	TestEqual(TEXT("so the lit count still falls"), Storm->GetLitLanternCount(), 0);

	const float DownedRate = Storm->GetCurrentRate();
	TestEqual(TEXT("and the clock does not move in EITHER direction for it"),
		DownedRate, LitRate);

	// ---- 4. nothing costs meter any more, which is the stronger rule ----
	//
	// **This section used to assert the opposite and was right to.** It
	// checked two now-removed weather-spike fields, and argued that neither was
	// a contradiction: the rule was not "nothing costs meter", it was
	// "failure does not cost meter". Being contained is failure; choosing to
	// get back up is a decision, and a decision may carry a price.
	//
	// Rows 72 and 73 changed which side of that line everything sits on. The
	// revive's price became a local distress pulse, the extraction call's
	// became the siren and the convergence it already had, and the clock
	// became weather alone. So the rule is now simply "nothing a player does
	// costs meter", and failure not costing meter is a special case of it
	// rather than a carve-out that had to be argued for.
	//
	// The two numbers are still in `data/zone-pressure.json` because deleting
	// values there is Chris's call (row 55) and the key removal is in Codex's
	// importer handoff. Nothing reads them, and
	// `tests/escalation.test.mjs` fails if anything starts to.
	//
	// **Row 73 deleted the allowlist rather than shortening it.** This
	// asserted that exactly one spike cause existed and that it raised the
	// meter. There are none now: the extraction call was the last, and
	// *"a radio call cannot accelerate the weather."*
	//
	// Which makes this test's whole subject simpler. It exists to prove that
	// a knockdown never escalates the night, and the strongest version of
	// that is no longer "knockdown is not on the allowlist" - it is that
	// nothing is, so there is no list for a future author to add it to.
	const float BeforeIdle = Storm->GetPressure();
	Vitals->SetDownedForTest(true, ELLKDownedReason::Contained);
	Vitals->SetDownedForTest(false, ELLKDownedReason::Revived);
	TestEqual(TEXT("ROW 73: going down and getting up moves the clock by nothing at all"),
		Storm->GetPressure(), BeforeIdle);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

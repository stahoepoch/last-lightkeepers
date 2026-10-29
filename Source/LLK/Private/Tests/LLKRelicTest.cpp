// Row 53's relic, asserted against Chris's own acceptance criteria.
//
// He gave them as three bullets, and the first two are tested here directly:
//
//   - Dark: the relic is physically visible, but its identity/value is
//     unreadable.
//   - Lantern on: it becomes legible and interactable.
//   - Two channels: the visual state change [his message was truncated here].
//
// **The first bullet is the one that needs a test most, because the wrong
// implementation passes every informal check.** `ELLKLootVisibility`'s own
// comment says it in capitals: light gates legibility, never existence. Not
// spawning a relic in the dark is easier to write, and in a screenshot of a
// dark room it looks identical to the correct behaviour. It is also fatal to
// the lesson - an invisible relic cannot tempt anybody, so nobody ever
// chooses to light the lantern, and the ground floor teaches nothing.
//
// So the test asserts there is always SOMETHING to look at, and that what
// cannot be read is only the identity and the value.
//
// The third bullet is a hook rather than a sound: the audio of a relic
// becoming legible is `Content/` and Chris's (row 41), so what is asserted
// here is that the broadcast carries a reason a responder can act on.

#include "Data/LLKLanternData.h"
#include "Data/LLKLootRulesData.h"
#include "LLKRelicTestListener.h"
#include "Loot/LLKRelicComponent.h"
#include "Misc/AutomationTest.h"
#include "Storm/LLKLightField.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKRelicTest,
	"LLK.Loot.RelicLegibility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKRelicTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK relic test reached RunTest."));

	ULLKRelicComponent* Relic = NewObject<ULLKRelicComponent>(GetTransientPackage());
	ULLKRelicTestListener* Listener = NewObject<ULLKRelicTestListener>(GetTransientPackage());
	Relic->OnLegibilityChanged.AddDynamic(Listener, &ULLKRelicTestListener::OnChanged);

	Relic->SetRelicIdentity(TEXT("PocketWatch"), FText::FromString(TEXT("Engraved pocket watch")), 450);

	// The threshold a resolved component would derive: one lantern's worth at
	// the edge of its own reach, which is intensity 250 halved.
	Relic->SetThresholdForTest(125.f);

	// ---- BULLET ONE: dark, visible, unreadable -------------------------
	TestFalse(TEXT("a relic in the dark is not legible"), Relic->IsLegible());

	// **The assertion that catches the easy wrong implementation.** There has
	// to be something to look at, or nobody is ever tempted to light up.
	const FText DarkName = Relic->GetDisplayName();
	TestFalse(TEXT("but it still has something to display - it is VISIBLE"), DarkName.IsEmpty());
	TestFalse(TEXT("and that display text is not its real name"),
		DarkName.ToString().Contains(TEXT("pocket watch")));

	// The value is withheld as -1 rather than as 0, because 0 is a real value
	// a worthless relic could have and the player has not earned the number.
	TestEqual(TEXT("its value is unreadable, and says so rather than reading zero"),
		Relic->GetReadableValue(), -1);

	TestFalse(TEXT("and it cannot be taken while unidentified"), Relic->CanBeTaken());
	TestEqual(TEXT("nothing has broadcast yet"), Listener->Count, 0);

	// Light below the bar changes nothing. A keeper at the far edge of the
	// room is not reading anything.
	Relic->EvaluateForTest(124.9f);
	TestFalse(TEXT("light just under the threshold does not reveal it"), Relic->IsLegible());
	TestEqual(TEXT("and does not broadcast"), Listener->Count, 0);

	// ---- BULLET TWO: lantern on, legible and interactable --------------
	Relic->EvaluateForTest(125.f);
	TestTrue(TEXT("light at the threshold makes it legible"), Relic->IsLegible());
	TestEqual(TEXT("its real name is readable now"),
		Relic->GetDisplayName().ToString(), FString(TEXT("Engraved pocket watch")));
	TestEqual(TEXT("and so is its value"), Relic->GetReadableValue(), 450);

	// ---- BULLET THREE: the change is announced, with a reason ----------
	TestEqual(TEXT("it broadcast exactly once"), Listener->Count, 1);
	TestTrue(TEXT("the broadcast said legible"), Listener->LastLegible);
	TestEqual(TEXT("and said why, so a sound or a flash can respond"),
		Listener->LastReason, ELLKRelicChangeReason::LitAndRead);

	// Sticky, and idempotent with it. Identity is knowledge the player paid
	// for once; re-broadcasting would make a HUD chime every tenth of a
	// second for as long as the keeper stood there.
	Relic->EvaluateForTest(5000.f);
	TestEqual(TEXT("more light does not re-announce it"), Listener->Count, 1);
	TestTrue(TEXT("and it stays legible"), Relic->IsLegible());

	// ---- the design call, asserted so reversing it is one flag ---------
	//
	// Identity is sticky but taking is gated on live light, so a crew cannot
	// light the floor once, go dark and loot for free. This is the proposal
	// flagged in the header for Chris; the test states the behaviour so that
	// changing his mind is a visible change rather than a silent one.
	ULLKRelicComponent* Lit = NewObject<ULLKRelicComponent>(GetTransientPackage());
	Lit->SetThresholdForTest(125.f);
	Lit->SetRelicIdentity(TEXT("Ledger"), FText::FromString(TEXT("Account ledger")), 200);
	Lit->EvaluateForTest(400.f);
	TestTrue(TEXT("a read relic is legible"), Lit->IsLegible());

	// ---- baseline loot is the contrast lesson two needs ----------------
	//
	// Lesson one of the ground floor is "there are things here and you can
	// carry them", taught with the lantern OFF. If baseline loot needed light
	// too, there would be nothing for lesson two to be a contrast against.
	ULLKRelicComponent* Baseline = NewObject<ULLKRelicComponent>(GetTransientPackage());
	Baseline->Visibility = ELLKLootVisibility::Always;
	Baseline->SetThresholdForTest(125.f);
	Baseline->SetRelicIdentity(TEXT("Bandage"), FText::FromString(TEXT("Bandage")), 5);
	Baseline->EvaluateForTest(0.f);
	TestFalse(TEXT("baseline loot is not made legible by zero light on its own"),
		Baseline->IsLegible());
	// ...but its visibility tier is what exempts it, and that is checked
	// through CanBeTaken rather than through the light, because `Always` loot
	// does not wait for a lantern to be liftable.
	Baseline->EvaluateForTest(TNumericLimits<float>::Max());
	TestTrue(TEXT("and once read, Always loot can be taken in the dark"), Baseline->CanBeTaken());

	// ---- the threshold agrees with the light field ---------------------
	//
	// The threshold is DERIVED: one lantern at the edge of its own reach.
	// This asserts the derivation against LLKLightField itself, so the two
	// cannot drift - if the falloff formula changes, "readable exactly within
	// one lantern's radius" has to still be true or this fails.
	FLLKLightSource Lantern;
	Lantern.Position = FVector::ZeroVector;
	Lantern.Intensity = 250.f;
	Lantern.RadiusCm = 840.f;

	const float AtEdge = LLKLightField::BrightnessAt({Lantern}, FVector(840.f, 0.f, 0.f));
	TestNearlyEqual(TEXT("one lantern at its own radius delivers half its intensity"),
		AtEdge, 125.f, 0.01f);
	TestNearlyEqual(TEXT("which is exactly the identify threshold"), AtEdge, 125.f, 0.01f);

	const float JustInside = LLKLightField::BrightnessAt({Lantern}, FVector(830.f, 0.f, 0.f));
	const float JustOutside = LLKLightField::BrightnessAt({Lantern}, FVector(850.f, 0.f, 0.f));
	TestTrue(TEXT("a relic just inside the lantern's reach is readable"), JustInside > 125.f);
	TestTrue(TEXT("and one just outside it is not"), JustOutside < 125.f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

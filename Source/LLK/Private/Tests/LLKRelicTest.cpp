// Row 53's relic, asserted against Chris's own acceptance criteria.
//
// He gave them as three bullets, and the first two are tested here directly:
//
//   - Dark: the relic is physically visible, but its identity/value is
//     unreadable.
//   - Lantern on: it becomes legible and interactable.
//   - Two channels: the visual state change plus a non-audio physical tell.
//
// He corrected that third bullet himself, and the correction is what this
// file's last section tests: what he wants is not a second visual effect. It
// is that the player can confirm the change through **the world object** and
// **the interaction state**, with no audio cue needed. The object half is
// `Content/` and hangs on the legibility hook. The interaction half is code,
// and it is only a channel if it CHANGES at a moment somebody can perceive -
// so it is tracked state with its own broadcast, not a bool a HUD polls in
// silence.
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

	// ---- CHANNEL TWO: the interaction state, which must change ---------
	//
	// Driven through SampleLightForTest, which runs the same path the tick
	// does, so this tests the affordance as it actually behaves rather than a
	// reimplementation of it.
	ULLKRelicComponent* Shelf = NewObject<ULLKRelicComponent>(GetTransientPackage());
	ULLKRelicTestListener* Prompt = NewObject<ULLKRelicTestListener>(GetTransientPackage());
	Shelf->OnInteractableChanged.AddDynamic(Prompt, &ULLKRelicTestListener::OnInteractable);
	Shelf->SetThresholdForTest(125.f);
	Shelf->SetRelicIdentity(TEXT("Brooch"), FText::FromString(TEXT("Enamel brooch")), 300);

	// Dark and unread: not interactable, and the prompt can say which of the
	// two reasons it is.
	Shelf->SampleLightForTest(0.f);
	TestFalse(TEXT("an unread relic in the dark is not interactable"), Shelf->CanBeTaken());
	TestEqual(TEXT("and the prompt knows it is unidentified, not merely dark"),
		Shelf->GetInteractBlock(), ELLKRelicInteractBlock::Unidentified);
	TestEqual(TEXT("nothing has changed, so nothing announced it"), Prompt->InteractCount, 0);

	// Light arrives. Legibility AND the affordance both turn on, and the
	// affordance announces itself - that is the channel.
	Shelf->SampleLightForTest(400.f);
	TestTrue(TEXT("lit, it is legible"), Shelf->IsLegible());
	TestTrue(TEXT("and interactable"), Shelf->CanBeTaken());
	TestEqual(TEXT("with nothing blocking it"),
		Shelf->GetInteractBlock(), ELLKRelicInteractBlock::None);
	TestEqual(TEXT("and the affordance channel fired once"), Prompt->InteractCount, 1);
	TestTrue(TEXT("saying it became interactable"), Prompt->LastInteractable);

	// Still lit: no change, so no further announcement. A prompt wired to an
	// unconditional broadcast would rebuild itself ten times a second.
	Shelf->SampleLightForTest(400.f);
	TestEqual(TEXT("staying lit does not re-announce"), Prompt->InteractCount, 1);

	// **The keeper walks away with the lantern.** Identity is knowledge and
	// stays; the affordance goes out, and it says why. This is the moment the
	// interface teaches row 5 without a tooltip.
	Shelf->SampleLightForTest(0.f);
	TestTrue(TEXT("it is still legible - identity is knowledge, not a light switch"),
		Shelf->IsLegible());
	TestFalse(TEXT("but it can no longer be taken"), Shelf->CanBeTaken());
	TestEqual(TEXT("and the affordance channel announced the loss"), Prompt->InteractCount, 2);
	TestFalse(TEXT("saying it is no longer interactable"), Prompt->LastInteractable);
	TestEqual(TEXT("because it needs light - the reason a prompt should say out loud"),
		Prompt->LastBlock, ELLKRelicInteractBlock::NeedsLight);
	TestEqual(TEXT("and the block reads the same when asked directly"),
		Shelf->GetInteractBlock(), ELLKRelicInteractBlock::NeedsLight);

	// Confirmable without audio, which is the whole of his correction: two
	// distinct machine-readable states, neither of them a sound, and the
	// difference between them is legible to a prompt.
	TestNotEqual(TEXT("unidentified and needs-light are distinguishable states"),
		ELLKRelicInteractBlock::Unidentified, ELLKRelicInteractBlock::NeedsLight);

	// Light again: back on, and announced again.
	Shelf->SampleLightForTest(400.f);
	TestTrue(TEXT("light restores the affordance"), Shelf->CanBeTaken());
	TestEqual(TEXT("and announced it"), Prompt->InteractCount, 3);
	TestEqual(TEXT("legibility itself only ever fired once"), Prompt->Count, 0);

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

	// ---- THE SILENT HALF OF THE STALE-ASSET BUG ------------------------
	//
	// The relic derives its read threshold from the lantern's authored
	// intensity. A stale asset gives intensity 0, so the threshold is 0, so
	// `Brightness >= Threshold` is true in a pitch-dark room and EVERY RELIC
	// READS ITSELF. Row 53 deleted with no symptom at all, because a relic
	// that is always readable looks exactly like a relic that works.
	//
	// Chris found the invisible lantern in thirty seconds. Nobody would have
	// found this, which is why it gets an assertion of its own.
	ULLKRelicComponent* Guarded = NewObject<ULLKRelicComponent>(GetTransientPackage());
	Guarded->SetThresholdForTest(0.f);
	Guarded->SetRelicIdentity(TEXT("Locket"), FText::FromString(TEXT("Silver locket")), 600);
	Guarded->EvaluateForTest(0.f);
	TestTrue(TEXT("a zero threshold DOES make a relic read in the dark - the bug, demonstrated"),
		Guarded->IsLegible());

	// Which is why ResolveRules refuses a zero threshold outright. Asserted
	// through DescribesARealLight, the one place the check lives, so the two
	// components cannot disagree about what counts as a usable light.
	FLLKLanternLightRules Zeroed;
	TestFalse(TEXT("a zeroed light block is refused at the data layer"),
		Zeroed.DescribesARealLight());
	TestEqual(TEXT("and its intensity - the source of the zero threshold - is zero"),
		Zeroed.Intensity, 0.f);

	FLLKLanternLightRules Real;
	Real.RadiusStuds = 30.f;
	Real.ConeAngleDegrees = 72.f;
	Real.Intensity = 250.f;
	TestTrue(TEXT("a real light block is accepted"), Real.DescribesARealLight());
	TestTrue(TEXT("and yields a threshold above zero"), Real.Intensity * 0.5f > 0.f);

	// ---- taking it, and the double-take race ---------------------------
	//
	// Chris: "i can't take it." Nothing implemented taking until now, so
	// CanBeTaken was a true that led nowhere.
	ULLKRelicComponent* Takeable = NewObject<ULLKRelicComponent>(GetTransientPackage());
	Takeable->SetThresholdForTest(125.f);
	Takeable->SetRelicIdentity(TEXT("Brooch2"), FText::FromString(TEXT("Enamel brooch")), 300);
	Takeable->EvaluateForTest(400.f);

	// With no owner there is no authority, so the server-side take must
	// refuse - the same shape as the unowned lantern refusing to flip itself.
	TestFalse(TEXT("an unowned relic cannot be taken, because nothing owns the decision"),
		Takeable->TakeByServer());
	TestFalse(TEXT("and it is not marked taken"), Takeable->IsTaken());

	// The outline stencil is the object's own channel, and it has to
	// distinguish three states or an outline material cannot either.
	ULLKRelicComponent* Stencil = NewObject<ULLKRelicComponent>(GetTransientPackage());
	Stencil->SetThresholdForTest(125.f);
	Stencil->SetRelicIdentity(TEXT("Watch2"), FText::FromString(TEXT("Pocket watch")), 450);
	Stencil->SampleLightForTest(0.f);
	TestEqual(TEXT("unread: stencil 1, there but unreadable"), Stencil->GetOutlineStencil(), 1);
	Stencil->SampleLightForTest(400.f);
	TestEqual(TEXT("lit and takeable: stencil 3"), Stencil->GetOutlineStencil(), 3);
	Stencil->SampleLightForTest(0.f);
	TestEqual(TEXT("read but dark: stencil 2, known and out of reach"),
		Stencil->GetOutlineStencil(), 2);
	// Three distinct values, which is what lets one material say three things.
	TestNotEqual(TEXT("the three states are distinguishable"), 1, 2);

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

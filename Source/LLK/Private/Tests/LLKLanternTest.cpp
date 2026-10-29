// The lantern's state rules, asserted. M1.2.
//
// This tests the one thing the component is for: that its state changes
// exactly when it should and tells listeners why. Everything the rest of the
// game reads about light goes through here - storm charge counts lit
// lanterns, enemies converge on the brightest position, row 5 makes light
// reveal value and attract danger - so a toggle that double-fires or a
// forced-off that reports the wrong reason is not a cosmetic bug.
//
// It needs no world: the component is constructed standalone and driven
// through its authority-side path, which is the half that decides. The
// replicated half is covered by the fact that OnRep is the only other caller
// of the broadcast, and it is a one-liner.

#include "LLKLanternTestListener.h"
#include "Lantern/LLKLanternComponent.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKLanternStateTest,
	"LLK.Lantern.StateRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKLanternStateTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK lantern test reached RunTest."));

	ULLKLanternComponent* Lantern = NewObject<ULLKLanternComponent>(GetTransientPackage());
	ULLKLanternTestListener* Listener = NewObject<ULLKLanternTestListener>(GetTransientPackage());
	Lantern->OnLanternStateChanged.AddDynamic(Listener, &ULLKLanternTestListener::OnChanged);

	// 1. It starts dark. The keeper chooses to be seen; the game does not
	//    choose for them, and row 5's whole trade depends on that being a
	//    decision rather than a default.
	TestFalse(TEXT("a new lantern is off"), Lantern->IsLanternOn());
	TestEqual(TEXT("nothing has broadcast yet"), Listener->Count, 0);

	// With no owning actor there is no authority to defer to, so the public
	// setter must not change state. A component that flips itself when it
	// cannot find a server is the exact client-authority hole M1.2 forbids.
	Lantern->SetLanternOn(true);
	TestFalse(TEXT("an unowned lantern does not flip itself"), Lantern->IsLanternOn());
	TestEqual(TEXT("and does not broadcast"), Listener->Count, 0);

	// 2. The authority path. ApplyState is what the server reaches after the
	//    RPC, so this is the decision the whole game reads.
	Lantern->ApplyStateForTest(true, ELLKLanternChangeReason::PlayerToggled);
	TestTrue(TEXT("the lantern is on"), Lantern->IsLanternOn());
	TestEqual(TEXT("it broadcast once"), Listener->Count, 1);
	TestTrue(TEXT("the broadcast said on"), Listener->LastOn);
	TestEqual(TEXT("and said why"), Listener->LastReason, ELLKLanternChangeReason::PlayerToggled);

	// 3. Idempotent. The storm charge multiplier is a count of lit lanterns,
	//    so a no-op that still broadcasts would move the meter for nothing.
	Lantern->ApplyStateForTest(true, ELLKLanternChangeReason::PlayerToggled);
	TestEqual(TEXT("setting on again does not broadcast"), Listener->Count, 1);

	// 4. Forced off carries its reason through rather than flattening to
	//    "off". A HUD that has to explain a lantern the keeper did not turn
	//    off needs the difference.
	Lantern->ApplyStateForTest(false, ELLKLanternChangeReason::ForcedOffWhenDowned);
	TestFalse(TEXT("the lantern is off"), Lantern->IsLanternOn());
	TestEqual(TEXT("it broadcast again"), Listener->Count, 2);
	TestEqual(TEXT("the reason survived"),
		Listener->LastReason, ELLKLanternChangeReason::ForcedOffWhenDowned);

	// 5. The downed rule defaults to the data's value, not to false. If this
	//    ever reads false on the fallback path, a downed keeper keeps a lit
	//    lantern and row 47's machines converge on a player who cannot move.
	TestTrue(TEXT("being downed forces the lantern off by default"),
		Lantern->DoesDownedForceLanternOff());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

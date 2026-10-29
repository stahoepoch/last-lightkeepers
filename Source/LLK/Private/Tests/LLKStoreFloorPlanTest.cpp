// The ground-floor plan, checked against the numbers it is tuned to. M1.5 scaffold.
//
// A disposable greybox does not usually earn a test. This one does, because
// three of its dimensions are tuned against values in `data/` and the whole
// point of the floor is to answer questions about them. If the plan drifts out
// of agreement with the data, the floor stops being able to answer anything
// and nothing else would notice.
//
// The three relationships, all from authored data:
//
//   1. **No sightline longer than `Sweepers.aggroRadiusLit` (38 studs).**
//      Past that a lit keeper can be hunted by something they had no way of
//      seeing, which reads as the game cheating rather than as danger.
//   2. **A counter must be taller than the jump apex.** Row 63 wants vaulting
//      a counter to feel desperate, and a counter you can hop is a vault with
//      nothing to do.
//   3. **A service ledge must be shorter than it.** The contrast is the
//      teaching: one obstacle you clear and one you cannot, in the same
//      building, learnable by trying.
//
// And the ramp: Chris asked for *"a gentle inclined plane"*, so it has to be
// comfortably inside Unreal's walkable limit rather than near it.

#include "Characters/LLKCharacter.h"
#include "Harness/LLKStoreFloorPlan.h"
#include "LLKUnits.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKStoreFloorPlanTest,
	"LLK.Harness.StoreFloorPlan",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKStoreFloorPlanTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK store floor plan test reached RunTest."));

	const FLLKStoreFloorPlan Plan;

	// ---- 1. THE SIGHTLINE RULE ------------------------------------------
	//
	// `enemies.json` Sweepers.aggroRadiusLit is 38 studs, and
	// tests/balance.test.mjs already pins the lantern's 30-stud reach against
	// it. This is the layout half of the same relationship.
	const float AggroRadiusLitStuds = 38.f;

	TestTrue(TEXT("the plan's own sightline limit is under the lit aggro radius"),
		Plan.MaxSightlineStuds < AggroRadiusLitStuds);
	TestTrue(TEXT("and the layout obeys its own limit"), Plan.ObeysSightlineRule());
	TestTrue(TEXT("so no straight run exceeds the range a lit keeper is noticed from"),
		Plan.LongestStraightStuds() < AggroRadiusLitStuds);

	// Not so short that the floor becomes a maze of cupboards. Lesson two
	// needs ONE long view: the player has to see the thing they woke and have
	// room to go dark and live.
	TestTrue(TEXT("but there is still a long view to see what you woke from"),
		Plan.MaxSightlineStuds > 20.f);

	// ---- 2. THE TWO HEIGHTS, against the real jump ----------------------
	//
	// Read from the character rather than restated, so a retune of the jump
	// shows up here rather than silently making a counter hoppable.
	const ALLKCharacter* Keeper = GetDefault<ALLKCharacter>();
	if (!TestNotNull(TEXT("the keeper class has a default object"), Keeper))
	{
		return false;
	}
	const float ApexCm = Keeper->GetJumpApexCm();
	TestTrue(TEXT("the keeper can jump at all"), ApexCm > 10.f);

	const float CounterCm = LLKUnits::DistanceFromStuds(Plan.CounterHeightStuds);
	const float LedgeCm = LLKUnits::DistanceFromStuds(Plan.ServiceLedgeHeightStuds);

	// Row 63: a counter must be mantled, never hopped, or the vault has
	// nothing it is the only answer to.
	TestTrue(TEXT("a shop counter is taller than the jump apex - it must be mantled"),
		CounterCm > ApexCm);
	TestTrue(TEXT("and not marginally, so nobody tries it and blames the game"),
		CounterCm > ApexCm * 1.3f);

	// The contrast is the teaching.
	TestTrue(TEXT("a service ledge is inside the jump apex - it can be cleared"),
		LedgeCm < ApexCm);
	TestTrue(TEXT("so the building contains one obstacle you clear and one you cannot"),
		LedgeCm < CounterCm);

	// ---- 3. the ramp is gentle, and derived rather than stated ----------
	const float Angle = Plan.RampAngleDegrees();
	TestTrue(TEXT("the ramp climbs at all"), Angle > 5.f);
	// Unreal walks up to 44 degrees by default. A gentle incline should be
	// nowhere near that, or it reads as a hill rather than an escalator.
	TestTrue(TEXT("and is comfortably inside Unreal's walkable limit"), Angle < 30.f);

	// ---- 4. the shell is a shop, not a corridor or a field --------------
	TestTrue(TEXT("the floor is deep enough to get lost in"), Plan.DepthStuds >= 80.f);
	TestTrue(TEXT("and wide enough to have departments rather than one aisle"),
		Plan.WidthStuds >= 60.f);
	TestTrue(TEXT("the ceiling is high, so the dark stays dark overhead"),
		Plan.WallHeightStuds >= 10.f);

	// The lantern reaches 30 studs. A floor narrower than that would be lit
	// wall to wall from the middle, which removes the trade entirely.
	TestTrue(TEXT("the floor is wider than the lantern reaches, so light is a choice"),
		Plan.WidthStuds > 30.f * 1.5f);

	// ---- 5. there is loot to find and places to put enemies -------------
	TestTrue(TEXT("there are jewellery cases to hold relics"), Plan.JewelleryCases >= 3);
	TestTrue(TEXT("and a cosmetics grid to hold baseline loot"),
		Plan.CosmeticsRows * Plan.CosmeticsColumns >= 6);

	// ---- 6. the service corridor is an escape, not a shortcut -----------
	//
	// Narrow enough that a lantern fills it and a keeper cannot be flanked.
	TestTrue(TEXT("the service corridor is narrower than the lantern reaches"),
		Plan.ServiceCorridorWidthStuds < 30.f);
	TestTrue(TEXT("but wide enough for two keepers to pass"),
		Plan.ServiceCorridorWidthStuds >= 5.f);
	TestTrue(TEXT("and its doors are passable"), Plan.ServiceDoorWidthStuds >= 3.f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

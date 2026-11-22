// What a child can climb onto, asserted against the store that already exists.
//
// The point of this verb is that Boswell's was already shaped for it:
// counters at 89.6 cm and shelves at 168 cm against a keeper whose eye is at
// 125 cm. So the tests that matter are not about the arithmetic in the
// abstract - they are about whether that sorting still holds, and it is a
// claim about three constants in three different files.
//
//   1. A counter is climbable and a shelf is not. If either flips, the verb
//      either does nothing in the whole store or lets the player onto the
//      top of every gondola in it.
//   2. The ceiling is the keeper's eye, derived and not restated, so a body
//      rescale carries it.
//   3. The mantle goes UP before it goes ACROSS, or the capsule is dragged
//      through the front of whatever is being climbed.
//   4. Climbing is louder than walking and quieter than pushing. Third entry
//      in the chapter's acoustic vocabulary.

#include "Characters/LLKClimbFeel.h"
#include "Harness/LLKStoreFloorPlan.h"
#include "Lantern/LLKLanternFeel.h"
#include "LLKUnits.h"
#include "Misc/AutomationTest.h"
#include "World/LLKPushFeel.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKClimbTest, "LLK.Characters.Climb",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKClimbTest::RunTest(const FString& Parameters)
{
	// The movement component's default, which is what the real gate uses.
	// Anything the walk steps over is not a climb.
	constexpr float StepHeightCm = 45.f;

	// ---- 1. The store sorts itself --------------------------------------
	{
		const FLLKStoreFloorPlan Plan;
		const float CounterCm = LLKUnits::DistanceFromStuds(Plan.CounterHeightStuds);
		const float ShelfCm = LLKUnits::DistanceFromStuds(Plan.ShelfHeightStuds);

		TestTrue(TEXT("a counter is something a child can get on top of"),
			LLKClimbFeel::IsClimbable(CounterCm, StepHeightCm));
		TestFalse(TEXT("and a shelf is not"),
			LLKClimbFeel::IsClimbable(ShelfCm, StepHeightCm));

		// The whole loop: a counter-height fixture pushed to the foot of a
		// shelf is the step that makes the shelf reachable. If the fixture
		// were unclimbable the push verb would lead nowhere.
		TestTrue(TEXT("and a fixture at counter height is climbable too, which is the loop"),
			LLKClimbFeel::IsClimbable(CounterCm, StepHeightCm));
	}

	// ---- 2. The ceiling is the keeper's own eye -------------------------
	{
		TestEqual(TEXT("the climb ceiling is the eye height, not a copy of it"),
			LLKClimbFeel::MaxLedgeHeightCm(), LLKLanternFeel::KeeperEyeHeightCm, 0.001f);

		// Exactly at the eye is in; a hair over is out. Stated because a
		// boundary nobody pinned is a boundary somebody will shift.
		TestTrue(TEXT("a ledge exactly at eye height is climbable"),
			LLKClimbFeel::IsClimbable(LLKLanternFeel::KeeperEyeHeightCm, StepHeightCm));
		TestFalse(TEXT("and one a centimetre above it is not"),
			LLKClimbFeel::IsClimbable(LLKLanternFeel::KeeperEyeHeightCm + 1.f, StepHeightCm));
	}

	// ---- 3. Small steps are the walk's job, not the climb's -------------
	{
		TestFalse(TEXT("a kerb the walk can step over does not trigger a mantle"),
			LLKClimbFeel::IsClimbable(StepHeightCm - 1.f, StepHeightCm));
		TestFalse(TEXT("nor does a step exactly at the walk's limit"),
			LLKClimbFeel::IsClimbable(StepHeightCm, StepHeightCm));
		TestTrue(TEXT("but one just above it does"),
			LLKClimbFeel::IsClimbable(StepHeightCm + 1.f, StepHeightCm));
		TestFalse(TEXT("and flat floor never does"),
			LLKClimbFeel::IsClimbable(0.f, StepHeightCm));
	}

	// ---- 4. Up, then across ---------------------------------------------
	//
	// The rule that keeps the capsule out of the counter it is climbing.
	{
		const FVector Start(0.f, 0.f, 0.f);
		const FVector Ledge(100.f, 0.f, 90.f);

		const FVector Half = LLKClimbFeel::MantlePosition(Start, Ledge, 0.5f);
		TestEqual(TEXT("halfway through, the keeper has risen to full height"),
			static_cast<float>(Half.Z), 90.f, 0.01f);
		TestEqual(TEXT("and has not moved forward at all yet"),
			static_cast<float>(Half.X), 0.f, 0.01f);

		const FVector Quarter = LLKClimbFeel::MantlePosition(Start, Ledge, 0.25f);
		TestTrue(TEXT("a quarter through, it is rising and still not advancing"),
			Quarter.Z > 0.f && FMath::IsNearlyZero(Quarter.X, 0.01f));

		TestEqual(TEXT("the start is the start"),
			LLKClimbFeel::MantlePosition(Start, Ledge, 0.f), Start);
		TestEqual(TEXT("and the end is the ledge"),
			LLKClimbFeel::MantlePosition(Start, Ledge, 1.f), Ledge);

		// Out-of-range alphas clamp rather than extrapolate. A hitch that
		// produced an alpha of 1.4 would otherwise throw the keeper past the
		// ledge and off the far side.
		TestEqual(TEXT("an overshooting alpha clamps to the ledge"),
			LLKClimbFeel::MantlePosition(Start, Ledge, 1.4f), Ledge);
		TestEqual(TEXT("and a negative one clamps to the start"),
			LLKClimbFeel::MantlePosition(Start, Ledge, -0.3f), Start);
	}

	// ---- 5. It takes long enough to be a commitment ---------------------
	{
		const FLLKStoreFloorPlan Plan;
		const float CounterSeconds = LLKClimbFeel::ClimbSeconds(
			LLKUnits::DistanceFromStuds(Plan.CounterHeightStuds));

		TestTrue(TEXT("getting onto a counter takes the better part of a second"),
			CounterSeconds > 0.5f && CounterSeconds < 1.5f);
		TestTrue(TEXT("and a taller ledge takes longer than a shorter one"),
			LLKClimbFeel::ClimbSeconds(120.f) > LLKClimbFeel::ClimbSeconds(60.f));
		TestEqual(TEXT("and nothing takes no time"),
			LLKClimbFeel::ClimbSeconds(0.f), 0.f);
	}

	// ---- 6. The acoustic vocabulary, in order ---------------------------
	//
	// walk 5 < climb < sprint 12.5 < push 16. Each verb has to be
	// distinguishable by ear or the player cannot judge what they just did.
	{
		TestTrue(TEXT("climbing is louder than walking"),
			LLKClimbFeel::ClimbNoiseStuds > 5.f);
		TestTrue(TEXT("and quieter than sprinting"),
			LLKClimbFeel::ClimbNoiseStuds < 12.5f);
		TestTrue(TEXT("and much quieter than shoving a fixture"),
			LLKClimbFeel::ClimbNoiseStuds < LLKPushFeel::PushNoiseStuds);

		// And it is not silent, which is the defect this constant exists to
		// prevent: a mantle moves the capsule directly, so velocity-derived
		// noise would be zero for the whole climb.
		TestTrue(TEXT("and a climb is never silent"),
			LLKClimbFeel::ClimbNoiseStuds > 0.f);
	}

	return true;
}

#endif

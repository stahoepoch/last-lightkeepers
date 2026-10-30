// Chris's three guarantees, asserted rather than intended. M2.5 feel slice.
//
// He found the constraint before anybody made the lantern feel good and behave
// wrong: the light component is both the visible cone and the position
// `ULLKStormChargeSubsystem::GetLitLightSources` reports into
// `LLKLightField`. Sway the component and the player sees one light position
// while the server judges another - and that position decides storm accrual,
// relic legibility, and what the Warden converges on.
//
// So the three things he asked to be guaranteed, in his order:
//
//   1. **No camera/input lag.** The look is an input to the filter and never
//      an output of it. Partly a claim about a signature, which is why the
//      assertion below is about what `AdvanceFeel` can and cannot return.
//   2. **The lantern never crosses blocking geometry.** The offset is clamped
//      against a measured free distance, and an unmeasured frame collapses to
//      the anchor rather than guessing.
//   3. **Cosmetic sway cannot change storm, relic or enemy logic.** The
//      gameplay anchor is a function of the capsule and an authored constant
//      and takes no feel state as an argument, so this is guaranteed by
//      construction. The test is here anyway, because "by construction" is a
//      claim about a signature somebody can widen.
//
// Plus the failures a feel filter actually ships with, which are not in his
// list because they are mine to worry about: a filter that explodes on a
// hitch, one that drifts because it reads its own output, and one whose
// amplitude is frame-rate dependent so it feels different on his machine than
// on a tester's.

#include "Lantern/LLKLanternFeel.h"

#include "Misc/AutomationTest.h"

#include <limits>

#if WITH_DEV_AUTOMATION_TESTS

namespace LLKLanternFeelTestFixture
{
	/** The authored defaults, so the test moves when Chris retunes. */
	FLLKLanternFeelRules Rules()
	{
		return FLLKLanternFeelRules();
	}

	FLLKLanternFeelInput Walking(float DeltaSeconds)
	{
		FLLKLanternFeelInput Input;
		Input.DeltaSeconds = DeltaSeconds;
		Input.SpeedCm = 448.f; // 16 studs/s, the authored walk speed
		Input.FreeDistanceCm = 1000.f; // nothing in the way
		return Input;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKLanternFeelTest,
	"LLK.Lantern.Feel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKLanternFeelTest::RunTest(const FString& Parameters)
{
	using namespace LLKLanternFeelTestFixture;

	const FLLKLanternFeelRules R = Rules();

	// ---- GUARANTEE 3: no cosmetic value can reach the gameplay anchor ----
	{
		const FVector Capsule(1200.f, -400.f, 90.f);
		const FVector Anchor = LLKLanternFeel::GameplayLightPosition(
			Capsule, LLKLanternFeel::KeeperEyeHeightCm);

		TestEqual(TEXT("the gameplay anchor is the capsule plus the authored eye height"),
			Anchor, FVector(1200.f, -400.f, 90.f + 152.f));

		// Run the filter hard for two seconds and assert the anchor is
		// untouched. It cannot be otherwise - GameplayLightPosition takes no
		// feel state - and that is exactly the claim: the only way to break
		// guarantee 3 is to widen this signature, and then this test stops
		// compiling rather than silently passing.
		FLLKLanternFeelState State;
		for (int32 Frame = 0; Frame < 120; ++Frame)
		{
			FLLKLanternFeelInput Input = Walking(1.f / 60.f);
			Input.Look = FRotator(Frame * 3.f, Frame * 7.f, 0.f);
			Input.LandingImpactSpeed = Frame % 20 == 0 ? 900.f : 0.f;
			State = LLKLanternFeel::AdvanceFeel(State, Input, R);
		}

		TestTrue(TEXT("GUARANTEE 3: after 120 frames of sway, trail and landing kicks, "
			"the gameplay anchor has not moved"),
			LLKLanternFeel::GameplayLightPosition(Capsule, LLKLanternFeel::KeeperEyeHeightCm)
				.Equals(Anchor));
		TestTrue(TEXT("and the cosmetic offset is not zero, so the test above is not "
			"passing because nothing happened"),
			!State.OffsetCm.IsNearlyZero());
	}

	// ---- GUARANTEE 1: the look is an input, never an output --------------
	{
		// The filter returns a state containing a TRAIL, which is where the
		// cone is - not where the camera is. There is no field on
		// FLLKLanternFeelState that a camera could be driven from, and the
		// trail always lags the live look rather than replacing it.
		FLLKLanternFeelState State;
		const FRotator Look(0.f, 90.f, 0.f);

		FLLKLanternFeelInput Input = Walking(1.f / 60.f);
		Input.Look = Look;
		State = LLKLanternFeel::AdvanceFeel(State, Input, R);

		const FRotator Lag = (Look - State.TrailDegrees).GetNormalized();
		TestTrue(TEXT("GUARANTEE 1: the cone lags the look on the frame the look moves"),
			FMath::Abs(Lag.Yaw) > 0.01f);
		TestTrue(TEXT("and never by more than the authored maximum, however far the look jumps"),
			FMath::Abs(Lag.Yaw) <= R.MaxTrailDegrees + KINDA_SMALL_NUMBER);

		// A 180-degree snap, the worst case a mouse can produce in one frame.
		FLLKLanternFeelInput Snap = Walking(1.f / 60.f);
		Snap.Look = FRotator(0.f, -90.f, 0.f);
		const FLLKLanternFeelState Snapped = LLKLanternFeel::AdvanceFeel(State, Snap, R);
		const FRotator SnapLag = (Snap.Look - Snapped.TrailDegrees).GetNormalized();
		TestTrue(TEXT("a 180-degree flick still leaves the cone within the trail limit, "
			"so the light cannot be left pointing behind the keeper"),
			FMath::Abs(SnapLag.Yaw) <= R.MaxTrailDegrees + KINDA_SMALL_NUMBER);
	}

	// ---- GUARANTEE 2: the cone cannot cross blocking geometry -----------
	{
		const FVector Desired(0.f, 10.f, 0.f);

		// Pressed against something 8cm away, with 6cm of authored clearance.
		const FVector Clamped = LLKLanternFeel::ClampedOffset(Desired, 8.f, R);
		TestTrue(TEXT("GUARANTEE 2: an offset longer than the free distance is shortened"),
			Clamped.Size() <= 8.f - R.WallClearanceCm + KINDA_SMALL_NUMBER);
		TestTrue(TEXT("and keeps its direction, so the cone eases back rather than snapping"),
			FVector::DotProduct(Clamped.GetSafeNormal(), Desired.GetSafeNormal()) > 0.99f);

		// Flush against a wall. Zero room, zero offset - not a negative one.
		const FVector Flush = LLKLanternFeel::ClampedOffset(Desired, 2.f, R);
		TestTrue(TEXT("less room than the clearance gives no offset at all, never a "
			"negative one that would push the cone through the wall"),
			Flush.IsNearlyZero());

		// Unmeasured. The conservative direction.
		const FVector Unmeasured = LLKLanternFeel::ClampedOffset(Desired, -1.f, R);
		TestTrue(TEXT("an unmeasured frame collapses to the anchor rather than guessing, "
			"which is the same direction ELLKVisibility::Unknown resolves in"),
			Unmeasured.IsNearlyZero());

		// Plenty of room: the offset survives, or the clamp is just an off
		// switch and none of the above proves anything.
		const FVector Roomy = LLKLanternFeel::ClampedOffset(Desired, 500.f, R);
		TestTrue(TEXT("and with room to spare the offset is kept, so the clamp is not "
			"simply disabling the feature"), Roomy.Equals(Desired, 0.01f));
	}

	// ---- the hard rail, under inputs no frame would produce -------------
	{
		const FVector Absurd(10000.f, -50000.f, 99999.f);
		const FVector Clamped = LLKLanternFeel::ClampedOffset(Absurd, 1.0e9f, R);
		TestTrue(TEXT("no free distance, however large, lets the offset exceed MaxOffsetCm"),
			Clamped.Size() <= R.MaxOffsetCm + KINDA_SMALL_NUMBER);

		const FVector NotANumber(
			std::numeric_limits<float>::quiet_NaN(), 0.f,
			std::numeric_limits<float>::infinity());
		const FVector Sanitised = LLKLanternFeel::ClampedOffset(NotANumber, 100.f, R);
		TestTrue(TEXT("a NaN or infinite offset is sanitised rather than propagated into "
			"a transform, where it would teleport the light and log nothing useful"),
			!Sanitised.ContainsNaN() && Sanitised.Size() <= R.MaxOffsetCm + KINDA_SMALL_NUMBER);
	}

	// ---- a hitch must not explode the filter ---------------------------
	{
		FLLKLanternFeelState State;
		State.KickCm = 7.f;

		FLLKLanternFeelInput Hitch = Walking(2.5f); // a two-and-a-half second frame
		Hitch.LandingImpactSpeed = 5000.f;
		State = LLKLanternFeel::AdvanceFeel(State, Hitch, R);

		TestTrue(TEXT("a 2.5-second frame leaves the offset inside its bounds"),
			State.OffsetCm.Size() <= R.MaxOffsetCm + KINDA_SMALL_NUMBER);
		TestFalse(TEXT("and does not leave NaN in the state"), State.OffsetCm.ContainsNaN());

		FLLKLanternFeelInput Negative = Walking(-1.f);
		const FLLKLanternFeelState Unmoved = LLKLanternFeel::AdvanceFeel(State, Negative, R);
		TestTrue(TEXT("a negative delta advances nothing rather than running time backwards"),
			Unmoved.SwayPhase == State.SwayPhase);
	}

	// ---- frame-rate independence, which is why the approach is exponential
	{
		// The same second of walking, at 30fps and at 144fps. If these
		// diverge, feel tuned on Chris's machine is a different game on a
		// tester's - and the usual cause is a `Delta * Rate` lerp.
		FLLKLanternFeelState Slow;
		for (int32 Frame = 0; Frame < 30; ++Frame)
		{
			FLLKLanternFeelInput Input = Walking(1.f / 30.f);
			Input.Look = FRotator(0.f, 45.f, 0.f);
			Slow = LLKLanternFeel::AdvanceFeel(Slow, Input, R);
		}

		FLLKLanternFeelState Fast;
		for (int32 Frame = 0; Frame < 144; ++Frame)
		{
			FLLKLanternFeelInput Input = Walking(1.f / 144.f);
			Input.Look = FRotator(0.f, 45.f, 0.f);
			Fast = LLKLanternFeel::AdvanceFeel(Fast, Input, R);
		}

		TestTrue(TEXT("one second of walking lands the trail in the same place at 30fps "
			"and at 144fps"),
			FMath::Abs((Slow.TrailDegrees - Fast.TrailDegrees).GetNormalized().Yaw) < 0.5f);
	}

	// ---- and the sway is evidence of walking, not a wobble --------------
	{
		FLLKLanternFeelState Still;
		for (int32 Frame = 0; Frame < 60; ++Frame)
		{
			FLLKLanternFeelInput Input = Walking(1.f / 60.f);
			Input.SpeedCm = 0.f;
			Still = LLKLanternFeel::AdvanceFeel(Still, Input, R);
		}

		TestTrue(TEXT("a standing keeper's lantern does not sway - a time-driven sway on a "
			"stationary player reads as a wobbling camera rather than as a body"),
			Still.OffsetCm.IsNearlyZero());

		FLLKLanternFeelState Sprinting;
		FLLKLanternFeelState Walked;
		float SprintPeak = 0.f;
		float WalkPeak = 0.f;
		for (int32 Frame = 0; Frame < 120; ++Frame)
		{
			FLLKLanternFeelInput Run = Walking(1.f / 60.f);
			Run.SpeedCm = 644.f; // 23 studs/s, the authored sprint speed
			Run.bSprinting = true;
			Sprinting = LLKLanternFeel::AdvanceFeel(Sprinting, Run, R);
			SprintPeak = FMath::Max(SprintPeak, Sprinting.OffsetCm.Size());

			Walked = LLKLanternFeel::AdvanceFeel(Walked, Walking(1.f / 60.f), R);
			WalkPeak = FMath::Max(WalkPeak, Walked.OffsetCm.Size());
		}

		TestTrue(TEXT("and a sprint swings it further than a walk"), SprintPeak > WalkPeak);
		TestTrue(TEXT("while still obeying the hard rail"), SprintPeak <= R.MaxOffsetCm);
	}

	return true;
}

#endif

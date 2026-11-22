// What a child can climb onto, and what it costs.
//
// **This is the verb the store was already shaped for.** `LLKStoreFloorPlan`
// authors counters at `CounterHeightStuds = 3.2` (89.6 cm) and shelves at
// `ShelfHeightStuds = 6` (168 cm). The keeper is 145 cm tall with their eye at
// 125 cm. Nobody designed those three numbers as a puzzle and together they
// are one: **a counter is something you can get on top of and a shelf is not**,
// everywhere in Boswell's, in geometry that is already placed.
//
// That is why this is worth building before anything is authored for it. The
// push verb needs a fixture somebody puts in a level; this needs a counter,
// and the store is full of them.
//
// **The rule a player learns in one attempt: you can climb onto what you can
// see the top of.** The ceiling is the keeper's own eye height, and that is a
// derivation rather than a number - `LLKLanternFeel` computes the eye from the
// capsule and the camera offset, both of which come from the single authored
// 145 cm, so a keeper rescale carries this with it. Against the floor plan:
//
//   - a counter at 89.6 cm is well under the 125 cm eye - **climbable**
//   - a shelf at 168 cm is far over it - **not**, and the answer to a shelf
//     is to push something to the foot of it and climb that instead
//
// It is deliberately generous. A real eleven-year-old mantling something at
// eye height is working hard, and a stricter ceiling - shoulder height, which
// would be `KeeperCapsuleHalfHeightCm + KeeperCapsuleRadiusCm` and about
// 100 cm - is also derived and would also sort counters from shelves. The
// generous one wins because this verb has no tutorial: a player who can see a
// surface and is told they can climb what they can see will try it once and
// learn the rule. One constant if it reads as too easy in play.
//
// **The floor of the range is the movement component's own step height**, not
// a number chosen here. Anything the walk already steps over must not trigger
// a mantle, or every kerb and cable tray in the store becomes a 1.1-second
// animation the player did not ask for.
//
// **Climbing is slow and loud, which is the whole cost.** Same economy as the
// pushable: not difficult, just a commitment of time during which something
// can find you. A keeper halfway onto a counter is the most helpless they ever
// are, and `LLKHearing` is what makes the exposure real rather than stated.

#pragma once

#include "CoreMinimal.h"
#include "Lantern/LLKLanternFeel.h"
#include "LLKUnits.h"

namespace LLKClimbFeel
{
	/**
	 * How fast the scramble goes, in studs of height per second.
	 *
	 * 4 studs/s puts a 125 cm mantle at about 1.1 seconds and a 90 cm counter
	 * at 0.8. Authored, not derived - it is a feel value, and the thing it has
	 * to be is slow enough that starting one while a machine is in the aisle
	 * is the wrong call.
	 */
	inline constexpr float ClimbStudsPerSecond = 4.f;

	/**
	 * How loud a scramble is, in studs of carry.
	 *
	 * Above a walk (5) and below a sprint (12.5). **You cannot climb
	 * quietly**, because hauling yourself onto a steel counter is not a quiet
	 * thing to do, but it is not a fixture grinding across tile either
	 * (`LLKPushFeel::PushNoiseStuds` is 16). The ordering walk < climb <
	 * sprint < push is the whole acoustic vocabulary of the chapter and this
	 * is its third entry.
	 */
	inline constexpr float ClimbNoiseStuds = 8.f;

	/**
	 * The highest surface this keeper can get onto, in cm above their feet.
	 *
	 * Their eye height, derived from the capsule and camera offset rather
	 * than restated, so the 145 cm authored keeper remains the only place a
	 * body measurement lives.
	 */
	inline float MaxLedgeHeightCm()
	{
		return LLKLanternFeel::KeeperEyeHeightCm;
	}

	/**
	 * Is a ledge at this height worth a mantle.
	 *
	 * `StepHeightCm` is the movement component's own `MaxStepHeight`, passed
	 * in rather than assumed: anything the walk clears by itself is not a
	 * climb, and hard-coding a floor here would be a second opinion about
	 * what walking can do.
	 */
	inline bool IsClimbable(const float LedgeHeightCm, const float StepHeightCm)
	{
		return LedgeHeightCm > StepHeightCm && LedgeHeightCm <= MaxLedgeHeightCm();
	}

	/** How long the scramble onto a ledge this high takes, in seconds. */
	inline float ClimbSeconds(const float LedgeHeightCm)
	{
		const float Studs = FMath::Max(0.f, LedgeHeightCm) / LLKUnits::StudsToCm;
		return Studs / ClimbStudsPerSecond;
	}

	/**
	 * Where the capsule is, partway through a mantle.
	 *
	 * `Alpha` is 0 at the start and 1 on the ledge. **Up first, then across**,
	 * which is what a body does and what keeps the capsule out of the thing
	 * being climbed: interpolating straight to the destination would drag it
	 * through the counter's front face for the first half of the move, and
	 * any collision there either stops the mantle or pops the keeper out
	 * sideways.
	 *
	 * The vertical half completes by `Alpha` 0.5 and the horizontal half
	 * starts there, so the keeper rises clear of the lip before moving over
	 * it. Pure, so the path can be asserted without a capsule.
	 */
	inline FVector MantlePosition(
		const FVector& Start, const FVector& Ledge, const float Alpha)
	{
		const float Clamped = FMath::Clamp(Alpha, 0.f, 1.f);
		const float Rise = FMath::Clamp(Clamped * 2.f, 0.f, 1.f);
		const float Across = FMath::Clamp(Clamped * 2.f - 1.f, 0.f, 1.f);

		return FVector(
			FMath::Lerp(Start.X, Ledge.X, Across),
			FMath::Lerp(Start.Y, Ledge.Y, Across),
			FMath::Lerp(Start.Z, Ledge.Z, Rise));
	}
}

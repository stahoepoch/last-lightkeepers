// What a haul costs to carry. M3.
//
// **This was authored and unread.** `loot.json` has carried
// `burden.speedPenaltyPerPoint` and `burden.maxSpeedPenalty` since the Roblox
// project, `FLLKBurdenRules` has had both fields since M0.5, and nothing in
// the engine ever looked at either: carrying four relics cost exactly nothing.
// That is the half of row 8 that makes extraction a decision rather than a
// formality - *"we have enough - call the loud extraction now, defend it, or
// risk one more store?"* is not a question if the haul is weightless.
//
// **Chris's note in the data is the whole design, and it is narrower than the
// key names suggest:** *"ordinary loot must not make a raid feel sluggish.
// Only relics impose a small movement penalty; supplies, components, and
// restoration crates do not."* So the points are relics, not items. One relic
// is one point today, because `ULLKInteractorComponent::GetCarriedCount` is a
// count of relics taken and nothing authors a per-relic weight. If a heavy
// relic ever wants to be two points, it belongs in the relic's own data and
// this function already takes points rather than a count for that reason.
//
// **What the authored numbers mean in play.** 0.04 per point against a cap of
// 0.12: one relic is 4% slower, two 8%, three 12%, and the fourth and every
// one after it is free. That cap is the interesting part - it means a crew can
// always choose to be greedy without becoming unable to run, and it is why
// burden is a nudge rather than a leash. Both numbers are his (row 55).
//
// **Light is not burden**, and the guard says so because he was explicit:
// *"Do not add a lantern speed penalty yet."* Holding a lantern costs
// visibility and machine attention (rows 72 and 73), never speed. Nothing in
// here takes a lantern, a light level or a storm value, and
// `tests/burden.test.mjs` fails if the movement path ever reads one.

#pragma once

#include "CoreMinimal.h"
#include "Data/LLKLootRulesData.h"

namespace LLKBurden
{
	/**
	 * How much slower, as a fraction in [0, 1). 0 when carrying nothing.
	 *
	 * Clamped twice on purpose: once at the authored cap, which is the design,
	 * and once at a hard rail of 0.9, which is not. A `maxSpeedPenalty` of 1.0
	 * typed into the data by accident would otherwise produce a keeper who
	 * cannot move at all while holding a brooch - a bug that reads as a frozen
	 * character rather than as bad data, and the kind this project would spend
	 * an evening on.
	 */
	LLK_API float PenaltyFraction(int32 Points, const FLLKBurdenRules& Rules);

	/** What to multiply an authored speed by. `1 - PenaltyFraction`. */
	LLK_API float SpeedMultiplier(int32 Points, const FLLKBurdenRules& Rules);

	/**
	 * The point at which carrying more is free, or 0 when it never is.
	 *
	 * For the readout, so a player can be told that the next relic is free
	 * rather than discovering it by feel. Row 5's principle applied to weight:
	 * show the cause beside the consequence, and the cap is part of the cause.
	 */
	LLK_API int32 PointsUntilCapped(const FLLKBurdenRules& Rules);
}

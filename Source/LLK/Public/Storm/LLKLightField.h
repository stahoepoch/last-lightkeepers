// Where the brightest place is. The rule the whole tutorial rests on.
//
// **Enemies converge on the brightest position, never on the nearest player.**
// That is locked, it is stated in `data/enemies.json` as the Warden's own
// behaviour, and `docs/THE-STORE.md` explains why it carries more weight than
// it looks like it should: it is what makes the store teach.
//
// The argument, because it decides the shape of this file. In four-player
// co-op the default outcome is that the player who lights the lantern and the
// player who meets the enemy are **different people**. One got a relic and no
// consequence; the other got a consequence and no cause. Play it ten times and
// the crew still cannot tell you what the lantern does, because nobody ever
// held both halves. Converging on the brightest position fixes that by
// construction: the keeper who lit up is the keeper who gets found. Cause and
// consequence land on the same pair of hands.
//
// The archived Roblox `EnemyService` retargeted the Warden to the **nearest**
// player, which looked like a fidelity bug for months. It is not. It is the
// bug that would stop the building teaching, and `THE-STORE.md` says plainly
// that the UE5 build must not inherit it. There is no enemy code in UE5 yet,
// so this exists now to make sure there is nothing to inherit it *from* - the
// seam is here before the first consumer, and it has only one answer.
//
// **Brightness is summed, which is physics rather than a design call.** Two
// keepers standing together genuinely are brighter than one, so a huddled pair
// outranks a lone lit keeper without anybody having to decide that they
// should. That matters: it means "the brightest position" needs no tie-break
// policy invented for it, and a crew's instinct to bunch up when frightened
// has a mechanical consequence that follows from the lights themselves.
//
// **Candidates are the sources' own positions.** The true maximum of a summed
// falloff field can sit between two lights, and deliberately is not searched
// for: an enemy converging on a point in mid-air between two keepers would be
// converging on nobody. Targeting a lit keeper is both the intent of the rule
// and the only reading that produces the lesson.

#pragma once

#include "CoreMinimal.h"

/** One lit lantern, reduced to what the field needs to know about it. */
struct FLLKLightSource
{
	FVector Position = FVector::ZeroVector;

	/** Candelas, from `data/lantern.json` `light.intensity`. */
	float Intensity = 0.f;

	/** Attenuation radius in CENTIMETRES - already converted, row 61. */
	float RadiusCm = 0.f;
};

namespace LLKLightField
{
	/**
	 * Summed brightness at a point, in arbitrary but comparable units.
	 *
	 * `Intensity / (1 + d^2 / r^2)`, summed. Inverse-square in shape, so
	 * distance matters the way light does, but with the `1 +` so that a source
	 * evaluated at its own position is finite rather than a division by zero -
	 * which is the single most likely input, since the candidates ARE the
	 * source positions.
	 *
	 * Sources past their own radius still contribute, by design and by a
	 * vanishing amount. A hard cutoff would make the brightest position jump
	 * discontinuously as a keeper walked a step, and an enemy that changes its
	 * mind on a boundary reads as broken rather than as dangerous.
	 */
	LLK_API float BrightnessAt(const TArray<FLLKLightSource>& Sources, const FVector& Point);

	/**
	 * The brightest source position, or false when nothing is lit.
	 *
	 * False rather than the world origin, because an all-dark crew is the
	 * normal state of this game and `FVector::ZeroVector` is a real place
	 * somebody will eventually stand. An enemy handed (0,0,0) as "the
	 * brightest position" would walk to the middle of the map all night.
	 *
	 * Ties resolve to the lowest index, so the answer is deterministic for
	 * identical inputs - a server and a client must not disagree about who is
	 * being hunted, and neither must two consecutive frames.
	 */
	LLK_API bool FindBrightestPosition(
		const TArray<FLLKLightSource>& Sources, FVector& OutPosition, float& OutBrightness);
}

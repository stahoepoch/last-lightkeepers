// The one place the project converts between the units `data/*.json` is
// written in and the units Unreal works in.
//
// **Every spatial number in `data/` is in Roblox studs**, because that is
// where this project started and `docs/PLATFORM.md` row 33 keeps the data
// layer portable rather than re-authoring it per engine. Unreal works in
// centimetres. Nothing documented the gap until M1.1 needed it, and the
// failure it causes is silent: `walkSpeed: 16` read straight into
// `MaxWalkSpeed` is 16 cm/s, a tenth of walking pace, which reads as "the
// character controller is broken" rather than "the units are wrong".
//
// The giveaway that settles the interpretation: **16 is Roblox's default
// humanoid WalkSpeed.** `lantern.json` is carrying engine defaults from the
// engine it was authored in, not a number anyone chose in centimetres.
//
// Converted here rather than in `data/`, deliberately. The JSON is asserted
// on by 98 balance tests and is the thing that survived one engine change
// already; re-authoring it in centimetres would invalidate those tests and
// trade a portable layer for a convenient one. Whether to re-author anyway
// is Chris's call - every value in those files is - and it is raised as an
// open question rather than decided here.

#pragma once

#include "CoreMinimal.h"

namespace LLKUnits
{
	/**
	 * One Roblox stud in Unreal centimetres.
	 *
	 * 1 stud = 0.28 m is the standard figure. Sanity, with it applied:
	 * walk 16 -> 448 cm/s, sprint 23 -> 644, crouch 9 -> 252, against
	 * Unreal's own default walk of 600. Those are believable numbers for a
	 * game about moving carefully in the dark, which is the cheap check that
	 * the factor is right rather than merely plausible.
	 *
	 * This is a scale, not a tuning value. If the keeper feels wrong, the
	 * answer is the speeds in `data/lantern.json`, not this constant.
	 */
	inline constexpr float StudsToCm = 28.f;

	/** Studs per second to Unreal centimetres per second. */
	inline constexpr float SpeedFromStuds(float Studs)
	{
		return Studs * StudsToCm;
	}

	/** A distance in studs to Unreal centimetres. Ranges, radii, reach. */
	inline constexpr float DistanceFromStuds(float Studs)
	{
		return Studs * StudsToCm;
	}
}

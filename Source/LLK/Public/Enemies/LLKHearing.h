// What the keeper's own movement sounds like, and how far it carries.
//
// **The defect this answers, and it is the mirror of the one fixed yesterday.**
// `LLKSweeperPresence` gave the machine a voice because a player cannot choose
// against something they cannot hear. Measuring the other direction found the
// same hole pointing the other way: `FLLKSweeperSenses` carries `Notice`
// (sight, via `LLKPerception`), `MisplacedItemPosition` (its stocking job) and
// `bAtPatrolPoint`. There is no sense of sound anywhere in it. **Nothing in
// this game can hear the player.**
//
// So stealth had one half. You could be seen, and the lantern, the light field,
// the aggro radii and five visibility rules all exist to make being seen
// interesting. Being *heard* was not modelled at all, which means:
//
//   - Sprinting is free. Its only cost is that a sprint across a lit aisle is
//     a sprint somebody can see, and in the dark it costs nothing whatsoever.
//   - Crouch is a visibility tool and nothing else, so the deliberate slow
//     mode `data/lantern.json` describes as "for tight shops and careful
//     approach" buys approach but not quiet.
//   - `ROADMAP.md` slice 3 asks a fresh player to "choose hide, detour or
//     sprint". Two of those three are the same choice when sound is free.
//   - No puzzle can ever cost anything acoustically, which is most of what
//     makes an environmental puzzle frightening to solve.
//
// The project's recurring failure shape is a correct system over an absent
// thing. This is a near relative: a correct system over *half* a thing, where
// the missing half is the one that would have made the present half a choice.
//
// **The radius is one second of travel, and that is deliberately not a new
// authored number.** How far you can be heard is how fast you are moving:
//
//     RadiusStuds = (speed in cm/s / StudsToCm) x SecondsOfTravel
//
// At `SecondsOfTravel = 1` and the pace Chris authored in `data/lantern.json`
// - crouch 3, walk 5, sprint 12.5 studs per second - that gives a crouch that
// carries 3 studs, a walk 5, and a sprint 12.5. Set against the Sweeper's
// `aggroRadiusDark` of 12 in `data/enemies.json`:
//
//   - **Crouching is genuinely quiet.** 3 studs is a quarter of the radius in
//     which that machine notices anything at all in the dark.
//   - **Walking is quiet enough to work with.** 5 studs still sits well
//     inside the dark notice radius, so walking is not a confession.
//   - **Sprinting is a decision.** 12.5 studs carries marginally further than
//     the machine can see in the dark. Running past a Sweeper in an unlit
//     aisle is now roughly as findable as standing in front of it.
//
// That last line is the mechanic, and it fell out of Chris's own numbers
// rather than being tuned into them. The `aggroRadiusLit` of 38 is three times
// the loudest noise a keeper can make, so **sound never exceeds sight in the
// light** - light remains the dominant way to be found, which is the game's
// whole thesis and must not be quietly overturned by a new sense.
//
// It also rescales itself. `data/lantern.json` has been globally rescaled
// twice (halved on 2026-11-04, then the child-scale pass on 2026-11-21) and
// `enemies.json` carries a `_retuneNote` about every speed moving together
// because "a chase is a ratio". A noise radius derived from speed survives the
// next rescale without a note. An authored `sprintNoiseStuds` would not, and
// would be the third number somebody has to remember to move.
//
// **Velocity, not gait.** The speed read is the keeper's actual ground speed,
// not the nominal speed of their movement mode, and that matters three times
// over: burden and stumble already multiply it at `LLKCharacter.cpp`, a keeper
// pushed to a crawl is correctly quieter, and **a keeper standing still makes
// no sound at all.** Stopping is the verb this gives the player, and it only
// exists because the input is how fast they are really going.
//
// **What this deliberately does NOT do: occlusion.** Same honesty as
// `LLKPerception::SenseReaching`, and for the same reason. This takes no
// `UWorld` and traces nothing, so a noise is heard through a gondola exactly
// as well as across an open aisle. `NoiseReaching` is named for what it
// answers - the sound got that far - which is necessary and not sufficient. A
// function called `WouldHear` that ignores walls is a function somebody will
// trust through a wall.

#pragma once

#include "CoreMinimal.h"
#include "LLKUnits.h"

namespace LLKHearing
{
	/**
	 * How many seconds of travel a keeper's noise carries.
	 *
	 * The one constant here, and it is the whole tuning surface. Raising it
	 * makes every gait louder in the same proportion, which keeps the gait
	 * relationships that `data/lantern.json`'s ratios were chosen for. The
	 * number to change if sound should matter more or less; the pace values
	 * are not it.
	 */
	inline constexpr float SecondsOfTravel = 1.f;

	/**
	 * A sound in the world, as the thing that hears it needs to know it.
	 *
	 * Deliberately not "the keeper's position and speed". A noise is an event
	 * at a place with a reach, which is the shape a thrown bottle, a breaker
	 * thrown in the service core or a dropped relic also has. The brain and
	 * this module only ever see noises, so adding a world noise source later
	 * changes who fills the senses and nothing else.
	 */
	struct FLLKNoise
	{
		/** Where the sound happened, in Unreal centimetres. */
		FVector Position = FVector::ZeroVector;

		/** How far it carries, in studs. Zero is silence. */
		float RadiusStuds = 0.f;
	};

	/**
	 * The noise a keeper makes by moving at this speed.
	 *
	 * `SpeedCms` is ground speed in Unreal centimetres per second - normally
	 * `GetVelocity().Size2D()`. Vertical motion is excluded by the caller on
	 * purpose: a keeper falling down an escalator well is not making a noise
	 * proportional to the fall, and a landing is a one-shot this does not
	 * model.
	 */
	inline FLLKNoise NoiseFromMovement(const FVector& Position, const float SpeedCms)
	{
		FLLKNoise Noise;
		Noise.Position = Position;
		Noise.RadiusStuds = FMath::Max(0.f, SpeedCms) / LLKUnits::StudsToCm * SecondsOfTravel;
		return Noise;
	}

	/**
	 * Did this noise reach a listener standing there.
	 *
	 * A strict comparison, so a silent noise (radius 0) reaches nothing at
	 * all - including a listener standing on the exact spot, which is the
	 * case a `<=` would get wrong and which is precisely the case that
	 * matters: a keeper who has stopped moving, with a Sweeper beside them,
	 * must be inaudible. Being found at that point is sight's job.
	 */
	inline bool NoiseReaching(const FLLKNoise& Noise, const FVector& ListenerPosition)
	{
		if (Noise.RadiusStuds <= 0.f)
		{
			return false;
		}

		const float ReachCm = LLKUnits::DistanceFromStuds(Noise.RadiusStuds);
		return FVector::DistSquared(Noise.Position, ListenerPosition) <= ReachCm * ReachCm;
	}

	/** How far the listener is from the noise, in studs. For the senses. */
	inline float DistanceStuds(const FLLKNoise& Noise, const FVector& ListenerPosition)
	{
		return FVector::Dist(Noise.Position, ListenerPosition) / LLKUnits::StudsToCm;
	}
}

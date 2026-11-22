// How loud a Sweeper is, and at what rate, for the state it is in.
//
// **The defect this answers.** `ALLKSweeper` fired one-shot cues at a fixed
// location and was otherwise silent. Measured on `LLK_Boswells_OpeningPass`,
// a Sweeper that notices the keeper walks 11962 cm of path - 71.2 s at
// 168 cm/s - and in all of it makes no sound. `DECISIONS.md` row 16 asks for
// an announcement before sight, and the telegraph cue does satisfy that
// literally. It does not let a player hear a machine *approaching*, and
// `ROADMAP.md` slice 3 asks a fresh player to identify a cue and choose hide,
// detour or sprint. You cannot choose against something you cannot hear.
//
// So the pawn now carries a looping, spatialised motor and the brain's state
// sets its volume and rate. This header is that mapping, as a pure function,
// so the balance can be read and tested without an audio device, a world, or
// a pawn - the same split `LLKLanternFeel` uses.
//
// **Telegraph is silence, and that is the design claim here.** Every other
// state raises the volume as the machine gets more interested. Telegraph
// drops it to nothing. Three reasons, in order of how much they matter:
//
//   1. The telegraph cue has to land. A one-shot played over a running motor
//      at the same location competes with it and a player reads one sound.
//   2. The brain already holds the Sweeper still through the telegraph, and a
//      machine that stopped moving should stop sounding like it is moving.
//      Row 16's second is a real head start; the motor cutting is what makes
//      that second *legible* as a head start rather than dead air.
//   3. A hole in a continuous sound is a stronger cue than a loud sound. A
//      player who has spent a minute hearing the aisles hum learns the hum,
//      and then the hum stops.
//
// If Chris wants the motor to idle through the telegraph instead, it is the
// one line below and `LLKSweeperPresenceTest` names what fails first.
//
// **Why one asset and not five.** Volume and playback rate at the component
// are enough to tell these states apart by ear, and playback rate moves the
// motor fundamental and the castor squeak together, which is what a machine
// actually does when it speeds up. Five recorded states would be five things
// to keep in agreement with one brain.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/LLKSweeperBrain.h"

namespace LLKSweeperPresence
{
	/** What the audio component should be set to this tick. */
	struct FLLKVoice
	{
		/** 0 is silent. Never above 1; the loop is rendered with headroom. */
		float Volume = 0.f;

		/** Playback rate. 1 is the rendered 58 Hz motor and 1.25 Hz castor. */
		float Pitch = 1.f;
	};

	/**
	 * The loop asset's own rate, named so the balance below reads as a
	 * multiple of something real rather than as bare numbers.
	 */
	constexpr float RenderedPitch = 1.f;

	/**
	 * Patrol is the floor's resting sound: audible in the same aisle, lost a
	 * department away. It has to be quiet enough that a player stops noticing
	 * it, because the whole value of Pursue and of the telegraph's hole is
	 * measured against a baseline the ear has stopped attending to.
	 */
	constexpr float PatrolVolume = 0.22f;
	constexpr float PatrolPitch = 0.92f;

	/**
	 * Stock is busier than patrol and must be tellable from it by ear, since
	 * row 47 makes filing the first encounter and a player who cannot hear
	 * the difference cannot know they have been given a free approach.
	 */
	constexpr float StockVolume = 0.34f;
	constexpr float StockPitch = 1.04f;

	/**
	 * Pursue is loud, and deliberately not louder. A motor that drowns the
	 * player's own footsteps takes away the other thing they navigate by,
	 * and a chase where you cannot hear yourself is disorienting rather than
	 * frightening.
	 */
	constexpr float PursueVolume = 0.62f;
	constexpr float PursuePitch = 1.18f;

	/**
	 * Search sits between the two. The machine is moving normally; what makes
	 * it frightening is where it is, not how it sounds, so this stays close
	 * to patrol and lets position carry the tension.
	 */
	constexpr float SearchVolume = 0.40f;
	constexpr float SearchPitch = 1.02f;

	/** Telegraph. See the header comment - this being zero is the point. */
	constexpr float TelegraphVolume = 0.f;

	inline FLLKVoice VoiceFor(const ELLKSweeperState State)
	{
		FLLKVoice Voice;
		switch (State)
		{
		case ELLKSweeperState::Telegraph:
			Voice.Volume = TelegraphVolume;
			Voice.Pitch = PatrolPitch;
			break;

		case ELLKSweeperState::Pursue:
			Voice.Volume = PursueVolume;
			Voice.Pitch = PursuePitch;
			break;

		case ELLKSweeperState::Search:
			Voice.Volume = SearchVolume;
			Voice.Pitch = SearchPitch;
			break;

		case ELLKSweeperState::Stock:
			Voice.Volume = StockVolume;
			Voice.Pitch = StockPitch;
			break;

		case ELLKSweeperState::Patrol:
		default:
			Voice.Volume = PatrolVolume;
			Voice.Pitch = PatrolPitch;
			break;
		}
		return Voice;
	}

	/**
	 * How fast the component is allowed to move toward `VoiceFor`, per second.
	 *
	 * Not a smoothing nicety. A motor that jumps from patrol to pursue in one
	 * tick reads as a cut, and the ear hears a cut as a new sound somewhere
	 * else rather than as the same machine speeding up - which would lose the
	 * only thing this system is for. The glide is fast enough to still be a
	 * reaction: patrol to pursue in about a fifth of a second.
	 *
	 * Telegraph is exempt. Its silence has to be immediate or the cue plays
	 * over a decaying motor and point 1 above is lost.
	 */
	constexpr float VolumeGlidePerSecond = 2.0f;
	constexpr float PitchGlidePerSecond = 1.4f;

	inline FLLKVoice GlideToward(const FLLKVoice& Current, const FLLKVoice& Target,
		const float DeltaSeconds, const bool bImmediate)
	{
		if (bImmediate || DeltaSeconds <= 0.f)
		{
			return Target;
		}
		FLLKVoice Next;
		Next.Volume = FMath::FInterpConstantTo(
			Current.Volume, Target.Volume, DeltaSeconds, VolumeGlidePerSecond);
		Next.Pitch = FMath::FInterpConstantTo(
			Current.Pitch, Target.Pitch, DeltaSeconds, PitchGlidePerSecond);
		return Next;
	}
}

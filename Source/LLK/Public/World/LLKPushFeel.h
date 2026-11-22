// What pushing a heavy thing across a department store floor feels like.
//
// **Deliberately not physics, and that is an engineering decision worth
// defending.** There is no rigid-body simulation anywhere in this runtime:
// `grep -rl "SetSimulatePhysics\|AddImpulse"` over `Source/LLK` returns
// nothing. Opening that door for a puzzle object would mean simulated bodies
// on a replicated pawn's collision, which is where determinism, server
// authority and a testable rule all go at once - and the first thing it would
// cost is the ability to assert any of this without a world.
//
// So a pushable slides along an axis its author chose, between an authored
// start and an authored limit, at an authored speed. That is what Tomb Raider
// did, it is what every block puzzle since has done, and it is legible: the
// player can see where a thing will go before they commit to shoving it. A
// crate that could be nudged anywhere is a crate a player can wedge in a
// doorway and soft-lock the chapter behind.
//
// **Travel is one scalar**, from 0 at the authored start to `TravelStuds` at
// the limit, so the whole state of a pushable is one replicated float. That
// also means it cannot drift off its rail, cannot rotate, and cannot end a
// session somewhere the author never saw.
//
// **It is slow on purpose.** The default is well under a crouch, which is
// already the slowest a keeper moves. The cost of a pushable is not that it
// is hard to push, it is that pushing it takes *time you can be found in* -
// and `LLKHearing` is what makes that true rather than merely stated: a thing
// grinding across tile is the loudest noise in the chapter.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/LLKHearing.h"
#include "LLKUnits.h"

namespace LLKPushFeel
{
	/**
	 * Studs per second a keeper moves a fixture. Slower than a crouch (3).
	 *
	 * A shop fixture full of stock is not something a child moves briskly.
	 * The number that matters is the ratio to the Sweeper's 6 studs/s: at 1.2,
	 * a machine crosses four studs for every one the fixture travels, so
	 * committing to a push while something is on its way is always the wrong
	 * call. That is the decision the object exists to create.
	 */
	inline constexpr float PushSpeedStuds = 1.2f;

	/**
	 * How loud a moving fixture is, in studs of carry.
	 *
	 * **Louder than a sprint**, which is the whole point and the first number
	 * here that is authored rather than derived. `LLKHearing` gives a sprint
	 * 12.5 studs because that is one second of travel; a fixture travels 1.2
	 * studs a second and deriving its noise the same way would make the
	 * loudest object in the store quieter than walking.
	 *
	 * Noise is a property of the sound a thing makes, not of how fast it
	 * moves, and a steel fixture dragging across tile is simply loud. 16
	 * studs puts it above a sprint (12.5) and below the Sweeper's lit sight
	 * (38), so pushing is the most audible thing a player can do and still
	 * never louder than being seen in the light.
	 */
	inline constexpr float PushNoiseStuds = 16.f;

	/**
	 * One deterministic step of a push.
	 *
	 * `AxisDot` is the keeper's facing against the fixture's authored axis:
	 * +1 pushing along it, -1 pushing back. Returns the new travel, clamped
	 * to the rail.
	 *
	 * **The dot is a gate, not a multiplier.** Scaling speed by how squarely
	 * you face the thing reads as the fixture being sticky at angles, which
	 * players interpret as the push being broken rather than as themselves
	 * being off-axis. Past the threshold it moves at full speed; short of it,
	 * not at all.
	 */
	inline float AdvancePush(
		const float CurrentStuds, const float AxisDot, const float DeltaSeconds,
		const float TravelStuds, const float MinAxisDot = 0.5f)
	{
		if (DeltaSeconds <= 0.f || FMath::Abs(AxisDot) < MinAxisDot)
		{
			return FMath::Clamp(CurrentStuds, 0.f, TravelStuds);
		}

		const float Direction = AxisDot > 0.f ? 1.f : -1.f;
		return FMath::Clamp(
			CurrentStuds + Direction * PushSpeedStuds * DeltaSeconds, 0.f, TravelStuds);
	}

	/** Is this fixture actually moving, given what one step did to it. */
	inline bool IsMoving(const float BeforeStuds, const float AfterStuds)
	{
		return !FMath::IsNearlyEqual(BeforeStuds, AfterStuds);
	}

	/**
	 * Which of two audible noises a machine goes to look at.
	 *
	 * **The most salient wins, and salience is excess reach - how far past the
	 * listener the sound carried - not raw loudness and not nearness.**
	 *
	 * Nearest would mean the keeper's own footsteps always beat a fixture
	 * grinding two aisles over, because the keeper doing the pushing is
	 * standing next to it. Loudest alone would mean a pushable anywhere on the
	 * floor outranks a sprint going past the machine's shoulder. Excess asks
	 * the right question: of the sounds that reached me, which one reached me
	 * most emphatically.
	 *
	 * The consequence is the one a player needs, stated at the width it is
	 * actually true - `LLKPushTest` section 6 refuted the wider version:
	 * **a fixture outranks the noise of the person pushing it**, because the
	 * pusher is standing next to the thing, and it outranks a keeper creeping
	 * nearer to the machine. It does NOT outrank a sprint going past that
	 * machine's shoulder, and it should not: a keeper running past is the
	 * more urgent thing in the room, and a rule that sent a machine to look
	 * at furniture instead would be the worse rule.
	 */
	inline float Salience(const LLKHearing::FLLKNoise& Noise, const FVector& ListenerPosition)
	{
		return Noise.RadiusStuds - LLKHearing::DistanceStuds(Noise, ListenerPosition);
	}
}

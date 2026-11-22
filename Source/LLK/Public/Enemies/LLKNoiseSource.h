// Something in the world that is making a sound right now.
//
// **The gap this closes was named the day hearing was built.** `LLKHearing`
// models a noise as a position and a radius precisely so that a thrown bottle,
// a breaker in the service core and a fixture grinding across tile would all
// have the same shape as a keeper's footsteps. Nothing emitted one. Only the
// keeper's own movement made sound, which meant the new sense was purely a
// cost: it could get you found and it could not help you.
//
// A distraction is the other half of a stealth sense, and it is the half that
// turns being heard from a tax into a tool. `docs/FINDINGS-NOTHING-CAN-HEAR-
// THE-PLAYER.md` lists this as the next thing to build, and it is.
//
// **Asked, not pushed.** A listener iterates sources and asks each one; a
// source does not register, queue or broadcast. That is the same choice
// `FindMisplacedItem` and `NearestStockBay` already make in `Observe`, and it
// keeps a noise from outliving the frame that made it - a queued sound that
// nobody drained would have a machine investigating something that stopped
// two minutes ago, which is the bug a player would describe as the game
// cheating.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/LLKHearing.h"
#include "UObject/Interface.h"

#include "LLKNoiseSource.generated.h"

UINTERFACE(MinimalAPI)
class ULLKNoiseSource : public UInterface
{
	GENERATED_BODY()
};

class LLK_API ILLKNoiseSource
{
	GENERATED_BODY()

public:
	/**
	 * The sound this thing is making this frame, if any.
	 *
	 * Returns false for silence rather than a zero-radius noise, so a source
	 * that is quiet costs a listener nothing and can never be selected by a
	 * comparison that forgot to check the radius.
	 */
	virtual bool GetCurrentNoise(LLKHearing::FLLKNoise& OutNoise) const = 0;
};

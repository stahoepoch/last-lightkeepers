// Whether Boswell's has its mains on, and what changes when it does.
//
// Chris, 2026-11-22, after seeing the store lit for the first time: *"What
// would it look like with only emergency lights on. That's the vibe we want.
// Only later can we turn the power on to see everything. So we need both...
// I envision being able to turn on the power at some point that we will need
// to. Even if it is a revisit after later progress."*
//
// So the lighting is not a setting, it is a **state the building is in**, and
// moving it is `ROADMAP.md` chapter goal 5 in its most literal form: *"find a
// story clue, change part of the building, and reach Sayers."* Everything
// before this changed the player (a verb) or an object (a fixture on a rail).
// This changes the building.
//
// **Emergency is the default and always will be.** A chapter that opens with
// the lights on has spent its best moment before the player has done
// anything. `BSW_EMERGENCY_FILL_*` is authored on; `BSW_MAINS_*` is authored
// off; this subsystem only ever moves the second group.
//
// **Found by tag, not by name or by pointer.** `AActor::Tags` carrying
// `LLK_Mains` is what makes the map and the runtime agree without either
// holding a list the other can invalidate - a renamed light stops being a
// mains light, which is correct, and a new one becomes one by being tagged,
// which is also correct. The alternative, a hand-maintained array on a
// manager actor, is a second place for the map to be wrong.
//
// **It is the loudest thing in the game.** A building coming back to life is
// not quiet, and `LLKHearing` is what makes that cost real rather than
// stated: every machine on the floor hears it. Restoring power should be a
// decision a player makes knowing it announces them, which is the same
// economy as the pushable and the climb, one step larger.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "LLKPowerSubsystem.generated.h"

/** The mains changed state. Broadcast on the server after the lights move. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLLKPowerChanged, bool, bPowerOn);

UCLASS()
class LLK_API ULLKPowerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The tag a light must carry to be part of the mains. */
	static const FName MainsTag;

	/** The tag the always-on emergency layer carries. Read for reporting only. */
	static const FName EmergencyTag;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** Is the building's mains supply on. False at the start of every chapter. */
	UFUNCTION(BlueprintPure, Category = "LLK|Power")
	bool IsPowerOn() const { return bPowerOn; }

	/**
	 * Turn the mains on or off, and move every tagged light to match.
	 *
	 * Returns whether anything changed, so a caller can tell "I switched the
	 * power on" from "it was already on" without asking twice and risking a
	 * different answer between the two calls.
	 */
	UFUNCTION(BlueprintCallable, Category = "LLK|Power")
	bool SetPowerOn(bool bOn);

	/** How many mains lights this map has. Zero is a map nobody can light. */
	UFUNCTION(BlueprintPure, Category = "LLK|Power")
	int32 GetMainsLightCount() const { return MainsCount; }

	/** Fires on the server after the lights have moved, never before. */
	UPROPERTY(BlueprintAssignable, Category = "LLK|Power")
	FLLKPowerChanged OnPowerChanged;

	/**
	 * How far the building is heard waking up, in studs.
	 *
	 * **Deliberately the largest noise in the game**, above a pushed fixture's
	 * 16 and well above the Sweeper's 38-stud lit sight, because a whole
	 * floor's lighting striking at once is not a local event - it is the
	 * building itself, and every machine in it should come. Restoring power
	 * has to be a decision rather than a free upgrade, and this is the price.
	 */
	static constexpr float PowerOnNoiseStuds = 70.f;

private:
	bool bPowerOn = false;
	int32 MainsCount = 0;
	int32 EmergencyCount = 0;

	/** Applies `bPowerOn` to every tagged light. The only writer of visibility. */
	void ApplyToLights();
};

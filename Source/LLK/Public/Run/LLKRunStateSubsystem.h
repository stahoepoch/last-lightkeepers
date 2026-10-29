// When the night has nobody left to act. M2.2b.
//
// **This is the condition that makes being down mean anything.** M2.1 built
// the downed state and M2.2 built the ways out of it, and between them they
// left a hole Chris found by asking the obvious question: a solo keeper who
// goes down twice waits forever, because `bleedOutSeconds` is 0, no teammate
// is coming, and the anchor revive is spent. The run never ends. That was
// reachable in the build with two console commands.
//
// The rule that fixes it was already written three times in `lantern.json` -
// *"you keep your loot until the crew wipes"*, *"the wipe already comes from
// the whole crew going down"* - and simply had no implementation. For a solo
// run, one keeper down IS the whole crew down.
//
// **The test is "can anybody still act", not "is anybody standing".** Chris's
// framing, and it is the elegant part: one formulation covers a crew with
// somebody on their feet, a solo keeper whose anchor revive is still in hand,
// and a keeper halfway through using it. Counting standing bodies would get
// the second and third wrong.
//
//   - A keeper who is up can act.
//   - A downed keeper with an anchor revive available can act - they can
//     still choose to continue the run.
//   - A keeper mid-self-rescue can act; they are in the middle of acting.
//   - Everyone else is waiting on somebody who, if nobody can act, is never
//     coming.
//
// **Row 3 of his spec, stated because it is the half somebody would soften:**
// a crew has no anchor revive at all (row 9 is solo-only), so a crew with
// every keeper down wipes. There is no waiting it out.
//
// **And it may never touch Storm Charge.** Row 64 line three: escalation
// comes from choices, never from failing, and a wipe is the purest failure in
// the game. The anchor revive costs the meter because continuing is a
// decision; the wipe is what happens when no decision is left.
// `tests/escalation.test.mjs` forbids this file from reaching the storm at
// all, for the same reason it forbids the knockdown path.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LLKRunStateSubsystem.generated.h"

class ULLKVitalsComponent;

/** Fires once, on authority, when the night has nobody left who can act. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLLKCrewWiped);

UCLASS()
class LLK_API ULLKRunStateSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/**
	 * Adds a keeper to the roll. Called by the owning actor, as with lanterns.
	 *
	 * The vitals component does not know this class exists, for the same
	 * reason the lantern does not know about the storm: a component aware of
	 * its listeners cannot be tested without them.
	 */
	void RegisterKeeper(ULLKVitalsComponent* Keeper);

	void UnregisterKeeper(ULLKVitalsComponent* Keeper);

	/** How many keepers could still do something about the night. */
	UFUNCTION(BlueprintPure, Category = "LLK|Run")
	int32 CountKeepersWhoCanAct() const;

	/** Registered keepers still in the run, downed or not. */
	UFUNCTION(BlueprintPure, Category = "LLK|Run")
	int32 CountKeepers() const;

	UFUNCTION(BlueprintPure, Category = "LLK|Run")
	bool HasCrewWiped() const { return bCrewWiped; }

	/** Fires once. Row 8's haul resolution hangs off this. */
	UPROPERTY(BlueprintAssignable, Category = "LLK|Run")
	FLLKCrewWiped OnCrewWiped;

#if WITH_DEV_AUTOMATION_TESTS
	/** Runs one evaluation with no world, exactly as Tick does. */
	void EvaluateForTest() { Evaluate(); }

	/** Bypasses the authority check, since a test subsystem has no net driver. */
	void SetAuthorityForTest(bool bInHasAuthority)
	{
		bForcedAuthority = bInHasAuthority;
		bHasForcedAuthority = true;
	}
#endif

protected:
	/** Decides whether the run is over, and says so exactly once. */
	void Evaluate();

	/** Row 8: a wipe without extraction loses the whole haul. */
	void ResolveHaul();

	bool HasRunAuthority() const;

	/** Weak: a keeper's actor can be destroyed mid-run and must not be kept alive by this. */
	TArray<TWeakObjectPtr<ULLKVitalsComponent>> Keepers;

	bool bCrewWiped = false;

	/**
	 * Seconds between evaluations.
	 *
	 * Not every frame. The condition only changes when somebody goes down or
	 * gets up, and both of those are rare; a quarter-second delay before the
	 * run ends is imperceptible against a keeper realising nobody is coming.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Run", meta = (ClampMin = "0.05"))
	float EvaluateIntervalSeconds = 0.25f;

	float SecondsSinceEvaluate = 0.f;

	bool bForcedAuthority = false;
	bool bHasForcedAuthority = false;
};

// The meter. M1.4.
//
// This is the clock the whole game runs on. It rises on its own, it rises
// faster when keepers are lit, and it never falls - row 6, which is not a
// guideline here: there is no subtracting accessor on this class and
// `AddSpike` clamps at zero so that a negative spike cannot be smuggled in as
// a reduction.
//
// **It owns two meters, not one.** Row 54 split the Warden out of the generic
// thresholds, because conflating them taught players that using a lantern
// starts a six-minute chase - so they correctly optimised toward darkness.
// The Warden accrues on its own dampened meter at
// `1 + LightSensitivity * (LanternMultiplier - 1)`, and the two are tracked
// separately all the way through rather than derived from each other at the
// end.
//
// **The curve lives in data, including the retired one.** Row 12 says
// switching back to `linearCapped` must never need a code change. Both curves
// are implemented, and `LanternMultiplier` reads which one to use from the
// asset rather than from a constant here.
//
// **It counts lanterns rather than trusting a counter.** Registered lanterns
// are recounted every tick instead of being incremented on the state-changed
// delegate. A counter is the version of this that drifts: a keeper downed
// while lit has their actor torn down, and a decrement that never arrives
// leaves the storm permanently accelerated for the rest of the night. With at
// most four lanterns in a run, recounting costs nothing and cannot be wrong.
//
// **Authority only.** The meter is one number the whole crew reads, and a
// client that accrues its own copy will disagree with the server about when
// the Warden came. Clients do not accrue. Replicating the value out to them
// for a HUD needs a GameState and is a later row; until then `GetCharge()` on
// a client is honestly zero rather than plausibly wrong.

#pragma once

#include "CoreMinimal.h"
#include "Data/LLKStormChargeData.h"
#include "Subsystems/WorldSubsystem.h"
#include "LLKStormChargeSubsystem.generated.h"

class ULLKLanternComponent;

/** Fires once, on authority, the first time a threshold's value is passed. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLLKStormThresholdCrossed, FName, ThresholdId, float, Charge);

/** Fires once, on authority, when the Warden's own meter fills. Row 7: it is unkillable. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLLKWardenConverged);

UCLASS()
class LLK_API ULLKStormChargeSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	// ---------------------------------------------------------- subsystem
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	// ---------------------------------------------------------- lanterns
	/**
	 * Adds a lantern to the count. Called by the owning actor, not discovered.
	 *
	 * The lantern component does not know this class exists and must not:
	 * its header is explicit that a component aware of its listeners cannot be
	 * tested without them. `ALLKCharacter` owns both and does the wiring.
	 */
	void RegisterLantern(ULLKLanternComponent* Lantern);

	void UnregisterLantern(ULLKLanternComponent* Lantern);

	/** Lit lanterns right now, recounted rather than remembered. */
	UFUNCTION(BlueprintPure, Category = "LLK|Storm")
	int32 GetLitLanternCount() const;

	// ---------------------------------------------------------- the meters
	UFUNCTION(BlueprintPure, Category = "LLK|Storm")
	float GetCharge() const { return Charge; }

	UFUNCTION(BlueprintPure, Category = "LLK|Storm")
	float GetWardenCharge() const { return WardenCharge; }

	UFUNCTION(BlueprintPure, Category = "LLK|Storm")
	bool HasWardenConverged() const { return bWardenConverged; }

	/** Seconds since the meter started moving. Not the same as the run's clock. */
	UFUNCTION(BlueprintPure, Category = "LLK|Storm")
	float GetElapsedSeconds() const { return ElapsedSeconds; }

	/**
	 * A one-off jump: the extraction call's 180, the anchor revive's 120.
	 *
	 * Row 6 in one place - a negative amount is dropped, not applied, so there
	 * is no expression anywhere in the codebase that can lower this meter.
	 */
	void AddSpike(float Amount, FName Reason);

	UPROPERTY(BlueprintAssignable, Category = "LLK|Storm")
	FLLKStormThresholdCrossed OnThresholdCrossed;

	UPROPERTY(BlueprintAssignable, Category = "LLK|Storm")
	FLLKWardenConverged OnWardenConverged;

	/** Where the tuning came from. Read this rather than trusting a log line. */
	UFUNCTION(BlueprintPure, Category = "LLK|Storm")
	ELLKDataSource GetRulesSource() const { return RulesSource; }

	// ---------------------------------------------------------- the curve
	/**
	 * `1 + bonus * (1 - decay^n) / (1 - decay)`, or the retired linear clamp.
	 *
	 * Static and pure so that the one piece of arithmetic the whole balance
	 * rests on can be asserted without a world, a tick or a lantern. Mirrors
	 * `multiplier()` in `tests/pacing.mjs`; row 12 keeps both curves real.
	 */
	static float LanternMultiplier(const FLLKLanternChargeRules& Rules, int32 LitCount);

	/** The Warden's dampening, row 54. At sensitivity 0 it ignores light entirely. */
	static float WardenMultiplierFrom(float LanternMult, float LightSensitivity);

	/** The tuning in use, so a test can assert against the same numbers the meter uses. */
	const ULLKStormChargeData* GetRules() const { return Rules; }

	/**
	 * Builds a transient asset carrying the declared fallbacks.
	 *
	 * Public and unguarded because `ResolveRules` uses it too: the fallback
	 * path a clean clone actually runs on and the one a test asserts against
	 * must be the same code, or the test is checking a second copy of the
	 * numbers.
	 */
	static ULLKStormChargeData* MakeFallbackRules(UObject* Outer);

#if WITH_DEV_AUTOMATION_TESTS
	/**
	 * Drives the accrual with no world and no lanterns.
	 *
	 * `Advance` is what Tick calls, and it deliberately touches neither
	 * `GetWorld()` nor the registered lanterns - the lit count is a parameter.
	 * That is what lets M1.8 assert the curve, the dampening and "only ever
	 * rises" against the real code path rather than a copy of it.
	 */
	void AdvanceForTest(float DeltaSeconds, int32 LitCount) { Advance(DeltaSeconds, LitCount); }

	/** Injects tuning so a test is not at the mercy of whether `Content/Data` was generated. */
	void SetRulesForTest(ULLKStormChargeData* InRules)
	{
		Rules = InRules;
		RulesSource = InRules != nullptr ? ELLKDataSource::Asset : ELLKDataSource::Unresolved;
	}
#endif

protected:
	/** Loads `/Game/Data/DA_StormCharge`, or builds the declared fallbacks loudly. */
	void ResolveRules();

	/**
	 * One step of both meters. No world, no lantern lookup - the lit count is
	 * passed in. Everything that decides how fast the night moves is here.
	 */
	void Advance(float DeltaSeconds, int32 LitCount);

	/**
	 * Fires any threshold or Warden event the meters have newly earned.
	 *
	 * Split out of `Advance` so that `AddSpike` announces at the moment of the
	 * call rather than on the next tick, without either path owning a second
	 * copy of the crossing rules.
	 */
	void AnnounceCrossings();

	/** True on a listen server, a dedicated server or standalone. False on a client. */
	bool HasStormAuthority() const;

	/** Non-const because UHT does not reflect a const object pointer; read it through GetRules(). */
	UPROPERTY()
	TObjectPtr<ULLKStormChargeData> Rules;

	/** Weak: a keeper's actor can be destroyed mid-run and must not be kept alive by this. */
	TArray<TWeakObjectPtr<ULLKLanternComponent>> Lanterns;

	/** Thresholds already announced, so each fires exactly once. */
	TSet<FName> CrossedThresholds;

	float Charge = 0.f;
	float WardenCharge = 0.f;
	float ElapsedSeconds = 0.f;
	bool bWardenConverged = false;

	ELLKDataSource RulesSource = ELLKDataSource::Unresolved;
};

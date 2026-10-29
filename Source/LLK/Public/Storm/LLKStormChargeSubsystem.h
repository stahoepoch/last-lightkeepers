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
#include "Storm/LLKLightField.h"
#include "Storm/LLKStormSnapshot.h"
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

	/**
	 * Every lit lantern as a light source, for the brightest-position rule.
	 *
	 * It lives on this class because this class already owns the registry of
	 * which lanterns exist and which are lit, and a second registry is a
	 * second thing to keep in step. If the light field ever grows past a
	 * query - a cached field, a spatial structure - the registry should be
	 * extracted and both should read it, rather than this class growing a
	 * second job.
	 */
	TArray<FLLKLightSource> GetLitLightSources() const;

	/**
	 * Where the brightest lit keeper is, or false when the crew is dark.
	 *
	 * **This is the function enemy targeting must call, and the reason it
	 * exists before any enemy does.** Row: enemies converge on the brightest
	 * position, never the nearest player, because that is what makes the
	 * keeper who lit the lantern the keeper who gets found - which is what
	 * makes the store teach (`docs/THE-STORE.md`). The archived Roblox
	 * EnemyService retargeted to nearest, and the UE5 build must not inherit
	 * it. There is nothing to inherit from if this is the only answer here.
	 */
	bool GetBrightestLitPosition(FVector& OutPosition, float& OutBrightness) const;

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

	/** The multiplier the meter is rising at right now, from the live lantern count. */
	UFUNCTION(BlueprintPure, Category = "LLK|Storm")
	float GetCurrentMultiplier() const;

	/** Charge per second right now - baseline times the multiplier above. */
	UFUNCTION(BlueprintPure, Category = "LLK|Storm")
	float GetCurrentRate() const;

	/** The Warden's dampened multiplier right now. Row 54: a different cost for the same light. */
	UFUNCTION(BlueprintPure, Category = "LLK|Storm")
	float GetCurrentWardenMultiplier() const;

	/**
	 * The lowest threshold not yet crossed, or the Warden once they all are.
	 *
	 * Returns false only when there is nothing left to arrive - every
	 * threshold crossed and the Warden already here - which is the one state
	 * a readout must not render as a countdown to nothing.
	 */
	bool GetNextEscalation(FName& OutId, float& OutTargetCharge, float& OutCurrentCharge) const;

	/**
	 * Seconds until `TargetCharge`, at the rate `LitCount` lanterns produce.
	 *
	 * **This is the number that makes the lantern a decision rather than a
	 * light.** A meter climbing from 336 to 480 teaches nothing; "patrols hunt
	 * in 2:24, and 0:58 if you keep that lantern lit" is the whole of row 5 in
	 * one line. Pure and static-ish so the projection can be asserted without
	 * a world - it is arithmetic the player will trust, so it should be
	 * checked rather than eyeballed.
	 *
	 * Returns a negative number when the target is already passed, and
	 * `TNumericLimits<float>::Max()` when the rate is zero - never a division
	 * by it, and never a plausible-looking zero.
	 */
	float SecondsUntilCharge(float FromCharge, float TargetCharge, int32 LitCount) const;

	/** Same projection for the Warden's own dampened meter, from its live value. */
	float SecondsUntilWarden(int32 LitCount) const;

	/**
	 * The Warden projection from an arbitrary meter value.
	 *
	 * Exists because a client's own `WardenCharge` is zero by design; it
	 * projects from the replicated snapshot's value instead. Both sides run
	 * THIS function on the SAME `ULLKStormChargeData`, which is what makes
	 * client and server agree on the countdown without the server having to
	 * send one - and what makes that agreement testable.
	 */
	float SecondsUntilWardenFrom(float FromWardenCharge, int32 LitCount) const;

	/**
	 * Everything a client is allowed to know, as one struct.
	 *
	 * Const and side-effect free so a test can compare it against the
	 * subsystem's own accessors field by field - a snapshot that silently
	 * disagrees with the meter it was built from would make every client
	 * wrong in a way no server-side check could see.
	 */
	FLLKStormSnapshot BuildSnapshot() const;

	/**
	 * Seconds to the next escalation described by a snapshot.
	 *
	 * The one projection entry point the HUD uses, server and client alike.
	 * It reads `NextFromCharge` rather than assuming `Charge`, because when
	 * the Warden is next the measurement is against its own dampened meter.
	 */
	float SecondsUntilSnapshotEscalation(const FLLKStormSnapshot& Snapshot, int32 LitCount) const;

	/**
	 * True where this subsystem is actually accruing - server or standalone.
	 *
	 * Public so a readout can say "not replicated yet" rather than render the
	 * honest zero a client holds as though it were a real meter. A debug HUD
	 * that shows 0 on a client is a debug HUD that lies, which is worse than
	 * no HUD.
	 */
	UFUNCTION(BlueprintPure, Category = "LLK|Storm")
	bool IsAccruing() const { return HasStormAuthority(); }

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

	/** Pushes a fresh snapshot into the GameState. Server only, throttled. */
	void PublishSnapshot();

	/** True on a listen server, a dedicated server or standalone. False on a client. */
	bool HasStormAuthority() const;

	/** Thresholds already announced, exposed to a readout so it can mark them. */
public:
	bool HasCrossed(FName ThresholdId) const { return CrossedThresholds.Contains(ThresholdId); }

protected:

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

	/**
	 * Seconds between published snapshots.
	 *
	 * 10Hz, because the meter moves between one and roughly four units per
	 * second and the readout renders whole seconds - replicating every frame
	 * would put sixty packets a second on the wire to change a number six
	 * times. The projection a client computes between snapshots is from
	 * static rules, so it stays smooth regardless.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Storm", meta = (ClampMin = "0.01"))
	float SnapshotIntervalSeconds = 0.1f;

	float SecondsSinceSnapshot = 0.f;

	/** Logged once rather than every tick when no ALLKGameState is in use. */
	bool bWarnedAboutMissingGameState = false;

	ELLKDataSource RulesSource = ELLKDataSource::Unresolved;
};

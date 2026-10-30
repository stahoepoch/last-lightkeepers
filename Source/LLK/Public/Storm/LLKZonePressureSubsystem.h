// The pressure clock. M1.4, retuned by rows 72 and 73 on 2026-10-29.
//
// This is the clock the whole game runs on. It rises on its own and it never
// falls - row 6, which is not a guideline here but a property of the one
// function that writes it: `Advance` takes a delta, adds the baseline times
// that delta, and nothing else in the project writes `Pressure` at all.
//
// **Nothing a player does moves it.** Row 73: *"Clock means weather alone."*
// Light does not change its rate (row 72), and as of row 73 neither does the
// extraction call - the last priced spike - because *"a radio call cannot
// accelerate the weather."* Row 6 no longer needs enforcing against a
// negative spike, and row 64 line three no longer needs an allowlist of
// permitted causes, because there are no causes.
//
// **Light does not touch its rate, and that is new.** Until 2026-10-29 it rose
// faster when keepers were lit, on a diminishing curve, and that was the
// largest lever in the game - a dark crew's night ran nearly five times longer
// than a lit crew's. Chris retired it because it could not be explained:
// *"The storm gets worse on its own. A lantern does not feed the storm. It
// only lets nearby machines notice you."* A meter that claimed to be the
// night arriving while your lamp accelerated it was two claims that could not
// both be true. Light still costs, locally, through `LLKPerception` and the
// light field.
//
// **One clock per run, never two.** Row 72 again, and it cost this class its
// second meter. The Warden used to accrue on its own dampened float at
// `1 + LightSensitivity * (LanternMultiplier - 1)`. With the rate flat, that
// second meter would have advanced at exactly the same speed as the first -
// two copies of one fact, replicated separately, able to disagree for no
// benefit. The Warden's arrival is now a threshold on the one clock. Row 54
// still holds: its arrival is its own event, and none of it is perception.
//
// **What it is called is zone-authored, which this class does not do yet.**
// Row 70: the mall's clock is Storm Arrival and a future solar farm's is
// Solar Output - the same mechanic with a true sentence behind each. The
// stage names and effects are already data-driven (rows 12 and 33), so only
// the rate's source needs to become per-zone. The class and asset are still
// named `ZonePressure`; renaming them touches Codex's importer, so it is a
// coordinated commit rather than a drive-by.
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
// for a HUD needs a GameState and is a later row; until then `GetPressure()` on
// a client is honestly zero rather than plausibly wrong.

#pragma once

#include "CoreMinimal.h"
#include "Data/LLKZonePressureData.h"
#include "Storm/LLKLightField.h"
#include "Storm/LLKNightStage.h"
#include "Storm/LLKZonePressureSnapshot.h"
#include "Subsystems/WorldSubsystem.h"
#include "LLKZonePressureSubsystem.generated.h"

class ULLKLanternComponent;

// **`ELLKStormSpike` was here and row 73 deleted it.**
//
// It was an allowlist rather than a label, and that was Codex's finding:
// `AddSpike` had taken an `FName Reason`, which logged beautifully and
// enforced nothing, so a future Sweeper in a new file could have written
// `AddSpike(60, TEXT("Knockdown"))` past both the compiler and the source
// guard. A typed cause made failure-triggered escalation inexpressible, and
// row 64 line three was enforced by the type system.
//
// Chris retired the last entry on 2026-10-29: *"Clock means weather alone."*
// Extraction's former weather-spike fields went with the mechanism. Calling
// has the right local price: siren, visible activity, and patrol convergence.
//
// So the allowlist is not empty - it is absent, which is stronger. There is no
// public API on this class that adds to the meter, so there is no longer a
// thing to allow or forbid. The enum's own doc comment argued that the right
// shape for this rule was to make the mistake impossible to express; the
// strongest form of that turned out to be having no expression at all.

/** Fires once, on authority, the first time a threshold's value is passed. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLLKStormThresholdCrossed, FName, ThresholdId, float, Pressure);

/** Fires once, on authority, when the clock reaches the Warden's value. Row 7: it is unkillable. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLLKWardenConverged);

/** A keeper's distress flare: where it is, and how long it lasts. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLLKDistressPulseSignature, FVector, Position, float, Seconds);

/**
 * Light the crew paid for rather than chose.
 *
 * Row 72's price for the Anchor Revive. Not a `UPROPERTY` and not replicated:
 * it exists to be a light source on the server for a few seconds, and a client
 * that needs to show the flare is told by `OnDistressPulse` instead of
 * inferring it from a list.
 */
struct FLLKDistressPulse
{
	/**
	 * Who owns this pulse, or `NAME_None` for a one-shot nobody refreshes.
	 *
	 * Added 2026-10-30 for the extraction rig (row 75). The anchor revive's
	 * flare has a known duration at the moment it is lit, so it needs no
	 * owner; the rig's work lights last as long as a recovery operation does,
	 * and a recovery operation has no knowable length - the hold pauses
	 * whenever the boarding route empties. A named pulse can therefore be
	 * refreshed rather than re-added, which is the difference between one
	 * light that stays on and six hundred stacked copies of one.
	 */
	FName Source;

	FVector Position = FVector::ZeroVector;

	/** Candelas, taken from the keeper's own lantern rather than invented. */
	float Intensity = 0.f;

	/** Already centimetres, like every other `FLLKLightSource`. Row 61. */
	float RadiusCm = 0.f;

	float SecondsRemaining = 0.f;
};

UCLASS()
class LLK_API ULLKZonePressureSubsystem : public UTickableWorldSubsystem
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
	float GetPressure() const { return Pressure; }

	UFUNCTION(BlueprintPure, Category = "LLK|Storm")
	bool HasWardenConverged() const { return bWardenConverged; }

	// ---------------------------------------------------------- the stage
	/**
	 * Which stage the night is in, derived from the meters. M2.3.
	 *
	 * **Row 64 line one in one call.** The charge governs what the building
	 * is authorised to do, and this is that authority in the form an enemy
	 * can ask about - `Stage.IsAtLeast(1)` rather than
	 * `GetPressure() >= 270`, which is the version where the escalation point
	 * lives in the enemy's file and six families disagree about when the
	 * night got worse.
	 */
	UFUNCTION(BlueprintPure, Category = "LLK|Storm")
	FLLKNightStage GetNightStage() const;

	/**
	 * The same derivation from arbitrary values, so a client can run it.
	 *
	 * Static and pure for the reason `SecondsUntilPressure` is: both machines
	 * run THIS function on the SAME `ULLKZonePressureData`, so a client
	 * computes the stage from its replicated charge and cannot disagree with
	 * the server about it. Nothing about the stage goes on the wire.
	 *
	 * Null rules give a calm stage with `Count` 0, which a readout can tell
	 * apart from a genuinely calm night by checking `GetRulesSource()`.
	 */
	static FLLKNightStage StageFrom(
		const ULLKZonePressureData* InRules, float InPressure, bool bInWardenConverged);

	/**
	 * What the authored data says a threshold changes, for the readout.
	 *
	 * `zone-pressure.json` already carries an `effect` sentence per threshold
	 * - "Patrol routes tighten and Sweepers investigate noise further from
	 * their spawn" - and M2.6 is judged on whether a player can name what
	 * changed. Printing the authored sentence is the cheapest honest way to
	 * make a crossing legible, and it means the words a player reads and the
	 * words a designer tunes are the same words.
	 *
	 * Empty when the id is unknown, never a placeholder: a stage badge with
	 * invented text is worse than one with none.
	 */
	UFUNCTION(BlueprintPure, Category = "LLK|Storm")
	FString GetThresholdEffect(FName ThresholdId) const;

	/** Seconds since the meter started moving. Not the same as the run's clock. */
	UFUNCTION(BlueprintPure, Category = "LLK|Storm")
	float GetElapsedSeconds() const { return ElapsedSeconds; }

	/**
	 * Pressure per second right now.
	 *
	 * **Flat, as of row 72.** There used to be a `GetCurrentMultiplier` and a
	 * `GetCurrentWardenMultiplier` either side of this, both driven by the lit
	 * lantern count. They are gone: nothing a player does changes the rate at
	 * which the night arrives. Light costs locally instead, which is what
	 * `LLKPerception` and the light field are for.
	 */
	UFUNCTION(BlueprintPure, Category = "LLK|Storm")
	float GetCurrentRate() const;


	/**
	 * The lowest threshold not yet crossed, or the Warden once they all are.
	 *
	 * Returns false only when there is nothing left to arrive - every
	 * threshold crossed and the Warden already here - which is the one state
	 * a readout must not render as a countdown to nothing.
	 */
	bool GetNextEscalation(FName& OutId, float& OutTargetPressure, float& OutCurrentPressure) const;

	/**
	 * Seconds until `TargetCharge` at the clock's one rate.
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
	float SecondsUntilPressure(float FromCharge, float TargetCharge) const;

	/** Same projection for the Warden's own dampened meter, from its live value. */
	float SecondsUntilWarden() const;

	/**
	 * The Warden projection from an arbitrary meter value.
	 *
	 * Exists because a client's own `WardenCharge` is zero by design; it
	 * projects from the replicated snapshot's value instead. Both sides run
	 * THIS function on the SAME `ULLKZonePressureData`, which is what makes
	 * client and server agree on the countdown without the server having to
	 * send one - and what makes that agreement testable.
	 */


	/**
	 * Everything a client is allowed to know, as one struct.
	 *
	 * Const and side-effect free so a test can compare it against the
	 * subsystem's own accessors field by field - a snapshot that silently
	 * disagrees with the meter it was built from would make every client
	 * wrong in a way no server-side check could see.
	 */
	FLLKZonePressureSnapshot BuildSnapshot() const;

	/**
	 * Seconds to the next escalation described by a snapshot.
	 *
	 * The one projection entry point the HUD uses, server and client alike.
	 * It reads `NextFromPressure` rather than assuming `Pressure` so that a
	 * client projects from the value the server actually published.
	 */
	float SecondsUntilSnapshotEscalation(const FLLKZonePressureSnapshot& Snapshot) const;

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

	UPROPERTY(BlueprintAssignable, Category = "LLK|Storm")
	FLLKStormThresholdCrossed OnThresholdCrossed;

	/**
	 * A keeper spent their Anchor Revive and lit the room doing it.
	 *
	 * Broadcast at the moment of the revive. The light itself is already in
	 * the field by then, so a listener is for the audible half and the visual
	 * flare - not for deciding who notices, which the field answers.
	 */
	UPROPERTY(BlueprintAssignable, Category = "LLK|Storm")
	FLLKDistressPulseSignature OnDistressPulse;

	/**
	 * Adds a distress flare at a position for a number of seconds.
	 *
	 * Every value is authored somewhere else: intensity and radius come from
	 * the reviving keeper's own lantern rules, and the duration from
	 * `revive.reviveSeconds`. Nothing new was tuned for this, which is the
	 * point - the price is "you are the brightest thing in the building for
	 * eight seconds", stated in the units the game already uses.
	 */
	void AddDistressPulse(const FVector& Position, float Intensity, float RadiusCm, float Seconds);

	/**
	 * The same flare, owned by a name, so it can be kept alive instead of stacked.
	 *
	 * Calling this repeatedly with one `Source` moves and re-arms a single
	 * pulse. Stop calling it and that pulse runs out on its own, which is how
	 * a light that belongs to an ongoing event fades rather than vanishing at
	 * the instant the event ends.
	 *
	 * `NAME_None` is refused rather than treated as a source, because an
	 * unnamed refreshable pulse is the stacking bug with extra steps.
	 */
	void AddOrRefreshDistressPulse(
		FName Source, const FVector& Position, float Intensity, float RadiusCm, float Seconds);

	/** How many flares are currently lit. For the tests and the readout. */
	int32 GetDistressPulseCount() const { return DistressPulses.Num(); }

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


	/** The Warden's dampening, row 54. At sensitivity 0 it ignores light entirely. */


	/** The tuning in use, so a test can assert against the same numbers the meter uses. */
	const ULLKZonePressureData* GetRules() const { return Rules; }

	/**
	 * Builds a transient asset carrying the declared fallbacks.
	 *
	 * Public and unguarded because `ResolveRules` uses it too: the fallback
	 * path a clean clone actually runs on and the one a test asserts against
	 * must be the same code, or the test is checking a second copy of the
	 * numbers.
	 */
	static ULLKZonePressureData* MakeFallbackRules(UObject* Outer);

#if WITH_DEV_AUTOMATION_TESTS

	/**
	 * Drives the accrual with no world and no lanterns.
	 *
	 * `Advance` is what Tick calls, and it deliberately touches neither
	 * `GetWorld()` nor the registered lanterns - the lit count is a parameter.
	 * That is what lets M1.8 assert the curve, the dampening and "only ever
	 * rises" against the real code path rather than a copy of it.
	 */
	void AdvanceForTest(float DeltaSeconds) { Advance(DeltaSeconds); }

	/** Injects tuning so a test is not at the mercy of whether `Content/Data` was generated. */
	void SetRulesForTest(ULLKZonePressureData* InRules)
	{
		Rules = InRules;
		RulesSource = InRules != nullptr ? ELLKDataSource::Asset : ELLKDataSource::Unresolved;
	}
#endif

protected:
	/** Loads `/Game/Data/DA_ZonePressure`, or builds the declared fallbacks loudly. */
	void ResolveRules();

	/**
	 * One step of both meters. No world, no lantern lookup - the lit count is
	 * passed in. Everything that decides how fast the night moves is here.
	 */
	void Advance(float DeltaSeconds);

	/** Ticks the flares down and drops the expired ones. */
	void ExpireDistressPulses(float DeltaSeconds);

	/**
	 * Fires any threshold or Warden event the clock has newly earned.
	 *
	 * It was split out of `Advance` so that a spike could announce at the
	 * moment of the call rather than on the next tick. Row 73 removed spikes,
	 * so `Advance` is now its only caller - and it stays a separate function
	 * because the crossing rules are worth having in one place whether or not
	 * two paths need them.
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
	TObjectPtr<ULLKZonePressureData> Rules;

	/** Weak: a keeper's actor can be destroyed mid-run and must not be kept alive by this. */
	TArray<TWeakObjectPtr<ULLKLanternComponent>> Lanterns;

	/** Thresholds already announced, so each fires exactly once. */
	TSet<FName> CrossedThresholds;

	float Pressure = 0.f;
	/**
	 * Transient light the crew paid for rather than chose.
	 *
	 * Only the Anchor Revive creates these today. Not replicated: the field
	 * they feed is read on the server, and a client that needs to see the
	 * flare gets it from `OnDistressPulse`.
	 */
	TArray<FLLKDistressPulse> DistressPulses;
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

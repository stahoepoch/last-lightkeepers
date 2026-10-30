// The thin half of extraction: measure the world, conclude nothing. M3.
//
// Every decision in here is made by `LLKExtraction`. This class loads the
// authored numbers, counts who is standing on the boarding route, hands both
// to a pure function, and broadcasts whatever that function says happened. The
// division is the same one `LLKMapReadiness::Observe` and `LLKLanternFeel`
// draw, and it is why the whole state machine is testable with no level.
//
// **What it knows about the world, and nothing more:**
//
//   - `ULLKLootRulesData` for the exits - their holds, payouts and gates.
//   - `ULLKStormChargeData::ExtractionCall` for the siren and the convergence
//     flag. Two of that block's three fields; the third is the retired spike,
//     which this class never reads and `FLLKExtractionAnnounce` cannot carry.
//   - `ULLKRunStateSubsystem` for the wipe and for who is still in the run.
//   - Boarding routes, registered by whatever actor ends up representing a
//     dock. Row 74 keeps the rig outside and the driver out of the interior,
//     so the route is the only geometry a rule here ever needs.
//
// **The route is the honest seam to the greybox, and it fails loudly.** No map
// exists yet, so no route is registered, so the hold cannot advance - and
// rather than letting that look like a hold that simply takes forever, the
// subsystem logs the exit by name once per call: the `LLK.CheckMap` discipline
// applied to the one piece of geometry extraction needs. An extraction that
// silently never completes would be the worst possible version of this slice.
//
// **Authority.** The server owns the state machine outright and publishes a
// snapshot. Clients hold a replicated copy and decide nothing, exactly as with
// the storm - a client that could conclude it had extracted would be a client
// that could conclude it had kept the haul.

#pragma once

#include "CoreMinimal.h"
#include "Run/LLKExtraction.h"
#include "Run/LLKExtractionSnapshot.h"
#include "Subsystems/WorldSubsystem.h"
#include "LLKExtractionSubsystem.generated.h"

class ULLKLootRulesData;
class ULLKStormChargeData;
class ULLKRunStateSubsystem;

/** Called, with the exit's id and how long the siren will run. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLLKExtractionCalled, FName, PointId, float, SirenSeconds);

/** The siren finished and the rig is at the dock. The hold starts now. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLLKExtractionRigArrived, FName, PointId);

/** Called off. Fires for a crew's decision, never for a failure. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLLKExtractionCancelled, FName, PointId);

/** Out, with what it was worth. Row 8's only path to keeping a haul. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FLLKExtractionSecured, FName, PointId, float, PayoutModifier, int32, ItemsSecured);

/** The crew wiped with an extraction underway, or without ever calling one. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLLKExtractionFailed);

/**
 * One dock's boarding route.
 *
 * A position and a radius rather than a volume, because a radius is the one
 * shape that needs no `.uasset` and no geometry: the greybox can register a
 * plain actor and the rule works. A real bay mouth is a volume and will want
 * one, and that is a map-lane decision rather than a rule change - this struct
 * is what the rule reads either way.
 */
USTRUCT(BlueprintType)
struct FLLKBoardingRoute
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	FName PointId;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	TWeakObjectPtr<AActor> Anchor;

	/** Centimetres. Row 61: converted at the boundary, never stored in studs. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	float RadiusCm = 0.f;
};

UCLASS()
class LLK_API ULLKExtractionSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/**
	 * Announce one. Server only; returns the refusal, `None` on success.
	 *
	 * `Caller` is optional and is used for one thing: row 74's convergence.
	 * The rig's own work lights are what the building notices, and until the
	 * rig exists the caller's lantern stands in for them.
	 */
	ELLKExtractionRefusal CallExtraction(FName PointId, AActor* Caller = nullptr);

	/** Call it off. Server only. True if there was something to cancel. */
	bool CancelExtraction();

	/** Registered by the dock actor, whenever the greybox grows one. */
	void RegisterBoardingRoute(FName PointId, AActor* Anchor, float RadiusCm);

	void UnregisterBoardingRoute(FName PointId);

	/**
	 * The gate on `FLLKExtractionPoint::Requires`.
	 *
	 * The freight lift wants `SubBasementBreakerRestored` and no restoration
	 * system exists to grant it, so this is the seam that one will use. Until
	 * then `LLK.Extraction.Satisfy` sets it by hand, which is also how the
	 * refusal path gets exercised in a real build.
	 */
	void SetRequirementSatisfied(FName Requirement, bool bSatisfied);

	UFUNCTION(BlueprintPure, Category = "LLK|Extraction")
	bool IsRequirementSatisfied(FName Requirement) const;

	UFUNCTION(BlueprintPure, Category = "LLK|Extraction")
	const FLLKExtractionState& GetState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "LLK|Extraction")
	ELLKExtractionPhase GetPhase() const { return State.Phase; }

	UFUNCTION(BlueprintPure, Category = "LLK|Extraction")
	float GetHoldFraction() const { return LLKExtraction::HoldFraction(State); }

	UFUNCTION(BlueprintPure, Category = "LLK|Extraction")
	bool WasHoldStalledLastStep() const { return bHoldStalled; }

	/** The exits this map could offer, authored or fallback. */
	UFUNCTION(BlueprintPure, Category = "LLK|Extraction")
	TArray<FName> GetExitIds() const;

	/** The terms of one exit. False when nothing authored that id. */
	bool FindExit(FName PointId, FLLKExtractionPointRules& OutPoint, FName& OutRequires) const;

	UPROPERTY(BlueprintAssignable, Category = "LLK|Extraction")
	FLLKExtractionCalled OnExtractionCalled;

	UPROPERTY(BlueprintAssignable, Category = "LLK|Extraction")
	FLLKExtractionRigArrived OnRigArrived;

	UPROPERTY(BlueprintAssignable, Category = "LLK|Extraction")
	FLLKExtractionCancelled OnExtractionCancelled;

	UPROPERTY(BlueprintAssignable, Category = "LLK|Extraction")
	FLLKExtractionSecured OnExtractionSecured;

	UPROPERTY(BlueprintAssignable, Category = "LLK|Extraction")
	FLLKExtractionFailed OnExtractionFailed;

#if WITH_DEV_AUTOMATION_TESTS
	void SetLootRulesForTest(ULLKLootRulesData* InRules) { LootRules = InRules; }
	void SetAnnounceForTest(const FLLKExtractionAnnounce& InAnnounce) { Announce = InAnnounce; }
	void SetStateForTest(const FLLKExtractionState& InState) { State = InState; }
	void SetAuthorityForTest(bool bInHasAuthority)
	{
		bForcedAuthority = bInHasAuthority;
		bHasForcedAuthority = true;
	}
	void AdvanceForTest(float DeltaSeconds, TOptional<int32> KeepersOnRoute, bool bCrewWiped)
	{
		FLLKExtractionInput Input;
		Input.DeltaSeconds = DeltaSeconds;
		Input.KeepersOnBoardingRoute = KeepersOnRoute;
		Input.bCrewWiped = bCrewWiped;
		Apply(LLKExtraction::Advance(State, Input));
	}
#endif

protected:
	/** Reads the two data assets. Fallbacks are declared, as everywhere else. */
	void ResolveRules();

	/** Counts keepers on the called exit's route. Unset when nothing measured it. */
	TOptional<int32> ObserveBoardingRoute() const;

	/** Broadcasts a step's transitions and keeps the new state. Decides nothing. */
	void Apply(const FLLKExtractionStep& Step);

	void ResolveSecuredHaul();

	/**
	 * Keeps the rig's lights lit for as long as the operation runs. Row 75.
	 *
	 * Chris's rule for the pause - *"the siren, rig lights, and converged
	 * patrols persist, so stepping away buys survival time, not a free reset
	 * or a safer retry"* - cannot be served by a light with a duration fixed
	 * at the call, because a paused hold has no knowable length. So the light
	 * is a named pulse re-armed every tick while the extraction is underway,
	 * and it runs out one refresh after the operation ends rather than
	 * blinking off at the instant of a cancel.
	 */
	void RefreshRigLight();

	/** The one pulse the rig owns, so refreshing it replaces rather than stacks. */
	static FName RigLightSourceName();

	void PublishSnapshot();

	bool HasExtractionAuthority() const;

	ULLKRunStateSubsystem* GetRunState() const;

	UPROPERTY()
	TObjectPtr<ULLKLootRulesData> LootRules;

	/** Only `ExtractionCall`'s two live fields are copied out of it, at Initialize. */
	FLLKExtractionAnnounce Announce;

	FLLKExtractionState State;

	TArray<FLLKBoardingRoute> Routes;

	TSet<FName> SatisfiedRequirements;

	bool bHoldStalled = false;

	/**
	 * The rig's brightness, captured at the call from the caller's lantern.
	 *
	 * **Not an authored number, and that is flagged rather than hidden.**
	 * Nothing in `data/` says how bright a recovery rig is. The anchor
	 * revive's flare borrows the keeper's own lantern for the same reason, and
	 * a rig should almost certainly be brighter than a hand lantern - that
	 * value belongs in `loot.json` beside the exit, which means it arrives
	 * with Codex's importer rather than here.
	 */
	float RigIntensity = 0.f;

	float RigRadiusCm = 0.f;

	/** Used for the light's position only when no boarding route is registered. */
	TWeakObjectPtr<AActor> RigLightFallback;

	/**
	 * How long each refresh of the rig's light lasts.
	 *
	 * Long enough that a hitching frame cannot let it lapse, short enough that
	 * the lights going out reads as the operation ending. It is a refresh
	 * interval rather than a duration anybody tunes, which is why it is here
	 * and not in `data/`.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Extraction", meta = (ClampMin = "0.1"))
	float RigLightRefreshSeconds = 1.f;

	/** Logged once per call rather than once per frame. */
	bool bWarnedAboutMissingRoute = false;

	float SecondsSinceSnapshot = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Extraction", meta = (ClampMin = "0.05"))
	float SnapshotIntervalSeconds = 0.1f;

	bool bForcedAuthority = false;
	bool bHasForcedAuthority = false;
};

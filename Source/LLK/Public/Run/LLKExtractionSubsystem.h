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
//   - `ULLKZonePressureData::ExtractionCall` for the siren and the convergence
//     flag. Two of that block's three fields; the third is the retired spike,
//     which this class never reads and `FLLKExtractionAnnounce` cannot carry.
//   - `ULLKRunStateSubsystem` for the wipe and for who is still in the run.
//   - `ALLKExtractionZone`, one per exit, which owns the trigger volume and
//     who is standing in it. Row 74 keeps the rig outside and the driver out
//     of the interior, so that zone is the only geometry a rule here ever
//     needs. **The subsystem does no geometry of its own** - it asks the zone,
//     the way perception asks an `ILLKVisibilitySource`. It measured a radius
//     itself for one day and that was a placeholder: a sphere cannot describe
//     a bay mouth, and two places doing geometry is how they come to disagree.
//
// **The zone is the honest seam to the greybox, and it fails loudly.** No map
// exists yet, so no zone is registered, so the hold cannot advance - and
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
class ULLKZonePressureData;
class ULLKRunStateSubsystem;
class ALLKExtractionZone;

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

	/**
	 * Registered by the zone itself at `BeginPlay`, so a greybox dock is an
	 * actor with an exit id typed into it and nothing else.
	 *
	 * One zone per exit. A second one for the same id is refused rather than
	 * kept, because two volumes for one exit is a hold that can be advanced
	 * from two places.
	 */
	void RegisterZone(ALLKExtractionZone* Zone);

	void UnregisterZone(ALLKExtractionZone* Zone);

	/** The zone for an exit, or null. Null is why a hold can stall forever. */
	ALLKExtractionZone* FindZone(FName PointId) const;

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

	/**
	 * Asks the called exit's zone who is standing in it.
	 *
	 * Unset when there is no zone, which is a different fact from an empty one
	 * and the whole reason the input is a `TOptional`: no dock in the map is
	 * not the crew's fault, and must never secure a haul on its own.
	 */
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

	/** Weak: a zone's level can be unloaded under a running extraction. */
	TArray<TWeakObjectPtr<ALLKExtractionZone>> Zones;

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

	/** Used for the light's position only when no zone is registered. */
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
	bool bWarnedAboutMissingZone = false;

	/**
	 * Whether a snapshot has ever reached the GameState this run.
	 *
	 * **The idle readout was unreachable without this.** `Tick` returned
	 * early while idle - "no state machine, no snapshot, no cost" - so
	 * `FLLKExtractionSnapshot::bPublished` stayed false for the whole of the
	 * idle phase, which is the whole run until someone calls. And
	 * `LLKExtractionReadout::Describe` refuses to say anything about an
	 * unpublished snapshot, correctly: before the server has spoken, a
	 * default snapshot and a genuine idle one are identical.
	 *
	 * So every line in the Idle branch was dead in play - the objective, and
	 * the dock's own "Hold [E]" prompt with it. Chris reported not knowing how
	 * to extract three times, each time against a HUD that could not speak.
	 * Every test built a published idle snapshot by hand, so none of them
	 * could see it.
	 *
	 * One publish, once, is all idle needs: the phase does not change while
	 * nothing is happening, so there is nothing to keep republishing and the
	 * early-out's reasoning still holds for every frame after the first.
	 */
	bool bEverPublished = false;

	float SecondsSinceSnapshot = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Extraction", meta = (ClampMin = "0.05"))
	float SnapshotIntervalSeconds = 0.1f;

	bool bForcedAuthority = false;
	bool bHasForcedAuthority = false;
};

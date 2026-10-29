// The lantern's state. M1.2.
//
// **This owns whether the lantern is on. It does not own a light.** The light
// actor, its mesh and the sound of it striking are M1.3 and belong to Chris
// (`Content/` is his, row 41). This component has no `ULightComponent`, no
// mesh and no reference to either, which is what lets M1.2 be finished,
// tested and replicated before any art exists.
//
// It broadcasts instead. `OnLanternStateChanged` is how the light actor knows
// to turn on, how `ULLKStormChargeSubsystem` (M1.4) counts lit keepers, and
// how enemies will learn where the brightest position is. None of those three
// are referenced here, deliberately: a component that knows about its
// listeners cannot be tested without them.
//
// **Server-authoritative with no exception.** The client asks, the server
// decides, the state replicates. `docs/ROADMAP.md` lists "replication
// retrofitted late" as a named risk, and the lantern is the one piece of
// state in this game that everything else reads - storm charge, enemy
// targeting, loot visibility. If any of it can be set locally, all of it can
// be lied about.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/LLKDataTypes.h"
#include "Data/LLKLanternData.h"
#include "LLKLanternComponent.generated.h"

/** Why the lantern ended up in its current state. Useful to listeners and to tests. */
UENUM(BlueprintType)
enum class ELLKLanternChangeReason : uint8
{
	/** A keeper asked for it. */
	PlayerToggled,
	/** Row: `knockdown.lanternForcedOffWhenDowned`. Not a choice. */
	ForcedOffWhenDowned,
	/** `charge.autoOffAtEmpty`, once charge is real. Armed, never fired in v1. */
	ChargeExhausted,
	/** Replicated in from the server; the local client did not decide this. */
	Replicated
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLLKLanternStateChanged, bool, bIsOn, ELLKLanternChangeReason, Reason);

UCLASS(ClassGroup = (LLK), meta = (BlueprintSpawnableComponent))
class LLK_API ULLKLanternComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULLKLanternComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Asks the server to flip the lantern. Never changes state locally. */
	UFUNCTION(BlueprintCallable, Category = "LLK|Lantern")
	void ToggleLantern();

	/** Asks the server for a specific state. Idempotent. */
	UFUNCTION(BlueprintCallable, Category = "LLK|Lantern")
	void SetLanternOn(bool bNewOn);

	/**
	 * Server-only. Turns the lantern off for a reason the keeper does not get
	 * to argue with - being downed, today. Separate from SetLanternOn so the
	 * reason survives into the delegate rather than being flattened to "off".
	 */
	void ForceOff(ELLKLanternChangeReason Reason);

	UFUNCTION(BlueprintPure, Category = "LLK|Lantern")
	bool IsLanternOn() const { return bLanternOn; }

	/** True when `knockdown.lanternForcedOffWhenDowned` is set in the data. */
	UFUNCTION(BlueprintPure, Category = "LLK|Lantern")
	bool DoesDownedForceLanternOff() const { return bForcedOffWhenDowned; }

	/** Fires on server and clients alike, whenever the state actually changes. */
	UPROPERTY(BlueprintAssignable, Category = "LLK|Lantern")
	FLLKLanternStateChanged OnLanternStateChanged;

	/** Where the rules came from. Mirrors the character's two source enums. */
	UFUNCTION(BlueprintPure, Category = "LLK|Lantern")
	ELLKDataSource GetRulesSource() const { return RulesSource; }

#if WITH_DEV_AUTOMATION_TESTS
	/**
	 * The authority-side decision, reachable by tests only.
	 *
	 * `ApplyState` is protected because nothing outside this component may
	 * decide the lantern's state - that is the whole point of M1.2. But the
	 * decision is also the only thing worth testing, so rather than widen the
	 * real API or leave the rules unasserted, this seam exists and exists
	 * only where `WITH_DEV_AUTOMATION_TESTS` does. A shipped build has no
	 * route to it.
	 */
	void ApplyStateForTest(bool bNewOn, ELLKLanternChangeReason Reason) { ApplyState(bNewOn, Reason); }
#endif

protected:
	/**
	 * Defaults to the importer's output path. A soft pointer that resolves to
	 * nothing simply fails, so a clean clone with no `Content/Data/` still
	 * runs - it just announces the fallback. That is strictly better than
	 * leaving this unset, which was the original shape: the assets existed on
	 * disk and nothing pointed at them, so the game ran on declared fallbacks
	 * that happened to match while reporting that it had read no data.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data")
	TSoftObjectPtr<ULLKLanternData> LanternData =
		TSoftObjectPtr<ULLKLanternData>(FSoftObjectPath(TEXT("/Game/Data/DA_Lantern.DA_Lantern")));

	/**
	 * Fallback for `knockdown.lanternForcedOffWhenDowned`, which is `true` in
	 * the data. Declared rather than assumed, and `GetRulesSource()` says when
	 * it is in use.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data")
	bool bFallbackForcedOffWhenDowned = true;

	UFUNCTION(Server, Reliable)
	void ServerSetLanternOn(bool bNewOn);

	UFUNCTION()
	void OnRep_LanternOn();

	/** Applies a decided state and broadcasts. Server side. */
	void ApplyState(bool bNewOn, ELLKLanternChangeReason Reason);

	void ResolveRules();

	UPROPERTY(ReplicatedUsing = OnRep_LanternOn)
	bool bLanternOn = false;

	UPROPERTY(VisibleInstanceOnly, Category = "LLK|Lantern")
	bool bForcedOffWhenDowned = true;

	UPROPERTY(VisibleInstanceOnly, Category = "LLK|Lantern")
	ELLKDataSource RulesSource = ELLKDataSource::Unresolved;

	/** Set on clients so OnRep can report Replicated rather than guessing. */
	ELLKLanternChangeReason LastReason = ELLKLanternChangeReason::PlayerToggled;
};

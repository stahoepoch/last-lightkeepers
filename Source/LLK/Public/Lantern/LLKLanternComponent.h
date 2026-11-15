// The lantern's state. M1.2.
//
// **This owns whether the lantern is on. It does not own a light.** The light
// actor, its mesh and the sound of it striking are M1.3 and belong to Chris
// (`Content/` is his, row 41). This component has no `ULightComponent`, no
// mesh and no reference to either, which is what lets M1.2 be finished,
// tested and replicated before any art exists.
//
// It broadcasts instead. `OnLanternStateChanged` is how the light actor knows
// to turn on, how `ULLKZonePressureSubsystem` (M1.4) counts lit keepers, and
// how enemies will learn where the brightest position is. None of those three
// are referenced here, deliberately: a component that knows about its
// listeners cannot be tested without them.
//
// **Server-authoritative with no exception.** The client asks, the server
// decides, the state replicates. `docs/ROADMAP.md` lists "replication
// retrofitted late" as a named risk, and the lantern is the one piece of
// state in this game that everything else reads - zone pressure, enemy
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
	Replicated,
	/**
	 * The keeper does not have the lantern. Not a choice either, and
	 * distinct from `ForcedOffWhenDowned` on purpose: being knocked down is
	 * a thing that happened to a keeper holding a light, and this is a
	 * keeper who has no light to hold.
	 */
	NotPossessed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLLKLanternStateChanged, bool, bIsOn, ELLKLanternChangeReason, Reason);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FLLKLanternPossessionChanged, bool, bHasLantern);

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
	/**
	 * Announced when the keeper gains or loses the lantern itself, which is
	 * a different question from whether it is switched on.
	 */
	UPROPERTY(BlueprintAssignable, Category = "LLK|Lantern")
	FLLKLanternPossessionChanged OnLanternPossessionChanged;

	/**
	 * **Does the keeper have a lantern at all.**
	 *
	 * The pawn has carried one since it was first written, switched off but
	 * always in hand and always toggleable. The story says the child starts
	 * without it, so possession is now state rather than an assumption.
	 *
	 * This gates `ApplyState`, which is the single place the lantern can
	 * turn on - so every consumer downstream respects possession without
	 * knowing it exists. The storm's lit-lantern count, the enemies' light
	 * field, relic legibility and the telemetry all ask `IsLanternOn()`, and
	 * an unpossessed lantern can never answer yes. That is deliberately an
	 * invariant and not a filter applied in five places: a filter is
	 * something a sixth consumer can forget.
	 */
	UFUNCTION(BlueprintPure, Category = "LLK|Lantern")
	bool HasLantern() const { return bHasLantern; }

	/** Server only. Idempotent. */
	void GrantLantern();

	/** Server only. Forces the light off first - you cannot leave a lit
	 *  lantern burning in a hand that no longer holds one. */
	void RevokeLantern();

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

	/**
	 * Possession, for tests that have no owning actor to grant it.
	 *
	 * `GrantLantern` needs an owner with authority, and these components are
	 * built on the transient package with no owner at all - the same reason
	 * `ApplyStateForTest` exists. Kept as a separate hook rather than
	 * loosening `GrantLantern`, because "the server decides who gets the
	 * lantern" is the rule and a test is not a reason to weaken it.
	 */
	void GrantLanternForTest() { bHasLantern = true; }
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

	UFUNCTION()
	void OnRep_HasLantern();

	/**
	 * False until something in the world grants it. Replicated, because the
	 * storm and the machines read the consequences of it on the server while
	 * the owning client draws the instrument in its own hand.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_HasLantern)
	bool bHasLantern = false;

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

// Being put down, and not being killed. M2.1.
//
// **The machines contain people; they do not kill them.** Row 47 says they
// want nothing and hate nothing - they are executing degraded maintenance,
// stocking, safety and loss-prevention instructions - and row 49 makes
// containment what happens to someone who loses. So there is no health bar
// here, no damage number and no death. There is a keeper who is standing and
// a keeper who is on the floor.
//
// **Four flags from `lantern.json` `knockdown`, and all four are load-bearing:**
//
//   - `downedWalkSpeed: 0` — a downed keeper cannot crawl. Not a tuning
//     placeholder: zero IS the value.
//   - `lanternForcedOffWhenDowned: true` — the light goes out, and not by the
//     keeper's choice. `ULLKLanternComponent::ForceOff` was built in M1.2 for
//     this exact caller and **has never had one until now**; the reason enum
//     entry `ForcedOffWhenDowned` has sat unused since the day it was written.
//   - `dropsCarriedCrate: false` — row 11. A downed keeper KEEPS what they
//     carry, which is what pins a crate to a body: with a walk speed of zero
//     and no drop, a crate cannot be banked while its carrier is on the floor.
//     That is what physically enforces "everyone protects the carrier" (row
//     59). Setting it true removes the enforcement, which is why the data
//     file's own note says so in capitals.
//   - `bleedOutSeconds: 0` — a downed keeper waits indefinitely. The wipe
//     comes from the whole crew going down, not from a timer.
//
// **The zero-means-stale trick from this morning is WRONG here, and that is
// worth stating.** `FLLKLanternLightRules::DescribesARealLight` exists because
// a radius of zero is not a tuning anybody would author, so zero meant the
// asset predated the field. Here zero is deliberate and documented twice in
// `lantern.json`: a downed keeper genuinely cannot move and genuinely has no
// bleed-out. Validating these the same way would silently replace Chris's
// authored design with a fallback. Same shape of code, opposite correct
// answer, and the only thing that distinguishes them is reading the data.
//
// **Server-authoritative, like everything that matters.** A client that could
// put itself up again is a client that cannot be put down.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/LLKDataTypes.h"
#include "Data/LLKLanternData.h"
#include "LLKVitalsComponent.generated.h"

/** Why a keeper went down, or got back up. For listeners and for tests. */
UENUM(BlueprintType)
enum class ELLKDownedReason : uint8
{
	/** A machine reached them. The ordinary case. */
	Contained,
	/** A teammate's hold completed, or the solo Anchor Revive. M2.2. */
	Revived,
	/** Put down or up by a debug command. Never in a shipped build. */
	Debug,
	/** Replicated in; this client did not decide it. */
	Replicated
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLLKDownedChanged, bool, bIsDowned, ELLKDownedReason, Reason);

UCLASS(ClassGroup = (LLK), meta = (BlueprintSpawnableComponent))
class LLK_API ULLKVitalsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULLKVitalsComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "LLK|Vitals")
	bool IsDowned() const { return bDowned; }

	/**
	 * Server only. Puts the keeper on the floor.
	 *
	 * Idempotent: a second machine reaching an already-downed keeper changes
	 * nothing and announces nothing, because the storm counts lit lanterns
	 * and the HUD counts downed keepers, and both would move for an event
	 * that did not happen.
	 */
	void GoDown(ELLKDownedReason Reason);

	/** Server only. M2.2's revive calls this; nothing else should. */
	void StandUp(ELLKDownedReason Reason);

	/** Fires on server and clients alike, whenever the state actually changes. */
	UPROPERTY(BlueprintAssignable, Category = "LLK|Vitals")
	FLLKDownedChanged OnDownedChanged;

	/** Seconds spent on the floor. What a revive prompt and the HUD read. */
	UFUNCTION(BlueprintPure, Category = "LLK|Vitals")
	float GetSecondsDowned() const { return SecondsDowned; }

	/** 0 means a downed keeper waits indefinitely, which is the authored value. */
	UFUNCTION(BlueprintPure, Category = "LLK|Vitals")
	float GetBleedOutSeconds() const { return BleedOutSeconds; }

	/** Studs/s while downed. 0 means cannot crawl - deliberate, not unset. */
	UFUNCTION(BlueprintPure, Category = "LLK|Vitals")
	float GetDownedWalkStuds() const { return DownedWalkStuds; }

	/** Row 11: false means a downed keeper keeps the crate, which pins it to their body. */
	UFUNCTION(BlueprintPure, Category = "LLK|Vitals")
	bool DoesDropCarriedCrate() const { return bDropsCarriedCrate; }

	UFUNCTION(BlueprintPure, Category = "LLK|Vitals")
	ELLKDataSource GetRulesSource() const { return RulesSource; }

#if WITH_DEV_AUTOMATION_TESTS
	/** The authority-side decision, reachable by tests with no owner or world. */
	void SetDownedForTest(bool bNewDowned, ELLKDownedReason Reason) { ApplyDowned(bNewDowned, Reason); }

	void SetRulesForTest(float InDownedWalkStuds, float InBleedOut, bool bInForceLanternOff, bool bInDropsCrate)
	{
		DownedWalkStuds = InDownedWalkStuds;
		BleedOutSeconds = InBleedOut;
		bLanternForcedOffWhenDowned = bInForceLanternOff;
		bDropsCarriedCrate = bInDropsCrate;
		RulesSource = ELLKDataSource::Asset;
	}

	float GetSecondsDownedForTest() const { return SecondsDowned; }
	void AdvanceForTest(float Seconds) { TickDowned(Seconds); }

	/**
	 * Runs the REAL resolution path against an injected asset.
	 *
	 * `SetRulesForTest` writes the resolved values directly, which tests the
	 * state machine and silently skips `ResolveRules` - so a misapplied
	 * zero-is-stale check inside it passed every assertion. This is the seam
	 * that closes that, and it exists because the break-proof found the hole.
	 */
	void ResolveFromAssetForTest(ULLKLanternData* InData)
	{
		LanternData = InData;
		ResolveRules();
	}
#endif

protected:
	/** Applies a decided state and broadcasts. Server side, and OnRep's one other caller. */
	void ApplyDowned(bool bNewDowned, ELLKDownedReason Reason);

	/** The bleed-out clock. Armed by the data and never fired while it reads 0. */
	void TickDowned(float DeltaSeconds);

	void ResolveRules();

	UFUNCTION()
	void OnRep_Downed();

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data")
	TSoftObjectPtr<ULLKLanternData> LanternData =
		TSoftObjectPtr<ULLKLanternData>(FSoftObjectPath(TEXT("/Game/Data/DA_Lantern.DA_Lantern")));

	// The declared fallbacks, matching `lantern.json` `knockdown`. Two of them
	// are zero and that is the authored value, not an absence - see the header.
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data", meta = (ClampMin = "0"))
	float FallbackDownedWalkStuds = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data", meta = (ClampMin = "0"))
	float FallbackBleedOutSeconds = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data")
	bool bFallbackLanternForcedOff = true;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data")
	bool bFallbackDropsCarriedCrate = false;

	UPROPERTY(ReplicatedUsing = OnRep_Downed)
	bool bDowned = false;

	UPROPERTY(VisibleInstanceOnly, Category = "LLK|Vitals")
	float DownedWalkStuds = 0.f;

	UPROPERTY(VisibleInstanceOnly, Category = "LLK|Vitals")
	float BleedOutSeconds = 0.f;

	UPROPERTY(VisibleInstanceOnly, Category = "LLK|Vitals")
	bool bLanternForcedOffWhenDowned = true;

	UPROPERTY(VisibleInstanceOnly, Category = "LLK|Vitals")
	bool bDropsCarriedCrate = false;

	UPROPERTY(VisibleInstanceOnly, Category = "LLK|Vitals")
	ELLKDataSource RulesSource = ELLKDataSource::Unresolved;

	/** Not replicated: every machine can count it from the moment the state arrives. */
	float SecondsDowned = 0.f;
};

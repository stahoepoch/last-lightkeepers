// `lantern.json` -> ULLKLanternData. The keeper's own numbers.
//
// Several values here are deliberately 0 and must not be "fixed" by anyone who
// reads a zero as unset: `knockdown.downedWalkSpeed` is 0 because a downed
// keeper cannot move at all, and `knockdown.bleedOutSeconds` is 0 because
// downed keepers wait indefinitely rather than dying on a timer. That is why
// the family-specific fields on FLLKEnemyDef are TOptional and these are not -
// here, zero IS the value.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LLKLanternData.generated.h"

USTRUCT(BlueprintType)
struct FLLKLanternChargeState
{
	GENERATED_BODY()

	/** 0 = the lantern does not deplete on a timer in v1. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charge")
	float SecondsOfLightPerFullCharge = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charge")
	bool bAutoOffAtEmpty = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charge")
	bool bRechargeAtRelay = false;
};

USTRUCT(BlueprintType)
struct FLLKKnockdownRules
{
	GENERATED_BODY()

	/** 0: a downed keeper cannot crawl. Deliberate - see the header note. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockdown")
	float DownedWalkSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockdown")
	bool bLanternForcedOffWhenDowned = true;

	/**
	 * False, and load-bearing for row 59. With DownedWalkSpeed at 0, a downed
	 * carrier cannot move and keeps the crate, so a crate cannot be banked
	 * while its carrier is on the floor. That is what physically enforces
	 * "everyone protects the carrier". Setting this true would let a crew
	 * abandon the carrier and still extract the crate.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockdown")
	bool bDropsCarriedCrate = false;

	/** 0 = downed keepers wait indefinitely. The wipe comes from the whole crew going down. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockdown")
	float BleedOutSeconds = 0.f;
};

USTRUCT(BlueprintType)
struct FLLKReviveRules
{
	GENERATED_BODY()

	/** Server-validated hold. Moving out of range cancels it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Revive")
	float TeammateHoldSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Revive")
	float MaxReviveDistance = 0.f;
};

USTRUCT(BlueprintType)
struct FLLKSweeperContactRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contact")
	bool bKnockdownOnContact = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contact")
	float ContactRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contact")
	float ContactCooldownSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contact")
	int32 HitsToKnockDown = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contact")
	float HitDecaySeconds = 0.f;
};

USTRUCT(BlueprintType)
struct FLLKMovementRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float CrouchSpeed = 0.f;
};

UCLASS(BlueprintType)
class LLK_API ULLKLanternData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern")
	FLLKLanternChargeState Charge;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern")
	FLLKKnockdownRules Knockdown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern")
	FLLKReviveRules Revive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern")
	FLLKSweeperContactRules SweeperContact;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern")
	FLLKMovementRules Movement;
};

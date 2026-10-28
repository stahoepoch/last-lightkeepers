// `loot.json` -> ULLKLootRulesData. Everything in the file except `tiers`,
// which is a DataTable of FLLKLootTierDef.
//
// This file is one of the two that are BOTH shapes - a keyed collection that
// becomes rows, plus singleton blocks that become an asset. See
// `docs/DATA-PIPELINE.md`.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/LLKDataTypes.h"
#include "LLKLootRulesData.generated.h"

/** `rules` - three statements that define what light is for. */
USTRUCT(BlueprintType)
struct FLLKLootVisibilityRules
{
	GENERATED_BODY()

	/** Row 5: going dark must never mean finding nothing at all. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
	bool bBaselineLootAlwaysVisible = true;

	/**
	 * Row 53, and the implementation note that matters: this gates
	 * LEGIBILITY, not existence. The relic renders in darkness; its value
	 * does not. An invisible relic cannot tempt anyone, which would turn the
	 * signature decision of the game into a hidden-object pass.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
	bool bRelicsRequireLanternLight = true;

	/** False: crates are carried in the world, which is what makes them a crew job. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
	bool bRestorationCratesArePocketable = false;
};

/** `burden` - what carrying costs in speed. */
USTRUCT(BlueprintType)
struct FLLKBurdenRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Burden")
	float SpeedPenaltyPerPoint = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Burden")
	float MaxSpeedPenalty = 0.f;
};

/**
 * `banking` - what survives a wipe.
 *
 * Both keep-fractions are 0.0 and that is the rule, not a placeholder: every
 * item recovered during a run requires extraction, relays never insure a haul,
 * and no paid item may bypass this. Row 59 inherits it - a wipe pays nobody.
 */
USTRUCT(BlueprintType)
struct FLLKBankingRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Banking")
	int32 RelayBanksPerRun = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Banking")
	float BankedKeptOnWipe = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Banking")
	float UnbankedKeptOnWipe = 0.f;

	/** True: disconnecting is not an escape hatch from the loss rule. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Banking")
	bool bDisconnectUsesSameRule = true;
};

/** `extraction.*` - one exit. Keyed by name in the JSON. */
USTRUCT(BlueprintType)
struct FLLKExtractionPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extraction")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extraction")
	ELLKExtractionReliability Reliability = ELLKExtractionReliability::High;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extraction")
	float PayoutModifier = 1.f;

	/** The hold. Never zero - an exit that costs nothing is not a decision. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extraction")
	float ArrivalSeconds = 0.f;

	/** Optional: a rung or restoration that must be done first. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extraction")
	FName Requires;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extraction")
	FString Note;
};

UCLASS(BlueprintType)
class LLK_API ULLKLootRulesData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	FLLKLootVisibilityRules Rules;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	FLLKBurdenRules Burden;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	FLLKBankingRules Banking;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TArray<FLLKExtractionPoint> Extraction;
};

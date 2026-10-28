// `storm-charge.json` -> ULLKStormChargeData. The clock the whole game runs on.
//
// Row 6: the meter only ever rises. Nothing here is a rate that can go
// negative, and no accessor subtracts.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/LLKDataTypes.h"
#include "LLKStormChargeData.generated.h"

/** `lantern` - how crew light multiplies the baseline rate. */
USTRUCT(BlueprintType)
struct FLLKLanternChargeRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern")
	float PerLanternBonus = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern")
	float MaxMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern")
	ELLKLanternCurve Curve = ELLKLanternCurve::Diminishing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern")
	float DiminishingDecay = 0.f;
};

/**
 * `run` - how long a night is. Row 48 made the night's window and the run the
 * same thing, which is what closed O1.
 */
USTRUCT(BlueprintType)
struct FLLKRunRules
{
	GENERATED_BODY()

	/** The loading dock. A valid six-minute survival call, never the target. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Run")
	float EarlyExitSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Run")
	float TargetSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Run")
	FLLKSecondsBand TargetBandSeconds;

	/** A safety valve nothing may exceed. Never a goal. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Run")
	float HardCeilingSeconds = 0.f;
};

/** `extractionCall` - what calling extraction costs. */
USTRUCT(BlueprintType)
struct FLLKExtractionCallRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extraction")
	float StormChargeSpike = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extraction")
	bool bConvergeAllPatrols = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extraction")
	float SirenSeconds = 0.f;
};

/**
 * `thresholds[]` - escalation stages, ascending by `value`.
 *
 * Row 54: the Warden is NOT one of these. It was, it was conflated with
 * "serious pursuit" in both the data and the tests, and splitting it out is
 * what exposed that. Anything reading the last threshold expecting the Warden
 * is reading the wrong thing.
 */
USTRUCT(BlueprintType)
struct FLLKStormThreshold
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threshold")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threshold")
	float Value = 0.f;

	// The three arrival times below are derived from Value and published so a
	// designer can see them without running the Node suite. They are not
	// inputs: changing one without changing Value makes the asset lie.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threshold|Derived")
	float DarkSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threshold|Derived")
	float RegularUseSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threshold|Derived")
	float OneLanternSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threshold")
	FString Effect;
};

/**
 * `warden` - its own dampened meter, split from the thresholds by row 54.
 *
 *   WardenMultiplier = 1 + LightSensitivity * (LanternMultiplier - 1)
 *
 * LightSensitivity is 0.3, derived to hit the stated arrival targets rather
 * than chosen. The dampening is the whole point: without it a lit crew met the
 * Warden at 4:15, and players would correctly optimise toward darkness.
 */
USTRUCT(BlueprintType)
struct FLLKWardenRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warden")
	float ChargeValue = 0.f;

	/** Row 55: the `normal` 25%-duty profile must land inside this band. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warden")
	FLLKSecondsBand ArrivalBandSeconds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warden")
	float LightSensitivity = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warden")
	FString Effect;
};

/** `anchorRevive` - the solo keeper's one way back up. */
USTRUCT(BlueprintType)
struct FLLKAnchorReviveRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anchor")
	int32 ChargesPerRun = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anchor")
	float ReviveSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anchor")
	bool bConsumesFullLanternCharge = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anchor")
	float StormChargeSpike = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anchor")
	bool bSoloOnly = false;
};

UCLASS(BlueprintType)
class LLK_API ULLKStormChargeData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storm")
	float BaselineRatePerSecond = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storm")
	FLLKLanternChargeRules Lantern;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storm")
	FLLKRunRules Run;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storm")
	FLLKExtractionCallRules ExtractionCall;

	/** Ascending by Value. Two entries today; the Warden is not among them. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storm")
	TArray<FLLKStormThreshold> Thresholds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storm")
	FLLKWardenRules Warden;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storm")
	FLLKAnchorReviveRules AnchorRevive;
};

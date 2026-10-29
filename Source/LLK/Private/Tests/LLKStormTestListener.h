// An event counter for LLKStormChargeTest.
//
// A UCLASS in its own header for the same reason `ULLKLanternTestListener` is:
// both storm delegates are BlueprintAssignable, which is deliberate - the
// things that react to a threshold are presentation and spawning, and some of
// that will be Chris's Blueprints. A dynamic delegate has no `AddLambda`; it
// binds to a UFUNCTION, and UHT needs that UFUNCTION in a header.
//
// It counts rather than records, because "fired exactly once" is the property
// worth asserting. A threshold that announces itself twice would spawn a
// patrol escalation twice, and the player would be told the same thing
// happened twice for one crossing.

#pragma once

#include "CoreMinimal.h"
#include "LLKStormTestListener.generated.h"

UCLASS()
class ULLKStormTestListener : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void OnThresholdCrossed(FName ThresholdId, float Charge)
	{
		++Counts.FindOrAdd(ThresholdId);
		LastCharge = Charge;
	}

	UFUNCTION()
	void OnWardenConverged()
	{
		++WardenCount;
	}

	int32 CountFor(FName ThresholdId) const
	{
		const int32* Found = Counts.Find(ThresholdId);
		return Found != nullptr ? *Found : 0;
	}

	TMap<FName, int32> Counts;
	float LastCharge = 0.f;
	int32 WardenCount = 0;
};

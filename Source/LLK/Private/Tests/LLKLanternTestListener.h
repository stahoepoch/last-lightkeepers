// A broadcast counter for LLKLanternTest.
//
// It is a UCLASS in its own header because the delegate is
// BlueprintAssignable - which is deliberate, since M1.3's light actor is
// Chris's and may well be a Blueprint reacting to state. A dynamic delegate
// needs a UFUNCTION to bind to, and UHT needs that UFUNCTION in a header.
//
// Presentation reacting to a delegate is not "Blueprint logic creeping in"
// (ROADMAP's named risk). The rule stays in C++; only the light responds.

#pragma once

#include "CoreMinimal.h"
#include "Lantern/LLKLanternComponent.h"
#include "LLKLanternTestListener.generated.h"

UCLASS()
class ULLKLanternTestListener : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void OnChanged(bool bIsOn, ELLKLanternChangeReason Reason)
	{
		++Count;
		LastOn = bIsOn;
		LastReason = Reason;
	}

	int32 Count = 0;
	bool LastOn = false;
	ELLKLanternChangeReason LastReason = ELLKLanternChangeReason::Replicated;
};

// A broadcast counter for LLKRelicTest.
//
// A UCLASS in its own header for the same reason the lantern's and the
// storm's listeners are: the delegate is BlueprintAssignable, deliberately,
// because the second channel row 16 asks for - the sound of a relic becoming
// legible, a flash on the shelf - is `Content/` and Chris's (row 41), and he
// may well hang a Blueprint on it. A dynamic delegate binds to a UFUNCTION,
// and UHT needs that UFUNCTION in a header.

#pragma once

#include "CoreMinimal.h"
#include "Loot/LLKRelicComponent.h"
#include "LLKRelicTestListener.generated.h"

UCLASS()
class ULLKRelicTestListener : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void OnChanged(bool bIsLegible, ELLKRelicChangeReason Reason)
	{
		++Count;
		LastLegible = bIsLegible;
		LastReason = Reason;
	}

	UFUNCTION()
	void OnInteractable(bool bIsInteractable, ELLKRelicInteractBlock Block)
	{
		++InteractCount;
		LastInteractable = bIsInteractable;
		LastBlock = Block;
	}

	int32 Count = 0;
	bool LastLegible = false;
	ELLKRelicChangeReason LastReason = ELLKRelicChangeReason::Replicated;

	int32 InteractCount = 0;
	bool LastInteractable = false;
	ELLKRelicInteractBlock LastBlock = ELLKRelicInteractBlock::None;
};

// A placeable, replicated relic for authored maps.
//
// The harness used to create anonymous StaticMeshActors and attach a relic
// component at runtime. That made the rule testable, but it left an authored
// map with no honest way to place the same pickup. This actor is that bridge:
// its visible placeholder and its identity live together in the level, while
// ULLKRelicComponent remains the sole owner of light gating, takeability and
// replicated state.

#pragma once

#include "CoreMinimal.h"
#include "Data/LLKDataTypes.h"
#include "GameFramework/Actor.h"
#include "LLKRelicActor.generated.h"

class ULLKRelicComponent;
class UStaticMeshComponent;

UCLASS(BlueprintType)
class LLK_API ALLKRelicActor : public AActor
{
	GENERATED_BODY()

public:
	ALLKRelicActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	/** The component which owns every gameplay rule for this placed relic. */
	UFUNCTION(BlueprintPure, Category = "LLK|Loot")
	ULLKRelicComponent* GetRelic() const { return Relic; }

	/** A visible, non-colliding placeholder until an authored relic mesh replaces it. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LLK|Loot")
	TObjectPtr<UStaticMeshComponent> Visual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LLK|Loot")
	TObjectPtr<ULLKRelicComponent> Relic;

	/** Stable id for telemetry and future loot tables. Unique inside one map. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "LLK|Loot")
	FName RelicId = TEXT("Relic");

	/** Revealed only after the lantern has made this object legible. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "LLK|Loot")
	FText RelicName;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "LLK|Loot", meta = (ClampMin = "0"))
	int32 Value = 300;

	/** Relics normally use LanternOnly; Always is reserved for baseline loot. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "LLK|Loot")
	ELLKLootVisibility Visibility = ELLKLootVisibility::LanternOnly;

private:
	/** Keeps the component's runtime identity in sync with editor-authored fields. */
	void ApplyDefinition();
};

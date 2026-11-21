#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LLKStoreElevator.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class APawn;

/**
 * A two-stop department-store elevator. Entering the cab calls the opposite
 * stop after a short, interrupt-proof travel beat; it is intentionally a
 * single-player chapter connector, not a fake static shaft.
 */
UCLASS()
class LLK_API ALLKStoreElevator : public AActor
{
	GENERATED_BODY()

public:
	ALLKStoreElevator();
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "LLK|Elevator")
	bool IsTravelling() const { return bTravelling; }

	UFUNCTION(BlueprintPure, Category = "LLK|Elevator")
	FVector GetLowerStop() const { return LowerStop; }

	UFUNCTION(BlueprintPure, Category = "LLK|Elevator")
	FVector GetUpperStop() const { return UpperStop; }

private:
	UFUNCTION()
	void OnCabOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	void CompleteTravel();
	bool CanUse(const APawn* Pawn) const;

	UPROPERTY(VisibleAnywhere, Category = "LLK|Elevator")
	TObjectPtr<UBoxComponent> CabTrigger;

	UPROPERTY(VisibleAnywhere, Category = "LLK|Elevator")
	TObjectPtr<UStaticMeshComponent> CabVisual;

	UPROPERTY(EditInstanceOnly, Category = "LLK|Elevator")
	FVector LowerStop = FVector(0.f, 0.f, 0.f);

	UPROPERTY(EditInstanceOnly, Category = "LLK|Elevator")
	FVector UpperStop = FVector(0.f, 0.f, 660.f);

	UPROPERTY(EditInstanceOnly, Category = "LLK|Elevator", meta = (ClampMin = "0.1"))
	float TravelSeconds = 2.5f;

	UPROPERTY(Replicated)
	bool bTravelling = false;

	TWeakObjectPtr<APawn> Passenger;
	FVector PendingStop = FVector::ZeroVector;
	FTimerHandle TravelTimer;
};

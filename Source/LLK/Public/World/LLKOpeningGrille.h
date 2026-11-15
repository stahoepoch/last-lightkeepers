#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LLKOpeningGrille.generated.h"

class APawn;

/** The opening's one lantern read and the grille it releases. Map pieces are found by tags. */
UCLASS()
class LLK_API ALLKOpeningGrille : public AActor
{
	GENERATED_BODY()

public:
	ALLKOpeningGrille();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** The exact reach, aim and obstruction question shared by the prompt and E. */
	bool CanFocus(const APawn* Keeper) const;
	/** A focused release still needs the issued lantern lit. */
	bool CanRelease(const APawn* Keeper) const;
	bool TryRelease(const APawn* Keeper);
	bool IsOpen() const { return bOpen; }
	FVector GetReadPoint() const;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Opening", meta = (ClampMin = "0"))
	float ReadReachCm = 220.f;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Opening", meta = (ClampMin = "0", ClampMax = "1"))
	float ReadAimDot = 0.85f;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Opening", meta = (ClampMin = "1"))
	float RaiseCm = 330.f;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Opening", meta = (ClampMin = "0.1"))
	float RaiseSeconds = 2.4f;

	UPROPERTY(Replicated)
	bool bOpen = false;

	TWeakObjectPtr<AActor> Witness;
	TWeakObjectPtr<AActor> ReleaseBar;
	TArray<TWeakObjectPtr<AActor>> Segments;
	TArray<FVector> ClosedLocations;
	float OpenAlpha = 0.f;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LLKLanternPickup.generated.h"

class ALLKCharacter;
class UStaticMeshComponent;

/**
 * The one lantern, lying in the world until somebody takes it.
 *
 * **Why this exists.** The pawn has carried a lantern since it was first
 * written: switched off, but in hand from the first frame and toggleable
 * immediately. The story says the child starts without one and finds his
 * father's, so "having a lantern" had to become a thing that happens rather
 * than a thing that is true.
 *
 * **What this deliberately does NOT do.** It does not decide how it is
 * taken. There is no overlap volume, no prompt, no interactor focus type and
 * no auto-grant, because the pickup, grille and release interfaces are being
 * agreed between the runtime and the map before either side connects them -
 * and a speculative trigger written now is one that has to be removed later
 * by someone who does not know it was speculative. `GrantTo` is the whole
 * surface; something else calls it.
 *
 * It also takes no position on WHERE it sits. The opening is unsettled - the
 * lantern may end up on the lower entrance tile or on the upper landing -
 * and nothing in this class cares which.
 */
UCLASS()
class LLK_API ALLKLanternPickup : public AActor
{
	GENERATED_BODY()

public:
	ALLKLanternPickup();

	/**
	 * Give this lantern to a keeper. Server only, and once only.
	 *
	 * Returns true if the keeper did not have a lantern and now does. False
	 * covers every other case - no authority, no keeper, no lantern
	 * component, already taken, or a keeper who already had one - and the
	 * caller is expected to read it rather than assume. A pickup that
	 * reports success it did not achieve is the failure this project has
	 * spent a day removing from three other scripts.
	 */
	UFUNCTION(BlueprintCallable, Category = "LLK|Lantern")
	bool GrantTo(ALLKCharacter* Keeper);

	/** True once somebody has taken it. */
	UFUNCTION(BlueprintPure, Category = "LLK|Lantern")
	bool IsTaken() const { return bTaken; }

protected:
	/**
	 * The lantern as it lies. Visual only - this is a prop on the floor, not
	 * a light source, and it does not enter the gameplay light field. A
	 * lantern that lit the room before it was picked up would answer the
	 * question the opening is built to ask.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LLK|Lantern")
	TObjectPtr<UStaticMeshComponent> Body;

	/**
	 * Replicated so a client cannot draw a lantern on the floor that the
	 * server knows is already in somebody's hand.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_Taken)
	bool bTaken = false;

	UFUNCTION()
	void OnRep_Taken();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

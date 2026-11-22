// The breaker that brings Boswell's back on.
//
// The second thing to use the `ILLKInteractable` seam, which is the point of
// having built it: a pushable and a breaker share nothing except that a
// keeper can press E on them, and neither required a line of change in
// `ULLKInteractorComponent` or in the HUD.
//
// **One way, deliberately.** Throwing it on is a decision with a price; being
// able to throw it back off would let a player undo the price and keep the
// light. There is no stealth value in darkness you can restore at will.
//
// **The price is noise, and it is the largest in the game.** A whole floor's
// lighting striking at once is the building itself waking up, not a local
// event, so `ULLKPowerSubsystem::PowerOnNoiseStuds` is 70 against a pushed
// fixture's 16. Every machine on the floor hears it and comes to look. The
// player gets the store lit and pays by telling everything in it where they
// are, which is the same economy as every other verb in this chapter, one
// size larger.
//
// It keeps making that noise for `NoiseSeconds` after the throw rather than
// for one frame, because a sound a listener has to be sampling on exactly the
// right tick is a sound that works by luck.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/LLKNoiseSource.h"
#include "GameFramework/Actor.h"
#include "World/LLKInteractable.h"

#include "LLKBreakerPanel.generated.h"

class UStaticMeshComponent;

UCLASS()
class LLK_API ALLKBreakerPanel : public AActor, public ILLKInteractable, public ILLKNoiseSource
{
	GENERATED_BODY()

public:
	ALLKBreakerPanel();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// ---- ILLKInteractable -------------------------------------------------

	virtual bool CanFocus(const APawn* Keeper) const override;
	virtual FText FocusVerb() const override;
	virtual void BeginInteract(APawn* Keeper) override;
	virtual void EndInteract(APawn* Keeper) override;

	// ---- ILLKNoiseSource --------------------------------------------------

	virtual bool GetCurrentNoise(LLKHearing::FLLKNoise& OutNoise) const override;

	/** Has it been thrown. Once true it never goes back. */
	UFUNCTION(BlueprintPure, Category = "LLK|Power")
	bool IsThrown() const { return bThrown; }

	/**
	 * How high up the panel the handle is, in studs.
	 *
	 * **A breaker panel is mounted for an adult**, which is the whole reason
	 * this number is authored rather than taken from the actor origin. At 4.5
	 * studs - 126 cm - it sits right at a 145 cm child's eye height: reachable
	 * standing, and the level author can raise it to make the player find
	 * something to climb first. That is the push and climb verbs becoming a
	 * puzzle rather than two demonstrations.
	 */
	UPROPERTY(EditAnywhere, Category = "LLK|Power", meta = (ClampMin = "0"))
	float HandleHeightStuds = 4.5f;

	/** How close the keeper's eye must be, in cm. The child's reach again. */
	UPROPERTY(EditAnywhere, Category = "LLK|Power", meta = (ClampMin = "0"))
	float ReachCm = 150.f;

	/** Cosine of the half-angle. 0.8 is about 37 degrees off centre. */
	UPROPERTY(EditAnywhere, Category = "LLK|Power", meta = (ClampMin = "-1", ClampMax = "1"))
	float MinAimDot = 0.8f;

	/** How long the building keeps making noise after the throw. */
	UPROPERTY(EditAnywhere, Category = "LLK|Power", meta = (ClampMin = "0"))
	float NoiseSeconds = 6.f;

	/** Where the keeper's hand goes. The focus point, not the actor origin. */
	UFUNCTION(BlueprintPure, Category = "LLK|Power")
	FVector GetHandlePoint() const;

protected:
	UPROPERTY(VisibleAnywhere, Category = "LLK|Power")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(Replicated)
	bool bThrown = false;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Server: counts down the window during which the building is loud. */
	float NoiseRemaining = 0.f;
};

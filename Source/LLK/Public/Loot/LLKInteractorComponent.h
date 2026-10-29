// What the keeper is looking at. The thing the HUD and the take key must agree on.
//
// **Why this is a component and not two searches.** Chris, 2026-10-29, after
// the first playable harness: *"i can see how much its worth, but there isn't
// anything on screen to guide me... in arc raiders it shows the item and E is
// take."* Fixing that needs the prompt and the take key to be talking about
// the same object - and the fastest wrong way to build it is a search in the
// HUD and another in the input handler, which disagree the moment two relics
// are nearly equidistant. Then the prompt says one thing and E takes another,
// which is the kind of bug players describe as "the game didn't take what I
// was looking at" and nobody can reproduce.
//
// So focus is resolved once, here, and both read it. The same argument as the
// light field: one definition of "how lit is this spot" rather than two.
//
// **It is reach AND aim, not reach alone.** Nearest-within-radius picks the
// relic behind you when you are standing between two shelves. A keeper
// looking at a thing is the thing they mean, so the dot product against the
// camera's forward is part of the test rather than a refinement of it.
//
// **Everything it does is local.** Which object has focus is a question about
// this player's camera, so it is answered on this machine; taking is a server
// RPC, because what leaves a shelf is the world's business and not a client's.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LLKInteractorComponent.generated.h"

class ULLKRelicComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLLKFocusChanged, ULLKRelicComponent*, Focused);

UCLASS(ClassGroup = (LLK), meta = (BlueprintSpawnableComponent))
class LLK_API ULLKInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULLKInteractorComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** The relic the keeper is looking at, or null. Read by the HUD and by the take key alike. */
	UFUNCTION(BlueprintPure, Category = "LLK|Interact")
	ULLKRelicComponent* GetFocused() const { return Focused; }

	/** Fires when focus moves to a different object, or away from everything. */
	UPROPERTY(BlueprintAssignable, Category = "LLK|Interact")
	FLLKFocusChanged OnFocusChanged;

	/**
	 * Asks to take whatever has focus.
	 *
	 * The client asks; the server decides and removes it. A client that could
	 * take locally could empty a shelf it was not allowed to touch, and the
	 * light gate on taking (row 62) would be advice rather than a rule.
	 */
	UFUNCTION(BlueprintCallable, Category = "LLK|Interact")
	void TryTakeFocused();

	/** How many relics this keeper is carrying. Replicated so a HUD can show it. */
	UFUNCTION(BlueprintPure, Category = "LLK|Interact")
	int32 GetCarriedCount() const { return CarriedCount; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Reach in STUDS. Converted at the boundary, row 61. */
	UPROPERTY(EditAnywhere, Category = "LLK|Interact", meta = (ClampMin = "0"))
	float ReachStuds = 9.f;

	/**
	 * How closely the keeper must be looking at it. Cosine of the half-angle.
	 *
	 * 0.75 is about 41 degrees off centre, which is generous enough that
	 * reaching for a thing works without aiming at it precisely, and tight
	 * enough that a relic behind your shoulder is never what E takes.
	 */
	UPROPERTY(EditAnywhere, Category = "LLK|Interact", meta = (ClampMin = "-1", ClampMax = "1"))
	float MinAimDot = 0.75f;

protected:
	UFUNCTION(Server, Reliable)
	void ServerTake(ULLKRelicComponent* Relic);

	/** Picks the best focus candidate from reach and aim together. */
	ULLKRelicComponent* FindFocus() const;

	UPROPERTY()
	TObjectPtr<ULLKRelicComponent> Focused;

	UPROPERTY(Replicated)
	int32 CarriedCount = 0;

	float SecondsSinceSearch = 0.f;

	/** Focus is resolved a few times a second, not every frame. A prompt does not need 60Hz. */
	UPROPERTY(EditAnywhere, Category = "LLK|Interact", meta = (ClampMin = "0.01"))
	float SearchIntervalSeconds = 0.08f;
};

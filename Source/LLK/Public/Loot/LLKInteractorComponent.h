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

class ALLKExtractionZone;
class ULLKRelicComponent;
class ULLKVitalsComponent;

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

	/**
	 * The downed teammate in reach, or null. M2.2.
	 *
	 * Resolved beside the relic focus and in the same pass, because a keeper
	 * standing over a fallen crewmate with a brooch on the shelf behind them
	 * must not have the prompt flicker between the two. A body outranks loot:
	 * if somebody is down in front of you, that is what E means.
	 */
	UFUNCTION(BlueprintPure, Category = "LLK|Interact")
	ULLKVitalsComponent* GetFocusedCasualty() const { return FocusedCasualty; }

	/**
	 * The extraction zone this keeper is standing in, or null. M3.
	 *
	 * Resolved locally and by overlap rather than by aim, because standing in
	 * a bay is not something you point at. It is the lowest-priority focus:
	 * a casualty, then a relic, then the zone - so a keeper reaching for a
	 * brooch on the dock floor takes the brooch, and looks away to call.
	 */
	UFUNCTION(BlueprintPure, Category = "LLK|Interact")
	ALLKExtractionZone* GetFocusedZone() const { return FocusedZone; }

	/** Press: begin a hold, or spend the anchor revive if this keeper is the one down. */
	UFUNCTION(BlueprintCallable, Category = "LLK|Interact")
	void BeginInteract();

	/** Release: a hold that is let go is a hold that cancels. */
	UFUNCTION(BlueprintCallable, Category = "LLK|Interact")
	void EndInteract();

	/** How many relics this keeper is carrying. Replicated so a HUD can show it. */
	UFUNCTION(BlueprintPure, Category = "LLK|Interact")
	int32 GetCarriedCount() const { return CarriedCount; }

	/**
	 * Row 8, on a crew wipe: the night keeps what you carried.
	 *
	 * Server only, and named for the one caller it has rather than left as a
	 * general setter - a carried count that anything could zero is a carried
	 * count that something eventually will.
	 */
	void ClearCarriedOnWipe();

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

	UFUNCTION(Server, Reliable)
	void ServerBeginRevive(ULLKVitalsComponent* Target);

	UFUNCTION(Server, Reliable)
	void ServerCancelRevive(ULLKVitalsComponent* Target);

	/**
	 * Server. Spends this keeper's own anchor revive and pays the meter.
	 *
	 * The price is applied here rather than in the vitals component, and that
	 * split is the escalation guard's doing: the knockdown path must not be
	 * able to reach the storm at all, so the component that owns "the player
	 * chose this" is the one that pays for it.
	 */
	UFUNCTION(Server, Reliable)
	void ServerAnchorRevive();

	/**
	 * Server. Starts the hold that calls extraction, and the hold is a confirmation.
	 *
	 * Held on the server for the reason the revive is: a client that could
	 * decide a hold had completed could complete it instantly. The whole
	 * price of calling is the siren and the convergence, so there is nothing
	 * here worth cheating - which is exactly when a project drifts into
	 * trusting clients about one thing and then about another.
	 */
	UFUNCTION(Server, Reliable)
	void ServerBeginExtractionCall(ALLKExtractionZone* Zone);

	UFUNCTION(Server, Reliable)
	void ServerCancelExtractionCall();

	/** Server. One step of the call hold; cancels itself if the keeper leaves or falls. */
	void TickExtractionCall(float DeltaSeconds);

	/** The zone whose trigger this keeper is inside, locally. */
	ALLKExtractionZone* FindZone() const;

	/** Finds the nearest downed teammate in reach, by the same aim rule as loot. */
	ULLKVitalsComponent* FindCasualty() const;

	/** Picks the best focus candidate from reach and aim together. */
	ULLKRelicComponent* FindFocus() const;

	UPROPERTY()
	TObjectPtr<ULLKRelicComponent> Focused;

	UPROPERTY()
	TObjectPtr<ULLKVitalsComponent> FocusedCasualty;

	UPROPERTY()
	TObjectPtr<ALLKExtractionZone> FocusedZone;

	/** Who we asked the server to revive, so releasing E cancels the right hold. */
	UPROPERTY()
	TObjectPtr<ULLKVitalsComponent> Holding;

	/** Server only: the zone being called from, and how long E has been down. */
	UPROPERTY()
	TObjectPtr<ALLKExtractionZone> CallingZone;

	float CallHeldSeconds = 0.f;

	UPROPERTY(Replicated)
	int32 CarriedCount = 0;

	float SecondsSinceSearch = 0.f;

	/** Focus is resolved a few times a second, not every frame. A prompt does not need 60Hz. */
	UPROPERTY(EditAnywhere, Category = "LLK|Interact", meta = (ClampMin = "0.01"))
	float SearchIntervalSeconds = 0.08f;

	/**
	 * How long E must be held to call extraction.
	 *
	 * **A confirmation, not a cost**, and the distinction is why this is not
	 * in `data/`: the cost of calling is the siren, the convergence and the
	 * hold, all of which Chris authored. This exists so that a keeper fumbling
	 * for a relic in the bay cannot summon a recovery rig by brushing a key,
	 * which is the single most annoying way to lose a run. If it ever becomes
	 * a tuned part of the decision rather than a safeguard against a misclick,
	 * it belongs beside the exit in `loot.json` and this should move.
	 */
	UPROPERTY(EditAnywhere, Category = "LLK|Interact", meta = (ClampMin = "0"))
	float CallHoldSeconds = 1.f;
};

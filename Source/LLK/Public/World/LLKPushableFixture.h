// A shop fixture a child can shove along the floor.
//
// **The first verb in this game that changes the building.** Everything the
// keeper could do until now either took something (a relic, the lantern),
// helped somebody (a revive), or opened a thing an author had already decided
// was openable (the grille). None of them moved the world. `ROADMAP.md`'s
// chapter goal 5 asks the player to *"find a story clue, change part of the
// building, and reach Sayers"*, and slice 4 asks for environmental puzzles.
// This is the smallest honest piece of that.
//
// **Why a child pushes rather than lifts, and why that is the design and not
// a limitation.** The keeper's eye is 125 cm up and their reach is 150 cm of
// slant range. `LLKStoreFloorPlan` authors `ShelfHeightStuds = 6` - 168 cm -
// and `CounterHeightStuds = 3.2` - 89.6 cm. **The top shelf of every fixture
// in Boswell's is already out of reach by 18 cm, today, and nobody designed
// that as a puzzle.** The store was authored at adult scale and then the
// keeper was scaled to a child, and the gap between those two decisions is
// free level design nobody has spent.
//
// So the answer to a thing you cannot reach is a thing you can move. A
// fixture pushed under a shelf is 89.6 cm of floor you can stand on, and
// 89.6 + 125 puts a child's eye above the shelf they could not see onto. That
// chain - cannot reach, so move something, so climb it - is the loop, and
// this actor is its first link. The climb is not built yet; this does not
// pretend otherwise, and a fixture with nothing above it is still a thing you
// can shove into a doorway or out of a sightline.
//
// **It is loud, and that is what makes it a decision.** Pushing is the most
// audible act in the chapter - `LLKPushFeel::PushNoiseStuds` is 16 against a
// sprint's 12.5 - and `ILLKNoiseSource` is how a Sweeper finds out. A puzzle
// you can solve at leisure is a chore. A puzzle that takes eleven seconds and
// announces every one of them to a machine that investigates sounds is the
// thing Chris asked for when he said the scare is not there yet.
//
// **Server-authoritative, one replicated float.** `TravelStuds` is the entire
// state: where on its authored rail the fixture sits. It cannot leave the
// rail, cannot rotate, and cannot be anywhere its author did not look at.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/LLKNoiseSource.h"
#include "GameFramework/Actor.h"
#include "World/LLKInteractable.h"

#include "LLKPushableFixture.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class LLK_API ALLKPushableFixture : public AActor, public ILLKInteractable, public ILLKNoiseSource
{
	GENERATED_BODY()

public:
	ALLKPushableFixture();

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ---- ILLKInteractable -------------------------------------------------

	virtual bool CanFocus(const APawn* Keeper) const override;
	virtual FText FocusVerb() const override;
	virtual void BeginInteract(APawn* Keeper) override;
	virtual void EndInteract(APawn* Keeper) override;

	// ---- ILLKNoiseSource --------------------------------------------------

	virtual bool GetCurrentNoise(LLKHearing::FLLKNoise& OutNoise) const override;

	/** Where on its rail it is, 0 to `TravelStuds`. The whole state. */
	UFUNCTION(BlueprintPure, Category = "LLK|Push")
	float GetTravelStuds() const { return Travel; }

	/** Is somebody moving it this frame. Read by the noise source and the log. */
	UFUNCTION(BlueprintPure, Category = "LLK|Push")
	bool IsMoving() const { return bMovingThisFrame; }

	/**
	 * The direction it slides, in the actor's own space.
	 *
	 * Authored per instance rather than taken from the actor's forward, so a
	 * fixture can be rotated to face the aisle it belongs in without that
	 * also deciding which way it rolls.
	 */
	UPROPERTY(EditAnywhere, Category = "LLK|Push")
	FVector LocalPushAxis = FVector(1.f, 0.f, 0.f);

	/**
	 * How far it can go, in studs. The rail's length.
	 *
	 * Finite on purpose. A fixture that could be pushed indefinitely is a
	 * fixture a player can wedge in a doorway, and a chapter that can be
	 * soft-locked by its own furniture is worse than one with no furniture.
	 */
	UPROPERTY(EditAnywhere, Category = "LLK|Push", meta = (ClampMin = "0"))
	float TravelStuds = 8.f;

	/**
	 * How close the keeper's eye must be, in cm.
	 *
	 * The same 150 cm the lantern pickup uses, and for the same reason: it is
	 * a child's reach measured eye-to-thing, which puts them close enough to
	 * have to come out from behind cover to start pushing.
	 */
	UPROPERTY(EditAnywhere, Category = "LLK|Push", meta = (ClampMin = "0"))
	float ReachCm = 150.f;

	/** Cosine of the half-angle. 0.75 is about 41 degrees off centre. */
	UPROPERTY(EditAnywhere, Category = "LLK|Push", meta = (ClampMin = "-1", ClampMax = "1"))
	float MinAimDot = 0.75f;

protected:
	UPROPERTY(VisibleAnywhere, Category = "LLK|Push")
	TObjectPtr<UStaticMeshComponent> Body;

	/** Server: who is pushing, or null. Weak so a keeper leaving cannot keep it alive. */
	TWeakObjectPtr<APawn> Pusher;

	/**
	 * Replicated, because a fixture that only moved on the server is a wall
	 * clients can walk through and a listener they cannot see the cause of.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_Travel)
	float Travel = 0.f;

	UFUNCTION()
	void OnRep_Travel();

	/** Set every tick by the push step. Not replicated: each side derives it. */
	bool bMovingThisFrame = false;

	/** Where the rail starts, captured once so travel is always measured from it. */
	FVector RailOrigin = FVector::ZeroVector;

	/** The authored axis in world space, normalised, captured with the origin. */
	FVector WorldAxis = FVector(1.f, 0.f, 0.f);

	/** Puts the body where `Travel` says it is. The only writer of the transform. */
	void ApplyTravel();

	/** Said once at BeginPlay, affirmatively, naming the rail it will slide on. */
	bool bReported = false;
};

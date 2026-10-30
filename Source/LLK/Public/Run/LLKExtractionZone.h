// The extraction zone: a volume and a list of who is standing in it. M3.
//
// **Chris's scope, and it is the whole of it:** *"It should own only the
// trigger volume and which keepers are currently inside, feeding the state
// machine's pause/advance rule. No dock mesh, no authored map placement, no
// recovery-rig visuals yet."* So this actor answers one question and refuses
// to answer any other. Both of `loot.json`'s exits then use the same tested
// mechanic, and the greybox supplies their real geometry later.
//
// **What it replaces.** Yesterday the subsystem measured the route itself: a
// position and a radius, with the distance test inside
// `ULLKExtractionSubsystem::ObserveBoardingRoute`. That was a placeholder with
// a real problem - a sphere cannot describe a bay mouth, and a rule layer that
// does its own geometry ends up with two of them. The zone owns the volume now
// and the subsystem asks it, which is the same relationship perception has
// with `ILLKVisibilitySource`.
//
// **Why the bookkeeping is a struct and not the actor.** `FLLKZoneOccupancy`
// holds who is inside, with no actor, no component and no world - so every
// rule about it is testable, and what remains in the actor is two overlap
// handlers that translate an engine event into one call each. Overlap wiring
// itself is only exercisable in PIE and is deliberately not claimed as tested;
// everything it could get wrong afterwards is.
//
// **Inside is geometry; standing is a rule.** The struct reports both, because
// they are different facts and only one of them advances a hold: row 75 says a
// downed keeper does not hold the route, since row 74 is explicit that no
// driver comes into the building to collect anybody. A readout that showed
// "nobody in the zone" while three unconscious keepers lay in it would be
// lying about the thing the player most needs to understand.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LLKExtractionZone.generated.h"

class UBoxComponent;
class ULLKVitalsComponent;

/**
 * Who is in a zone, with no engine anywhere near it.
 *
 * Weak pointers for the reason the run state's roll uses them: a keeper's
 * actor can be destroyed while standing in the zone, and an occupancy that
 * kept it alive would hold a hold open with a corpse. Stale entries are
 * dropped when counted rather than tracked, which is the same no-ghost rule
 * the lit-lantern count follows.
 */
USTRUCT(BlueprintType)
struct FLLKZoneOccupancy
{
	GENERATED_BODY()

	/** Idempotent: a keeper counted twice is a keeper who stays after leaving once. */
	void Enter(ULLKVitalsComponent* Keeper);

	void Leave(ULLKVitalsComponent* Keeper);

	/** Everyone in the volume, conscious or not. Geometry. */
	int32 CountInside() const;

	/** Everyone in the volume who could board. Row 75's rule. */
	int32 CountStanding() const;

	/** For a readout: inside but down, which is the state worth showing. */
	int32 CountDowned() const { return CountInside() - CountStanding(); }

	void Clear() { Inside.Reset(); }

private:
	TArray<TWeakObjectPtr<ULLKVitalsComponent>> Inside;
};

/**
 * One exit's boarding route, as a volume in the world.
 *
 * Placed by nobody yet. It registers itself with `ULLKExtractionSubsystem` on
 * `BeginPlay` and takes itself out on `EndPlay`, so a greybox dock is an actor
 * dragged into a level with an id typed into it and nothing else.
 */
UCLASS(Blueprintable)
class LLK_API ALLKExtractionZone : public AActor
{
	GENERATED_BODY()

public:
	ALLKExtractionZone();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Which exit this is the route for - `LoadingDock` or `FreightLift`.
	 *
	 * An `FName` typed in the details panel rather than a dropdown, because the
	 * exits live in `data/loot.json` and a C++ enum of them would be the same
	 * list in two places. A zone naming an exit that nothing authored is
	 * reported by name at `BeginPlay`, which is the cheapest version of the
	 * `LLK.CheckMap` discipline for a typo.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Extraction")
	FName ExtractionPointId;

	UFUNCTION(BlueprintPure, Category = "LLK|Extraction")
	int32 CountKeepersInside() const { return Occupancy.CountInside(); }

	UFUNCTION(BlueprintPure, Category = "LLK|Extraction")
	int32 CountKeepersStanding() const { return Occupancy.CountStanding(); }

	UFUNCTION(BlueprintPure, Category = "LLK|Extraction")
	int32 CountKeepersDowned() const { return Occupancy.CountDowned(); }

	UBoxComponent* GetTrigger() const { return Trigger; }

#if WITH_DEV_AUTOMATION_TESTS
	/**
	 * The overlap handlers' effect, without an overlap.
	 *
	 * The handlers themselves need a world and a moving capsule, which is PIE
	 * work; everything they do afterwards is a call to these.
	 */
	void EnterForTest(ULLKVitalsComponent* Keeper) { Occupancy.Enter(Keeper); }
	void LeaveForTest(ULLKVitalsComponent* Keeper) { Occupancy.Leave(Keeper); }
#endif

protected:
	UFUNCTION()
	void OnTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& Sweep);

	UFUNCTION()
	void OnTriggerEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex);

	/**
	 * Query-only, and overlapping pawns only.
	 *
	 * A box rather than a sphere because a boarding route is a mouth: the
	 * stretch between the bay door and the first shelf, which a radius cannot
	 * describe without either including the aisle behind it or excluding half
	 * the bay.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LLK|Extraction")
	TObjectPtr<UBoxComponent> Trigger;

	FLLKZoneOccupancy Occupancy;
};

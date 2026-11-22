// A floor to stand on, built at runtime. M0.8's harness.
//
// **This exists so somebody can press Play tonight.** M0.8 is the
// milestone's deliberate "a human actually looked at it" step, and it was
// blocked on something nobody should have to make first: a map. `Content/`
// is Chris's alone (row 41), so an agent cannot author one, and waiting for
// a map to exist before anyone can walk is the wrong order - the whole point
// of M0.8 is to catch what tests cannot.
//
// So this GameMode builds its world in code: a collision floor, some
// upright reference so motion is visible, a light so the floor is not black,
// and a PlayerStart above it. No saved map, no Blueprint, no project asset,
// nothing written into `Content/`.
//
// **It is a harness and it should be deleted.** When there is a real
// grey-box store floor (M1.5, Chris's), this class stops being the way to
// play and becomes clutter. It is named, commented and isolated so that
// removing it is one file and one config line.
//
// **On lighting, deliberately and carefully.** `DONTFUCKUPTHELIGHTS.md`
// exists because lighting globals were broken repeatedly by people changing
// one number in good faith. Nothing here touches a project lighting setting,
// a World Settings value, or any saved asset. It spawns two actors into a
// running PIE world and that is all. A dark room would make the harness
// useless, and making the harness visible must not cost the project its
// lighting rules - so it buys visibility the only way that leaves no trace.

#pragma once

#include "CoreMinimal.h"
#include "Data/LLKDataTypes.h"
#include "Harness/LLKStoreFloorPlan.h"
#include "GameFramework/GameModeBase.h"
#include "LLKTestGameMode.generated.h"

class UDirectionalLightComponent;
class USkyLightComponent;
class UStaticMesh;

UCLASS()
class LLK_API ALLKTestGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALLKTestGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;

protected:
	/**
	 * Say out loud how many lantern pickups this world actually has.
	 *
	 * **Why this earns a place in a class marked for deletion.** On
	 * 2026-11-15 Chris played two full sessions looking for a lantern in
	 * `LLK_Boswells_ApparelPass`, which has none - the pickup is only in the
	 * opening map, and `EditorStartupMap` points at the other one. Every
	 * system behaved correctly and the log said so eleven times
	 * ("Lantern stayed off: the keeper does not have one"), but nothing said
	 * the *reason*: there was no lantern in the building. Two sessions and a
	 * log grep to learn a fact the world knew at frame one.
	 *
	 * This is the sentence that was missing. It is deliberately loud in the
	 * empty case and deliberately says what the player will experience
	 * rather than printing a count for somebody to interpret.
	 */
	void ReportLanternPickups();

	/**
	 * Say whether each machine in this world can actually path to the keeper.
	 *
	 * **Why this is not covered by anything already here.** The opening has a
	 * threat in it - a Sweeper, awake, 168 cm/s, noticing a dark keeper at 12
	 * studs and a lit one at 38 - and every component of it reports
	 * correctly whether or not the navmesh connects its patrol to the floor
	 * the child walks on. A machine that cannot reach the player is a horror
	 * game with the monster in another room, and it fails with a clean log.
	 *
	 * This project has met that shape three times: a crate in a recess the
	 * Sweeper could never reach, a lantern 189 cm above a 150 cm reach, and
	 * fixtures whose collision never imported. Each time the system was right
	 * and the geometry was not.
	 *
	 * **Deliberately not `LLKMapReadiness`, which asks a different
	 * question.** That facility asks whether a map is harness-ready - tagged
	 * enemy spawns, test lanes, a route from the player spawn - and on a
	 * dressed map most of its checks would report conventions the map never
	 * adopted. This asks the one runtime question: from where this machine
	 * stands, is there a path to where this keeper stands, and how long does
	 * it take at the speed the machine actually has. (`LLKMapReadiness` has
	 * never been run against a real map at all; that is filed separately
	 * rather than fixed by being quietly called from here.)
	 */
	/** Says what in this map can be pushed, and warns loudly when nothing can. */
	void ReportPushables();

	void ReportThreatReachability();

protected:
	// ------------------------------------------------- M2.3, the storm outside
	/**
	 * The storm's answer to a threshold crossing, in the harness's own sky.
	 *
	 * **This is M2.3's "something observable", and it is deliberately the
	 * smallest honest one.** The roadmap row says crossing `PatrolsAlert` and
	 * `PatrolsHunt` must change something a player can see, and the two
	 * delegates that should cause it - `OnThresholdCrossed` and
	 * `OnWardenConverged` - have broadcast since M1.4 with nothing listening.
	 * This is the listener. It is not the consequence that matters: patrols
	 * tightening is M2.6, and it needs enemies, routes and a navmesh that do
	 * not exist yet. What it buys now is a crossing that is legible without
	 * reading the HUD, which is what makes the meter judgeable by eye.
	 *
	 * **Two honest limitations, both of which are why this lives in the
	 * harness rather than in a gameplay class.** It is server-side, so on a
	 * listen server the host sees the sky change and a client does not -
	 * acceptable in a harness, unacceptable in the shipped version, which
	 * wants a replicated storm actor or a post-process driven from the
	 * snapshot every machine already has. And it is disposable: these two
	 * actors are spawned into a running world and leave nothing behind, per
	 * the lighting note at the top of this file, so when the authored store
	 * arrives this goes with the rest of the harness.
	 *
	 * The numbers are properties rather than constants because "how much
	 * darker" is a thing to judge by eye, not by argument.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Harness|Storm", meta = (ClampMin = "0", ClampMax = "1"))
	float StormDimPerStage = 0.3f;

	/** Where the sky goes as the night worsens - colder and greener, never black. */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Harness|Storm")
	FLinearColor StormSkyColour = FLinearColor(0.3f, 0.42f, 0.4f);

	/** The Warden's own arrival, which is not a third patrol state. */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Harness|Storm", meta = (ClampMin = "0", ClampMax = "1"))
	float WardenDim = 0.75f;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Harness|Storm")
	FLinearColor WardenSkyColour = FLinearColor(0.42f, 0.14f, 0.12f);

	/**
	 * Listens to the delegate M1.4 built and nobody called.
	 *
	 * A UFUNCTION because dynamic delegates have no `AddLambda` - the binding
	 * needs a reflected member, which is the one piece of UHT ceremony this
	 * costs.
	 */
	UFUNCTION()
	void HandleThresholdCrossed(FName ThresholdId, float Pressure);

	UFUNCTION()
	void HandleWardenConverged();

	/** Re-reads the stage and drives the two lights from it. Idempotent. */
	void ApplyStormStage();

	/**
	 * Weak, because the harness does not own these any more than the floor.
	 *
	 * A raw pointer here would keep two spawned actors' components alive past
	 * a world teardown, and this class is created per world.
	 */
	TWeakObjectPtr<UDirectionalLightComponent> HarnessSun;
	TWeakObjectPtr<USkyLightComponent> HarnessSky;
	/** Half-extent of the floor in cm. 50m square: big enough to sprint across and notice. */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Harness")
	float FloorHalfExtent = 2500.f;

	/**
	 * How bright the harness's own sky is. Low, deliberately.
	 *
	 * A box lit well enough to walk around in is a box where the lantern
	 * changes nothing, and the lantern is what this harness is for. Raise
	 * these two to see the geometry; lower them to judge the light. Neither
	 * touches a project lighting value - they are arguments to two actors
	 * spawned into a running world.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Harness", meta = (ClampMin = "0"))
	float HarnessSunIntensity = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Harness", meta = (ClampMin = "0"))
	float HarnessSkyIntensity = 0.1f;

	/**
	 * The ground floor's dimensions, in studs.
	 *
	 * **Chris, 2026-10-29, authorising it:** *"build the ground-floor
	 * department-store scaffold in code, not as a final Unreal map... It is
	 * disposable greybox, not your final Galleria. You play it and say 'this
	 * feels too square', 'this ramp feels wrong', 'I need more
	 * department-store density'."*
	 *
	 * So the plan is one struct of named numbers, each commented with why it
	 * is that number, because the whole point is that he changes them by eye.
	 * `LLKStoreFloorPlan.h` holds the reasoning.
	 *
	 * The previous version of this was four aisles of shelving in a box - a
	 * step up from twelve pillars and still not a shop. This one has a sealed
	 * street front you spawn inside, cosmetics counters, the mirrored columns
	 * that break the long sightline, jewellery cases holding the relics, a
	 * dead escalator climbing to a mezzanine, and a back-of-house corridor
	 * that jogs so it cannot be seen down in one look.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Harness")
	FLLKStoreFloorPlan StoreFloorPlan;

	/** Where M2.6's first enemy may start. Authored by the builder, read by the roadmap row. */
	UFUNCTION(BlueprintPure, Category = "LLK|Harness")
	const TArray<FVector>& GetEnemySpawnPoints() const { return EnemySpawnPoints; }

	UPROPERTY()
	TArray<FVector> EnemySpawnPoints;

	/**
	 * Spawns the disposable floor, store, light and start only when the map's
	 * WorldSettings carries the `LLK.Harness` actor tag.  The opt-in is checked
	 * before World Partition actors stream, so authored maps can never be
	 * mistaken for empty maps by a content query during InitGame.
	 */
	void BuildHarnessWorld();

	/** True when the open map already provides one, so the harness adds nothing. */
	bool WorldAlreadyHas(TSubclassOf<AActor> ActorClass) const;
};

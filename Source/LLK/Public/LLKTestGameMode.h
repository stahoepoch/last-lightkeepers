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
	void HandleThresholdCrossed(FName ThresholdId, float Charge);

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

	/** Spawns the floor, the store, a light and a start, skipping whatever the map already has. */
	void BuildHarnessWorld();

	/** True when the open map already provides one, so the harness adds nothing. */
	bool WorldAlreadyHas(TSubclassOf<AActor> ActorClass) const;
};

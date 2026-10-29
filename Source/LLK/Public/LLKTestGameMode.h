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
#include "GameFramework/GameModeBase.h"
#include "LLKTestGameMode.generated.h"

UCLASS()
class LLK_API ALLKTestGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALLKTestGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

protected:
	/** Half-extent of the floor in cm. 50m square: big enough to sprint across and notice. */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Harness")
	float FloorHalfExtent = 2500.f;

	/**
	 * Upright blocks placed around the floor.
	 *
	 * Not decoration. On a flat untextured plane a first-person camera gives
	 * almost no motion cue, so "did the keeper move?" becomes unanswerable by
	 * eye - which would defeat the one thing M0.8 is for. Vertical edges at
	 * known spacing make walk, sprint and crouch visibly different speeds.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Harness")
	int32 ReferencePillars = 12;

	/**
	 * How bright the harness's own sky is. Low, deliberately.
	 *
	 * A box lit well enough to walk around in is a box where the lantern
	 * changes nothing, and the lantern is what M0.8 is now for. Raise these
	 * two to see the geometry; lower them to judge the light. Neither touches
	 * a project lighting value - see the header note - they are arguments to
	 * two actors spawned into a running world.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Harness", meta = (ClampMin = "0"))
	float HarnessSunIntensity = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Harness", meta = (ClampMin = "0"))
	float HarnessSkyIntensity = 0.1f;

	/** Spawns the floor, the pillars, a light and a start, skipping whatever the map already has. */
	void BuildHarnessWorld();

	/** True when the open map already provides one, so the harness adds nothing. */
	bool WorldAlreadyHas(TSubclassOf<AActor> ActorClass) const;
};

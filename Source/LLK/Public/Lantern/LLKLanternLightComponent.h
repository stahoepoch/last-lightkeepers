// The lantern you can actually see. M1.3's placeholder.
//
// **M1.3 is Chris's row and this is not it.** The real lantern - its mesh, its
// materials, the sound of it striking, whatever it looks like when it swings -
// is `Content/` and his alone (row 41). What M1.3's proof line asks for is
// that "the C++ class is complete and works with a placeholder before this
// exists", and this is that placeholder: a spot light built in code, with no
// asset of any kind behind it.
//
// **It is a listener, not an owner.** `ULLKLanternComponent` decides whether
// the lantern is on and knows nothing about this class, deliberately - its
// header says a component that knows about its listeners cannot be tested
// without them. So the arrow points this way only: this component subscribes
// to `OnLanternStateChanged` and does as it is told. Deleting this file would
// not change a single rule about light in the game.
//
// **It replaces itself.** When Chris's real lantern lands, the thing to keep
// is the subscription shape; the spot light goes. That is why the radius and
// intensity are declared properties rather than constants buried in the cpp.
//
// **On the radius, which is a real data gap and not a tuning choice.** There
// is no light radius anywhere in `data/`. But `data/enemies.json` authors
// `aggroRadiusDark: 12` and `aggroRadiusLit: 38`, so the game already has a
// number for how far away a lit keeper is noticed - and no number at all for
// how far that keeper can see. Those two decide between them whether light
// lets you spot danger first or only tells danger where you are, which is
// the whole of row 5's trade. The placeholder below sits INSIDE 38 studs on
// purpose, so being lit means being seen from beyond your own light, but that
// is a design call sitting in C++ where it does not belong. It wants
// authoring in `data/lantern.json` next to the movement speeds.

#pragma once

#include "CoreMinimal.h"
#include "Components/SpotLightComponent.h"
#include "Lantern/LLKLanternComponent.h"
#include "LLKLanternLightComponent.generated.h"

UCLASS(ClassGroup = (LLK), meta = (BlueprintSpawnableComponent))
class LLK_API ULLKLanternLightComponent : public USpotLightComponent
{
	GENERATED_BODY()

public:
	ULLKLanternLightComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Subscribes to a lantern and matches its state immediately.
	 *
	 * Called by the owning actor rather than discovered, so that an actor with
	 * two lanterns and two lights - a dropped lantern on the floor, later -
	 * does not depend on component search order.
	 */
	void WatchLantern(ULLKLanternComponent* InLantern);

	/** Placeholder reach, in STUDS. See the header note: this belongs in `data/`. */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Lantern", meta = (ClampMin = "0"))
	float PlaceholderRadiusStuds = 30.f;

protected:
	UFUNCTION()
	void OnLanternStateChanged(bool bIsOn, ELLKLanternChangeReason Reason);

	/** Visibility only. Never touches the lantern's state - the arrow points one way. */
	void ApplyVisibleState(bool bIsOn);

	UPROPERTY()
	TWeakObjectPtr<ULLKLanternComponent> WatchedLantern;
};

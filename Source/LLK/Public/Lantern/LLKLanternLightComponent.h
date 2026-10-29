// The lantern you can actually see. M1.3's placeholder.
//
// **M1.3 is Chris's row and this is not it.** The real lantern - its mesh, its
// materials, the sound of it striking, whatever it looks like when it swings -
// is `Content/` and his alone (row 41). What M1.3's proof line asks for is
// that "the C++ class is complete and works with a placeholder before this
// exists", and this is that placeholder: a spot light with no asset of any
// kind behind it.
//
// **It is a listener, not an owner.** `ULLKLanternComponent` decides whether
// the lantern is on and knows nothing about this class, deliberately - its
// header says a component that knows about its listeners cannot be tested
// without them. So the arrow points this way only: this component subscribes
// to `OnLanternStateChanged` and does as it is told. Deleting this file would
// not change a single rule about light in the game.
//
// **Its numbers are data, not constants.** Chris, 2026-10-29: "the actual
// reach must be portable, tunable, and shared by light visibility/enemy logic
// later." Radius, cone angle, intensity, colour and the shadow toggle come
// from `data/lantern.json` through `FLLKLanternLightRules`. They used to be
// constants in the cpp, which put the most consequential number in the game -
// how far a keeper can see - somewhere `tests/` could not read it, and
// somewhere the enemy logic that must eventually agree with it could not
// either.
//
// **The radius is the one to read against `enemies.json`.** `aggroRadiusDark`
// is 12 and `aggroRadiusLit` is 38; the radius is 30. A lit keeper is noticed
// from eight studs beyond the edge of their own light, which is row 5's trade
// made physical: light reveals value and attracts danger, and the danger
// arrives out of the dark you just announced yourself to. Those two numbers
// now live in the same layer and can be asserted against each other.
//
// **What stays out of the data, deliberately.** The inner cone angle softens
// the falloff and changes nothing about reach. Mesh placement and cosmetic
// flicker are Chris's. Neither is portable and neither is worth a key.

#pragma once

#include "CoreMinimal.h"
#include "Components/SpotLightComponent.h"
#include "Data/LLKDataTypes.h"
#include "Data/LLKLanternData.h"
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

	/** Where the light's numbers came from. Read this rather than trusting a log line. */
	UFUNCTION(BlueprintPure, Category = "LLK|Lantern")
	ELLKDataSource GetLightSource() const { return LightSource; }

	/** The reach in STUDS, after resolution. What enemy and visibility logic must agree with. */
	UFUNCTION(BlueprintPure, Category = "LLK|Lantern")
	float GetRadiusStuds() const { return ResolvedRules.RadiusStuds; }

	/** The rules in use, so a test asserts against the same values the light applies. */
	const FLLKLanternLightRules& GetResolvedRules() const { return ResolvedRules; }

	/**
	 * Points this light at a data asset and re-resolves immediately.
	 *
	 * The component keeps its own `LanternData` so it can stand alone on an
	 * actor that is not a keeper - a dropped lantern on the floor, later. But
	 * an owner that has its own copy must push it down rather than let the two
	 * be set independently: one asset set here and a different one set on the
	 * character is the duplication this project keeps catching in its own
	 * paper trail, and it would show up as a light that disagrees with the
	 * speeds about which data it read.
	 */
	void SetLanternData(const TSoftObjectPtr<ULLKLanternData>& InData);

	/**
	 * Picks the asset's light rules or the declared fallbacks, then applies them.
	 *
	 * Public and callable at any time so that a live retune - Chris changing
	 * the radius and re-running the importer - can be pushed without a
	 * restart. Idempotent.
	 */
	void ResolveAndApplyLight();

	/**
	 * The inner cone as a fraction of the authored outer one.
	 *
	 * In code and not in `data/` on purpose: it softens the falloff and
	 * changes nothing about reach, so it is the cosmetic half of a key whose
	 * other half is gameplay. Keeping it here is what lets the authored cone
	 * angle mean exactly one thing.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Lantern", meta = (ClampMin = "0", ClampMax = "1"))
	float InnerConeFraction = 0.47f;

protected:
	UFUNCTION()
	void OnLanternStateChanged(bool bIsOn, ELLKLanternChangeReason Reason);

	/** Visibility only. Never touches the lantern's state - the arrow points one way. */
	void ApplyVisibleState(bool bIsOn);

	/**
	 * Defaults to the importer's output path. A soft pointer that resolves to
	 * nothing simply fails, so a clean clone with no `Content/Data/` still
	 * runs - it just announces the fallback. That is strictly better than
	 * leaving this unset, which was the original shape: the assets existed on
	 * disk and nothing pointed at them, so the game ran on declared fallbacks
	 * that happened to match while reporting that it had read no data.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data")
	TSoftObjectPtr<ULLKLanternData> LanternData =
		TSoftObjectPtr<ULLKLanternData>(FSoftObjectPath(TEXT("/Game/Data/DA_Lantern.DA_Lantern")));

	/**
	 * The declared fallbacks, matching `data/lantern.json` at the time of
	 * writing. A duplicate, and a real cost, so it is declared rather than
	 * hidden: `GetLightSource()` reports when it is in use and the log says so.
	 * A copy nobody can detect is the problem; a copy that announces itself is
	 * a fallback.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data")
	FLLKLanternLightRules FallbackRules;

	/** What was actually applied, from whichever source won. */
	UPROPERTY(VisibleInstanceOnly, Category = "LLK|Lantern")
	FLLKLanternLightRules ResolvedRules;

	UPROPERTY(VisibleInstanceOnly, Category = "LLK|Lantern")
	ELLKDataSource LightSource = ELLKDataSource::Unresolved;

	UPROPERTY()
	TWeakObjectPtr<ULLKLanternComponent> WatchedLantern;
};

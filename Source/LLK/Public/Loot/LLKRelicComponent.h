// A relic you can see and cannot read. Row 53.
//
// **The rule it implements, and the way it must not be implemented.**
// `data/loot.json` authors `rules.relicsRequireLanternLight: true` and gives
// the Relics tier `visibility: lanternOnly`. `ELLKLootVisibility`'s own
// comment states the trap in capitals: light gates *legibility*, never
// existence. A relic renders in darkness and cannot be read. **An invisible
// relic could not tempt anyone**, so this is never "do not spawn the actor" -
// and that is the single most likely way to get this wrong, because not
// spawning is easier and looks identical in a screenshot of a dark room.
//
// **Why it carries the ground floor.** `docs/THE-STORE.md` makes lesson two -
// *light reveals value and attracts danger* - the lesson the whole building is
// arranged around, and row 43 makes the building the tutorial. The sequence it
// needs is: the player sees something, cannot tell what it is, lights up out of
// their own curiosity, learns what it is, is noticed, and gets away. Nobody is
// told to light the lantern. **They want to know.** That only works if the
// relic is visible and unreadable, so this component is the mechanism lesson
// two is made of, and without it M1.5's greybox would land unable to prove the
// one thing it exists to prove.
//
// **Server-authoritative, like the lantern.** Identification is a fact about
// the world, not a client's opinion of its own screen. A client that could
// identify locally could read every relic in the building with the lantern off
// and the trade would evaporate.
//
// **The threshold is derived from authored data, not chosen.** "Enough light
// to read by" is one lantern's worth at the edge of its own reach: a single
// source at distance `radius` delivers `Intensity / 2` by
// `LLKLightField::BrightnessAt`, so that is the bar. No new data key, nothing
// invented, and it moves correctly on its own if Chris retunes the lantern.
//
// **Two channels, and the second one is a hook rather than a sound.** Row 16's
// pairing needs a channel that is not the visual, and the sound of a relic
// becoming legible is `Content/` and Chris's (row 41). So the state change
// broadcasts, with the reason, and whatever he hangs on it is his. The visual
// half is complete here.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/LLKDataTypes.h"
#include "Data/LLKLanternData.h"
#include "Data/LLKLootRulesData.h"
#include "LLKRelicComponent.generated.h"

/** Why a relic's legibility changed. For listeners, and for tests. */
UENUM(BlueprintType)
enum class ELLKRelicChangeReason : uint8
{
	/** Lantern light reached it and it became readable. */
	LitAndRead,
	/** Replicated in from the server; this client did not decide it. */
	Replicated,
	/** Identified by something other than light - a tool, later. Armed, never fired in v1. */
	IdentifiedByOther
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLLKRelicLegibilityChanged, bool, bIsLegible, ELLKRelicChangeReason, Reason);

UCLASS(ClassGroup = (LLK), meta = (BlueprintSpawnableComponent))
class LLK_API ULLKRelicComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULLKRelicComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * True once this relic's identity has been read.
	 *
	 * **Sticky on purpose.** Identity is knowledge: the player paid amber to
	 * learn what this is, and charging them again for the same relic would
	 * make the lesson a tax instead of a trade. Taking it is the part that
	 * still costs - see `CanBeTaken`.
	 */
	UFUNCTION(BlueprintPure, Category = "LLK|Loot")
	bool IsLegible() const { return bLegible; }

	/**
	 * Whether the relic can be picked up right now.
	 *
	 * **A PROPOSAL and the one design call in this file - Chris's to settle.**
	 * Identity is sticky but taking is gated on live light, so lifting a relic
	 * is a deliberate, exposed act rather than something a crew does in the
	 * dark after one scouting sweep. Without this, a crew lights up once, reads
	 * the whole floor, goes dark and loots at no cost - which is the trade of
	 * row 5 defeated by a single sweep. With it, light is what the taking
	 * costs, every time.
	 *
	 * Set `bRequiresLightToTake` false to make identification the only gate.
	 */
	UFUNCTION(BlueprintPure, Category = "LLK|Loot")
	bool CanBeTaken() const;

	/**
	 * What a player sees when they look at it.
	 *
	 * Unreadable in the dark, which is the point: it returns the shape rather
	 * than the name, so a HUD has something honest to draw instead of hiding
	 * the relic entirely.
	 */
	UFUNCTION(BlueprintPure, Category = "LLK|Loot")
	FText GetDisplayName() const;

	/** The relic's real name. Only meaningful once legible; a HUD should ask IsLegible first. */
	UFUNCTION(BlueprintCallable, Category = "LLK|Loot")
	void SetRelicIdentity(FName InId, const FText& InName, int32 InValue);

	/** -1 until legible, so a HUD cannot accidentally show a value the player has not earned. */
	UFUNCTION(BlueprintPure, Category = "LLK|Loot")
	int32 GetReadableValue() const { return bLegible ? Value : -1; }

	/** Brightness needed to read this, derived from the lantern's own authored intensity. */
	UFUNCTION(BlueprintPure, Category = "LLK|Loot")
	float GetIdentifyThreshold() const { return IdentifyThreshold; }

	/** Which path the rules came from. Mirrors every other system's source enum. */
	UFUNCTION(BlueprintPure, Category = "LLK|Loot")
	ELLKDataSource GetRulesSource() const { return RulesSource; }

	/** Fires on server and clients alike, whenever legibility actually changes. */
	UPROPERTY(BlueprintAssignable, Category = "LLK|Loot")
	FLLKRelicLegibilityChanged OnLegibilityChanged;

	/**
	 * How this relic is gated. `LanternOnly` for relics, from `loot.json`.
	 *
	 * `Always` is a legitimate setting and means baseline loot: visible and
	 * readable with the lantern off, which is lesson ONE of the ground floor
	 * and the thing lesson two needs to contrast against.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Loot")
	ELLKLootVisibility Visibility = ELLKLootVisibility::LanternOnly;

#if WITH_DEV_AUTOMATION_TESTS
	/** The authority-side decision, reachable by tests only. */
	void EvaluateForTest(float BrightnessAtRelic) { EvaluateLegibility(BrightnessAtRelic); }

	/** Injects resolved rules so a test does not depend on `Content/Data` existing. */
	void SetThresholdForTest(float InThreshold) { IdentifyThreshold = InThreshold; }
#endif

protected:
	/** Decides legibility from the light reaching this relic. Server side. */
	void EvaluateLegibility(float BrightnessAtRelic);

	void ResolveRules();

	UFUNCTION()
	void OnRep_Legible();

	/**
	 * Seconds between light checks. Not every frame: a relic becoming readable
	 * a tenth of a second late is imperceptible, and a floor full of relics
	 * each summing a light field every frame is not.
	 */
	UPROPERTY(EditAnywhere, Category = "LLK|Loot", meta = (ClampMin = "0.01"))
	float CheckIntervalSeconds = 0.1f;

	/** See `CanBeTaken`. A proposal, declared rather than hidden. */
	UPROPERTY(EditAnywhere, Category = "LLK|Loot")
	bool bRequiresLightToTake = true;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data")
	TSoftObjectPtr<ULLKLootRulesData> LootRules =
		TSoftObjectPtr<ULLKLootRulesData>(FSoftObjectPath(TEXT("/Game/Data/DA_LootRules.DA_LootRules")));

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data")
	TSoftObjectPtr<ULLKLanternData> LanternData =
		TSoftObjectPtr<ULLKLanternData>(FSoftObjectPath(TEXT("/Game/Data/DA_Lantern.DA_Lantern")));

	/** Declared fallback for the lantern's intensity, matching `data/lantern.json`. */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data", meta = (ClampMin = "0"))
	float FallbackLanternIntensity = 250.f;

	UPROPERTY(ReplicatedUsing = OnRep_Legible)
	bool bLegible = false;

	/** Live, server-side, and NOT replicated: clients read light locally for taking. */
	bool bCurrentlyLit = false;

	UPROPERTY(VisibleInstanceOnly, Category = "LLK|Loot")
	float IdentifyThreshold = 0.f;

	UPROPERTY(VisibleInstanceOnly, Category = "LLK|Loot")
	ELLKDataSource RulesSource = ELLKDataSource::Unresolved;

	/** True when `loot.json` says relics need light at all. If false, relics are legible outright. */
	UPROPERTY(VisibleInstanceOnly, Category = "LLK|Loot")
	bool bRelicsRequireLight = true;

	UPROPERTY()
	FName RelicId;

	UPROPERTY()
	FText RelicName;

	UPROPERTY()
	int32 Value = 0;

	float SecondsSinceCheck = 0.f;
};

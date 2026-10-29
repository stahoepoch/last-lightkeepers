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
// **Two channels, neither of which is audio.** Chris, 2026-10-29, correcting
// his own first phrasing - what he wants is not a second visual effect:
//
//   - Dark: the relic's physical silhouette is there. Something occupies that
//     shelf.
//   - Lit: its details become readable AND its interaction affordance becomes
//     live.
//   - "The player should be able to confirm that change through the world
//     object and the interaction state, without needing an audio cue."
//
// So the two channels are the **object** and the **affordance**, and the
// affordance half is code. That is why interactability is not merely a query
// here: it is tracked state with its own broadcast, because a channel the
// player is meant to notice changing has to actually change at a moment, and
// a bool polled silently by a HUD is not an event anybody can perceive.
//
// `ELLKRelicInteractBlock` is the part that teaches. A prompt that can say
// *needs light* rather than going blank is the rule being stated by the
// interface in the player's own situation - which is what row 43 asks for and
// what a tutorial-free game has instead of a tooltip.
//
// The object half is `Content/` and Chris's (row 41): the paper label, the
// engraved tag, the photograph detail, whatever the relic turns out to be. It
// hangs on `OnLegibilityChanged`, and nothing here guesses at it.

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

/** Why a relic cannot be taken right now. The prompt's own vocabulary. */
UENUM(BlueprintType)
enum class ELLKRelicInteractBlock : uint8
{
	/** Nothing in the way: it can be taken. */
	None,
	/** Never read, so there is nothing to decide about. Light it first. */
	Unidentified,
	/**
	 * Read, but dark right now.
	 *
	 * **The one a prompt should say out loud.** "Needs light" in the moment a
	 * player reaches for a relic they already identified is row 5 explaining
	 * itself without a tooltip, which is the whole of row 43.
	 */
	NeedsLight
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLLKRelicLegibilityChanged, bool, bIsLegible, ELLKRelicChangeReason, Reason);

/** The affordance channel: fires whenever the relic becomes takeable or stops being. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLLKRelicInteractableChanged, bool, bIsInteractable, ELLKRelicInteractBlock, Block);

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

	/**
	 * Server only. Removes the relic from the world and reports whether it went.
	 *
	 * Returns false when it was already taken, which is the double-take race:
	 * two keepers pressing E on the same brooch in the same tick. The first
	 * call wins and the second is told no, rather than both being told yes
	 * and one relic becoming two.
	 */
	bool TakeByServer();

	UFUNCTION(BlueprintPure, Category = "LLK|Loot")
	bool IsTaken() const { return bTaken; }

	/**
	 * **The object acknowledging the light, which is Chris's "making it WHY".**
	 *
	 * Row 16 needs the change readable on the object and not only in a
	 * readout, and an outline needs a post-process material, which is
	 * `Content/` and his. What code can do is set the custom-depth stencil
	 * the material will read: 1 for a relic that is visible but unreadable,
	 * 2 once it is legible, 3 when it is takeable right now. So the moment he
	 * authors one outline material, three states already drive it and nothing
	 * here has to change.
	 */
	UFUNCTION(BlueprintPure, Category = "LLK|Loot")
	int32 GetOutlineStencil() const;

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
	 * The second channel. Fires when the relic becomes takeable or stops being.
	 *
	 * Local to each machine on purpose: the affordance depends on the light
	 * reaching this relic *here and now*, and a prompt that waited for a round
	 * trip would appear a moment after the player had already decided.
	 * Legibility is the server's; whether you can lift it this instant is a
	 * question every machine can answer identically from replicated state.
	 */
	UPROPERTY(BlueprintAssignable, Category = "LLK|Loot")
	FLLKRelicInteractableChanged OnInteractableChanged;

	/** What is in the way, for a prompt to say. `NeedsLight` is the one that teaches. */
	UFUNCTION(BlueprintPure, Category = "LLK|Loot")
	ELLKRelicInteractBlock GetInteractBlock() const;

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

	/**
	 * Drives one light sample through the whole per-tick path, with no world.
	 *
	 * Sets the live-lit flag and refreshes the affordance exactly as
	 * TickComponent does, which is what lets the affordance channel be tested
	 * against the real code rather than a copy of it.
	 */
	void SampleLightForTest(float BrightnessAtRelic)
	{
		bCurrentlyLit = BrightnessAtRelic >= IdentifyThreshold;
		EvaluateLegibility(BrightnessAtRelic);
		RefreshInteractable();
	}

	/** Injects resolved rules so a test does not depend on `Content/Data` existing. */
	void SetThresholdForTest(float InThreshold) { IdentifyThreshold = InThreshold; }
#endif

protected:
	/** Decides legibility from the light reaching this relic. Server side. */
	void EvaluateLegibility(float BrightnessAtRelic);

	void ResolveRules();

	/** Recomputes the affordance and broadcasts only when it actually changed. */
	void RefreshInteractable();

	UFUNCTION()
	void OnRep_Legible();

	UFUNCTION()
	void OnRep_Taken();

	/** Pushes the stencil state onto the owner's primitives, so an outline material can read it. */
	void ApplyOutlineState();

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

	/** Replicated so a client stops drawing a prompt for something already gone. */
	UPROPERTY(ReplicatedUsing = OnRep_Taken)
	bool bTaken = false;

	/** Live and NOT replicated: every machine reads the light field locally. */
	bool bCurrentlyLit = false;

	/**
	 * Last broadcast affordance, so the delegate fires on change rather than
	 * on every sample. A prompt driven by an event that fires ten times a
	 * second is a prompt that flickers.
	 */
	bool bInteractableNow = false;

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

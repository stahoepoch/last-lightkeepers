// The instrument. It listens, and it never touches anything. M4's gate, early.
//
// **Read-only is the whole design constraint.** This class subscribes to
// delegates other systems already broadcast, appends to a list, and writes one
// file when the run ends. It calls no setter, spawns nothing, and nothing in
// the game reads it back - because a metric a rule consults is a difficulty
// system nobody designed, which is exactly the secret rubber-banding Chris's
// fifth replayability pillar forbids. `tests/telemetry.test.mjs` fails if any
// gameplay module includes `LLKRunMetrics.h`, and fails if this file ever
// calls into a system rather than listening to one.
//
// **What it listens to, all of it already there.** The extraction subsystem's
// five events, the zone clock's thresholds and the Warden, the run state's
// wipe, every keeper's downed state and carried count, and every lantern's
// on/off. The only thing it polls is the extraction snapshot's `bHoldStalled`,
// because a stall is a condition rather than an event and nothing broadcasts
// it - and polling it is how row 75's pause becomes a number instead of an
// impression.
//
// **Server only.** A client's copy would see its own partial view and write a
// second, disagreeing file about the same run.
//
// **Where it writes, and why not `Saved/Automation`.** One JSON per run under
// `Saved/LLKTelemetry/`, named by wall-clock time. `Saved/` is a build
// artefact nobody's lane owns and nothing is committed from it, so an
// instrument that fills a folder cannot dirty a working tree - and a playtest
// produces one file per run that Chris can send me without exporting
// anything.

#pragma once

#include "CoreMinimal.h"
#include "Run/LLKRunMetrics.h"
#include "Subsystems/WorldSubsystem.h"
#include "LLKRunTelemetrySubsystem.generated.h"

class ULLKInteractorComponent;
class ULLKLanternComponent;
class ULLKRelicComponent;
class ULLKVitalsComponent;
enum class ELLKDownedReason : uint8;
enum class ELLKLanternChangeReason : uint8;
enum class ELLKRelicInteractBlock : uint8;

UCLASS()
class LLK_API ULLKRunTelemetrySubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** The run so far, summarised. Safe to call mid-run. */
	UFUNCTION(BlueprintPure, Category = "LLK|Metrics")
	FLLKRunSummary Summarise() const { return LLKRunMetrics::Summarise(Events); }

	UFUNCTION(BlueprintPure, Category = "LLK|Metrics")
	int32 CountEvents() const { return Events.Num(); }

	/**
	 * Writes the run to `Saved/LLKTelemetry/` and returns the path.
	 *
	 * Called automatically when a run resolves, and by `LLK.Telemetry.Write`
	 * for a run that is still going - a playtest that ends with the editor
	 * being closed should not lose its numbers.
	 */
	FString WriteToDisk();

#if WITH_DEV_AUTOMATION_TESTS
	const TArray<FLLKRunEvent>& GetEventsForTest() const { return Events; }
	void RecordForTest(ELLKRunEvent Kind, float AtSeconds, FName Detail = NAME_None, float Value = 0.f)
	{
		FLLKRunEvent Event;
		Event.Kind = Kind;
		Event.AtSeconds = AtSeconds;
		Event.Detail = Detail;
		Event.Value = Value;
		Events.Add(Event);
	}
#endif

protected:
	/** The one writer. Stamps the time itself so no caller can lie about when. */
	void Record(ELLKRunEvent Kind, FName Detail = NAME_None, float Value = 0.f);

	/** Binds to whatever exists now. Keepers arrive late, so this runs again. */
	void BindToWhatExists();

	bool HasTelemetryAuthority() const;

	// ---- the handlers, one per delegate -----------------------------------
	//
	// `UFUNCTION` because the delegates are dynamic, which is also why they
	// cannot be lambdas.

	UFUNCTION()
	void OnThresholdCrossed(FName ThresholdId, float Pressure);

	UFUNCTION()
	void OnWardenConverged();

	UFUNCTION()
	void OnCrewWiped();

	UFUNCTION()
	void OnExtractionCalled(FName PointId, float SirenSeconds);

	UFUNCTION()
	void OnRigArrived(FName PointId);

	UFUNCTION()
	void OnExtractionCancelled(FName PointId);

	UFUNCTION()
	void OnExtractionSecured(FName PointId, float PayoutModifier, int32 ItemsSecured);

	UFUNCTION()
	void OnDownedChanged(bool bIsDowned, ELLKDownedReason Reason);

	UFUNCTION()
	void OnLanternStateChanged(bool bIsOn, ELLKLanternChangeReason Reason);

	UFUNCTION()
	void OnCarriedCountChanged(int32 CarriedCount);

	/** Server: a keeper reached for a relic, and the reason they were or were not allowed. */
	UFUNCTION()
	void OnInteractAttempted(ULLKRelicComponent* Relic, ELLKRelicInteractBlock Block);

	/**
	 * Walks the relics being watched and records the ones left behind.
	 *
	 * **Polled rather than bound, for the same reason the stall is: there is
	 * nobody to ask.** `OnInteractableChanged` fires on the relic and carries
	 * no sender, so a listener cannot tell which relic went dark - and the
	 * abandonment signal is useless without the relic's id. A bool getter per
	 * watched relic a few times a second is cheaper than making the gameplay
	 * delegate carry a payload it has no other use for.
	 */
	void PollAbandonedRelics(float DeltaSeconds);

	/** The block reason as the event list spells it. One place, so counts stay comparable. */
	static FName NameOfBlock(ELLKRelicInteractBlock Block);

	TArray<FLLKRunEvent> Events;

	/** Seconds since the first event. The run is its own timeline, not the level's. */
	float RunSeconds = 0.f;

	/** True once an ending has been recorded, so a wipe cannot follow a secure. */
	bool bResolved = false;

	/** Written once per run, however many ways the run can end. */
	bool bWritten = false;

	/** The stall is polled, so the last value is needed to spot the edges. */
	bool bWasStalled = false;

	/** Carried counts, per keeper, so a drop is not recorded as a take. */
	TMap<TWeakObjectPtr<ULLKInteractorComponent>, int32> LastCarried;

	/**
	 * Relics being watched, and when each became takeable - or a negative
	 * number for one that is not takeable now.
	 *
	 * Tracked by the relic rather than by the keeper looking at it, and that
	 * is a limitation worth naming: focus is a client-local question, so a
	 * remote client's aim is not visible here. What this measures is a relic
	 * the LIGHT FIELD made available and nobody took, which is the signal on a
	 * solo run and an over-count on a crowded one.
	 */
	TMap<TWeakObjectPtr<ULLKRelicComponent>, float> TakeableSince;

	/** Seconds of accumulated poll debt, so the sweep runs on an interval. */
	float SecondsSinceAbandonPoll = 0.f;

	TArray<TWeakObjectPtr<ULLKVitalsComponent>> BoundKeepers;
	TArray<TWeakObjectPtr<ULLKLanternComponent>> BoundLanterns;
	TArray<TWeakObjectPtr<ULLKInteractorComponent>> BoundInteractors;

	bool bBoundToSubsystems = false;

	/** Rebinding looks for keepers who joined; it does not need to be fast. */
	float SecondsSinceBind = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Metrics", meta = (ClampMin = "0.25"))
	float BindIntervalSeconds = 1.f;

	/** How often the abandonment sweep runs. Dwell is measured in seconds, so this is plenty. */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Metrics", meta = (ClampMin = "0.05"))
	float AbandonPollSeconds = 0.25f;

	/**
	 * How long a relic must stay takeable before leaving it counts as leaving it.
	 *
	 * **The floor that keeps this from counting the wrong thing.** A lantern
	 * swung past a shelf makes every relic on it takeable for a moment, and
	 * none of those is an offer anybody refused. Three seconds is long enough
	 * that the player had time to see the outline and the prompt and decide,
	 * and short enough that a genuine pause still registers.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Metrics", meta = (ClampMin = "0.1"))
	float AbandonDwellSeconds = 3.f;
};

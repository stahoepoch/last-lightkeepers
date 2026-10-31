// What a run actually was, as numbers. M4's instrument, built early.
//
// **Why this exists and why now.** `docs/LANES.md` makes playtest
// instrumentation mine and names the risk it covers: *"Row 3's question
// answered with opinions instead of data. The Roblox A1 slice was written for
// this and never built."* M4 is the gate - *"do testers kill their lanterns,
// bank, and argue for one more room? Four real testers, instrumented"* - and
// the roadmap is blunt about why asking them is not enough: Grizzard et al.
// found that arousal habituates **before players notice it has**, so a crew
// reports a run as tense well after it has stopped being so. *"Measure what
// they did."*
//
// Chris is about to run the dock bay twice, alone, and the thing he wants to
// know is whether twenty seconds at the dock is a decision. That is an
// observation about behaviour, so it should be observed.
//
// **What this layer is: arithmetic over a list of events.** No world, no actor,
// no file. `ULLKRunTelemetrySubsystem` collects the events by listening to
// delegates that already exist, and everything below turns them into a summary.
// The split is the usual one, and here it earns itself twice over - a metric
// that cannot be computed from a list of events in a test is a metric nobody
// can check the definition of.
//
// **The three questions it is built to answer**, in the roadmap's own words:
//
//   1. *Do they kill their lanterns?* `LitFraction` - how much of the run any
//      light was burning. Not whether they ever used one.
//   2. *Do they bank?* `RelicsTaken` against `RelicsSecured`, and whether the
//      run ended `Secured` at all.
//   3. *Do they argue for one more room?* The closest honest proxy is
//      hesitation and reversal: `SecondsFromFirstStallToCall` is not it, but
//      `ExtractionCalledSeconds` against `RunSeconds`, `CancelCount`, and
//      `HoldStallSeconds` together are. A crew that called at 4:00, cancelled,
//      and called again at 9:30 had that argument.
//
// **The fourth question, added 2026-10-31, which had no instrument at all.**
// Row 43 locks the department store as the tutorial and judges it on whether
// it teaches, and nothing here measured that: `LitFraction` says whether a
// lantern burned and `RelicsTaken` says whether loot moved, and a player who
// never understood that taking costs light produces the same zeroes as one who
// understood it and left early. So:
//
//   4. *Did they understand what they could do?* `InteractAttempts` with
//      `BlockedNeedsLight` and `BlockedUnidentified` - the reasons the game
//      refused them, which it computed anyway and used to throw away -
//      `SecondsToFirstRelicTaken`, and `AbandonedWhileTakeable`.
//
// **These are behavioural proxies and the file must not claim more.** They say
// what a player did, not what they understood; the person who was there
// answers whether it felt confusing. A run with no blocked attempts and a fast
// first take did not have a legibility problem, and a run with eleven
// `NeedsLight` refusals did - between those two the numbers narrow the
// question rather than settling it.
//
// **What it deliberately does NOT measure:** anything about how a run felt,
// and anything about who was playing. No names, no ids, no machine details.
// The file it produces is a description of a run, not of a person.
//
// **And it must never become an input.** A metric that a rule reads is a
// difficulty system nobody designed - the exact shape of the rubber-banding
// Chris's fifth replayability pillar forbids. `tests/telemetry.test.mjs`
// fails if any gameplay module includes this header.

#pragma once

#include "CoreMinimal.h"
#include "LLKRunMetrics.generated.h"

/** What happened. One enum rather than one struct per event, because the list is read as a whole. */
UENUM(BlueprintType)
enum class ELLKRunEvent : uint8
{
	RunStarted,

	/** Any keeper's lantern came on or went off. The lit clock runs between them. */
	LanternOn,
	LanternOff,

	/**
	 * A keeper changed lantern level. `Value` is the index, `Detail` the name.
	 *
	 * **The question this exists to answer is whether the dial gets used at
	 * all.** Chris authored three named settings on 2026-10-30, and the
	 * failure mode for a feature like that is not that it plays badly - it is
	 * that everybody leaves it on Working for the whole run and nobody ever
	 * finds out, because the behaviour and the no-behaviour look identical in
	 * a log. `LitFraction` cannot see it: a run spent entirely hooded and a
	 * run spent entirely open are both "lit the whole time".
	 *
	 * Emitted only on an actual change - `SetActiveLevelIndex` returns false
	 * for a no-op precisely so this stream stays readable.
	 */
	LightLevelChanged,

	/** A relic left a shelf. Row 53's whole point is that this costs light. */
	RelicTaken,

	/**
	 * A keeper asked to take a relic, and `Detail` is the answer they got.
	 *
	 * **The legibility instrument's highest-value event, because it records a
	 * decision the game already made.** `ULLKRelicComponent::GetInteractBlock`
	 * computes exactly why an interact cannot proceed - `Unidentified`,
	 * `NeedsLight`, or `None` for one that went through - and before this
	 * event that answer was computed, acted on, and discarded.
	 *
	 * Row 43 makes the department store the tutorial and judges it on whether
	 * it teaches. The distribution of these reasons over a playtest is the
	 * only thing in this file that says *which rule* a player failed to
	 * understand rather than merely that they struggled: a run full of
	 * `NeedsLight` is a player who found the relic and not the rule, and a run
	 * full of `Unidentified` is a player reaching for things they have never
	 * lit at all. Those are different failures and they want different fixes.
	 *
	 * Recorded on the server, in `ServerTake`, because the client's prompt is
	 * a courtesy and the server's answer is the one that happened.
	 */
	InteractAttempted,

	/**
	 * A relic was takeable for long enough to read and was left behind.
	 *
	 * `Value` is the dwell in seconds; `Detail` is the relic's id.
	 *
	 * **The purest available signal that an affordance was visible and not
	 * understood.** A relic that is lit, legible and in reach is a relic the
	 * game is offering; one that goes dark again un-taken after the player had
	 * seconds to act on it is an offer that was not read. Nothing else in this
	 * file can see that, because every other event is something a player did -
	 * this is the one that fires when they did nothing.
	 *
	 * Timed against the DEBOUNCED `bCurrentlyLit` (`ab9818f`), not a raw
	 * sample, so trace jitter at a shelf edge cannot manufacture an
	 * abandonment. The dwell floor does the rest: a relic lit in passing by a
	 * lantern swinging past it is not an offer anybody refused.
	 */
	RelicAbandoned,

	/** A zone-pressure threshold crossed, or the Warden arrived. */
	ThresholdCrossed,
	WardenConverged,

	/** The radio call, the rig's arrival, and a crew changing its mind. */
	ExtractionCalled,
	RigArrived,
	ExtractionCancelled,

	/** The hold stopped advancing, and started again. Row 75's pause. */
	HoldStalled,
	HoldResumed,

	KeeperDowned,
	KeeperUp,

	/** One of the two endings. Row 8: only the first keeps a haul. */
	ExtractionSecured,
	CrewWiped
};

/** One thing that happened, at a time, with at most one number attached. */
USTRUCT(BlueprintType)
struct FLLKRunEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	ELLKRunEvent Kind = ELLKRunEvent::RunStarted;

	/** Seconds since the run began, not wall-clock. A run is its own timeline. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	float AtSeconds = 0.f;

	/**
	 * Whatever the event needs: a threshold id, an exit's name, a count.
	 *
	 * One loose field rather than a struct per event, and the reason is that
	 * this list is written once and read as a whole - a hierarchy of event
	 * types would be more correct and would make the summary below harder to
	 * read, which is the wrong trade for an instrument.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	FName Detail;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	float Value = 0.f;
};

/** How a run ended, or that it did not. */
UENUM(BlueprintType)
enum class ELLKRunOutcome : uint8
{
	/** Still going, or the editor was closed. Not a failure - an absence. */
	Unresolved,

	/** Row 8's only path to keeping a haul. */
	Secured,

	Wiped
};

/** The run, in the numbers M4 is judged on. */
USTRUCT(BlueprintType)
struct FLLKRunSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	ELLKRunOutcome Outcome = ELLKRunOutcome::Unresolved;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	float RunSeconds = 0.f;

	/** Seconds with at least one lantern burning. The denominator is the run. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	float LitSeconds = 0.f;

	/** **Row 3's first question**, as a number between 0 and 1. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	float LitFraction = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	int32 RelicsTaken = 0;

	/** What left the building. Zero on a wipe, by row 8. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	int32 RelicsSecured = 0;

	// ---- legibility: row 43's question, which had no instrument -----------
	//
	// **These are behavioural proxies for understanding, not measurements of
	// it.** Nothing below can tell a player who did not understand the light
	// rule from one who understood it perfectly and chose to leave a brooch
	// behind, and no arithmetic will ever separate those two. What they do is
	// bound the question: a run with no blocked attempts and a fast first take
	// did not have a legibility problem, and a run with eleven `NeedsLight`
	// refusals did. The person who was there answers whether it felt
	// confusing; this answers what they did.

	/**
	 * Seconds from the run's start to the first relic that actually left a
	 * shelf, or -1 if none ever did.
	 *
	 * Blunt on purpose, and derived rather than recorded - it is the first
	 * `RelicTaken`, which this file has always had. The honest reading is a
	 * floor and not a score: it includes walking to the shelf, so a long time
	 * is ambiguous and a short one is conclusive.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	float SecondsToFirstRelicTaken = -1.f;

	/** Every interact that reached the server, refused or not. The denominator. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	int32 InteractAttempts = 0;

	/**
	 * Refused because the relic was read but dark. Row 5's rule, hit.
	 *
	 * The most diagnostic single number here. A player who reaches for a
	 * relic they have already identified has understood that relics are
	 * takeable and not that taking costs light.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	int32 BlockedNeedsLight = 0;

	/** Refused because it had never been read. Reaching before lighting. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	int32 BlockedUnidentified = 0;

	/** Takeable, taken for long enough to read, and left. See `RelicAbandoned`. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	int32 AbandonedWhileTakeable = 0;

	/**
	 * The share of interacts the game refused, between 0 and 1.
	 *
	 * Zero when nobody tried, which is not the same as zero when everybody
	 * succeeded - read it beside `InteractAttempts` and never alone.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	float RefusedFraction = 0.f;

	/** When the radio call went out, or -1 if it never did. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	float ExtractionCalledSeconds = -1.f;

	/** Called and thought better of it. The clearest sign of an argument. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	int32 CancelCount = 0;

	/** How long the hold spent not advancing, and how many times it stopped. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	float HoldStallSeconds = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	int32 HoldStallCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	int32 KnockdownCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	int32 ThresholdsCrossed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	bool bWardenArrived = false;

	/**
	 * The part of the run spent after the call, as a fraction.
	 *
	 * **The number that answers Chris's actual question about the dock.** A
	 * run that is 95% looting and 5% extracting has a hold nobody had to think
	 * about; one where the last fifth is the climax has the shape row 8 wants.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Metrics")
	float ClimaxFraction = 0.f;
};

namespace LLKRunMetrics
{
	/** The arithmetic. Pure, so the definition of every metric above is checkable. */
	LLK_API FLLKRunSummary Summarise(const TArray<FLLKRunEvent>& Events);

	/** One screen of plain language, for the console and the log. */
	LLK_API FString Describe(const FLLKRunSummary& Summary);

	/**
	 * The run as JSON, events and summary together.
	 *
	 * Hand-built rather than through a serialiser: the file is read by a human
	 * and by `tests/`, both of which want a stable shape, and a reflection
	 * dump would change whenever a field was reordered.
	 */
	LLK_API FString ToJson(const TArray<FLLKRunEvent>& Events, const FLLKRunSummary& Summary);

	LLK_API const TCHAR* NameOf(ELLKRunEvent Kind);
	LLK_API const TCHAR* NameOf(ELLKRunOutcome Outcome);
}

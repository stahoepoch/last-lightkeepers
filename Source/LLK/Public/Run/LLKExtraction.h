// Extraction, as rules nothing can cheat. M3.
//
// **What this file is for.** `docs/GAME-THESIS.md` insists extraction is
// *"called, announced, converged upon, and held"* rather than a door, and
// `DECISIONS.md` row 74 finally says who arrives: Lanternhold's recovery crew,
// in a lit rig, on the service-road side of the dock. None of that was a rule
// yet - the data authored two exits and a siren length and nothing read them.
// This is the state machine, and the geometry it eventually stands on is the
// greybox's job rather than this file's.
//
// **The two halves, as everywhere else in this module.** Everything here is a
// pure function of a state, a step and some authored numbers: no `UWorld`, no
// actor, no subsystem, no clock. `ULLKExtractionSubsystem` is the thin half
// that measures the world and concludes nothing. `LLKMapReadiness` and
// `LLKLanternFeel` are the same shape, and the reason is always the same - a
// rule that can only be exercised by standing in a level is a rule nobody
// tests.
//
// **Row 73 is structural here, not a promise.** The clock means weather alone:
// *"A radio call cannot accelerate the weather."* So this layer cannot see the
// clock. It takes no pressure value, returns no pressure change, and there is
// deliberately no field it could be handed through - `FLLKExtractionAnnounce`
// carries the siren and the convergence flag and NOT the
// `FLLKExtractionCallRules::StormChargeSpike` that still sits beside them in
// the data asset, waiting for Codex to delete it. Copying two of three fields
// is the point: a rule that cannot be expressed cannot regress.
//
// **What the call actually costs**, since it no longer costs the night: noise,
// light and a route. The siren runs for `extractionCall.sirenSeconds`, the rig
// commits itself to the dock, and then somebody has to keep the boarding route
// clear for the exit's whole `arrivalSeconds`. That is the entire price, and
// every part of it is something a crew can see coming.
//
// **One decision in here is a tuning call rather than a fact**, flagged so it
// is argued about on purpose: an empty boarding route PAUSES the hold, it does
// not reset it. Resetting would mean one Sweeper walking through the bay
// costing a crew the whole twenty seconds again, which is failure they could
// not have planned around; pausing costs them time, and time is already the
// pressure clock. If Chris wants the harsher version it is one branch in
// `Advance`, and the test that names it is the hold-pauses case.

#pragma once

#include "CoreMinimal.h"
#include "LLKExtraction.generated.h"

/**
 * Where a called extraction has got to.
 *
 * `Secured` and `Failed` are terminal and this is enforced rather than
 * documented: a run has one ending, and an extraction that could be re-entered
 * after it resolved would be two.
 */
UENUM(BlueprintType)
enum class ELLKExtractionPhase : uint8
{
	/** Nothing called. The crew is still deciding, which is most of the run. */
	Idle,

	/** Called. The siren is running and the rig is on its way; nothing can be held yet. */
	Announcing,

	/** The rig is at the dock. The hold is running and wants somebody on the route. */
	Holding,

	/** Held to the end. The haul is out. */
	Secured,

	/** The crew wiped. Row 8: the night keeps everything. */
	Failed
};

/**
 * Why a call was refused, by name.
 *
 * The same discipline as `ELLKMapCheck`: a refusal that only says "no" sends
 * somebody reading code to find out which of six reasons applied.
 */
UENUM(BlueprintType)
enum class ELLKExtractionRefusal : uint8
{
	None,

	/** The exit has no id. An unnamed exit cannot be announced or logged. */
	NoSuchPoint,

	/** `arrivalSeconds` is zero or not finite. "An exit that costs nothing is not a decision." */
	NoHoldAuthored,

	/** `requires` is not satisfied - the freight lift without `SubBasementBreakerRestored`. */
	RequirementNotMet,

	/** Already announcing or holding. One call at a time; cancel first. */
	AlreadyUnderway,

	/** Already out. */
	AlreadyExtracted,

	/** The crew wiped. Nobody is calling anybody. */
	RunIsOver
};

/**
 * `extractionCall` - what announcing costs, minus the field that no longer applies.
 *
 * Two of the three keys in that block, deliberately. See the header note on
 * row 73.
 */
USTRUCT(BlueprintType)
struct FLLKExtractionAnnounce
{
	GENERATED_BODY()

	/** `extractionCall.sirenSeconds` - 8. The rig's travel time, and the warning. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Extraction")
	float SirenSeconds = 0.f;

	/** `extractionCall.convergeAllPatrols` - true. Row 74 gives it its reason. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Extraction")
	bool bConvergeAllPatrols = false;
};

/**
 * One exit's terms, flattened out of `FLLKExtractionPoint`.
 *
 * Taken by value at the moment of the call and then kept in the state, so that
 * reloading a data asset mid-hold cannot change a deal a crew already accepted.
 */
USTRUCT(BlueprintType)
struct FLLKExtractionPointRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Extraction")
	FName Id;

	/** `arrivalSeconds` - 20 at the dock, 30 at the lift. The hold. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Extraction")
	float HoldSeconds = 0.f;

	/** `payoutModifier` - 1.0 and 1.6. Carried through so the resolution can pay it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Extraction")
	float PayoutModifier = 1.f;
};

/** The whole of what an extraction is, at one instant. */
USTRUCT(BlueprintType)
struct FLLKExtractionState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	ELLKExtractionPhase Phase = ELLKExtractionPhase::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	FName PointId;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	float SirenSecondsRemaining = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	float HoldSecondsRemaining = 0.f;

	/** What the hold was when it was accepted, so a readout can show progress. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	float HoldSecondsRequired = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	float PayoutModifier = 1.f;

	/**
	 * How many times this run has called, and how many it cancelled.
	 *
	 * Kept because each call is another siren in a building that listens, and
	 * because the cheapest version of "calling twice should cost something" is
	 * knowing it happened. No rule reads these yet; that price is Chris's to
	 * set if he wants one.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	int32 CallCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	int32 CancelCount = 0;

	bool IsUnderway() const
	{
		return Phase == ELLKExtractionPhase::Announcing || Phase == ELLKExtractionPhase::Holding;
	}

	bool IsTerminal() const
	{
		return Phase == ELLKExtractionPhase::Secured || Phase == ELLKExtractionPhase::Failed;
	}
};

/**
 * What the world looked like for one step.
 *
 * `KeepersOnBoardingRoute` is a `TOptional` for the reason it always is in this
 * module: unset means nobody measured, which is a different thing from a
 * measured zero. The greybox has no dock yet, so unset is the normal case
 * today - and an unmeasured route must not quietly behave like an empty one
 * that somehow still secures a haul.
 */
struct FLLKExtractionInput
{
	float DeltaSeconds = 0.f;

	TOptional<int32> KeepersOnBoardingRoute;

	/** Row 8 and `ULLKRunStateSubsystem`. Terminal, from any phase. */
	bool bCrewWiped = false;
};

/** One step's transitions, so the caller broadcasts and this layer decides. */
struct FLLKExtractionStep
{
	FLLKExtractionState State;

	/** The siren finished this step. The rig is at the dock and the hold has begun. */
	bool bRigArrived = false;

	bool bSecured = false;
	bool bFailed = false;

	/** Holding, and the hold did not move: nobody on the route, or nobody measuring it. */
	bool bHoldStalled = false;
};

namespace LLKExtraction
{
	/**
	 * Announce one. Returns the new state; `OutRefusal` says why if nothing moved.
	 *
	 * `bRunIsOver` rather than a run-state pointer, and `bRequirementMet` rather
	 * than a restoration subsystem, for the usual reason: both are one bool by
	 * the time they reach a rule, and taking the bool is what makes the rule
	 * testable without the systems that answer it.
	 */
	LLK_API FLLKExtractionState Call(
		const FLLKExtractionState& State,
		const FLLKExtractionPointRules& Point,
		const FLLKExtractionAnnounce& Announce,
		bool bRequirementMet,
		bool bRunIsOver,
		ELLKExtractionRefusal& OutRefusal);

	/**
	 * Call it off. Legal while announcing or holding, and nowhere else.
	 *
	 * The siren that already sounded is not un-sounded, which is the whole
	 * shape of the decision: cancelling buys back the route, never the noise.
	 */
	LLK_API FLLKExtractionState Cancel(const FLLKExtractionState& State, bool& bOutCancelled);

	/** One step of time. The only function here that can finish a run. */
	LLK_API FLLKExtractionStep Advance(
		const FLLKExtractionState& State, const FLLKExtractionInput& Input);

	LLK_API const TCHAR* NameOf(ELLKExtractionPhase Phase);
	LLK_API const TCHAR* NameOf(ELLKExtractionRefusal Refusal);

	/** 0 before the hold begins, 1 when it is done. For a readout, never for a rule. */
	LLK_API float HoldFraction(const FLLKExtractionState& State);
}

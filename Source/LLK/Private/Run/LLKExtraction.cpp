#include "Run/LLKExtraction.h"

namespace
{
	/** Non-finite authored data is zero rather than poison. The lantern feel's rule. */
	float Finite(float Value)
	{
		return FMath::IsFinite(Value) ? Value : 0.f;
	}
}

namespace LLKExtraction
{
	FLLKExtractionState Call(
		const FLLKExtractionState& State,
		const FLLKExtractionPointRules& Point,
		const FLLKExtractionAnnounce& Announce,
		const bool bRequirementMet,
		const bool bRunIsOver,
		ELLKExtractionRefusal& OutRefusal)
	{
		// Order matters and this is the order: the run being over outranks
		// everything, because a refusal that said "requirement not met" to a
		// wiped crew would be technically true and actively misleading.
		if (bRunIsOver || State.Phase == ELLKExtractionPhase::Failed)
		{
			OutRefusal = ELLKExtractionRefusal::RunIsOver;
			return State;
		}

		if (State.Phase == ELLKExtractionPhase::Secured)
		{
			OutRefusal = ELLKExtractionRefusal::AlreadyExtracted;
			return State;
		}

		if (State.IsUnderway())
		{
			OutRefusal = ELLKExtractionRefusal::AlreadyUnderway;
			return State;
		}

		if (Point.Id.IsNone())
		{
			OutRefusal = ELLKExtractionRefusal::NoSuchPoint;
			return State;
		}

		const float Hold = Finite(Point.HoldSeconds);
		if (Hold <= 0.f)
		{
			// `loot.json` says it in words - "never zero: an exit that costs
			// nothing is not a decision" - and this is where that sentence
			// becomes a refusal rather than a comment. An exit that secures
			// instantly is a door, which is the thing row 74 and the thesis
			// both exist to prevent.
			OutRefusal = ELLKExtractionRefusal::NoHoldAuthored;
			return State;
		}

		if (!bRequirementMet)
		{
			OutRefusal = ELLKExtractionRefusal::RequirementNotMet;
			return State;
		}

		OutRefusal = ELLKExtractionRefusal::None;

		FLLKExtractionState Next = State;
		Next.Phase = ELLKExtractionPhase::Announcing;
		Next.PointId = Point.Id;
		Next.SirenSecondsRemaining = FMath::Max(0.f, Finite(Announce.SirenSeconds));
		Next.HoldSecondsRequired = Hold;
		Next.HoldSecondsRemaining = Hold;

		// A payout of zero is almost certainly missing data rather than a free
		// extraction, so it falls back to the authored default of 1 instead of
		// quietly paying nobody for a hold they survived.
		const float Payout = Finite(Point.PayoutModifier);
		Next.PayoutModifier = Payout > 0.f ? Payout : 1.f;

		Next.CallCount = State.CallCount + 1;
		return Next;
	}

	FLLKExtractionState Cancel(const FLLKExtractionState& State, bool& bOutCancelled)
	{
		if (!State.IsUnderway())
		{
			bOutCancelled = false;
			return State;
		}

		bOutCancelled = true;

		FLLKExtractionState Next = State;
		Next.Phase = ELLKExtractionPhase::Idle;
		Next.PointId = NAME_None;
		Next.SirenSecondsRemaining = 0.f;
		Next.HoldSecondsRemaining = 0.f;
		Next.HoldSecondsRequired = 0.f;
		Next.PayoutModifier = 1.f;

		// The counts survive, which is the only part of a cancel that does.
		Next.CallCount = State.CallCount;
		Next.CancelCount = State.CancelCount + 1;
		return Next;
	}

	FLLKExtractionStep Advance(const FLLKExtractionState& State, const FLLKExtractionInput& Input)
	{
		FLLKExtractionStep Step;
		Step.State = State;

		// Terminal is terminal. Checked before the wipe, so a crew that
		// secured and then wiped on the way to the truck still extracted -
		// the haul is out, and nothing that happens afterwards takes it back.
		if (State.IsTerminal())
		{
			return Step;
		}

		// A wipe ends it from any phase, including Idle. An Idle wipe reads
		// oddly until you say it out loud: the run is over and no extraction
		// happened, which is exactly what Failed means.
		if (Input.bCrewWiped)
		{
			Step.State.Phase = ELLKExtractionPhase::Failed;
			Step.State.SirenSecondsRemaining = 0.f;
			Step.bFailed = true;
			return Step;
		}

		if (State.Phase == ELLKExtractionPhase::Idle)
		{
			return Step;
		}

		const float Delta = Finite(Input.DeltaSeconds);
		if (Delta <= 0.f)
		{
			// A hitching or paused frame is not progress and not a rewind.
			// Reported as stalled while holding, because a readout saying the
			// hold is running during a zero-length step would be a lie.
			Step.bHoldStalled = State.Phase == ELLKExtractionPhase::Holding;
			return Step;
		}

		float Remaining = Delta;

		if (Step.State.Phase == ELLKExtractionPhase::Announcing)
		{
			const float Spent = FMath::Min(Remaining, Step.State.SirenSecondsRemaining);
			Step.State.SirenSecondsRemaining -= Spent;
			Remaining -= Spent;

			if (Step.State.SirenSecondsRemaining > 0.f)
			{
				return Step;
			}

			Step.State.SirenSecondsRemaining = 0.f;
			Step.State.Phase = ELLKExtractionPhase::Holding;
			Step.bRigArrived = true;

			// The leftover delta carries into the hold rather than being
			// dropped at the phase boundary. At sixty frames a second the
			// difference is a sixtieth of a second per extraction, which
			// nobody could feel - but a rule that loses time in proportion to
			// frame length is a rule that behaves differently on a slower
			// machine, and that is the class of bug this project keeps
			// choosing to not have.
		}

		if (Step.State.Phase != ELLKExtractionPhase::Holding)
		{
			return Step;
		}

		// Unset is nobody measuring, which cannot advance a hold. The dock
		// does not exist yet, so this is today's normal case and it must not
		// resolve a run on its own.
		if (!Input.KeepersOnBoardingRoute.IsSet())
		{
			Step.bHoldStalled = true;
			return Step;
		}

		if (Input.KeepersOnBoardingRoute.GetValue() <= 0)
		{
			// Paused, not reset. The header says why, and says it is the one
			// line to flip if Chris wants the harsher deal.
			Step.bHoldStalled = true;
			return Step;
		}

		if (Remaining <= 0.f)
		{
			// The whole step was spent on the siren. Somebody is on the route
			// and the rig has only just arrived, so this is not a stall.
			return Step;
		}

		Step.State.HoldSecondsRemaining = FMath::Max(0.f, Step.State.HoldSecondsRemaining - Remaining);
		if (Step.State.HoldSecondsRemaining <= 0.f)
		{
			Step.State.HoldSecondsRemaining = 0.f;
			Step.State.Phase = ELLKExtractionPhase::Secured;
			Step.bSecured = true;
		}

		return Step;
	}

	const TCHAR* NameOf(const ELLKExtractionPhase Phase)
	{
		switch (Phase)
		{
		case ELLKExtractionPhase::Idle:       return TEXT("Idle");
		case ELLKExtractionPhase::Announcing: return TEXT("Announcing");
		case ELLKExtractionPhase::Holding:    return TEXT("Holding");
		case ELLKExtractionPhase::Secured:    return TEXT("Secured");
		case ELLKExtractionPhase::Failed:     return TEXT("Failed");
		}
		return TEXT("Unknown");
	}

	const TCHAR* NameOf(const ELLKExtractionRefusal Refusal)
	{
		switch (Refusal)
		{
		case ELLKExtractionRefusal::None:              return TEXT("None");
		case ELLKExtractionRefusal::NoSuchPoint:       return TEXT("NoSuchPoint");
		case ELLKExtractionRefusal::NoHoldAuthored:    return TEXT("NoHoldAuthored");
		case ELLKExtractionRefusal::RequirementNotMet: return TEXT("RequirementNotMet");
		case ELLKExtractionRefusal::AlreadyUnderway:   return TEXT("AlreadyUnderway");
		case ELLKExtractionRefusal::AlreadyExtracted:  return TEXT("AlreadyExtracted");
		case ELLKExtractionRefusal::RunIsOver:         return TEXT("RunIsOver");
		}
		return TEXT("Unknown");
	}

	float HoldFraction(const FLLKExtractionState& State)
	{
		if (State.Phase == ELLKExtractionPhase::Secured)
		{
			return 1.f;
		}

		if (State.Phase != ELLKExtractionPhase::Holding || State.HoldSecondsRequired <= 0.f)
		{
			return 0.f;
		}

		const float Done = State.HoldSecondsRequired - State.HoldSecondsRemaining;
		return FMath::Clamp(Done / State.HoldSecondsRequired, 0.f, 1.f);
	}
}

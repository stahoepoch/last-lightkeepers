#include "Enemies/LLKSweeperBrain.h"

// **Named, not anonymous, and that is a unity-build rule this project has
// already paid for once.** Anonymous namespaces from several files merge into
// one translation unit, and `LLKExtraction.cpp` has its own `Finite` - adding
// a second one broke the build the moment the two landed in the same chunk.
namespace LLKSweeperBrainDetail
{
	float Finite(float Value)
	{
		return FMath::IsFinite(Value) ? Value : 0.f;
	}

	/**
	 * How long a Sweeper searches when the authored value is missing.
	 *
	 * `losesTargetAfterSeconds` is a `TOptional` on the def and Sweepers do
	 * not author one - it belongs to the Stockers' sight cone. Three seconds
	 * is declared here rather than silently zero, because zero would mean a
	 * Sweeper that forgets you the instant your lantern goes out, which makes
	 * light a switch instead of a gamble.
	 */
	constexpr float FallbackSearchSeconds = 3.f;

	/**
	 * How long a Sweeper spends filing one misplaced thing.
	 *
	 * **Declared, not authored, and it is Chris's to move.** `enemies.json`
	 * has no key for this and every number in that file is his, so this
	 * follows `FallbackSearchSeconds`' precedent rather than inventing a
	 * balance value in a header. The number matters to play: it is the window
	 * in which a player can decide to light up and be noticed in order to
	 * interrupt the work. Too short and the choice does not exist; too long
	 * and the Sweeper is furniture.
	 *
	 * Two seconds is one second longer than the Sweeper's own
	 * `audioTelegraphSeconds`, so a player who reacts to the start-of-work
	 * cue has time to act on it. If Chris authors `stockSeconds`, read it
	 * here and delete this.
	 */
	constexpr float FallbackStockSeconds = 2.f;
}

namespace LLKSweeperBrain
{
	FLLKSweeperStep Advance(
		const FLLKSweeperMind& Mind,
		const FLLKSweeperSenses& Senses,
		const FLLKEnemyDef& Def,
		const FLLKSweeperContactRules& Contact)
	{
		FLLKSweeperStep Step;
		Step.Mind = Mind;

		const float Delta = FMath::Max(0.f, LLKSweeperBrainDetail::Finite(Senses.DeltaSeconds));

		// ---- the clocks, which run whatever the state is -----------------
		//
		// **Each hit expires on its own clock**, which is what
		// `hitDecaySeconds` actually says. The first version of this drained
		// one float continuously and therefore drained through the cooldown as
		// well: three hits never stacked and a Sweeper needed nine touches,
		// which `LLK.Enemies.SweeperBrain` reported as "it takes exactly the
		// authored three ... but it was 9".
		//
		// Aged forward and expired here, before anything else, so a Sweeper
		// that broke off and came back does not inherit stale hits.
		const float DecaySeconds = FMath::Max(0.f, LLKSweeperBrainDetail::Finite(Contact.HitDecaySeconds));
		for (int32 Index = Step.Mind.HitAgesSeconds.Num() - 1; Index >= 0; --Index)
		{
			Step.Mind.HitAgesSeconds[Index] += Delta;
			if (DecaySeconds > 0.f && Step.Mind.HitAgesSeconds[Index] >= DecaySeconds)
			{
				Step.Mind.HitAgesSeconds.RemoveAt(Index);
			}
		}

		Step.Mind.ContactCooldownRemaining =
			FMath::Max(0.f, Step.Mind.ContactCooldownRemaining - Delta);

		// ---- was the keeper noticed this frame ---------------------------
		//
		// One question, asked of `LLKPerception` and never re-derived here.
		// `bNoticed` is already false for `Unknown` visibility - a trace that
		// could not be made is not a sighting - so a brain that looked at the
		// distance itself could pursue through geometry nobody measured.
		const bool bNoticed = Senses.Notice.bNoticed;

		if (bNoticed)
		{
			Step.Mind.LastKnownKeeperPosition = Senses.KeeperPosition;
			Step.Mind.bHasLastKnown = true;
		}

		// ---- the states --------------------------------------------------
		switch (Step.Mind.State)
		{
		case ELLKSweeperState::Patrol:
			if (bNoticed)
			{
				// **Row 16, and it is a build requirement rather than polish.**
				// Every enemy announces itself with sound before sight. A
				// Sweeper that noticed you and started walking in the same
				// frame would be a fair fight with an unfair presentation.
				Step.Mind.State = ELLKSweeperState::Telegraph;
				Step.Mind.TelegraphSecondsRemaining =
					FMath::Max(0.f, LLKSweeperBrainDetail::Finite(Def.AudioTelegraphSeconds));
				Step.bTelegraphStarted = true;
				Step.Move = ELLKSweeperMove::Hold;
				break;
			}

			// Unset is nobody measuring: no patrol points in the map. It holds
			// rather than inventing somewhere to be, and the pawn says so by
			// name - the same seam the boarding route has.
			// Nothing has noticed anybody. Is there stock in the wrong place?
			//
			// Gated on Chris's authored `attractedToDroppedLoot` and on the
			// map having a bay to put it in. Both false by default, so a
			// Sweeper in a map that authored neither behaves exactly as it
			// did before this state existed.
			if (Def.bAttractedToDroppedLoot.Get(false)
				&& Senses.bHasStockBay
				&& Senses.MisplacedItemPosition.IsSet()
				&& Contact.ContactRange > 0.f
				&& Senses.DistanceToMisplacedStuds <= Contact.ContactRange)
			{
				Step.Mind.State = ELLKSweeperState::Stock;
				Step.Mind.StockSecondsRemaining = LLKSweeperBrainDetail::FallbackStockSeconds;
				Step.bStockStarted = true;
				Step.Move = ELLKSweeperMove::Hold;
				break;
			}

			Step.Move = Senses.bHasPatrolPoint.Get(false)
				? ELLKSweeperMove::PatrolPoint : ELLKSweeperMove::Hold;
			break;

		case ELLKSweeperState::Stock:
			// A keeper beats the work, every time. This is the whole decision
			// the state creates: being seen is how you stop it, and a player
			// who lights up to save their crate has paid the right price.
			if (bNoticed)
			{
				Step.Mind.State = ELLKSweeperState::Telegraph;
				Step.Mind.TelegraphSecondsRemaining =
					FMath::Max(0.f, LLKSweeperBrainDetail::Finite(Def.AudioTelegraphSeconds));
				Step.Mind.StockSecondsRemaining = 0.f;
				Step.bTelegraphStarted = true;
				Step.bStockInterrupted = true;
				Step.Move = ELLKSweeperMove::Hold;
				break;
			}

			// The thing left reach while it was working - the keeper picked it
			// up, or another Sweeper filed it first. Abandon the job rather
			// than finishing it on nothing, which would relocate whatever
			// happened to be nearest when the timer expired.
			if (!Senses.MisplacedItemPosition.IsSet())
			{
				Step.Mind.State = ELLKSweeperState::Patrol;
				Step.Mind.StockSecondsRemaining = 0.f;
				Step.Move = Senses.bHasPatrolPoint.Get(false)
					? ELLKSweeperMove::PatrolPoint : ELLKSweeperMove::Hold;
				break;
			}

			Step.Mind.StockSecondsRemaining =
				FMath::Max(0.f, Step.Mind.StockSecondsRemaining - Delta);

			Step.Move = ELLKSweeperMove::Hold;

			if (Step.Mind.StockSecondsRemaining > 0.f)
			{
				break;
			}

			Step.bItemShelved = true;
			Step.Mind.State = ELLKSweeperState::Patrol;
			Step.Move = Senses.bHasPatrolPoint.Get(false)
				? ELLKSweeperMove::PatrolPoint : ELLKSweeperMove::Hold;
			break;

		case ELLKSweeperState::Telegraph:
			Step.Mind.TelegraphSecondsRemaining =
				FMath::Max(0.f, Step.Mind.TelegraphSecondsRemaining - Delta);

			// **It holds still while it announces.** The arguable half, and the
			// reason is that a warning a player cannot use is not a warning:
			// one second of nothing moving is one second of head start. See the
			// header if this ever wants to change.
			Step.Move = ELLKSweeperMove::Hold;

			if (Step.Mind.TelegraphSecondsRemaining > 0.f)
			{
				break;
			}

			// Done announcing. Where it goes next depends on whether the
			// keeper is still there - a keeper who used the second well is
			// searched for, not followed.
			if (bNoticed)
			{
				Step.Mind.State = ELLKSweeperState::Pursue;
				Step.Move = ELLKSweeperMove::Keeper;
			}
			else
			{
				Step.Mind.State = ELLKSweeperState::Search;
				Step.Mind.SearchSecondsRemaining =
					Def.LosesTargetAfterSeconds.Get(LLKSweeperBrainDetail::FallbackSearchSeconds);
				Step.Move = Step.Mind.bHasLastKnown
					? ELLKSweeperMove::LastKnown : ELLKSweeperMove::Hold;
			}
			break;

		case ELLKSweeperState::Pursue:
			if (!bNoticed)
			{
				// Lost. Not forgotten: it goes to where you were, which is the
				// difference between pressure and omniscience.
				Step.Mind.State = ELLKSweeperState::Search;
				Step.Mind.SearchSecondsRemaining =
					Def.LosesTargetAfterSeconds.Get(LLKSweeperBrainDetail::FallbackSearchSeconds);
				Step.Move = Step.Mind.bHasLastKnown
					? ELLKSweeperMove::LastKnown : ELLKSweeperMove::Hold;
				break;
			}

			Step.Move = ELLKSweeperMove::Keeper;

			// ---- contact, which only Pursue can do ----------------------
			//
			// `contactRange` is 5 studs and `hitsToKnockDown` is 3 - the
			// authored numbers that make a Sweeper *"weak alone"*. A single
			// Sweeper needs four and a half seconds of touching you at a
			// 1.5-second cooldown, which is survivable; three of them is not,
			// and that is the swarm the data describes.
			if (Step.Mind.ContactCooldownRemaining <= 0.f
				&& Contact.ContactRange > 0.f
				&& Senses.DistanceToKeeperStuds <= Contact.ContactRange)
			{
				Step.bContactLanded = true;
				Step.Mind.HitAgesSeconds.Add(0.f);
				Step.Mind.ContactCooldownRemaining =
					FMath::Max(0.f, LLKSweeperBrainDetail::Finite(Contact.ContactCooldownSeconds));

				const int32 Needed = FMath::Max(1, Contact.HitsToKnockDown);
				if (Contact.bKnockdownOnContact && Step.Mind.HitsLanded() >= Needed)
				{
					Step.bKnockedDown = true;

					// Spent. The stack does not carry over into the next
					// knockdown, or a revived keeper would be one touch from
					// the floor again - which is row 9's bleed-out problem
					// wearing a different hat.
					Step.Mind.HitAgesSeconds.Reset();
				}
			}
			break;

		case ELLKSweeperState::Search:
			if (bNoticed)
			{
				// Found again. No second telegraph: it never stopped hunting,
				// and re-announcing would turn a tense re-acquisition into a
				// free second every time a keeper rounded a corner.
				Step.Mind.State = ELLKSweeperState::Pursue;
				Step.Move = ELLKSweeperMove::Keeper;
				break;
			}

			Step.Mind.SearchSecondsRemaining =
				FMath::Max(0.f, Step.Mind.SearchSecondsRemaining - Delta);

			if (Step.Mind.SearchSecondsRemaining > 0.f)
			{
				Step.Move = Step.Mind.bHasLastKnown
					? ELLKSweeperMove::LastKnown : ELLKSweeperMove::Hold;
				break;
			}

			// Given up. The last known position is cleared with the state:
			// a Sweeper that kept it would walk back to a place the keeper
			// left two minutes ago the next time it noticed anything.
			Step.Mind.State = ELLKSweeperState::Patrol;
			Step.Mind.bHasLastKnown = false;
			Step.Mind.LastKnownKeeperPosition = FVector::ZeroVector;
			Step.bLostTheKeeper = true;
			Step.Move = Senses.bHasPatrolPoint.Get(false)
				? ELLKSweeperMove::PatrolPoint : ELLKSweeperMove::Hold;
			break;
		}

		return Step;
	}

	const TCHAR* NameOf(const ELLKSweeperState State)
	{
		switch (State)
		{
		case ELLKSweeperState::Patrol:    return TEXT("Patrol");
		case ELLKSweeperState::Telegraph: return TEXT("Telegraph");
		case ELLKSweeperState::Pursue:    return TEXT("Pursue");
		case ELLKSweeperState::Search:    return TEXT("Search");
		case ELLKSweeperState::Stock:     return TEXT("Stock");
		}
		return TEXT("Unknown");
	}

	const TCHAR* NameOf(const ELLKSweeperMove Move)
	{
		switch (Move)
		{
		case ELLKSweeperMove::Hold:        return TEXT("Hold");
		case ELLKSweeperMove::PatrolPoint: return TEXT("PatrolPoint");
		case ELLKSweeperMove::Keeper:      return TEXT("Keeper");
		case ELLKSweeperMove::LastKnown:   return TEXT("LastKnown");
		}
		return TEXT("Unknown");
	}
}

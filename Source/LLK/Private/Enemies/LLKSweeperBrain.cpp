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

	/**
	 * How long a Sweeper stands at a patrol point before taking the next one.
	 *
	 * **Declared, not authored, and Chris's to move** - same precedent as
	 * `FallbackSearchSeconds` and `FallbackStockSeconds`, because
	 * `enemies.json` has no key for it and every number in that file is his.
	 *
	 * 2.5 seconds, reasoned rather than picked: it sits just above the
	 * 2-second stock job so a pause reads as the machine doing something
	 * deliberate rather than stalling, and well above the 1-second audio
	 * telegraph so it is perceptible as a rhythm at all. Much shorter and
	 * there is no crossing window - a keeper walks 448 cm/s, so a 5-stud gap
	 * takes about a third of a second and the pause has to be long enough to
	 * decide in, not merely to move in. Much longer and the Sweeper spends
	 * most of its night standing still, which is a different wrong answer.
	 *
	 * If Chris authors `patrolDwellSeconds`, read it here and delete this.
	 */
	constexpr float FallbackPatrolDwellSeconds = 2.5f;
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
		Step.Mind.ContactRecoverySecondsRemaining =
			FMath::Max(0.f, Step.Mind.ContactRecoverySecondsRemaining - Delta);

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
				// A keeper beats the route. Dropping the dwell here means it
				// does not resume a half-spent pause minutes later, somewhere
				// else, for no reason a player could read.
				Step.Mind.PatrolDwellSecondsRemaining = 0.f;
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
				&& Senses.DistanceToMisplacedStuds <= Def.AggroRadiusDark.Get(0.f))
			{
				Step.Mind.State = ELLKSweeperState::Stock;
				Step.Mind.StockSecondsRemaining = LLKSweeperBrainDetail::FallbackStockSeconds;
				Step.Mind.StockApproachSecondsRemaining =
					Def.LosesTargetAfterSeconds.Get(LLKSweeperBrainDetail::FallbackSearchSeconds);
				Step.Mind.PatrolDwellSecondsRemaining = 0.f;
				Step.bStockStarted = true;

				// It may already be in reach, in which case it works where it
				// stands. Deciding that here rather than next frame keeps the
				// in-reach case identical to the behaviour the floor already
				// observed at 3.59 studs.
				Step.Move = (Contact.ContactRange > 0.f
					&& Senses.DistanceToMisplacedStuds <= Contact.ContactRange)
					? ELLKSweeperMove::Hold : ELLKSweeperMove::MisplacedItem;
				break;
			}

			// Mid-dwell: standing at a point, doing the job it stopped for.
			if (Step.Mind.PatrolDwellSecondsRemaining > 0.f)
			{
				Step.Mind.PatrolDwellSecondsRemaining =
					FMath::Max(0.f, Step.Mind.PatrolDwellSecondsRemaining - Delta);

				if (Step.Mind.PatrolDwellSecondsRemaining > 0.f)
				{
					Step.Move = ELLKSweeperMove::Hold;
					break;
				}

				// Done standing. The route moves on now, and not before -
				// which is the whole reason this decision lives here rather
				// than in the pawn.
				Step.bAdvancePatrolPoint = true;
				Step.Move = Senses.bHasPatrolPoint.Get(false)
					? ELLKSweeperMove::PatrolPoint : ELLKSweeperMove::Hold;
				break;
			}

			// Just arrived. Stop and start the clock.
			if (Senses.bAtPatrolPoint)
			{
				Step.Mind.PatrolDwellSecondsRemaining =
					LLKSweeperBrainDetail::FallbackPatrolDwellSeconds;
				Step.bDwellStarted = true;
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
				Step.Mind.StockApproachSecondsRemaining = 0.f;
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
				Step.Mind.StockApproachSecondsRemaining = 0.f;
				Step.Move = Senses.bHasPatrolPoint.Get(false)
					? ELLKSweeperMove::PatrolPoint : ELLKSweeperMove::Hold;
				break;
			}

			// Not in reach yet: walk, on a budget. The work timer does not run
			// while it is still on its way - otherwise a Sweeper that noticed
			// something across the aisle would file it from a distance.
			if (Contact.ContactRange <= 0.f
				|| Senses.DistanceToMisplacedStuds > Contact.ContactRange)
			{
				Step.Mind.StockApproachSecondsRemaining =
					FMath::Max(0.f, Step.Mind.StockApproachSecondsRemaining - Delta);

				if (Step.Mind.StockApproachSecondsRemaining <= 0.f)
				{
					// Could not get there. Behind glass, up a step, or a hole
					// in the navmesh. Go back to work rather than standing at
					// it forever.
					Step.Mind.State = ELLKSweeperState::Patrol;
					Step.Mind.StockSecondsRemaining = 0.f;
					Step.bStockAbandoned = true;
					Step.Move = Senses.bHasPatrolPoint.Get(false)
						? ELLKSweeperMove::PatrolPoint : ELLKSweeperMove::Hold;
					break;
				}

				Step.Move = ELLKSweeperMove::MisplacedItem;
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
			Step.Mind.StockApproachSecondsRemaining = 0.f;
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
				// This is an ordinary lost sighting, not a contact. It goes to
				// the last place it saw the keeper; only a physical contact earns
				// the anti-pin recovery hold in the Search state below.
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

				// Every contact smacks your feet. It is a floor machine, and
				// that is what being clipped by one is.
				Step.bStumbleLanded = true;

				const int32 Needed = FMath::Max(1, Contact.HitsToKnockDown);
				if (Step.Mind.HitsLanded() >= Needed)
				{
					// **The threshold means two things now, and
					// `knockdownOnContact` is the switch.** True: a containment
					// enemy puts the keeper down. False: a maintenance machine
					// files a fault and the floor starts paying attention. Same
					// count, same accumulator, same decay - a different
					// consequence for a different kind of machine.
					if (Contact.bKnockdownOnContact)
					{
						// You do not stumble and go down from the same hit.
						Step.bStumbleLanded = false;
						Step.bKnockedDown = true;
					}
					else
					{
						Step.bFloorAlerted = true;
					}

					// Spent. The stack does not carry over into the next
					// knockdown, or a revived keeper would be one touch from
					// the floor again - which is row 9's bleed-out problem
					// wearing a different hat.
					Step.Mind.HitAgesSeconds.Reset();
				}

				// A contact is a trip, not a containment grapple. The already
				// established patrol rhythm is the recovery: this machine works
				// where it clipped the keeper rather than snapping back into a
				// visible target after the shorter hit cooldown. No new balance
				// value is introduced; this is FallbackPatrolDwellSeconds, the
				// same interval that already makes its route readable. It remains
				// short enough for three contacts to fit inside the authored
				// hit-decay window, so a machine can still report a repeated fault.
				Step.Mind.State = ELLKSweeperState::Search;
				Step.Mind.ContactRecoverySecondsRemaining =
					LLKSweeperBrainDetail::FallbackPatrolDwellSeconds;
				Step.Mind.SearchSecondsRemaining =
					LLKSweeperBrainDetail::FallbackPatrolDwellSeconds;
				// Stop immediately on contact. A movement request here is made
				// after the hit has already landed, so it can only push further
				// into the same occupied aisle before recovery starts next tick.
				Step.Move = ELLKSweeperMove::Hold;
			}
			break;

		case ELLKSweeperState::Search:
			if (bNoticed
				&& Step.Mind.ContactCooldownRemaining <= 0.f
				&& Step.Mind.ContactRecoverySecondsRemaining <= 0.f)
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
				if (Step.Mind.ContactRecoverySecondsRemaining > 0.f)
				{
					// Contact recovery is a physical reset, not a pursuit in
					// disguise. See the contact branch above: holding here
					// prevents an overlap capsule from turning narrow retail
					// geometry into a trap.
					Step.Move = ELLKSweeperMove::Hold;
					break;
				}

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
		case ELLKSweeperMove::MisplacedItem: return TEXT("MisplacedItem");
		}
		return TEXT("Unknown");
	}
}

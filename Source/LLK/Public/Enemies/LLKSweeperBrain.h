// What a Sweeper decides, as a rule. M3's first encounter.
//
// **Nothing in this game walks yet.** `LLKPerception` answers whether a keeper
// is noticed and `LLKVisibility` answers whether a wall was in the way, and
// between them they have never moved an enemy one centimetre - there is no
// Sweeper pawn, no controller and no patrol. This is the half of that which
// can be built and judged before a map exists: the decision. The pawn that
// carries it out needs a navmesh, and a navmesh needs Chris's floor.
//
// **The three states and the one that matters.** Patrol, Telegraph, Pursue,
// Search. `Telegraph` is not a flourish - `DECISIONS.md` row 16 makes it a
// build requirement: every enemy announces itself with sound before sight,
// distinguishably. `enemies.json` authors `audioTelegraphSeconds: 1.0` for
// Sweepers, and that second is the whole difference between a horror game and
// an unfair one. So a Sweeper that has noticed you CANNOT move toward you
// until it has finished saying so.
//
// **The design decision inside that, flagged because it is mine and arguable:**
// during the telegraph the Sweeper holds still. It could instead close slowly,
// which is more menacing and gives the warning less value. Holding still makes
// the second a real head start, which is what a warning is for - and a keeper
// who hears a Sweeper wake and gets away has learned the lesson the sound
// exists to teach. If Chris wants it to advance during the telegraph, it is
// one branch and the test that fails first is named.
//
// **Losing you is not forgetting you.** Light makes the difference enormous:
// `aggroRadiusLit` is 38 studs against `aggroRadiusDark`'s 12, so killing a
// lantern is how a chase ends. But a Sweeper that lost its notice and
// instantly returned to patrol would make light a switch rather than a
// gamble, so it searches the last place it had you for
// `losesTargetAfterSeconds` first. It never searches somewhere it never saw
// you, which is the difference between pressure and omniscience.
//
// **`Unknown` visibility never pursues.** A trace that could not be made is
// not a wall and not a sighting; `LLKPerception` already refuses to notice on
// it, and this refuses to act on it a second time, because an enemy that
// guesses on a failed trace is an enemy that hunts you through geometry
// nobody can see.

#pragma once

#include "CoreMinimal.h"
#include "Data/LLKEnemyDef.h"
#include "Data/LLKLanternData.h"
#include "Enemies/LLKPerception.h"
#include "LLKSweeperBrain.generated.h"

/** Where a Sweeper has got to. */
UENUM(BlueprintType)
enum class ELLKSweeperState : uint8
{
	/** Walking its route. Most of a Sweeper's night. */
	Patrol,

	/** Row 16: announcing itself. It has noticed you and has not moved yet. */
	Telegraph,

	/** Coming. */
	Pursue,

	/** Lost you, and looking where you were. Never where you are. */
	Search,

	/**
	 * Dealing with stock in the wrong place. **It has not noticed you.**
	 *
	 * This is the state row 47 is made of, and it is the one the first
	 * encounter should be. `enemies.json` authors
	 * `Sweepers.attractedToDroppedLoot: true` - Chris's key, imported, asserted
	 * for presence by `tests/balance.test.mjs`, and until now consumed by
	 * nothing. A Sweeper that walks its route and *files the crate you were
	 * walking toward* teaches indifference in one beat with no dialogue, and
	 * it costs the player something without ever threatening them.
	 *
	 * **Being noticed interrupts it**, which is the decision this creates: the
	 * only way to stop a Sweeper taking your crate is to let it see you. Light
	 * is already a rate on the held pickup; here it is the price of
	 * interrupting the building's work.
	 *
	 * It never deletes anything. `docs/WORLD-SYSTEMS-BOARD.md` is explicit -
	 * no arbitrary bag deletion, a shelved thing stays in the world with a
	 * recoverable destination. The pawn moves it to an authored stock bay or
	 * this state never starts.
	 */
	Stock
};

/** What the pawn should be moving toward this frame. */
UENUM(BlueprintType)
enum class ELLKSweeperMove : uint8
{
	/** Stand still. The telegraph, and a Sweeper with nowhere to be. */
	Hold,

	/** The next point on its route. */
	PatrolPoint,

	/** The keeper's position, as last measured. */
	Keeper,

	/** Where the keeper was when it lost them. */
	LastKnown
};

/** A Sweeper's own state, carried between frames. */
USTRUCT(BlueprintType)
struct FLLKSweeperMind
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Enemies")
	ELLKSweeperState State = ELLKSweeperState::Patrol;

	/** Counts down during Telegraph. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Enemies")
	float TelegraphSecondsRemaining = 0.f;

	/** Counts down during Search, from `losesTargetAfterSeconds`. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Enemies")
	float SearchSecondsRemaining = 0.f;

	/** Counts down during Stock. The seconds it spends filing the thing. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Enemies")
	float StockSecondsRemaining = 0.f;

	/** Only meaningful in Pursue and Search. Set when it last had the keeper. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Enemies")
	FVector LastKnownKeeperPosition = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Enemies")
	bool bHasLastKnown = false;

	/**
	 * How long ago each landed hit was, in seconds. `hitsToKnockDown` is 3.
	 *
	 * **An age per hit, because the first model was wrong and a test caught
	 * it.** Draining a single float continuously seemed like the cheap version
	 * of `hitDecaySeconds`, and it drained during the 1.5-second cooldown too -
	 * so three hits never accumulated and a Sweeper needed NINE touches to put
	 * a keeper down. `hitDecaySeconds: 6` means a hit stops counting six
	 * seconds after it lands, which is three hits at a 1.5-second cooldown
	 * comfortably inside the window, and four and a half seconds of being
	 * touched is what *"weak alone"* is supposed to cost.
	 *
	 * At most `hitsToKnockDown` entries ever live here - three - so the array
	 * is smaller than the float plus the timestamp it would have needed.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Enemies")
	TArray<float> HitAgesSeconds;

	/** Hits still counting toward a knockdown. */
	int32 HitsLanded() const { return HitAgesSeconds.Num(); }

	/** Time until it may touch the keeper again. `contactCooldownSeconds` is 1.5. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Enemies")
	float ContactCooldownRemaining = 0.f;
};

/** What the world looked like this frame. Measured by the pawn, concluded by nobody. */
struct FLLKSweeperSenses
{
	float DeltaSeconds = 0.f;

	/** From `LLKPerception::Notice`. The only route by which a keeper is seen. */
	FLLKNotice Notice;

	/** Where the keeper is, as measured. Only read when the notice is positive. */
	FVector KeeperPosition = FVector::ZeroVector;

	/** Studs, already converted. Row 61 keeps the conversion at the boundary. */
	float DistanceToKeeperStuds = 0.f;

	/**
	 * Whether the route has a next point. Unset when nobody measured.
	 *
	 * The same `TOptional` discipline as the boarding route: a map with no
	 * patrol points is a different fact from a Sweeper standing at the end of
	 * its route, and only the second one is a Sweeper behaving correctly.
	 */
	TOptional<bool> bHasPatrolPoint;

	/**
	 * Where the nearest thing that is in the wrong place is. Unset when there
	 * is none in reach, or when nobody looked.
	 *
	 * Deliberately **not** a separate sense radius. The Sweeper deals with
	 * what its ordinary route brings it within `ContactRange` of, so the
	 * horror is that its predictable lane is what costs you - not that it
	 * acquired a new appetite. It also means no invented number: the range is
	 * the authored contact range it already reaches keepers with.
	 */
	TOptional<FVector> MisplacedItemPosition;

	/** Studs to that thing. Only read when the position is set. */
	float DistanceToMisplacedStuds = 0.f;

	/**
	 * Whether the map authored somewhere for a shelved thing to GO.
	 *
	 * The gate that makes this safe to land before the map has one. No bay,
	 * no `Stock` state, and the slice behaves exactly as it did - because a
	 * Sweeper that filed something into nowhere would be deleting a player's
	 * objective, which the board forbids outright.
	 */
	bool bHasStockBay = false;
};

/** One step's decision, plus the two things a pawn must act on once. */
struct FLLKSweeperStep
{
	FLLKSweeperMind Mind;

	ELLKSweeperMove Move = ELLKSweeperMove::Hold;

	/** Row 16: play the telegraph now. True on the frame it begins, never after. */
	bool bTelegraphStarted = false;

	/** It reached the keeper and the cooldown allowed it. One hit. */
	bool bContactLanded = false;

	/** That hit was the third. The keeper goes down. */
	bool bKnockedDown = false;

	/** Gave up and went back to its route this frame. For a log, and for a readout. */
	bool bLostTheKeeper = false;

	/**
	 * Began filing a misplaced thing this frame. True once, like the telegraph.
	 *
	 * This is the cue hook. Row 16 requires every enemy to announce itself
	 * with sound before sight, and this is the quieter sibling of that: the
	 * sound of a machine starting work nearby, which a player should learn to
	 * recognize as *something of mine is about to move.*
	 */
	bool bStockStarted = false;

	/**
	 * It finished. The pawn moves the thing to the bay on this frame, once.
	 *
	 * The pawn relocates; it never destroys. `ULLKRelicComponent::TakeByServer`
	 * is the player's verb and must not be called from here - a Sweeper
	 * calling it would remove the relic from the world and silently delete the
	 * run's objective.
	 */
	bool bItemShelved = false;

	/**
	 * A keeper was noticed while it was working, so the job lost.
	 *
	 * Reported rather than inferred because this is the moment the mechanic
	 * exists for: the player chose to be seen in order to save the thing, and
	 * a readout, a metric or a playtest note should be able to say so.
	 */
	bool bStockInterrupted = false;
};

namespace LLKSweeperBrain
{
	/**
	 * One step. Pure: no world, no actor, no navmesh, no `MoveTo`.
	 *
	 * `Contact` is folded in rather than given its own function because the
	 * cooldown, the decay and the pursuit share one clock - three functions
	 * advancing the same delta is three chances to advance it twice.
	 */
	LLK_API FLLKSweeperStep Advance(
		const FLLKSweeperMind& Mind,
		const FLLKSweeperSenses& Senses,
		const FLLKEnemyDef& Def,
		const FLLKSweeperContactRules& Contact);

	LLK_API const TCHAR* NameOf(ELLKSweeperState State);
	LLK_API const TCHAR* NameOf(ELLKSweeperMove Move);
}

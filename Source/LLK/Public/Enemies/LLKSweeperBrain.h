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
	LastKnown,

	/**
	 * The misplaced thing it is on its way to deal with.
	 *
	 * **This was left out of the first version and the floor refuted it.** The
	 * argument for omitting it was that a Sweeper should only deal with what
	 * its route already brings it next to, so it never "acquires a new
	 * appetite". Measured on the authored floor: the closest any patrol leg
	 * comes to any authored relic is **6.26 studs**, against a 5-stud gate.
	 * Thirty-five centimetres, and three laps produced nothing.
	 *
	 * Diverting a stride to deal with a box is not an appetite - it is a floor
	 * cleaner doing its job, and `attractedToDroppedLoot` is literally the
	 * word *attracted*. What would break row 47 is diverting toward a keeper,
	 * which it still never does.
	 *
	 * It is also better: a player watching a machine leave its route because
	 * of something they left on the floor is told more than one that happens
	 * to pass close enough.
	 */
	MisplacedItem
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

	/** Counts down during Stock, once it is in reach. The seconds spent filing. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Enemies")
	float StockSecondsRemaining = 0.f;

	/**
	 * Counts down while walking to the thing. Gives up at zero.
	 *
	 * Without this a Sweeper that cannot reach a relic - behind glass, up a
	 * step, across a hole in the navmesh - would walk at it forever and stop
	 * patrolling. That is worse than the behaviour not existing, because the
	 * floor would look fine and one machine would quietly be gone.
	 *
	 * The budget is `losesTargetAfterSeconds`, which already means *how long
	 * before it gives up on something it cannot resolve.* Authored, correct,
	 * and no new number.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Enemies")
	float StockApproachSecondsRemaining = 0.f;

	/**
	 * Counts down while it stands at a patrol point doing something.
	 *
	 * **Why a patrol pauses at all, which is not a cosmetic question.** The
	 * NPU-threat literature separates fear from anxiety by one variable:
	 * whether the threat is *cued*. Predictable, cued threat produces
	 * fear-potentiated startle that spikes and resolves; uncued threat
	 * produces anxiety-potentiated startle that is sustained and does not.
	 * See `docs/FEAR-AND-LIGHT.md`.
	 *
	 * A patrol with no dwell is the uncued condition, continuously - and it
	 * was exactly that until now, because the pawn advanced its route index
	 * the instant it arrived. The Sweeper never once stopped. Chris's report
	 * was *"he patrols too hard"*, which is what that feels like.
	 *
	 * A dwell makes the loop **learnable**: the route gains a rhythm, the gap
	 * between passes can be timed, and crossing it becomes a decision rather
	 * than a dice roll. That is what a telegraph does - convert unpredictable
	 * threat into predictable threat with a cue.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Enemies")
	float PatrolDwellSecondsRemaining = 0.f;

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

	/**
	 * Time until it may touch the keeper again. `contactCooldownSeconds` is 1.5.
	 *
	 * This is also a pursuit break, not merely a hit-rate limiter. A floor
	 * machine that keeps steering into the keeper it just clipped turns a narrow
	 * retail aisle into a body-block, even though the player technically still
	 * has input. While this is positive, Search keeps working the last known
	 * position instead of immediately re-acquiring a visible keeper. The player
	 * gets one honest escape window; the authored cooldown stays the only timer.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Enemies")
	float ContactCooldownRemaining = 0.f;

	/**
	 * The post-contact escape window. While this is positive the Sweeper works
	 * the last place it had the keeper, even if it can still see them.
	 *
	 * Separate from ContactCooldownRemaining on purpose: the latter is the
	 * authored hit-rate limit (1.5s), while this is the existing patrol rhythm
	 * (2.5s). Collapsing them made a visible keeper re-lock the instant the hit
	 * timer expired, which felt like perpetual pursuit rather than a machine
	 * that had just clipped someone and overshot its route.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Enemies")
	float ContactRecoverySecondsRemaining = 0.f;
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
	 * Whether it is standing at its current patrol point this frame.
	 *
	 * Plain `bool` rather than `TOptional`, unlike `bHasPatrolPoint`, and the
	 * difference is honest: "nobody measured" and "not arrived" both mean
	 * *do not start dwelling*, so there is no third state worth carrying.
	 * A route with no points never reports arrival either way.
	 */
	bool bAtPatrolPoint = false;

	/**
	 * Where the nearest thing that is in the wrong place is. Unset when there
	 * is none in reach, or when nobody looked.
	 *
	 * **Two authored ranges, each used for what it means.** The first version
	 * used `ContactRange` for both noticing and reaching, on the argument that
	 * reusing it invented no number. Measuring the floor refuted that: the
	 * closest approach of any patrol leg to any authored relic is 6.26 studs
	 * against a 5-stud gate, so the beat could not fire at all. Contact range
	 * is the reach for *touching a keeper*; noticing that a box is in the lane
	 * is a different sense, and coupling them to avoid declaring a number was
	 * the wrong operand rather than a tidy saving - the error `ADVERSARY.md`
	 * names.
	 *
	 * So: it notices within `aggroRadiusDark`, which already means *what this
	 * machine becomes aware of nearby in the dark*, walks there, and files
	 * within `ContactRange`, which now means physical reach and nothing else.
	 * Still no invented number, and the authored floor works unaltered.
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

	/**
	 * It smacked your feet. Every landed contact below the threshold.
	 *
	 * **Costs distance and tempo, never input.** A brief speed penalty and a
	 * shove off your line - no camera lock, no input suppression, no
	 * animation to wait out. A control-removal the player cannot act through
	 * is a knockdown with a shorter timer, and that is the thing that makes
	 * people stop playing.
	 *
	 * Superseded by `bKnockedDown` when the enemy is a containment type: you
	 * do not stumble and go down from the same hit.
	 */
	bool bStumbleLanded = false;

	/**
	 * The threshold hit, on a machine that does not contain people: **it
	 * reports you.**
	 *
	 * `hitsToKnockDown` still names the count, because it is still the count
	 * that matters - what changes is what happens when you reach it.
	 * `knockdownOnContact` is now the switch between the two kinds of enemy:
	 * true for a containment type that puts a keeper down, false for a
	 * maintenance machine that files a fault and widens what the floor
	 * notices. Chris's authored key, finally meaning two things instead of
	 * one.
	 *
	 * Row 47 is the whole character of this: the answer to being bumped three
	 * times is not to punish the keeper, it is to raise what the building is
	 * paying attention to. Nothing is hurt, frightened or retaliated against.
	 */
	bool bFloorAlerted = false;

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

	/**
	 * Gave up walking to it. Reported so an unreachable relic is visible as a
	 * map problem rather than as a Sweeper that mysteriously stopped filing.
	 */
	bool bStockAbandoned = false;

	/**
	 * It reached a patrol point and has stopped there. True once, like the
	 * telegraph.
	 *
	 * The hook the route's rhythm hangs on. A dwell the player cannot
	 * perceive teaches nothing, and the whole value of the pause is that the
	 * pattern becomes learnable - so this is where a directional work sound
	 * belongs.
	 */
	bool bDwellStarted = false;

	/**
	 * The dwell finished: take the next point.
	 *
	 * **This moves a decision out of the pawn, where it did not belong.**
	 * `ALLKSweeper` advanced `PatrolIndex` itself on arrival, and its own
	 * comment admitted it: *"The only thing this class decides about a route
	 * is 'that one is done'."* That one thing was still a decision, and it is
	 * the reason a dwell was impossible to express - the route moved on
	 * before the brain was ever consulted.
	 */
	bool bAdvancePatrolPoint = false;
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

// "Would this enemy notice this keeper right now." M2.4.
//
// **One function, because six copies of a distance check is the failure mode
// this file exists to prevent.** `docs/ROADMAP.md` M2.4 says it outright: the
// question has to be one function reading the light field, not one per family.
// Six copies is how the archived Roblox build ended up retargeting to the
// nearest player in one place and the brightest in another, and it is how four
// of six families would quietly stop caring about light.
//
// **The six families do not share a sense, and that is the whole design
// problem.** Reading `data/enemies.json`:
//
//   - **Sweepers** carry `aggroRadiusDark` 12 and `aggroRadiusLit` 38 and no
//     sight fields at all. They notice by proximity, and how far that reaches
//     depends on light.
//   - **Stockers** carry `sightRange` 70 and `sightConeDegrees` 70 and no
//     aggro radii. They notice by looking, and only where they are facing.
//   - **Glimmers** carry `scanRange` 30 and `scanRequiresActiveLantern`. They
//     notice only what is lit.
//   - **WindowWalkers** and **Ficus** carry `wakeRange` and
//     `wakeRequiresActiveLantern`, plus a `sightRange` with NO cone. They are
//     furniture until light touches them.
//   - **The Warden** carries none of these. Row 7 and row 54: it converges on
//     the brightest position when its own meter fills, which is not
//     perception and must never be given a radius to make it look like one.
//
// So the answer is not a radius comparison with per-family numbers. It is:
// **which of this family's senses reaches this keeper**, evaluated against
// whichever senses the family actually has.
//
// **Absence is authority here, not a missing value.** `FLLKEnemyDef` already
// made that expressible with `TOptional`, and this file must not undo it: a
// family without `AggroRadiusLit` is not a family with a zero one, so nothing
// below calls `.Get(0.f)` on a sense to bring it into existence. A Stocker
// with an accidental aggro radius of zero would notice nobody, which looks
// like a broken enemy; a Stocker whose absent radius got defaulted to 38 would
// notice everybody, which looks like the game cheating.
//
// **Light is read from the field, never from the keeper's own switch.** The
// gate is brightness at the keeper's position, which `LLKLightField` already
// computes from every lit lantern in the world. That distinction is the whole
// of co-op: a keeper with their own lantern off, standing inside a teammate's
// cone, IS lit and must be noticed as lit. Keying any of this off
// "is my own lantern on" would make the crew's light stop being shared, and
// `docs/THE-STORE.md`'s lesson about the brightest keeper being the one who
// gets found would become unteachable.
//
// **What `SenseReaching` deliberately does NOT do: occlusion.** It takes no
// `UWorld` and traces nothing, because geometry, navmesh and line of sight are
// M2.5 and belong to Chris and Codex. So the answer is "this sense reaches
// that keeper", which is necessary and not sufficient. Naming it
// `SenseReaching` rather than `WouldNotice` is that honesty in the signature:
// a function called `WouldNotice` that ignores walls is a function somebody
// will trust through a wall.
//
// **`Notice` is the sufficient half, added 2026-10-29 as M2.5 support.** It
// takes an `ILLKVisibilitySource` (`Enemies/LLKVisibility.h`) and returns both
// halves of the answer, so the wall arrives through an interface rather than
// through perception learning about `UWorld`. The two stay separable: the
// sense rules are testable with no level, the visibility rules are testable
// with no enemy data, and `docs/MAP-READINESS.md` holds the contract the real
// map has to satisfy for the trace to mean anything.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/LLKVisibility.h"
#include "LLKPerception.generated.h"

struct FLLKEnemyDef;

/**
 * Which sense reached the keeper, never a bare bool.
 *
 * **The reason is row 5 and it is not cosmetic.** "Why did it notice me" is
 * the thing the player has to be able to answer, and a readout, a telegraph
 * or a bug report cannot say it if the function returns true. It also makes
 * the tests able to distinguish a right answer from a lucky one: a Sweeper
 * that noticed by `SightCone` would be a pass under a bool and is a failure
 * here.
 */
UENUM(BlueprintType)
enum class ELLKNoticeSense : uint8
{
	/** No sense of this family's reaches this keeper. */
	None,

	/**
	 * Sweepers. Within a radius that is `aggroRadiusLit` when the keeper is
	 * lit and `aggroRadiusDark` when they are not - 38 against 12, which is
	 * the single largest consequence of the lantern in the data.
	 */
	LitProximity,

	/**
	 * Stockers. Inside `sightRange` AND inside `sightConeDegrees` of where
	 * the enemy is facing. Light does not enter into it, which is what makes
	 * them the family darkness does not solve.
	 */
	SightCone,

	/** Glimmers. Inside `scanRange`, and lit if `scanRequiresActiveLantern`. */
	Scan,

	/**
	 * WindowWalkers and Ficus. Inside `wakeRange`, and lit if
	 * `wakeRequiresActiveLantern`. Furniture until the light arrives.
	 */
	Wake,

	/**
	 * The Warden. Not perception at all.
	 *
	 * Row 7 makes it unkillable and row 54 gives it its own arrival meter, so
	 * it converges on the brightest position rather than noticing anybody.
	 * Returned instead of `None` so a caller cannot read "the Warden does not
	 * see you" as "the Warden is not coming".
	 *
	 * **Only for the Warden**, which is named rather than inferred from
	 * having no senses. A malformed row - a Sweeper that lost one of its two
	 * radii - also has no sense, and reporting that as `StormAuthority` would
	 * make broken data behave like the one enemy that cannot be escaped.
	 * Broken data degrades to `None`.
	 */
	StormAuthority
};

/**
 * The whole answer: which sense reached, what the wall said, and the verdict.
 *
 * **Three fields and not one bool, for the same reason `ELLKNoticeSense`
 * exists.** "Why did it notice me" is the question the player has to be able
 * to answer (row 5), and "why did it NOT notice me" is the question the person
 * tuning the map has to be able to answer. A bool answers neither. With this,
 * a readout can say *a Stocker was looking at you and the shelving stopped
 * it*, which is a sentence about the floor plan rather than about the code.
 */
USTRUCT(BlueprintType)
struct FLLKNotice
{
	GENERATED_BODY()

	/**
	 * The sense that reached, whether or not it fired.
	 *
	 * Deliberately filled in even when a wall stopped the notice. The sense
	 * is a fact about the enemy and the distance; the wall is a separate
	 * fact, and collapsing them would throw away the half that explains the
	 * outcome.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Perception")
	ELLKNoticeSense Sense = ELLKNoticeSense::None;

	/**
	 * What the visibility source said, or `Unknown` when it was not asked.
	 *
	 * Not asked happens in three cases and all three are honest: no sense
	 * reached, so there was nothing to occlude; the Warden, which walls do
	 * not stop; or no source was bound, which is an error and logs one.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Perception")
	ELLKVisibility Visibility = ELLKVisibility::Unknown;

	/** The verdict. `Clear` plus a reaching sense, or the Warden. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Perception")
	bool bNoticed = false;

	/**
	 * A sense reached and nobody could say whether a wall was in the way.
	 *
	 * **This is the state a map problem looks like**, and it is why `Unknown`
	 * is a value rather than a silent `Blocked`. An enemy that is not
	 * noticing because the floor has no collision reads exactly like an enemy
	 * that is not noticing because it is behind a shelf, and the difference
	 * is a bug in one case and the design working in the other. The debug
	 * readout should say *unverified*, never *safe*.
	 */
	bool IsUnverified() const
	{
		return Sense != ELLKNoticeSense::None && Visibility == ELLKVisibility::Unknown
			&& Sense != ELLKNoticeSense::StormAuthority;
	}
};

/** An enemy, reduced to what a sense needs. No actor, so this is testable. */
USTRUCT(BlueprintType)
struct FLLKPerceiver
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "LLK|Perception")
	FVector Position = FVector::ZeroVector;

	/** Unit vector. Only `SightCone` reads it; the other senses are omnidirectional. */
	UPROPERTY(BlueprintReadWrite, Category = "LLK|Perception")
	FVector Facing = FVector::ForwardVector;
};

/** A keeper, reduced to what a sense needs. */
USTRUCT(BlueprintType)
struct FLLKPerceivedKeeper
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "LLK|Perception")
	FVector Position = FVector::ZeroVector;

	/**
	 * How marked this keeper is, 0 to 1, from `LLKLightField::DetectabilityAt`.
	 *
	 * **Not "my lantern is on", and no longer "how bright is it here".** A
	 * keeper standing in a teammate's cone with their own lantern off is
	 * marked, because that is what makes light a shared cost and what makes
	 * the brightest-keeper lesson teachable. Feeding this from a bool is the
	 * one mistake that would quietly uncouple the whole enemy roster from the
	 * crew's light.
	 *
	 * **It was `Brightness` in candelas until row 80.** The reason it is not
	 * any more is that brightness could not express a hooded lantern: the
	 * threshold it was compared against is derived from intensity, intensity
	 * cancels, and a keeper's own lantern sits at distance zero - so every lit
	 * keeper cleared the bar at any radius and any brightness. Detectability is
	 * authored per level instead, which is what lets dim mean less attention.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "LLK|Perception")
	float Detectability = 0.f;
};

namespace LLKPerception
{
	/**
	 * How far this family notices a keeper at this detectability, in studs.
	 *
	 * **The one function light passes through**, and the reason a hooded
	 * lantern means anything. A Sweeper authors two radii - `aggroRadiusDark`
	 * 12 and `aggroRadiusLit` 38 - and until row 80 the only question asked of
	 * light was which of the two to use. Now those two are the ends of a range
	 * and detectability says where on it the keeper sits:
	 *
	 * ```
	 * Radius = Lerp(AggroRadiusDark, AggroRadiusLit, Detectability)
	 * ```
	 *
	 * **The ends are unchanged**, which is what makes this safe to land before
	 * the data exists: at 0 a crew is noticed from 12 as it always was, and at
	 * 1 from 38 as it always was. A single level authored at detectability 1 is
	 * bit-for-bit the behaviour this project shipped with.
	 *
	 * Chris's sentence, 2026-10-30: *"even the slightest light charges an enemy
	 * and they move, but the brighter the more powerful"*. This is that, and it
	 * is deliberately the only place it is true - he was equally clear that the
	 * clock stays out of it (rows 72 and 73).
	 */
	LLK_API float AggroRadiusAt(float DarkStuds, float LitStuds, float Detectability);

	/**
	 * Which of this family's senses reaches this keeper, or None.
	 *
	 * **The one function.** Every family's noticing goes through here, so a
	 * change to how light gates perception is one edit rather than six, and
	 * "does this family care about light" is answerable by reading one place.
	 *
	 * Occlusion is NOT considered - see the file header. The caller applies a
	 * visibility trace once M2.5 has geometry to trace.
	 *
	 * When several senses reach at once the earliest in `ELLKNoticeSense`
	 * order wins, which is stable rather than meaningful: no family in the
	 * data has two senses that can both fire, and a test asserts that, so if
	 * one ever does the precedence becomes a design decision somebody has to
	 * make rather than one this function already made quietly.
	 */
	LLK_API ELLKNoticeSense SenseReaching(
		const FLLKEnemyDef& Enemy,
		const FLLKPerceiver& Perceiver,
		const FLLKPerceivedKeeper& Keeper);

	/**
	 * Does this family have a sense that could reach anybody at all, with no
	 * keeper and no distance involved?
	 *
	 * **A data question, not a perception one**, and it exists because of a
	 * real failure: on 2026-10-31 every `TOptional` field in `DT_Enemies`
	 * arrived unset, so the Sweeper noticed nobody for a day while the table
	 * loaded six families without complaint. `SenseReaching` already degrades
	 * correctly in that case - it returns `None`, and the comment beside it
	 * says a family whose senses failed to import is a family that does not
	 * act - but nothing announced it, and "the enemies did nothing" looks
	 * exactly like "the enemies have no radii".
	 *
	 * The Warden answers false here and that is correct: its senselessness is
	 * authored (row 7). A caller auditing imported data has to name it,
	 * exactly as `SenseReaching` does.
	 */
	LLK_API bool HasReachingSense(const FLLKEnemyDef& Enemy);

	/**
	 * Does this enemy notice this keeper right now, walls included.
	 *
	 * **The one place the two halves meet.** `SenseReaching` decides whether
	 * a sense arrives; `Visibility` decides whether anything is in the way;
	 * this returns both and the verdict, and nothing else in the project is
	 * allowed to combine them itself.
	 *
	 * `Visibility` is NOT optional. Passing `nullptr` logs an error and
	 * notices nothing - there is deliberately no "assume clear" path, because
	 * an assume-clear path is the one that ships. The same rule
	 * `ULLKEnemyRosterSubsystem` follows about fallback rosters: a missing
	 * input degrades to inert, never to invented.
	 *
	 * **Positions are used as given, in centimetres.** This does not raise
	 * either endpoint to eye height, because a Glimmer's scan origin is not a
	 * Stocker's eye and a helpful guess here would hide that difference. The
	 * caller passes the sense's actual origin and the point on the keeper
	 * that counts as hittable. `docs/MAP-READINESS.md` records the keeper's
	 * eye at 152cm, which is the number the map's sight breaks beat.
	 *
	 * **Walls do not stop the Warden.** A `StormAuthority` result notices
	 * without consulting the source at all: row 7 and row 54 make its arrival
	 * storm-driven rather than perceived, and letting a shelf break it would
	 * turn the one inescapable thing in the game into a hiding puzzle.
	 */
	LLK_API FLLKNotice Notice(
		const FLLKEnemyDef& Enemy,
		const FLLKPerceiver& Perceiver,
		const FLLKPerceivedKeeper& Keeper,
		const ILLKVisibilitySource* Visibility);

	/**
	 * How far this family's noticing reaches a keeper in this light.
	 *
	 * Exists for the debug readout and for M2.5's spawn placement, which both
	 * need the number rather than the yes/no - `LLKStoreFloorPlan`'s
	 * sightline rule is written against `aggroRadiusLit` and should be able
	 * to ask for it instead of restating 38. Returns 0 for a family with no
	 * reaching sense, including the Warden.
	 *
	 * In STUDS, like everything in `data/`. Row 61: the conversion happens
	 * where the value is used, not here.
	 */
	LLK_API float NoticeRangeStuds(const FLLKEnemyDef& Enemy, float Detectability);
}

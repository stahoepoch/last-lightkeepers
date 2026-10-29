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
// **What this function deliberately does NOT do: occlusion.** It takes no
// `UWorld` and traces nothing, because geometry, navmesh and line of sight are
// M2.5 and belong to Chris and Codex. So the answer is "this sense reaches
// that keeper", which is necessary and not sufficient - the caller applies a
// visibility trace once there is a floor to trace against. Naming it
// `SenseReaching` rather than `WouldNotice` is that honesty in the signature:
// a function called `WouldNotice` that ignores walls is a function somebody
// will trust through a wall.

#pragma once

#include "CoreMinimal.h"
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
	 */
	StormAuthority
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
	 * Light at this keeper's position, from `LLKLightField::BrightnessAt`.
	 *
	 * **Not "my lantern is on".** A keeper standing in a teammate's cone with
	 * their own lantern off has a high brightness here and must be treated as
	 * lit, because that is what makes light a shared cost and what makes the
	 * brightest-keeper lesson teachable. Feeding this from a bool is the one
	 * mistake that would quietly uncouple the whole enemy roster from the
	 * crew's light.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "LLK|Perception")
	float Brightness = 0.f;
};

namespace LLKPerception
{
	/**
	 * The brightness above which a keeper counts as lit.
	 *
	 * **Derived from the lantern, and deliberately the same derivation the
	 * relic uses.** `ULLKRelicComponent` sets its identify threshold to half
	 * the lantern's intensity - one lantern at the edge of its own reach,
	 * since `BrightnessAt` falls to `I/2` at distance `r`. "Readable by
	 * lantern light" and "noticeable as lit" are not obviously the same
	 * question, but having one definition Chris can split later is better
	 * than two constants that drift apart silently. If a playtest wants them
	 * different, the difference should be authored in `data/` rather than
	 * appearing here.
	 */
	LLK_API float LitThresholdFrom(float LanternIntensity);

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
		const FLLKPerceivedKeeper& Keeper,
		float LitThreshold);

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
	LLK_API float NoticeRangeStuds(const FLLKEnemyDef& Enemy, float Brightness, float LitThreshold);
}

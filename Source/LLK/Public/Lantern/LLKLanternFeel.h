// How the lantern moves, and the one thing that must never move with it.
//
// Chris, 2026-10-29, having found the constraint before anybody made it feel
// good and behave wrong: *"The current ULLKLanternLightComponent is both the
// visible spotlight and the source used for storm/relic/enemy light-field
// positions. So a cosmetic sway cannot simply move that component locally: it
// would make the player see one light position while the server judges
// another."*
//
// He is exactly right, and the coupling is one line:
// `ULLKZonePressureSubsystem::GetLitLightSources` reads
// `Light->GetComponentLocation()`. That position feeds `LLKLightField`, which
// feeds the storm's accrual, `FindBrightestPosition` - which is what the
// Warden converges on - and relic legibility. Sway the component and all three
// start judging a position the keeper never occupied.
//
// **So the split is: one honest anchor, and all cosmetic motion downstream of
// it.** The anchor is derived, not stored: the capsule's location plus the
// authored eye height. Nothing cosmetic can reach it, because nothing cosmetic
// is upstream of it - not the sway, and not the landing dip, which is the same
// trap one component along. A dip applied to the camera would move a light
// parented to the camera, so the anchor is taken from the capsule instead and
// the camera is free to move as much as it likes.
//
// **Three guarantees, which are Chris's and are tested in
// `LLK.Lantern.Feel`:**
//
//   1. Camera look is immediate. No filter, no lag, no smoothing on input.
//      The lantern trails the look; the look never trails anything.
//   2. The visible lantern never crosses blocking geometry. The offset is
//      clamped against the free distance from the eye, so the cone can be
//      pressed back by a shelf but cannot be pushed through one.
//   3. Cosmetic sway cannot change storm, relic or enemy logic. Guaranteed by
//      construction rather than by care: the gameplay anchor is a function of
//      the capsule and an authored constant, and takes no sway state as input.
//
// **This file is pure.** Offsets in, offsets out, no `UWorld` and no
// component - the same division `LLKMapReadiness` uses, and for the same
// reason: the judgement is worth testing without a level, and the gathering is
// three lines that cannot be got wrong in an interesting way. The trace that
// produces `FreeDistanceCm` happens in the component; the decision about what
// to do with it happens here.

#pragma once

#include "CoreMinimal.h"
#include "LLKLanternFeel.generated.h"

/**
 * How much the visible lantern is allowed to lag, sway and kick.
 *
 * Authored rather than constant, and **not** in `data/lantern.json`: every
 * number in there is portable and shared with enemy logic, and none of these
 * is. A sway amplitude cannot be read by `tests/balance.test.mjs` against
 * anything, because it has no consequence to be balanced against - which is
 * the test for whether a number belongs in `data/`. These are Chris's to tune
 * by eye in the editor, which is what `EditAnywhere` is for.
 */
USTRUCT(BlueprintType)
struct FLLKLanternFeelRules
{
	GENERATED_BODY()

	/**
	 * How far behind the look the cone trails, as a fraction per second.
	 *
	 * Interpreted as an exponential approach, so it is frame-rate
	 * independent: at 0.85 the lantern closes 85% of the remaining angle each
	 * second. **Zero means no trail at all**, which is the honest way to
	 * switch this off - and 1 is not "instant", it is "all of it in a
	 * second", so values near 1 still read as a held object.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Feel", meta = (ClampMin = "0", ClampMax = "1"))
	float TrailPerSecond = 0.88f;

	/** The most the cone may ever lag the look, in degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Feel", meta = (ClampMin = "0", ClampMax = "45"))
	float MaxTrailDegrees = 9.f;

	/**
	 * Sway amplitude at a walk, in centimetres.
	 *
	 * Small on purpose. The lantern is held, not swung: this is the evidence
	 * of a hand, not a pendulum. Row 64's restraint about pressures that are
	 * only leashes applies to feel too - a sway big enough to notice as an
	 * effect is a sway big enough to fight.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Feel", meta = (ClampMin = "0"))
	float WalkSwayCm = 2.4f;

	/** Sway amplitude at a sprint. Larger, because the arm is working harder. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Feel", meta = (ClampMin = "0"))
	float SprintSwayCm = 5.2f;

	/** Sway cycles per second at a walk. A footfall rhythm, not a wobble. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Feel", meta = (ClampMin = "0.1"))
	float SwayHertz = 1.6f;

	/** How far the lantern drops on the hardest landing, in centimetres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Feel", meta = (ClampMin = "0"))
	float MaxLandingKickCm = 7.f;

	/**
	 * The impact speed that produces the full kick, in cm/s.
	 *
	 * The jump apex is 56cm, so a plain jump lands at roughly 330cm/s. Set
	 * above that, so stepping off a service ledge is felt and a standing jump
	 * is barely anything - the contrast the floor plan's two heights exist to
	 * teach (`LLKStoreFloorPlan.h`).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Feel", meta = (ClampMin = "1"))
	float FullKickImpactSpeed = 600.f;

	/** How quickly a landing kick recovers, as a fraction per second. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Feel", meta = (ClampMin = "0", ClampMax = "1"))
	float KickRecoveryPerSecond = 0.94f;

	/**
	 * The hard ceiling on total cosmetic offset, in centimetres.
	 *
	 * **The one number that is a safety rail rather than a taste.** Sway plus
	 * kick plus trail can compound, and a cone that can be thrown far enough
	 * reintroduces exactly the problem this file exists to prevent: the player
	 * seeing light somewhere the gameplay anchor is not. The anchor never
	 * moves, so this does not create a correctness bug - it creates a visible
	 * lie, which is worse to debug. Clamped, and a test asserts the clamp
	 * holds under absurd inputs.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Feel", meta = (ClampMin = "0", ClampMax = "40"))
	float MaxOffsetCm = 12.f;

	/**
	 * How far short of a wall the cone is held, in centimetres.
	 *
	 * The visible lantern is a light now and a mesh later (Chris: *"the
	 * eventual lantern mesh attaches to that same visual transform"*), so the
	 * clamp leaves room for an object rather than for a point.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LLK|Feel", meta = (ClampMin = "0"))
	float WallClearanceCm = 6.f;
};

/**
 * What the lantern's cosmetic transform is doing right now.
 *
 * Carried between frames and advanced by `AdvanceFeel`. Deliberately a plain
 * struct with no behaviour: it is the filter's memory, and a filter whose
 * memory can be inspected is a filter whose misbehaviour can be asserted.
 */
USTRUCT(BlueprintType)
struct FLLKLanternFeelState
{
	GENERATED_BODY()

	/** Offset from the honest anchor, in the keeper's local space, in cm. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Feel")
	FVector OffsetCm = FVector::ZeroVector;

	/** How far the cone currently lags the look. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Feel")
	FRotator TrailDegrees = FRotator::ZeroRotator;

	/** Current landing kick, in cm, recovering toward zero. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Feel")
	float KickCm = 0.f;

	/** Sway phase in radians, advanced by speed rather than by time alone. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Feel")
	float SwayPhase = 0.f;
};

/** What the filter is told each frame. */
USTRUCT(BlueprintType)
struct FLLKLanternFeelInput
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "LLK|Feel")
	float DeltaSeconds = 0.f;

	/**
	 * Where the keeper is looking, right now, unfiltered.
	 *
	 * **Read, never written.** Guarantee 1: the lantern trails the look and
	 * the look trails nothing. `AdvanceFeel` takes this by const reference and
	 * returns no rotation for the camera, so there is no signature through
	 * which input lag could be introduced.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "LLK|Feel")
	FRotator Look = FRotator::ZeroRotator;

	/** Horizontal speed in cm/s. Drives sway amplitude and phase rate. */
	UPROPERTY(BlueprintReadWrite, Category = "LLK|Feel")
	float SpeedCm = 0.f;

	/** True while sprinting, which widens the sway rather than just speeding it. */
	UPROPERTY(BlueprintReadWrite, Category = "LLK|Feel")
	bool bSprinting = false;

	/**
	 * The keeper's authored walk speed in cm/s - the denominator `SpeedCm` is
	 * read as a fraction of.
	 *
	 * **An input rather than a constant, because it is gameplay data and this
	 * layer is pure.** It was `const float WalkSpeedCm = 448.f` inside
	 * `Advance()` until 2026-11-04, with a comment citing
	 * `data/lantern.json movement.walkSpeed` as its source - true when
	 * written, and false the moment the retune halved the walk to 224. The
	 * sway then ran at half phase rate AND half amplitude for a walking
	 * keeper, which presents as "the lantern stopped feeling like a carried
	 * object" and is not a symptom anyone would trace to a literal.
	 *
	 * Zero or negative means nobody supplied it. The sway then holds at full
	 * fraction rather than silently halving, so a missing wire reads as a
	 * lantern swaying while you stand still - wrong in a way you notice
	 * immediately, which is the point. Compare `ELLKVisibility::Unknown`:
	 * an unanswered question must not default to the plausible answer.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "LLK|Feel")
	float ReferenceWalkSpeedCm = 0.f;

	/**
	 * Downward speed at the instant of landing, in cm/s. Zero on every other
	 * frame.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "LLK|Feel")
	float LandingImpactSpeed = 0.f;

	/**
	 * How far the lantern's desired position can go before it hits something,
	 * measured from the eye, in cm. Negative means nobody traced.
	 *
	 * **Negative rather than zero for "unmeasured", because zero is a real
	 * answer** - a keeper pressed against a shelf genuinely has no room. The
	 * same distinction `ELLKVisibility::Unknown` draws, for the same reason:
	 * an unanswered question that defaults to the permissive answer is how
	 * the cone ends up inside a wall on the one machine where the trace
	 * failed.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "LLK|Feel")
	float FreeDistanceCm = -1.f;
};

namespace LLKLanternFeel
{
	/**
	 * A keeper's eye, 152cm above the floor.
	 *
	 * The capsule's origin sits at its half-height of 88cm and the camera is
	 * authored 64cm above that (`LLKCharacter.cpp`:
	 * `SetRelativeLocation(FVector(0, 0, 64))`). An actor's location for a
	 * character IS that capsule origin, so adding this to
	 * `GetActorLocation()` lands on the eye.
	 *
	 * **The single authority, because it was briefly two.**
	 * `LLKMapReadiness` needed it for its sightline sweep and declared its
	 * own 152; this file needs it for the gameplay light anchor. Two copies
	 * of a number derived from two engine defaults is two copies that stop
	 * agreeing the day a capsule changes, and the one that would be noticed
	 * last is the one that decides what enemies hunt. So it lives here, next
	 * to the thing most likely to break it, and
	 * `tests/map-readiness.test.mjs` asserts that nobody re-declares it.
	 */
	LLK_API extern const float KeeperEyeHeightCm;

	/**
	 * The honest gameplay position of a keeper's light.
	 *
	 * **Guarantee 3, by construction.** A function of the capsule's location
	 * and an authored eye height, and of nothing else - it takes no feel
	 * state, no camera, and no sway, so there is no argument through which a
	 * cosmetic value could reach it. `GetLitLightSources` reads this, so the
	 * storm, the light field, the Warden's convergence and relic legibility
	 * all judge the position the keeper actually occupies.
	 *
	 * Deliberately NOT the camera's location. The camera is where the landing
	 * dip and the view feel live, and a light parented to it would inherit
	 * every one of them - which is the same bug as the sway, one component
	 * along and harder to see.
	 */
	LLK_API FVector GameplayLightPosition(const FVector& CapsuleLocation, float EyeHeightCm);

	/**
	 * Advances the cosmetic filter by one frame.
	 *
	 * Pure: state and input in, new state out. Every output is bounded by
	 * `Rules.MaxOffsetCm`, including under inputs no frame would produce - a
	 * negative delta, a NaN speed, an absurd impact - because a feel filter
	 * that can be made to explode by a hitch is a feel filter that will be.
	 */
	LLK_API FLLKLanternFeelState AdvanceFeel(
		const FLLKLanternFeelState& State,
		const FLLKLanternFeelInput& Input,
		const FLLKLanternFeelRules& Rules);

	/**
	 * The offset the cone may actually take, given what the trace found.
	 *
	 * **Guarantee 2.** With a measured free distance the offset is shortened
	 * to stop `WallClearanceCm` short of whatever is there; with none
	 * measured (`FreeDistanceCm` negative) the offset collapses to zero,
	 * which puts the visible cone exactly on the honest anchor. That is the
	 * conservative direction: an unmeasured frame shows the player the truth
	 * rather than a guess.
	 */
	LLK_API FVector ClampedOffset(
		const FVector& DesiredOffsetCm, float FreeDistanceCm, const FLLKLanternFeelRules& Rules);
}

// "Is there anything between these two points." M2.5's half of noticing.
//
// **Why this is an interface and not a line trace inside perception.**
// `LLKPerception::SenseReaching` takes no `UWorld` and traces nothing, on
// purpose, and the comment at the top of `LLKPerception.h` promises that the
// caller supplies visibility once there is geometry to trace. This is that
// seam. Putting the trace inside perception would have made the one function
// that answers "would this notice me" untestable without a level, and the
// tests that found the two M2.9 bugs would not exist.
//
// So: perception answers *which sense reaches*, this answers *whether anything
// is in the way*, and `LLKPerception::Notice` is the one place they meet.
//
// **The third answer is the reason this file has an enum rather than a bool.**
// A trace can fail to answer: no world, a floor with no collision, a level
// where the geometry has not been built yet. `Clear` would make enemies see
// through unbuilt walls. A silent `Blocked` would make the entire roster inert
// and look like a design problem rather than a missing volume. So `Unknown`
// exists, it never notices, and it is carried out to the caller so a readout
// can say *unverified* instead of *safe*.

#pragma once

#include "CoreMinimal.h"
#include "LLKVisibility.generated.h"

class AActor;
class UWorld;

/** Whether anything stands between two points, or whether nobody could tell. */
UENUM(BlueprintType)
enum class ELLKVisibility : uint8
{
	/**
	 * Nobody could answer.
	 *
	 * A null world, a trace against a level with no collision, a source that
	 * has not been bound yet. **Never treated as `Clear`**: see the file
	 * header. Reported rather than swallowed, because a map that answers
	 * `Unknown` everywhere is a map that is missing requirement 1 of
	 * `docs/MAP-READINESS.md`, and that is worth saying out loud.
	 */
	Unknown,

	/** Nothing in the way. The sense, if one reaches, actually fires. */
	Clear,

	/** Something is in the way. The sense reaches and does not fire. */
	Blocked
};

/**
 * A source of visibility answers.
 *
 * Deliberately one method. Everything that wants to know about walls asks
 * this, so a test can answer with a constant, the real implementation can
 * answer with a trace, and perception does not know which it got.
 *
 * Not a `UINTERFACE`: implementations are plain structs that live for the
 * duration of a query, there is nothing to serialise, and making it reflected
 * would mean an actor has to own one.
 */
class LLK_API ILLKVisibilitySource
{
public:
	virtual ~ILLKVisibilitySource() = default;

	/**
	 * Whether anything stands between the two points, both in CENTIMETRES.
	 *
	 * Centimetres and not studs, unlike most of this project, because both
	 * arguments come from a running world rather than from `data/` - row 61
	 * puts the conversion at the boundary, and the boundary is above this.
	 *
	 * The caller supplies the exact endpoints, including eye height. This
	 * function does not raise anything to eye height on the caller's behalf,
	 * because a Glimmer's scan origin is not a Stocker's eye and guessing
	 * which is which here would hide the difference.
	 */
	virtual ELLKVisibility VisibilityBetween(const FVector& From, const FVector& To) const = 0;
};

/**
 * The real one: a line trace against the world's visibility channel.
 *
 * Cheap to construct, holds a raw world pointer, and is meant to be made on
 * the stack for the duration of a perception pass rather than stored.
 */
struct LLK_API FLLKWorldVisibility final : public ILLKVisibilitySource
{
	explicit FLLKWorldVisibility(const UWorld* InWorld, const AActor* InIgnore = nullptr)
		: World(InWorld)
		, Ignore(InIgnore)
	{
	}

	/**
	 * `ECC_Visibility`, which is the channel sight-blocking geometry is
	 * expected to block - and which a glass counter can be authored NOT to
	 * block while still blocking movement. That distinction is the whole
	 * reason the cosmetics counters can be low and transparent while the
	 * shelving is tall and solid, and it is why this does not trace
	 * `ECC_WorldStatic`.
	 */
	virtual ELLKVisibility VisibilityBetween(const FVector& From, const FVector& To) const override;

private:
	const UWorld* World = nullptr;

	/** Usually the perceiving enemy, so it does not block its own sightline. */
	const AActor* Ignore = nullptr;
};

/**
 * A fixed answer, for tests and for the senseless cases.
 *
 * Exists so a test can assert that perception respects a `Blocked` without
 * standing up a level, and so `LLK.Enemies.Perception` keeps running with no
 * world at all. Constructing one in shipping code is almost certainly a
 * mistake; constructing one with `ELLKVisibility::Clear` certainly is.
 */
struct LLK_API FLLKFixedVisibility final : public ILLKVisibilitySource
{
	explicit FLLKFixedVisibility(ELLKVisibility InAnswer)
		: Answer(InAnswer)
	{
	}

	virtual ELLKVisibility VisibilityBetween(const FVector&, const FVector&) const override
	{
		return Answer;
	}

private:
	ELLKVisibility Answer = ELLKVisibility::Unknown;
};

// Shared vocabulary for the generated data assets.
//
// Roadmap task M0.5, runtime half. `docs/DATA-PIPELINE.md` is the contract and
// this header is its C++ face: every type here exists because a key in
// `data/*.json` needs somewhere typed to land.
//
// Two rules from that page bind every file in this folder:
//
//   Rule 1c - no field here may be named for a JSON key beginning with `_`.
//   Those are analysis and documentation, stripped before conversion. A
//   `USTRUCT` field matching one would let `_dutyProfiles` reach runtime as
//   configuration, which is the exact hazard the rule was written for.
//
//   Rule 2 - an unrecognised non-underscore key is a hard error. So every
//   gameplay key in the eight files has a home here, and nothing here is
//   speculative. If a field below has no key, that is a bug in this header.

#pragma once

#include "CoreMinimal.h"
#include "LLKDataTypes.generated.h"

/** `storm-charge.json` -> `lantern.curve`. Row 12 is LOCKED on `Diminishing`. */
UENUM(BlueprintType)
enum class ELLKLanternCurve : uint8
{
	// Retired, and deliberately still implemented. Row 12 says switching back
	// must never need a code change, which only holds if this stays a real
	// enum entry rather than a comment.
	LinearCapped UMETA(DisplayName = "Linear, capped"),
	Diminishing  UMETA(DisplayName = "Diminishing")
};

/** `loot.json` -> `tiers.*.visibility`. */
UENUM(BlueprintType)
enum class ELLKLootVisibility : uint8
{
	Always UMETA(DisplayName = "Always visible"),
	// Row 53: light gates *legibility*, never existence. A relic renders in
	// darkness and cannot be read. An invisible relic could not tempt anyone,
	// so this must never be implemented as "do not spawn the actor".
	LanternOnly UMETA(DisplayName = "Visible, unreadable without lantern light")
};

/** `loot.json` -> `extraction.*.reliability`. */
UENUM(BlueprintType)
enum class ELLKExtractionReliability : uint8
{
	High  UMETA(DisplayName = "High"),
	Risky UMETA(DisplayName = "Risky")
};

/** `enemies.json` row names. Decides which optional fields on FLLKEnemyDef are set. */
UENUM(BlueprintType)
enum class ELLKEnemyFamily : uint8
{
	Sweepers,
	Stockers,
	Glimmers,
	WindowWalkers,
	Ficus,
	Warden
};

/**
 * A two-element `[low, high]` second range.
 *
 * `run.targetBandSeconds` and `warden.arrivalBandSeconds` are both JSON arrays
 * of exactly two numbers. A `TArray<float>` would import them and lose the
 * "exactly two, in order" guarantee the importer enforces, so they land here
 * instead - the shape is in the type rather than in a comment.
 */
USTRUCT(BlueprintType)
struct FLLKSecondsBand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Band")
	float Low = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Band")
	float High = 0.f;

	bool Contains(float Seconds) const { return Seconds >= Low && Seconds <= High; }
	bool IsOrdered() const { return Low <= High; }
};

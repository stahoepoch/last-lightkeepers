// `enemies.json` -> one DataTable of six rows.
//
// `docs/DATA-PIPELINE.md` settled the shape this file implements: ONE struct,
// not six. The file is heterogeneous - five universal fields, then whatever
// that family needs - and the alternative was six tables, which would make
// "every enemy telegraphs" (row 16) unassertable in one place.
//
// The family-specific fields are TOptional. Verified against the installed
// 5.8.4 rather than assumed: FOptionalProperty exists in CoreUObject and the
// engine ships TOptional<bool> and TOptional<float> as UPROPERTYs, so this
// reflects and serialises. An unset TOptional is the type saying "this family
// does not have this field", which a 0.f default could not distinguish from a
// deliberately zero value - and `data/lantern.json` is full of deliberate
// zeroes, so that distinction is not hypothetical here.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/LLKDataTypes.h"
#include "LLKEnemyDef.generated.h"

USTRUCT(BlueprintType)
struct FLLKEnemyDef : public FTableRowBase
{
	GENERATED_BODY()

	// ---------------------------------------------------------------------
	// Universal. All six rows carry these, and the importer hard-errors if a
	// row is missing one.
	// ---------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	ELLKEnemyFamily Family = ELLKEnemyFamily::Sweepers;

	/** What it used to do for a living. Row 47: it is still trying to do it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FString FormerJob;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FString Behaviour;

	/** Row 17: every enemy has an answer that is not a weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FString Counterplay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float MoveSpeed = 0.f;

	/**
	 * Row 16: every enemy telegraphs before sight through at least two
	 * distinguishable channels, one of which may be sound. This is the sound
	 * channel's lead time, and it is universal because the rule is universal -
	 * a row reaching here with 0 is a design bug, not a quiet enemy.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float AudioTelegraphSeconds = 0.f;

	// ---------------------------------------------------------------------
	// Family-specific. Unset means "not part of this family's contract".
	// ---------------------------------------------------------------------

	/**
	 * Five of six. Absent on the Warden, and that absence is a rule rather
	 * than an omission: row 7 makes it unkillable. Unset here is what makes
	 * "the Warden has no health" expressible instead of "the Warden has zero
	 * health", which any damage routine would happily treat as already dead.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
	TOptional<float> Health;

	/** Stockers, Window Walkers, Ficus. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Senses")
	TOptional<float> SightRange;

	/** Window Walkers, Ficus. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Senses")
	TOptional<float> WakeRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Senses")
	TOptional<bool> bWakeRequiresActiveLantern;

	// Sweepers.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Sweepers")
	TOptional<float> AggroRadiusDark;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Sweepers")
	TOptional<float> AggroRadiusLit;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Sweepers")
	TOptional<float> SwarmCallRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Sweepers")
	TOptional<bool> bAttractedToDroppedLoot;

	/**
	 * How far this family hears a keeper at each gait, in studs. Row 92's
	 * *"the player must be able to hide"*, ratified by Chris 2026-11-14 at
	 * **4 / 8 / 14** for Sweepers.
	 *
	 * **Unset is deaf, and that is the whole safety of landing this early.**
	 * Until `data/enemies.json` carries the keys, every family leaves these
	 * unset and behaves exactly as it does today: notice stays light and line
	 * of sight, as it has been since the handoff. Nothing in the roster
	 * changes until a value is authored for it.
	 *
	 * **Hearing is a notice, never a lock.** It wakes a walk toward where the
	 * keeper *was*; it must not grant pursuit, a target or a swarm call. A
	 * machine arriving at the place you left is what makes a sprint a decision
	 * instead of a death.
	 *
	 * The derivation is in `docs/PROPOSAL-THE-MACHINES-SHOULD-HEAR.md`: the
	 * ratios come from the 2D prototype Chris tuned by feel on 2026-11-12,
	 * expressed against this family's own `aggroRadiusDark` rather than
	 * converted from its pixels, because that world is not to scale. The
	 * sprint radius is deliberately larger than the dark notice radius - **you
	 * can be heard before you are seen**, which is the point of the sense.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Senses")
	TOptional<float> HearingRadiusCrouch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Senses")
	TOptional<float> HearingRadiusWalk;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Senses")
	TOptional<float> HearingRadiusSprint;

	// Stockers.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Stockers")
	TOptional<float> SightConeDegrees;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Stockers")
	TOptional<float> LosesTargetAfterSeconds;

	// Glimmers.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Glimmers")
	TOptional<float> ScanRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Glimmers")
	TOptional<float> ScanSeconds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Glimmers")
	TOptional<float> TagSeconds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Glimmers")
	TOptional<bool> bTagRevealsPositionToAllRobots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Glimmers")
	TOptional<float> TagAddsPressure;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Glimmers")
	TOptional<bool> bTagGivesWardenPermanentWaypoint;

	/** Row 5 reaching into the senses: a dark crew is not scanned. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Glimmers")
	TOptional<bool> bScanRequiresActiveLantern;

	// Warden. Row 7, and row 54 moved its *arrival* into ULLKZonePressureData.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Warden")
	TOptional<bool> bKillable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Warden")
	TOptional<bool> bDropsLoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Warden")
	TOptional<FString> SpawnStage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Warden")
	TOptional<float> RetargetIntervalSeconds;

	/** Row 7 in one call, so no gameplay class has to remember the exception. */
	bool CanBeDamaged() const { return Health.IsSet() && bKillable.Get(true); }
};

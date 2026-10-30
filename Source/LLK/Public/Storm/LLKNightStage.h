// Which stage the night is in. M2.3.
//
// **Derived, never stored, and that is the whole design.** The temptation here
// is an `ELLKNightStage { Calm, Alert, Hunt, Warden }` enum and a member that
// tracks it. That member would be a second copy of something the meter already
// knows, and a second copy is a thing that can disagree - which on a client is
// a stage badge that says Alert while the countdown beside it says patrols
// hunt in 0:00.
//
// So this struct is a *view*, computed from three things that already exist:
// the charge, the Warden's flag, and the authored thresholds. There is no
// setter, no tick, and nothing to replicate. `FLLKZonePressureSnapshot` carries the
// charge already and both machines load the same `ULLKZonePressureData`, so a
// client computes its own stage and cannot drift from the server's by
// construction. That is the same argument the snapshot header makes for not
// replicating the countdown, applied one level up.
//
// **It is an index, not a named enum, so that a third threshold costs no
// code.** `docs/DECISIONS.md` row 12 says switching the curve must never need
// a code change, and row 33 makes `data/` the layer that survives an engine
// change; a stage enum with `Alert` and `Hunt` baked in would make
// `zone-pressure.json` unable to grow a threshold without a C++ edit. The index
// counts how many authored thresholds the charge has passed and `Id` carries
// the authored name of the highest one, so a third threshold appears in the
// readout the moment it appears in the data.
//
// **Row 64 is what this is for.** Line one: Zone Pressure permanently increases
// what the building is AUTHORIZED to do. The stage *is* that authority, and it
// is monotonic for the same reason the meter is - `Index` can only rise
// because `Charge` can only rise, and `IsAtLeast` is the question an enemy
// asks rather than reading the meter and picking its own number. Line three
// then stays enforced for free: nothing here can be reached from a failure,
// because the only inputs are the charge and the Warden's own meter, and
// `ELLKStormSpike` already makes failure unable to move either.

#pragma once

#include "CoreMinimal.h"
#include "LLKNightStage.generated.h"

/**
 * How far into the night the building is authorised to go.
 *
 * Construct it with `ULLKZonePressureSubsystem::StageFrom`, or read it from
 * `GetNightStage()`. There is deliberately no other way to make one that
 * means anything - a default-constructed stage is calm, which is the honest
 * answer for "the rules have not resolved yet".
 */
USTRUCT(BlueprintType)
struct FLLKNightStage
{
	GENERATED_BODY()

	/**
	 * How many authored thresholds the charge has passed. 0 is a calm night.
	 *
	 * An index rather than an enum value so the data can grow. Compare stages
	 * with `IsAtLeast` instead of comparing this against a literal, or the
	 * literal becomes the hardcoded enum this struct exists to avoid.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	uint8 Index = 0;

	/**
	 * How many thresholds exist at all, so a readout can say "1 of 2".
	 *
	 * Carried rather than looked up because the thing reading a stage should
	 * not need the rules asset to render it.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	uint8 Count = 0;

	/**
	 * The authored id of the highest threshold crossed, or None when calm.
	 *
	 * `PatrolsAlert`, `PatrolsHunt`, or whatever `zone-pressure.json` is
	 * holding the day somebody adds a third. This is what the HUD prints and
	 * what a log line should name, because an index alone is not something a
	 * player or a bug report can say out loud.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	FName Id;

	/** Row 7: the Warden is unkillable, and arrives on its own dampened meter. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	bool bWardenConverged = false;

	/** Nothing has escalated yet. Not the same as "the rules failed to load". */
	bool IsCalm() const { return Index == 0 && !bWardenConverged; }

	/**
	 * The question an enemy asks. Never "what is the charge".
	 *
	 * An enemy that reads the meter and compares it against a number of its
	 * own choosing is an enemy whose escalation point lives in its own file,
	 * which is how six families end up with six different ideas of when the
	 * night got worse. This asks the authored thresholds instead.
	 */
	bool IsAtLeast(uint8 Stage) const { return Index >= Stage; }

	/** Every threshold crossed and the Warden here: the night has nothing left. */
	bool IsFinal() const { return bWardenConverged && Index >= Count; }

	/**
	 * True when `Other` is a strictly later stage than this one.
	 *
	 * Used to decide whether a crossing is worth announcing. It treats the
	 * Warden as later than any threshold count, because it is the closing act
	 * rather than a third patrol state.
	 */
	bool IsEarlierThan(const FLLKNightStage& Other) const
	{
		if (bWardenConverged != Other.bWardenConverged)
		{
			return Other.bWardenConverged;
		}
		return Other.Index > Index;
	}

	bool operator==(const FLLKNightStage& Other) const
	{
		return Index == Other.Index
			&& Count == Other.Count
			&& Id == Other.Id
			&& bWardenConverged == Other.bWardenConverged;
	}

	bool operator!=(const FLLKNightStage& Other) const { return !(*this == Other); }
};

// What a client is allowed to know about the storm. M1.7's replication seam.
//
// **One struct, replicated as one unit, deliberately.** Codex's review asked
// for the snapshot; making it a single `USTRUCT` rather than five replicated
// properties is the part that matters. Replicated separately, charge and
// multiplier arrive in different packets, and a client can hold a new charge
// beside an old multiplier - so the HUD would compute a projection from two
// different moments and be confidently wrong for a frame. A player watching
// "2:24" flicker to "0:58" and back learns nothing except not to trust the
// number. One struct cannot tear.
//
// **It carries state, never rules.** `ULLKStormChargeData` is static data
// present on both sides already - a client loads `DA_StormCharge` in
// `Initialize` like the server does - so the baseline rate, the curve and the
// Warden's sensitivity do not belong on the wire. Only what changes does.
// That is also what lets a client compute its own projection rather than
// having the server send a countdown: the arithmetic is identical on both
// sides because it is the same code reading the same asset.
//
// **What is NOT here.** The set of crossed thresholds. A client needs to know
// what is coming next, not the history of what has been, and replicating a
// `TArray<FName>` that only grows would put the whole escalation log on the
// wire to render one line. `NextEscalationId` plus its two charge values is
// the same information at a fixed cost.

#pragma once

#include "CoreMinimal.h"
#include "Data/LLKDataTypes.h"
#include "LLKStormSnapshot.generated.h"

USTRUCT(BlueprintType)
struct FLLKStormSnapshot
{
	GENERATED_BODY()

	/** The thresholds' meter. Row 6: this only ever rises. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	float Charge = 0.f;

	/** The Warden's own dampened meter, row 54. Never derived from the above. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	float WardenCharge = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	float ElapsedSeconds = 0.f;

	/** The rate multiplier in force when this snapshot was taken. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	float Multiplier = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	float WardenMultiplier = 1.f;

	/** Lit lanterns the server counted. A crew is at most four. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	uint8 LitLanternCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	bool bWardenConverged = false;

	/**
	 * False when the night has nothing left to send.
	 *
	 * Distinct from "the next escalation is zero seconds away", which is why
	 * it is a flag rather than a sentinel value in the fields below. A readout
	 * that cannot tell those apart renders a countdown to nothing.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	bool bHasNextEscalation = false;

	/** `PatrolsAlert`, `PatrolsHunt`, or `WardenConverges` once both are crossed. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	FName NextEscalationId;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	float NextTargetCharge = 0.f;

	/**
	 * The meter the next escalation is measured against.
	 *
	 * Carried rather than assumed to be `Charge`, because when the Warden is
	 * next it is measured against `WardenCharge` - and those two differ by the
	 * whole of row 54's dampening. A client that assumed the wrong one would
	 * be wrong in the direction that makes a lit crew look safer than it is.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	float NextFromCharge = 0.f;

	/** Which data the SERVER read. A client showing its own source would be guessing. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	ELLKDataSource RulesSource = ELLKDataSource::Unresolved;

	/**
	 * True once the server has published at least once.
	 *
	 * A default-constructed snapshot and a genuine all-zero start of night are
	 * identical in every field above, and they mean opposite things: "no data
	 * has arrived" versus "the night has just begun". Without this a client's
	 * first frames render a real-looking meter made of nothing.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Storm")
	bool bPublished = false;
};

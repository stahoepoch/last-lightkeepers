// What every machine may know about the extraction. M3.
//
// The same reasoning as `FLLKZonePressureSnapshot`, and the same shape for the same
// reason: one struct cannot tear. Phase and hold replicated as separate
// properties would let a client hold `Secured` beside a hold of nine seconds
// for a frame, and the one moment a readout must not be confidently wrong is
// the moment the run ends.
//
// **State, never rules.** The siren length, the hold and the payout all come
// from data assets both sides already load. What goes on the wire is where the
// extraction has got to, who it is for, and how much of the hold is left.
//
// **`HoldSecondsRemaining` is sent rather than a countdown computed from a
// start time**, which is the one place this differs from the storm. The storm
// advances at an authored rate, so a client can project it exactly; a hold
// pauses whenever the boarding route empties, and only the server knows that.
// A client projecting the hold would show it draining through a stall.

#pragma once

#include "CoreMinimal.h"
#include "Run/LLKExtraction.h"
#include "LLKExtractionSnapshot.generated.h"

USTRUCT(BlueprintType)
struct FLLKExtractionSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	ELLKExtractionPhase Phase = ELLKExtractionPhase::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	FName PointId;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	float SirenSecondsRemaining = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	float HoldSecondsRemaining = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	float HoldSecondsRequired = 0.f;

	/**
	 * True when the hold is running and not advancing.
	 *
	 * The single most useful thing a readout can say during an extraction, and
	 * it cannot be derived on a client: a stall and a pause between packets
	 * look identical from there.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	bool bHoldStalled = false;

	/** False until the server has spoken once. Distinguishes "idle" from "unknown". */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	bool bPublished = false;
};

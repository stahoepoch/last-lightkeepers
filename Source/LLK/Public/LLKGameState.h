// The one thing every machine agrees on. M1.7's replication seam.
//
// The storm's meters live in `ULLKZonePressureSubsystem`, which accrues on the
// server alone - that is the authority rule and it does not bend. But a world
// subsystem is not a replicated actor, so a client's copy holds an honest
// zero and the HUD had nothing to draw. This is where the server publishes
// what clients are allowed to know.
//
// **The GameState is the right home and not merely a convenient one.** It
// already exists on every machine, it is replicated to everyone rather than
// per-player, and the storm is a property of the night rather than of any one
// keeper - a crew argues about one meter, not four. A PlayerState copy would
// be four copies of one number, and a custom replicated actor would be a
// GameState with extra steps.
//
// **It stores and does not decide.** Nothing here computes a rate, crosses a
// threshold or counts a lantern. The subsystem does all of that on authority
// and pushes the result; this class owns one struct and a `Replicated`
// keyword. That is deliberate: the moment a GameState starts deriving storm
// values, there are two places that know how the night escalates.
//
// **Any real GameMode must use this class or a subclass of it.** The harness
// sets `GameStateClass` itself so Play needs no setup. If a later GameMode
// forgets, the subsystem logs it once and the HUD says so rather than drawing
// a meter made of nothing - a silent failure here would look exactly like a
// storm that never advances.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Run/LLKExtractionSnapshot.h"
#include "Storm/LLKZonePressureSnapshot.h"
#include "LLKGameState.generated.h"

UCLASS()
class LLK_API ALLKGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ALLKGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** What every machine may read. Default-constructed and `bPublished` false until the server speaks. */
	UFUNCTION(BlueprintPure, Category = "LLK|Storm")
	const FLLKZonePressureSnapshot& GetStormSnapshot() const { return StormSnapshot; }

	/**
	 * Server only. Called by `ULLKZonePressureSubsystem`, which owns the truth.
	 *
	 * Public because the subsystem is not a friend and should not be - but
	 * authority-checked inside, so a client calling it changes nothing. The
	 * storm is the one piece of state everything else reads; if any of it can
	 * be set locally, all of it can be lied about.
	 */
	void PublishStormSnapshot(const FLLKZonePressureSnapshot& InSnapshot);

	/**
	 * Server only. Called once by `ULLKRunStateSubsystem`. M2.2b.
	 *
	 * Replicated rather than recomputed per client, because the wipe must be
	 * ONE event: a client deciding for itself that the crew is finished could
	 * disagree with the server about whether the run is over, which is the
	 * worst possible thing to be wrong about.
	 */
	void PublishCrewWiped();

	UFUNCTION(BlueprintPure, Category = "LLK|Run")
	bool HasCrewWiped() const { return bCrewWiped; }

	/**
	 * The haul got out. The success terminus, and the mirror of
	 * `PublishCrewWiped`.
	 *
	 * **This existed only as a loss before 2026-11-03.** A wipe set run state,
	 * logged, resolved the haul and replicated; securing an extraction logged
	 * and broadcast to telemetry and changed no run state at all. So
	 * `bRunIsOver` in the extraction subsystem was `HasCrewWiped()` alone -
	 * you could lose a run and you could not win one. Row 8 makes extraction
	 * the only thing that secures loot, which makes it the success terminus,
	 * and a terminus the world does not know about is not one.
	 */
	void PublishRunSecured();

	UFUNCTION(BlueprintPure, Category = "LLK|Run")
	bool HasRunSecured() const { return bRunSecured; }

	/** What every machine may read about the extraction. M3. */
	UFUNCTION(BlueprintPure, Category = "LLK|Extraction")
	const FLLKExtractionSnapshot& GetExtractionSnapshot() const { return ExtractionSnapshot; }

	/**
	 * Server only. Called by `ULLKExtractionSubsystem`, which owns the truth.
	 *
	 * A separate struct from the storm's rather than fields added to it,
	 * because they change at different rates and for different reasons: the
	 * storm publishes ten times a second all night, and this publishes on a
	 * transition and then only while an extraction is underway. Merging them
	 * would put the whole extraction on the wire every tenth of a second of
	 * every run that never calls one.
	 */
	void PublishExtractionSnapshot(const FLLKExtractionSnapshot& InSnapshot);

protected:
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "LLK|Storm")
	FLLKZonePressureSnapshot StormSnapshot;

	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "LLK|Run")
	bool bCrewWiped = false;

	UPROPERTY(Replicated)
	bool bRunSecured = false;

	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "LLK|Extraction")
	FLLKExtractionSnapshot ExtractionSnapshot;
};

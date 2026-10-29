// The one thing every machine agrees on. M1.7's replication seam.
//
// The storm's meters live in `ULLKStormChargeSubsystem`, which accrues on the
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
#include "Storm/LLKStormSnapshot.h"
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
	const FLLKStormSnapshot& GetStormSnapshot() const { return StormSnapshot; }

	/**
	 * Server only. Called by `ULLKStormChargeSubsystem`, which owns the truth.
	 *
	 * Public because the subsystem is not a friend and should not be - but
	 * authority-checked inside, so a client calling it changes nothing. The
	 * storm is the one piece of state everything else reads; if any of it can
	 * be set locally, all of it can be lied about.
	 */
	void PublishStormSnapshot(const FLLKStormSnapshot& InSnapshot);

protected:
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "LLK|Storm")
	FLLKStormSnapshot StormSnapshot;
};

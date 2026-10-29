// Storm Charge, on screen. M1.7.
//
// **This is not the HUD.** The real one is M3 and Chris's, and it will be a
// widget with art. This is a debug readout drawn with `DrawText`: no UMG, no
// widget Blueprint, no `Content/` of any kind, deleted the day the real one
// lands.
//
// **Why it is worth building before the real one.** Chris, 2026-10-29: the
// HUD "turns the lantern from 'a cool light' into the game's actual risk
// decision." Right now a player can hold F and nothing visibly answers. The
// meter is rising 2.5x faster, which is the entire premise of the game, and
// it is invisible.
//
// **So the primary number here is TIME, not charge.** A meter climbing from
// 336 to 480 teaches nothing - it is two numbers with no stakes. "Patrols
// hunt in 2:24" teaches, and watching that figure jump to 0:58 the instant
// the lantern comes on teaches the whole of row 5 in one frame, without a
// line of tutorial text. Row 43 makes the store teach by being legible; this
// is the same principle applied to a number instead of a room.
//
// That is also why the lit-lantern count and the multiplier are shown beside
// it rather than instead of it: the projection is the consequence, and those
// two are the cause. A player who sees only the consequence learns to fear
// the meter. A player who sees both learns what they did.
//
// **It reads the replicated snapshot, and only that - on the server too.**
// The subsystem accrues on authority alone, so a client's own copy holds an
// honest zero; `ALLKGameState` is where the server publishes what clients may
// know. The HUD could read the subsystem directly where it happens to be
// authoritative, and deliberately does not: two code paths would mean the
// server sees a readout no client ever sees, and the first thing M1.9's
// two-machine test needs is for both screens to be showing the same thing for
// the same reason. One path also means a snapshot bug is visible on the
// machine running the editor rather than only on the other one.
//
// **The projection is computed locally on both sides, not sent.** The rules
// are static data - a client loads `DA_StormCharge` like the server does - so
// only the meters travel, and both machines run the same arithmetic on the
// same asset. `LLK.Storm.Snapshot` asserts that those two agree.
//
// **It refuses to show a number it does not have.** Before the server's first
// publish the snapshot is default-constructed, which is indistinguishable
// field-by-field from a genuine start of night; `bPublished` is what tells
// them apart, and until it is set the HUD says so rather than drawing a meter
// made of nothing.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Storm/LLKStormSnapshot.h"
#include "LLKDebugHUD.generated.h"

class ULLKStormChargeSubsystem;

UCLASS()
class LLK_API ALLKDebugHUD : public AHUD
{
	GENERATED_BODY()

public:
	ALLKDebugHUD();

	virtual void DrawHUD() override;

	/** mm:ss, or a dash when there is nothing to count down to. */
	static FString FormatSeconds(float Seconds);

protected:
	/** One line, left-aligned, advancing a cursor. Keeps DrawHUD readable. */
	void DrawLine(const FString& Text, const FLinearColor& Colour, float& CursorY, float Scale = 1.f);

	/**
	 * Resolution-scaled text size.
	 *
	 * **Chris, 2026-10-29, from a playtest: "the UI and stuff shows up on the
	 * top left corner, very hard to read."** The engine's medium font at scale
	 * 1 is sized for a 1080p viewport, so on anything larger the readout
	 * shrinks in proportion to the monitor - which is the opposite of what a
	 * bigger monitor should do. Everything now scales off viewport height, so
	 * the panel occupies the same fraction of the screen at any resolution.
	 */
	float GetUIScale() const;

	/** A dark panel behind the text, because nothing is legible over both a dark floor and a lit pillar. */
	void DrawBacking(float Height) const;

	void DrawStormPanel(const ULLKStormChargeSubsystem* Storm, const FLLKStormSnapshot& Snapshot, float& CursorY);

	/** In 1080p-equivalent pixels; multiplied by GetUIScale at draw time. */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Debug")
	FVector2D PanelOrigin = FVector2D(32.f, 28.f);

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Debug")
	float LineHeight = 26.f;

	/**
	 * Base text scale, on top of the resolution scale.
	 *
	 * 1.35 rather than 1 because the engine's medium font is small for a
	 * readout somebody is meant to glance at while being chased.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Debug")
	float TextScale = 1.35f;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Debug")
	FLinearColor BackingColour = FLinearColor(0.02f, 0.025f, 0.035f, 0.72f);
};

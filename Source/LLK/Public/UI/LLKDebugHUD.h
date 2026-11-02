// Zone Pressure, on screen. M1.7.
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
// are static data - a client loads `DA_ZonePressure` like the server does - so
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
#include "Storm/LLKZonePressureSnapshot.h"
#include "UObject/ObjectKey.h"
#include "LLKDebugHUD.generated.h"

class ULLKZonePressureSubsystem;

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

	void DrawStormPanel(const ULLKZonePressureSubsystem* Storm, const FLLKZonePressureSnapshot& Snapshot, float& CursorY);

	/**
	 * The nearest relic, and what the keeper can currently do about it.
	 *
	 * **This is here so row 53 can be LOOKED at.** The relic's whole
	 * behaviour - visible but unreadable, legible once lit, affordance
	 * closing when the light leaves - was asserted in tests and invisible in
	 * play, which is a mechanic nobody can judge. Walking up to a shelf and
	 * watching "Something on the shelf" become "Engraved pocket watch", then
	 * watching the prompt turn into "needs light" when the lantern goes off,
	 * is the entire lesson of the ground floor in one line of text.
	 *
	 * It is a debug readout and it is not the prompt. The real one is Chris's
	 * and will be art on the object itself; this is the scaffold that proves
	 * there is something there to make art out of.
	 */
	void DrawRelicPanel(float& CursorY);

	/**
	 * The haul and what it costs to carry, as one line. M3's burden.
	 *
	 * **Row 5's shape applied to weight: the cause beside the consequence.**
	 * The storm panel shows what light does by printing the time it costs, and
	 * burden needs the same treatment or a player feels sluggish and blames
	 * the controls. `carrying 2 - 8% slower` is diagnosable; a keeper who is
	 * quietly 8% slower is not.
	 *
	 * It also says when the next relic is free, because the authored cap is
	 * part of the decision: at 0.04 to a cap of 0.12 the fourth relic costs
	 * nothing, and a crew that knows that pushes their luck on purpose rather
	 * than by accident.
	 *
	 * Drawn only while carrying something. An empty-handed keeper has nothing
	 * to be told.
	 */
	void DrawBurdenLine(float& CursorY);

	/** How far the readout looks for a relic, in STUDS. Converted at the boundary, row 61. */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Debug", meta = (ClampMin = "0"))
	float RelicSearchStuds = 12.f;

	/**
	 * The prompt drawn AT the object, which is the thing Chris actually asked for.
	 *
	 * *"there isn't anything on screen to guide me. like in arc raiders it
	 * shows the item and E is take... you can see it as a visual queue on the
	 * screen, its noticeable."* A readout in a corner tells you about the
	 * world; a marker on the object is the world telling you about itself, and
	 * only the second one guides a hand.
	 *
	 * Projected from the relic's world position each frame, so it tracks the
	 * object as the keeper moves rather than sitting in a fixed place. The
	 * bracket is a stand-in for an outline: a real outline needs a
	 * post-process material reading custom depth, which is `Content/` and
	 * Chris's - and `ULLKRelicComponent` already sets the stencil for it, so
	 * the day he authors one it has three states waiting.
	 */
	/**
	 * Binds this HUD to every Sweeper's four cue delegates.
	 *
	 * **Why this exists at all.** `OnTelegraph`, `OnStockStarted`,
	 * `OnStockInterrupted` and `OnStockFiled` were all declared and
	 * broadcast, and **nothing in the project bound any of them** - no
	 * `AddDynamic`, no `BindUFunction`, and the pawn in Boswell's is
	 * `LLKSweeper_0`, the C++ class itself rather than a `BP_..._C`, so no
	 * Blueprint was binding them either. Found by the MCP session, 2026-11-02.
	 *
	 * So row 16 - every enemy announces itself with sound before sight - has
	 * been unmet *in play* since `OnTelegraph` landed, and the filing beat's
	 * interrupt was invisible: a player lit up on purpose to save their crate
	 * and success looked identical to nothing happening.
	 *
	 * **This is a debug readout and not the answer.** The authored sound and
	 * effect are Chris's (row 41), written up as item 4 of
	 * `NOTES-FOR-CHRIS.md`. What this does is make the beat *perceptible
	 * tonight*, so the question of whether it reads can be judged by playing
	 * rather than by reading code. When the real cues land, this can go.
	 */
	void BindSweepers();

	UFUNCTION()
	void OnSweeperTelegraph(FVector Position);

	UFUNCTION()
	void OnSweeperStockStarted(FVector Position);

	UFUNCTION()
	void OnSweeperStockInterrupted(FVector Position);

	UFUNCTION()
	void OnSweeperStockFiled(FVector Position);

	/** Latches a notice for `NoticeSeconds`. Later notices replace earlier ones. */
	void ShowNotice(const FString& Text, const FLinearColor& Colour);

	/**
	 * Draws the latched notice, outside the diagnostic gate.
	 *
	 * With the storm panel off a player still needs this, for the same reason
	 * `DrawWorldPrompt` and `DrawExtraction` sit outside that gate: it is the
	 * playable world's language rather than a diagnostic.
	 */
	void DrawBeatNotice();

	void DrawWorldPrompt();

	/**
	 * A downed crewmate, and the rescue in progress. M2.2.
	 *
	 * **Built because I repeated a mistake Chris had already caught.** The
	 * relic went in with its state in a corner readout and nothing on the
	 * object, and he said so: *"there isn't anything on screen to guide me."*
	 * Three hours later I landed the revive the same way - a working hold
	 * with no bracket, no prompt and no progress, so a rescue was invisible
	 * to the person performing it.
	 *
	 * A body outranks loot, so this draws instead of the relic prompt rather
	 * than beside it. Four people on voice chat need to see WHO is down and
	 * HOW FAR a rescue has got without asking.
	 */
	void DrawCasualtyPrompt(const class ULLKVitalsComponent* Casualty);

	/**
	 * What a keeper on the floor is told, centre-screen.
	 *
	 * Their own state, not somebody else's: whether they have an anchor
	 * revive left, or whether they are waiting on the crew. Row 9 makes the
	 * second case the common one, and a keeper who does not know they are
	 * waiting reads an indefinite wait as a hang.
	 */
	void DrawDownedPanel(const class ULLKVitalsComponent* Own);

	/**
	 * Extraction, in the two places Chris allowed it. M3.
	 *
	 * *"It must not become another top-left debug paragraph. Make it
	 * contextual... Put the actionable prompt in-world at the zone; reserve
	 * centre-screen messaging for the active climax."* So this draws nothing
	 * at all for most of a run, nothing in the panel ever, and the words come
	 * from `LLKExtractionReadout::Describe` rather than from here - the real
	 * HUD is his, and it should read one function instead of a screenshot.
	 */
	void DrawExtraction();

	/** A filled bar, for a hold. Returns the height it used. */
	float DrawProgressBar(const FVector2D& At, float Width, float Fraction, const FLinearColor& Colour) const;

	/** Half-width of the bracket around a focused object, in 1080p pixels. */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Debug", meta = (ClampMin = "4"))
	float BracketHalfSize = 26.f;

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

	/**
	 * How long a beat notice stays up.
	 *
	 * Three seconds against the 1.93 s the MCP session measured between
	 * "began filing" and "filed" in PIE, so a start notice is still readable
	 * when the crate lands rather than being replaced before the player has
	 * connected the two.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Debug", meta = (ClampMin = "0.1"))
	float NoticeSeconds = 3.f;

	/** Sweepers already bound, so a re-scan does not double-bind. */
	TSet<FObjectKey> BoundSweepers;

	/** Next world time at which to look for Sweepers that have appeared since. */
	float NextSweeperScanSeconds = 0.f;

	FString NoticeText;

	FLinearColor NoticeColour = FLinearColor::White;

	/** World time the notice stops drawing. */
	float NoticeExpirySeconds = 0.f;
};

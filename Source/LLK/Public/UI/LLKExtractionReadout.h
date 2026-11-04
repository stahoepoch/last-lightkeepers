// What extraction says on screen, and where. M3.
//
// **Chris's instruction, which is mostly about restraint:** *"It must not
// become another top-left debug paragraph. Make it contextual... Put the
// actionable prompt in-world at the zone; reserve centre-screen messaging for
// the active climax."* So there are exactly two places a message can go, and
// which one it goes to is a rule rather than a drawing decision:
//
//   - **In-world, at the zone**, when there is something to do there. One
//     line: `Hold [E] - call Lanternhold`.
//   - **Centre screen**, only while the climax is actually running. Four
//     states, and no fifth: incoming, hold, holding-for-you, secured.
//
// They never both speak. The prompt is an offer and the centre is an event,
// and once the event is underway the offer is gone - calling again is refused
// as `AlreadyUnderway`, so a prompt still saying "call Lanternhold" would be
// advertising a key that does nothing.
//
// **Why the strings live in a pure function rather than in the HUD.** This is
// the one part of extraction a player ever reads, and the readout is a debug
// HUD today and Chris's real widget later (the M3 HUD, which is his). If the
// words and the rules about when to show them live inside `DrawHUD`, the real
// widget starts by reimplementing them from a screenshot. Here, it reads one
// function and draws what it says.
//
// It also makes the wording testable, which matters more than it sounds: the
// difference between a countdown that reads `0:08` and one that reads `0:07`
// at the moment the siren starts is one `CeilToInt`, and nobody notices it in
// a screenshot.
//
// **One state is deliberately NOT invented here.** A gated exit - the freight
// lift without `SubBasementBreakerRestored` - has no line in Chris's five, and
// the honest consequence is that the prompt shows for it and the server
// refuses with `RequirementNotMet` in the log. Writing copy for a sixth state
// he did not ask for would be putting words in the game's mouth; it wants a
// decision from him first, and a client cannot see the requirement anyway.

#pragma once

#include "CoreMinimal.h"
#include "Run/LLKExtractionSnapshot.h"
#include "LLKExtractionReadout.generated.h"

/**
 * The two slots, filled or empty.
 *
 * Empty strings rather than flags, because "is there anything to say" and
 * "what is it" are the same question and splitting them gives a caller two
 * things to get out of step.
 */
USTRUCT(BlueprintType)
struct FLLKExtractionReadout
{
	GENERATED_BODY()

	/** Drawn at the zone, in the world. Empty unless the keeper is standing there. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	FString ZonePrompt;

	/** Drawn centre screen. Empty unless the climax is running. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	FString Centre;

	/**
	 * True when the centre line is a demand rather than an announcement.
	 *
	 * The hold and the empty dock are things a player must act on now; the
	 * incoming rig and the secured haul are things that have happened. A HUD
	 * is free to ignore this, which is why it is one bool and not a colour.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	bool bUrgent = false;

	/**
	 * Where to take what you are carrying. Empty unless you are carrying
	 * something and are not already standing at an exit.
	 *
	 * **Why this is a third field and not `Centre`.** Chris, 2026-11-04, after
	 * walking the dressed circuit: *"i still don't know how to extract."* He
	 * was right, and nothing was broken - the zone prompt works, and it only
	 * ever appears once you are already standing in the bay. So the game told
	 * you how to extract exactly when you no longer needed telling, and never
	 * told you the dock existed.
	 *
	 * `Centre` is deliberately silent while the run is idle: an idle run has
	 * no climax, and `tests/extraction.test.mjs` guards that branch against
	 * writing one. That rule is right and this is not an exception to it - a
	 * keeper holding a relic with nowhere to put it is not a climax, it is an
	 * errand. So it gets its own field, and the HUD places it quietly.
	 *
	 * It is gated on carrying because extraction REFUSES an empty-handed
	 * call, and a line telling you to do something the rules forbid is worse
	 * than no line. Before you are carrying, the objective is to find
	 * something, which is a different sentence nobody has written yet.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Extraction")
	FString Objective;

	bool HasAnything() const
	{
		return !ZonePrompt.IsEmpty() || !Centre.IsEmpty() || !Objective.IsEmpty();
	}
};

namespace LLKExtractionReadout
{
	/**
	 * `bAtZone` is this keeper standing in an extraction zone, locally.
	 *
	 * Local on purpose: whether the prompt is on your screen is a question
	 * about where you are standing, so it is answered on your machine, exactly
	 * as the relic focus is. The hold itself is server truth and arrives in
	 * the snapshot - above all `bHoldStalled`, which a client could not work
	 * out for itself because only the server knows who is in the bay.
	 */
	/**
	 * `CarriedCount` and `NearestExitId` are what the objective line needs,
	 * and both are local facts like `bAtZone`: what you are holding, and which
	 * authored exit is closest to you. Defaulted so every existing caller and
	 * test keeps compiling and keeps getting exactly the readout it got
	 * before - an unsupplied exit means no objective line rather than a line
	 * naming nowhere.
	 */
	LLK_API FLLKExtractionReadout Describe(
		const FLLKExtractionSnapshot& Snapshot, bool bAtZone,
		int32 CarriedCount = 0, FName NearestExitId = NAME_None);

	/**
	 * `LoadingDock` -> `Loading Dock`, for a line a player reads rather than
	 * types. The ids are authored in `data/loot.json` and are the same strings
	 * `LLK.CallExtraction` takes, so this only ever changes presentation.
	 */
	LLK_API FString HumaniseExitId(FName ExitId);

	/** `0:08`, the way a countdown reads. Rounds up, so a running clock never shows 0:00. */
	LLK_API FString FormatSeconds(float Seconds);
}

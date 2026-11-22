// One thing a keeper can press E on, without the interactor learning its name.
//
// **The defect this answers is structural, and it is already five deep.**
// `ULLKInteractorComponent` resolves focus for a casualty, a relic, an
// extraction zone, a lantern pickup and an opening grille. Each one cost the
// same three edits: a `TObjectPtr` member, a `Resolve...` function, and a
// `Server...` RPC, plus a new arm in `BeginInteract` and a new clause in the
// condition that suppresses the zone prompt. Five types, five copies of one
// shape.
//
// That was fine while E meant "take the thing" and the exceptions were rare.
// It stops being fine the moment the chapter wants puzzles: a puzzle with
// three parts is that surgery three times, in a component that already
// carries a comment warning that the fix for a hidden prompt is never to
// reorder these arms. The next person to add a sixth interactable would be
// reading five precedents and copying the one nearest the cursor.
//
// So: one interface, one focus slot, one RPC. **The existing five are
// deliberately NOT migrated.** They work, they are covered, and one of them -
// the grille - has another writer in it right now. This stops the bleeding
// rather than reopening it; migrating them is a separate change with its own
// proof, and doing both at once would mean a regression in the old path and a
// bug in the new one arriving in the same commit, indistinguishable.
//
// **The actor owns its own reach and aim, and that is the point.** The grille
// asks for a read at its mark; a pushable asks for a shove at hand height;
// something on a top shelf asks for a reach a child does not have. A single
// reach constant on the interactor would have to be the loosest of them, and
// then every object would be interactable from where the loosest one is. So
// `CanFocus` is pure virtual and each actor answers for itself, exactly as
// `ALLKOpeningGrille::CanFocus` already does.
//
// **Held, not just pressed.** `BeginInteract` and `EndInteract` are a pair
// because pushing is a verb you do *for a while* - the keeper holds E and
// walks, and letting go stops it. A one-shot interactable simply ignores the
// release. The interactor already has the press/release pair wired to input;
// this is the first thing to use both ends of it for the same object.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "LLKInteractable.generated.h"

UINTERFACE(MinimalAPI)
class ULLKInteractable : public UInterface
{
	GENERATED_BODY()
};

class LLK_API ILLKInteractable
{
	GENERATED_BODY()

public:
	/**
	 * Is this keeper close enough, and looking at it closely enough, now.
	 *
	 * Asked by the search that drives the prompt AND by the server before it
	 * grants, so the two cannot disagree about what E meant. The server does
	 * not trust the client's answer: it re-resolves and only acts when both
	 * sides name the same actor. Same contract as `ResolveLanternPickup`.
	 */
	virtual bool CanFocus(const APawn* Keeper) const = 0;

	/**
	 * What the prompt says, in the imperative. "Push", "Climb", "Pull free".
	 *
	 * On the interface rather than in the HUD, because the HUD asking what
	 * kind of thing it is looking at is how the five hard-coded branches got
	 * copied into the prompt layer as well.
	 */
	virtual FText FocusVerb() const = 0;

	/** Server. E went down with this focused. */
	virtual void BeginInteract(APawn* Keeper) = 0;

	/**
	 * Server. E came up, or focus was lost, or the keeper went down.
	 *
	 * Called unconditionally on release - the actor knows whether it had
	 * anything running, and a component tracking that separately is a second
	 * copy of the same fact that can disagree with the first. That is the
	 * lesson `EndInteract` already applies to the extraction call.
	 */
	virtual void EndInteract(APawn* Keeper) = 0;
};

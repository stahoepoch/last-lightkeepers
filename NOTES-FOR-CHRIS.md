# Notes for Chris — the `Content/` work the code is waiting on

> **The open items across all four of us, with owners and proofs of closure,
> are in `docs/HANDOFFS.md`.** This file stays as the detailed spec for the
> `Content/` work; that one is the ledger.
>
> **Why this file exists.** `docs/LANES.md` says the Claude → Chris handoff is
> "a C++ class landed complete with a written note saying exactly what to make
> and what to set." I have been landing the classes and skipping the note, so
> the specs for two finished systems existed only in commit messages and code
> comments. This is the note. It is symmetric with `NOTES-FOR-CODEX.md`.
>
> Everything here degrades to nothing when the content is absent — no crashes,
> no blocked play. That is the rule and it holds today: the keeper walks, the
> lantern lights, relics read and can be taken, all on fallbacks. None of the
> three below is urgent. They are what turns scaffolding into the real thing.

---

## 1. The Input assets — `/Game/Input`

**What it unblocks:** rebindable keys and full controller support, replacing
the in-code bindings. **Guarded by:** `.\Tools\validate_input_assets.ps1`
(Codex's), which fails right now on the missing `IMC_LLK` rather than silently
accepting the fallback.

One `UInputMappingContext` named **`IMC_LLK`**, plus seven `UInputAction`
assets. The names and types are not suggestions — the validator and
`ALLKCharacter`'s eight soft object paths (the context plus the seven actions)
both expect exactly these:

| Asset | Value type | Keyboard | Gamepad |
| --- | --- | --- | --- |
| `IA_LLK_Move` | Axis2D | W A S D | Left stick |
| `IA_LLK_Look` | Axis2D | Mouse XY | Right stick |
| `IA_LLK_Sprint` | Boolean | Left Shift | Left thumbstick click |
| `IA_LLK_Crouch` | Boolean | Left Ctrl | Right thumbstick click |
| `IA_LLK_Jump` | Boolean | Space | Face button bottom (A / Cross) |
| `IA_LLK_Lantern` | Boolean | F | D-pad right |
| `IA_LLK_Interact` | Boolean | E | Face button left (X / Square) |

**Two things that will bite if missed.**

Mark the mappings **player-mappable**, or rebinding has nothing to bind. The
validator checks this, which is the point of it existing.

And the `Move` mappings need the modifiers the code path already uses: **S
negated on X; D swizzled YXZ; A swizzled YXZ *and* negated on Y.** Swizzle
runs first, so negating X after it makes A identical to D — that exact bug is
why `LLK.Input.TransientFallback` asserts the modifier shape. `Look` needs **Y
negated** so pushing the mouse forward looks up.

**It is all seven or none.** `FLLKInputBindings::IsComplete()` falls back
wholesale rather than mixing sources, because a keeper with authored movement
and a code-bound lantern is a keeper whose rebinding screen lies about one key.
Six authored assets and a missing seventh behaves exactly like zero.

---

## 2. The relic outline material — the object saying *why*

**What it unblocks:** your own note — *"the animation and making it WHY, like
it doesn't light up or whatever."* You were right that the rule only existed
in a corner of the screen. The code half is done and driving three states; the
look is yours.

`ULLKRelicComponent` sets **custom depth stencil** on every primitive of a
relic actor, updated whenever legibility or the affordance changes:

| Stencil | State | What it means to the player |
| --- | --- | --- |
| **1** | Visible, unidentified | Something occupies that shelf. You cannot tell what. **This one must read from the dark** — if it is invisible, nobody is ever tempted to light the lantern and the ground floor teaches nothing (row 53) |
| **2** | Read, but dark now | You know what it is. You cannot have it yet. The knowledge is kept; the reach is gone |
| **3** | Read and takeable | Lit, in range, yours |
| **0** | Taken | Nothing. Custom depth off |

A post-process material reading `CustomDepth` / `CustomStencil` gets all three
for free. **The interesting one is 2 → 3 and back**, because that transition is
row 5 explaining itself on the object: walk away with the lantern and the thing
you identified visibly stops being available. If one state gets art, make it
that pair.

Nothing in code changes when you author this. The stencil values are already
correct in play.

---

## 3. M1.5 — the greybox ground floor

**What it unblocks:** everything. Codex and I both reached the same conclusion
independently: the next blocker is content, not code. It also gates row 63's
traversal work, because vault heights and ledge placement are measurements of
real geometry — built against my runtime harness they would encode the harness.

**I am not going to restate the brief here.** `docs/THE-STORE.md` has the six
acceptance questions, and five of them can be judged on a blockout with no
art, no audio and no enemies — the day it exists, rather than after a
playtest. The sixth is a code rule and it is mine.

Two things from this side that may be worth knowing while you lay it out:

- **The lantern reaches 840cm** (30 studs, `data/lantern.json` `light.radiusStuds`).
  My harness puts aisles 14 studs apart so a lit keeper spills into the
  neighbouring two, which is where the trade becomes visible. Whatever spacing
  you choose, that relationship is the thing to feel.
- **A lit keeper is noticed from 38 studs** (`enemies.json` `aggroRadiusLit`),
  further than they can see. `tests/balance.test.mjs` enforces that asymmetry,
  so sightlines longer than about 38 studs are where a player can be hunted by
  something they have no way to have seen.

**And please leave a counter and a service ledge at honest heights.** Row 63's
vault and mantle have to be tuned against something real, and that is the one
piece of preparation that can happen before the traversal work starts.

---

## 4. The Sweeper's four cues — nothing is listening to any of them

**What it unblocks:** whether the first Sweeper beat can be *read* at all.
Added 2026-11-02, and it is the one item on this page that is not about polish.

`ALLKSweeper` broadcasts four Blueprint-assignable delegates, all with the
same signature — `FLLKSweeperTelegraph(FVector Position)` — so one cue actor
pattern serves all four:

| Delegate | Fires when | Position it hands you |
| --- | --- | --- |
| `OnTelegraph` | it is about to come for a keeper | the Sweeper |
| `OnStockStarted` | it begins filing something misplaced | the Sweeper |
| `OnStockInterrupted` | it was seen, and dropped the job | the Sweeper |
| `OnStockFiled` | it finished; the thing is elsewhere now | **the bay**, not the Sweeper |

**Corrected within the hour: `ALLKDebugHUD` now binds all four**, in
`8738b57`, and that commit was already in the tree when the paragraph below
was written. Read the next two paragraphs as history, and `8738b57` as the
reason there is now *something* to see rather than nothing.

~~**All four broadcast into nothing today.**~~ As first written: grepping
`Source/` for the four names found only the four `Broadcast` calls and the four
declarations - no `AddDynamic`, no `BindUFunction`, nowhere. The pawn in
Boswell's is `LLKSweeper_0`, the C++ class's own name rather than a `BP_..._C`,
so there was no Blueprint in that chain doing it either. **The beat played in
silence, and that included `OnTelegraph`**, which had been broadcasting to
nobody since it landed.

**That last part is the serious half and it is still not fixed by a debug
readout.** Row 16 - every enemy telegraphs with sound before sight,
distinguishably - is LOCKED in `DECISIONS.md` and is an accessibility
requirement, not polish. It has been unmet *in play* since `OnTelegraph`
landed: in Boswell's the Sweeper announces itself with nothing at all. It was
invisible because the declaration and the broadcast both exist and read as
finished code. Tracked as `docs/HANDOFFS.md` **C2**, which is the row to read
before the rest of this page.

**What `8738b57` is and is not.** The debug HUD latches an on-screen notice for
three seconds per cue - against the measured 1.93 s between `began filing` and
`filed`, so a start notice is still up when the crate lands - drawn outside the
`LLK.ShowStormHUD` gate. **It is a debug readout, not the cue.** It exists so
you can judge the beat by playing it rather than by reading code, and it
deletes cleanly when the authored sound and effect land. Everything below still
needs authoring.

**`OnStockInterrupted` is the one to author first, if only one gets authored.**
It is the only one of the four the player *caused on purpose* — they raised the
lantern, accepted being seen, and saved their crate. Without a cue there, the
mechanic's entire result is invisible: the crate simply stays put, which is
indistinguishable from nothing having happened. A mechanic whose success looks
like nothing is not a mechanic the player will ever learn.

**`OnStockFiled` hands you the bay's position deliberately**, so the cue can
point at where the crate went rather than at the machine that moved it. The
cost of the beat is the walk back; a sound at the Sweeper tells the player they
were robbed, and a sound at the bay tells them where to go.

Nothing in code changes when you author these. The broadcasts are already
correct in play - `began filing` to `filed ... into the stock bay` was observed
1.93 s apart in PIE on 2026-11-02, so there is a real 1.93-second window for a
start cue to occupy and an interrupt to land inside.

**One thing worth keeping from the placeholder text**, which was written to be
legible rather than good and will be deleted. The debug notice for a finished
filing reads *"your crate has been put somewhere it belongs"* - the machine's
view of the event, not the player's loss. That is deliberate and it is row 47:
the building is not wrong, it is working, and being robbed by something that
does not consider itself a thief is the whole feeling. If the authored cue
carries any text at all, that is the distinction to keep.

## What is NOT waiting on you

Said explicitly, because a list like this reads as a backlog of blockers and
only the first one actually blocks anything:

- The keeper walks, sprints, crouches, jumps and toggles the lantern now.
- Relics read, refuse, and can be taken, with a world-space prompt.
- The storm meter runs, replicates and projects.
- Eleven in-engine tests and the full data suite are green.

All of it on declared fallbacks that announce themselves in the log. If you
never author any of the above, none of it breaks — it just stays scaffolding.

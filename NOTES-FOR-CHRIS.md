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

## 5. The fixture light contract asks for something light cannot do

**What it unblocks:** the first art pass on any lit aisle. Added 2026-11-02
after measuring against your own locked contract rather than around it.

Your rule in `docs/FIXTURE-LIGHT-CONTRACT.md` - ambient fixtures *"may help
players navigate, but must never look bright enough to read a relic under"* -
is the right rule and it has **no candela solution.** Measured in `LLK_LightLab`
under the Lumen baseline:

| | |
| --- | --- |
| inspection ceiling - highest candela where a price card is still NOT readable at 2 studs | **below 0.5 cd** |
| orientation floor - lowest candela where a 40-stud run stays legible | **~2 cd** troffer, **~6 cd** neon |
| the gap | **about 4x**, in the wrong direction |

Pre-Lumen that gap was over 12x; indirect light helped orientation far more
than inspection, because orientation is read off the floor plane and the card
was always lit directly. **Three levers were tried against the inspection half
and all three failed:**

| lever | card | orientation |
| --- | --- | --- |
| lower the candela | still reads at 0.5 cd, the bottom of the ladder | the run goes dark first |
| barn doors 15 deg | **-11%** | fine, and the side walls improve to zero |
| barn doors 2 deg | card dark | run dark too, ratio *worse* than plain |
| floor albedo 0.18 -> 0.03 | **-2.5%** | floor read **-63%** |

The reason is one sentence: **the card is lit *directly* by any fixture broad
enough to light the aisle**, so every lever that acts on indirect light misses
it. Darkening the floor sixfold moved the card by 0.7%.

### The lever that works is geometry, and it is yours

A shelf lip over the readable face. Ambient troffer at 2 cd throughout:

| rig | card mean | text contrast | bay wall | readable? |
| --- | --- | --- | --- | --- |
| no lip | 92.29 | 102.7 | 4.92 | yes |
| shallow lip, 10 cm | 71.36 | **104.1** | 4.70 | **yes - unchanged** |
| **deep lip, 30 cm** | 10.33 | **14.8** | **4.64** | **no** |

**Text contrast falls sevenfold and the bay wall does not move** - 4.64 against
4.92. Every earlier lever bought card darkness by taking light out of the room;
this one takes none.

**The depth is not a taste call, it is a ray.** The lip must reach past where
the line from the fixture to the *bottom* of the readable face crosses the lip's
height. For the lab's pose - fixture at (200, 0, 290), card spanning z 75-105 -
that crossing is at x ~= 315, so the lip needs about **23 cm**. Ten could not;
thirty could.

**And the shallow lip is the useful negative.** It drops the card's *mean* by
shading its top edge while leaving the glyph band fully lit, so contrast does
not move at all. **Shading a label is not the same as shading the part you
read** - which is the trap if this gets eyeballed rather than traced.

Neither the lantern's path nor the player's eye line is occluded by the lip;
both were traced and both first-hit the card face. So the mechanic works: the
lip shades the fixture directly overhead and nothing at eye level.

### One thing to check when you do it

At 60 cm over a recessed label, the readable window measured **6 cd reading
cleanly with zero clipping, and clipping beginning between 6 and 15 cd.** The
lantern now authors **25 cd**, which is above that. That reading was taken on a
**72 deg** outer cone and the authored lantern is **110 deg**, so the same
candela is spread wider and the two numbers are not directly comparable - which
is why this is a thing to look at rather than a number to change. Tracked as
`HANDOFFS.md` M5, and it needs the editor.

Nothing in the contract has been edited. It is LOCKED and it is yours; this is
the measurement and the remedy, as `HANDOFFS.md` **C9**.

## What is NOT waiting on you

Said explicitly, because a list like this reads as a backlog of blockers and
only the first one actually blocks anything:

- The keeper walks, sprints, crouches, jumps and toggles the lantern now.
- Relics read, refuse, and can be taken, with a world-space prompt.
- The storm meter runs, replicates and projects.
- Eleven in-engine tests and the full data suite are green.

All of it on declared fallbacks that announce themselves in the log. If you
never author any of the above, none of it breaks — it just stays scaffolding.

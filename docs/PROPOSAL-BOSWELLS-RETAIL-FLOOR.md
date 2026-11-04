# Proposal — dressing Boswell's as a sales floor, with the numbers

> **Status: PROPOSAL, 2026-11-04. Nothing placed, nothing bought.** Written
> against `FINDINGS-BOSWELLS-READS-AS-STOCKROOM.md`, which diagnosed Chris's
> *"it looks like the stockroom and not a dept. store."* That finding named the
> shopping list; this one does the arithmetic on it, because the obvious
> version of the swap makes the room worse and the measurements say so.
>
> **No patrol points, route shapes, collision tuning or spawn spacing are
> proposed here.** The dock map authors those. This is dressing geometry only.
> Every number derives from `FINDINGS-BOSWELLS-DRESSING-PASS.md` or from
> `docs/patches/dress-boswells-circuit.py`.

## The trap: shallower shelving makes the aisle worse

The racking's **outer** face is flush at x ±392.5 and its 106 cm depth grows
*inward*, so the central aisle is 2 × (392.5 − 106) = **573 cm**. The dressing
pass deliberately took all 41 cm of its tightening out of the middle and left
the 167.5 cm side aisle alone, because the middle had 655 and could afford it.

A retail gondola is 45–60 cm deep instead of 106. Swap like for like and the
depth that was eating the aisle stops eating it:

| Wall gondola depth | inner face | central aisle | vs 573 today |
| --- | --- | --- | --- |
| 45 cm | ±347.5 | 695 | **+122** |
| 55 cm | ±337.5 | 675 | **+102** |
| 60 cm | ±332.5 | 665 | **+92** |

**So the naive swap hands back more than the pass just took, and ends up wider
than the 655 it started from.** The props would read retail and the space would
read emptier — a wide bare hall, which is a warehouse feeling arriving by a
different road. `FINDINGS-BOSWELLS-DRESSING-PASS.md` already flagged 573 as
"the number that decides how the aisle plays"; 695 is not a smaller version of
that question, it is the question reopened.

## The fix is the thing that makes a sales floor a sales floor

A department store is not one wide aisle with shelves at the walls. It is a
**grid**: wall runs plus free-standing double-sided islands. Adding a central
island at x 0 turns one too-wide aisle into two shoppable ones.

| Wall depth | central island | two aisles of | × a 68 cm Sweeper | ≥ 200 cm gap |
| --- | --- | --- | --- | --- |
| 45 | 90 | 302.5 | 4.45× | yes |
| 45 | 110 | 292.5 | 4.30× | yes |
| **55** | **110** | **282.5** | **4.15×** | **yes** |
| 55 | 130 | 272.5 | 4.01× | yes |
| 60 | 120 | 272.5 | 4.01× | yes |
| 60 | 140 | 262.5 | 3.86× | yes |

Every row clears the authored **200 cm escape gap** with room to spare, which
is the constraint the dressing pass was built around and measured at 200.0 at
the shelf face. The middle row is the recommendation: a 55 cm wall run with a
110 cm double-sided island (two 55 cm backs), giving two aisles of 282.5.

Two consequences worth stating plainly, because they are the point:

- **The aisle count doubles.** Two parallel routes through the room instead of
  one, which is a route-choice change, and route choice is what
  `data/lantern.json` says movement is *for*. Whether that is wanted is Chris's
  call, not a measurement.
- **A Sweeper in one aisle is not a Sweeper in the other.** Today the central
  aisle is the patrol lane and the room has one. This would make avoidance
  positional. That is a design change dressed as an art change, and it should
  be decided deliberately rather than discovered in play.

If either is unwelcome, the honest alternative is **re-zoning rather than
replacing** — the first option in the stockroom finding. Boswell's has a
stockroom, a dock and back-of-house; everything placed belongs in one of them.
That costs nothing and keeps the 573.

## Sightlines, which is where this stops being about depth

Racking is 250 cm. A gondola is 122–198 cm. A keeper's eye is authored at
**152 cm above the floor**, so the shelf height decides whether the floor is a
space you can read or a maze you cannot:

| Shelf | a 152 cm eye |
| --- | --- |
| 48" / 122 cm | sees over, whole floor legible |
| 54" / 137 cm | sees over, just |
| 60" / 152 cm | exactly eye level — sees the top surface edge-on and nothing beyond |
| 72" / 183 cm | blocked |
| 78" / 198 cm | blocked |

**152 cm shelving is the one height to avoid**, because it is the only one whose
read changes with a few centimetres of floor slope or a crouch. 137 is the
interesting one: a keeper sees across the floor by 15 cm, which means heads and
lantern glow are visible over the shelves while bodies are not.

At the store's 2 candela fixture level, silhouette is what reads — the
stockroom finding makes that point about rack frames and it cuts the same way
here. A floor of 137 cm gondolas is a room where you can see something coming
and still not see what it is.

**Not blocked, and an earlier version of this file said it was.** That
paragraph claimed the pass could not run until the gameplay light anchor was
settled, on the grounds that it sat at 240 cm. **It does not** —
`FINDINGS-LANTERN-ANCHOR-ORIGIN.md` has since been corrected: the live anchor is
**133 cm** standing level, 19 cm below the eye by deliberate design, and it
travels between **106 and 194 cm with look pitch alone**, because
`LanternAnchor` hangs 42 cm forward of a camera that rotates.

That makes the floor read *better* rather than worse, and it changes what shelf
height buys:

| Shelf | eye, 152 | light level, 133 | light looking up |
| --- | --- | --- | --- |
| 48" / 122 cm | sees over | clears | clears |
| **54" / 137 cm** | **sees over, just** | **blocked** | **clears** |
| 60" / 152 cm | eye level | blocked | clears |
| 72" / 183 cm | blocked | blocked | clears above 72° |
| 78" / 198 cm | blocked | blocked | never clears |

At 137 cm the keeper reads the shape of the floor without lighting it, and
raising the lantern to look at the far wall spills light over every shelf in
between — a legible cost the player can learn and choose. **137 cm is now the
recommendation rather than merely the interesting case.** Whether the pitch
travel should be that large is a separate design question in that finding, and
it does not gate this pass either way.

## What the pack can do today, with no purchase

The relics are the cheapest change with the largest effect and they need no new
kit, which the stockroom finding is right about. The map places two —
`BSW_WRONG_CRATE` and `BSW_DISPLAY_RECEIVER` — and both currently use
`SM_Crate_Stack_1`, a floor-standing warehouse pallet, for objects
`data/relics.json` authors as hand-held. All 20 authored relics are
department-store objects; none is a crate.

The pack's 101 meshes are industrial, but it does contain boxes at honest
hand scale:

| Mesh | Size | Serves |
| --- | --- | --- |
| `SM_Box_Cardboard_25x20x10cm` | 25×20×10 | the smallest thing in the pack — a boxed ring, a ledger, a handset |
| `SM_Box_Cardboard_50x20x15cm` | 50×20×15 | long and flat — a rota, a drawing, a lanyard on a card |
| `SM_Box_Cardboard_30x30x30_Open` | 30³, open top | **arcade tokens, six hundred** — an open box of them |
| `SM_Box_Cardboard_30x30x30_Cap` | 30³, lidded | a closed presentation box |

The change that matters is as much **scale and height** as material. A relic is
something you lean in to read, so it belongs small and at hand height on a
surface, not floor-standing at pallet size. A 25×20×10 box on a counter at
~110 cm reads as an object someone left behind. The same box on the floor reads
as stock. It is still cardboard, and it is still far closer to a boxed ring
than a pallet of crates is.

This is a swap of mesh and transform on two existing actors. **It needs the
editor**: `LLK_Boswells_FirstRun.umap` is monolithic, so unlike `LLK_DockBay`
there are no One-File-Per-Actor files to read or edit from outside. The actors
keep their authored positions; only what you see changes.

## The shopping list, if the floor is to be re-kitted

Small and specific, per the stockroom finding, and governed by
`ASSET-DEPENDENCIES.md`: **meshes, materials and blueprints freely; texture
sets one at a time and only when something in the map uses one.** The last
import measured 101 meshes at 21 MB against 26 textures at 302 MB, so the
meshes are effectively free and the textures are the entire cost.

1. **Gondola shelving, 55 cm deep, 137 cm tall**, single- and double-sided,
   with end caps. The one item that does the work.
2. **A sales counter and a glass display case.** There is already a counter in
   the room — `dress-boswells-circuit.py:98` probes a route "crossing E to
   counter y 350" — so this dresses something the layout assumes rather than
   adding furniture. (The stockroom finding says the pass placed a toy counter
   and a dead display case; the script only proves a counter is *routed
   around*, so who placed it is unconfirmed here.)
3. **Clothing rails.** `data/enemies.json:53` authors an enemy family whose
   `formerJob` is "Department-store display mannequins", so rails have a second
   job and are not pure dressing.
4. **Ceiling troffers** rather than high-bay pendants. The pack's
   `Air_Conditioning` set has lit and unlit ceiling panels and is the nearest
   thing already in hand — worth trying before buying anything.

Items 1 and 2 answer Chris's sentence. Items 3 and 4 are polish.

## What this proposal does not claim

- **It does not justify undoing the dressing pass.** Its measurements are
  sound, the 200 cm gap survived real geometry, and the dock is a real opening.
- **It does not decide 282.5 cm is a good aisle**, any more than the pass could
  decide 573 was. Only a dressed circuit at the authored speed answers that,
  and that playtest has still never happened.
- **It proposes no purchase.** The list is a list.

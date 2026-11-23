# Three child steps wide, and a high ceiling with glass

**2026-11-23.** Chris, remote and unable to look at anything:

> *"The pacing should be three child-like steps in width of a walkway. And the
> ceilings are high, high ceilings with the glass on the ceilings. That's the
> most important thing."*

Two rules. The first is now a constant the floor repacks around. The second is
blocked by the building, and this page says by what.

---

## 1. The walkway: 180 cm

Nothing in the repo measured a step, so it is derived rather than picked —
decision row 81 allows that when the derivation is written beside it. Walking
step length is about 0.41 of stature; the keeper is a child of 145 cm, so a
step is 59.5 cm, taken as **60**. Three of them is **180 cm**, and
`CHILD_STEP` and `WALKWAY` in
`docs/reference/boswells/make_groundfloor_plan.py` are now those numbers.

Changing the constant repacks the whole ground floor: every bay grows until
its aisle is exactly one walkway, a bay facing another bay taking half the
slack and a bay facing the shell or the main spine taking all of it. Going
from the drafting gap of 250 to 180 gave the departments **1,886 m² of the
4,167 m² plate, up from 1,623** — 39% selling to 45%.

**What 180 costs, which is the part worth knowing before it is built.** The
measured side aisle in `docs/reference/boswells/README.md` is 168 cm, *"two 68
cm bodies pass"*, so 180 still passes two. But the **escape gap in the same
table is 200 cm, and 180 is under it**. Inside a department you can no longer
slip past a Sweeper.

**Corrected 2026-11-23, and this correction matters more than the rule it
corrects.** The first version of this page said the escapes *"become the cross
aisles, the court and the main spine, which keep their widths."* **That is
false, and `Tools/verify_boswells_ground_cells.py` is what proved it.** The
27-cell plan has no straight cross aisle at all. It cannot have one: every
column splits into its own rows precisely so that no row runs across the
store, which is what the Meriden sheet does and what Chris asked for, and
check any single y against the six columns and at least three of them are
mid-bay there. **A straight cross aisle and that irregularity are mutually
exclusive**, so the mitigation I wrote for the 180 cm aisle was a route that
does not exist.

What the floor does still have is the main spine at x ±900 (18 m), the
escalator court, the perimeter walk, and a walkable network that a keeper-wide
body can get all the way through — the verifier floods it and checks the walk
from the service door to the mall grille. What it does not have is a 200 cm
way out of the middle of a department on the east–west axis. That is now an
open question with three honest answers and it is Chris's to pick: live with
180 everywhere and let a department be a trap, widen the walkway to 200 and
lose the "three child steps" derivation, or jog a 300 cm racetrack through the
floor so the escape route is continuous without being straight — which is what
a real anchor store does and what *"all kinds of intricate little paths... they
have angles"* describes.

**Answered 2026-11-23, by Chris and by measuring.** Chris: *"Jogging through
the middle of the floor continues to be a strange choice."* So no racetrack
through the departments. The escapes are the ones a real store has, and they
are now measured from the map rather than claimed:

| Route | Narrowest | |
| --- | --- | --- |
| West perimeter | 250 cm | clear |
| East perimeter | 250 cm | clear |
| Front perimeter | 861 cm | clear |
| Back-of-house aisle | 250 cm | clear |
| Main spine | 1800 cm | clear |

All above the 200 cm escape gap. The back-of-house aisle became continuous
across the full width on the same day, when the two invented service cells
came out of the middle of the floor.

So **inside a department there is no way past a Sweeper, and that is the
design.** Chris, the same day: *"You're a nine-year-old kid, so getting lost
should be pretty easy. You're tiny."* The 180 cm runs are the trap and the
perimeter is the way out, which is also why the player has to reach an edge
to be safe.

Unverified either way: nobody has walked any of it with the keeper capsule,
and the Sweeper's patrol has not been re-checked against it. And the walkway
itself is now unsettled from underneath — see
`docs/FINDINGS-AT-NINE-YOU-CANNOT-SEE-OVER-THE-RACKS.md`. Three child steps
of a 145 cm child is 180 cm; of a nine-year-old at 134 cm it is **165**. The
rule is Chris's and the age under it moved, so the number is his to re-say.

Unverified: nobody has walked a 180 cm aisle with the keeper capsule, and the
Sweeper's patrol has not been re-checked against it.

## 2. The high ceiling, and what stops it

The section at x = 0 (`BoswellsGroundFloor_Section.svg`) is the answer, and it
is not the one the directive wants.

| Height | What is there | Covers |
| --- | --- | --- |
| 5.14 m | suspended tile ceiling | both wings |
| 5.13 m | painted aisle bulkhead, `86cbdfd` | the 18 m main aisle |
| 6.38 m | mezzanine soffit | everything the decks sit over |
| 6.50 m | upper selling floor | — |
| **14.88 m** | **the roof** | **the atrium only** |

**A second storey and a high ceiling cannot share a floor.** The four
`ANCHOR_UPPER_*` decks cover the whole plate except the atrium void at
x ±600, y ±900. So the building already has exactly one high, glass-roofed
room — 18 m of it — and no amount of raising the tile ceiling makes another,
because the thing above the tile is not air, it is the floor somebody walks on
upstairs.

That means the directive is really a question about the mezzanine, and only
Chris can answer it:

- **Leave it.** The glass ceiling is the atrium: 216 m² of a 4,167 m² floor,
  which is what is built now, and what `THE-STORE.md` already calls Boswell's
  locked signature — the chandelier and the mirrored ceiling over the
  escalator well.
- **Open it up.** Shrink the mezzanine to a gallery ring and take the atrium
  out to the court's 20 × 20 m, or wider. The high glass room then covers the
  escalators, the court and the middle of the main aisle — the whole centre of
  the chapter's route — and the upper floor loses the selling area it gains in
  drama. This is what real anchors with glass ceilings do; the glass is over
  the well, and the selling floor around it is low.

Either way the tile ceiling over the wings stays at 5.14 m, because a deck is
over it.

**Knock-on, if the atrium widens:** the two lighting states hang 26 cm under
whatever ceiling is actually above each light, read from the map, so they
re-derive themselves on a re-run. The aisle bulkhead from `86cbdfd` does not —
it is 18 m wide because the aisle is, and a wider atrium cuts a hole in it.
`ALLKTestGameMode::ReportFloorWithoutCeiling` would report that hole, which is
what it is for.

## 3. A square with diamonds

**2026-11-23, Chris answering his own question:**

> *"The glass should be in each four corners of the square, and the middle
> part is where the escalators are, the high ceiling with the chandelier. So
> there'll be a small patch on each part of the ceiling on the bottom floor
> only of glass that is an oblongish square of glass in each corner, like a
> diamond. So it's a square with diamonds."*

So the mezzanine stays. The high ceiling is the middle, as built, and the
corners get four panels rather than one big roof.

**Corrected the same day: it is mirror, not glass.** Chris: *"Mirrored
ceiling!!! My bad. Boscovs mirror ceiling."* And
`docs/reference/boswells/source-photos/07-mirrored-atrium.jpg` — his own
photograph, in this repo — shows exactly what it is: a **faceted, panelised
mirrored coffer** stepping up out of the white tile, among ordinary troffers,
each of which it doubles. Not a flat sheet. The mirror also carries down the
vertical fascia at the atrium edge, and
`10-escalator-glass-ceiling.jpg` shows the tiered glass-rod chandelier hanging
in the well with the mirror above it.

**This makes the whole problem go away.** The objection to glass was that a
deck sits over the corners, so no daylight can reach them. A mirror does not
want daylight — it reflects the room. The 124 cm plenum stops being a lightbox
that has to be lit and becomes simply the coffer's depth.

| | |
| --- | --- |
| Coffers | 4, one per corner of the square |
| Size | **1400 × 950 cm**, turned 45° so the oblong reads as a diamond — 16.6 m corner to corner, 133 m² each |
| At | (±2625, ±1200) |
| Set in | the suspended tile, underside 5.14 m, 300 cm grid |
| Depth | recessed **124 cm** up into the plenum, to the mezzanine soffit at 6.38 m |
| Surface | mirror on the flat top and on all four slopes, in panels, as the photograph shows |
| Middle | the atrium, chandelier, 14.88 m, smoked mirror canopy |

Drawn in `docs/reference/boswells/BoswellsGroundFloor_Ceiling.svg`. The
generator refuses to place a panel that is not wholly inside a tile field, so
none of them lands half in the painted aisle bulkhead.

**Resized 2026-11-23**, same day: *"The glass panels are much larger than
that. Look at some reference pictures of Costco's glass ceiling and you can
definitely see what it's supposed to look like."* The first pass drew 500 ×
350 cm, which is a light fitting, not a ceiling. 1400 × 950 is nearly half the
width of its own tile field.

### What a mirrored ceiling does to a dark store

This is the part that is worth more than the drawing, and none of it was true
of the glass version:

- **It doubles the lantern.** 532 m² of mirror overhead in a building whose
  only moving light is the keeper's. The lantern gets a twin on the ceiling
  that moves when the player moves, and lights nothing.
- **It shows the player what is behind them.** A coffer ahead reflects the
  aisle behind. That is a horror instrument and a navigation aid at once, and
  it is free — it needs no script.
- **A reflection is a visual, not a light.** Decided below.
- **It is the one locked signature we already had.** `THE-STORE.md` has the
  chandelier and the mirrored ceiling as Boswell's signature, and §13 records
  Chris's *"glass ceiling mirrors"* as the same room Buda remembered. The
  corners now rhyme with the middle instead of competing with it.
- **Cost, unverified.** Real mirror in UE is a planar reflection, which is
  expensive, or screen-space, which drops whatever is off screen — and a
  mirror that loses the thing behind you loses the whole point. Nobody has
  measured either here.

### A reflected lantern is a visual, not a light

**2026-11-23. Chris, asked whether a reflection should count as light to the
machines:**

> *"What would it do in real life if that light were there? That's what we'll
> try to mimic. Probably it would count as a visual."*

Real life agrees with him, and it is worth writing down why, because the
physics also says what the mirror *does* add.

**A mirror does not emit. It redirects.** A real mirrored coffer does put more
light on the floor — the lantern's upward spill comes back down, less the
coating's loss, and a dusty 1990s glass mirror returns well under the ~90% a
clean one would. But the bounce is 5.14 m up and 5.14 m back, specular, so the
returned light lands roughly where it already was. It brightens the floor the
keeper is standing on. **It does not light the next aisle, and it does not
light anything the lantern was not already pointed at.** Nobody is revealed by
a reflection who was not already revealed by the lamp.

So the rule: **`LLKLightField` stays lantern-only.** No reflection registers as
a light source, nothing that reads light sees a mirror, and no machine gets a
second lamp to hunt. If the bounce is ever wanted, it belongs as a small
brightness multiplier on the floor directly beneath a coffer — the coffer's own
faint contribution — not as a second emitter. That is both the cheap
implementation and the true one, which is the rare case.

**What a mirror really adds is a sight line, not a light.** That is the
physically real thing the corners give the chapter: a coffer ahead shows the
player an aisle they cannot see, around a corner, before they walk into it.
Strict realism says that cuts both ways — a Sweeper that can see the coffer
could see what the coffer sees. Whether the machines get the mirror as a sight
line is a different question from light, and this rule does not answer it. Our
own sight checks are straight traces, so today the answer is no by default: the
mirror helps the player and not the machine. If that ever changes it should be
a deliberate choice, and an expensive one.

Unverified, unchanged: nobody has measured the cost of real mirror in UE —
planar reflection, which is expensive, or screen-space, which drops whatever is
off screen and so loses the aisle behind you, which was the whole point.

## Built, 2026-11-23

Chris: *"I checked the drawing and everything that you sent me, so go ahead
and move forward with some things."* So both rules are now in
`LLK_Boswells_OpeningPass` rather than only in drawings.

| | |
| --- | --- |
| `Tools/floor_boswells_ground_cells.py` | lays the 27-cell plan as floor |
| `Tools/build_boswells_mirror_coffers.py` | hangs the four mirrored diamonds |
| `Tools/verify_boswells_ground_cells.py` | reads the map back and draws it |

**The floor.** 20 carpeted cells, 1,679 m², one plane sized to each cell
rather than a grid of tiles — so the 180 cm walkway is not drawn, it is the
gap where the carpet stops. The other 7 cells are left as the stone tile
underneath, because a cosmetics hall, a jewellery counter, a lift lobby, a
service desk and a stock room stand on hard floor in a real anchor store.

This replaces the carpet laid by `Tools/floor_boswells_departments.py`, which
had to *infer* the departments from merchandise centroids because nothing had
authored them. The census of what that produced is the argument for replacing
it: `MI_LLK_Carpet_Ladies=7` planes against `MI_LLK_Carpet_Electronics=18`,
when LADIES is the largest department on this floor and **ELECTRONICS is not
on it at all**. Section 13 of `docs/THE-MALL-IS-STILL-TRADING.md` puts the
lower floor as juniors, jewelry, mens, perfume, ladies, shoes, with the CRT
sets sitting among juniors. The conflict is cited rather than tidied away:
the `MI_LLK_Carpet_Electronics` asset and the sign at (1600, 650) are left
alone, and no actor now uses that carpet.

**The ceiling.** Four mirror panels, 1400 × 950 cm turned 45°, at (±2625,
±1200), 532 m² in all, in `M_BSW_SmokedMirror` — the material the atrium
canopy already uses, so the corners and the middle are one ceiling family.

**They are flush, not recessed, and that is a decision.** The lower ceiling is
one slab per wing, so a coffer rising into the 124 cm plenum needs a hole in
it, and the hole is a rotated rectangle in an axis-aligned slab — which leaves
four corner triangles that neither an axis-aligned nor a 45° piece can fill
without overlapping, and `ReportFloorWithoutCeiling` would rightly call each
one a hole. At a 16.6 m span the recess is a shallow dish anyway, and the
reflection is identical either way. The faceting in Chris's photograph is
therefore still only in the drawing.

**What the verifier checks, and what it caught.** It reads every actor's real
transform, matches it to its cell by geometry rather than by label, and draws
`BoswellsGroundFloor_AsBuilt.svg` from the map. Then it floods the walkable
space at the keeper's own 68 cm and walks it: **1,425 m² reachable, and the
route from the service door the player enters by to the mall grille connects.**

Its first version also refused any cell crossing a cross aisle at y ±500, and
flagged eight. That check was wrong and the plan was right — see the
correction in section 1, which is the most useful thing this pass produced.

## Not done

Neither rule is in the map. The walkway is in the plan generator; the ceiling
is a drawing and a question. Chris is remote and will field-check later.

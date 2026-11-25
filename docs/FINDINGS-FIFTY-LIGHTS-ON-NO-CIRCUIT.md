# Fifty lights on no circuit, and a blockout nobody took out

**2026-11-23.** Chris, having opened `LLK_Boswells_OpeningPass` and looked at
it himself — the first time anyone has walked this floor in the editor rather
than looked at a render of it:

> *"a lot of placements of objects overlap on the top floor, and the entire
> area has a ceiling that also drops down super low. we need to fix some of
> the height issues on that upper floor, the lower floor has some paths
> blocked by clothes and obsturcted, some look intentional, some overlap. so
> overall, also, the lighting is still to bright and not emergency looking."*

Three complaints. Each one turned into a number before anything was changed,
and two of the three numbers were not what anybody expected.

---

## 1. The lighting: it was never on emergency power

`ULLKPowerSubsystem` is explicit, in Chris's own words at the top of its
header: *"Emergency is the default and always will be."* It finds lights by
tag — `LLK_Mains` goes dark when the breaker is off, `LLK_Emergency` stays
lit — and the header argues well for tags over a list.

What nothing said out loud is the other half of that rule:

> **An untagged light is on in both states.** `ApplyToLights()` only ever
> touches actors carrying `LLK_Mains`. A light with no tag is never switched,
> never counted, and never reported. It simply burns.

`Tools/audit_boswells_clearance_and_mood.py` read every light below the
mezzanine deck. 138 of them, **79 lit at boot**. Exactly 22 carried
`LLK_Emergency`, at **1.3 to 2.8 candela**. Here is what was actually lighting
the room:

| Family | n | cd | circuit |
| --- | --- | --- | --- |
| `LLK_DOCK_CHAPTER_WING_POOL_*` | 13 | **31–46** | none |
| `BSW_EGRESS_LOWER_WING/MAIN_*` | 17 | **34** | none |
| `GALLERIA_NEAR_ROUTE_LIGHT_*` | 10 | 19 | none |
| `BSW_DEPT_ROUTE_POOL_*` | 7 | 28 | none |
| `BSW_ESC_DETAIL_*_ROUTE` | 4 | 23 | none |
| `BSW_OPENING_ENTRY_POOL_*` | 2 | **55** | none |
| `BSW_EMERGENCY_FILL_*`, `BSW_EASTFILL_*` | 22 | **1.3–2.8** | `LLK_Emergency` |
| `BSW_MAINS_*` | 27 | 26 | `LLK_Mains`, **off** |

**The emergency state was brighter than the mains state.** The mains sit at 26
candela and were dark; the floor was lit instead by about fifty untagged pools
at 19 to 55, and the layer actually called emergency contributed roughly one
twentieth of the light in the room. Chris's *"too bright and not emergency
looking"* is a precise description of that table.

It also means **the breaker barely did anything**. `ROADMAP.md` goal 5 is
*"find a story clue, change part of the building, and reach Sayers"*, and
restoring power is that beat in its most literal form. Both states looked
nearly the same, because the lights that lit the room were not on the circuit
the breaker moves.

This is the shape this project keeps producing, and
`Tools/verify_boswells_ground_cells.py` named it in its own header a day
earlier: **a correct system over an absent thing, with a clean log.** The
subsystem is right. Every tool that ever placed a light is right on its own
terms. Nothing asked whether the two agreed — and `OnWorldBeginPlay` reported
*"Boswell's is on emergency power: 22 emergency light(s) lit, 27 mains
light(s) dark"* in a room lit by fifty lights it had never heard of.

### What changed

`Tools/repair_boswells_emergency_state.py`. **Every light inside the building
now belongs to exactly one circuit**, classified by what the light is for and
printed before it is applied: the egress route is emergency, selling light is
mains, scripted staging and the loading dock are left alone, and **anything
the tool cannot place is reported as unclassified and not touched**. 49 joined
the mains and went dark; 61 joined the emergency layer.

A mains light is now **authored** off in the saved map as well as switched off
at boot, because every picture either of us ever sees of this game is a render
taken without PIE.

**The emergency layer stopped being orange.** It was `rgb(232, 178, 150)` —
the colour of a living-room lamp — and no amount of dimming makes orange read
as emergency, because the association a player brings is a table lamp. It is
now the cold, slightly green white of a battery pack, `rgb(158, 189, 179)`.

**No intensity was touched.** The 12-to-26× ratio between the egress pools and
the fill is already the pools-and-dark shape an emergency floor wants; it was
buried under fifty untagged lights. Removing those is one change, and the
measurement comes after it rather than beside it.

| measured on the saved map | before | after |
| --- | --- | --- |
| lights lit at boot | 79 | **55** |
| peak floor brightness | 0.0013 | **0.0005** (−61%) |
| p99 floor brightness | 0.0005 | 0.0003 |
| share of lit lights on a circuit | 28% | **96%** |

## 2. The upper floor: it is still wearing its own blockout

Seven actors — `UPPER_TOYS`, `UPPER_KIDS`, `UPPER_BEDDING`, `UPPER_SEASONAL`,
`UPPER_CURTAINS`, `UPPER_FURNITURE_EAST`, `UPPER_HOME_EAST` — stand on the
mezzanine deck. Every one is an **engine `Cube`** on a default material, 300 ×
600 × 105 cm or so, 152 m² of footprint between them.

They are not architecture, and this repo already worked that out twice.
`Tools/ceiling_boswells_aisle_soffit.py` wrote it down — *"merchandise
standing on the mezzanine deck"* — and `assets/models/boswells/place_fixtures.py`
is blunter: `UPPER_BEDDING` and `UPPER_SEASONAL` *"predate the department"*.

Then the departments were built, **on top of them**. `BSW_DEPARTMENT_*` put
toy gondolas, bed vignettes, sofa vignettes and home sideboards on that deck;
`BSW_LEFTDAY_DEPARTMENT_*` added the after-close dressing; 72
`BSW_CIRCULATION_UPPER_STOCK_*` units went in. Nobody removed the cubes, so a
7 m pale slab stands inside the home department with the real sideboards
showing through it. It is the single most visible thing in any render of that
storey, which is why Chris led with it.

**Hidden and decollided, not deleted.**
`docs/FINDINGS-KIDS-FLOOR-STEALTH.md` counts them as cover, so the cover they
were providing is measured and printed, and one line brings them back if that
measurement turns out to matter. Deleting a thing another document depends on
because a render looks wrong is how a project loses an argument it has already
had.

Overlapping pairs on the deck: **326 → 184.**

## 3. The ceiling complaint was the opposite of what it sounded like

*"The entire area has a ceiling that also drops down super low."* The first
measurement agreed — 11% of the deck under 200 cm, some of it at 13 cm — and
**that measurement was wrong.** The trace started at `MEZZ_DECK + 10`, which
is *inside* the furniture, so the first thing it hit going up was the top face
of a sofa. Corrected to start above the props, the deck reads:

| | |
| --- | --- |
| lowest headroom | **375 cm** |
| median | **838 cm** |
| under 200 cm | **0%** |

So nothing on that floor is low. What is true is stranger: **the upper storey
has no ceiling height at all.**

- **89%** of the deck is open to `ANCHOR_FULL_ROOF`, **838 cm** overhead
- **10%** is under `BSW_SERVICE_CORE_SERVICE_CEILING` at **375 cm**
- the kids department has a tile field at **448 cm** — with **no collision**,
  so no trace finds it and nothing can be stopped by it

Three heights and a void, on one floor, with no datum anywhere. A real
anchor's upper selling floor is 350–450 cm. **This is left for Chris**,
because picking the height of a storey is not a repair script's decision.

One piece of it was unambiguous and is fixed: the kids ceiling light housings
hung at 935, **163 cm below the ceiling they belong to**. They now hang 26 cm
under it, which is the drop this project already uses everywhere else.

## 4. The clothes in the aisles, and the theory that was wrong

Measured between every pair of facing department cells: **12 walkways a 68 cm
body cannot pass at all**, 5 to 27 cm of clear floor where 167 was drafted.

Chris's own distinction is the one that matters — *"some look intentional,
some overlap."* A department with no way through on one side is a design this
floor already defends;
`docs/plans/BOSWELLS-WALKWAY-AND-CEILING-RULES.md` settles it in his words,
*"inside a department there is no way past a Sweeper, and that is the
design"*, with the perimeter as the way out. **A closed gap is not
automatically a fault**, and a tool that opened all twelve would be undoing a
decision rather than fixing a defect.

So the repair moved a fixture only where it was **both** standing inside
another rack **and** outside its own department cell, and moved it back onto
its own carpet.

**That theory was mostly wrong, and the number says so.** 271 such pairs; 10
moved; **52 moves were tried and put back because they cleared nothing**; 67
were already inside their cell. Net: **271 → 261**. The intrusions are not the
cause. The cause is that the dressing *inside* a cell is overpacked —
successive passes put racks at the same coordinates — and that is a different
job with a different shape.

**Two measurement errors of my own are recorded in the tools rather than
quietly fixed.** The first version counted a rack's own parts as overlapping
themselves: `dress_boswells_juniors.py` builds a rail from an UPRIGHT, a FOOT,
a BAR and a row of GARMENTs at one spot, so every pair of those overlaps by
construction. That gave "380 overlapping pairs", and a repair aimed at it
**moved 64 fixtures an average of 85 cm — half an aisle width — to remove four
pairs.** The map was reverted and both tools now carry a notion of which
assembly a piece belongs to. The second was the headroom trace above.

## What is measured and deliberately not fixed

**The upper storey's ceiling height.** Three heights and a void, above.
Chris's to pick; the numbers are attached.

**261 ground-floor fixtures standing inside each other.** Real, counted, and
not an aisle problem. Needs a dressing pass that knows what is already in a
cell, not a nudge.

**11 walkways still closed to a 68 cm body.** Left closed on purpose, because
this floor's own rules say a department is a trap and the perimeter is the way
out. Named individually in the audit so a decision can be made per aisle
rather than for all of them.

**The emergency housings are still the brightest thing in frame.** A fitting
renders near white while the pool it casts is a tenth of that, so it reads as
a bulb pasted on the ceiling. Measured by eye in `f_aisle.png`, not yet by a
tool.

**Eight `LLK_DOCK_CHAPTER_*` lights are still on no circuit**, at 18–29 cd.
They are outside the sales plate in the dock chapter's own staging, so the
repair reports them and leaves them; if the dock is meant to go dark with the
store, they need tagging.

**Nobody has played any of it.** Every number here is a measurement against a
saved map, and every picture is a render.

---

**Answered 2026-11-23: Chris said "Build it at 400."** The storey now has one
datum at 400 cm, and building it turned up a second copy of the mains and
emergency grid hanging in the roof space. See
`docs/FINDINGS-THE-UPPER-STOREY-HAD-NO-HEIGHT.md`.

---

**Source identified 2026-11-25.** The fifty untagged lights had an author, and
it is `Tools/rehabilitate_boswells_emergency_lighting.py` — tracked in
`bc380d7` with its uncommitted changes, after Chris asked for the loose tools
to be committed.

Compare its values with the table in section 1:

| family | this document measured | that tool sets |
| --- | --- | --- |
| `BSW_EGRESS_*` | 34 cd, no circuit | **34.0** |
| `BSW_DEPT_ROUTE_POOL_*` | 28 cd, no circuit | **28.0** |
| `BSW_ESC_DETAIL_*_ROUTE` | 23 cd, no circuit | **23.0** |
| `GALLERIA_NEAR_ROUTE_LIGHT_*` | 19 cd, no circuit | **19.0** |
| `BSW_OPENING_ENTRY_POOL_*` | 55 cd, no circuit | **55.0** |

Five families, five exact matches. **Chris's "the lighting is still too bright
and not emergency looking" was a description of that tool's output**, and it
sets no `LLK_Mains` or `LLK_Emergency` tag on anything it touches, which is why
every one of those lights burned in both power states and why the breaker
barely did anything.

It was run once on 2026-11-25, deliberately and reversibly, because Chris asked
to see both lightings side by side. The map was clean at `a3610f7` before and
was restored with `git checkout` after; the same camera measured 27.32 before,
30.91 under that tool, and 27.31 after the restore.

What the pair shows, at the escalator looking south down the store:

| the floor the keeper walks | current | that tool |
| --- | --- | --- |
| mean luminance | 67.5 | **84.9** |
| p90 | 114.1 | **143.5** |
| colour | (56, 72, 61) cold green-white | (87, 85, 74) warm cream |
| saturation | 0.221 | 0.146 |

**It is 26% brighter and it is the wrong colour.** The warm cream is the
`SERVICE` colour that tool authors, `(226, 214, 194)` — a corridor in a shop
that is open. The current floor is the battery-pack white this document chose,
and it reads as a building whose power is off.

That is not an alternative direction. It is the state this whole pass replaced.

# The Anchor's plate is right and its circulation is closed

**Status: AUDIT, 2026-11-07.** Written on Codex's request, relayed by Chris:
*"Claude's most useful task would be a 'plan audit,' not another lighting or
AI pass: review the current `docs/reference/boswells/README.md`, Grok's
research, and the new Anchor map; then produce one authoritative full-store
block plan with dimensions, vertical circulation, entrances, departments,
service ring, and extraction."*

This is that, in two parts. Part A is what `Content/LLK_Boswells_Anchor.umap`
actually contains, measured. Part B is the block plan, which is a **proposal**
and not a `DECISIONS.md` row. Nothing here resizes `LLK_Boswells_FirstRun`,
and nothing here authors a foot.

**It does not unblock extraction.** Chris's own priority this session was
*"just getting the raid to work is good, home can wait."* The two findings
that most likely explain what he played — D1 and D2 below — are transform
errors, and both are fixed by moving actors rather than by having a plan.
The plan is for after that.

## Phase 1 has landed, 2026-11-09

Chris said *"fix the map."* `Tools/fix_anchor_map.py`, run headless through
`Tools/run_editor_python.ps1` with no editor open, applied **B7 step 1** and
saved: **22 re-seated, 2 moved, 6 deleted, 1 added.** The map went from 108
actors to 103.

| Defect | State |
| --- | --- |
| D1 every box floating | **closed** — all 22 now sit on their plate |
| D2 the ramp into a solid slab | **closed by deletion** — B2 puts the well in the atrium |
| D3 guardrails off the deck | **closed** — deleted with the ramp |
| D5 two relics on one transform | **closed** — the radio moved to the teens block |
| D6 patrol on the dock approach | **closed** — moved to (−1400, 2100) |
| D7 no exit signage | **opened** — one red `EXIT` over the dock |
| D8 three `_TEST` actors | **closed** — deleted |
| D4, D9–D13 | **open** — phases 2 to 4, and B8 is Chris's |

Verified three ways rather than once: the script logged each change, a second
run reported `0 re-seated, 0 moved, 0 deleted` against the same map, and
`Tools/read_map_transforms.py` decoded the saved file independently and
agreed to the centimetre on all 22. `npm test` is 90/90.

Two consequences of this pass worth stating plainly:

- **`DEPT_LOADING_APPROACH`'s top is now z 40**, under the 45 cm step, so the
  straight line from spawn to the exit is walkable for the first time.
- **The upper floor has no route at all** until the escalator exists, which
  it did not have before either — the ramp stopped a body at about z 461.
  `ANCHOR_RELIC_TEDDY` is still up there and still unreachable. That is B7
  step 2, not a regression.

**The map is still untracked.** `Content/LLK_Boswells_Anchor.umap` has never
been committed, so none of this is in version control and the only copy of
the pre-fix file is a backup outside the repository. Whether that asset gets
committed is Chris's call under row 24 and `LANES.md:108`.

## How this was read, and what it cannot say

The map was not opened. Every number in Part A comes from decoding the
transform records in the `.umap`'s own text payload: for each actor, the
quaternion, translation and 3D scale, read as ten doubles after the class
name and its guid. 108 actors, all of them. Heights below are
`scale x 100 cm`, because every mesh in this map is
`/Engine/BasicShapes/Cube`, a 100 cm cube with its origin at its centre.

Two limits, stated because `FINDINGS-BOSWELLS-GEOMETRY.md` earned them:

- **Nothing here is traced.** The geometry findings' own trap is that bounds
  are not collision. These are authored transforms, which is a third thing
  again — it is what the level says, not what a probe hits. Every claim below
  that a body cannot pass assumes `/Engine/BasicShapes/Cube` carries its
  default collision. That is the assumption to kill first, and the way to
  kill it is a trace up the ramp in the editor.
- **Component-level overrides are not in this payload.** A property set on a
  component rather than the actor would not show. The one that matters is the
  extraction zone's `ExtractionPointId`: the name `LoadingDock` is in the
  map's name table exactly once, which is consistent with the zone being
  wired to it and is not proof.

## Part A — the Anchor, measured

### The shell

| | |
| --- | --- |
| Lower floor plate | **8700 x 5400**, top at **z 0**. 4,698 m², 50,570 sq ft |
| Walls | 40 thick, **z 0 to 1500**, at x ±4350 and y ±2700. **All four solid** |
| Roof | z 1487.5 to 1512.5 |
| Upper floor | four slabs, 25 thick, top **z 662.5**, underside 637.5 |
| Upper floor, as a union | the full plate **minus a 1200 x 1800 void at the centre**. 4,482 m² walkable |
| Both plates | 9,180 m², **98,813 sq ft** |
| Lower headroom | **637.5** |
| Upper headroom | **825** |
| Atrium posts | 30 x 30 x 650 at (±650, ±950), z 650 to 1300 |
| Nav bounds | 9000 x 5800 x 3600 about (0, 0, 700). Covers both plates |

The four upper slabs are north y 900…2700, south y −2700…−900, west
x −4350…−600 and east x 600…4350, the last two spanning y −900…+900. Their
union leaves exactly one hole, 1200 x 1800, centred on the origin.

**98,813 sq ft is the defensible one of the three published figures.** It
sits beside section 12's secondary 101,300 and effectively on top of section
10's measured 98,778. It does not reach the Tribune-Democrat's 160,000, and
section 12 already says the two areas stay in conflict. Whoever sized this
plate picked the figure that can be reached by two plates, which is right.

### The route, the crew and the exit

| | |
| --- | --- |
| Spawn | `ANCHOR_PLAYERSTART_ENTRANCE` (0, **−2350**, 120) |
| Exit | `ANCHOR_LOADING_DOCK_EXIT`, `LLKExtractionZone` (0, **+2475**, 110) |
| Straight line between them | **4825 cm**, down x = 0 |
| Sweeper | (300, 250, 110) — on that line |
| Patrol | (−2900, 300), **(0, 2100)**, (2800, 250) |
| Receiving | `ANCHOR_STOCK_BAY_RECEIVING` (3300, 2100) |
| Relics | 4 actors, **3 distinct positions**, one of them upstairs |

Spawn south, exit north, one axis, 48 m. That is the clearest readable
intent in the map and it is worth keeping.

### Light as placed

Twelve lower spots at z 600, housings at 615, on a grid: x ∈ {−3000, −1000,
+1000, +3000} by y ∈ {−1600, 0, +1600}. Eight upper spots at z 1450,
housings 1465, x the same four by y ∈ {−1800, +1800}. Nine department
`TextRenderActor` signs. Three actors still named `_TEST`.

## Part A, continued — thirteen defects

### D1. Every merchandise box floats at half its own height

The rule the map was authored with is `z = plate + 100 x scaleZ`. The rule it
needed is `z = plate + 50 x scaleZ`, because the cube's origin is its centre.
**22 actors**, every `DEPT_*` and `UPPER_*` box.

The floor, the four walls, the roof, the four upper slabs and the atrium
posts are all seated correctly, so the convention was known. This is an
arithmetic slip repeated down a list, not a misunderstanding.

Two consequences, and the second is the one that cost Chris a night.

**It inverts the sightline.** `DEPT_LADIES_WEST` is 105 tall. Seated as
authored, its top is at **157.5** — above the 152 cm keeper eye
(`LLKLanternFeel.cpp:7`), so a keeper sees over nothing on this floor.
Seated correctly its top is at 105, which is **below** the 133 cm resting
lantern, so the lantern clears it too and the read is lost the other way.
Neither reading is the 137 the block-in authored precisely so that an eye
clears a run and a lantern does not
(`docs/reference/boswells/README.md`, and `zmax_cm 137.0` in the CSV).
So the height is wrong independently of the seating.

**It puts a ledge across the exit.** `DEPT_LOADING_APPROACH` is 1200 x 220 x
40 at (0, 2350). Seated as authored its top is at **z 60**; `MaxStepHeight`
is UE's default **45** (nothing in `Source/` assigns it; `THE-HARD-RUN.md:138`
records the 45). So the one straight line from spawn to the extraction zone
ends, 125 cm short of the zone, in a 12 m wide ledge the player must jump —
with no prompt, in the dark, at the exit. Seated correctly its top is at 40,
under the step, and it is a floor you walk across without noticing.

### D2. The upstairs route is closed, and a scaled cube cannot be given a hole

`ANCHOR_WEST_RAMP` is a 2090 x 800 x 25 deck pitched **18°** about
(−3200, −1200, 325). Its ends are at z 2.1 and z 647.9, so as a ramp it is
correctly built: it rises 646 cm and arrives 14.6 cm under the upper floor.

It arrives there **underneath `ANCHOR_UPPER_SOUTH`**, which is one scaled
cube, 8700 x 1800 x 25, with no opening anywhere in it.

The deck's walking surface reaches the slab's underside 921 cm horizontally
from the ramp's centre. A 176 cm body's head reaches it **542 cm earlier**,
at a deck height of about **z 461**. So a keeper climbs the ramp to roughly
4.6 m, stops, and has 6.4 m of ramp left running inside the second floor.

Codex already named this in his own words — the upstairs route was *"an
unproven greybox ramp"* — and this is the specific way it fails.

### D3. The ramp's guardrails are 430 cm off the deck, on both sides

`ANCHOR_WEST_RAMP_GUARD_A` and `_B` carry the same 18° pitch and the same
2090 length, so they were built for this deck. They sit at y −2030 and
y −370. The deck's edges are at y −1600 and y −800. Both rails are 430 cm
clear of the surface they guard, symmetrically, as though the deck were
1660 wide instead of 800. The deck itself is open on both sides at up to
6.5 m.

### D4. The atrium has no balustrade

4,482 m² of walkable upper ring, one 1200 x 1800 hole in the middle of it,
and four 30 cm decorative posts at its corners. Nothing else on the inner
edge.

### D5. Two relics share one transform, and a third is in the closed half

`ANCHOR_RELIC_PERFUME_LEDGER` and `ANCHOR_RELIC_RADIO` are both at
**(−2700, −1050, 140)**, to the centimetre. `ANCHOR_RELIC_TEDDY` is at
(2450, 1500, 790), on the upper floor, which D2 makes unreachable.

Four relic actors, three distinct positions, **two reachable**. In a run
whose objective line is *"Find something worth carrying"*, that is the
supply of things worth carrying.

### D6. A patrol point sits on the approach to the exit

`ANCHOR_PATROL_2` is at (0, 2100): on the spawn-to-exit axis, 375 cm from the
extraction zone, immediately south of the dock platform. `make_salesfloor.py`
refuses a solid in the patrol spine and a solid in the dock trigger for
exactly this reason; **nothing checks this map at all.**

### D7. There is no exit signage in the building

Nine department signs. Zero exit signs. This is the same gap as the missing
compass, in architecture instead of in the HUD — and the lighting research
delivered this session already says the exit sign is the one legitimately red
thing in an emergency-lit building. The wayfinding Chris could not find is a
fixture type the plan has never placed.

### D8. Three test actors are in the level

`ANCHOR_EMERGENCY_LIGHT_TEST`, `ANCHOR_NAV_SPOT_TEST`, `ANCHOR_SIGN_TEST`.

### D9. The light is a grid, which is what reads as "on at night"

Lower fixtures are 2000 cm apart in x and 1600 in y; upper fixtures 2000 by
3600. The spacing relates to nothing — not a department, not a threshold, not
the route. A regular grid at regular intensity is a uniform floor, and a
uniform floor reads as working lighting at night whatever colour it is. That
is the complaint, and the cause is the layout rather than the temperature.

The east and west upper wings — x ±600…±4350 by y −900…+900, **1,350 m²** —
have no fixture at all, and also no merchandise. The balcony is empty and
unlit.

### D10. There is 8.25 m of unexplained air over the second selling floor

Lower headroom 637.5, upper headroom **825**, and no office plate in the map.
Sections 12 and 13 both put an office level above the two selling floors, on
the elevator's third stop. The arching ceiling belongs to the mall concourse;
section 12's falsifier says the anchor does not borrow it. So this volume is
either the office level, unbuilt, or it is nothing.

### D11. The Anchor is three times sparser than the room that already felt empty

| | Fixture area | Plate | Coverage |
| --- | --- | --- | --- |
| Anchor, lower | 175.9 m² | 4,698 m² | **3.7%** |
| Anchor, upper | 152.0 m² | 4,482 m² | **3.4%** |
| `BoswellsSalesFloor_Fixtures.csv` | 55.5 m² | 504 m² | **11.0%** |

The block-in is the floor Chris has already walked, and his verdict on the
night was that the floorplan was off. The Anchor is **three times less
furnished than that**. Nine boxes at 3.7% on a 87 x 54 m plate is not a
sparse store; it is a hangar with crates in it, and no amount of lighting
fixes a room with nothing in it to cast a shadow.

### D12. Three departments are on the wrong floor and five are missing

Section 13 is Chris's own witness and section 12 defers to it.

- **`DEPT_HOME_PREVIEW` is downstairs.** Home is in his upper list.
- **`DEPT_ELECTRONICS` is a department he never named.** The CRTs are a prop
  in the teens area on the lower floor — section 13 is explicit that they do
  not even join `LLKLightField`. A department was built out of a prop.
- **`DEPT_SERVICE_DESK` is downstairs.** The courtesy desk is on the second
  floor in the 2019 WJAC piece and in section 13.
- **Missing upstairs:** candy, the services cluster (hearing, optical,
  restrooms, gift wrapping), babies, appliances, secret saver.

Candy is the omission that matters. It is the only room in this building with
three independent witnesses — Buda in 2022, the Remembering Retail caption in
2026, and Chris — and it is the thing he remembers walking upstairs *to*. The
upper floor has no landmark because its one landmark was not placed.

`DEPT_COSMETICS_COUNTER` is perfume under another name. Chris said perfume.

### D13. The plate's area agrees with the evidence and its shape does not

Section 10's measured directory box is **200.55 ft by 246.27 ft**, a ratio of
**1:1.23**. The authored plate is 8700 x 5400, a ratio of **1.61:1**. The
areas agree within 2.4%; the proportions do not.

This is the weakest finding here, and it is listed last for that reason:
section 10 says in so many words that the anchor boxes in that model are
simplified, and that no centimetre may come from the glTF. So the ratio is
soft evidence. But it is the only shape evidence in the record, and a
near-square plate puts the escalator well in the middle of a room you can
read from any corner, where a 1.6:1 plate makes a long hall. **Do not rebuild
the plate on the glTF.** Resolve it against TLP-1's 100 ft bar once that sheet
is scaled in the editor, which section 8 step 2 already asks for.

---

## Part B — the block plan

Proposed. Centimetres, Unreal axes, Z up, origin at the plate's centre.
"South" is −Y and "north" is +Y in this level's own frame; which real end
faces the concourse is still Chris's call, per section 12 item 7.

### B1. Dimensions

Keep the plate at **8700 x 5400** until the 100 ft bar settles D13. Two
selling plates, one office plate, one roof, with the two selling floors given
**identical headroom** — which is the language a two-floor department store
reads in, and which the map currently breaks by 188 cm.

| Plate | Slab top | Underside | Headroom below |
| --- | --- | --- | --- |
| Lower selling | z 0 | — | **637.5** |
| Upper selling | z 662.5 | 637.5 | **637.5** |
| Office (shell) | z 1325 | 1300 | 300 |
| Roof | z 1650 | 1625 | — |

Walls become 1650 tall: `scaleZ` 15 → **16.5**, centre z 750 → **825**. Every
other shell transform in the map is already correct and does not move.

### B2. Vertical circulation — the hole you already cut is the one the escalator wants

Sections 12 and 13 agree, and Chris's memory is the deciding witness: **one
centre criss-cross escalator, two runs, one well**, plus one elevator with
three stops. The map has neither.

The atrium void is 1200 x 1800. A 30° escalator rising 662.5 cm has a
**1147.5 cm** run; with a 244 cm landing plate at each end its structural
length is **1635.5 cm**. The void is 1800 long. **It fits, with 164.5 cm to
spare.** Two 1000 mm-step units side by side occupy about 350 cm of the
void's 1200, leaving the rest for the balustrade and the landing walk. At 35°
the run is 946 and the structural length 1434, which fits with room over.

So the well Chris remembers drops into the hole that is already there, needing
no new cut. The west ramp, by contrast, needs a **second** hole about
900 x 850 cm, which means splitting `ANCHOR_UPPER_SOUTH` into three cubes to
have one at all.

**Delete `ANCHOR_WEST_RAMP` and both guards. The atrium is the well.** That
closes D2, D3 and half of D4 in three deletions, and it moves the map toward
Chris's witness instead of away from it.

The elevator is a **250 x 250 shaft with three stops**, in the back of house
beside receiving — not in the atrium. Section 10's glass clock-tower car is
the mall's machine; this is the anchor's, a Schindler hydraulic, one car
witnessed, model number disputed. The third stop is what the office plate is
for.

Both ends of the well take an emergency light, which is the rule the concourse
already uses.

### B3. Entrances

Two openings in an otherwise solid shell. Row 28 does not move: the grilles
stay down.

- **The mall face**, south wall, centred on x 0 where the spawn already
  stands: a closed grille **1800 wide**, header underside at z 280. It is the
  face you can see the concourse through and cannot use. The player arrives
  in front of it and the building's whole axis runs away from it.
- **The dock**, north wall, centred on x 0 where the dock door, the approach
  platform and the extraction zone already are.

Everything else stays sealed. One grille, one dock, 48 m between them: the
building teaches its own route by having exactly one.

### B4. Departments

Tags on the correct plate. **No bay widths** — section 13's own floor rule is
that a drafted width is a filled invention.

**Lower selling floor**, in Chris's order, read from the grille: perfume and
jewelry on the entrance counters (they already sit at y −2050, which is
right), then juniors — carrying the teens area and the CRT sets, which are
props and not a department — then mens, ladies, shoes. Checkouts at the
grille end.

Deletions: `DEPT_ELECTRONICS`, `DEPT_HOME_PREVIEW`. Rename
`DEPT_COSMETICS_COUNTER` to `DEPT_PERFUME`. Move `DEPT_SERVICE_DESK` upstairs.

**Upper selling floor**, from the head of the well: **candy at the escalator
head** — it is the landmark and it is the one upper room three witnesses
agree on — then the services cluster (hearing, optical, restrooms, gift
wrapping, courtesy desk), then kids, babies, toys, seasonal, bedding,
curtains, home, secret saver, appliances, furniture.

The east and west upper wings are where the missing five go. They are empty
now, and they are also the 1,350 m² with no light.

### B5. Service ring

**It is not a ring, and the plan should stop calling it one.** Section 8 step
6 is explicit: TLP-1 draws service rooms at the anchors and does not draw a
continuous back corridor, so the ring stays a proposal until a sheet shows
the wall.

Inside this anchor it is a **back-of-house strip along the north wall,
y 2100 to 2680, full width**, walled off from the selling floor with two
doors. It holds the dock mouth, receiving (already tagged at 3300, 2100), the
stockroom, and the elevator shaft.

This earns its place in play rather than in fiction: it turns the dock from
open floor into a **threshold**, which is a thing the readout can name and a
thing a light can mark. And it is where `FreightLift`'s head goes when the
sub-basement exists.

### B6. Extraction

One zone, `LoadingDock`, at (0, 2475). It stays exactly where it is.

- **Do not place a `FreightLift` zone in this map yet.** Its gate,
  `SubBasementBreakerRestored`, can never be set in play (M14), and the
  nearest-exit search in `LLKDebugHUD` has no gate filter. That was latent
  while the readout was silent. As of `80849f8` the readout speaks, so a
  second zone would make the HUD name a door that cannot open — and a
  confident wrong instruction is worse than the silence Chris spent three
  sessions complaining about. One line in the HUD's search loop comes first.
- **Move `ANCHOR_PATROL_2` off the approach**, per D6.
- **Port the block-in's own checks to this plate.** `make_salesfloor.py`
  already refuses a solid in the patrol spine, a solid in the dock trigger,
  and a wall slot narrower than a 68 cm Sweeper, and then walks a 68 cm disc
  from the door to every aisle, to each patrol point, and to the dock mouth.
  Nothing checks the Anchor. That script is the cheapest guard in the project
  and it is pointed at the smaller room.

### B7. The order, against Chris's priority

He said it plainly: *"just getting the raid to work is good, home can wait."*

1. **The raid.** D1 (re-seat 22 boxes), D5 (separate the relics), D6 (move the
   patrol), D8 (delete three test actors), D2 and D3 **by deleting the ramp
   and its guards**, D7 (exit signage over the dock and at each threshold on
   the route). Every item is a transform or a deletion. No new geometry. This
   is the pass that makes the night playable, and on its own it fixes the
   ledge at the exit, the two unreachable relics and the absent wayfinding.
2. **The well and the strip.** Escalator into the atrium, balustrade on the
   ring, back-of-house wall and its two doors.
3. **Density and light.** Fixtures to 137 and coverage to at least the
   block-in's 11%; fixtures off the grid and onto the grille, the department
   thresholds, the well's head and foot, the back-of-house door and the dock.
4. **The office plate and the elevator.** Shell. Unshopped. The third stop.

### B8. Three calls that are Chris's, not mine

- **Fixture height.** 137, as the block-in authored it, or something else. The
  whole standing-versus-crouched read hangs on one number and D1 shows the
  Anchor currently carries neither value.
- **Which real end is the concourse face**, still open from section 12 item 7.
- **Whether the plate stays 1.61:1** or goes to the measured 1:1.23 (D13).
  This one should wait for the 100 ft bar.

### Turned over

**Operand.** This plan, against the Anchor as placed, and against keeping
`FirstRun` as the only room anyone tests in.

**Floor.** A plate whose area matches a published figure is a plate. A route
that ends inside a slab is not circulation. A relic you cannot reach is not
loot. A department with no wall position is a tag; a department invented out
of a prop is not even that.

**Clock.** The audit's clock is the decode: 108 actors, every transform.
Nothing in Part A is dated by a web source and nothing in it is a trace.
Part B's clock is still the sitting in Boswell's.

**Falsifier.** D2 falls if a trace in the editor walks a capsule from the
ramp's foot to the upper floor — which would mean the cube's collision is not
what is assumed, and would be worth knowing on its own. D1 falls if
`/Engine/BasicShapes/Cube` is not origin-centred, which the floor, walls,
roof and upper slabs in this same map all contradict. D11 falls if a real
1990s department store floor runs near 4% fixture coverage. B2 falls if a
counted frame shows two separated escalator wells, which reopens section 12's
question and still does not import another store's plan. B4 falls on a 1990s
directory or photograph that puts mens or shoes upstairs, or toys down.

**Upside down.** The expensive half of this map is correct and the cheap half
is what broke it. The plate is the right size, the area matches the one
reachable published figure, the atrium is cut in the right place and at very
nearly the right size for the escalator nobody built, and the spawn and the
exit are 48 m apart on a single readable axis. What stopped the night was an
arithmetic slip repeated down a list of 22 actors, and a ramp that rises
correctly into a floor with no hole in it. A plan audit was the right ask and
the wrong diagnosis: the Anchor does not need a new plan to be walked. It
needs the one it has to be seated on the floor.

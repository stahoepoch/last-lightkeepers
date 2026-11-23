# The ceiling was lying about the floor

**2026-11-23.** Chris, home: *"hit me some big shit, make sure the lighting is
right, i need this to be kick ass."*

The ground floor was rebuilt the same morning — 25 authored cells, 165 cm
aisles, four mirrored coffers, sixteen glass cases round the well — and **no
light moved.** Every fixture on this floor was positioned for a layout that
stopped existing before lunch, and nothing in the repository had checked the
two against each other. `audit_boswells_opening_lights.py` prints an
inventory, and an inventory is a census;
`docs/FINDINGS-THE-LIGHTS-WERE-IN-THE-ATTIC.md` is this project's record of
what a census misses.

So the floor was measured and then photographed, and the two methods were made
to argue. **They disagreed twice, and the photographs won both times.** That is
the most useful thing in this document.

---

## The method

`Tools/audit_boswells_ground_lighting.py` samples 13,426 points of clear floor
at a **124 cm eye** — a nine-year-old, decision row 90 — and sums
`E = I / d²` from every light that is not behind a ceiling, with spot cones and
a Lambert term at the sample. It is deliberately optimistic: no occlusion by
fixtures, no bounce, and **no lantern**, because `ROADMAP.md` slice 2 requires
the chapter to remain navigable with the lantern off and that had never been
tested at floor level.

`Tools/shoot_map.ps1` then photographed four places at the same 124 cm, in both
power states: down the main spine, along the east wall, inside a 165 cm aisle
in ladieswear, and under a mirrored coffer.

## Where the measurement was wrong

**The mains are not too dim.** The audit found mains-on gives a median 0.84 lux
at the eye and a maximum of 6.1, against the 300–500 lux a working department
store runs at, and concluded the payoff was short by two orders of magnitude.
The arithmetic is right. The conclusion is wrong, because
`r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange=0` is pinned in
`Config/DefaultEngine.ini` and the map's `BSW_EXPOSURE_LOCK` pins min and max
brightness together at 0.5. Screen brightness is a **fixed** mapping of scene
luminance here, not a physical one, and the photograph of the mains state is a
fully, evenly lit room. Acting on the lux number would have blown it out.

**The mirrored coffers are not broken.** They read as black slabs hanging in a
lit ceiling, which looks exactly like a missing material. With the mains on
they reflect the ceiling correctly. They are black because a mirror shows the
room and the room is dark. Nothing was wrong; a fix was one edit away.

Both near-misses have the same shape, and it is worth naming: **a number
without a picture is a hypothesis.** This project can now take the picture in
about a minute per frame, so there is no longer an excuse for shipping the
hypothesis.

## The three faults that survived both

### 1. The way in was invisible

| route, lantern off | lit at a 124 cm eye | longest dark run |
| --- | --- | --- |
| west perimeter | 100% | 0 cm |
| **east perimeter** | **0%** | **4,590 cm** |
| front walk | 89% | 950 cm |
| back-of-house | 90% | 850 cm |
| main spine, both sides | 100% | 0 cm |

`verify_boswells_ground_cells.py` puts the player in at (3200, 2000), inside
the east service door. **The entire 45 m of wall they then walk is absolute
black**, and the photograph from (4200, 1700) is a lit ceiling over an
invisible floor. The west wall, built by the same pass, is lit end to end.

**The first repair was wrong, and the way it was wrong is the point.** The
asymmetry looked like the fault, so the fix mirrored the live lights near the
west wall across to the east — two walls of one building ought to agree.
Thirteen lights and thirteen fittings went in. The re-render shows the new
troffers in the ceiling and **the floor still black.** Copying the lit wall
copied a symptom.

The cause is one line of arithmetic. `BSW_EMERGENCY_FILL_*` stands on columns
at x = **−3600, −2100, −600, 900, 2400**: a step of 1500, started at the west
edge, run five times and stopped. The plate runs to **4350**, so the first
column stands 750 cm off its wall and the last stands **1950 cm** off its own.
The grid did not fail — it ran out. Nobody chose the dark east wall, and nobody
chose the lit west one either; they are the two ends of a loop.

So the repair continues the grid's own step until the next column would leave
the plate: **one more at x = 3900**, 450 cm off the east wall, four lights in
the existing rows. Each copies the light in its own row, because the fill is
authored to fade with distance from the mall entrance glass and a column that
ignored that would make the back of the store brighter than the front. The
step, the rows, the intensities and the heights are all read off the map.
Nothing is invented. Four lights:

| route, lantern off | before | after |
| --- | --- | --- |
| west perimeter | 100% | 100% |
| **east perimeter** | **0%**, 45 m dark | **100%**, no dark run |
| front walk | 89%, 9.5 m dark | **100%**, no dark run |
| back-of-house | 90%, 8.5 m dark | **100%**, no dark run |
| main spine, both sides | 100% | 100% |

and the floor as a whole goes from **87% to 100%** at "you can tell something
is hanging there", with nothing left in the two bands below it.

### 2. The reward looked worse than the thing it replaced

The emergency state photographs beautifully — warm pools on the coves, the
escalator picked out, LADIES and SHOES legible across the spine, the racks
reading as racks. Throwing the breaker turns it into **an untextured model
viewer**: flat, shadowless, evenly grey. The payoff for crossing the whole
store currently costs the picture.

`light_boswells_power_states.py` authored both layers unshadowed and said why:
*"the mains do not [fall off], because mains lighting is designed to be even."*
That is true of a real store, and it is true **because a real store hangs
troffers at a spacing near its mounting height.** Ours are 1,200 cm apart over
a 390 cm drop to a child's eye — about **three times** the mounting height,
far past the 1.0–1.5× that produces even light. The evenness was never coming
from the grid. It was coming from the lights not casting shadows, which is a
different thing wearing the same word.

So the mains cast shadows now, and their radius is trimmed from unlimited to
1,400 cm — enough to reach the floor and about 6 m sideways, half the 12 m
spacing, so neighbours meet in the middle instead of every lamp washing the
whole wing. **This overrides a written decision**, which is why it is in a
document rather than only in a diff.

Intensity is untouched, for the reason in the section above.

### 3. Twenty-nine fittings were glowing over dead lamps

`ULLKPowerSubsystem::MainsFixtureTag` carries Chris's rule in his own words,
locked 2026-11-22:

> *"A fixture that glows while its light is off is worse than no fixture at
> all, because it tells the player the power is on and the room disagrees with
> it."*

The runtime obeys it for the mains. **Nothing obeyed it for the emergency
circuit.** `rehabilitate_boswells_emergency_lighting.py` deliberately kills
about half of those lamps — *"original trading fixtures are dead"* — while
`fixture_boswells_lights.py` hands every non-mains light the same
`MI_LLK_Fixture_Emergency` housing glowing at 2.5, lit or not. So the ceiling
was advertising a working emergency circuit over a floor that has none, in
twenty-nine places.

They are dark now, in a new `MI_LLK_Fixture_Dead`. **Not black** — a dead
fluorescent is a cold grey diffuser that still catches what light reaches it,
so the base colour is 0.42–0.46 grey with emissive 0. A black fitting would be
a hole in the ceiling, which is the mistake the mirrored coffers already
demonstrate.

## What the floor looks like now

| at a 124 cm eye, lantern off | before | after |
| --- | --- | --- |
| median illuminance | 0.048 lux | 0.055 lux |
| floor at "shapes" or better | 87% | **100%** |
| floor in the two darkest bands | 13% | **0%** |
| escape routes fully lit | 3 of 6 | **6 of 6** |
| fittings glowing over dead lamps | 29 | **0** |
| lights stranded in the plenum | 0 | 0 |

The emergency state barely moved in brightness — the median rose by 0.007 lux
— and that is the right shape for this fix. It did not make the store lighter.
It made the dark parts of it reachable, which is the whole of `ROADMAP.md`
slice 2 and none of the mood.

The mains state is where the picture changed, and the table cannot show it:
shadows under the rails, pools along the coves, polished tile. Compare
`n23_eastwall_mains.png` with `n23c_spine_mains.png`.

## What is still not true

- **Nobody has played any of it.** Everything above is a measurement and a
  still. The 165 cm aisles have not been walked with the keeper capsule.
- **The keeper capsule is not nine years old.** Row 90 requires eye height to
  be measured from a built capsule, and 124 cm is still derived from stature
  rather than read off an actor.
- **The Sweeper has not been re-checked** against either the new aisles or the
  new light. It sees over everything the player cannot.
- **The emergency state leaks contents.** The band table says 17% of the floor
  is at "you can tell it is clothing" and 6% at "colour starts", against a
  brief of *"shapes, not contents"*. It photographs well, so this is recorded
  and not acted on.
- **The mirrored coffers are wayfinding that only works with the power on.**
  `docs/FINDINGS-AT-NINE-YOU-CANNOT-SEE-OVER-THE-RACKS.md` argues they become
  the only wayfinding a child has, and in the state the chapter is played in
  they are dark. That is physically correct and may still be the wrong design.
  Open.

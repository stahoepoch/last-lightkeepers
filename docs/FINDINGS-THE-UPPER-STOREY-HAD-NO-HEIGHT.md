# The upper storey had no height, and its lights were in the roof

**2026-11-23.** Chris, answering the one question
`docs/FINDINGS-FIFTY-LIGHTS-ON-NO-CIRCUIT.md` left open:

> **"Build it at 400."**

Done, and the building of it turned up two things nobody had looked for.

---

## 1. What "no ceiling" actually meant

The complaint that started this — *"the entire area has a ceiling that also
drops down super low"* — was the opposite of true, and the correction is in
the previous findings file. The deck's median headroom was **838 cm**.

What was true is that the storey had no ceiling *height*: 89% of it open to
`ANCHOR_FULL_ROOF` at 838 cm, 10% under the service core's lid at 375, and the
kids department under a tile field at 448 **with no collision**, so no trace
could find it and nothing could be stopped by it. Three heights and a void,
with no datum.

`Tools/ceiling_boswells_upper_storey.py` lays one at Chris's number:

| | before | after |
| --- | --- | --- |
| lowest headroom on the deck | 375 | **354** |
| median | 838 | **400** |
| p90 | — | **404** |
| under 200 cm | 0% | **0%** |

402 pieces, **3,618 m²**, on the ground floor's own tile material so the two
storeys are one building. The service core keeps its 375 — a service drop is
*supposed* to be lower than the selling floor — and the kids field comes down
from 448 to 404, four centimetres above the general field so where the two
meet the new tile reads in front of the old grid instead of fighting it for
the same plane.

The 354 cm low point is not the ceiling. It is a sign and a light fitting
hanging under it, which is what fittings do.

**The new ceiling has query collision, and the kids field was given it.** The
kids tile field at 448 had none, which is why every audit of this floor
reported that department as open to the sky. A ceiling nobody can measure is a
ceiling nobody can prove is there.

## 2. The grid exists twice, and half of it was in the roof

The probe written for the ceiling found what the lighting pass had missed:
**`BSW_MAINS_*` and `BSW_EMERGENCY_FILL_*` exist once below the deck and once
above it, under the same family names.** The low copy lights the ground floor
and is the one yesterday's pass measured. The high copy hangs at **z 1462** —
812 cm above the deck, in the roof space — and is the upper storey's lighting.

While that deck was open to the sky this was correct. A ceiling laid under it
would have buried every light the floor has, and Chris would have opened the
map to a department lit by nothing at all.

So **78 lights and 75 housing meshes were re-hung** to 1024, which is the 26 cm
drop this project already uses everywhere. A light over the open atrium stays
in the roof, because a light in an atrium hangs from the roof — and moving
those down would also have dropped them 438 cm nearer the ground floor through
the one hole in its ceiling, brightening a floor that had just been changed.

## 3. The fittings were telling yesterday's story

The first render of the roofed deck looked like a department store **with the
lights on**: rows of white tubes to the far wall. The breaker is off and not
one of those lights was lit.

A fitting's glow is a material parameter, not a light, and it had come loose
from the circuit in two ways:

- `MI_LLK_Fixture_Emergency`, `_Mains` and `_Dead` all exist and are right, but
  what picks between them is which circuit the light was on **when the fitting
  was placed**. Yesterday's pass moved 110 lights between circuits and touched
  no housing, so 13 `LLK_DOCK_CHAPTER_WING_POOL` fittings and six
  `BSW_DEPT_ROUTE_POOL_*` fittings glowed on the emergency material over
  lights that are now mains and dark.
- twenty `ANCHOR_EMERGENCY_*_Housing` wore `BasicShapeMaterial` — the engine
  placeholder, on a light fitting.

`Tools/repair_boswells_fixture_housings.py` applies one rule — **a fitting
wears the circuit of its own light, and is tagged to match** — and repainted
**60 material slots** and corrected **35** `LLK_MainsFixture` tags. A housing
with no light (4) and a light with no housing (83) are reported rather than
guessed at.

`MI_LLK_Fixture_Mains` turned out to be authored at `Emissive = 0` already, so
the bright tubes were the emergency layer doing its job, not the mains leaking.

## 4. My eye said the deck was too bright. The arithmetic says it is not

The wide render of the finished deck reads much brighter than the ground
floor, and that looked like Chris's original complaint reappearing one storey
up. `Tools/audit_boswells_clearance_and_mood.py` now samples the deck the same
way it samples the ground floor, so the two can be put side by side:

| | ground floor | upper deck |
| --- | --- | --- |
| median | 0.0000 | **0.5× the ground floor** |
| peak | 0.0005 | 0.0007 |
| p90/p10 contrast | 13.1 | **29.5** |

The deck is **darker on median than the floor below it and more than twice as
pools-and-dark**. What reads as brightness in the frame is the fittings' own
emissive — 2.5 on a 120 cm tube, seen from a wide angle that puts twenty of
them in shot at once — and not the light they cast.

That is the second time in two days a measurement has contradicted an
impression of mine, and it is left alone on purpose: the emissive value is
shared with the ground floor, and changing it would alter a floor Chris has
not yet judged.

## What is measured and deliberately not fixed

**The fittings are still the brightest thing in frame**, on both storeys. Now
measured as a cause rather than guessed at: `MI_LLK_Fixture_Emergency` sits at
`Emissive = 2.5` while the pool it casts is a fraction of that. One number,
one asset, both floors at once — so it is Chris's call and wants a before and
after picture of each storey.

**Four atrium posts and four canopy wraps now pass through the new ceiling**,
650 to 1300. Columns go through ceilings in real buildings, so they are
reported and left. `BSW_KIDS_CEILING_SALES_COLUMN` does the same.

**The atrium canopy at 1449 is now above the ceiling** and can only be seen
from the void. It was built to be seen from the ground floor looking up, and
it still can be, but nothing on the deck sees it any more.

**184 overlapping prop pairs remain on the deck** and **261 on the ground
floor**, unchanged by this pass. Both need a dressing pass that knows what is
already in a cell.

**Nobody has played any of it.** Every number here is a measurement against a
saved map, and every picture is a render.

---

**Corrected 2026-11-24.** Two of the lines above did not survive contact with
a player.

Chris had in fact played the map - *"I played it yesterday. I looked at it,
walked through it"* - so the closing caveat was already false when it was
written. See `docs/FINDINGS-THERE-WAS-NOTHING-TO-DO.md`.

And **the offer at the top of this section was the wrong fix.** Dimming
`MI_LLK_Fixture_Emergency` from 2.5 treats as a slightly-too-high value what
is a scale error of roughly twenty times: on a floor that peaks at 0.0005,
anything emissive over about 0.2 clips to white before its colour is visible.
The sweep that settled it is in the new file. The value is still unchanged,
but for a better reason than the one given here.

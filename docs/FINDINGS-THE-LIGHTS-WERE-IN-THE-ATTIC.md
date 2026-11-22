# There were no beams. There was a hole — and the lights were in the attic.

**2026-11-22.** Chris, choosing what to do after the floors: *"The ceiling
beams."*

Two defects came out of looking. The one he pointed at was real and smaller
than it looked. The one underneath it was mine, from earlier the same day,
and it had already been shipped and described to him as working.

---

## 1. The beams were a gap

`Tools/audit_boswells_ceiling.py` counted 884 pieces of geometry overhead and
a rendered frame from the west wing settled what they were. The sales floor
already had a suspended tile ceiling — `BSW_LOWER_CEILING_WEST_FIELD` and
`..._EAST_FIELD`, at 5.14 m, and it reads correctly. **Both fields stop at
x = ±900.** The 18 m strip between them is the main aisle, and it had nothing
overhead at all.

So from anywhere in a wing you looked across a black trench with the near
field's 4 cm edge in front of it. That edge is the "beam". There was never a
beam; there was a lit ceiling ending at a hole, which is what a blade looks
like.

**The hole was exactly the route.** The racetrack laid the day before runs up
that strip. It is where the chapter walks from Boswell's to Sayers, and it was
the one part of the store with no ceiling.

`Tools/ceiling_boswells_aisle_soffit.py` lays a painted bulkhead over it —
drywall rather than more tile, because real stores change the ceiling over
the racetrack, and that change overhead is the same signal the carpet gives
underfoot read the other way up. 152 pieces, 1,368 m². The atrium is skipped,
derived from the map as whatever rectangle the four `ANCHOR_UPPER_*` decks
leave uncovered, because an anchor store's atrium is open to the roof on
purpose.

The rule it applies is **"no part of the sales floor is unroofed"**, not "the
aisle is 18 m wide". The fields also stop 6 m short of the north wall. A tool
with the aisle's width typed into it would have closed one gap and left the
other, which is how this one got here.

## 2. Every light added that day was above the ceiling

`Tools/audit_boswells_trapped_light.py`, written to ask a different question,
answered this one: **86 of 174 lights in the map stood above a lid.**

The cause was two constants in `Tools/light_boswells_power_states.py`:

```python
EMERGENCY_Z = 1150.0
MAINS_Z     = 1380.0
```

applied across the whole footprint. **The lower sales floor's ceiling is at
514 and the mezzanine deck above it at 638.** All 48 lights of the two-state
lighting system were hung at 11.5 m and 13.8 m — above both, in the roof
void, lighting the top of a ceiling nobody can see.

What the screenshots showed, and what Chris approved as *"the vibe"*, was the
original `ANCHOR_EMERGENCY_LOWER` spots at ~485 doing all the work. The layer
added that morning contributed nothing to the floor the chapter plays on.

And the part that matters: **throwing the breaker would have lit the
mezzanine and left the sales floor exactly as dark as before.** The feature
Chris asked for — *"I envision being able to turn on the power at some
point"* — did not reach the room.

The heights are no longer typed. Each light is hung 26 cm under the ceiling
that is actually above its own position, read from the map: one under the
lowest lid for the floor you walk on, one under the highest for the
mezzanine. The constant was never a number that could have been right
everywhere, because the building has two storeys and an atrium. The tool now
refuses to save a map where no mains light reaches the sales floor.

## 3. The check that said it was fine

`ALLKTestGameMode::ReportLightsWithoutFixtures` was written that morning to
enforce Chris's rule that every light must have something it comes out of. It
reported, truthfully:

> Every one of 174 light(s) in LLK_Boswells_OpeningPass has a fixture within
> 260 cm, so nothing in this map is lit from nowhere.

Both the light and its fixture were sealed in the roof. A nearness test
cannot tell a fitted lamp from a lamp and its own housing in an attic.

Worse was coming. Once the lights were re-hung 26 cm under their ceilings,
**every light in the map had a very large mesh very close by**, and the same
check would have gone green for a second wrong reason — reporting every light
fitted on a map where none had a fitting. A ceiling is not a fixture. Both
the C++ check and `fixture_boswells_lights.py` now ignore anything spanning
more than 3 m: a troffer is 0.6 m, and anything bigger is the room.

## 4. The rule that would have caught all of it

`ALLKTestGameMode::ReportFloorWithoutCeiling`. Every other check this project
owns counts actors, and **no count of actors can notice a hole.** This one
samples the navmesh on a 6 m grid and traces straight up.

Two things were wrong with the obvious version, and both are worth keeping:

- **Tracing down from the top of the nav bounds finds the ceiling, not the
  floor.** It would then trace up from above the roof, find nothing, and
  report that a fully roofed building has no ceiling anywhere. The navmesh is
  the only source that means "stood on".
- **Counting open samples is true and useless.** The first run said 513 of
  690, most of them the dock approach at y = −35,000, which is outdoors and
  supposed to be. What it reports now is a *hole in a ceiling*: an open cell
  with a roofed cell beside it. Open beside open is a yard.

After the pass it reports 105, down from the aisle's whole length, and names
their bounding box — x −4400..4600, y −22500..3300. They are a one-cell ring
at the building's edge, where the navmesh inflates past the wall, and the
dock chapter, which has no ceiling pass yet.

---

## What this does not prove

Nobody has played it. The two states, the bulkhead and the re-hung lights
were judged from four rendered stills, and whether the store is *frightening*
on emergency power is not a thing any of these instruments reports on.

The four atrium posts are no longer on `BasicShapeMaterial`, but they are
still unlit, and under mains they read as black monoliths. 44 more actors
overhead are on placeholder materials and were deliberately left alone rather
than repainted by a rule broad enough to catch the store's own walls — which
two earlier versions of that rule did.

`ELECTRONICS` on the west wall renders mirrored. It is visible in every aisle
screenshot and is not touched here.

# The lots had nothing in them, and the moon had nothing behind it

**2026-11-25.** Chris, picking from the list of things
`docs/FINDINGS-THE-STORE-HAD-NO-OUTSIDE.md` said it had not done:

> *"Lots and sky"*

Those were its own two admissions, quoted back:

> **You can walk out but there is nothing to walk to.** The lots are flat
> asphalt with lamps - no cars, no kerbs, no cart corrals, no treeline, no
> mall beyond.
>
> **The sky is black.** There is no `SkyAtmosphere` and no stars - the moon is
> a directional light with nothing behind it.

---

## The rule this pass had to obey

**The inside of the store must not move.** The week began with Chris saying
the interior was too bright, and a sky is the one change most likely to undo
that without anyone noticing: a `SkyAtmosphere` feeds a sky light, a sky light
reaches wherever the envelope leaks, and nothing fails when it does.

So the acceptance test was the interior, measured at the same two traced poses
through all three passes:

| | before any exterior | after the envelope | **after the lots and the sky** |
| --- | --- | --- | --- |
| ground pose, frame mean | 16.71 | 16.39 | **16.44** |
| ground pose, median | 1.79 | 1.72 | **1.79** |
| deck pose, frame mean | 41.24 | 40.16 | **40.31** |

A sky went in over the building and the floor under it moved by **three
tenths of one percent**. The envelope is sealed; the exterior is genuinely
outside.

## What went in

`Tools/dress_boswells_exterior_lots.py`, 144 actors:

- **87 stall markings** across three rows, with kerbed islands planted down
  the middle of them. Every stall is measured from its lot's own bounds, so a
  lot that gets resized does not leave paint hanging off the end.
- **Eight cars**, and the number is the point. An anchor store's lot holds two
  hundred; eight cars parked near the entrance on a dark night is a building
  **with people still in it**, which is the only story this pass tells.
- **A cart corral** by the east entrance - the one object that reads as a car
  park even in silhouette.
- **Fourteen trees** along the shoulder of the hill, jittered rather than
  spaced, because fourteen identical cones at a fixed interval read as a
  fence.
- **A `SkyAtmosphere`**, and the sky light switched to real-time capture so
  it stops lighting the level with the black it was built against.

## Three things the asserts caught

1. **Every tree was floating.** They had been seated at a computed height that
   put each trunk 17 cm *inside* the lower lot's own slab, where a downward
   trace starts within the collider and finds nothing. They are traced onto
   the ground now, and they stand on the slope rather than out on the asphalt,
   which is where that first calculation had put them.
2. **The cars read as glass slabs.** The cabins were made of
   `M_BSW_MallEntranceGlass` - the storefront's clear glass - which under a
   sodium lamp was the brightest thing on the lot, so each car was a dark
   plinth under a glowing block. Smoked mirror for the cabins, and paint
   values roughly doubled so a car catches a lamp instead of vanishing.
3. **The jitter threw the end tree off the hill**, 230 cm past the edge, where
   the ground trace found nothing at all. Clamped.

## What is measured and deliberately not fixed

**A car is two boxes.** Body, cabin, no wheels, no lights, no plate. It holds
up as a mass with a windscreen at forty metres in the dark, which is the only
distance this level currently shows one at. Up close it is a box.

**A tree is a cone on a stick**, varied in height and spread but all the same
shape, all the same two colours.

**There is still no mall beyond.** The east lot ends at a hard edge with
nothing past it, and `Content/LLK_Galleria_Shell.umap` - the separate
uncommitted map that models the Galleria itself - remains the unresolved
question `docs/FINDINGS-THE-STORE-HAD-NO-OUTSIDE.md` raised. Two efforts model
the same mall and somebody has to decide which one is real.

**The sky has a gradient and nothing else.** No stars, no clouds, no visible
moon disc - the `SkyAtmosphere` gives the night a horizon and a colour, and
that is all. A `VolumetricCloud` was not added because its cost against this
floor has not been profiled, and `Config/DefaultEngine.ini` is explicit that
renderer features here get measured before they get enabled.

**The lot lamps light pools, not the lot.** At 9 cd with a 26 m radius the
asphalt between them stays dark. That reads correctly at night and it has not
been checked against anything a player needs to see.

**Still no exterior door opens.** Unchanged from the envelope pass: all five
are sealed geometry with no verb on any of them.

**Nobody has played it.** Every number here is a measurement against a saved
map and every picture is a render.

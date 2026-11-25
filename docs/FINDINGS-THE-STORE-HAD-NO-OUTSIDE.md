# The store had no outside, and two of its walls were engine placeholders

**2026-11-25.** Chris, having walked the opening map:

> *"there's supposed to be like a couple exits or entrances from the outside
> to Boscov's. Definitely the entrance-exit to the mall on both top and bottom
> have that glass, but so do the exits to the outside. There's at least two
> that I can recall on Boscov's that I'm from. Plus the whole exit for the the
> loading dock and stuff. Like if you actually looked at the whole Johnstown
> Galleria and everything, you could see how it's on a hillside and the things
> are weird because we don't even have like an exterior or anything."*

And, asked which to do first:

> *"Any or all lol"*

---

## 1. The survey was worse than the complaint

Before this pass, the entire outside of Boswell's was two actors:

| | |
| --- | --- |
| `ANCHOR_FULL_WALL_WEST` | x −4350, 5400 long, 1500 tall, **`BasicShapeMaterial`** |
| `ANCHOR_FULL_WALL_EAST` | x 4350, 5400 long, 1500 tall, **`BasicShapeMaterial`** |

No north face. No south face. No door to the outside anywhere in 3,078
actors. No ground beyond the floor slab. And **no sky, no sky light, no
directional light and no fog in the level at all** - the only atmospheric
actor was a `PostProcessVolume` holding the exposure lock. Step through a wall
and you were in vacuum.

**Both of those walls wear the engine's default material.** That is the exact
defect Chris led with on the upper storey, where seven engine cubes on
`BasicShapeMaterial` were the most visible thing in every render - and it had
been true of this building's east and west elevations the entire time, unnoticed
because nobody had ever been outside to look at them.

## 2. What is there now

`Tools/build_boswells_exterior.py`, 91 actors:

- **An envelope.** The two placeholder slabs are **hidden and decollided, not
  deleted** - the precedent set for the upper-storey blockout in
  `docs/FINDINGS-FIFTY-LIGHTS-ON-NO-CIRCUIT.md`, because
  `docs/FINDINGS-KIDS-FLOOR-STEALTH.md` counts geometry as cover and an audit
  told those walls exist should still find them. In their place: segmented
  walls on a real facade material, a brick base course, a parapet cap, piers
  at 14.5 m, and the north elevation that never existed.
- **Two exterior entrances**, west and east, glazed in four bays with
  mullions, a canopy, a threshold and a hooded wallpack over each.
- **The upper mall entrance.** Both `MALL_*` headers in this level are at
  ground level, so the "top" half of what Chris described was simply absent.
  There is now a glazed opening at the deck.
- **The dock's own way out** - an overhead door, a staff door, an apron and a
  kerb, on the north face where the dock actually is.
- **The hill.** Three tilted runs at 8°, 11° and 14° falling **1043 cm** from
  store grade to a lower lot, with the east lot flat at grade. That asymmetry
  is the whole point: you read a hillside from inside a building by the ground
  not being at your feet on every side.
- **A night to see it against** - a moon at 0.30 lux, a sky light at 0.40, thin
  fog, and five sodium standards over the lots.
- **The store's name on its own building**, in the project's own idiom: a
  `TextRenderActor`, the same way every sign inside this store is made.

## 3. The acceptance test was the interior, not the exterior

The complaint that started this week was that the inside was too bright. An
exterior with a sky in it is the most plausible way to undo that by accident,
so the test was that **the two interior poses must not move**:

| | before any exterior | after all 91 actors |
| --- | --- | --- |
| ground pose, frame mean | 16.71 | **16.39** |
| ground pose, median | 1.79 | **1.72** |
| deck pose, frame mean | 41.24 | **40.16** |

The envelope is genuinely sealed. The night made the interior about 2%
*darker*, not brighter - the fog, taking a little off the far end of the long
shots.

## 4. Four things only the verifier found

`Tools/verify_boswells_exterior.py` checks each item Chris named by its own
name, and it earned its place on the first run:

1. **The placeholder walls still collided.** `set_collision_enabled` on the
   component alone did not take; the actor kept its collision, so both new
   doors opened into an invisible wall. It needs
   `set_actor_enable_collision(False)` and the `NoCollision` profile as well.
   The same bug then turned up again on the glass, which blocked a visibility
   trace through its own doorway.
2. **The base course ran straight across the east entrance** - a 120 cm kerb
   through the middle of the doors.
3. **The west entrance opened into `BSW_JUNIORS_DRESS_WALL_1_STOCK_2`.** The
   door's position had been typed in. It is now traced: the tool scans each
   elevation for the longest stretch with nothing standing inside it, and
   **the widest clear bay on either side is 900 cm**, so the entrances are
   820 cm wide rather than the 1840 first attempted. The floor decides how
   wide the door is.
4. **A lamp standard stood directly in front of the east entrance**, bisecting
   the word BOSWELL'S, because the lamp spacing was typed and the entrance was
   traced. Lamps now step aside for a door.

The verifier then reported two doors blocked that were perfectly clear,
because it carried the build tool's *old* door positions as constants. It now
finds the doors by looking for them. **A verifier that remembers where things
used to be is worse than no verifier**, and this one was caught by its own
failure message naming an actor at a coordinate nothing is built at any more.

## 5. What this is not

**It is not the Galleria.** `Content/LLK_Galleria_Shell.umap` is a separate,
uncommitted 350-actor map by another writer that models the mall itself, with
its own `Tools/build_galleria_shell.py` and verifier. This pass does not open,
read or touch it - it is the outside of one anchor store, built in the map
Chris actually plays. **Those two efforts now overlap and somebody has to
decide which one is the Galleria**, because the opening map's mall concourse
runs from y −2700 to y −35700 and the shell map models the same mall again.

## What is measured and deliberately not fixed

**You can walk out but there is nothing to walk to.** The lots are flat
asphalt with lamps - no cars, no kerbs, no cart corrals, no treeline, no mall
beyond. The exterior reads from the doorway and from the lot; it does not
reward exploring.

**The sky is black.** There is no `SkyAtmosphere` and no stars - the moon is a
directional light with nothing behind it. Looking up outside shows nothing.

**No exterior door opens.** All five are sealed geometry. The glass is
non-colliding so a trace passes, but there is no `ALLKOpeningGrille` or
equivalent on any of them, and no verb.

**The exterior lights carry no circuit tag, deliberately.** A car park light
is not on the store's breaker, and the verifier asserts that none of them has
picked up `LLK_Mains` or `LLK_Emergency`.

**The hill is three tilted boxes, not terrain.** It falls continuously and the
verifier checks that it does, but it is flat-shaded slabs on a turf-coloured
material, not a landscape.

**Nobody has played it.** Every number here is a measurement against a saved
map and every picture is a render.

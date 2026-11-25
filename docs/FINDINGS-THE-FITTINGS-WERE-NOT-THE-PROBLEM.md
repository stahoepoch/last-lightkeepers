# The fittings were blown out, and the room was never lit by them

**2026-11-25.** Chris, asked whether to do the exterior or the fixture
emissive pass first:

> *"Any or all lol"*

So both. This file is the emissive pass, and it corrects a claim I made to him
yesterday. `docs/FINDINGS-THE-STORE-HAD-NO-OUTSIDE.md` is the exterior.

---

## What I told him yesterday, and what the sweep says

`docs/FINDINGS-THERE-WAS-NOTHING-TO-DO.md` closes by arguing that
`MI_LLK_Fixture_Emergency` at `Emissive = 2.5` is *"roughly seventeen times
past the point where colour survives"*, and that this is why *"nothing in this
building has had a colour at the fitting itself."* The reasoning was carried
over from the exit sign sweep, where a small orange face on a dark wall went
from white to orange between 4.0 and 0.15.

**It does not transfer, and the measurement says so plainly.** Four renders of
the same pose, changing only that one number:

| `Emissive` | frame mean | frame median | ceiling tile field | fully clipped pixels |
| --- | --- | --- | --- | --- |
| **2.50** (as authored) | 17.21 | 1.79 | 52.65 | 0.134% |
| 1.00 | 16.89 | 1.79 | — | 0.008% |
| 0.40 | 16.71 | 1.79 | 52.63 | 0.005% |
| 0.15 | 16.55 | 1.79 | 52.61 | 0.003% |

A **seventeenfold** cut moves the lit ceiling by **0.04 of 255** and the frame
median not at all. The fittings are not lighting this store and never were;
the placed lights are. So "the fittings are the brightest thing in frame,
now measured as a cause rather than guessed at" - the line
`docs/FINDINGS-THE-UPPER-STOREY-HAD-NO-HEIGHT.md` ends on - was true of the
fitting and false of the room, and the fix I offered twice would have changed
nothing a player could see.

## What *is* wrong with them

Measure the tube face itself rather than the brightest half-percent of the
frame, and the defect appears immediately:

| `Emissive` | tube face mean | tube p90 | face saturation | **face pixels clipped** |
| --- | --- | --- | --- | --- |
| **2.50** | 126.3 | 253.1 | 0.089 | **26.5%** |
| 1.00 | 106.3 | 244.7 | 0.096 | 0.0% |
| **0.40** | **86.4** | **225.5** | **0.110** | **0.0%** |
| 0.15 | 63.4 | 182.0 | 0.144 | 0.0% |

**At the authored value a quarter of the fitting is pure white**, which is why
every render for a week has shown a row of featureless blobs instead of
tubes. The authored colour is a cool `(0.72, 0.84, 1.00)` with a saturation of
0.28; at 2.5 the face delivers 0.089 of it.

**Set to 0.40.** That is where clipping reaches zero while the tube still
reads at p90 225 - unmistakably the brightest object in the room, with an edge
and a visible cool cast. 0.15 holds more colour and starts to look unlit.

There is a second thing the sweep found. The grainy cloud hanging above each
fitting in every wide render is **the emissive itself**, and it scales with
the value exactly as the face does:

| `Emissive` | cloud mean | cloud p90 |
| --- | --- | --- |
| 2.50 | 40.9 | 177.4 |
| 0.40 | 20.5 | 85.8 |

Halving the blow-out halves the noise. `docs/FINDINGS-EMISSIVE-LIGHTS-NOTHING.md`
already records the standard Lumen remedy for a noisy emissive - take the mesh
out of the indirect-lighting scene and let the placed light carry the
illumination - so it was tried on all 41 emergency housings.

**It changed nothing.** Every patch measured byte-identical before and after:
cloud 20.47, face 86.44, ceiling 52.6. `affect_dynamic_indirect_lighting` is
not what puts that mesh in Lumen's scene, or not under this software-tracing
baseline. **The flag was reverted** rather than left set, because 41
components carrying a flag with no measured effect is this project's own
failure shape - a correct system over an absent thing - in miniature.

## What is measured and deliberately not fixed

**The noise cloud is still there**, at half its old strength. The remedy our
own notes recommend did not work and the real control has not been found.

**`MI_LLK_Fixture_Mains` is at 0 and `_Dead` has no emissive at all.** Only
the emergency layer was re-authored, because only the emergency layer is lit
in the state the game boots into.

**The ceiling tile is the brightest *surface* in the store**, at 52.6 and
flat across the field. Nothing here touched it. If the floor still reads
"1996" after this, that tile is the better suspect than the fittings were.

**Nobody has pressed anything.** These are renders of a saved map. What Chris
played is a week older than this file.

# The racking is 106 deep, and that one number governed the whole dressing pass

> **Status: DONE 2026-11-03, shipped in `d399e40`.** The placement pass Chris
> approved — *dress Boswell's existing circuit, and nothing else.* 67 props,
> every one labelled `DRESS_*`.
>
> Measurements below were read through the MCP bridge against **real
> collision**, not actor bounds. The kit pieces were measured offstage first
> and the probes removed before placing. This file exists because the numbers
> were otherwise only in a chat transcript, and the headline one changes how
> the aisle plays.

## The kit, measured before anything was placed

Unrotated, in centimetres. `base z` is given where the mesh origin is not at
its own floor, because placing to a z without that correction buries or floats
the prop.

| Mesh | X | Y | Z | base z |
| --- | --- | --- | --- | --- |
| `SM_Upright_Frame` | 9.0 | **106.0** | 250.0 | 0 |
| `SM_Load_Beam_3_5m` | **349.5** | 106.4 | 19.3 | −19.1 |
| `SM_Decking_Steel` | 113.0 | 106.3 | 3.4 | 0 |
| `SM_Lamp_Indoor_B` | 17.7 | 131.0 | 69.1 | −69.1 |
| `SM_Dock_Open` | 400 | 62 | 463.8 | −150 |

Two of those numbers decided the pass.

**`SM_Load_Beam_3_5m` is 349.5 cm — the exact length of the gondola segments
cut in `7916fb9`.** The racking fits the circuit as already authored, one bay
per segment, with no re-cutting and no fudged scale. That is luck, and it is
worth recording as the reason nothing was rescaled.

**The racking is 106 cm deep. The cube gondolas it dresses are 65.** Real
racking is 41 cm deeper than the greybox standing in for it, and that depth has
to go somewhere.

## Where the 41 cm went, and why

```
RACK_OUTER = 392.5          # the gondola's outer face, as authored
RACK_DEPTH = 106.0
RACK_CX    = 392.5 - 53.0   # 339.5, so the rack's OUTER face is flush
```

The rack's **outer** face sits flush at x ±392.5 and the depth grows inward.

The alternative — centring the rack on the cube — would have split the 41 cm
and eaten 20 cm off the **167.5 cm side aisle**, which is the tightest
measured route in the room and already the one a 68 cm Sweeper shares with the
player. The central aisle had 655 cm and could afford it. So the side aisle is
untouched at 167.5, and the whole 41 cm came out of the middle.

## The 200 cm escape gap, measured against the racking

The gap cut in `7916fb9` is the pass's governing constraint: it must not be
widened, and it must not be closed. Both rows now carry frame posts standing
41 cm proud of the shelf face, so the mouth is no longer one number. Sweeping x
across the rack depth and tracing along y at each step:

| x | south edge | north edge | gap |
| --- | --- | --- | --- |
| −388.0 | −96.3 | 96.3 | 192.5 |
| −382.0 … −328.0 | −100.0 | 100.0 | **200.0** |
| −292.0 | −97.0 | 97.0 | **194.1** |

**200.0 cm at the shelf face — the authored value, intact.** **194.1 cm at the
front post line**, a loss of 5.9 cm to the frame. Against a 68 cm capsule that
is 126 cm of clearance at the tightest point, so the route still reads as
passable, and **nothing was widened**.

## What did change: the central aisle is 573, not 655

A character crossing the middle is now bounded by the rack's inner face at
x ±286.5 rather than the cube at ±327.5.

| | before | after |
| --- | --- | --- |
| Central aisle, clear width | 655 | **573** |
| Side aisle, west | 167.5 | 167.5 |

573 cm is still 8.4× a 68 cm Sweeper, so this is a feel question rather than a
blocking one — but the central aisle is the patrol lane, and it is the number
to watch in play.

## Racking collision is its frame, not its box

This is the finding that saved the routes, and it is the opposite of what the
bounding box suggests.

A rack reads as solid **at the beam heights (z 70, 78), at the decking (168),
and at the uprights.** Between those it is open. The cube gondolas underneath
still carry the body-blocking, so the row is as sealed as it was.

That is why **every route probe is byte-identical before and after the pass**:
escape through both gaps, both crossings, the spawn-to-dock sightline, the
patrol A→C along x 0, the central aisle to the dock mouth, and the gap clear at
head height.

It also means a probe at the wrong height reads a rack as open. Three probes
during this pass missed a beam by centimetres — z 90 passes over a beam whose
top is 80.2; z 175 passes over decking at 170.2. **Probe at a height a body
occupies, not at a height that happens to be round.**

## The dock is a dock

`SM_Dock_Open` is 400 × 464, modelled 150 below its origin, so z 150 puts its
base on the floor. Placed at y 888, set into the north wall, with wheel
stoppers, parking poles and a pallet truck outside it.

Confirmed in place: `DRESS_Dock_Opening` x[−200, 200], z[0, 463.8].

It was a debug box, and **nobody has ever reached it** — extraction has not
been hit in a playtest. Making it readable as a destination is row M9's
*"the dock readable as a destination"* and nothing more; it is not a claim that
extraction works.

## Fixture housings do not light anything

Housings hang at z 586 with their undersides at 516.9, and the authored spots
sit at z 560 **inside** them. The lights were not touched and still do all the
lighting.

This is `FIXTURE-LIGHT-CONTRACT.md` observed, and it is also simply how Lumen
wants it: a real light inside an unlit body. The housing is there so a player
can read where the light comes from. **No intensity, cone or radius changed in
this pass.** C11's camera reading is still owed, and now has real fixtures to
read against.

Confirmed in place: `DRESS_Fixture_Aisle_1` z[516.9, 586.0].

## The pass is re-runnable

`dress.py` wipes every actor whose label starts with `DRESS_` before placing
anything. So it can be re-run against a changed circuit without duplicating,
and **nothing authored by hand is ever touched** — which is the lesson from
`slice.py`, whose non-idempotence squared a scale factor and spawned a
duplicate actor set on 2026-11-03.

## Verified

- Automation **35/35**, including `Harness.StoreFloorPlan` and
  `Harness.MapReadiness`.
- `npm test` exit 0.
- PIE restarts in 0.107 s with no missing-asset errors.
- Placement bounds spot-checked against intent:
  `DRESS_Rack_Upright_WS_0` x[−392.5, −286.5].

## What this pass deliberately did not do

**Movement speeds were not touched**, on Chris's instruction that the retune
lands as one package: player walk/sprint/crouch, Sweeper patrol speed and the
contact-recovery window together, under the invariant that **sprint must always
escape a stumbled Sweeper**. See `PROPOSAL-MOVEMENT-RETUNE.md`.

That matters for reading this pass, because the live complaint is that
*everything moves too fast* — the player walks at 448 cm/s, 3.2× human walking.
**Props judged at the wrong traversal speed will read wrong**, and this pass
cannot tell you whether 573 cm is a good aisle. Only a dressed circuit at a
tuned speed can.

Also out of scope, and recorded so they are not rediscovered as gaps: no demo
map, no further pack import, no texture hunting, no Game Animation Sample
import.

# Boswell's is 2800 by 1800, and both rows seal the room end to end

> **Status: OPEN, and these are traced measurements rather than arithmetic.**
> Read 2026-11-03 from the live editor through the MCP bridge, on the M9
> environment pass. Every width and height below comes from `trace_world`
> against real collision, not from actor bounds — **bounds lied repeatedly
> during this read** and the trap is recorded at the end. One defect found
> here is already fixed in this commit; the rest are open.
>
> **CORRECTED 2026-11-03, the same day, after Codex checked the source.** The
> original headline said the Sweeper was 160 cm wide and therefore 7.5 cm
> narrower than the side aisle. **That was wrong, and the error was mine.**
> 160 cm is the actor's world AABB, dominated by `SweeperChassis_Temporary`
> and the skeletal mesh component. The gameplay-authoritative footprint is
> the capsule, read live as `capsuleRadius: 34`, `capsuleHalfHeight: 88` —
> **68 cm wide**, with 99.5 cm to spare in a 167.5 cm aisle. The body-block
> is **not** geometry. The sealed-rows finding below is unaffected.

## The room, measured

| | |
| --- | --- |
| Floor | **z 0**, consistent at the centre aisle, the west side aisle and the dock mouth |
| Walls | 600 tall, 12 thick, at x −1406/+1394 and y −906/+894 |
| Interior footprint | **2800 × 1800** |
| Ceiling over the main aisles | **z 586** |
| Ceiling over the side aisle | z 604 |
| Roof plate | `BSW_Roof` at z 614 |
| Fixture housings, underside | z 532 at x 0, y 0 |

So there is a roof, and there are **three ceiling surfaces at three heights**
plus a housing soffit. That is the "ceiling language not integrated" Codex
named, with numbers on it: 586, 604, 614.

## The aisles, and why the robot pins you

| Aisle | Spans | Width |
| --- | --- | --- |
| Centre | x −327.5 … +327.5 | **655 cm** |
| West side | x −560 … −392.5 | **167.5 cm** |
| East side | x +392.5 … +560 | **167.5 cm** |

**What blocks, read off the capsule instead of inferred from bounds:**

| | |
| --- | --- |
| Sweeper collision | `capsuleRadius` **34**, `capsuleHalfHeight` **88** → **68 cm wide**, 176 tall |
| `bDynamicObstacle` | **true** — the capsule is exported to navigation as a moving hole |
| Sweeper world AABB | 160 × 274 × 176 — the *visual* chassis and mesh, **not** what blocks |
| Player collision | **not verifiable from the level.** `ALLKCharacter` never calls `SetCapsuleSize` either, so it is a class default needing a runtime read |

A 68 cm Sweeper in a 167.5 cm aisle has **99.5 cm to spare**, and two 68 cm
capsules pass each other with 31.5 cm between them. Tight, and passable.
**Geometry is not what pinned anyone**, and the first version of this document
said it was, from the AABB. That claim is withdrawn.

The one thing worth keeping from the wrong version is the gap between what
blocks and what you see: **68 cm of collision inside a 160 cm visual chassis.**
In a game about judging whether you can squeeze past something in the dark,
the player's read of the machine is wrong by a factor of two, in the direction
that makes them abandon a gap they would have made.

**The real spatial problem is that you cannot reach a side aisle at all.** Both gondolas run **y −450 to
+450 with no break** — 900 cm of continuous shelf. The patrol legs are
y −200 → +250 → +650. A player caught at y 0 is 450 cm from the nearest gap
in either direction, with a 160 cm machine in a 655 cm aisle between them.

`LLKSweeperBrain`'s contact cooldown gives an honest escape window, and the
nearest place to spend it is 450 cm away down a sealed corridor.
**The single highest-value edit in this map is a mid-gondola gap at y ≈ 0**,
which turns one 900 cm wall into two 400 cm runs with a side-step between
them, reachable from anywhere in the aisle within 200 cm.

## One actor was causing three defects, and is fixed here

`WL_DarkPocket_Partition` — the fourth `WL_` orphan, already on the ledger as
C12 — was x 0…500, y 470…530, z 0…280. Four traces, all hitting its y 470
face:

| Probe | Before | After |
| --- | --- | --- |
| Sweeper patrol leg along x 0 | blocked at y 470 | **clear** |
| Sweeper's right shoulder, x +70 | blocked at y 470 | **clear** |
| Spawn → dock sightline, eye height | blocked at y 470 | **clear, full 1530 cm** |
| Stock-bay shadow at x 450 | 170 | 170, unchanged |

It did three things at once: **the Sweeper could not complete its patrol**
(its right half, x 0…80, intersected the partition for the whole leg from
B to C), **the dock was invisible from spawn**, and it crowded
`BSW_STOCK_BAY` to 30 cm, which is the clearance finding from 2026-11-02.

Shrunk to **x 200…500** — scale x 1.0 → 0.6, location x 250 → 350, pivot at
its centre. The first two clear completely; the bay stays in shadow, so the
dark pocket the name asks for still exists. Nothing new intersects: the
right-hand gondola starts at x 522 and the shrunk end stops at 500.

## What is an art-direction call, not a measurement

- **Two overlapping sets of gondolas.** `BSW_Gondola_Left/Right` at
  x ±327.5…±392.5, and `WL_Gondola_Left/Right_A/B` whose real collision face
  traces to x ±560. Both are present and both have collision. Which set is
  canonical decides every aisle width in the map, so it is not mine to pick.
- **Which of the three ceilings is real**, at 586, 604 or 614.
- **Five different prop base heights** over a floor that is flat at z 0:
  furniture platform 17.5, toy displays 40, display case 47.5, endcaps 50,
  gondolas 75. Nothing agrees on where things sit.

## C11 is unaffected, and I checked rather than assuming

While reading this I thought the aisle-spot arithmetic had used the wrong
ceiling and was about to publish a correction. It had not.
`BSW_Emergency_Aisle_1` … `_4` read **z 560, pitch −90** from their
transforms, and the floor traces at **z 0**. 560 cm is exactly the distance
`FINDINGS-BOSWELLS-LIGHTING-AUDIT.md` used. The 10.1× and 50× figures stand
as computed, and the camera reading still waits on this pass per M9.

## The trap, for `TOOLING-TRAPS.md`

**An actor's bounds are not its collision, and this one cost a published
headline.** The world AABB includes every visual component: the Sweeper reads
160 cm that way and 68 cm at the capsule. For anything a pawn must fit
through, read `capsuleRadius` off the collision component. For anything a pawn
must walk around, trace. Never size a lane from an AABB.

**`get_actor_bounds` also returns a 256-unit placeholder cube on actors
without real mesh bounds, and nothing marks it as unreliable.** Every blueprint-style
`Actor_N` in this map reported some dimension as exactly 256, including point
lights, patrol points and the partition's y extent. Read literally, the
partition is 256 deep; traced, it is 60. A width taken from bounds here is a
guess wearing a number's clothes. **Trace for anything that has to be true.**

Two reads in this session were also wasted on probes that started inside a
prop — a floor probe at x −1000, y 0 returned z 120, which is the top of
`BSW_Toy_Display_2`, not the floor. Probe in open space and say where.

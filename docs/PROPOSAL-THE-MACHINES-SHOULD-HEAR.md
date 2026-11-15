# The machines cannot hear, and the numbers for it have already been playtested

Status: **proposal, 2026-11-14.** Written for `ROADMAP.md` slice 3 — *"give
machines distinct jobs, directional sound and visible route commitments"* —
and for row **92**'s requirement that the player must be able to **hide**.
Claude's lane under `LANES.md`: `Source/LLK/**` and the schema half of
`data/*.json`. **Every number below is a proposal; the values are Chris's
under row 55.**

## The hole

`FINDINGS-KIDS-FLOOR-STEALTH.md` found it and was right to stop there: **there
is no noise, sound or volume input anywhere in the enemy code.** Every
`Volume` in `LLKSweeper.cpp` is cue playback — output, not input. Notice is
light and line of sight, and nothing else. A machine cannot hear a sprint.

That was a deliberate old position — *"wiring volume into aggro is a second
notice rule"* — taken when the product was a four-player raid where the
noise-maker was the lantern and the crew. **Row 92 makes stealth a pillar of a
solo game**, and in a solo game the player's own movement is the only thing
making noise. Sprinting past a machine and crouching past it are currently the
same event to that machine, which means the game has one stealth verb (douse
the light) where it reads as having three.

**This is a decision to re-take, not a defect to fix.** It belongs to Chris.
What follows is the evidence that did not exist when the old position was
taken.

## The evidence: it has been played, and he tuned it himself

The Playground prototype has a hearing model and has been played with it. On
**2026-11-12** Chris played it, said *"those sweepers are a little too fast and
aggressive"*, and the resulting pass cut the Sweeper's hearing specifically —
sprint **180 → 120 px** and walk **90 → 65 px** — while leaving sight at a
separately-tuned number. **He has tuned these values by feel already**, which
is a kind of evidence this project has never had for an enemy sense.

The prototype's model, as built:

| Movement | General hearing | Against a Sweeper |
| --- | --- | --- |
| Sprint | 180 px | **120 px** |
| Walk | 90 px | **65 px** |
| Crouch | 40 px | **30 px** |
| Sweeper sight | — | 240 lit / 100 dark |

Three movement states, three radii, and crouch is roughly **a quarter of a
sprint**. That spread is what makes crouch a verb rather than a slow walk.

## The proposal, derived rather than picked

Row **81**: *a derived physical number beats a hand-picked one, provided its
derivation is written down beside it.* So these are expressed as multiples of
the Sweeper's existing dark notice radius rather than converted from pixels,
because the prototype's world is not to scale and its ratios are the part that
was playtested.

`data/enemies.json` already carries `aggroRadiusDark: 12` studs for Sweepers.
Against the prototype's Sweeper sight-dark of 100 px:

| | Prototype px | As a multiple of dark sight | Proposed studs | Proposed cm |
| --- | --- | --- | --- | --- |
| Crouch | 30 | 0.30 | **4** | 112 |
| Walk | 65 | 0.65 | **8** | 224 |
| Sprint | 120 | 1.20 | **14** | 392 |

At `StudsToCm = 28` (row 61) that is a machine that hears a sprint from about
four metres, a walk from a bit over two, and a crouching keeper only when
almost touching it. **Sprinting becomes louder than the dark is dark** — the
sprint radius exceeds the 12-stud dark sight radius, which is the point: you
can be heard before you are seen.

**Schema**, mine to propose, values his to set:

```
"hearingRadiusCrouch": 4,
"hearingRadiusWalk": 8,
"hearingRadiusSprint": 14
```

Three keys on the enemy family, defaulting to **0 — deaf** — so every enemy
that is not given values behaves exactly as it does today and nothing silently
changes. Row 56's rule applies: a stripped or stale key reaching runtime is a
hard error rather than a silent one.

**Hearing is a notice, not a sight.** It should wake the existing
`investigate` behaviour toward the heard position, never grant a lock or a
chase. The player who is heard gets a machine walking to where they *were*,
which is the thing that makes a sprint a decision instead of a death.

## The second hole, which is not mine to close

The same findings document established that sight is traced between
`GetActorLocation()` on both ends — **capsule centre, 88 cm standing and 40 cm
crouched** — while the player looks out from 152 cm. The children's racks
block only between 88 and 130 cm, so **crouching behind one lowers you out of
the band that hides you and makes you more visible.**

That is a geometry-and-trace problem rather than a hearing one, and the fix
could be in either lane: raise the traced point to something nearer eye height
so cover matches what the player sees, or author fixtures whose blocking band
starts at the floor. **Both are worth one line in a decision row**, because
"crouching makes you harder to see" is an assumption every player brings and
the build currently contradicts it.

Worth noting against row 90, which proposes a 12-or-13-year-old measured from
a built capsule: a shorter keeper changes both of these numbers, and this is a
second reason that row wants settling before the fixture pass hardens.

## What I am not proposing

**No second notice rule competing with light.** Light remains the loud,
visible, player-chosen cost (rows 5 and 10). Hearing is the quiet one the
player controls with their feet, and it should never produce the Warden's
attention or a swarm call — `swarmCallRadius` stays sight-and-light driven.

**No hearing for the Warden.** Row 7's unkillable Warden is a pursuit, not a
patrol, and giving it ears makes a chase unloseable rather than tense.

## The falsifier

If a machine that hears produces a chapter where the player sprints anyway —
because the route is long, the lantern is the real cost and footsteps never
decide anything — then this is a system that costs AI work and buys a number
nobody feels, and the old position was right. The test is whether a player
crouches **without being told to**, which is exactly the test the prototype can
run this week for the price of one generation.

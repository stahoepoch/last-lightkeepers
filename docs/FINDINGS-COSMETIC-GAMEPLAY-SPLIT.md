# Finding — Chris's cosmetic/gameplay split, audited against the code

Chris's ruling, 2026-11-04, in full:

> Keep pendulum sway cosmetic. Use the lantern's stable hand/mount anchor for
> gameplay origin and aim, but never let its 1 Hz visual arc pulse relic reach
> or enemy notice. A player shouldn't become detectable—or lose a valid
> "Present" interaction—because the bail happened to be at one end of a swing.
>
> - Physical pendulum: mesh, visible source, visual cone/trail, landing ring.
> - Stable mount: gameplay radius, detection, relic inspection, infrastructure
>   interactions.
>
> The pitch/aim arm should drive gameplay direction; the pendulum should make
> it feel carried.

This document audits every gameplay reader of the lantern against that split.
**The three light-field consumers are compliant, with Chris's own fingerprints
on one of them. The aim half describes a mechanism that does not exist. And
one infrastructure interaction violates the ruling today.**

## 1. Origin — compliant, and structurally so

`ULLKLanternLightComponent::GetGameplayLightLocation()` returns
`GetAttachParent()->GetComponentLocation()`, which is `LanternAnchor`.
Cosmetic sway is written to the *light component's* relative offset, one level
BELOW the anchor, so no sway term can reach the anchor's world transform. This
is not a convention that could be broken by a careless edit in `AdvanceFeel`;
it is a consequence of the attachment order.

`ULLKZonePressureSubsystem::GetLitLightSources` sets

```cpp
Source.Position = Light->GetGameplayLightLocation();
```

with a comment crediting Chris for catching it "before it shipped." The same
function also reads `Light->GetGameplayRadiusCm()` rather than
`AttenuationRadius`, because the attenuation radius is a value a cosmetic
crossfade is allowed to move. That is the identical principle in the other
axis, and `LLKLanternLevelTest.cpp:287-292` proves it: it shoves
`AttenuationRadius` to a quarter of its value and asserts
`GetGameplayRadiusCm()` does not move.

## 2. The three gameplay consumers — all position-only

| Consumer | Query | Reads a direction? |
| --- | --- | --- |
| Enemy notice (`LLKSweeper.cpp:326`) | `DetectabilityAt(Sources, Senses.KeeperPosition)` | no |
| Relic legibility (`LLKRelicComponent.cpp:158`) | `BrightnessAtVisible(Sources, relic location, visibility)` | no |
| Warden convergence (`LLKZonePressureSubsystem.cpp:375`) | `FindBrightestPosition(Sources)` | no |

Detection is evaluated **at the keeper's position**, not at the lantern's, so
even a hypothetical pendulum term in the source position would move the
distance by the swing amplitude and nothing else. Relic legibility adds a
`Visibility`-channel trace and a two-equal-samples debounce in
`ApplyLightSample`, specifically so "a shelf edge flipping the prompt on every
tenth of a second" cannot change the answer — which is the same class of
protection Chris is asking for, already present for a different cause.

The "Present" interaction itself consults no light at all, so it cannot be
lost to a swing. But one infrastructure interaction does read the lantern, and
it reads the wrong value.

## 2b. The live violation: the revive pulse reads the cosmetic radius

`LLKInteractorComponent.cpp:456-461`, building the distress pulse that an
Anchor Revive puts into the light field:

```cpp
Intensity = Light->GetResolvedRules().Intensity;
RadiusCm  = Light->AttenuationRadius;        // <-- cosmetic
```

`AttenuationRadius` is the value a lantern-level crossfade is allowed to lerp.
The gameplay-correct reach is `GetGameplayRadiusCm()`, which derives from the
named level's `GetRadiusStuds()` and is the value `GetLitLightSources` was
changed to use for exactly this reason. **The interactor was missed.**

The history is identical: reading `AttenuationRadius` was correct while a
lantern had one setting, and became wrong the moment it had three and a visual
crossfade between them. One call site was fixed and the other was not.

Why it matters more than it looks. A distress pulse enters the field with
`Detectability` pinned at 1.0 - it is *meant* to wake the room, and it decides
what the Warden converges on and what Sweepers notice from 38 studs instead of
12. So a revive that lands mid-crossfade gets whatever reach the visual lerp
happened to be passing through. `LLKLanternLevelTest.cpp:287-292` establishes
that `AttenuationRadius` can sit at a quarter of the gameplay radius, so the
pulse can under-reach by 4x - which quietly inverts the rule it exists to
serve: *"A solo player gets back up, but wakes up the room."* A pulse at a
quarter reach does not wake the room.

The comment directly above the defect states the correct intent - *"the
brightness and reach are the keeper's own lantern's"* - so this is a one-word
fix to match a stated intention, not a design question. `Source/` is Codex's
lane; it belongs in the lantern-anchor handoff.

## 3. Aim — the ruling's second half has no subject

```cpp
struct FLLKLightSource
{
    FVector Position;
    float   Intensity;
    float   RadiusCm;
    float   Detectability;
};
```

There is no direction, no cone angle and no orientation anywhere in the
gameplay light field. **The gameplay light is an isotropic sphere.** The
visible cone is 110 degrees; detection, relic reach and Warden convergence are
360.

So "the pitch/aim arm should drive gameplay direction" cannot be implemented as
a correction to something currently wrong — there is nothing there yet. Two
consequences, and the second is the interesting one:

1. The fairness risk Chris names is structurally impossible on the aim axis
   too, for the same reason: a sphere has no end of a swing.
2. **The asymmetry runs the other way.** A keeper is detectable, and can read
   relics, in directions they cannot see. Facing away from a relic lights it
   exactly as much as facing it. That is a real readability gap, it is not
   caused by the pendulum, and the ruling's aim clause is the natural place
   from which to decide whether to close it.

If gameplay aim is ever added, this ruling already decides where it comes
from: `LanternAnchor`'s rotation, not the cone's `TrailDegrees`, which is the
cosmetic lag computed in `AdvanceFeel`.

## 4. Row 80 was double-booked — RESOLVED 2026-11-04

`docs/DECISIONS.md` had rows up to **79** until 2026-11-04. There was no row
80. Verified:

```
git show a0bd1e2~1:docs/DECISIONS.md | grep -c "^| 80 "   ->  0
```

Yet "row 80" is cited in at least twelve places in `Source/`, for **three
different older rules**:

| Citation | What it means there |
| --- | --- |
| `LLKPerception.cpp:97`, `LLKPerception.h:218,236`, `LLKNoticeTest.cpp:103`, `LLKPerceptionTest.cpp:73`, `LLKPerceptionFailureTest.cpp:92,146`, `LLKSweeper.cpp:317` | detectability replaced brightness-and-a-derived-threshold |
| `LLKZonePressureSubsystem.cpp:349`, `LLKLanternLevelTest.cpp:257` | *"transitions smoothly lerp visually, while gameplay resolves to the named state"* |
| `LLKEnemyRosterTest.cpp:108`, `LLKDebugHUD.cpp:1052` | the two authored aggro radii are the ends of a range |

On 2026-11-04 the realism ruling was appended as row 80 — the next free
number, which is the correct thing to do against a file ending at 79. The
effect is that a dozen dangling citations stopped dangling and started
resolving **to the wrong rule**. A reader who follows `LLKSweeper.cpp:317` to
row 80 now finds a ruling about pendulum physics where they expected one about
detectability.

`FLEET.md` says: *"Cite rows by number. 'Row 5' is checkable; 'the light rule'
is not, and a paraphrase is how a locked decision gets quietly softened."* A
citation that resolves to the wrong row is worse than a paraphrase, because it
is checkable and wrong.

### Why it happened, and it was my debt

Row 80 was not an unlucky collision. It was **reserved** and never paid:

- `docs/HANDOFF-CODEX-LANTERN-LEVELS.md:165` — *"Row 80 itself, in
  `docs/DECISIONS.md` | **Claude**, not yet written"*, with the line below it:
  *"Row 80 is referenced by this change and does not exist yet."*
- `docs/PREREQS-AUTHORED-MAP-SEAM.md:359` — *"**Row 80** in
  `docs/DECISIONS.md` — mine, still unwritten"*

So a previous turn of mine claimed the number, told Codex the code could cite
it, wrote the citations into eight files' worth of comments and tests, and
never wrote the row. The realism ruling then landed on the next free number,
which was correct against a file ending at 79 and would have been harmless if
80 had been occupied as promised.

### How it was resolved

Chris's instruction: *"Fix the collision and sort out issues."* The twelve
`Source/` citations are the older claim and `Source/` is Codex's lane, so the
citations could not move. The ruling moved instead:

| Row | Content |
| --- | --- |
| **80** | lantern levels — detectability authored per level, enemy reach continuous in it, gameplay resolves to the named state while visuals lerp. **Reconstructed** from the citations that depended on it; `LLKPerception.cpp:97` calling continuous detectability *"the whole of row 80"* is what fixed its content |
| **81** | the realism ruling, renumbered from 80 |
| **82** | Chris's cosmetic/gameplay split, the ruling this document audits |

All twelve `Source/` citations now resolve to the rule their comments
describe, and no source file was touched to achieve it. Doc citations that
meant the realism ruling were renumbered to 81 (`FINDINGS-LANTERN-PHYSICS.md`,
`FINDINGS-LANTERN-ANCHOR-ORIGIN.md`, `HANDOFF-CODEX-LANTERN-ANCHOR.md`);
citations that meant lantern levels (`LIGHTING-IN-UNREAL.md`) were left alone.

**Audited for others:** every `row N` citation in `Source/`, `tests/` and
`docs/` was compared against the rows that exist. 73 distinct numbers are
cited and all 73 now resolve. Row 80 was the only dangling one.

### Two fixes made while here

- `FINDINGS-LANTERN-PHYSICS.md` was titled *"three places the model
  disagrees"* over a table listing five, and row 80's reasoning repeated the
  undercount. Both say five.
- Row 81 said the look-pitch travel is 88 cm. It is **about 84** — the 42 cm
  forward offset of `LanternAnchor` rotating through an unclamped pitch range
  (`z = x·sin θ + z·cos θ`, so +42 at the top and −42 at the bottom; no
  `ViewPitchMin`/`Max` is authored, so UE's ±89.9 applies). 88 was the capsule
  half-height, carried over from the retracted 240 cm finding.

## 5. Summary against the ruling

| Ruling clause | State |
| --- | --- |
| pendulum stays cosmetic | holds by attachment order, not by convention |
| gameplay origin from the stable mount | holds (`GetGameplayLightLocation`) |
| gameplay radius from the stable mount | holds in the light field; **violated** in the revive pulse (2b) |
| detection unaffected by the swing | holds; queried at the keeper's position |
| relic inspection unaffected | holds, plus a two-sample debounce |
| infrastructure interactions | "Present" reads no light; the revive pulse is the violation |
| gameplay aim from the pitch arm | no gameplay aim exists to govern |

No code, data or test file was changed by this audit.

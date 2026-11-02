# Handoff to Codex — the Boswell's visual pass

**RELEASED BY CHRIS, 2026-11-02.** Four items, in order. Everything below was
read live through the MCP bridge or measured, not inferred from config.

The gate is open: `r.DynamicGlobalIlluminationMethod` = **1** (Lumen live, not
inert) and `r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange` = **0**
(legacy luminance, so the light lab's ladder stands). The second is now pinned
in `Config/DefaultEngine.ini` because it was inherited rather than authored and
0 is not the engine default.

---

## 1. ~~The lantern core material is `MSM_Unlit`~~ — REFUTED 2026-11-02

**Wrong, and Codex had the measurement.** The LightLab already tested this
exact material under live Lumen: at `CoreEmissive 0.5` it is a visible amber
source with essentially no bounce, and at **5–20 it does light nearby
surfaces** — clipping and desaturating toward white as it climbs.

An Unlit material that produces bounce at 5–20 is not being excluded from
Lumen's scene. **So the shading model was never the cause**, the emissive
value is the axis, and flipping to Default Lit would have been a blind change
that likely looked *worse* — the material has no Base Color or Roughness
wired, so the lit path has nothing to shade.

**The actual cause was more boring and more likely: the material was not
assigned to the held lantern mesh at all.** Fixed by Codex, verified in PIE,
and visible in the tree at `LLKCharacter.cpp:147` —
`Glass->SetMaterial(0, LanternCoreMaterial.Object)`.

**The lesson, because it is the second instance in two days.** I read that
material's shading model, emissive value, parent, blend mode and two-sidedness
— and never asked whether anything *used* it. That is the asset-level form of
*a broadcast with no subscriber is silence that compiles*: an inspection that
was entirely sound, pointed at the wrong question. A material nobody assigned
looks identical to a material that does not work.

The hedge in the original was right and is worth keeping as the reason this
cost nothing: it said the engine rule was unverified and that the check cost
one property flip. Codex's measurement *was* that check.

**What remains, which is Codex's and not a material swap:** a carried lantern
needs three things, and all three components already exist —

| Job | Component | State |
| --- | --- | --- |
| Amber emissive core | `MI_LLK_LanternCore_Working` on the glass | assigned now |
| Short near-field fill — hands, floor, shelf lip | `LanternCoreGlow_Temporary` (`UPointLightComponent`) | exists, untuned |
| Hooded directional wash | `ULLKLanternLightComponent : USpotLightComponent` | 25 cd, 110° |

So this is a **tuning** item, not a build item. Keep the core restrained — it
should not become a small white floodlight — and tune the near-field response
against the floor and the shelf lip.

Firewall check, since the near-field fill is a second light on the player:
`GetLitLightSources()` finds `ULLKLanternLightComponent` by class, and
`LanternCoreGlow` is a plain `UPointLightComponent`, so it is visual-only and
cannot move detectability or relic legibility. Verified.

### The original claim, kept for the record

**This is the strongest candidate for "the lantern effect isn't there", and it
means `FINDINGS-EMISSIVE-LIGHTS-NOTHING.md` recorded the wrong cause.** That
document blames the absence of Lumen. Lumen is now verifiably live and the core
still lights nothing.

Read from the asset:

| | |
| --- | --- |
| `MI_LLK_LanternCore_Working` | `CoreEmissive` = **0.5** |
| parent `M_LLK_LanternCore` | `ShadingModel` = **`MSM_Unlit`**, `BLEND_Opaque`, `TwoSided` |

Unlit is authored on the parent, so it is not an instance override.

**The claim I am NOT making:** I have not verified the exact rule by which
Unlit surfaces are excluded from Lumen's scene representation, and I am not
going to assert an engine behaviour I cannot test from here. What is certain
is that Unlit is not a dependable emissive GI contributor, and that the check
costs one property flip — a lit shading model with the emissive routed, then
look at whether the core throws any bounce at all.

Raising `CoreEmissive` from 0.5 is the second half. 0.5 is a low number for a
surface meant to read as the hottest thing in a dark room.

## 2. The same material is on the Sweeper's beacon — and fixing one fixes both

`SweeperBeaconHousing_Temporary` uses `MI_LLK_LanternCore_Working`. So the
housing does not glow, which is **why the beacon needed a 300 cd point light to
read at all.**

The beacon as it stands breaks the fixture contract Chris ratified this
morning, by roughly 600x:

| | |
| --- | --- |
| `StateBeacon`, pursuing | **120–300 cd**, 260 cm attenuation |
| working | 70–120 cd |
| idle | 10 cd |
| the keeper's lantern | **25 cd** |
| `FINDINGS-FIXTURE-LIGHT-BAND.md` inspection ceiling | **below 0.5 cd** |

A 300 cd moving light lights a price card directly, which four measured levers
concluded was the thing that cannot be allowed. And because the beacon is
visually bright but mechanically absent from the gameplay light field, a player
will see a relic's detail while the prompt still says *needs light*.

**So do item 1 first.** If the housing actually emits, the beacon can drop into
the orienting band and the contract violation disappears rather than being
traded off.

The gameplay firewall is intact and must stay that way: `GetLitLightSources()`
is registration-based (distress pulses plus registered lantern components) and
does not scan the world, so no beacon value changes detectability or relic
legibility. Verified.

## 3. The ceiling fixtures light the ceiling

Captured from `(-300, -350, 170)`, yaw 45, FOV 90. Both fixtures pool their
light **on the ceiling tile directly above themselves** — one warm, one green —
and the floor is essentially black. The green also confirms this file's
predecessor: *"Replace the present cream/green emergency-light placeholder."*

Fixtures that illuminate ceiling tiles orient nobody. This fails the ratified
contract in the opposite direction from the beacon: *"Fixtures orient; geometry
keeps inspection detail dark."* One thing is far too bright to be legal and
these are aimed at nothing.

**What this observation does NOT establish:** how bright the room should be.
There is no `PostProcessVolume` yet, so the editor's own exposure is deciding
what we see. *Where* light lands is geometric and safe to conclude; *how bright
it looks* is item 4's job.

## 4. Seed the PostProcessVolume

The do-not-regress rule said no PPV "until the exposure decision is made." It
is made.

- One volume, **priority 1, unbound**.
- `AutoExposureMinBrightness` **= `AutoExposureMaxBrightness` = 4.0**, pinned
  equal so exposure cannot drift.
- Those fields are **legacy luminance**, confirmed live. `L = 0.125 x 2^EV100`,
  so 4.0 is the measured EV100 5 optimum.
- Measured separation was 175.8 at 0% clipped against 104.2 at the current
  lock of 1.0, where the threshold is 100 — so the current lock is *marginal*,
  not merely suboptimal.

Seed it there and judge it by eye in the dressed aisle. The orientation half of
the sweep was never run and does not need to be: it was always waiting on a
look rather than a measurement, and the question is Chris's — *navigable but
uneasy* against *nothing is there*.

---

## One judgement call that is yours, not mine

`tests/enemies.test.mjs` is **red at 49/50**, and was before any of today's
nav work. The fear pass calls `SetCollisionEnabled`, which trips an existing
guard forbidding `SetCapsuleSize`, `SetCollisionResponseToChannel`,
`GravityScale` and `SetCollisionEnabled` in the pawn, on the grounds that *a
Sweeper's real dimensions are a question about doorways and shelf gaps, which
is map work*.

That intent is about the **capsule**. `NoCollision` on a decorative mesh does
not change the pawn's dimensions, so the guard looks too broad for this case.

**I did not narrow it.** Editing a guardrail to turn a suite green is how a
guardrail gets deleted by someone who never knew it was one. It wants the
judgement of whoever wrote it. Note also that the pre-commit hook runs only
`tests/lighting.test.mjs`, so this red would not have stopped a commit.

## Do not regress

- **Intensity is visual-only.** `data/lantern.json` says so in its own note:
  detectability and gameplay reach are authored separately. Tune the look
  freely; nothing may touch `GetGameplayRadiusCm()`.
- **The navmesh is baked and the beat works.** `73c659e`. A Sweeper filed a
  relic unaided, 2.5 s start to finish. Do not casually re-bake or re-add
  cosmetic components without `SetCanEverAffectNavigation(false)` —
  `tests/enemies.test.mjs` now guards the C++ ones, and it was proven red
  before being called done.
- `Content/LLK_Boswells_FirstRun.umap` has one owner under row 41. Binary does
  not merge.
- **Protect the feel.** Chris's only unprompted praise all day was the bob,
  the stride and the bulk. That came out of movement values and a placeholder
  mesh. Replacing the rig should not cost it.

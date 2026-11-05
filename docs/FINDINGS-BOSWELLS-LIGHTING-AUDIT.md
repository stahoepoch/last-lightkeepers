# What is actually lighting Boswell's, and the aisle is about ten times over

> **Status: OPEN, and the headline number is computed rather than measured.**
> Read 2026-11-02 from the live editor through the MCP bridge, on Chris's
> *"let's at least get real lighting from environmental systems"*. The
> inventory is fact. The comparison against the lab is inverse-square
> arithmetic from authored candelas, not a camera reading — see
> **How solid this is** at the end before acting on it.

## The inventory

Nine light actors in `/Game/LLK_Boswells_FirstRun`, read off their components:

| Actor | Label | At | Intensity | Atten | Source R | Shadows | Indirect |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `DirectionalLight_0` | `WL_TEMP_OVERLIT_GEOMETRY_RIG` | (0, 0, 500) | 50 | — | — | off | 1 |
| `PointLight_0` | `WL_EMERGENCY_LIGHT_A` | (−650, −250, 530) | **18 cd** | 1800 | 40 | on | **0.25** |
| `PointLight_1` | `WL_EMERGENCY_LIGHT_B` | (100, 0, 530) | **18 cd** | 1800 | 40 | on | **0.25** |
| `PointLight_2` | `WL_EMERGENCY_LIGHT_C` | (850, −250, 530) | **18 cd** | 1800 | 40 | on | **0.25** |
| `PointLight_3` | `BSW_LOADING_DOCK_EXIT_LAMP` | (0, 790, 500) | 12 cd | 1200 | 30 | on | 1 |
| `SpotLight_3`–`_6` | `BSW_Emergency_Aisle_1`–`4` | y 0, z 560, x −900 / −300 / 300 / 900 | **75 cd** | 1000 | 18 | on | 1 |

All local lights are pinned to **Candelas**, which is the one unit mistake
this project has already paid for (`_intensityWas6`) and nobody repeated.

## Two things are already right, and they are the article's own technique

Worth saying first, because the rest is a correction.

- **`IndirectLightingIntensity` is 0.25 on the three emergency points.** The
  Lumen guidance Chris circulated names that value as a direct noise control —
  the higher it is, the noisier the bounce. Somebody had already turned it
  down on the fixtures that fill the room.
- **`SourceRadius` is 40 on those same points**, against a default of 0. Also
  straight out of the same practice: a wide source softens contact noise
  instead of producing a hard, speckled pool.

Neither of those was asked for by any document in this repo. They are good
instincts that happen to match published practice.

## The aisle spots are the outlier

`docs/FINDINGS-FIXTURE-LIGHT-BAND.md` measured, under this renderer:

- **orientation floor** — the dimmest a run can be and still be legible from
  40 studs: **2 cd** from a ceiling troffer at 290 cm;
- **inspection ceiling** — the brightest a fixture can be with a price card
  still *not* readable: **below 0.5 cd**, at 243 cm to the card.

Converting both to illuminance at the surface, `E = I / d²`, and doing the
same for what Boswell's actually authors:

| | lux at the surface | against the lab |
| --- | --- | --- |
| lab orientation floor (2 cd at 290 cm) | 0.238 | — |
| lab inspection ceiling (0.5 cd at 243 cm) | 0.085 | — |
| **aisle spot on the floor** (75 cd at 560 cm) | **2.392** | **10.1× the orientation floor** |
| **aisle spot on a shelf label** (75 cd, z 150, 100 cm off axis) | **4.211** | **50× the inspection ceiling** |
| emergency point on the floor (18 cd at 530 cm) | 0.641 | 2.7× the orientation floor |

**So the four aisle spots are the problem and the three emergency points are
roughly in band.** 2.7× the dimmest legible value is a defensible authoring
choice — a floor you can read without squinting. 10× is a lit shop.

**The 55° cone does not rescue it.** Candelas are luminous intensity *per
steradian*, so widening or narrowing a spot's cone changes how much floor is
covered and not how bright the covered part is. The on-axis 75 cd is 75 cd at
any cone angle. (This is the same principle recorded as a prediction against
`HANDOFFS.md` M5, and it cuts both ways: it is why the authored 110° lantern
is probably not dimmer than the 72° one that was measured.)

**What it means for the contract.** `FIXTURE-LIGHT-CONTRACT.md`, ratified
earlier the same day, is *"Fixtures orient; geometry keeps inspection detail
dark."* A label under these fixtures receives roughly fifty times the
illuminance at which the lab found a price card already readable. If that
holds up under a camera, the only fixtures in the map break the contract's
central rule, and the shelf lips added in `bb50592` are being asked to do
something no lip can do.

## One orphan, inert, and the fourth of its kind

`WL_TEMP_OVERLIT_GEOMETRY_RIG` is a DirectionalLight at 50, in a sealed
interior, with **`bAffectsWorld: false`** — so it contributes nothing today
and is not a defect. It is left alone here deliberately: the name says it is
somebody's rig for looking at geometry, and deleting another writer's tool
without asking is not this project's way.

It is worth flagging because it is the **fourth** `WL_` orphan to turn up this
week, after `WL_TempRobotBody` carving the navmesh, three `WL_` point lights
doing most of the room's lighting, and `WL_DarkPocket_Partition` putting a
wall 20 cm from the stock bay. An inert light named `TEMP_OVERLIT` is one
checkbox away from flooding a horror level, and the pattern is now plain
enough to be worth a sweep of its own.

## How solid this is

**The inventory is read from the live editor and is fact.** The comparison is
not a measurement, and three things weaken it:

- It is **inverse-square only** — no cone attenuation, no cosine falloff at
  the surface, no albedo, no exposure. Real values will be lower than these.
- The lab's figures were **camera-measured luminance**; these are **computed
  illuminance**. Comparing them is only fair as a ratio, because both sides
  use the same model from authored candelas.
- The lab bay is 400 cm wide with a 290 cm ceiling; Boswell's is a store with
  a 560 cm ceiling. Geometry differs, and bounce with it.

So treat 10× and 50× as **orders of magnitude, not readings**. What they are
good for is deciding that this is worth a camera, and what they are not good
for is choosing the replacement number. The honest next step is the fixture
ladder re-run in the real room — which needs the editor window frontmost, per
`LIGHTLAB-PROTOCOL.md` section 9, and that is the same thing blocking check 4.

## Addendum, 2026-11-04 — the one correction available without a camera

**Still OPEN; the headline survived.** This is editor-free work on the audit's
own self-flagged weakness. Two things came out of it: the arithmetic above
reproduces exactly, and the model was missing a term that only ever cuts
downward — but not enough to change any conclusion.

**First, the numbers check.** Recomputing `E = I/d²` independently from the
inventory gives 0.238, 0.085, 2.392, 4.211 and 0.641 lux — the audit's five
figures to three decimal places. Whatever is wrong with the comparison, it is
not the arithmetic.

**Second, the missing term: UE's attenuation-radius windowing.** The audit says
it is "inverse-square only." That is not quite the engine's model. A UE local
light with inverse-squared falloff multiplies the inverse-square term by a
window that drives the contribution to **zero at `AttenuationRadius`**, not just
to something small:

```
Falloff = saturate(1 − (d/R)⁴)²
```

This matters here because **the two sides of the comparison sit at very
different fractions of their own radius.** The lab troffer has R 2000 and works
at 243–290 cm, which is d/R ≈ 0.12–0.15 — the window is 0.999, i.e. absent. The
Boswell's aisle spots have R **1000** and light a floor **560** below them, which
is d/R 0.56, where the window is 0.813. So the correction is almost entirely
one-sided:

| | audit's lux | windowed | ratio before → after |
| --- | --- | --- | --- |
| lab orientation floor (2 cd, 290 cm, R 2000) | 0.238 | 0.238 (−0.1%) | — |
| lab inspection ceiling (0.5 cd, 243 cm, R 2000) | 0.085 | 0.085 (−0.0%) | — |
| **aisle spot on the floor** | 2.392 | **1.944** (−18.7%) | 10.1× → **8.2×** |
| **aisle spot on a shelf label** | 4.211 | **3.949** (−6.2%) | 50× → **47×** |
| emergency point on the floor | 0.641 | 0.631 (−1.5%) | 2.7× → 2.7× |

**So the conclusion stands and is now better grounded than it was.** 8.2× is not
10× and it is not in band either; the three emergency points stay where the
audit put them. What changed is that the headline is no longer resting on a
model the engine does not use.

**That the windowing applies at all is an inference, and here is its basis.** UE
exposes physical light units only on lights using inverse-squared falloff —
switch the falloff off and intensity becomes unitless. The audit read *"All
local lights are pinned to Candelas"* off the live components, and this project's
own code pins the same unit explicitly (`LLKLanternLightComponent.cpp:41`,
`LLKCharacter.cpp:164`). A light reporting candelas is therefore a light using
inverse-squared falloff, so the window is on every one of the nine. **No project
code sets `bUseInverseSquaredFalloff` or `IntensityUnits` on a map light** — the
map authors those — so this is reasoned from the engine's unit rule rather than
read, and a camera would settle it in one frame.

**One thing the flat ratio hides, and it cuts toward the audit's case rather
than against it.** Because the window grows with d/R, the aisle floor is not
uniformly 8.2×. The spots sit at x −900/−300/300/900, so a floor point midway
between two of them is 635 cm from the nearer one, where the window is 0.701
against 0.813 directly beneath. The pools under the fixtures are therefore about
**16% brighter relative to the gaps** than the naive model says — more pooled,
more gradient, a stronger dark edge. For a horror level that is arguably the
good direction, and it is also exactly the *"hard dark edge at the attenuation
radius"* that `FINDINGS-FIXTURE-LIGHT-BAND.md` recorded at 250 cd. Choosing the
replacement number still needs the camera; this only sharpens what it will find.

**What this addendum does not touch.** No map, no data, no light actor, and not
the orphan `WL_TEMP_OVERLIT_GEOMETRY_RIG`, which is still somebody else's rig and
still inert. Check 4 and the fixture-ladder re-run are still blocked on an
editor window, per `LIGHTLAB-PROTOCOL.md` §9.

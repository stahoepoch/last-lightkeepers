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

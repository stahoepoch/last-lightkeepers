# LLK_LightLab — the build and the measurement protocol

Written 2026-10-31 by Claude, with the editor down. `docs/FIXTURE-LIGHT-CONTRACT.md`
locks the rule that ambient fixtures *"may help players navigate, but must never
look bright enough to read a relic under."* **That is two thresholds and a gap,
and nothing in the project has measured any of them.** This page is the
instrument, written so the measurement does not have to be re-derived the next
time the editor comes up.

Nothing here is a finding. Every number below is an *input* read off an
authored file or an existing asset. The three answers the lab exists to produce
are listed at the end and are still blank.

---

## 1. The method rule that makes or breaks the whole thing

**The lab must carry an identical copy of DockBay's exposure setup.** A ladder
measured under a different clamp transfers nothing and — this is the dangerous
part — *looks exactly like a successful measurement*. Same disease as every
entry in `docs/TOOLING-TRAPS.md`: a true answer to a question next to the one
being asked.

So the values were read off DockBay's own volume rather than assumed. Read
2026-10-31 from
`/Game/LLK_DockBay.LLK_DockBay:PersistentLevel.PostProcessVolume_UAID_D843AEC613C1711003_1770798619`:

| Property | Value |
| --- | --- |
| `Priority` | 1 |
| `bUnbound` | true |
| `bEnabled` | true |
| `BlendRadius` | 100 |
| `BlendWeight` | 1 |
| `bOverride_AutoExposureMinBrightness` | **true** |
| `bOverride_AutoExposureMaxBrightness` | **true** |
| `autoExposureMinBrightness` | **1.0** |
| `autoExposureMaxBrightness` | **1.0** |
| `autoExposureMethod` | `AEM_Histogram` |
| `autoExposureBias` | 1.0 — **but `bOverride_AutoExposureBias` is false**, so this is the inherited default, not an authored value. Do not copy it as an override. |

**Min == max == 1.0, so exposure is pinned, not merely bounded.** That is what
makes a candela ladder meaningful at all: the camera cannot quietly
re-normalise between runs and hand back a flat result.

Two further readings worth carrying, because they change how light renders and
would invalidate a comparison if the lab differed:

- `bMegaLights: true` (overridden on this volume). MegaLights changes how many
  local lights actually light the scene. The lab must match it.
- Every other exposure and tonemapper field is **not** overridden, so the lab
  inherits project defaults for them — which it will do automatically *provided
  the lab contains exactly one unbounded volume and nothing else competes.*

**One volume, not two.** An earlier pass of mine flagged *two* unbounded
exposure volumes in DockBay as an order-dependence risk. Enumerating the
persistent level today returned exactly **one** `PostProcessVolume`, at priority
1 and unbound — consistent with Codex having consolidated them in `be52f3f`.
The risk I named appears to be closed; it is recorded here because the earlier
note is still on file and should not be read as current.

## 2. Units — one conversion authority

`LLKUnits::StudsToCm = 28.f` (`Source/LLK/Public/LLKUnits.h:47`). Row 61: one
conversion authority. Every camera distance below is derived from it, not
guessed.

| Camera position | Studs | Centimetres |
| --- | --- | --- |
| Bay A near | 2 | 56 |
| Bay A mid | 4 | 112 |
| Bay A far | 8 | 224 |
| Bay B far end | 40 | 1120 |

## 3. The lantern is a SPOT, and this is its authored shape

`ULLKLanternLightComponent` derives from `USpotLightComponent`. **The 6cd
reading that looked right was taken on a POINT light and does not transfer** —
that is the untested matrix cell and the reason Bay C exists.

Read from `data/lantern.json` (`light` block) and
`Source/LLK/.../LLKLanternLightComponent.{h,cpp}`:

| Parameter | Value | Source |
| --- | --- | --- |
| `IntensityUnits` | `ELightUnits::Candelas` — **pinned in code** | `LLKLanternLightComponent.cpp:40` |
| `Intensity` | 250 cd (current authored value) | `data/lantern.json` → `light.intensity` |
| `AttenuationRadius` | 30 studs → **840 cm** | `light.radiusStuds`, via `DistanceFromStuds` |
| `OuterConeAngle` | **72°** | `light.coneAngleDegrees` |
| `InnerConeAngle` | 72 × 0.24 = **17.28°** | `InnerConeFraction` (`.h:190`) — edge softness, deliberately code-side |
| `LightColor` | linear (1.0, 0.82, 0.56) | `light.warmColor` |
| `CastShadows` | true | `light.castsShadows` — load-bearing, not cosmetic |

A plain `ASpotLight` configured to exactly this table is the right instrument:
it reproduces the authored shape without pulling gameplay code into a
measurement map. **Pin `IntensityUnits` to Candelas explicitly.** An unpinned
intensity is precisely how `6` came to mean "invisible" rather than "6 cd" —
`data/lantern.json`'s `_intensityWas6` note records that incident, and a lab
that repeats it measures nothing.

## 4. The build

One sealed box. **No landscape, no World Partition** — so the base is
`/Engine/Maps/Entry`, duplicated to `/Game/LLK_LightLab`. Checked: `Entry`'s
only content dependency is `DefaultMaterial`, so it carries no sky, no floor,
no directional light and no landscape. (`Template_Default` drags in a sky
sphere, a volumetric cloud and a procedural grid material; `OpenWorld` is World
Partition. Neither is usable.)

Three bays off a central spine, all dimensions in cm:

| Bay | Purpose | Box | Fixture position |
| --- | --- | --- | --- |
| **A** | inspection | X 0→400, Y −200→200, Z 0→300 | ceiling centre, (200, 0, 290) |
| **B** | orientation | X 0→1120 (40 studs), Y −250→250, Z 0→300 | ceiling midpoint, (560, 0, 290) |
| **C** | lantern | X 0→900, Y −300→300, Z 0→300 | spot at (50, 0, 160), pitch −10° |

- **Bay A** carries a relic-sized prop with a **price card bearing legible
  text** on its vertical face, at X=350 facing −X. Camera at X = 294 / 238 /
  126, i.e. 2 / 4 / 8 studs off the card. Text as a `TextRenderActor` at
  price-card character height (~8 cm) — real glyphs, so "readable" is a
  judgement about reading and not about seeing a grey rectangle.
- **Bay B** is a 40-stud straight run with **two gondola-height blockers
  offset from each other**: ~140 cm tall (store gondola height), one at
  (380, −80), one at (760, +80). The question is whether run direction and
  blocker arrangement are legible from the far end.
- **Bay C** aims the spot down the room from keeper eye height so the cone
  lands on floor and far wall, which is where "carried lantern" and
  "floodlight" actually part company.

**The bays are sealed from each other, not left open onto the spine.** Ladders
1 and 2 light Bay A and Bay B at the same candela in the same run, so an open
doorway would let each bay contribute to the other's reading and every number
would be a sum of two fixtures. The spine is built as the connecting geometry;
the bay walls are closed. This is a deliberate departure from how a store is
actually laid out, and it is the right one for an instrument.

### One fixture per bay, and why

A real aisle has a *row* of troffers. The lab uses **exactly one fixture per
bay**, at equal ceiling height, because the inspection ceiling (Bay A) and the
orientation floor (Bay B) have to land on **one shared candela axis** to be
comparable at all. Give Bay A one fixture and Bay B three and the "band" is
two different units pretending to be one number.

**So every result below is a per-fixture candela figure, not an installed-aisle
figure.** Converting it to a lighting plan means deciding spacing separately.
That limit is the price of a comparable axis and is stated rather than buried.

## 5. The ladders

Each run is **one fixture type at one intensity**, shot from every camera
position in that bay.

| # | Fixture | Bays | Candela steps |
| --- | --- | --- | --- |
| 1 | Fluorescent troffer | A, B | 2, 6, 15, 40, 100 |
| 2 | Neon tube | A, B | 2, 6, 15, 40, 100 |
| 3 | Lantern spot | C | 6, 25, 100, 250 |

Ladders 1–2: 4 camera positions per step (A near/mid/far, B far end) × 5 steps
× 2 fixtures = **40 shots**. Ladder 3 adds **8**. Forty-eight in total.

**Record per shot: fixture, candela, attenuation radius, camera distance.**
A shot without its parameters recorded is not data — it is a picture.

## 6. The three answers — still blank

1. **Inspection ceiling.** Highest candela at which the Bay A price card is
   still **not** readable at 2 studs. → *unmeasured*
2. **Orientation floor.** Lowest candela at which Bay B's run and blockers are
   legible from 40 studs. → *unmeasured*
3. **The band** between them — **or the finding that there isn't one.** →
   *unmeasured*

Ladder 3 answers separately: the candela at which a spot reads as a **carried
lantern** rather than a floodlight. → *unmeasured*

**If floor > ceiling there is no band, and that is the more valuable result.**
It means the contract cannot be satisfied with uniform fixtures and the answer
is directional ones — downlights that wash the floor without lighting a
vertical card face. That finding must be reported plainly rather than tuned
until something fits. A band produced by quietly adjusting the rig until the
numbers overlap would be the worst possible outcome here, because it would read
as permission.

## 7. What stopped the first attempt

The bridge was up and answering, the world was verified as `LLK_DockBay`, and
the DockBay exposure values in section 1 were read. Then the editor was closed
and restarted while the lab was being planned, and the Model Context Protocol
server **does not come back with it** — `ModelContextProtocol.StartServer` is a
manual console command (see `docs/TOOLING-TRAPS.md` entry 4). Nothing was built
and nothing was written to `Content/`.

Codex owns `LLK_DockBay` for an art pass (row 41, one owner per `.umap`), so
the lab is a separate map by construction and no part of this protocol touches
DockBay beyond reading the volume above.

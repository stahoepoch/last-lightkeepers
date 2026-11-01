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

**But 1.0 is not EV100 here, and that matters.** Queried from the live editor:

```
r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange = 0
```

The project sets this nowhere, so it sits at the engine default of **0 —
legacy range**. The cvar's own help says it "change[s] the
PostProcessSettings.Exposure.MinBrightness, MaxBrightness, HistogramLogMin and
HistogramLogMax to be expressed in EV100 values instead of in Luminance". With
it off, these two 1.0s are **legacy luminance values, not EV100 stops**.

Anyone who reads "min = max = 1.0" as EV100 and types 1.0 into a project where
extended range *is* on has set a wildly different exposure and will get a
confidently wrong ladder. Copy the numbers **and** the cvar, or copy neither.

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

## 5a. Step zero, every run

Before stepping a single candela:

1. **Negative control.** All lights to 0 cd, capture. **It must be black.** If
   the room is visible, stop — the box has an ambient floor and the ladder can
   only measure that. See section 7.
1b. **Positive control, and do not skip this one.** Same pose, one fixture at
   an absurd value — **5000 cd** — and capture. It must look obviously, wildly
   different. On 2026-10-31 it was *pixel-identical* to the 0 cd frame, which
   is how it emerged that the captured viewport had no lighting in it at all.
   Zeroing proves the floor; only a maximum proves the dial is connected.
2. **Capture-liveness proof.** Capture, make a change that alters most of the
   frame (a light from 0 to 100 cd), capture again, and compare the two by hash
   or byte size. Identical means the capture path is dead; see
   `docs/TOOLING-TRAPS.md` entry 7.
3. **Camera-liveness proof.** Move the camera a long way, capture, and require
   the image to change. On 2026-10-31 the rendered viewport ignored
   `SetCameraTransform` for many calls at a time while `GetCameraTransform`
   cheerfully reported the new pose — so a shot can carry the right recorded
   distance and the wrong actual one.

A ladder whose shots are all the same frame, or all the same ambient floor,
produces a beautifully consistent dataset that means nothing, and it reads as a
clean result.

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

## 7. Build status — the lab exists

`Content/LLK_LightLab.umap` is **built and saved**, 44 actors. Second attempt,
2026-10-31.

- Base is `/Engine/Maps/Templates/Template_Default`, **not** `Entry` as section
  4 planned. Duplicating `Entry` into `/Game/` produces a map that cannot be
  saved: `Illegal reference to private object: 'Model /Engine/Maps/Entry.Model2'
  … (private object belongs to an external map)`. The duplicate keeps a pointer
  to the engine map's BSP model. `Template_Default` duplicates cleanly.
- All seven exterior sources from the template were deleted: `DirectionalLight`,
  `SkyAtmosphere`, `SkyLight`, `ExponentialHeightFog`, `VolumetricCloud`, the
  sky-sphere mesh and the template `Floor`. The box is genuinely sealed and the
  only light in it is the fixture under test.
- Three sealed rooms plus the spine, 24 slabs, at the coordinates in section 4.
  Bay A carries the prop and price card, Bay B the two offset gondola blockers.
- **Exposure verified field-by-field against DockBay**, not assumed: priority 1,
  unbound, `bOverride_AutoExposureMinBrightness` and `…MaxBrightness` both true,
  both values 1.0, `AEM_Histogram`, MegaLights on and overridden, and
  `autoExposureBias` left at 1.0 **un-overridden** exactly as DockBay has it.
- Fixtures: two `RectLight`s (Bay A and Bay B ceilings) and one `SpotLight`
  (Bay C) carrying the authored lantern shape from section 3. All three are
  pinned to `Candelas` and saved at intensity 0, so the map opens dark.

### Surfaces, and why they are a recorded parameter

The first render was a uniform white blowout: the engine's basic-shape material
is far too bright, and a lab made of it is a lightbox, not a store. **Surface
reflectance is as consequential as candela**, and a threshold measured in a
white box would be far too low and would transfer nowhere.

Room, prop and blockers are therefore `/Engine/EngineMaterials/DefaultMaterial`
(mid-grey); the price card keeps the lighter `BasicShapeMaterial`, which is what
a paper card should be. Both are engine assets, so **nothing beyond the lab map
was written to `Content/`**. Their absolute albedo was not measured — the asset
paths are recorded instead, and any future reading has to cite them.

### Step zero: the negative control, and it FAILED

**Run this before every ladder. It is not optional and it is not a formality.**

Set every light in the lab to **0 cd** and capture. A sealed box with no sky,
no directional light, no skylight and no fog must render **black**. If anything
is visible, the box has an ambient floor, and every candela reading taken in it
is a measurement of that floor rather than of the fixture.

`docs/lightlab/control-00cd-all-lights-off.png`, 2026-10-31: **the room is
plainly visible with all three fixtures at zero.** Walls, floor, ceiling and
both of Bay B's gondola blockers are legible. The box is not dark.

So the readings below are **void**, and they are recorded only to show what the
contamination looks like:

| fixture | candela | attenuation | bay | camera | apparent result |
| --- | --- | --- | --- | --- | --- |
| troffer, RectLight 120×60, cool white | 100 cd | 2000 cm | A | 2 studs | card readable |
| troffer, RectLight 120×60, cool white | 2 cd | 2000 cm | A | 2 studs | card readable |
| **none — all lights 0 cd** | **0** | — | B | 40 studs | **run and blockers legible** |

"Readable at 2 cd" looked like a finding — an inspection ceiling below the
bottom of the ladder — and it is not one. **The card is readable with the
lights off.** Reporting that 2 cd reads the card would have been reporting the
ambient floor and calling it a fixture threshold.

### It is not a leak. The captured view has no lighting in it at all.

Chased in the order "most likely and fastest to rule out", and the answer
arrived from a measurement rather than from looking at a picture.

**The decisive test.** Hold the camera still, set Bay B's own fixture to
**5000 cd**, and compare against the same pose at **0 cd**:

```
0 cd  vs  5000 cd   mean absolute difference = 0.0   max difference = 0
```

**Pixel-identical, across a 2500-fold change in intensity.** Earlier, the 2 cd
and 100 cd Bay A frames likewise shared a mean of 127.56 and a max of 255 to
the last decimal.

So this was never an ambient floor that happened to drown the fixtures. **The
fixtures contribute nothing whatsoever to the captured image.** What is being
captured is albedo with no lighting applied — which is why the control looked
flat and shadowless, why the card was "readable at 2 cd", and why the very
first render looked blown out (that was simply a bright material at full
albedo, not an exposure result).

Per the bar set for this control — *if you cannot get the card unreadable at
all, stop and report it* — this is that outcome, and the cause is the viewport,
not the lab.

### What was ruled out, by enumeration rather than assumption

| Suspect | Finding |
| --- | --- |
| Template lights left behind | **None.** The only light-type actors in the level are `RectLight_0`, `RectLight_1`, `SpotLight_0` — the three placed deliberately. No DirectionalLight, SkyLight, SkyAtmosphere, VolumetricCloud or Fog actor survives. |
| Copied PPV carrying light | **Clean.** `bOverride_AmbientCubemapIntensity` false, `ambientCubemap` None, `bOverride_IndirectLightingIntensity` false, `lumenSkylightLeaking` 0. Nothing light-bearing was copied from DockBay. |
| Fixtures disabled | **No.** All three read `bAffectsWorld` true, `bVisible` true, `IntensityUnits` Candelas. |
| Emissive materials | Not separately tested, and **moot**: an emissive room would still change when a 5000 cd light is added. Nothing changed. |

### The viewport the tools reach is not the viewport being captured

This is the thread tying together every failure in this session. Three
independent controls all target a viewport that is **not** the one
`CaptureEditorImage` returns:

- `SetCameraTransform` reports success and `GetCameraTransform` reads the new
  pose back, while the captured image keeps the old one — sometimes for many
  calls, then catching up without warning.
- `viewmode lit`, executed through the editor's own console (`Cmd: viewmode
  lit` is in the log), changed the captured image by **nothing**: mean 97.20
  and max 130 before and after.
- `CaptureViewport`, the one tool that takes an explicit pose *and* hides UI,
  hangs to the 300-second timeout every single time.

**So the fix is a human one and it is small:** set the *visible* level viewport
to **Lit** from its View Mode dropdown, and confirm it with the 0 cd / 5000 cd
test above before anything else. If that pair stops being pixel-identical, the
lab is live and the ladders in section 5 can run as written. Nothing in the map
needs rebuilding.

### Earlier suspicions, now superseded

Not diagnosed, and listed in order of suspicion so whoever picks this up starts
in the right place rather than from mine:

1. **The viewport may not be applying the PostProcessVolume at all.** An editor
   viewport honours post-process volumes only with its **Game Settings** show
   flag on. With it off the viewport uses its own auto-exposure, which adapts
   to a near-black room and lifts it to mid-grey — which is exactly what the
   control image looks like. If this is the cause then **the exposure lock in
   section 1 was never in force**, and the whole point of copying DockBay's
   values was defeated silently. Check this first.
2. Lumen ambient or surface-cache fallback with no light sources present.
3. A residual emissive or ambient term in the engine materials chosen in
   section 7.

Until the control renders black, **no ladder in section 5 can produce a number
that means anything**, however consistent the numbers look.

## 8. What stopped it — the editor stopped drawing

Not the bridge. The bridge stayed up and answered every non-rendering tool for
the whole session.

**The editor stopped producing frames.** Every capture after the first few
returned the *same* image — byte-identical, md5 `c983014fe6d3` — no matter where
the camera had been moved, and captures that could not return a stale frame hung
until the 300-second tool timeout instead. The editor process reported
`Responding: True` and burned **0.4 seconds of CPU over six seconds**: idle, not
busy, with no `ShaderCompileWorker` running.

Tried, and none of it restored rendering: `Slate.bAllowThrottling` (already 0),
`Slate.ThrottleWhenMouseIsMoving` (already 0), bringing the window to the
foreground via the Slate automation toolset, and the viewport realtime toggle.
The one thing that *did* force a redraw was `Alt+G`, which changes the viewport
type — a structural change, not a camera move.

This needs someone at the machine. A measurement of what a fixture *looks like*
cannot be taken from an editor that is not drawing, and no amount of retrying
changes that — it only produces the same frame with new parameters written
underneath it, which is the most dangerous failure mode available here.

**Resuming:** open `LLK_LightLab` with the editor actually rendering, confirm a
camera move produces a different image before trusting anything, then run the
ladders in section 5 from step 1. Nothing needs rebuilding.

Codex owns `LLK_DockBay` for an art pass (row 41, one owner per `.umap`). It was
opened read-only to take the exposure values in section 1, was never modified,
and was verified clean (`is_dirty` false) at the end of the session.

## 9. Capture method — READ THIS BEFORE MEASURING ANYTHING

**Do not use `CaptureViewport` to measure light.** An earlier version of this
section recommended it and gave three reasons why it was better. That advice
was wrong and is retracted here; it was written before the instrument had been
checked against a known quantity.

`CaptureViewport` renders offscreen and has been caught disagreeing with the
on-screen viewport **twice, in two different ways**:

| what it got wrong | evidence |
| --- | --- |
| **No global illumination** | At emissive 20, all lights 0, it read the near wall at 11.207 before Lumen and 11.207 after — unmoved across the largest renderer change this project has made, while the on-screen viewport moved to 70.3. |
| **Wrong about direct occlusion** | With the shelf lip shading the Bay A card, it read the card at **99.06, contrast 202** — plainly readable — where the on-screen viewport read the same state at **10.33, contrast 14.8**. A factor of ten on the quantity the whole experiment was about. |

It is a dangerous instrument precisely because it is otherwise well-behaved: it
returns a fresh, correctly-exposed, plausible image every time, responds to
every parameter, carries its own camera metadata and is immune to window
geometry. Everything about it invites trust except its numbers.

### So: measure from the on-screen viewport

Use `CaptureEditorImage` and crop. It photographs the editor window, which
brings its own costs, and all of them are manageable if you check:

- **The window must be visible and uncovered.** Anything on top of it, this
  app included, freezes the frame.
- **Verify the crop against the window size every time.** The crop is
  `(3, 126, W-263, H-35)`, which is correct only for the standard layout. The
  window has changed size repeatedly mid-run — 772, 773 and 693 px tall have
  all occurred — and at the wrong height the sampling boxes land on different
  world content and produce confident nonsense. **Print the size with every
  frame and discard rows that do not match.**
- **Hash every frame.** Stale repeats are common and silent. On 2026-11-01 the
  0.75 cd and 2 cd rungs came back byte-identical to each other and matched the
  6 cd frame to 0.01. Only the hash caught it.
- **Verify the camera by tracing, not by asking.** `SetCameraTransform` can
  report success, and `GetCameraTransform` agree, while the rendered view has
  not moved. `ScreenCoordsToWorld` at screen centre must hit the surface you
  think you are pointing at. Re-issue the set and re-trace until it does.
- **Trace every measurement region too**, so you know what each sampling box is
  actually looking at. See `docs/TOOLING-TRAPS.md` 10.

`CaptureViewport` remains useful for things that are not photometry — checking
framing, confirming an actor is where you think, producing an annotated overlay
of actor names. Just never read a brightness off it.

### Record the capture method with every result

Every table in every findings doc should say which instrument produced it, so
that a future instrument failure tells you exactly which numbers to re-take.
Results taken before 2026-11-01 do not carry this, and **the fixture contract in
`docs/FINDINGS-FIXTURE-LIGHT-BAND.md` (`a28fa57`) is the first that should be
re-checked** — its "inspection ceiling below 0.5 cd, orientation floor 6 cd, no
band" is load-bearing for the whole fixture design, and the instrument that
produced it has since been shown wrong about occlusion.

### Add to step zero (section 5a): trace your measurement regions

Before trusting any per-region number, run `ScreenCoordsToWorld` on the centre
of each sampling box and record what it hits and at what distance. A region is
a guess about geometry until something resolves it to a surface.

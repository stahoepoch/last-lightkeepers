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

**And the standing question before any of it: what does this test return when
the instrument is not there at all?** If the answer is anything but an error,
the test is unfinished — every control in this section exists because the
answer was "a plausible number". **No measurement may record an empty or
unchanged result as data; any null becomes an explicit failure before it
reaches a table.** For the minority of failures that are confident rather than
silent, the second question is *what does this read on a quantity whose answer
is **forced**?* — forced, not merely known: a second instrument gives a
disagreement, only an impossibility gives a verdict.

**In practice that means an ordering, and it is one extra capture.** Every
ladder must include at least one pair whose ordering is forced — brighter at
higher intensity, darker when shaded, a quarter at twice the distance — and
**the ordering is verified before any value is recorded**. See
`docs/TOOLING-TRAPS.md`, closing section.

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

### Confirmed 2026-11-02, and the diagnostic is sharper than the warning

The bullet above says the window must be visible and uncovered. That is right,
and here is what it looks like from the inside, because the failure does not
announce itself:

**`CaptureEditorImage` returns a stale frame rather than failing.** With the
editor window occluded, two genuinely different editor states returned a
**byte-identical PNG** - same sha1, same size. Not a similar image: the same
image. A run that does not hash its frames will read that as a stable
measurement.

The three symptoms, in the order they appeared, because the first two are
ambiguous and only the third is a verdict:

| # | Observation | What it could mean |
| --- | --- | --- |
| 1 | 0 cd and 5000 cd frames had **different file hashes but identical sampling boxes** to three decimals | either the light is not reaching the view, or the boxes are on the wrong pixels |
| 2 | The frame showed **Bay A** while `GetCameraTransform` *and* `ScreenCoordsToWorld` both agreed the camera was in Bay C | the rendered viewport is not the viewport the tools reach |
| 3 | After `FocusOnActors` on the Bay C spot, the capture was **byte-identical to the previous one** | **the window is not redrawing at all.** Two different editor states cannot produce one identical image |

Only 3 is conclusive, and it is cheap: issue any command that must change the
view, capture, and compare hashes. If the hash does not move, nothing after it
is a measurement.

**So the precondition is operational, not technical:** the Unreal editor window
must be frontmost and unoccluded for the duration of a ladder. An agent driving
the bridge from another application cannot satisfy that on its own - the person
at the machine has to leave the editor on top. Worth stating plainly in a
project where three parties share one editor.

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

### Define sampling regions in world space, not in pixels

**This is the fix for the capture-geometry error, and it deletes the degree of
freedom rather than detecting it.** Name the region by the thing being measured
— the card face — and project it through the actual camera and the actual
capture dimensions at capture time.

**Fractional insets do not fix this.** Scaling the box with the frame still
samples different world content, because changing the viewport changes which
world the frame contains. Only a world-space region is invariant.

A calibration target is the second-best fix and still worth having, but it must
be a **known step edge at a known world position**, not a flat swatch — so the
measured value maps monotonically to box offset and a miss reads as "the box is
seven pixels low" rather than merely "wrong". A uniform patch certifies the one
thing that never breaks: the geometry error moves a structured region by 1.76x
and a uniform one by 5%.

### Geometry is recoverable retroactively

Every PNG records its own dimensions in its IHDR, so the capture size of any
past reading can be audited without re-shooting it:

    find docs -iname '*.png' | sort | while read -r f; do
      od -An -tu1 -j16 -N8 "$f" | awk -v n="$f" '{printf "%5d x %-5d  %s
",         $1*16777216+$2*65536+$3*256+$4, $5*16777216+$6*65536+$7*256+$8, n}'
    done

Capture size is the editor window minus the crop insets, currently (266, 161):
window 773 gives 612, 772 gives 611, 693 gives 532. **Record the capture size
with every reading**, and when auditing an old one, compare it against the rest
of its own set rather than against an absolute.

### Add to step zero (section 5a): trace your measurement regions

Before trusting any per-region number, run `ScreenCoordsToWorld` on the centre
of each sampling box and record what it hits and at what distance. A region is
a guess about geometry until something resolves it to a surface.

## 10. The exposure lock sweep — specified, not yet run

Designed 2026-11-01 with the peer session. **Not run.** The editor is with
Codex; the handover is the editor restart Codex needs for its GameFeatureData
reopen proof, so one restart serves both.

### Why

Every photometric number in this project has been judged against an exposure
constant nobody chose: the lab's volume pins `AutoExposureMinBrightness` and
`MaxBrightness` to **1.0**, copied field-for-field from DockBay. The tell is
that at a 6 cd handheld lantern the bay **ceiling reads 123.4** — a six-candela
lamp lighting a room, which no plausible eye would see.

**Exposure is a global gain and cannot change a ratio.** The 178x between the
card at 0.6 studs and the room at 8 studs is invariant under any lock point.
What the lock changes is *where that range sits on the tonemapper's curve* —
which end is in the shoulder clipping and which is in the toe crushing. Moving
the lock cannot create a window physics denies; it decides whether the window
the scene already has fits the camera's latitude. 178x is about **7.5 stops**,
and a filmic tonemapper usually carries more than that, so a lock point holding
both ends probably exists. **That is a prediction, not a result.**

### The units trap — read before setting anything

`r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange` is **0** on this
project, so those two fields are **legacy luminance, not EV100**. Dialling in
"EV100 5" by typing 5 is wrong. Convert with `L = 0.125 x 2^EV100`
(equivalently `EV100 = log2(L / 0.125)`):

| EV100 | value to set |
| --- | --- |
| 1 | 0.25 |
| 3 | 1 |
| 5 | 4 |
| 7 | 16 |
| 9 | 64 |

So the current lock of **1.0 is EV100 3**, and the predicted target of EV100 5
is **4.0**. Setting the literal ladder 1, 3, 5, 7, 9 would span only EV100 3 to
6.2 — about three stops where eight were intended — and would probably miss the
answer while looking like a completed sweep.

`AutoExposureBias` is 1 in the volume but its override is **off**, so it does
not apply. If anyone enables it, every reading moves a stop.

### The predicted arithmetic

Illuminance `E = I / d^2`; for a diffuse white card at ~0.8 albedo,
`L = E x 0.8 / pi`.

| lantern at 60 cm | E | card L | card's own EV100 |
| --- | --- | --- | --- |
| 6 cd | 16.7 lux | ~4.2 cd/m2 | ~5.1 |
| 250 cd (authored) | 694 lux | ~177 cd/m2 | ~10.5 |

At the current EV100 3 lock, 6 cd puts the card about two stops over mid-grey —
the near-white paper at 252.9 with glyphs still separated — and 250 cd puts it
about seven and a half over, which is the fully blown 3.4.

### What the baseline correction does to the prior

The lock being EV100 3 rather than 1 halves the cost of moving. The predicted
target of EV100 5 is **two stops up from where the lab sits, not four**, so
reaching it drops the 2 cd ambient by two stops rather than four. **The prior
is therefore that a single lock point clears both ends, not that this is a
knife edge.** The two-region design and the pass criteria do not change —
a favourable prior is a reason to read the far-end frames carefully, not a
reason to skip them. The 250 cd case is untouched at about 7.5 stops over the
lock, which is why it is gone rather than merely bright.

### The method — both ends, every rung

**The failure mode to avoid is the one every lever in this project has hit:**
optimise one end, do not look at the other. Raising the lock darkens
*everything*, and Bay B at 2 cd is already only giving silhouettes. A sweep
that photographs only the card will find EV100 5 beautiful and will have
crushed the orientation read four stops into the toe without noticing.

**Five lock points x two regions, same frame set, ten captures:**

1. **Card**, Bay A, camera (279, 0, 90), deep lip at (323, 0, 110), lantern at
   the held position (285, 20, 110) aimed at (338, 0, 90) at **6 cd**, ambient
   troffer 2 cd.
2. **Orientation**, Bay B, camera (10, 1050, 160), troffer at **2 cd**, lantern
   off.

On-screen path only, with every check in section 9: window uncovered, crop
verified against window size per frame, every frame hashed, camera verified by
tracing to a known surface.

### The forced orderings for this run

**The ladder supplies one for free.** Raising the lock point darkens
everything, so across the five lock points **both** series must decrease
monotonically — the card and the far end independently. That is two forced
sequences spanning all ten captures, verifiable before any value is recorded,
and stronger than a single pair: a confident failure would have to respect
monotonicity across five rungs in two regions.

**It is not sufficient on its own, and the eleventh capture stays.** Monotonic
darkening is *invariant to whether the lip casts a shadow*. An instrument that
has silently lost the occlusion — which is exactly what voided the offscreen
sweep — would still darken smoothly and monotonically across all five lock
points, in both regions, and pass that check cleanly. The exposure ladder tests
that exposure is being applied. It cannot test that the geometry blocking the
light is being rendered, and the whole card result rests on that.

So the eleventh is a **shaded-versus-unshaded pair at one lock point**: capture
the card with the lip, and again with the lip hidden, at the same exposure and
intensity. The shaded reading must be lower. That is the ordering the offscreen
capture violated — 99.06 shaded against 92.29 unshaded — and the only one in
this run that exercises occlusion rather than exposure.

Two axes, two orderings, one extra capture.

### Pass criteria, declared before the run

A lock point passes only if **both** hold:

- **Card:** glyph separation above 100 with **0% of the card at 254**.
- **Orientation:** Bay B's far end still gives a shape read — both blockers and
  their offset legible — not black.

If a lock point clears both, the exposure constant is found, `a28fa57` is
re-taken once against it, and the lantern design fork may dissolve. If none
does, the scene's range genuinely exceeds the camera's latitude and the answer
is geometric or stateful — but established rather than assumed.

**Settle this before re-taking `a28fa57`.** Every photometric number in both
findings docs moves with the lock point, so re-checking the fixture contract
first would mean measuring it twice and holding two contradictory documents in
the repo in between.

## 10b. End every restore by enumerating, not by remembering

**"The lab is at rest" has been asserted several times tonight and was never
once read.** The restore scripts set the lights they had changed, which is a
claim about intent rather than about the map. A second session then reported
`RectLight_1` at 2 cd with the package dirty, and there was no way to tell from
anything written down whether that was true.

It was not — enumeration found all four lights at 0 and the package clean —
but *being right by luck is the failure*, because the claim and the check were
the same act.

**So a restore ends with a read of every light in the map, logged:**

```
find_actors -> for each actor -> get_components
  -> keep components whose name contains "LightComponent"
  -> get_properties(Intensity, bAffectsWorld, bVisible)
```

Verified at rest, 2026-11-01, 51 actors:

| component | intensity | affects world | visible |
| --- | --- | --- | --- |
| `RectLight_0.LightComponent0` | 0 | true | true |
| `RectLight_1.LightComponent0` | 0 | true | true |
| `SpotLight_0.LightComponent0` | 0 | true | true |
| `PointLight_0.LightComponent0` | 0 | true | true |

Enumerating finds lights nobody remembered adding, which tracking-what-you-set
cannot. It also lets the next session verify a handover **without asking the
previous one**, which is the actual point: a handover note that says "at rest"
is worth nothing, and one that lists four components with their intensities is
worth not having to ask.

This is the day's error class one final time — *a claim and its check must not
be the same act* — and the repair is the same shape as all the others: make
the assertion a measurement.

## 11. Making the exposure units self-describing — procedure, not yet approved

A proposal from the peer session, recorded so it can be done correctly if
Chris approves it. **It is his call, and the deciding reason is that it edits
`LLK_DockBay`'s volume, which is Codex's map under the one-owner rule
(row 41).**

### What and why

`r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange` is **0**, the
legacy range, and it is at the *engine default* — nothing in this project sets
it. That makes `AutoExposureMinBrightness` / `MaxBrightness` luminance values
with no label, which has already produced one error in a doc
(`a647391`) and one in a planned sweep (§10). Setting the flag to **1** makes
those fields mean EV100 directly, so every future value is self-describing.
This fixes the class of error rather than labelling it.

### Scope, verified rather than assumed

- **No exposure code in `Source/`** — nothing matching `AutoExposure`,
  `MinBrightness`, `MaxBrightness`, `EV100` or `ExtendDefaultLuminance`.
- **Nothing exposure-related in `Config/`**, which is why the flag sits at its
  engine default.

**Do not find the volumes by listing maps.** `LLK_DockBay` is World Partition,
so its `PostProcessVolume` is not in the `.umap` at all — it is an external
actor. A `Content/*.umap` search converts the lab, misses DockBay and silently
re-exposes it.

Unreal serializes property names into the package, so search for the property
instead and let it find the files wherever they live:

```
grep -rl --binary-files=text "AutoExposureMinBrightness" Content/
```

Which returns exactly two, the second of which no map listing would have found:

```
Content/LLK_LightLab.umap
Content/__ExternalActors__/LLK_DockBay/1/54/RVCGAGWU3BP95V6ADJDK28.uasset
```

Both also contain `bOverride_AutoExposure`, so both are genuine overrides
rather than incidental mentions.

**That search has the same failure shape as the test above, so guard it too.**
Both files are Git LFS objects (`git check-attr filter` reports `lfs`). In a
clone without git-lfs installed they are **131-byte pointer texts**, the grep
matches nothing, and it exits successfully with an empty list. An empty result
means either "no exposure overrides exist" or "the content was never checked
out", and the output cannot tell you which.

**Before trusting a short or empty list, confirm the content is real.** Do not
use a size threshold — the lab's map is 126,432 bytes but DockBay's external
actor is only 10,941, so any "should be six figures" rule rejects a valid file.
Test for the pointer itself, which is exact and size-independent:

```
head -c 40 <file> | grep -q "^version https://git-lfs" && echo "POINTER - not checked out"
```

**Read DockBay's current values; do not assume they match the lab's.** §1 of
this document says the lab's exposure was copied field-for-field from DockBay,
and that copy has already proved imperfect once: the `bMegaLights` override was
on in the lab and off in DockBay, which cost a wrong claim in two commits
before anyone checked. Convert what is actually there.

### The neutrality test, and the control it needs

Done properly the flip is **rendering-neutral**: flip the flag and convert each
volume to the EV100 equivalent of its current luminance in the same commit, and
every committed number stays valid. The obvious test is to capture a frame
before and after and require them to match.

**That test, alone, is worthless — and this document is the reason to know it.**
A rendering-neutral flip predicts two matching frames. So does a dead capture
path, which this project has hit twice: the 0 cd versus 5000 cd pair that came
back pixel-identical because the viewport was unlit
(`docs/TOOLING-TRAPS.md` 8), and the 0.75 and 2 cd rungs that came back
byte-identical to each other because the viewport had stopped redrawing
(trap 7). A match cannot distinguish success from no measurement.

**So gate it the way every ladder here is gated:**

1. **Prove the path is live.** Drive a light to an absurd value, capture,
   confirm the frame moved, put it back.
2. Capture the **before** frame.
3. Flip the flag and convert both volumes.
4. Capture the **after** frame, same pose, same window size, hashed.
5. **Require the match.** Now it is evidence.

### Two commits, not one

If the §10 sweep moves the lock point, **do not bundle that with the flip.**

- **Commit 1 — flip and convert.** Proves neutrality: frames match, with the
  liveness control above.
- **Commit 2 — set the chosen lock point in EV100.** Proves the change: frames
  differ, and by the amount the sweep predicted.

Bundled, the two proofs destroy each other — the frames are supposed to differ,
so neutrality becomes untestable, and a failure in either step looks like a
failure in both. Separated, each carries its own proof, which is the pattern
that caught the config token and the dead viewport.

### And it belongs in config deliberately

The flip is a one-line addition under `[/Script/Engine.RendererSettings]` in
`Config/DefaultEngine.ini`. That file only recently got its renderer baseline
stated on purpose (`aa8b8a5`, `70d81a3`) after years of running on engine
defaults nobody had chosen. Write the intended value in with a comment rather
than letting another renderer setting sit at a default by accident.

## 12. The exposure lock sweep — run 2026-11-01, card half only

**The card half is done and lands on the predicted value. The orientation half
is not established.** Every frame below was accepted by a geometry assertion at
1014x612 and a distinct-crop check; refused frames were retaken, not measured.

### Bay A, the card — lantern 6 cd, ambient 2 cd, deep lip

Declared criterion, before the run: **glyph separation above 100 with 0% of the
card at 254.**

| lock | EV100 | card mean | clip% | glyph range | verdict |
| --- | --- | --- | --- | --- | --- |
| 0.25 | 1 | 253.21 | **83.3** | 35.2 | fail — blown |
| 1.0 | 3 | 246.84 | 0.0 | 104.2 | pass, marginal *(current)* |
| **4.0** | **5** | **221.43** | **0.0** | **175.8** | **pass — best** |
| 16 | 7 | 159.15 | 0.0 | 171.2 | pass |
| 64 | 9 | 70.52 | 0.0 | 90.5 | fail — glyphs below 100 |

**The ladder is monotonic in both card and wall** — card 253.2, 246.8, 221.4,
159.2, 70.5; wall 88.3, 28.6, 5.7, 0.5, 0.01 — so the run's own forced ordering
holds across all five rungs.

**The optimum is lock 4.0, which is EV100 5 — the predicted value.** Glyph
separation 175.8 against 104.2 at the current lock, with nothing clipped. The
prediction was arithmetic from illuminance, made before the run, and it landed.

### Bay B, orientation — an instrument fault, and still not measurable

The orientation ladder was captured at the same five lock points and lock 0.25
and 1.0 measured identically where it mattered. **I argued that the frames were
not duplicates because their cropped pixels hashed differently. That argument
is invalid** and the peer session caught it.

**A hash detects presence, not magnitude.** One flipped bit of temporal jitter
makes two identical renders hash differently, so non-identity is no more
evidence of a changed render than identity was evidence of a live one. It is
the byte-identical error run backwards.

The test that *is* available costs one subtraction — mean absolute difference,
against a measured noise floor:

| pair | MAD | % pixels differing by >1 |
| --- | --- | --- |
| noise floor: same config, 80 min apart | 0.317 | 4.3 |
| Bay A 0.25 -> 1.0, which responded 3.1x | 52.274 | 99.6 |
| **Bay B 0.25 -> 1.0, the suspect** | **0.131** | **0.5** |
| Bay B 1.0 -> 4.0, which responded | 10.877 | 63.4 |

**The suspect pair differs by less than the noise floor.** It is one render
plus jitter: the lock change never reached the second capture. No asymmetry
between the bays, no renderer mystery — a stale frame, which is what the
impossibility was pointing at all along. *A global gain cannot move one bay and
not the other* was the diagnosis, and I filed it as the mystery.

**The assertion now uses magnitude instead of identity**: it records the crop
mean for every frame and refuses any frame whose MAD from the previous accepted
frame sits at or below 0.35. On the retake it immediately caught a **second**
stale rung — lock 16 at MAD 0.129 — that the hash check had passed.

**Bay B is still not measurable.** The retaken lock 1.0 rung violates the
forced ordering outright: scene mean 13.48 at lock 1.0 against 11.44 at lock
0.25, brighter at a higher lock, which cannot happen under a global gain. Two
stale rungs and an ordering violation in one ladder, so **the orientation half
is not reported and the sweep is not concluded.**

**What this means for the decision.** Lock 4.0 is a strong candidate on the
inspection side and it is where the theory said it would be. It is not adoptable
until the orientation side is measured, because raising the lock darkens the
run and that is the half this project has failed to protect four times.

### Two failures were being carried as one, and they have different cures

The peer session separated them and the distinction decides what to build:

| failure | what it is | the cure |
| --- | --- | --- |
| **misalignment** | fixed insets against a window that moved, so the box lands on different world content | **world-space ROI** |
| **staleness** | the capture returns a render from before the change landed | **a liveness assertion** |

**A world-space ROI does nothing for staleness.** It makes the box land on the
right world content; it has no opinion about *when* that content was rendered.
A perfectly aligned box measures a stale frame perfectly. And misalignment does
not block the orientation half anyway — it is a *cross-run* problem, which is
why every load-bearing ladder in the geometry audit came out internally
consistent. The ROI buys comparability across runs and the project should still
get it; it would not have bought a Bay B ladder.

**Every Bay B failure was staleness**: MAD below the noise floor, a second
stale rung at 0.129, and an ordering violation consistent with a frame from an
earlier state.

### Why the MAD detector is not enough, by its own logic

MAD infers staleness from pixel similarity, so **its reliability scales with
the size of the real change** — the sensitivity problem again. It reads 52.274
in Bay A where the signal is large and is asked to adjudicate 0.131 in Bay B
where the signal is small. Worse, in a nearly-black far end two genuinely
different exposures can legitimately differ by less than the floor, so a
*correct* frame gets refused as stale **exactly where the orientation half has
to measure**. The detector is most trustworthy where it is least needed. And it
leaks the other way too: a frame passed the 0.35 threshold and still produced
an impossible ordering.

**So a statistical detector may raise the question and must never answer it.**

### Difference against a fixed baseline, not the previous frame

**This is the whole of the detector's sensitivity problem, and it costs nothing
to fix.** Adjacent differencing throws away accumulation. Compare each frame to
the ladder's *first* rung instead: the signal accumulates and climbs out of the
noise floor, while a stale rung still shows plainly as a **flat spot** - it
sits at the same distance from the reference as the frame before it.

Demonstrated on committed data. The offscreen core ladder, adjacent versus
cumulative:

| rung | adjacent MAD | cumulative from v_e000 |
| --- | --- | --- |
| v_e0125 | 0.7037 | 0.7037 |
| v_e025 | 0.4241 | 1.1260 |
| v_e05 | 0.4657 | 1.5897 |
| v_e1 | 0.3623 | 1.9469 |
| v_e2 | **0.2582 - refused** | 2.2017 |
| v_e5 | **0.3313 - refused** | 2.5307 |
| v_e20 | 1.2459 | 3.7741 |

**Cumulative is strictly increasing, so every rung moved.** The two adjacent
refusals are false rejections, and they land in the offscreen ladder precisely
because its per-step signal is tiny - it has no GI, so raising the emissive
changes the glowing object and almost nothing else. The same emissive step
reads 3.95 MAD offscreen and **27.61 in the viewport**.

A full pairwise audit of the archived viewport ladders found **no pair below
the floor** in TA, TB, FA, FB, DB, NB, LNT, the N_e core ladder, trofA, trofB,
spotC or any of the four lantern poses. Staleness is not a standing condition
of the viewport path; it was specific to the Bay B run.

So **the offscreen path is now suspect on two independent grounds** - no GI,
and dimension drift (1671x1015 against the ladder's uniform 1669x1012) with no
viewport to blame. "Offscreen is the trustworthy instrument" is retired.

### Retracted: the frame-counter check tests the wrong proposition

An earlier version of this section proposed comparing the engine frame index at
the state change against the index at the capture. **That check would come back
clean and mean nothing**, and my own measurement refutes it: dispatches read
frames 10, 409, 461, so hundreds of engine frames elapse between MCP calls.
Frames were never scarce.

**Engine tick and viewport redraw are different clocks.** The log counter
advances with the engine loop; a viewport redraws when something invalidates
it. The counter can advance by fifty while the rendered content sits still and
the capture reads the last render. So the check tests whether *time passed*,
not whether the capture *incorporated the change* - and a clean result would
have been wired to the false conclusion that the staleness comes from
somewhere a settle cannot reach.

**The settle, as first written, is wrong for the same reason.** Waiting k
frames is useless against a viewport that is not redrawing; fifty already went
by. What is needed is k *forced redraws*, which is a different operation.

**The realtime flag was checked and does not explain it.** In
`Saved/Config/WindowsEditor/EditorPerProjectUserSettings.ini` the perspective
viewport carries `bIsRealtime=True`; only the three orthographic panes are
False. The obvious cause is ruled out, leaving the one observed directly during
the resize work: **the window must be visible.** The editor ran at roughly
0.2 fps with another tab in front and about 100 fps with the level viewport
foreground, and `CaptureEditorImage` photographs the window.

### The frame counter works as a *rate*, and it confirms the mechanism

A frame-index **gap** tests whether time passed and always comes back clean. A
frame **rate** tests the editor's throttle directly, and the difference it has
to resolve is five hundredfold. The log carries both the counter and
timestamps, so **frames per second around any past capture is recoverable** —
the whole Bay B run was audited from the log alone, with no new captures.

| rung | local | fps | outcome |
| --- | --- | --- | --- |
| EXP_A_025 | 16:20:38 | **118.3** | good, 99.6% of pixels moved |
| EXP_A_100 | 16:20:56 | 26.1 | good |
| EXP_A_400 | 16:31:48 | 2.9 | good despite low fps |
| EXP_A_1600 | 16:32:05 | 58.3 | good |
| EXP_A_6400 | 16:32:21 | 40.3 | good |
| EXP_B_025 | 16:33:17 | 68.1 | good |
| **EXP_B_100** | 16:33:35 | **3.0** | **stale** — MAD 0.131 |
| EXP_B_400 | 16:39:07 | 1.7 | accepted |
| **EXP_B_1600** | 16:39:26 | **3.0** | **stale** — MAD 0.129 |
| EXP_B_100b | 16:50:40 | 3.0 | retake, violated forced ordering |

**Corrected: measure the rate across the change-to-capture interval itself,
not across a time window.** The windowed version above put EXP_A_400 at 2.9 fps
and good, which looked like a counterexample and forced a weaker conclusion.
It was an artefact: that row follows a 300-second capture hang, so a window
around it averages across the hang. Measured across the interval that actually
matters — the dispatch of the state change to the dispatch of the capture —
the separation is total:

| rung | frames | dt (s) | fps | outcome |
| --- | --- | --- | --- | --- |
| EXP_A_025 | 546 | 4.6 | **118.7** | good |
| EXP_A_100 | 520 | 4.4 | **118.2** | good |
| EXP_A_1600 | 481 | 4.0 | **120.2** | good |
| EXP_A_6400 | 478 | 4.0 | **119.5** | good |
| EXP_B_025 | 514 | 4.3 | **119.5** | good |
| EXP_B_100 | 18 | 6.0 | **3.0** | stale |
| EXP_B_1600 | 19 | 6.3 | **3.0** | stale |
| EXP_B_100b | 20 | 6.7 | **3.0** | ordering violation |
| EXP_A_400 | 14 | 634.6 | — | post-hang, unmeasurable |
| EXP_B_400 | 513 | 316.0 | — | post-hang, unmeasurable |

**Good rows: 118, 119, 120, 120, 120. Bad rows: 3.0, 3.0, 3.0.** Forty-fold
apart with nothing in between and no counterexample in this data.

**But rate is still a screen, not a verdict — restored on mechanism.** An
earlier revision dropped that caveat because its stated counterexample
(EXP_A_400 at 2.9 fps and good) turned out to be an unmeasurable row. Rejecting
bad evidence for a claim is not the same as refuting the claim, and the real
justification was never that row:

**Rate measures the throttle. Staleness is what the throttle causes.** On this
data the two are perfectly correlated because the coupling runs through
*focus* — but the capture depends on *visibility*, and those diverge. A window
covering the viewport while the editor keeps focus, a modal over the viewport,
the editor on a second display that has gone to sleep: each gives a
non-redrawing viewport **at 120 fps**, and rate waves it through. The clean
separation describes today's failure mode, not every failure mode.

### Closed: no settle of any kind fixes this

The stale rungs show 18 and 19 frames elapsed at 3.0 fps — about **six seconds
of wall time** between the state change and the capture. They waited six
seconds and were stale regardless. **So waiting does not fix it, in frames or
in milliseconds**, and every variant of "settle before capturing" is dead:
waiting k frames, waiting k seconds, forcing k redraws through a throttled
loop. Recorded as closed so nobody reaches for it again. The only thing that
moves the needle is the window being in front.

### To make rate a verdict, induce the condition instead of auditing it

The separation is currently a *fit*: two clusters with a threshold interpolated
across an empty gap. The honest upgrade is to create the condition with known
ground truth on both sides:

1. **Editor backgrounded** — run a three-rung ladder.
2. **Editor foregrounded** — run the same three rungs.
3. **Editor focused, viewport covered** — the case that decides whether rate
   *generalises* or merely *describes today*, because it should produce a
   non-redrawing viewport at full frame rate.

Staleness then exists by construction, the threshold is measured rather than
guessed, and case 3 tests the mechanism rather than the correlation. This is
the same inversion standard the project already applies to a config change, and
it is minutes of work — but it needs someone at the machine to move the window,
so it is not a thing this session can run alone.

**A capture whose interval spans a hang cannot be judged this way**, because
the interval includes the stall. Those rows need the other checks, and
`EXP_B_400` — accepted by no detector firing rather than by passing one — is
the weakest row in the ladder for exactly that reason.

### The gap, as a boolean, does not work

A tempting repair is to ask only whether the counter *advanced* between the
change and the capture: if a frame was drawn after the change, the capture
cannot show the old state. **Audited, and it fails.** The two known-stale rungs
show 18 and 19 frames elapsed. Frames advanced and the capture was still stale.

The reason is the one that killed the first version: **the log counter is the
engine loop, not the viewport.** Eighteen engine ticks can pass with zero
viewport redraws, so advancement proves nothing. Only the *magnitude*
discriminates, and magnitude is the rate.

So the operand lesson lands a third time: the same counter is useless as a
gap, useless as a boolean, and decisive as a rate over the right interval.

**Record the fps with every capture.** It is free, it is recoverable
retrospectively, and it certifies the operating condition rather than the
pixels.

### The cause is a checkbox

0.2 fps backgrounded against 100 fps foreground is the signature of the
editor's background CPU throttle — **Editor Preferences, Performance, "Use Less
CPU when in Background"**, `bThrottleCPUWhenNotForeground` under
`[/Script/UnrealEd.EditorPerformanceSettings]`. It is **written nowhere** in
this project's `Config/` or `Saved/Config/`, so it sits at the engine default,
which is on. **It has therefore been active for every capture this project has
ever taken.**

Turning it off removes the dominant term. It does not make the instrument
sound: `CaptureEditorImage` photographs the window, so occlusion, a tooltip, a
lock screen and whatever the compositor does to a minimised window all remain.
**The deeper defect is that the measurement depends on what happened to be in
front of the screen, which is a human variable that appears in no log and no
commit.**

### Which instrument deserves the engineering

Corrected from an earlier claim here that the offscreen path was suspect on two
grounds. **Only one survives.** Its "smallest signal in the corpus" was a
property of the no-GI scene in that run, not of the instrument — an emissive
change moves little when nothing bounces. The dimension drift is real, and it
is one bug.

That asymmetry decides where to invest. **The offscreen path's defect is
fixable engineering; photographing a window has a defect no engineering
removes.** If an instrument is going to be built, build the offscreen one.

### A pattern worth more than either fix

Both broken detectors were repaired the same way, and neither needed a new
mechanism:

| detector | wrong operand | right operand |
| --- | --- | --- |
| MAD | difference from the **predecessor** | difference from a **fixed baseline** |
| frame counter | the **gap** between indices | the **rate** |

**Check for the right operand before reaching for a build.** It is a much
cheaper class of repair than the ones this project keeps reaching for, and
twice today it was the whole fix.

### Superseded: the frame counter as a liveness assertion

Unreal stamps every log line with its frame number (modulo 1000, as
`[... ][ 461]`), and `LogsToolset.GetLogEntries` can read it. Each MCP tool
dispatch is logged, so **the frame index at the state change and the frame
index at the capture are both recoverable**:

    GetLogEntries(pattern="Dispatching toolset tool", maxEntries=12)
    -> "frame    409  execute_tool_script"
       "frame    461  execute_tool_script"

**Require the capture's frame index to differ from the preceding state
change's.** Equal means no frame was drawn in between, so the capture is a
render of the old state. This is causal rather than statistical: it does not
care whether the picture changed by 52 or by 0.1, so it works identically in a
blown-out bay and a black one.

Fallback if a capture path cannot reach the counter: **tag the frame** — set
some small visible thing to a value derived from the capture index and read it
back out of the crop. Same property, travels through any path.

### The cheap mitigation to try first

Bay B is the bay the camera *moves to*; Bay A is where it already was. A larger
view change means more to re-render and re-converge, so staleness should
correlate with how far the camera just moved — which predicts that Bay B goes
stale and Bay A does not, and that is exactly what was observed. If that holds,
the fix is a **fixed settle after any camera move**: force k redraws, discard
them, then capture. Minutes of work, and with the liveness assertion it is a
complete answer to this failure without a build.

### A bug this run found in its own instrument

The geometry assertion initially hashed the **full capture** rather than the
cropped region it measures. Editor chrome differs between frames while the
viewport is stale, so a stale viewport passed the duplicate check. Fixed to
hash the measured crop.

That is the rule from `docs/TOOLING-TRAPS.md` — *a check must run where the
measurement is sensitive, not where it is convenient* — failing against the
tool written to enforce it, one commit after it was written down. The full-byte
hash was the convenient thing to hash.

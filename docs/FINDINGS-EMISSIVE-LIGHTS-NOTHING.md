# The lantern core's emissive lights nothing, because this project has no Lumen

Measured 2026-11-01 by Claude in `LLK_LightLab`, against the question left open
by `21405ca`: *"I have not yet called its Lumen bounce behavior proven."*

**It cannot be proven, because there is no Lumen.** This project runs with
`r.DynamicGlobalIlluminationMethod = 0` (None). An emissive material therefore
contributes **no light to the scene at all** — only to its own pixels, plus
post-process bloom around them.

`CoreEmissive` is a visual-only parameter. That is exactly what `21405ca` called
it, so the commit message is accurate. What is not yet true is the implied
next step: there is no bounce to tune.

---

## The three readings

| | Result |
| --- | --- |
| **GI method** | `r.DynamicGlobalIlluminationMethod = 0` — **None**. Also `r.ReflectionMethod = 2` (SSR, not Lumen), `r.GenerateMeshDistanceFields = 0`, `r.Shadow.Virtual.Enable = 0`, `r.MegaLights.EnableForProject = 0`. |
| **Light the core puts on surfaces** | **Zero, at every value tested from 0.125 to 20.** A wall 2.6 m away reads 0.000/255 even at `CoreEmissive 20`, forty times the authored value. |
| **Where the core stops reading amber** | Between **1.0 and 2.0**. At the committed **0.5** it is amber; at 2.0 it is cream; by 5.0 it is white. |

Nothing in `Config/` sets a GI method — there is no `[/Script/Engine.RendererSettings]`
block mentioning Lumen, ray tracing, distance fields or reflections anywhere in
the project, and nothing in `Saved/Config` either. So this is not a LightLab
quirk and not a stale editor session: it is the project's effective state, and a
fresh editor on another machine would do the same.

## The measurements

All nine frames: one sealed box, every placed light at **0 cd**, so the emissive
core is the only possible light source. Identical camera pose on every frame
(250, 100, 180), pitch −5.4°, yaw 159.4°, FOV 90 — **read back from each
capture's own metadata**, not assumed.

Luminance 0–255. `wall86` and `wall261` are two patches of **the same wall**
(Y = 200), 86 cm and 261 cm from the core.

| CoreEmissive | core | core saturation | wall @86cm | wall @261cm | ceiling | floor |
| --- | --- | --- | --- | --- | --- | --- |
| 0 (control) | 0.00 | — | 0.000 | 0.000 | 0.000 | 0.000 |
| 0.125 | 77.2 | 0.949 | 0.000 | 0.000 | 0.000 | 0.000 |
| 0.25 | 123.3 | 0.888 | 0.000 | 0.000 | 0.000 | 0.000 |
| **0.5 (committed)** | **172.6** | **0.810** | **0.000** | **0.000** | **0.000** | **0.000** |
| 1.0 | 209.2 | 0.702 | 0.048 | 0.000 | 0.000 | 0.000 |
| 2.0 | 231.8 | 0.483 | 0.226 | 0.000 | 0.000 | 0.000 |
| 5.0 | 247.3 | 0.224 | 1.478 | 0.000 | 0.039 | 0.036 |
| 20 | 254.0 | 0.054 | 11.278 | 0.000 | 0.993 | 0.981 |
| 500 | 255.0 | 0.000 | 153.247 | 2.233 | 59.789 | 59.540 |

**At the committed value, every surface in the box reads exactly zero.**

## Why the glow at high values is bloom, not bounce

At `CoreEmissive 500` the frame looks, at a glance, like a warm lantern filling a
room. It is not. Three things rule bounce out:

- **It ignores geometry.** `wall86` reads 153 and `wall261` reads 2.2 — a 70×
  difference across one flat wall. Real illumination from a source 86 cm and
  261 cm away differs by about 9×, not 70×. What the brightness actually tracks
  is **distance from the sphere on screen**, which is the signature of a
  screen-space post effect.
- **There are no edges.** The 500 frame is a smooth radial gradient with no wall
  corner, no floor line, no ceiling join and no shadow cast by the sphere. Lit
  surfaces would show all of those.
- **The far wall is nearer than it sounds.** `wall261` is 261 cm from the core
  but only **116 cm from the camera** — a large, close, directly-facing surface.
  It reads 0.000 at every rung up to 20.

`docs/lightlab/core/v_e500_annotated.png` makes it visible: the overlay labels
`BAYA_WYP`, `BAYA_WXN`, `SPINE_WXP` and `SPINE_CEIL` — real wall and ceiling
actors, in frame, black, beside a blazing core.

## The controls, both ends

- **Zero reads zero.** All lights 0, `CoreEmissive` 0 → the frame is black.
- **The maximum proves the instrument is wired.** `CoreEmissive` 500 → the core
  blazes and bloom floods the frame. The render path is live and emissive does
  reach the screen, so a flat result at lower values is a real measurement, not
  a dead pipe.
- **The frame is not empty.** `GetVisibleActors` returns **16 actors** in the
  frustum, ten of them static meshes. Screen-to-world traces from the core
  position, both wall patches, the ceiling box and the floor box all **hit solid
  geometry** at 116–316 cm. A black frame here means unlit, not absent.

That last control is the one that mattered. A sealed box photographed in the
dark looks identical to a camera pointed at nothing.

## What this means for the lantern

- **`CoreEmissive 0.5` is a good choice and the work in `21405ca` stands.** The
  core reads amber and contained; see `docs/lightlab/core/SHEET_core_ladder.png`.
  The measured ceiling is **about 1.0** — past that the glass desaturates toward
  white and stops reading as a warm chamber.
- **Hooded / Working / Open can move the visible glass as intended.** That part
  of the plan needs no GI; it is the core's own pixels.
- **But the visible glass will never spill onto the hand, the floor or the
  cage.** Everything the lantern actually *lights* comes from
  `ULLKLanternLightComponent`, the spot. The emissive and the light are two
  independent channels, and nothing in the material can couple them.
- So the shutter should drive **both**: `CoreEmissive` for the glass and the
  spot's cone/intensity for the beam, kept in step by whatever owns the
  aperture. They will not track each other for free.

## Decided: Lumen is now the renderer baseline

**Chris took this decision on 2026-11-01**, as one deliberate renderer change
rather than a lantern tweak, on the reasoning that the store's visual language —
neon as landmarks, a lantern that answers the dark, glass and reflections —
depends on indirect light, so without GI the "living mall" cannot exist
physically.

`Config/DefaultEngine.ini` now carries a `[/Script/Engine.RendererSettings]`
block: Lumen GI, Lumen reflections, mesh distance fields on. **Software tracing
only** — hardware ray tracing and Virtual Shadow Maps are deliberately left off
until their cost is profiled against dense retail geometry.

The earlier bar on editing that file (dirty from another writer) had lapsed: it
was clean in the working tree and last touched by a landed commit.

**The same change also pins the project to SM6.** The editor had been running at
**Shader Model 5** — `rhifeaturelevel="SM5"`, and `Skipped NVAPI RT queries
since we the feature level is below SM6` in the startup log — because the
engine's base default is `PCD3D_SM5` and this project's hand-written config
never overrode it. That is the *same* gap that left GI unset: a minimal
`DefaultEngine.ini` that never received the renderer and platform block a
UE5 template project gets. Without SM6 the Lumen settings would have been
enabled in config and inert in the editor, which looks identical to the no-GI
behaviour they replace.

**Verify before believing it.** After the restart, the check is the positive
control already in this document: `CoreEmissive 500` with every light at 0.
Under working Lumen the walls, ceiling and floor must light up. If that frame
still comes back black with a bloom halo, Lumen is not running and nothing
below has changed.

**Every number in this document, and in
`docs/FINDINGS-FIXTURE-LIGHT-BAND.md`, was measured before that change and
describes the no-GI renderer.** They are kept as the record of what was true,
and all three sheets — fixture, lantern, emissive — need re-running under Lumen.
Until that happens, treat the tables here as history rather than as the current
behaviour of the game.

Worth saying plainly: **the whole fixture-light programme so far was measured
without GI.** `docs/FINDINGS-FIXTURE-LIGHT-BAND.md` is unaffected in its
conclusion — it measured direct light from placed fixtures, which is exactly
what the game currently renders — but its remedy of *"luminous sources that are
visible without illuminating"* turns out to be less of a design choice than it
read at the time. **Under this renderer, every self-luminous object is already
visible without illuminating.** Neon signage and exit signs will read as bright
landmarks and will not light the aisle, for free and with no tuning.

If Lumen is later switched on, that changes and all three documents need
re-measuring.

## Measured under Lumen, 2026-11-01

Lumen went live on 2026-11-01 (`aa8b8a5`, `70d81a3`). The editor now reports
`rhifeaturelevel="SM6"`, `r.DynamicGlobalIlluminationMethod=1`,
`r.ReflectionMethod=1`, `r.DistanceFields=1`, ray tracing off by project
setting, and Lumen's own shaders (`FLumenCardPS`, `FVisualizeLumenSceneCS`)
compiled at startup.

**Emissive now lights the room.** With all four placed lights at 0 cd and
`CoreEmissive` at 500, the frame
(`docs/lightlab/core/lumen_e500_first_look.png`) shows a *room*: the wall
corner reads as a crisp vertical edge, the ceiling and floor planes are lit
and recede correctly, falloff is shaped by the geometry, and there is genuine
occlusion below the lamp housing. The pre-Lumen frame at the same settings had
none of that — only a smooth radial bloom gradient with no edge anywhere.

**This is one qualitative frame, not a ladder.** It is recorded because it
answers the yes/no question. It does not replace the tables above, and no
number in this document has been re-measured yet. Specifically not established:
the shape of the response across `CoreEmissive` values, whether 0.5 still reads
amber once the room around it is lit, or any fixture figure.

### The ladder, re-run

Editor foreground with Realtime on, which is what the two capture paths needed.
Nine frames, every light at 0 cd, identical camera pose, **nine distinct md5s**
so no frame is a stale repeat. `wall86` and `wall261` are two patches of the
same Y=200 wall, 86 cm and 261 cm from the core. Luminance 0-255.

| CoreEmissive | core | core saturation | wall @86cm | wall @261cm | ceiling | floor |
| --- | --- | --- | --- | --- | --- | --- |
| 0 (control) | 0.0 | — | 0.000 | 0.000 | 0.000 | 0.036 |
| 0.125 | 58.0 | 0.949 | 0.044 | 0.000 | 0.000 | 0.034 |
| 0.25 | 93.1 | 0.888 | 0.207 | 0.000 | 0.000 | 0.037 |
| **0.5 (committed)** | **131.2** | **0.811** | **0.908** | **0.000** | **0.000** | **0.034** |
| 1 | 161.2 | 0.703 | 2.790 | 0.000 | 0.053 | 0.093 |
| 2 | 182.3 | 0.485 | 7.563 | 0.001 | 0.301 | 0.342 |
| 5 | 203.0 | 0.226 | 21.474 | 0.220 | 1.836 | 2.002 |
| 20 | 229.0 | 0.055 | 70.327 | 3.140 | 13.330 | 14.176 |
| 500 | 251.2 | 0.021 | 130.976 | 6.079 | 28.724 | 37.432 |

**Emissive now lights surfaces.** Pre-Lumen every one of those surface columns
read exactly 0.000 up to CoreEmissive 20. They now rise smoothly, and roughly
in proportion to the parameter: doubling the value roughly doubles the wall.
That is real indirect light, and `docs/lightlab/core/SHEET_lumen_core_ladder.png`
shows the room appearing out of the dark as the value climbs.

### But at the authored value it still lights nothing

At the committed **0.5** the nearest wall reads **0.9 out of 255**. The far wall,
the ceiling and the floor read **zero**. In the contact sheet that frame is an
amber orb floating in black — indistinguishable, to the eye, from the no-GI
result it replaced.

The room only appears from about **2**, and reads as genuinely lit at **5 to 20**.

### A second band that is not there

The two requirements pull opposite ways, and the numbers say they do not meet:

| | |
| --- | --- |
| Glass still reads **amber** | CoreEmissive **≤ 1** (saturation ≥ 0.70) |
| Core contributes **visible** light | CoreEmissive **≥ 5** (wall ≥ 21; 2 is faint at 7.6) |

So there is no value where the chamber looks like warm amber glass *and* throws
useful light — the same shape as the fixture contract in
`docs/FINDINGS-FIXTURE-LIGHT-BAND.md`, arrived at independently.

**The way out is colour, not intensity.** The desaturation is the tonemapper
clipping to white, and the scalar is the only parameter the material exposes.
A deeper, more saturated emissive colour would hold its hue to a higher
intensity, which would move the amber limit up toward the useful-light floor.
That is a change to `M_LLK_LanternCore`, not to the instance, and it is Chris's
call — this lab only measured the scalar it was given.

Until then the honest reading is the one from before, for a new reason: **the
glass and the beam are separate channels.** At 0.5 the core is pure look, and
`ULLKLanternLightComponent` still does all the lighting.

### How it was captured, and what fought back



Two instruments, both currently unusable for a Lumen ladder:

- **`CaptureViewport` (offscreen) does not appear to resolve Lumen.** At
  identical settings it returned a black room with a bloom halo while the
  on-screen viewport showed a lit room. Offscreen scene captures have no
  temporal history, and Lumen is temporal. Treat offscreen captures as
  unusable for GI until proven otherwise. It also still hangs to the 300 s
  timeout on lit scenes.
- **`CaptureEditorImage` returns a stale frame while the editor window is
  occluded.** Changing `CoreEmissive` from 500 to 0 produced a **byte-identical
  viewport crop**, md5 and all, while the engine ticked on at about 3 fps. The
  desktop grab returns the window's last composited content, so with the editor
  behind another window the capture is of whatever was last drawn on screen.

**What unblocks it:** the Unreal Editor window foreground and visible, with
Realtime enabled in the level viewport (Ctrl+R). Then `CaptureEditorImage` is
fresh and the ladder can be run with the usual md5-differs check per frame.

### Two things found while trying

- **MegaLights was on in the lab and could not work.** The lab's post-process
  volume had `bOverride_bMegaLights` set — one of only three overrides on it.
  MegaLights needs hardware ray tracing, which this baseline deliberately
  leaves off, so the viewport showed a standing warning: *"MegaLights is
  enabled, but has no ray tracing data and won't operate correctly."* It was
  inert at SM5 and merely broken at SM6. **Cleared in the lab on 2026-11-01**
  by rewriting the whole 463-field `Settings` struct, which preserves the two
  exposure overrides; a partial write would have replaced the struct and taken
  the exposure lock with it.

  **Correction: `LLK_DockBay` was never affected.** This document previously
  claimed DockBay carried the same active setting, reasoning that the lab's
  volume had been copied from it. Chris checked the map directly:
  DockBay's volume has `bOverride_bMegaLights = false`. The `bMegaLights = true`
  value is stored but **dormant**, because the override flag that would activate
  it is off. The lab's flag was switched on somewhere in the copy; DockBay's
  never was. Nothing in DockBay needs changing, and clearing a dormant value
  there would mean the same 463-field rewrite for no benefit, against the
  exposure lock. **Inferring a second map's state from a copy is not checking
  it** — the claim should have carried a read of DockBay's own flag or not been
  made.
- **The lab's Bay A fixture is still in neon configuration** — `LightColor`
  (0.749, 0.349, 1.0), `SourceWidth` 120, `SourceHeight` 8 — left from Ladder 2
  last session. Any re-run of the fixture ladder must set troffer vs neon
  explicitly rather than trusting the saved state. A violet-lit room was
  briefly mistaken here for a broken material.

## What this does not establish

- **No value above 500 or below 0.125 was tested**, and the sweep is of the
  scalar only — the parent material's emissive colour was not varied.
- **Bloom intensity was not separated from the exposure setup.** The halo's
  size at a given value depends on the post-process volume copied from DockBay;
  a different exposure or bloom setting changes how large the glow reads,
  though not whether surfaces are lit.
- **This says nothing about packaged builds.** Every reading is from the editor
  viewport. A cooked build reads its own config, and the same `Config/` is what
  it would read, but that was not verified here.
- **The core is a 36 cm sphere stand-in** (`/Engine/BasicShapes/Sphere` at scale
  0.36), not the authored lantern glass.

## Reproducing

`docs/LIGHTLAB-PROTOCOL.md` §1 (exposure) and §5a (step zero) still apply. The
capture method changed — see §9 — and the traps met on the way are
`docs/TOOLING-TRAPS.md` 9, 10 and 11. The lab is `Content/LLK_LightLab.umap`,
saved with all four lights at 0 cd and `CoreEmissive` restored to the committed
0.5.

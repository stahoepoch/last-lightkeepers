# The fixture light contract has no band

> **Status: ANSWERED, and the answer is that no band exists.** Rewritten
> 2026-11-02 after reading the contract it measures against. The question "what
> is the band" is settled by the evidence below: **there is no candela value
> that satisfies both halves**, three lighting levers were tried and all three
> failed, and the one lever that works is geometry rather than intensity. So
> there is nothing numeric to put in the contract, and
> `docs/FIXTURE-LIGHT-CONTRACT.md` is **LOCKED by Chris** in any case. It is
> **his decision, and he made it: the contract is amended and re-locked,
> 2026-11-02.** The mechanism is the section "How the rule is met - geometry,
> not intensity"; his one-line statement of it is *"Fixtures orient; geometry
> keeps inspection detail dark. Navigation signage may emit; game-relevant
> information still requires the lantern."* So the question this document asks
> is closed, and `HANDOFFS.md` C9 with it. `NOTES-FOR-CHRIS.md` item 5 has the
> spec. `HANDOFFS.md` M2 is
> rewritten accordingly - it previously asked for "a numeric band in the
> contract", which is the wrong proof of closure for a document that proves no
> such band is available.
>
> **Two numbers below are stale, both about the lantern.** They do not affect
> any fixture conclusion.
>
> - **The authored lantern is 25 cd, not 250.** `a060e78` changed
>   `data/lantern.json` on 2026-11-01, adopting this lab's own recommendation.
>   Every "250 - the value `data/lantern.json` currently authors" below should
>   be read as a past value. The 250 cd *measurements* stand; only the word
>   "authored" has moved.
> - **Ladder 3 and the held-lantern sweep used a 72 deg outer cone; the authored
>   lantern is 110 deg** (`coneAngleDegrees`). The same candela spread over a
>   wider cone is not the same light, so "25 cd reads as a carried lantern" was
>   established on a narrower fixture than the one shipping. Flagged rather than
>   corrected, because re-running it needs the editor and the bridge.

Measured 2026-10-31 by Claude in `LLK_LightLab`, against
`docs/FIXTURE-LIGHT-CONTRACT.md`'s rule that ambient fixtures *"may help
players navigate, but must never look bright enough to read a relic under."*

**There is no candela value that satisfies both halves.** The orientation floor
is at least twelve times the inspection ceiling. Uniform fixtures cannot meet
the contract, and the answer is directional ones.

> **Re-measured under Lumen on 2026-11-01.** The original tables below were
> taken with `r.DynamicGlobalIlluminationMethod = 0`, direct light only. The
> re-run is the section immediately after this note. **The conclusion holds —
> there is still no band — but the gap narrowed roughly threefold**, and the
> orientation floor moved more than the inspection ceiling did.

---

## The re-run under Lumen

Software Lumen, SM6, MegaLights override cleared from the lab's post-process
volume first so nothing was measured through a lighting path that cannot
operate. Every fixture state set **explicitly per rung** — source size, colour,
attenuation and intensity written together — rather than inherited, after the
leftover violet neon from Ladder 2 was briefly mistaken for a broken material.
Captured from the on-screen viewport, because the offscreen path has no global
illumination in it (`docs/TOOLING-TRAPS.md` 12). Every frame hash-checked
distinct; camera verified by tracing the centre ray to a known surface.

### Ladder 1 — troffer, Bay A, camera 56 cm off the card face

| candela | card mean | text mean | text contrast | wall | scene | card readable? |
| --- | --- | --- | --- | --- | --- | --- |
| 0 (control) | 0.67 | 0.48 | 1.1 | 0.00 | 0.26 | **no** |
| 0.5 | 30.79 | 26.23 | 36.1 | 0.26 | 6.70 | yes, dim |
| 2 | 92.29 | 79.17 | 102.7 | 4.92 | 24.95 | yes |
| 6 | 168.45 | 145.28 | 172.7 | 23.84 | 59.12 | yes |
| 15 | 211.78 | 185.46 | 191.5 | 56.80 | 99.29 | yes |
| 40 | 235.94 | 213.24 | 170.8 | 116.85 | 155.92 | yes |
| 100 | 248.02 | 233.51 | 116.7 | 182.66 | 205.20 | yes |

**The inspection ceiling has not moved: still below 0.5 cd.** The card reads at
every rung down to 0.5 and fails only at 0. Values run about 5-10% above the
pre-Lumen column, which is the bounce contribution — small, because the card is
lit directly and GI only adds to that.

### Ladder 1 — troffer, Bay B, camera at the far end, 1110 cm down the run

| candela | near blocker | far blocker | far wall | floor | blocker contrast | legible? |
| --- | --- | --- | --- | --- | --- | --- |
| 0 (control) | 0.01 | 0.00 | 0.00 | 0.26 | 7.6 | **no** |
| 2 | 4.19 | 3.27 | 0.00 | 8.33 | 67.2 | **yes** — both blockers and the offset read |
| 6 | 15.98 | 13.97 | 0.69 | 30.42 | 141.3 | yes, comfortably |
| 15 | 37.00 | 33.44 | 3.94 | 66.81 | 200.8 | yes |

**The orientation floor dropped from 6 cd to about 2 cd.** Pre-Lumen, 2 cd gave
"faint silhouettes, offset not readable". With bounce the floor plane and the
lower walls fill in, and the offset arrangement reads.
See `docs/lightlab/fixtures/SHEET_bayB_troffer.png`.

### Ladder 2 — neon, Bay B, same pose

| candela | near blocker | far blocker | floor | blocker contrast |
| --- | --- | --- | --- | --- |
| 2 | 1.07 | 0.86 | 2.23 | 17.8 |
| 6 | 4.12 | 3.67 | 8.01 | 53.3 |
| 15 | 10.32 | 9.10 | 17.74 | 132.2 |

**Neon still costs about 3x the candela for the same surface luminance** — neon
at 6 cd lands almost exactly on troffer at 2 cd (4.12 vs 4.19 on the near
blocker, 8.01 vs 8.33 on the floor). So neon's orientation floor is about
**6 cd**, down from roughly 15. The 3x penalty for a narrow saturated source is
unchanged by GI; the whole scale simply shifted.
See `docs/lightlab/fixtures/SHEET_bayB_neon.png`.

### What changed, and why it changed that way

| | pre-Lumen | under Lumen |
| --- | --- | --- |
| Inspection ceiling | below 0.5 cd | **below 0.5 cd** (unchanged) |
| Orientation floor, troffer | 6 cd | **~2 cd** |
| Orientation floor, neon | ~15 cd | **~6 cd** |
| Gap between them | more than 12x | **about 4x** |

**Still no band.** But the gap closed by roughly a factor of three, and it did
so asymmetrically, for a reason worth keeping: **indirect light helps
orientation far more than it helps inspection.** Orientation is read off the
floor plane and off silhouettes against it, which is exactly what bounce fills
in. The price card was always lit directly by the fixture above it, so GI adds
only a few percent there. Lumen pushes down the floor without moving the
ceiling.

That does not rescue a uniform-fixture solution — a 4x gap is still a gap — but
it means the remedy in the next section needs to do less work than the
pre-Lumen numbers implied.

---

## The three answers

| | Result |
| --- | --- |
| **1. Inspection ceiling** — highest candela at which the Bay A price card is still NOT readable at 2 studs | **below 0.5 cd.** The card reads at every rung tested down to 0.5 cd. Only 0 cd fails. |
| **2. Orientation floor** — lowest candela at which Bay B's run and blockers are legible from 40 studs | **6 cd** for the troffer. At 2 cd only faint silhouettes survive. Higher for neon — see below. |
| **3. The band** | **There is none.** Floor ≥ 6 cd, ceiling < 0.5 cd: the floor sits more than **12×** above the ceiling. |

**Ladder 3, answered separately:** a spot reads as a **carried lantern at
about 25 cd**. It is invisible at 6, borderline floodlight at 100, and a plain
floodlight at **250 — the value `data/lantern.json` currently authors.**

## Conditions — every number above depends on these

**Two things about that exposure clamp, because every number in this document
depends on it.** `AutoExposureMinBrightness` and `MaxBrightness` are **legacy
luminance, not EV100** (`ExtendDefaultLuminanceRange` is 0), so the clamp of
1.0 is **EV100 3**; convert with `L = 0.125 x 2^EV100`. And `AutoExposureBias`
sits at **1 with its override off**, so it is inert — **if anyone enables it,
every reading in this document moves a stop.**

Exposure copied field-for-field from DockBay's volume and verified by reading
it back: priority 1, unbound, `AutoExposureMinBrightness` and `MaxBrightness`
both overridden to **1.0**, MegaLights on, bias left un-overridden. Note those
are **legacy luminance units, not EV100** — `ExtendDefaultLuminanceRange` is 0
on this project (see `docs/LIGHTLAB-PROTOCOL.md` §1).

- **One fixture per bay**, so every figure is a **per-fixture candela**, not an
  installed-aisle figure. Converting to a lighting plan means choosing spacing
  separately.
- **The troffer and the neon are both `RectLight`s, and that is worth a note
  added 2026-11-02.** This project runs **software** Lumen tracing, and the
  usual guidance for that mode is to prefer a wide-cone spot over a rect
  light: rect lights are stochastically sampled, noisier, and heavier on the
  GPU. **It does not invalidate anything above** - the ceiling and floor were
  read off surfaces lit *directly*, which is not where stochastic sampling
  shows up, and the whole conclusion is that direct light is the problem. But
  an authored aisle built from wide-cone spots is not quite the instrument
  these numbers were taken on, so expect the orientation floor to be the
  figure that moves if anyone re-measures with spots.
- Troffer = RectLight 120×60 cm, cool white (0.72, 0.84, 1.0), attenuation
  2000 cm. Neon = RectLight 120×8 cm, violet (0.75, 0.35, 1.0), same
  attenuation. Lantern = SpotLight, candelas pinned, 840 cm attenuation,
  72° outer / 17.28° inner, warm (1.0, 0.82, 0.56).
- Surfaces are `/Engine/EngineMaterials/DefaultMaterial` (mid grey); the price
  card is the lighter `BasicShapeMaterial`.
- Both gates passed before any rung was recorded: 0 cd vs 5000 cd differ
  (scene mean 3.86 → 149.76), and at 0 cd the card region is **pure black,
  mean 0.00, max 0** — the card is genuinely unreadable with the lights off.

## Ladder 1 — fluorescent troffer

Bay A, camera 2 studs (56 cm) from the card face. `card_contrast` is the
max−min luminance across the card region, which is what makes text legible.

| candela | scene mean | card mean | card contrast | card readable? |
| --- | --- | --- | --- | --- |
| 0 (control) | 3.8 | 0.0 | 0 | **no** |
| 0.5 | 5.7 | 24.3 | 30 | yes, dim |
| 2 | 22.4 | 82.0 | 96 | yes |
| 6 | 54.7 | 158.2 | 170 | yes |
| 15 | 93.0 | 205.5 | 197 | yes |
| 40 | 147.4 | 232.5 | 183 | yes |
| 100 | 184.2 | 246.3 | 134 | yes |

Bay B, camera 40 studs (1120 cm) from the far end:

| candela | run direction and blockers legible? |
| --- | --- |
| 2 | **no** — faint silhouettes, offset not readable |
| 6 | yes |
| 15 | yes, comfortably |

## Ladder 2 — neon tube

At matched candela the neon reads **materially darker and flatter** than the
troffer: at 6 cd in Bay B the blockers are visible but the floor and the run
barely register, where the troffer at 6 cd is comfortable. A narrow 8 cm source
in a saturated violet puts less usable luminance on surfaces than a 60 cm cool
white panel of the same intensity.

**So neon's orientation floor is higher than the troffer's**, in the region of
15 cd. Neon makes the contract *harder*, not easier: it buys less orientation
per candela while still lighting a card at close range.

## Ladder 3 — the lantern as a spot

| candela | reads as |
| --- | --- |
| 6 | **nothing.** A faint smudge of floor. This is the `_intensityWas6` incident made visible — 6 is not a dim lantern, it is an absent one. |
| 25 | **a carried lantern.** A warm pool on the floor ahead, walls only catching the edge, the room itself unlit. |
| 100 | borderline — walls and ceiling washing out, the room starting to read as lit rather than carried. |
| 250 | **a floodlight.** The near half of the room is bright, ceiling included, with a hard dark edge at the attenuation radius. |

**The current authored value of 250 cd is a floodlight, not a lantern.** On a
spot at 72° with an 840 cm reach, about **25 cd** is where it reads as
something a keeper is holding. This is the matrix cell that had never been
tested: the old 6 cd reading was taken on a point light and does not transfer.

That is a finding, not a recommendation — `data/lantern.json` is Chris's lane
and the number is his call. It is worth saying that dropping 250 → 25 is a
large change to the most consequential number in the game and should be felt
in play, not accepted from a lab.

## The directional-downlight remedy, tested — it only half works

The section below recommends "directional downlights that wash floor and lower
aisle while leaving vertical faces dark". Tested on 2026-11-01 under Lumen,
using the rect light's own barn doors, which is the "barn doors or narrow
cones" option named there. Same bays, same poses, same per-rung explicit
fixture state.

### In the run, barn doors do exactly what was hoped

Bay B, 6 cd, camera at the far end. "Vertical face" is a gondola blocker's
front face, which is where a shelf label lives.

| rig | floor | vertical face | floor : face | side wall |
| --- | --- | --- | --- | --- |
| plain troffer | 30.42 | 15.98 | 1.90 | 5.81 |
| **barn doors 15°** | **21.98** | **8.21** | **2.68** | **0.00** |
| barn doors 2° | 3.44 | 2.48 | 1.39 | 0.00 |

**15° is the useful setting.** The side walls go from 5.81 to **exactly zero** —
in `docs/lightlab/fixtures/SHEET_barndoor_vs_plain.png` the checker texture up
both walls simply disappears — while the floor keeps 21.98, far above the ~8
that marked the orientation floor. The run stays legible and arguably reads
*better*: the blockers now stand as silhouettes against a lit floor instead of
being lost against lit walls.

**2° is not selectivity, it is just darkness.** Everything drops, the ratio gets
*worse* than plain (1.39), and the run stops being legible — only a small pool
survives directly under the fixture. Worth recording because the Bay A result
below, read alone, would have suggested tightening further.

### But the price card barely notices

Bay A, 6 cd, camera 56 cm off the card face.

| rig | card mean | text contrast | side wall | readable? |
| --- | --- | --- | --- | --- |
| plain troffer | 168.45 | 172.7 | 23.84 | yes |
| **barn doors 15°** | **145.08** | **153.1** | **5.84** | **still yes** |
| barn doors 2° | 3.48 | 16.3 | 0.31 | no — but the room is dark too |

The side wall drops fourfold. **The card drops 11%.** And it cannot be direct
light: the card sits 34.6° off the fixture's vertical axis, well outside a 15°
cone.

### Why, and what it means

**Why — corrected 2026-11-01 by the albedo sweep below.** This section first
claimed the card was being lit by bounce off the floor pool it faces, reasoning
that 34.6° sits outside a 15° cone so the light could not be direct. **That was
wrong.** Darkening the floor sixfold changes the card by 0.7%, which rules
bounce out as its source.

The simpler explanation survives: **`BarnDoorAngle` is not a cone half-angle**,
and at 15° the fixture still emits directly at 34.6°. The card is directly lit
in every configuration that leaves the room usable. That is consistent with the
2° row, where the card finally goes dark — and so does everything else.

**The fixture is the wrong lever; the next ones to try are not lighting at
all.** Untested, in order of cheapness:

- **Floor albedo.** Bounce scales directly with floor reflectance, and the lab
  floor is mid-grey `DefaultMaterial`. A dark floor keeps the lit-pool read for
  orientation — you still see the pool and the silhouettes against it — while
  cutting what reaches the labels. This is art direction, not lighting, and it
  is the single highest-leverage untested variable in this whole document.
- **Label placement out of the bounce path** — recessed into a shelf, or on a
  surface that does not face the floor.
- **Self-luminous signage**, unchanged from the recommendation below.

A caveat on the Bay A rows: the editor window was resized between the 15° and
2° captures, so that last row's sampling boxes sit slightly differently. The
42x drop is far larger than that can account for, but the row is indicative
rather than exact.

## Floor albedo, swept — it does nothing, and that settles it

Plain troffer at its Lumen orientation floor of 2 cd, Bay A and Bay B floors
swept through three linear albedos against the `DefaultMaterial` baseline.
Every frame verified at a matched 1014x611 crop, because the editor window
resized mid-sweep and one rung had to be recaptured — at the wrong size its
sampling boxes land on different world content and the row is meaningless.

| floor albedo | Bay B floor | Bay B blocker face | Bay B blocker contrast | Bay A card contrast |
| --- | --- | --- | --- | --- |
| baseline (`DefaultMaterial`) | 8.33 | 4.19 | 67.2 | 102.7 |
| 0.18 | 19.55 | 4.73 | 68.1 | 103.2 |
| 0.08 | 11.42 | 3.98 | 67.2 | 101.9 |
| 0.03 | 7.25 | 3.67 | 67.1 | 100.6 |

**Darkening the floor sixfold costs 63% of the floor read and buys 2.5% of card
contrast.** The test fails its own success criterion by a wide margin: it harms
orientation far more than it helps inspection. The blocker face and its
contrast barely move either — those surfaces are directly lit too.

### And it fails in the barn-doored case as well

The sweep above uses a plain troffer, which lights the card directly, so bounce
was never the card's main source there. The cell where albedo *should* matter
is the barn-doored one, where the card was believed to be bounce-lit:

| Bay A, 6 cd | card contrast |
| --- | --- |
| plain troffer, baseline floor | 172.7 |
| barn doors 15°, baseline floor | 153.1 |
| barn doors 15°, floor albedo 0.03 | **152.1** |

**0.7%.** The dark floor does nothing there either, which is what falsified the
bounce explanation recorded above.

### The conclusion: there is no lighting-only solution

Three levers have now been tried against the inspection half of the contract
and all three fail:

| lever | effect on card | effect on orientation |
| --- | --- | --- |
| lower candela | card still reads at 0.5 cd, the bottom of the ladder | run goes dark first |
| barn doors 15° | −11% | fine, walls improve |
| barn doors 2° | card dark | run dark too, ratio *worse* than plain |
| floor albedo 0.18 → 0.03 | −2.5% | floor read −63% |

The card is **directly lit by any fixture broad enough to light the aisle**, and
nothing about the fixture's optics or the room's materials changes that without
taking the aisle with it. This refines rather than repeats the original
inverse-square argument: the problem is not that the card is close to its
fixture, it is that *direct* light is what reaches it, so every lever that acts
on indirect light misses.

**So legible detail has to be moved out of direct light paths** — recessed under
a shelf lip, angled away from the ceiling plane, or placed on faces the
fixtures do not see — rather than dimmed into illegibility. Combined with
self-luminous signage for landmarks, that is the whole remedy, and none of it
is a candela value.

## The recessed label — this one passes

Tested 2026-11-01. A shelf lip over the Bay A price card: real geometry that
blocks the ceiling fixture's direct path to the readable face while leaving the
relic silhouette visible from the aisle. Ambient troffer at 2 cd, the Lumen
orientation floor, throughout.

**Geometry.** The lip is **in the map**, as `StaticMeshActor_0` at
(323, 0, 110) scale (0.30, 0.60, 0.04) — the deep setting. It survived the
power cut, verified by explicit path after a `find_actors` sweep had wrongly
reported it absent: that sweep ran against `LLK_DockBay`, which was the open
level, so it was an empty answer from the wrong world
(`docs/TOOLING-TRAPS.md` 6). The spec below is the build recipe for both
settings. A `/Engine/BasicShapes/Cube` with `DefaultMaterial`,
horizontal, 60 cm wide and 4 cm thick, at **z = 110** — just above the card,
which spans z 75–105 at x ≈ 338. Depth is the variable:

- **shallow:** 10 cm, spanning x 328–338, actor at (333, 0, 110) scale (0.10, 0.60, 0.04)
- **deep:** 30 cm, spanning x 308–338, actor at (323, 0, 110) scale (0.30, 0.60, 0.04)

The depth that matters is set by the ray from the fixture at (200, 0, 290) to
the *bottom* of the card: it crosses z = 110 at x ≈ 315, so a lip must reach
about **23 cm** out to shade the whole face. 10 cm cannot; 30 cm can.

| rig, troffer 2 cd | card mean | text contrast | bay wall | readable? |
| --- | --- | --- | --- | --- |
| no lip | 92.29 | 102.7 | 4.92 | yes |
| shallow lip, 10 cm | 71.36 | **104.1** | 4.70 | **yes — unchanged** |
| **deep lip, 30 cm** | 10.33 | **14.8** | **4.64** | **no** |

**The deep lip is the first lever that works.** Text contrast falls sevenfold,
from 102.7 to 14.8, while **the bay wall is untouched — 4.64 against 4.92**.
Every earlier lever bought card darkness by taking light out of the room; this
one takes none. The shallow lip is a useful negative: it drops the card's *mean*
by shading its top edge while leaving the glyph band fully lit, so contrast does
not move at all. Shading a label is not the same as shading the part you read.

### The lantern can still reach it, and the lip does not block the player

Traced rather than eyeballed, with the lantern at a held position — (285, 20,
110), hand height and slightly off the centre line, aimed at the card:

| trace | first hit | path length |
| --- | --- | --- |
| lantern → card | 57.8 | 60.1 |
| player eye → card | 57.0 | 59.0 |

Both first-hits are the card face itself, so **neither the lantern's path nor
the player's read trace is occluded by the lip.** Geometrically the mechanic
works: the lip shades the fixture directly overhead and nothing at eye level.

### But the authored lantern destroys the label

With the deep lip and the lantern at that held position:

| lantern | card mean | text contrast |
| --- | --- | --- |
| 250 cd (as authored in `data/lantern.json`) | 254.89 | **3.4** |

**Unreadable, by blowing out rather than by darkness.** At 60 cm the authored
value clips the whole card to white. This is the same 250 cd that
`docs/LANTERN-LOOKDEV.md` found reads as a floodlight rather than a carried
lamp, now failing a second, independent test: it cannot perform the inspection
the store is being designed around.

### The held-lantern sweep, and where it stops being trustworthy

Run 2026-11-01 after the power cut. Ambient troffer 2 cd, deep lip, lantern at
the held position, camera 56 cm off the card. Only frames at a matched window
size are listed; `clipped` means the card region has reached 254+.

Measured on-screen (`CaptureEditorImage`, cropped and hash-checked).
`clip%` is the fraction of the card region at 254+; `glyph range` is the 5th-to-
95th percentile spread inside the text band, which is what actually separates
ink from paper.

| lantern | card mean | clip% | glyph range | text contrast | verdict |
| --- | --- | --- | --- | --- | --- |
| 0 (ambient only) | 10.33 | 0.0 | 13.6 | 14.8 | unreadable — the lip working |
| **6 cd** | 246.84 | **0.0** | **104.2** | 141.4 | **reads cleanly** |
| 15 cd | 252.05 | 36.3 | 52.3 | 79.6 | clipping begins |
| 40 cd | 254.03 | 94.7 | 21.1 | 35.8 | blown |
| 250 cd (authored) | 254.89 | 94.9 | 1.4 | 3.4 | fully blown |

**Correction.** This table first said "everything from 6 cd upward clips", on
the strength of the card's high *mean*. That was wrong, and the peer session
caught it. **At 6 cd nothing clips at all** — 0.0% of the card is at 254, and
the glyphs sit at 148.7 against paper at 252.9, a clean 104-point separation.
The mean is high because the paper is bright, which is not the same as the
label being unreadable. Clipping starts between 6 and 15 cd.

So the readable window is **not** below 6 cd. Its upper bound is somewhere
between 6 and 15, and 6 cd is inside it. The authored 250 cd is still roughly
two orders of magnitude too bright, and contrast still collapses monotonically
as intensity climbs — but the usable region is wider and higher than this
document first claimed.

**The lower bound remains unmeasured**, for the reason below.

**Below 6 cd is unmeasured and must not be inferred.** The 0.75 cd and 2 cd
captures came back **byte-identical to each other** (md5 96426774) and match the
6 cd frame to 0.01, so the viewport had stopped redrawing — tooling trap 7, and
the only reason it was caught is that every frame is hashed. A 25 cd row was
also discarded: its capture arrived at a different window height, and its card
value broke the monotonic trend, which is the tell.

**An offscreen attempt was discarded entirely.** `CaptureViewport` reported the
ambient-only card at 99.06 with contrast 202 — plainly readable — where the
on-screen measurement of the identical state gives 10.33 and 14.8. A tenfold
disagreement with a known-good reading, so the whole offscreen sweep was
dropped. See `docs/TOOLING-TRAPS.md` 12; this is a second, independent way that
instrument lies.

### The caveat that governs all of it

**This clipping is a consequence of the locked exposure.** The lab's
post-process volume pins `AutoExposureMinBrightness` and `MaxBrightness` to 1.0,
copied from DockBay, so the view is calibrated for a dark store and anything
held at arm's length saturates almost at once. If the shipping game uses
adaptive exposure, a held lantern will not behave this way and these numbers do
not transfer. **Whether exposure is locked is now a gameplay decision, not a
lookdev one** — it decides whether close inspection is even possible.

### Retracted: they do reproduce, and the restart claim was wrong

**This section previously said the 6 cd reading failed to reproduce after an
editor restart, and suspended the table above. Both halves were wrong.**

**They reproduce exactly.** Re-measured on the same rig: card **246.83** against
the original 246.84, bay wall **28.56** against 28.56. The lantern table stands.

**There was no restart between the two runs.** The engine log runs on UTC and
the machine on EST. The only `Engine is initialized` in the log is at 18:18:44
UTC = **13:18:44 local**, which is the power-loss recovery — and both readings
(14:37 and 15:59 local) came after it. The claim was built by reading a log
timestamp as a local one.

**What the anomalous frame actually was is still unexplained.** It read the
wall at 154.48 where the settled value is 28.56. A per-tile ratio against the
good frame ranges from **1.31 to 7.51** — not the single constant a failed
exposure lock would produce, so the lock was not silently off. A decay
hypothesis (indirect light still falling from a preceding 500 cd test) was
tested directly and **not supported**: a 2 cd frame captured immediately after
500 cd reads the wall at 27.11, essentially settled. One frame, cause unknown,
recorded as an anomaly rather than reasoned from.

### The hazard that is demonstrated: capture geometry silently rescales everything

Chasing the retraction above produced one solid result. The **identical scene**
measures:

| capture size | card mean | bay wall |
| --- | --- | --- |
| 1014 x 612 | 246.83 | 28.56 |
| 1014 x 532 | 140.22 | 27.11 |

**The card moves by a factor of 1.76 with no change to the scene**, because the
crop uses fixed pixel insets, so the sampling boxes land on different world
content. The wall barely moves, because that region is uniform — **the error is
largest exactly where the measurement matters and invisible exactly where it
would be noticed.**

**The mapping, since two different numbers get quoted.** The capture is the
editor window minus the crop insets: capture = window - (266, 161) for the
current rect. So window 773 gives 612, 772 gives 611, 693 gives 532. The window
has arrived at all three during this project without anyone resizing it, and
the inset rect itself changed between sessions, which is why older frames are
1020 wide rather than 1014.

### Audit of every committed frame, and what survives

PNG files record their own dimensions, so the geometry nobody wrote down is
recoverable for every reading ever taken. The audit reads the IHDR of every
committed frame; see the regeneration command in `docs/LIGHTLAB-PROTOCOL.md`.

**The load-bearing sets are internally consistent, which is the result that
matters.** Each held its geometry within its own run: the lantern ladder
`LNT_006/015/040` all 1014x612; `TA_*` and `TB_*` uniformly 1014x611; `LIP_*`
likewise; the core ladder `N_e*` uniformly 1014x611; the offscreen `v_e*` set
uniformly 1669x1012. Today's re-measurement
(`reproducibility_after_restart.png`) is 1014x612, matching the lantern set
exactly, so the retraction above stands on its own geometry.

**Five frames do not match the set they belong to:**

| frame | is | its set is |
| --- | --- | --- |
| `trofA_100cd.png` | 1020x671 | 1020x612 |
| `GATE2_00cd_A2.png` | 1020x671 | — |
| the five ambient controls | 1020x606 | the ladders they gate are 1020x612 / 1014x611 |
| `spotC_250cd.png` | 1020x612 | the LNT set is 1014x612 |
| **`DA_06_tight.png`** | **1014x532** | **the barn-door set is 1014x611** |

The controls are the worst of these, because a control exists to be compared
against a reading. `DA_06_tight` is the 2 degree barn-door row, already
caveated in this document as indicative rather than exact — the caveat was
right, and the magnitude is now known rather than guessed: a geometry change of
this kind moves the card by about 1.76x, and that row reports a 42x drop, so
the conclusion survives comfortably.

**Nothing needs re-shooting.** The dimensions are recoverable for every frame,
so the repair is an annotation pass over the existing tables, not a re-run.

### A 6 cd lamp should not light a room

At 6 cd the bay's **ceiling reads 123.4** and its walls 28.6. A six-candela
lamp held at arm's length is lighting an entire room, which no plausible eye
would see. That is not a lantern result, it is a camera result: the exposure is
pinned at 1.0 with no latitude, and **that constant was copied from DockBay
rather than chosen for a lantern**.

Which reframes the problem. A 178x range is about **7.5 stops** — an
unremarkable scene range that real vision and film handle without complaint.
Nothing clips because the range is too wide; it clips because the view has no
latitude and its lock point was never defended. This is the same disease as
250 cd: a number tuned by eye against an exposure constant nobody had chosen.

**So the next experiment is one number, not a redesign.** Keep exposure locked —
locked brightness is honest and learnable, which is worth keeping — but set the
lock point for the lantern's working range and re-run this identical sweep. It
either opens a usable window below 6 cd or proves there is none. Credit to the
peer session for this branch; it is better than the fork below, which should
not be settled until this is tried.

### Resolved in play, not in the lab: light as a rate

**Chris's playtest of `LLK_LanternWalkLab`, 2026-11-01.** A design result
rather than a measurement, recorded here because it answers the fork below.

Pickup is currently instant. The proposal is **hold-to-interact, with the hold
shortened when the lantern is lit.**

That dissolves the fork rather than picking a side. The question underneath
every lighting finding in this document has been whether illumination should
*gate* inspection — readable or not, a threshold to be found — or whether
inspection should stop depending on illumination altogether. Hold-to-interact
makes light **continuous**: it stops being a yes/no and becomes a rate.

Three things follow, and they are why this is worth more than a tuned number:

- **It removes the need for a threshold.** Every band this document failed to
  find — the inspection ceiling against the orientation floor, amber against
  useful output, the readable lantern window — was a search for a line
  separating "legible" from "not". A rate has no line to find.
- **It is robust to everything the instrument got wrong.** A contrast of 104
  against 176 is a large difference in how *fast* a relic resolves and a
  marginal one in whether it resolves at all. The exposure constant stops being
  load-bearing for whether the game functions.
- **It generates the central tension from two systems that already exist.**
  Holding still in the light is exactly what makes a Sweeper your problem. The
  cost of inspecting is paid in the currency the game already charges in, with
  no new mechanic.

**This does not retire the exposure work.** The orientation half still decides
whether the store reads as navigable, which is about whether the space is
legible rather than whether a relic is. But it does mean the lock point is a
look decision rather than a functional one, and that is a much safer thing for
it to be.

### The third appearance of one shape

The lantern has to light a room at 2–8 studs *and* read a label at 0.6 studs.
Inverse-square across that range is about **178x**. No single intensity serves
both, which is why 25 cd can read as a carried lamp in
`docs/LANTERN-LOOKDEV.md` and still destroy a label here — both are true.

That is the same structural result as the fixture contract and the lantern
core, reached a third time by a third route: **the store's lighting problems are
not tuning problems.** Either the lantern's output varies with what it is doing
(a focus or shutter state, which the housing already implies), or inspection
stops depending on raw illumination.

### Where this leaves the contract

The store can withhold inspection detail physically — geometry does what no
candela value could. What is still open is the instrument: at its authored
intensity the lantern over-lights the thing it is meant to reveal, so the
inspection mechanic needs the lantern number settled before it can be tuned.

## Why there is no band, and what to do instead

The gap is not a tuning failure. **It is inverse-square geometry.** The price
card sits roughly a metre and a half from its fixture; the far end of the aisle
is eleven metres from its own. A single uniform fixture bright enough to put
usable light eleven metres down a run will always put far more than enough on a
card at arm's length. No candela chosen for one job can fail the other.

So the contract cannot be met by dimming. It needs fixtures whose light does
not land where reading happens:

- **Directional downlights** that wash floor and lower aisle while leaving
  vertical faces dark. Orientation comes from the floor plane and from
  silhouettes against it; a price card or relic label is vertical.
- **Barn doors or narrow cones** on the retail fixtures, so spill is
  controlled rather than ambient.
- **Luminous sources that are visible without illuminating** — exit signs and
  neon read as landmarks from a distance by being bright *objects*, not by
  lighting their surroundings. `docs/THE-STORE.md` already asks for exactly
  this ("neon as a broken retail memory", a functioning exit sign at a distant
  edge). That is the contract-compliant instrument and it is already in the
  art direction.

The last point is the cheerful one: the store's own visual language —
self-luminous signage read as landmarks, with genuine dark between — satisfies
the contract where uniform ambient fixtures cannot.

## What this does not establish

- **The card is white with black text: maximum contrast.** A relic's engraved
  detail is far subtler, so the ceiling for *relic* legibility is higher than
  0.5 cd. The contract names "a price tag, engraved detail or relic label",
  and the price tag is the easiest of the three — which makes it the right
  one for a ceiling, since a fixture that passes for a price card passes for
  all of them.
- **Bay A's 4 and 8 stud positions were not shot**, and neither was neon's
  full Bay A column. The ceiling is bounded from above by a readable 0.5 cd
  rather than located between two rungs, and the exact figure below 0.5 cd is
  unmeasured. Neither gap changes the conclusion: the band's absence is a
  12-fold gap, not a close call.
- Nothing here measures how the fixtures interact with the **lantern** in the
  same room, which is the case the store actually presents.

Screenshots and contact sheets: `docs/lightlab/`. The lab is
`Content/LLK_LightLab.umap`, saved with all three fixtures at 0 cd, and the
method is `docs/LIGHTLAB-PROTOCOL.md`.

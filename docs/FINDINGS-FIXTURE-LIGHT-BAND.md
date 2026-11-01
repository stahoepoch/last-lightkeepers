# The fixture light contract has no band

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

Exposure copied field-for-field from DockBay's volume and verified by reading
it back: priority 1, unbound, `AutoExposureMinBrightness` and `MaxBrightness`
both overridden to **1.0**, MegaLights on, bias left un-overridden. Note those
are **legacy luminance units, not EV100** — `ExtendDefaultLuminanceRange` is 0
on this project (see `docs/LIGHTLAB-PROTOCOL.md` §1).

- **One fixture per bay**, so every figure is a **per-fixture candela**, not an
  installed-aisle figure. Converting to a lighting plan means choosing spacing
  separately.
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

**Under Lumen, a downlight that washes the floor turns the floor into a
secondary source pointed straight up at every vertical face above it.** The
card faces that lit pool from 90 cm up. Barn doors control what the fixture
emits; they cannot control what the floor re-emits.

So the remedy fails for *precisely the mechanism that made it work in the run*.
Bounce off the floor plane is what dropped the orientation floor from 6 cd to
2 cd, and it is what keeps the price card readable. One mechanism, both
results. Aiming light downward cannot darken a label that faces a lit floor.

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

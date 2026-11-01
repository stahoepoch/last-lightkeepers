# The fixture light contract has no band

Measured 2026-10-31 by Claude in `LLK_LightLab`, against
`docs/FIXTURE-LIGHT-CONTRACT.md`'s rule that ambient fixtures *"may help
players navigate, but must never look bright enough to read a relic under."*

**There is no candela value that satisfies both halves.** The orientation floor
is at least twelve times the inspection ceiling. Uniform fixtures cannot meet
the contract, and the answer is directional ones.

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

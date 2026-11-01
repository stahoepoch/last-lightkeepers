# Lantern lookdev — A/B/C for Chris to pick from

Built 2026-10-31 by Claude in `LLK_LightLab`. **This is a comparison, not a
decision.** Nothing here changes `data/lantern.json`, gameplay code, or the
gameplay light field. Chris picks which one feels carried.

**Contact sheets:** `docs/lightlab/lantern/SHEET_close.png` (the important one)
and `docs/lightlab/lantern/SHEET_far.png`. Individual frames alongside them.

---

## The finding the rigs are built on

The lantern does not feel carried, and **intensity is not why**.
`data/lantern.json` authors `coneAngleDegrees: 72` on a `USpotLightComponent`,
which is a point emitting a directional cone — a flashlight. No intensity value
fixes a cone. The three rigs vary the *shape and the housing*, not the brightness
(beyond the one measured change from 250 → 25 cd).

Candela on a spot is **independent of cone angle**: widening the cone changes
how much of the room is lit, not how brightly a lit surface is lit. So the
~25 cd measured in `docs/FINDINGS-FIXTURE-LIGHT-BAND.md` stays valid across
every cone below, and cone and intensity can be tuned separately.

## The rigs

| | A — control | B — wide hood + visible source | C — B plus near-field fill |
| --- | --- | --- | --- |
| Outer cone | 72° (as authored) | 170° | 170° (and a 110° variant) |
| Inner cone | 17.28° | 60° | 60° / 40° |
| Intensity | 250 cd (as authored) | 25 cd | 25 cd |
| Colour | warm RGB tint 1.0/0.82/0.56 | **2700 K** via Use Temperature | **2700 K** via Use Temperature |
| Source radius | 0 (a mathematical point) | 0 | **10** — soft shadow edges |
| Shadows | on | on | on, **wash only** |
| Near-field fill | — | — | 3 cd point, 300 cm, **shadow casting OFF** |
| Visible housing | none | caged chamber in frame | caged chamber in frame |

All three: attenuation 840 cm, candelas pinned, same camera positions.

**2700 K, electric, confirmed.** The lantern is a sealed old service
instrument — caged glass chamber, hood and shutter, institutional rather than
cosy. No combustion flicker is built into any rig, deliberately: any future
instability should read as failing circuitry or storm interference, which is
brief and electrical, not a continuous organic flutter. Nothing here should be
inherited as an approved flame.

## The housing is doing most of the work

The caged chamber is six primitives parented to the lantern body: a hood above,
a rear plate, three cage bars, and a base disc. They are stand-ins, not art.
What they buy is visible in `SHEET_close.png` and it is the whole point:

- **A dark disc directly beneath the housing**, from the base plate.
- **Bar shadows** thrown across floor and wall by the cage.
- **Limited rear spill** — the rear plate physically explains why light goes
  forward and down, which is what sells "a hood" instead of "a cone".

Those three cues come from **geometry, not light settings**, and they are what
separate B and C from A far more than the cone angle does. A is a bright wash
with no source and no story about where the light comes from.

## What the frames show

**Close (2 studs, hands and near floor)** — `SHEET_close.png`, four rows:

- **A** blows the near floor and the blocker face to near-white. No visible
  source, no shadow pattern, nothing that reads as held.
- **B** reads as a lantern immediately: glowing chamber in frame, hard cage-bar
  shadows, the dark disc beneath, warm wash falling off into dark.
- **C** is B with softer shadow edges and the housing itself legible as an
  object — the near-field fill lifts the underfoot floor and the lamp body out
  of silhouette. **This is the one that reads as carried equipment.**
- **C at 110°** is a tighter pool with more edge falloff. Still carried, more
  directed. Worth seeing next to 170° because Chris described a hood *and a
  shutter*, which implies the aperture is meant to vary.

**Far (40 studs down the Bay B run)** — `SHEET_far.png`:

- **A** is a narrow hard-edged beam down the aisle with everything outside it
  black. Unmistakably a flashlight.
- **B** lights the near aisle and both walls with a soft falloff into dark.
- **C** is the most enveloping — ceiling catches light, near floor is lit,
  edges soft.

## Limits, stated

- **The source is hidden in the far frames, in all three rigs.** The emissive
  stand-in is a plain engine material at full brightness, and held near the
  lens it blooms into a white dome that swamps the frame. Hiding it makes the
  three *reach* comparisons fair, which is what the far pose is for. Dimming it
  properly needs a material instance, which would mean a new asset in
  `Content/` and is outside this brief.
- **The mid pose (8 studs) is only partly usable** for the same reason: `rigA_mid`
  and `rigB_mid` are included, `rigC_mid` is dominated by the bloomed stand-in.
  The close and far sheets carry the comparison.
- The editor window was resized mid-session, so framing is not pixel-identical
  between the close set and the far set. **Within each sheet the framing is
  identical**, which is what matters for flipping between them.
- No IES profile was tried (optional item D). It shapes distribution only and
  costs nothing at runtime, so it remains worth a pass later.

## What this does not touch

`data/lantern.json` is unchanged — `intensity` is still 250 and
`coneAngleDegrees` still 72. No gameplay code was modified. The gameplay light
field reads `GetGameplayRadiusCm()`, not `AttenuationRadius` (`d065469`, with a
test that moves attenuation behind the game's back and requires the gameplay
answer not to follow), so **nothing in this lab is capable of changing whether a
relic is readable.** That separation is deliberate and was left intact.

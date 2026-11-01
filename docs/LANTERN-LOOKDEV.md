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

Twelve captures: **A, B, C170, C110 at close, mid and far**, identical framing
within each distance, all clean viewport crops at 1014x610.
`SHEET_close.png`, `SHEET_mid.png`, `SHEET_far.png` are 2x2 grids in that
order (A top-left, B top-right, C170 bottom-left, C110 bottom-right).

**Close (2 studs, hands and near floor)**

- **A** blows the near floor and blocker face to near-white. No visible source,
  no shadow pattern, nothing that reads as held.
- **B** reads as a lantern immediately: glowing chamber in frame, hard cage-bar
  shadows, the dark disc beneath, warm wash falling off into dark.
- **C170** is B with softer shadow edges and the housing legible as an object —
  the fill lifts the underfoot floor and the lamp body out of silhouette.
  **This is the one that reads as carried equipment.**
- **C110** is a tighter, more directed pool. Still carried, more purposeful.

**Mid (8 studs, a shelf edge)**

- **A** floods the whole corridor — both walls, the floor, the far end — and
  blows the near blocker to white.
- **B / C170 / C110** put a warm pool on the near blocker and wall and let the
  corridor beyond fall away. C110 is marginally more contained than C170; the
  two are close enough that the aperture is a taste call, not a legibility one.

**Far (40 studs down the Bay B run)**

- **A** lights the entire corridor: both walls to full length, the ceiling, the
  floor, the far wall. It is a flashlight that happens to also be a room light.
- **B / C170 / C110** give a modest warm pool that dies within a few metres,
  with the ceiling and the far end of the run genuinely dark.

## Correction: 170 degrees does NOT turn the corridor into a lit room

The earlier far sheet suggested that at 170 degrees the lantern lights the
ceiling and most of the corridor, and that this might be wrong for a game whose
premise is that darkness is a place you enter.

**The clean captures do not support that, and reverse it.** At far range the
170-degree rigs leave the ceiling and the far half of the run dark; it is
**A — the authored 72 degrees at 250 cd — that lights the whole corridor,
ceiling included.** The wide cone spreads 25 cd thinly over a large solid
angle, so each surface gets little; the narrow cone concentrates 250 cd and
then bounces it off everything.

The earlier impression came from a compromised sheet: full editor screenshots
at a different aspect, with the emissive stand-in blooming and the lantern at a
different offset. It was a real observation about a bad image. **Worth keeping
as the reason the recapture was demanded** — the matched-framing requirement is
what made the difference visible.

So the premise is safe. If anything the risk runs the other way: at 25 cd the
wide rigs may be *too* contained, and the number to argue about is intensity,
not cone.

## Limits, stated

- **The source is visible in B, C170 and C110 at every distance**, at a reduced
  body scale (0.08 close, 0.05 mid and far) so the plain engine emissive does
  not bloom over the frame. Dimming it properly needs a material instance,
  which would be a new asset in `Content/` and is outside this brief.
- The cage is six stand-in primitives, not art. It is there to prove the cues
  exist, not to propose a silhouette.
- **C170 and C110 are close at mid and far.** The aperture reads mostly at
  close range. If the shutter is going to be a visible mechanic, it will need
  to move further than 170 → 110 to be felt down a corridor.
- No IES profile was tried (optional item D). It shapes distribution only and
  costs nothing at runtime, so it is still worth a later pass.

## What this does not touch

`data/lantern.json` is unchanged — `intensity` is still 250 and
`coneAngleDegrees` still 72. No gameplay code was modified. The gameplay light
field reads `GetGameplayRadiusCm()`, not `AttenuationRadius` (`d065469`, with a
test that moves attenuation behind the game's back and requires the gameplay
answer not to follow), so **nothing in this lab is capable of changing whether a
relic is readable.** That separation is deliberate and was left intact.

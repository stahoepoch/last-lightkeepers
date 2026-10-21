# DON'T FUCK UP THE LIGHTS

This game is about carrying a lantern into a dark place. If the dark stops
being dark, there is no game. The lighting has been broken and re-broken
several times, each time by someone changing one number in good faith.

**Read this before you touch `Lighting` in `default.project.json`, before you
add a light to a map builder, and before you add anything that draws its own
brightness.**

## The two rules

**1. `Ambient` stays at `0, 0, 0`.**

`Ambient` is a flat fill added to every surface in the game, lit or not. It
does not care about `Brightness`, it does not care about exposure, and it does
not care whether a single lamp is switched on. Raise it and the whole world
goes milky, and no lighting change you make afterwards will appear to do
anything — because the thing washing the screen out is not a light.

It was set to `46, 52, 64`. With a near-white palette (plaster 238,232,222,
terrazzo 196,186,164, mall tile 166,161,150) that alone is enough to blow the
picture out.

**2. `EnvironmentDiffuseScale` stays at or near `0`.**

Sky bounce. Indoors it fills every room from above with no regard for the roof
over it. It was set to `0.72`. If you want some, raise it a little and look at
the Galleria court and the Lanternhold basement before committing.

## How we proved it

Not by eye, and not by arguing. Bisect, in a live session:

1. Set `Ambient`, `OutdoorAmbient`, `EnvironmentDiffuseScale`,
   `EnvironmentSpecularScale`, `Brightness` and `ExposureCompensation` all to
   zero, and disable every light in `Workspace`. **The screen went black.** That
   is the control: it proves nothing else is drawing.
2. Add back the sun (`Brightness 0.45`), the sky (`OutdoorAmbient 28,34,48`)
   and all 45 lamps, leaving `Ambient` and `EnvironmentDiffuseScale` at zero.
   **The result was correct** — dark mall, warm pool of light around the
   keeper, readable shopfronts.

The difference between step 2 and the broken build is exactly those two
numbers.

## The known-good set

```json
"Ambient":                  [0, 0, 0],
"OutdoorAmbient":           [28, 34, 48],
"Brightness":               0.45,
"ExposureCompensation":     -0.05,
"EnvironmentDiffuseScale":  0,
"EnvironmentSpecularScale": 0,
"ShadowSoftness":           0.35,
"ClockTime":                2,
"GlobalShadows":            true,
"Technology":               "Future"
```

`GlobalShadows` must stay `true`. With it off, interiors take the full sky as
if they had no roof, and the hub renders as a white box. `FloodedMallBuilder`
used to set it `false` at runtime; that builder is retired.

## Things that ignore the lighting entirely

These draw at full brightness no matter what the Lighting block says. Every
one of them has caused a "the lighting is broken" report:

| thing | rule |
|---|---|
| `Material.Neon` | Only tiny objects meant to glow in the dark — a Sweeper's eye. **Never a panel.** 14 Neon ceiling lenses in the Galleria washed the whole mall white. |
| `Beam` with `LightInfluence = 0` | **Always set `LightInfluence = 1`.** At 0 a beam is a hard strip of light that no lighting change can touch. The hub's floor guide was one of these. |
| `SurfaceGui` / `BillboardGui` | Unlit by nature. Fine for text on a dark background; never a large pale panel. |

## Adding a lamp

The house lamp is **brightness 1.5, range 42, `Shadows = false`**. Match it.

If you think something needs to be brighter than the house lamp, it almost
certainly needs to be the *only lit thing nearby* instead. The departure
markers were set to 2.2 and 2.6 at range 52 to make them obvious; they just
blew out the end of the hall, and dropping them to 1.2 and 1.4 read better.

Shadows stay off on lamps: a lantern that casts them strobes as you walk past
a row of posts.

## The carried lantern is not a lamp

`KeeperLantern.Lens.Lantern` is **brightness 2.4, range 34** — brighter than
anything in either map, and that is deliberate. It is the light the game is
about.

Which means it sets the ceiling on how pale a surface can be. Stand next to a
column or a wall and a 2.4 light at a few studs will saturate any surface with
a high albedo — that is what "the centre area is all white" turned out to be
the second time, long after the Lighting block was correct. The court ring
tested 80 of 80 points walkable and 80 of 80 legs pathable at the time; the
geometry was fine and the surface next to the player was simply blown.

**The lantern and the palette have to be tuned against each other.** If you
lower the lantern the world goes further away; if you keep it, the surfaces it
stands next to cannot be near-white. The mall tile has already come down once
from 232,228,217 to 166,161,150 for this reason. Plaster at 238,232,222 and
terrazzo at 196,186,164 are the next candidates if close-range blowout is
still a complaint.

Do not "fix" this by raising exposure or dropping `Brightness`: those are
global, and they will take the dark parts with them.

## The protocol

1. Change one number.
2. Look at **both** maps — the Galleria court and the Lanternhold basement.
   They fail differently; a value that looks fine in the hub can white out the
   mall.
3. If it looks wrong, bisect from the black control above. Do not guess, and
   do not stack another change on top of a change you have not verified.
4. `node --test tests/*.test.mjs` guards the values in this file. If
   `tests/lighting.test.mjs` fails, you changed something this document says
   not to. Read the failure before you edit the test.

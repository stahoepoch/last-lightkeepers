# The store is not empty. It is unlit.

**2026-11-22.** Chris: *"boscovs still doesn't look like a real boscovs...
the department store doesn't look like a department store. It's empty... I
just don't see the realism with it."*

He is right about what he sees and wrong about the cause, and the difference
decides what to do about it. `HANDOFFS.md` M24.

---

## Nobody had ever looked

Every judgment made about this map — mine, in three sessions — came from log
lines. On 2026-11-22 it turned out a rendered screenshot is one command:

```
UnrealEditor-Cmd.exe <project> /Game/<Map> -game -windowed -resx=1600 -resy=900
  -ExecCmds="ghost, BugItGo X Y Z Pitch Yaw Roll, HighResShot 1600x900"
```

`BugItGo` puts the camera anywhere, `ghost` frees it from collision, and the
PNG lands in `Saved/Screenshots/WindowsEditor`. **That is the single most
useful thing found today**, and it is the reason the diagnosis below is right
rather than plausible: Chris is remote and cannot play, and now neither of us
has to guess.

## What the census said

`Tools/census_boswells_opening.py` and `census_boswells_floor.py`:

| | |
| --- | --- |
| actors | **2,249**, of which 2,047 are static meshes |
| fixtures | **1,638**, roughly one every 3 m² of sales floor |
| sales floor | 87 m × 54 m — a believable anchor-store footprint |
| floor material | **76.3% one material**, `M_BSW_StoneTile`, 16,207 m² |
| carpet | 12.6%, and **all of it the same carpet**, the Kids sage |
| placeholder materials | **148 components** on `WorldGridMaterial` or `BasicShapeMaterial`, 75 of them on `ROUTE_` actors |

So the store is **not** under-furnished. It has 1,638 fixtures standing in it.

## What the screenshot said

The first picture, from the sales floor looking south: a tiled racetrack
aisle, a well-modelled escalator catching light, ceiling soffits with recessed
fixtures, a lit `EXIT` sign, a blue `JUNIORS` department sign, and the mall
entrance glass at the far end.

**And pure black either side of it**, where all 1,638 fixtures are.

## The cause, measured

`Tools/audit_boswells_ambient.py`:

```
AMBIENT SkyLight=0
AMBIENT DirectionalLight=0
AMBIENT ExponentialHeightFog=0
AMBIENT SkyAtmosphere=0
AMBIENT SpotLight=102  PointLight=24
AMBIENT total_authored_intensity=1990
AMBIENT PPV BSW_EXPOSURE_LOCK min=0.5 max=0.5 (both overridden)
```

**There is no ambient light source of any kind in this map.** No sky light, no
directional, no fog, no atmosphere. The only light is 102 emergency spots and
24 ceiling fills, which pool on the racetrack and leave the volume between
them at absolute black. Exposure is locked at 0.5 so nothing adapts.

84% of the floor is within 9 m of *a* light, which is why "add more lights"
was the wrong instinct — the lights are spread fine. Nothing fills the air
between them.

This also means `ROADMAP.md` **slice 2 was failing and nobody knew**: *"the
chapter remains navigable with the lantern off"* is false anywhere off the
racetrack, because off the racetrack there is nothing to navigate by.

## The fix

`Tools/light_boswells_departments.py` lays 20 unshadowed movable point fills
at 1,150 cm, 42 m radius, **4.5 to 14 candela** against emergency spots
averaging ~16. Cold daylight-blue, so the building's own amber emergency light
stays tellable apart by eye.

**They fade with distance from the mall entrance glass**, which is the
brightest thing in the building and the only honest ambient source an enclosed
anchor store has. Deeper into the store is darker — what the architecture
implies, and what the chapter wants, since the lantern should matter more the
further in you go.

A sky light was the obvious answer and is the wrong one: in an enclosed
interior it captures the interior, which is black, and contributes nothing.

### What it changed

Same camera, before and after. Before: two lit strips and black. After:
**`LADIES`** on the left with racks of hanging clothes, a mannequin and a cash
wrap; **`SHOES`** opposite with rows of racking; the carpet visible underneath
them; ceiling beams and soffits reading as structure; the escalator in context
rather than floating in void. At the dock, `RECEIVING` became legible on the
back wall.

Not one fixture was added. All of it was already there.

## The regression this caused, and the gate that caught it

Saving the map from a commandlet **destroyed the navmesh** — `NavmeshNotBuilt
- 2 nav data actor(s) and NONE with any built tiles`. The Sweeper could not
path anywhere. `docs/MAP-NAV-WORKFLOW.md` exists for exactly this and its step
2 restored it; the map grew 518 KB of tile data and the gate went back to
`Map readiness: ready` with `11045 cm of path ... 65.7 s`, identical to before.

**The first run of step 2 exited 255 and did nothing**, and a second identical
run exited 0 and wrote the tiles. That is worth knowing: the build is not
reliably idempotent on the first attempt, so the gate — not the resave's exit
code — is what says it worked.

## What this does not claim

- **It is not finished.** This lifts the departments off black. It does not
  make them look like a Boscov's.
- **The floor is still 76% one material.** Chris asked twice for *"walkways
  that are linoleum or ceramic, and then there's carpeted areas"* and the map
  has the inverse: a tile field with a few carpet patches, all the same
  carpet. Inverting that — hard racetrack, carpeted departments, a different
  carpet per department — is the next largest realism gain and is untouched.
- **148 components still wear engine placeholder materials**, 75 of them on
  route geometry, which renders as the grey checkerboard.
- **The dock is still nearly black.** It is a back-of-house space and some of
  that is right, but Chris's opening happens there and it has to be watchable.
- **Nobody has judged the brightness in motion.** It is tuned from three
  stills. The number to move is `NEAR_INTENSITY`/`FAR_INTENSITY` in the tool,
  and the tool is idempotent, so a retune is one run.
- **Chris offered to walk Boscov's with a camera.** For floor transitions,
  fixture heights, sign placement and what the ceiling actually does, that
  footage would beat any amount of guessing from here.

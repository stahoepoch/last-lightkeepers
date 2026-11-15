# Boswell's spatial plans

## Full anchor: authored two-floor layout

`BoswellsFullStore_Plan.svg` and `BoswellsFullStore_Sockets.csv` cover the
existing 87 × 54 m Anchor plates, not just the first playable room below.
Regenerate both with `python Tools/plan_boswells_store.py`. The generator
checks that 104 named fixture sockets stay within the plates, do not overlap
one another or reserved routes, and that a 50 cm clearance grid connects the
entry/escalator landing to the sales wings and service end on each floor.
The lower sockets cover 9.3% of the plate; upper sockets cover 8.8%. The
routes reserve at least 300 cm in the plan. The lower service wall has two
400 cm doorways; the route checker reaches the dock through one of them.
Fixture *kinds* distinguish
clothing runs, four-ways, round racks, shoe benches, toy gondolas, beds,
sofas, counters, and service rooms. A socket is a place to dress with art,
not a finished actor or permission to fill it with one generic cube.

The plate dimensions and department adjacency come from
`docs/PLAN-AUDIT-BOSWELLS-ANCHOR.md` and Chris's recollection recorded there.
The public Galleria map confirms Boscov's as a two-level anchor; it does not
publish Boscov's internal fixture plan. This is an authored level layout.

### Meriden Boscov's visual reference, 2015

[CT Mommy Blog's Meriden store visit](http://ctmommyblog.com/boscovs-meriden-ct/)
includes photographs of that store's [first-floor diagram](http://ctmommyblog.com/wp-content/uploads/2015/10/photo-2.jpg),
[second-floor diagram](http://ctmommyblog.com/wp-content/uploads/2015/10/photo-1.jpg),
and sales floor. **Meriden is a different building and the photographs are from
2015.** Neither diagram supplies Boswell's walls, department positions,
escalator geometry, dimensions, or a 1990s material palette. The diagrams are
low-resolution photos without a readable scale. Chris's memory in
`docs/THE-MALL-IS-STILL-TRADING.md` section 13 remains the department authority
for our 1990s room. No image from the blog is copied into this repository.

What the visit *does* show, and what the fixture pass should carry over as a
retail design pattern:

| Visible evidence | Boswell's dressing consequence |
| --- | --- |
| The two floor diagrams divide sales into named departments around shared circulation and a central vertical connection. | Preserve a recognizable main route and a visible route between floors. Department boundaries should read through fixtures, signs, and floor treatment, not a new wall around each category. |
| Clothing and children's photos show tightly packed hanging stock, wall displays, end caps, small sale signs, and mannequins. | Dress clothing sockets as layered racks and wall standards with varied heights. A single repeated cuboid per socket would still look like a warehouse. |
| Home and toy photos show packed gondolas and display islands; the bedding photograph includes a made bed as a complete vignette. | Give toys, bedding, and home distinct silhouettes and materials. A bed needs bedding, curtains, nearby stock, and a price placard rather than only a mattress mesh. |
| The furniture photograph shows sofas in open rows, with long views across the floor and department banners on columns. | Keep furniture more open than clothing. Use low seating masses and columns as wayfinding; do not fill its entire wing with tall gondolas. |
| Photos show a pale drop ceiling with a regular field of recessed panels, square structural columns, polished tile in some aisles, and softer floor treatment in others. | Build a consistent ceiling and column rhythm before local clutter. Floor changes should help identify departments, while emergency lighting still follows `FIXTURE-LIGHT-CONTRACT.md`. The bright trading-store exposure in these photos is not the game's light level. |
| The author reports navigating an aisle with a stroller. | Check our intended customer routes with keeper and Sweeper collision in Unreal. A stroller anecdote does not give a usable aisle width, so retain the project's measured clearances. |

### Children's department photo pass, 2026-11-14

Chris's two additional reference photos show short white rolling apparel
racks, a carpeted sales area, a tile customer route at its edge, a pillar,
and a sales desk. They also show how a low grid ceiling makes stock and
department boundaries readable at human scale. These are composition and
fixture references, not textures or a measured plan. The bright trading
store lighting in the photographs is not the game's emergency-light state.

`Content/LLK_Boswells_ApparelPass.umap` now has an upper west children's
slice: 23 short rolling racks, a desk, two non-colliding sage carpet islands,
four dark edge strips, a pillar, and a lower ceiling with 100 cm tile seams.
The two 300 cm cross aisles and the outer/atrium bypasses remain clear in
the saved actor bounds. The ceiling uses six local visual fill lights; the
broader map has 31 new downcast emergency route lights. The current camera
view is still a blockout with flat materials, dark surrounding departments,
and no authored retail sign lettering. Visual completion and a played route
remain open. The separate atrium canopy is smoked glass beneath a solid roof;
the photographed daylight skylight is still an environmental-lighting task.

The next fixture pass should choose one arrival-to-escalator sightline and one
upper-floor furniture sightline, dress those with these distinct fixture types,
and test navigation. This reference does not justify copying the Meriden plan
onto the Anchor plate.

**Separate route map:** `Content/LLK_Boswells_RoutePass.umap` is a copy of
the Anchor with a split mall frontage and closed grille, an open dock mouth
with a guarded pad, a two-door service threshold, and paired sloped routes
through the existing atrium void. `Tools/build_boswells_route_pass.py`
creates it through the headless Unreal editor and refuses to overwrite it.
The original Anchor map is untouched. Downward collision traces on the new
map hit the ascent at z 55, 344 and 633 cm, then the upper landing at 662.5
cm. This proves collision surfaces and the direction of the slopes, not
keeper movement or navmesh connectivity. The commandlet's nav query returned
an empty path because navigation was not built in that process. The route
still needs a PIE walk, capsule clearance, and a navigation build. Run
`Tools/run_editor_python.ps1 -Script Tools/verify_boswells_route_pass.py`
to repeat the 13 saved-collision checks.

**Still unbuilt:** most fixture sockets beyond the lower apparel pass and
upper children's slice. The elevator, real escalator art, back-of-house
rooms, mall concourse, child-height sightlines, finished lighting, and a
complete played route remain. The grid check tests horizontal plan
connectivity only.

## First-run room: walked 28 × 18 m test space

Fixtures for the room that already plays. Not a level, and not shipped art.
`LLK_Boswells_FirstRun` is still the night. This file does not replace it,
and it does not open the full anchor. The full plate is about nine times
this room. Section 13 of `docs/THE-MALL-IS-STILL-TRADING.md` is the
department list. These boxes are the fixture types, set into the measured
28 m by 18 m interior.

The room is the one in `docs/FINDINGS-BOSWELLS-GEOMETRY.md`: interior
**2800 × 1800**, walls at x −1406/+1394 and y −906/+894, floor at z 0.
Nothing here resizes that room. The Sweeper that has to fit is the capsule
from the same finding, radius 34, **68 cm wide** and 176 tall. The player
capsule is not in this check. `ALLKCharacter` does not set one.

`BoswellsSalesFloor_Greybox.obj` is centimeters, Unreal axes, Z up. Import
at scale 1, when the level is open. It has no walls, no roof, and no lights.
The map already has those. Turn collision off on the import until a patrol
trace in the editor says the Sweeper still fits. Do not stack this mesh on
the gondolas already in the night. Those are a different reading of the
same floor, and two readings at once is not a store.

`BoswellsSalesFloor_Plan.svg` is the drawing, north up. The script is the
check. It refuses a solid in the patrol spine, a solid in the LoadingDock
trigger, and a slot against a wall that is wider than nothing and narrower
than the Sweeper. Then it walks a 68 cm disc from the customer door to every
aisle, to the three patrol points (y −200, +250, +650 on x 0), and to the
dock mouth.

Regenerate with `py docs/reference/boswells/make_salesfloor.py`.

## What the walk found

| Way | Clear width | What it is for |
| --- | --- | --- |
| West aisles, four of them | **209 cm** | Wall, then a clothing island, then the next. Over the 200 cm escape gap. |
| East aisles, four of them | **206 cm** | The room is 12 cm longer on the west, so the east aisles are the remainder. Still over 200. |
| Patrol spine | **360 cm** (\|x\| < 180) | Empty from the south patrol point through the dock approach. |
| Cross at y = 0 | **200 cm** | The gondola runs break here. A side aisle is 150 cm from the middle of a run. |
| Checkout to the gondolas | **168 cm** | Two 68 cm bodies pass. This is the measured side-aisle standard, not a second escape gap. |
| Customer door | **180 cm** | Posts on the south wall. The opening is empty. Header underside at z 210, over the 176 cm body. |
| EAS pair | **160 cm** between the inner faces | One Sweeper at a time. The chirp gate, not a second corridor. |
| Dock mouth | empty | Header only, z 260–300. The trigger (x −252…252, y 612…948, up to z 212) has no solid in it. |

West wall run is jewelry and perfume on one counter, 60 deep, sitting on the
wall. East wall run is shoes, 45 high, also on the wall. 45 is under the
hop, so those tables are not a vault and the path is in front of them.
Clothing islands are 110 wide and **137** high. At 137 an eye at 152 cm
sees over the run, and the lantern at rest (133 cm) does not
(`docs/FINDINGS-LANTERN-ANCHOR-ORIGIN.md`). Which island is juniors, mens,
or ladies is not set. Section 13 put all three on the lower floor and did
not give them walls.

Checkouts are 90 high. That is over the ~56 cm hop, so the counter is a
ledge the vault could use later. The vault is not opened here.

Round racks stand on the outer islands, in the end bays, 90 across and 160
high. They are boxes in this file. They are not round meshes, and a four-way
is not in this pass.

## What repeats

Five meshes, repeated, are the sales floor: a gondola, a glass case, a
checkout counter, a door, an alarm pedestal. Racks and shoe tables are the
same idea. `docs/FIELD-KIT.md` already calls for a round rack, a four-way,
a glass case, a gondola, and a wall standard. `docs/ASSETS.md` says we make
those, and a bought pack enters through `docs/ASSET-INTAKE.md` rather than
by being dropped on this map. The warehouse kit stays the stockroom and the
dock.

Every box is `BoswellsSalesFloor_Fixtures.csv`. Names are research labels.
They are not signs.

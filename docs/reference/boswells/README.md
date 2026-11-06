# Boswell's sales floor, walked

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

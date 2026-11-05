# Galleria shell greybox

Reference for the block-in. Not a level, and not shipped art. The lease
plan, the Verge3D scene, and the York directory stay out of the repo. Real
names on these objects locate the real building. They do not become signs.

`GalleriaShell_Greybox.obj` is boxes we rebuilt from the live directory
model's coverage. Units are centimeters, Unreal axes (X/Y plan, Z up).
`REF_100FT` is a bar that must measure 3,048 cm after import. If it does
not, the import scale is wrong and the rest of the file is wrong with it.

The size is one diagram unit = 708.91 cm, from bay 172's mesh area against
its 2016 area of 8,601 sq ft, checked by bay 178 still holding the same
ratio. Sheet TLP-1 wins wherever a wall disagrees with that scale. The
upper plate sits at 482 cm. That is the diagram's own gap run through the
same scale, about 15.8 ft. It is the floor-to-floor stand-in. It is not
the arch.

What is in the file: `WALK_L1`, `WALK_L2` with the three openings cut,
three anchor boxes, the exit-light markers, the stair and escalator
markers, and `EXTRACT_LOADINGDOCK_CANDIDATE` on the outer face of the
Boscov's box. The fourth built anchor is not here. It is still on TLP-1.
The fifth pad is not here. It is still unmeasured grass.

`GalleriaShell_Markers.csv` is the same points as a list.

Import the OBJ at scale 1 into a centimeter project. **File > Import Into
Level** wants a glTF, GLB, or FBX, so this OBJ comes in as a mesh. Do not
point that command at the leasing model's glTF. That file has no unit, and
glTF's default is metres, which would land a forty-unit diagram as a
forty-metre building with every tenant name in the outliner.

The level this belongs in is `LLK_Galleria_Shell`, and it does not exist
yet. `LLK_Boswells_FirstRun` is not the file. The cook's order is section
11 of `docs/THE-MALL-IS-STILL-TRADING.md`. The seats are in
`docs/HANDOFFS.md`.

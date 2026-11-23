"""Ring the escalator well with counter islands.

Runs headlessly with `Tools/run_editor_python.ps1`.

Chris, 2026-11-23: *"What are we doing with that giant empty space in front
of the escalators?"*

Three different spaces were hiding behind that one question, and only one of
them is this tool's to fill.

1. **The escalator court**, x ±1000, y ±1000. A probe of the map found 81
   actors in it and every one is the escalator machine itself - treads,
   combs, the route plates - or the floor slab. The well has a 20 x 20 m
   room around it and nothing in that room. **That is what this fills.**
2. **The main aisle in front of it**, x ±900 running south to the mall
   entrance. Not empty: 58 actors, and they are the opening - the drag
   marks, the witness pool, `BSW_OPENING_GRILLE_CONTROLLER`, the release
   hardware. That staging is on hold until Grok's opening is accepted and
   Chris picks one, so nothing goes there.
3. **Behind the court.** Emptied deliberately on the same day, by removing
   two invented cells from the plan; see the note in
   `docs/reference/boswells/make_groundfloor_plan.py`. The back-of-house
   aisle runs the full width there now.

## What goes round a well

Counter islands, which is what the Meriden sheet shows and what every anchor
store does with the floor beside its escalators: small high-margin goods
where the whole store walks past. The plan has carried four of them in
`COURT_ISLANDS` since it was written - jewelry, watches, accessories - and
nothing had ever built them.

They sit in the 400 cm strip between the atrium void at x ±600 and the court
edge at x ±1000, which is floor, with the void beside them. So the player
walking the ring passes a lit case on one side and a drop to nothing on the
other.

Fixtures come from the kit that already exists -
`SM_BSW_CaseGoods_Jewelry_A/B` and `SM_BSW_CaseGoods_Perfume_A/B` - rather
than from new art, and the tool refuses to place one that overhangs its
island instead of letting it poke into the walk.

Idempotent: everything it creates is `BSW_COURT_ISLAND_*` and those are
removed first.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_COURT_ISLAND_"
KIT = "/Game/LLK/Boswells/Fixtures/"

ATRIUM = (-600.0, -900.0, 600.0, 900.0)
COURT = (-1000.0, -1000.0, 1000.0, 1000.0)
GAP = 30.0          # between two units on one island

# name, mesh, x0, y0, x1, y1 - from COURT_ISLANDS in the plan generator.
ISLANDS = [
    ("JEWELRY_SW",     "SM_BSW_CaseGoods_Jewelry_A",  -900.0, -820.0, -640.0, -560.0),
    ("ACCESSORIES_SE", "SM_BSW_CaseGoods_Perfume_A",   640.0, -820.0,  900.0, -560.0),
    ("WATCHES_NW",     "SM_BSW_CaseGoods_Jewelry_B",  -900.0,  560.0, -640.0,  820.0),
    ("ACCESSORIES_NE", "SM_BSW_CaseGoods_Perfume_B",   640.0,  560.0,  900.0,  820.0),
]


def require(condition, message):
    if not condition:
        raise RuntimeError("COURT " + message)


# Every island has to be on floor, not over the hole, and inside the court.
for name, _mesh, x0, y0, x1, y1 in ISLANDS:
    require(x1 <= ATRIUM[0] or ATRIUM[2] <= x0
            or y1 <= ATRIUM[1] or ATRIUM[3] <= y0,
            "%s overhangs the atrium void; it would stand on nothing" % name)
    require(COURT[0] <= x0 and x1 <= COURT[2]
            and COURT[1] <= y0 and y1 <= COURT[3],
            "%s is outside the escalator court" % name)

world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "map unavailable")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

removed = 0
for actor in subsystem.get_all_level_actors():
    if actor.get_actor_label().startswith(PREFIX):
        subsystem.destroy_actor(actor)
        removed += 1
print("COURT removed=%d" % removed)

placed = 0
for name, mesh_name, x0, y0, x1, y1 in ISLANDS:
    mesh = unreal.EditorAssetLibrary.load_asset(KIT + mesh_name)
    require(isinstance(mesh, unreal.StaticMesh),
            "fixture %s missing from the kit" % mesh_name)
    extent = mesh.get_bounds().box_extent
    unit_x, unit_y = 2 * extent.x, 2 * extent.y
    width, length = x1 - x0, y1 - y0
    require(unit_x <= width + 0.1 and unit_y <= length + 0.1,
            "%s is %.0f x %.0f cm and island %s is %.0f x %.0f; it would "
            "overhang into the walk round the well"
            % (mesh_name, unit_x, unit_y, name, width, length))

    # As many units as fit along the island's longer axis, centred, so an
    # island reads as a counter run rather than as one lonely box.
    along_y = length >= width
    span = length if along_y else width
    unit = unit_y if along_y else unit_x
    count = max(1, int((span + GAP) // (unit + GAP)))
    used = count * unit + (count - 1) * GAP
    start = -used * 0.5 + unit * 0.5

    cx, cy = (x0 + x1) * 0.5, (y0 + y1) * 0.5
    for index in range(count):
        offset = start + index * (unit + GAP)
        x = cx if along_y else cx + offset
        y = cy + offset if along_y else cy
        actor = subsystem.spawn_actor_from_class(
            unreal.StaticMeshActor, unreal.Vector(x, y, 0.0))
        require(actor is not None, "spawn failed for %s" % name)
        actor.set_actor_label("%s%s_%d" % (PREFIX, name, index))
        actor.set_folder_path("Boswells/LowerSales/Court")
        comp = actor.static_mesh_component
        comp.set_mobility(unreal.ComponentMobility.MOVABLE)
        require(comp.set_static_mesh(mesh), "mesh failed for %s" % name)
        placed += 1
    print("COURT island %-16s %s x%d unit=%.0fx%.0f cm"
          % (name, mesh_name, count, unit_x, unit_y))

print("COURT placed=%d round the well" % placed)
require(placed >= len(ISLANDS), "fewer units than islands")

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP),
        "map save failed")
print("COURT saved=True")

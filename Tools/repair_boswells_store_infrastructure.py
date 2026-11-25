"""Add the missing physical systems that make Boswell's read as a department store.

This is a one-shot additive pass on the active opening map.  Existing opening,
lantern, Sweeper, escalator collision, and department actors are untouched.
"""

import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_REALITY_"


def require(condition, message):
    if not condition:
        raise RuntimeError("BOSWELLS_REALITY " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "opening map unavailable")
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {actor.get_actor_label(): actor for actor in sub.get_all_level_actors()}
require(not any(label.startswith(PREFIX) for label in before),
        "infrastructure pass is already present")
for label in ("ANCHOR_FULL_ROOF", "ANCHOR_LOADING_DOCK_EXIT",
              "ROUTE_ESCALATOR_ASCEND", "ROUTE_ESCALATOR_DESCEND",
              "BSW_OPENING_LANTERN", "BSW_OPENING_GRILLE_CONTROLLER"):
    require(label in before, "required opening actor absent: " + label)

cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
require(isinstance(cube, unreal.StaticMesh), "engine cube missing")
paths = {
    "tile": "/Game/LLK/Boswells/Architecture/M_BSW_StoneTile",
    "carpet": "/Game/LLK/Boswells/Architecture/M_BSW_KidsSageCarpet",
    "steel": "/Game/Props/Boswells/Materials/steel",
    "wood": "/Game/Props/Boswells/Materials/dark_wood",
    "cream": "/Game/Props/Boswells/Materials/cream",
    "dark": "/Game/LLK/Boswells/Architecture/M_BSW_DarkChrome",
    "teal": "/Game/LLK/Boswells/Architecture/M_BSW_MallMullionTeal",
}
mats = {name: unreal.EditorAssetLibrary.load_asset(path) for name, path in paths.items()}
require(all(isinstance(mat, unreal.MaterialInterface) for mat in mats.values()),
        "store material missing")


def box(name, x0, x1, y0, y1, z0, z1, mat, solid=True, folder="Infrastructure"):
    require(x1 > x0 and y1 > y0 and z1 > z0, "bad dimensions " + name)
    actor = sub.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2),
        unreal.Rotator())
    require(actor is not None, "spawn failed " + name)
    actor.set_actor_label(PREFIX + name)
    actor.set_folder_path("Boswells/Reality/" + folder)
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(cube), "mesh failed " + name)
    actor.set_actor_scale3d(unreal.Vector((x1 - x0) / 100,
                                          (y1 - y0) / 100,
                                          (z1 - z0) / 100))
    comp.set_material(0, mats[mat])
    actor.set_actor_enable_collision(solid)
    comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS
                               if solid else unreal.CollisionEnabled.NO_COLLISION)
    comp.set_editor_property("can_ever_affect_navigation", solid)
    return actor


# Carpeting identifies departments without obstructing the existing plan.  It
# sits 1 cm above the tile and has no collision; existing fixtures remain the
# physical cover and aisle boundaries.
for index, (x0, x1, y0, y1) in enumerate((
        (-4100, -850, -2100, -850), (-4100, -850, 900, 1950),
        (900, 4100, -2100, -850), (900, 4100, 900, 1950),
        (-4100, -850, -2100, -850), (-4100, -850, 900, 1950))):
    z = 1 if index < 4 else 663.5
    box("CARPET_ZONE_%02d" % index, x0, x1, y0, y1, z, z + 1.5,
        "carpet", False, "Flooring")

# Department stores have repeated structural rhythm.  These columns occupy
# only the exterior margins, leaving all 300 cm planned public routes clear.
pillar_points = []
for y in (-2050, -1050, 0, 1050, 2050):
    pillar_points.extend([(-4200, y), (4200, y)])
for index, (x, y) in enumerate(pillar_points):
    box("PILLAR_%02d" % index, x - 42, x + 42, y - 42, y + 42,
        0, 1488, "cream", True, "Structure")

# The upper services edge is now a real third, office-only level: it is not a
# third mall floor.  The shaft offers three plainly modeled stops.
box("OFFICE_LEVEL", -4200, -1500, 1850, 2600, 1120, 1142,
    "carpet", True, "Office")
box("ELEVATOR_SHAFT", -1200, -500, 1860, 2560, 0, 1488,
    "teal", True, "Vertical")
for label, z in (("LOWER", 24), ("UPPER", 686), ("OFFICE", 1144)):
    box("ELEVATOR_DOOR_" + label, -520, -480, 2040, 2380, z, z + 250,
        "dark", False, "Vertical")

# Fitting rooms belong beside upper apparel, not as loose cubes on the sales
# floor: a carpeted lobby, counter, four cubicles, mirrors, and door gaps.
base_x, base_y, floor = 2750, 1880, 664
box("FITTING_LOBBY_FLOOR", base_x, 4075, base_y, 2550, floor, floor + 2,
    "carpet", False, "FittingRooms")
box("FITTING_COUNTER", base_x + 80, base_x + 380, base_y + 80, base_y + 310,
    floor, floor + 105, "wood", True, "FittingRooms")
for cubicle in range(4):
    x0 = base_x + 500 + cubicle * 390
    x1 = x0 + 330
    box("FITTING_CUBICLE_%d_BACK" % cubicle, x0, x1, base_y + 550, base_y + 575,
        floor, floor + 240, "cream", True, "FittingRooms")
    box("FITTING_CUBICLE_%d_W" % cubicle, x0, x0 + 24, base_y + 250, base_y + 575,
        floor, floor + 240, "cream", True, "FittingRooms")
    box("FITTING_CUBICLE_%d_E" % cubicle, x1 - 24, x1, base_y + 250, base_y + 575,
        floor, floor + 240, "cream", True, "FittingRooms")
    # Dark mirror plane faces the changing bay; it does not add collision.
    box("FITTING_CUBICLE_%d_MIRROR" % cubicle, x0 + 42, x1 - 42,
        base_y + 548, base_y + 552, floor + 45, floor + 205,
        "dark", False, "FittingRooms")

# Replace the token receiving pad with a usable back-of-house volume beyond
# the existing dock threshold.  The public route remains bounded and the dock
# gets a roof, bumpers, a roll-up opening, pallet lanes and a receiving aisle.
box("DOCK_FLOOR", -1700, 1700, 2680, 4550, 0, 22, "steel", True, "Dock")
box("DOCK_ROOF", -1700, 1700, 2680, 4550, 1080, 1102, "dark", True, "Dock")
box("DOCK_WALL_W", -1700, -1665, 2680, 4550, 0, 1100, "dark", True, "Dock")
box("DOCK_WALL_E", 1665, 1700, 2680, 4550, 0, 1100, "dark", True, "Dock")
box("DOCK_BACK_W", -1700, -520, 4515, 4550, 0, 1100, "dark", True, "Dock")
box("DOCK_BACK_E", 520, 1700, 4515, 4550, 0, 1100, "dark", True, "Dock")
box("DOCK_ROLLUP_HEADER", -520, 520, 4515, 4550, 400, 1100, "teal", True, "Dock")
for index, x in enumerate((-1450, -1080, 1080, 1450)):
    box("DOCK_BUMPER_%d" % index, x - 55, x + 55, 4440, 4515, 0, 105,
        "steel", True, "Dock")
for index, (x, y) in enumerate(((-1100, 3180), (-1100, 3900), (1100, 3180),
                                 (1100, 3900), (-700, 4200), (700, 4200))):
    box("DOCK_PALLET_%02d" % index, x - 120, x + 120, y - 95, y + 95,
        22, 152, "wood", True, "Dock")
    box("DOCK_CARTON_%02d" % index, x - 92, x + 92, y - 70, y + 70,
        152, 270, "cream", False, "Dock")

# Correct the *appearance* at the escalator mouths without touching the
# verified collision ramps: tiled threshold bands, comb plates, and side
# stringers clarify which floor belongs to which landing.
for name, x, bottom_y, top_y in (("ASCEND", -250, -760, 760),
                                  ("DESCEND", 250, 760, -760)):
    for side, y in (("LOW", bottom_y), ("HIGH", top_y)):
        z = 2 if side == "LOW" else 664
        box("ESC_%s_%s_LANDING_TILE" % (name, side), x - 145, x + 145,
            y - 130, y + 130, z, z + 1.5, "tile", False, "Escalators")
        box("ESC_%s_%s_COMB" % (name, side), x - 100, x + 100,
            y - 18, y + 18, z + 2, z + 5, "steel", False, "Escalators")
    for side in (-1, 1):
        box("ESC_%s_STRINGER_%s" % (name, side), x + side * 104 - 7,
            x + side * 104 + 7, -650, 650, 20, 680, "dark", False,
            "Escalators")

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
after = {actor.get_actor_label(): actor for actor in sub.get_all_level_actors()}
new = [label for label in after if label.startswith(PREFIX)]
require(len(new) >= 70, "infrastructure density unexpectedly low")
print("BOSWELLS_REALITY SAVED: %d new architecture actors; fitting rooms, "
      "elevator/office stops, pillars, carpets, full loading dock, and escalator "
      "landing detail added without replacing opening route actors" % len(new))

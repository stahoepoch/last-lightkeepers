"""Turn the upper rear void into a plausible, reachable Boswell's service core.

This is not a secret third sales floor or an unplayable black volume.  It is
the public upper-floor edge: courtesy/gift wrap, optical, hearing, restrooms,
and the historic candy counter.  Staff rooms are visibly bounded behind each
counter; the public front aisle remains continuous and the fitting-room lobby
stays reachable at the east end.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_SERVICE_CORE_"
CUBE = "/Engine/BasicShapes/Cube.Cube"


def require(condition, message):
    if not condition:
        raise RuntimeError("BOSWELLS_SERVICE_CORE " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {actor.get_actor_label(): actor for actor in sub.get_all_level_actors()}
require(not any(name.startswith(PREFIX) for name in before), "pass already exists")
for label in ("BSW_REALITY_FITTING_LOBBY_FLOOR", "BSW_REALITY_ELEVATOR_DOOR_UPPER",
              "ROUTE_ESCALATOR_ASCEND", "ROUTE_ESCALATOR_DESCEND"):
    require(label in before, "required upper-floor architecture missing: " + label)

mesh = unreal.EditorAssetLibrary.load_asset(CUBE)
materials = {
    "tile": unreal.EditorAssetLibrary.load_asset("/Game/LLK/Boswells/Architecture/M_BSW_StoneTile"),
    "carpet": unreal.EditorAssetLibrary.load_asset("/Game/LLK/Boswells/Architecture/M_BSW_KidsSageCarpet"),
    "cream": unreal.EditorAssetLibrary.load_asset("/Game/Props/Boswells/Materials/cream"),
    "dark": unreal.EditorAssetLibrary.load_asset("/Game/LLK/Boswells/Architecture/M_BSW_DarkChrome"),
    "wood": unreal.EditorAssetLibrary.load_asset("/Game/Props/Boswells/Materials/dark_wood"),
    "steel": unreal.EditorAssetLibrary.load_asset("/Game/Props/Boswells/Materials/steel"),
    "teal": unreal.EditorAssetLibrary.load_asset("/Game/LLK/Boswells/Architecture/M_BSW_MallMullionTeal"),
}
require(isinstance(mesh, unreal.StaticMesh), "cube missing")
require(all(isinstance(value, unreal.MaterialInterface) for value in materials.values()),
        "material missing")


def box(name, x0, x1, y0, y1, z0, z1, material, solid=True, folder="Core"):
    require(x1 > x0 and y1 > y0 and z1 > z0, "invalid box " + name)
    actor = sub.spawn_actor_from_class(unreal.StaticMeshActor,
        unreal.Vector((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2), unreal.Rotator())
    require(actor is not None, "spawn failed " + name)
    actor.set_actor_label(PREFIX + name)
    actor.set_folder_path("Boswells/UpperServiceCore/" + folder)
    component = actor.static_mesh_component
    require(component.set_static_mesh(mesh), "mesh failed " + name)
    actor.set_actor_scale3d(unreal.Vector((x1 - x0) / 100, (y1 - y0) / 100, (z1 - z0) / 100))
    component.set_material(0, materials[material])
    actor.set_actor_enable_collision(solid)
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS
                                    if solid else unreal.CollisionEnabled.NO_COLLISION)
    component.set_editor_property("can_ever_affect_navigation", solid)
    return actor


# The back-of-house is no longer open darkness.  It has a real, deliberately
# narrow staff band bounded by a continuous rear wall; players read its doors
# and counters from the public hall rather than assuming a missing map exists.
FLOOR_Z = 664
CEILING_Z = 1025
box("PUBLIC_SERVICE_FLOOR", -4100, 2500, 1540, 2580, FLOOR_Z, FLOOR_Z + 3,
    "tile", False, "Architecture")
box("REAR_SERVICE_WALL", -4100, 2500, 2550, 2580, FLOOR_Z, CEILING_Z,
    "cream", True, "Architecture")
box("SERVICE_CEILING", -4100, 2500, 1540, 2580, CEILING_Z, CEILING_Z + 18,
    "dark", True, "Architecture")

# Four deep, named service bays face the public aisle.  They are finished
# rooms, not holes: counter, staffed backdrop, side partitions, stock shelves,
# and a deliberately visible staff-only door.  No partition reaches the public
# aisle, so the entire front remains navigable.
bays = (
    ("COURTESY", -4000, -2750, "wood"),
    ("OPTICAL", -2650, -1400, "teal"),
    ("HEARING", -1300, -50, "steel"),
    ("CANDY_GIFT", 50, 1300, "wood"),
)
for index, (name, x0, x1, accent) in enumerate(bays):
    # Counter faces south into the sales floor; free aisle runs from y=1540..2045.
    box(name + "_COUNTER", x0 + 70, x1 - 70, 2045, 2165, FLOOR_Z, FLOOR_Z + 112,
        "wood", True, "Counters")
    box(name + "_FASCIA", x0 + 70, x1 - 70, 2035, 2050, FLOOR_Z + 112, FLOOR_Z + 205,
        accent, False, "Counters")
    box(name + "_BACKDROP", x0 + 60, x1 - 60, 2420, 2450, FLOOR_Z, FLOOR_Z + 280,
        "cream", True, "Rooms")
    box(name + "_PARTITION_W", x0, x0 + 28, 2165, 2550, FLOOR_Z, FLOOR_Z + 280,
        "dark", True, "Rooms")
    box(name + "_PARTITION_E", x1 - 28, x1, 2165, 2550, FLOOR_Z, FLOOR_Z + 280,
        "dark", True, "Rooms")
    # Staff door inset on the rear, marked by a dark slab and an emergency-lit header.
    door_x = x0 + 130 if index % 2 == 0 else x1 - 310
    box(name + "_STAFF_DOOR", door_x, door_x + 180, 2415, 2445,
        FLOOR_Z, FLOOR_Z + 230, "dark", False, "Rooms")
    box(name + "_STAFF_HEADER", door_x - 15, door_x + 195, 2398, 2420,
        FLOOR_Z + 230, FLOOR_Z + 255, accent, False, "Rooms")
    for shelf in range(3):
        box(name + "_STOCK_%d" % shelf, x0 + 130 + shelf * 320,
            x0 + 330 + shelf * 320, 2240, 2310,
            FLOOR_Z + 35, FLOOR_Z + 145, accent if shelf == 1 else "cream",
            False, "Stock")

# The retail facts we can support: an upper-floor candy counter plus current
# optical/hearing offerings, courtesy desk, gift wrap, and customer restrooms.
# Restrooms are a public side room with an unmistakable entry recess, not a
# purported gameplay basement or unverified department.
box("RESTROOM_WALL_W", 1420, 1450, 1750, 2550, FLOOR_Z, CEILING_Z, "cream", True, "Restrooms")
box("RESTROOM_WALL_E", 2470, 2500, 1750, 2550, FLOOR_Z, CEILING_Z, "cream", True, "Restrooms")
box("RESTROOM_BACK", 1450, 2470, 2520, 2550, FLOOR_Z, CEILING_Z, "cream", True, "Restrooms")
box("RESTROOM_DIVIDER", 1945, 1975, 2020, 2520, FLOOR_Z, FLOOR_Z + 230, "dark", True, "Restrooms")
box("RESTROOM_SIGN_BAND", 1450, 2470, 1760, 1790, FLOOR_Z + 220, FLOOR_Z + 260,
    "teal", False, "Restrooms")

# Finish the public edge with a return desk and a seated waiting pocket.  Both
# create department-store density while retaining the path to fitting rooms.
box("RETURNS_DESK", 1400, 2200, 1910, 2015, FLOOR_Z, FLOOR_Z + 110, "wood", True, "Public")
for index, x in enumerate((1525, 1740, 2150, 2360)):
    box("WAIT_SEAT_%02d" % index, x, x + 115, 1600, 1715, FLOOR_Z, FLOOR_Z + 48,
        "steel", False, "Public")
    box("WAIT_BACK_%02d" % index, x, x + 115, 1695, 1715, FLOOR_Z + 48, FLOOR_Z + 112,
        "teal", False, "Public")

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "save failed")
new_labels = [actor.get_actor_label() for actor in sub.get_all_level_actors()
              if actor.get_actor_label().startswith(PREFIX)]
require(len(new_labels) >= 55, "service core too sparse: {}".format(len(new_labels)))
print("BOSWELLS_SERVICE_CORE SAVED: {} actors; finished public service hall, bounded staff rooms, restrooms, and an upper-floor candy/service edge".format(len(new_labels)))

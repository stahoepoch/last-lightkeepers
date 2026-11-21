"""Wire the opening store's real circulation without sacrificing navigation.

This pass makes the existing two retail floors usable as a department store:
paired elevator calls, an open service/fitting/restroom edge, and dense,
occluding merchandise layers that keep the authored escape spine readable.
Run only through the documented save -> build-navigation -> strict-gate flow.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_CIRCULATION_"


def require(ok, message):
    if not ok:
        raise RuntimeError("BOSWELLS_CIRCULATION " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = {actor.get_actor_label(): actor for actor in sub.get_all_level_actors()}
require(not any(name.startswith(PREFIX) for name in actors), "pass already exists")
for name in ("BSW_REALITY_ELEVATOR_SHAFT", "BSW_REALITY_FITTING_LOBBY_FLOOR",
             "BSW_SERVICE_CORE_RESTROOM_SIGN_BAND", "ROUTE_ESCALATOR_ASCEND",
             "ROUTE_ESCALATOR_DESCEND"):
    require(name in actors, "missing required actor " + name)

cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
rack = unreal.EditorAssetLibrary.load_asset("/Game/LLK/Boswells/Fixtures/SM_BSW_ClothingRun")
mats = {
    "cream": unreal.EditorAssetLibrary.load_asset("/Game/Props/Boswells/Materials/cream"),
    "dark": unreal.EditorAssetLibrary.load_asset("/Game/LLK/Boswells/Architecture/M_BSW_DarkChrome"),
    "wood": unreal.EditorAssetLibrary.load_asset("/Game/Props/Boswells/Materials/dark_wood"),
    "teal": unreal.EditorAssetLibrary.load_asset("/Game/LLK/Boswells/Architecture/M_BSW_MallMullionTeal"),
    "oat": unreal.EditorAssetLibrary.load_asset("/Game/Props/Boswells/Materials/oat"),
    "steel": unreal.EditorAssetLibrary.load_asset("/Game/Props/Boswells/Materials/steel"),
}
require(isinstance(cube, unreal.StaticMesh) and isinstance(rack, unreal.StaticMesh), "fixture assets missing")
require(all(isinstance(value, unreal.MaterialInterface) for value in mats.values()), "material missing")


def box(name, x0, x1, y0, y1, z0, z1, mat, solid, folder):
    actor = sub.spawn_actor_from_class(unreal.StaticMeshActor,
        unreal.Vector((x0+x1)/2, (y0+y1)/2, (z0+z1)/2), unreal.Rotator())
    require(actor is not None, "spawn failed " + name)
    actor.set_actor_label(PREFIX + name)
    actor.set_folder_path("Boswells/Circulation/" + folder)
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(cube), "cube mesh failed " + name)
    actor.set_actor_scale3d(unreal.Vector((x1-x0)/100, (y1-y0)/100, (z1-z0)/100))
    comp.set_material(0, mats[mat])
    actor.set_actor_enable_collision(solid)
    comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS if solid else unreal.CollisionEnabled.NO_COLLISION)
    comp.set_editor_property("can_ever_affect_navigation", solid)
    return actor


def hanging_run(name, x, y, z, yaw):
    actor = sub.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator(0, yaw, 0))
    require(actor is not None, "rack spawn failed " + name)
    actor.set_actor_label(PREFIX + name)
    actor.set_folder_path("Boswells/Circulation/Stock")
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(rack), "rack mesh failed " + name)
    # This is dense hanging stock, not a new collision claim. The original
    # socket bodies carry collision and nav; this layer blocks sightlines.
    actor.set_actor_enable_collision(False)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    comp.set_editor_property("can_ever_affect_navigation", False)
    return actor


# The old shaft was a single opaque collision box. Hide it, then make its two
# public doorways/call pads actual destinations on the public side of the core.
old_shaft = actors["BSW_REALITY_ELEVATOR_SHAFT"]
old_shaft.set_actor_hidden_in_game(True)
old_shaft.set_actor_enable_collision(False)
old_shaft.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
old_shaft.static_mesh_component.set_editor_property("can_ever_affect_navigation", False)

lower_stop = unreal.Vector(-850, 1750, 55)
upper_stop = unreal.Vector(-850, 1750, 717)
elevator_class = unreal.load_class(None, "/Script/LLK.LLKStoreElevator")
require(elevator_class is not None, "compiled LLKStoreElevator class unavailable")
for name, location in (("ELEVATOR_LOWER_CALL", lower_stop), ("ELEVATOR_UPPER_CALL", upper_stop)):
    lift = sub.spawn_actor_from_class(elevator_class, location, unreal.Rotator())
    require(lift is not None, "elevator spawn failed " + name)
    lift.set_actor_label(PREFIX + name)
    lift.set_folder_path("Boswells/Circulation/Elevator")
    lift.set_editor_property("lower_stop", lower_stop)
    lift.set_editor_property("upper_stop", upper_stop)
    lift.set_editor_property("travel_seconds", 2.5)

for level, z in (("LOWER", 0), ("UPPER", 662.5)):
    box("ELEVATOR_%s_FRAME_W" % level, -1220, -1170, 1670, 1870, z, z + 260, "dark", True, "Elevator")
    box("ELEVATOR_%s_FRAME_E" % level, -530, -480, 1670, 1870, z, z + 260, "dark", True, "Elevator")
    box("ELEVATOR_%s_HEADER" % level, -1170, -530, 1840, 1870, z + 225, z + 260, "teal", False, "Elevator")
    box("ELEVATOR_%s_THRESHOLD" % level, -1120, -580, 1670, 1760, z + 2, z + 5, "steel", False, "Elevator")

# Make the public side of restrooms and fitting rooms legible and passable.
# Split the old solid returns desk into two side desks, leaving a 180 cm entry.
old_returns = actors.get("BSW_SERVICE_CORE_RETURNS_DESK")
if old_returns:
    old_returns.set_actor_hidden_in_game(True)
    old_returns.set_actor_enable_collision(False)
    old_returns.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    old_returns.static_mesh_component.set_editor_property("can_ever_affect_navigation", False)
box("RESTROOM_DESK_W", 1400, 1810, 1910, 2015, 664, 774, "wood", True, "Services")
box("RESTROOM_DESK_E", 1990, 2200, 1910, 2015, 664, 774, "wood", True, "Services")
box("RESTROOM_ENTRY_W", 1420, 1450, 1740, 1910, 664, 900, "cream", True, "Services")
box("RESTROOM_ENTRY_E", 2470, 2500, 1740, 1910, 664, 900, "cream", True, "Services")
box("FITTING_QUEUE_RAIL", 2740, 2780, 1880, 2270, 664, 770, "steel", False, "Services")

# Dense selling stock should hide long views without becoming a maze. These
# non-colliding rack layers sit over the approved fixture sockets; the central
# 360 cm spine, two 200 cm cross-aisles, elevator approach, and escalators are
# deliberately omitted from this pattern.
stock_rows = []
for z, prefix, ys in ((5, "LOWER", (-1850, -1350, -850, 450, 950, 1450)),
                      (667, "UPPER", (-1850, -1350, -850, 450, 950, 1450))):
    for x in (-3850, -3350, -2850, -2350, -1850, -1350, 1350, 1850, 2350, 2850, 3350, 3850):
        for y in ys:
            # Cross-aisle mouths remain visually open at y +/- 250 and the
            # outside service edge remains free for the elevator and rooms.
            stock_rows.append(("%s_STOCK_%+05d_%+05d" % (prefix, x, y), x, y, z))
for name, x, y, z in stock_rows:
    hanging_run(name, x, y, z, 90 if abs(x) > 2500 else 0)

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
after = [actor for actor in sub.get_all_level_actors() if actor.get_actor_label().startswith(PREFIX)]
require(len(after) >= 145, "circulation/stock density too low: {}".format(len(after)))
print("BOSWELLS_CIRCULATION SAVED: {} actors; paired working elevator calls, restroom/fitting access, and dense sightline-blocking stock".format(len(after)))

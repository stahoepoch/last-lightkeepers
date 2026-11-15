"""Complete the saved mall-glass pass with a real landing and side glass.

Run once with Tools/run_editor_python.ps1 while Unreal Editor is closed. The
original builder now includes this geometry for a clean replay; this script
applies the correction to the already-saved ApparelPass map.
"""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"


def require(ok, message):
    if not ok:
        raise RuntimeError("BOSWELLS_ENTRANCE_SUPPORT " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_ApparelPass", "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = {a.get_actor_label(): a for a in subsystem.get_all_level_actors()}
for name in ("MALL_GLASS_ENTRANCE", "MALL_GLASS_RETURN_W",
             "MALL_GLASS_RETURN_E", "ROUTE_MALL_GRILLE_22",
             "ANCHOR_FULL_FLOOR_LOWER"):
    require(name in actors, "glass pass is missing: " + name)
require("MALL_ENTRANCE_APPROACH_FLOOR" not in actors,
        "landing already exists; refusing duplicate")


def bounds(actor):
    center, extent = actor.get_actor_bounds(False)
    return (center.x - extent.x, center.x + extent.x,
            center.y - extent.y, center.y + extent.y,
            center.z - extent.z, center.z + extent.z)


require(all(abs(a - b) < 1 for a, b in zip(
    bounds(actors["ANCHOR_FULL_FLOOR_LOWER"]),
    (-4350, 4350, -2700, 2700, -30, 0))),
    "store floor bounds changed")
require(all(abs(a - b) < 1 for a, b in zip(
    bounds(actors["MALL_GLASS_RETURN_W"]),
    (-900, -896, -2880, -2680, 280, 1468))),
    "west return changed; review before extending")
require(all(abs(a - b) < 1 for a, b in zip(
    bounds(actors["MALL_GLASS_RETURN_E"]),
    (896, 900, -2880, -2680, 280, 1468))),
    "east return changed; review before extending")

cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
require(isinstance(cube, unreal.StaticMesh), "engine cube missing")
floor_material = actors["ANCHOR_FULL_FLOOR_LOWER"].static_mesh_component.get_material(0)
floor = subsystem.spawn_actor_from_class(
    unreal.StaticMeshActor, unreal.Vector(0, -2890, -15),
    unreal.Rotator(0, 0, 0))
require(floor is not None, "floor spawn failed")
floor.set_actor_label("MALL_ENTRANCE_APPROACH_FLOOR")
floor.set_folder_path("Boswells/Architecture/MallGlassEntrance")
floor.set_actor_scale3d(unreal.Vector(18, 4.2, .3))
comp = floor.static_mesh_component
require(comp.set_static_mesh(cube), "floor cube assignment failed")
if floor_material:
    comp.set_material(0, floor_material)
floor.set_actor_enable_collision(True)
comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)

# The exterior bay's side glazing now reaches the new slab. The front glass
# still begins at 280 cm, leaving the human-height mall entry below it.
for name, x in (("MALL_GLASS_RETURN_W", -898),
                ("MALL_GLASS_RETURN_E", 898)):
    side = actors[name]
    side.set_actor_location(unreal.Vector(x, -2780, 734), False, False)
    side.set_actor_scale3d(unreal.Vector(.04, 2, 14.68))

require(all(abs(a - b) < 1 for a, b in zip(
    bounds(floor), (-900, 900, -3100, -2680, -30, 0))),
    "approach floor bounds differ")
for name in ("MALL_GLASS_RETURN_W", "MALL_GLASS_RETURN_E"):
    require(abs(bounds(actors[name])[4]) < 1,
            "side glass still floats: " + name)
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP),
        "map save failed")
message = "BOSWELLS_ENTRANCE_SUPPORT SAVED: 4.2 m approach, side glass grounded"
unreal.log(message)
print(message)

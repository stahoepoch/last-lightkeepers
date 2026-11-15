"""Dress one upper-floor children's department from the supplied photos.

The two carpet islands stop short of the reserved tiled cross aisles. Rolling
racks and the desk live inside those islands, leaving broad routes around
them. This script never moves the escalators, service route, or gameplay actors.
Run headlessly while Unreal Editor is closed.
"""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
FIXTURES = "/Game/LLK/Boswells/Fixtures/"
MATERIALS = "/Game/LLK/Boswells/Architecture"
PREFIX = "BSW_KIDS_"
CUBE = "/Engine/BasicShapes/Cube.Cube"
FLOOR_Z = 662.5


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_KIDS " + why)


def material(name, color, roughness):
    path = MATERIALS + "/" + name
    require(not unreal.EditorAssetLibrary.does_asset_exist(path),
            "material already exists; refusing overwrite: " + name)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, MATERIALS, unreal.Material, unreal.MaterialFactoryNew())
    require(isinstance(asset, unreal.Material), "material create failed: " + name)
    edit = unreal.MaterialEditingLibrary
    base = edit.create_material_expression(
        asset, unreal.MaterialExpressionConstant3Vector, -320, -70)
    base.set_editor_property("constant", unreal.LinearColor(*color, 1.0))
    edit.connect_material_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)
    rough = edit.create_material_expression(
        asset, unreal.MaterialExpressionConstant, -320, 80)
    rough.set_editor_property("r", roughness)
    edit.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    edit.recompile_material(asset)
    require(unreal.EditorAssetLibrary.save_loaded_asset(asset),
            "material save failed: " + name)
    return asset


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_ApparelPass", "wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {a.get_actor_label(): a for a in actors.get_all_level_actors()}
require(not any(label.startswith(PREFIX) for label in before),
        "department already placed; refusing duplicate")
require(all(label in before for label in (
    "ROUTE_ESCALATOR_ASCEND", "ROUTE_ESCALATOR_DESCEND",
    "ANCHOR_LOADING_DOCK_EXIT")), "vertical or dock route missing")
cube = unreal.EditorAssetLibrary.load_asset(CUBE)
rack = unreal.EditorAssetLibrary.load_asset(FIXTURES + "SM_BSW_KidsRollingRack")
desk = unreal.EditorAssetLibrary.load_asset(FIXTURES + "SM_BSW_KidsSalesDesk")
require(all(isinstance(m, unreal.StaticMesh) for m in (cube, rack, desk)),
        "children's fixture source missing")
require(rack.get_bounds().box_extent.x <= 80 and
        desk.get_bounds().box_extent.x <= 90, "fixture overhang changed")

carpet = material("M_BSW_KidsSageCarpet", (.21, .25, .15), .94)
border = material("M_BSW_KidsCarpetEdge", (.065, .075, .065), .72)


def actor_mesh(label, mesh, x, y, z, folder):
    actor = actors.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, z),
        unreal.Rotator(pitch=0, yaw=0, roll=0))
    require(actor is not None, "spawn failed: " + label)
    actor.set_actor_label(PREFIX + label)
    actor.set_folder_path(folder)
    require(actor.static_mesh_component.set_static_mesh(mesh),
            "mesh assignment failed: " + label)
    return actor


def covering(label, x, y, width, length, mat, height=1.0):
    actor = actor_mesh(label, cube, x, y, FLOOR_Z + height / 2 + .05,
                       "Boswells/Children/Floor")
    actor.set_actor_scale3d(unreal.Vector(width / 100, length / 100,
                                          height / 100))
    actor.set_actor_enable_collision(False)
    comp = actor.static_mesh_component
    comp.set_material(0, mat)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    comp.set_cast_shadow(False)
    if hasattr(comp, "set_can_ever_affect_navigation"):
        comp.set_can_ever_affect_navigation(False)
    return actor


# The source photos separate the hard tile promenade from softer sales-floor
# carpet. The upper front aisle and both 300 cm cross aisles remain clear.
covering("SOUTH_CARPET", -2500, -1750, 2800, 550, carpet)
covering("NORTH_CARPET", -2500, -700, 2800, 750, carpet)
for name, y in (("EDGE_S0", -2025), ("EDGE_S1", -1475),
                ("EDGE_N0", -1075), ("EDGE_N1", -325)):
    covering(name, -2500, y, 2800, 12, border, height=1.4)

centers_x = (-3650, -3150, -2650, -2150, -1650, -1250)
centers_y = (-1900, -1650, -900, -500)
racks = []
for y in centers_y:
    for x in centers_x:
        if (x, y) == (-2650, -1900):
            continue
        obj = actor_mesh("ROLLING_RACK_{}_{}".format(x, y), rack,
                         x, y, FLOOR_Z, "Boswells/Children/Racks")
        racks.append(obj)
sales_desk = actor_mesh("SALES_DESK", desk, -2650, -1900, FLOOR_Z,
                        "Boswells/Children/SalesDesk")

require(len(racks) == 23, "rack count changed")
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
after = {a.get_actor_label(): a for a in actors.get_all_level_actors()}
require(sum(label.startswith(PREFIX) for label in after) == 30,
        "saved department actor count differs")
require(all(label in after for label in (
    "ROUTE_ESCALATOR_ASCEND", "ROUTE_ESCALATOR_DESCEND",
    "ANCHOR_LOADING_DOCK_EXIT")), "route lost after save")
message = "BOSWELLS_KIDS SAVED 23 racks, one desk, two carpets, four borders"
unreal.log(message)
print(message)

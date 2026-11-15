"""Hang one glass-rod chandelier in the escalator well.

Run with Tools/run_editor_python.ps1 while Unreal Editor is closed. The rods
and mount share a folder. They are decoration only: no collision, navigation
carving, light component, or gameplay-light registration.
"""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
GLASS_PATH = "/Game/LLK/Boswells/Architecture/M_BSW_ChandelierGlass"
PREFIX = "BSW_ATRIUM_CHANDELIER_"


def require(ok, message):
    if not ok:
        raise RuntimeError("BOSWELLS_CHANDELIER " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_ApparelPass", "wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {a.get_actor_label(): a for a in actors.get_all_level_actors()}
require(not any(name.startswith(PREFIX) for name in before),
        "already placed; refusing duplicate")
for name in ("ANCHOR_FULL_ROOF", "ROUTE_ESCALATOR_ASCEND",
             "ROUTE_ESCALATOR_DESCEND", "BSW_CANOPY_GLASS_02_02"):
    require(name in before, "well landmark absent: " + name)
roof_center, roof_extent = before["ANCHOR_FULL_ROOF"].get_actor_bounds(False)
require(abs(roof_center.z - roof_extent.z - 1488) < 1,
        "roof underside moved; review rod suspension")

cylinder = unreal.EditorAssetLibrary.load_asset(
    "/Engine/BasicShapes/Cylinder.Cylinder")
cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
chrome = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Architecture/M_BSW_DarkChrome")
require(isinstance(cylinder, unreal.StaticMesh) and
        isinstance(cube, unreal.StaticMesh) and
        isinstance(chrome, unreal.MaterialInterface), "mesh or mount unavailable")


def glass_material():
    if unreal.EditorAssetLibrary.does_asset_exist(GLASS_PATH):
        asset = unreal.EditorAssetLibrary.load_asset(GLASS_PATH)
        require(isinstance(asset, unreal.Material), "glass path occupied")
        return asset
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_BSW_ChandelierGlass", "/Game/LLK/Boswells/Architecture",
        unreal.Material, unreal.MaterialFactoryNew())
    require(isinstance(asset, unreal.Material), "glass material creation failed")
    asset.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    asset.set_editor_property("two_sided", True)
    edit = unreal.MaterialEditingLibrary
    for material_property, value, row in (
            (unreal.MaterialProperty.MP_BASE_COLOR, (.48, .68, .75), -200),
            (unreal.MaterialProperty.MP_EMISSIVE_COLOR,
             (.035, .055, .075), 200)):
        color = edit.create_material_expression(
            asset, unreal.MaterialExpressionConstant3Vector, -430, row)
        color.set_editor_property("constant", unreal.LinearColor(*value, 1))
        edit.connect_material_property(color, "", material_property)
    for material_property, value, row in (
            (unreal.MaterialProperty.MP_ROUGHNESS, .07, 0),
            (unreal.MaterialProperty.MP_OPACITY, .38, 400)):
        scalar = edit.create_material_expression(
            asset, unreal.MaterialExpressionConstant, -430, row)
        scalar.set_editor_property("r", value)
        edit.connect_material_property(scalar, "", material_property)
    edit.recompile_material(asset)
    return asset


glass = glass_material()
parts = []


def piece(name, mesh, material, x, y, z, size):
    actor = actors.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator(0, 0, 0))
    require(actor is not None, "spawn failed: " + name)
    actor.set_actor_label(PREFIX + name)
    actor.set_folder_path("Boswells/Architecture/AtriumChandelier")
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(mesh), "mesh failed: " + name)
    comp.set_material(0, material)
    actor.set_actor_scale3d(unreal.Vector(*(v / 100 for v in size)))
    actor.set_actor_enable_collision(False)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    comp.set_cast_shadow(False)
    if hasattr(comp, "set_can_ever_affect_navigation"):
        comp.set_can_ever_affect_navigation(False)
    parts.append(actor)


# Seven by seven vertical rods describe a stepped hanging volume. Their top
# ends sit under the mirrored grid; the longest reaches z 1107, well clear of
# the upper-floor walking plane. The 236 cm footprint stays between slopes.
piece("MOUNT", cube, chrome, 0, 0, 1453, (246, 246, 5))
for ix in range(-3, 4):
    for iy in range(-3, 4):
        x, y = ix * 38, iy * 38
        length = 340 - 52 * max(abs(ix), abs(iy))
        top = 1447
        piece("ROD_{}_{}".format(ix + 3, iy + 3), cylinder, glass,
              x, y, top - length / 2, (3.2, 3.2, length))

require(len(parts) == 50, "part count differs")
require(unreal.EditorAssetLibrary.save_loaded_asset(glass),
        "glass material save failed")
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP),
        "map save failed")
message = "BOSWELLS_CHANDELIER SAVED: 49 glass rods, no light or collision"
unreal.log(message)
print(message)

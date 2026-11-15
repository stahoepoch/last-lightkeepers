"""Replace the default grid floor with metrically scaled store tile.

Texture world coordinates repeat every 240 cm; its four-by-four pattern is
therefore 60 cm per tile on every floor slab, regardless of slab dimensions.
The script changes materials only, so floor collision and route geometry stay.
Run with the editor closed after generate_boswells_store_tile.py.
"""

from pathlib import Path

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
ROOT = "/Game/LLK/Boswells/Architecture"
SOURCE = Path(unreal.Paths.project_dir()) / "assets/generated/boswells/T_BSW_StoneTile_4x4.png"
TEXTURE = ROOT + "/T_BSW_StoneTile_4x4"
MATERIAL = ROOT + "/M_BSW_StoneTile"
FLOORS = (
    "ANCHOR_FULL_FLOOR_LOWER",
    "ANCHOR_UPPER_NORTH",
    "ANCHOR_UPPER_SOUTH",
    "ANCHOR_UPPER_WEST",
    "ANCHOR_UPPER_EAST",
    "MALL_ENTRANCE_APPROACH_FLOOR",
)
OLD_MATERIAL = "/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"


def require(ok, reason):
    if not ok:
        raise RuntimeError("BOSWELLS_TILE " + reason)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_ApparelPass", "wrong map")
actors = {a.get_actor_label(): a for a in unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()}
require(SOURCE.is_file(), "source texture missing")
require(not unreal.EditorAssetLibrary.does_asset_exist(TEXTURE), "texture already imported")
require(not unreal.EditorAssetLibrary.does_asset_exist(MATERIAL), "material already exists")
for label in FLOORS:
    require(label in actors, "floor missing: " + label)
    comp = actors[label].static_mesh_component
    existing = comp.get_material(0)
    require(existing and existing.get_path_name() == OLD_MATERIAL,
            "material changed on " + label)

task = unreal.AssetImportTask()
task.set_editor_property("filename", str(SOURCE))
task.set_editor_property("destination_path", ROOT)
task.set_editor_property("destination_name", "T_BSW_StoneTile_4x4")
task.set_editor_property("automated", True)
task.set_editor_property("save", True)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
texture = unreal.EditorAssetLibrary.load_asset(TEXTURE)
require(isinstance(texture, unreal.Texture2D), "texture import failed")

material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
    "M_BSW_StoneTile", ROOT, unreal.Material, unreal.MaterialFactoryNew())
require(isinstance(material, unreal.Material), "material creation failed")
edit = unreal.MaterialEditingLibrary
position = edit.create_material_expression(material,
                                           unreal.MaterialExpressionWorldPosition,
                                           -700, -250)
xy = edit.create_material_expression(material,
                                     unreal.MaterialExpressionComponentMask,
                                     -475, -250)
xy.set_editor_property("r", True)
xy.set_editor_property("g", True)
xy.set_editor_property("b", False)
xy.set_editor_property("a", False)
scale = edit.create_material_expression(material,
                                        unreal.MaterialExpressionConstant,
                                        -475, 30)
scale.set_editor_property("r", 1.0 / 240.0)
multiply = edit.create_material_expression(material,
                                           unreal.MaterialExpressionMultiply,
                                           -230, -160)
sample = edit.create_material_expression(material,
                                         unreal.MaterialExpressionTextureSample,
                                         20, -170)
sample.set_editor_property("texture", texture)
roughness = edit.create_material_expression(material,
                                            unreal.MaterialExpressionConstant,
                                            20, 150)
roughness.set_editor_property("r", 0.34)
# UE 5.8 exposes ComponentMask's sole input as an unnamed pin and the
# TextureSample coordinate input as "UVs". A guessed name fails silently;
# the live graph inspection caught the stretched, default-UV result.
for ok, name in (
        (edit.connect_material_expressions(position, "", xy, ""), "world position to XY"),
        (edit.connect_material_expressions(xy, "", multiply, "A"), "XY to scale"),
        (edit.connect_material_expressions(scale, "", multiply, "B"), "scale constant"),
        (edit.connect_material_expressions(multiply, "", sample, "UVs"), "sample UVs"),
        (edit.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR),
         "tile base color"),
        (edit.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS),
         "tile roughness")):
    require(ok, "material wire failed: " + name)
edit.recompile_material(material)
require(unreal.EditorAssetLibrary.save_loaded_asset(material), "material save failed")
require(unreal.EditorAssetLibrary.save_loaded_asset(texture), "texture save failed")

for label in FLOORS:
    actors[label].static_mesh_component.set_material(0, material)
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
print("BOSWELLS_TILE SAVED: 60cm stone tile on six floor slabs, no geometry or lights changed")

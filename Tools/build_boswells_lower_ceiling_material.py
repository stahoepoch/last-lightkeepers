"""Import the one-tile ceiling texture and make a 30 x 40 repeat material."""

from pathlib import Path

import unreal


ROOT = "/Game/LLK/Boswells/Architecture"
SOURCE = Path(unreal.Paths.project_dir()) / "assets/generated/boswells/T_BSW_LowerCeilingTile.png"
TEXTURE = ROOT + "/T_BSW_LowerCeilingTile"
MATERIAL = ROOT + "/M_BSW_LowerCeilingTile"


def require(ok, reason):
    if not ok:
        raise RuntimeError("BOSWELLS_CEILING_MATERIAL " + reason)


require(SOURCE.is_file(), "source texture absent")
require(not unreal.EditorAssetLibrary.does_asset_exist(TEXTURE), "texture already exists")
require(not unreal.EditorAssetLibrary.does_asset_exist(MATERIAL), "material already exists")

task = unreal.AssetImportTask()
task.set_editor_property("filename", str(SOURCE))
task.set_editor_property("destination_path", ROOT)
task.set_editor_property("destination_name", "T_BSW_LowerCeilingTile")
task.set_editor_property("automated", True)
task.set_editor_property("save", True)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
texture = unreal.EditorAssetLibrary.load_asset(TEXTURE)
require(isinstance(texture, unreal.Texture2D), "texture import failed")

material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
    "M_BSW_LowerCeilingTile", ROOT, unreal.Material, unreal.MaterialFactoryNew())
require(isinstance(material, unreal.Material), "material creation failed")
edit = unreal.MaterialEditingLibrary
uv = edit.create_material_expression(material,
                                     unreal.MaterialExpressionTextureCoordinate,
                                     -400, -180)
uv.set_editor_property("u_tiling", 30.0)
uv.set_editor_property("v_tiling", 40.0)
sample = edit.create_material_expression(material,
                                         unreal.MaterialExpressionTextureSample,
                                         -150, -180)
sample.set_editor_property("texture", texture)
rough = edit.create_material_expression(material,
                                        unreal.MaterialExpressionConstant, 80, 180)
rough.set_editor_property("r", 0.8)
glow = edit.create_material_expression(material,
                                       unreal.MaterialExpressionConstant, -150, 120)
glow.set_editor_property("r", 0.035)
multiply = edit.create_material_expression(material,
                                           unreal.MaterialExpressionMultiply, 90, -100)
for ok, name in (
        (edit.connect_material_expressions(uv, "", sample, "UVs"), "UV repeat"),
        (edit.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR),
         "base color"),
        (edit.connect_material_expressions(sample, "RGB", multiply, "A"), "emissive color"),
        (edit.connect_material_expressions(glow, "", multiply, "B"), "emissive scale"),
        (edit.connect_material_property(multiply, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR),
         "subdued reflected glow"),
        (edit.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS),
         "roughness")):
    require(ok, "material connection failed: " + name)
edit.recompile_material(material)
require(unreal.EditorAssetLibrary.save_loaded_asset(material), "material save failed")
require(unreal.EditorAssetLibrary.save_loaded_asset(texture), "texture save failed")
unreal.log("BOSWELLS_CEILING_MATERIAL SAVED: 30 x 40 tiles, subtle ambient underside")

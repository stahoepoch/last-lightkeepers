"""Make an unlit copy of Unreal's masked distance-field TextRender material."""

import unreal


SOURCE = "/Engine/EngineMaterials/DefaultTextMaterialOpaque"
ROOT = "/Game/LLK/Boswells/Architecture"
NAME = "M_BSW_EmissiveWayfindingText"
PATH = ROOT + "/" + NAME
if unreal.EditorAssetLibrary.does_asset_exist(PATH):
    raise RuntimeError("wayfinding text material already exists")
material = unreal.EditorAssetLibrary.duplicate_asset(SOURCE, PATH)
if not isinstance(material, unreal.Material):
    raise RuntimeError("could not duplicate engine TextRender material")
vertices = [expr for expr in
            unreal.MaterialEditingLibrary.get_material_expressions(material)
            if isinstance(expr, unreal.MaterialExpressionVertexColor)]
if len(vertices) != 1:
    raise RuntimeError("engine text material's vertex-color graph changed")
material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
if not unreal.MaterialEditingLibrary.connect_material_property(
        vertices[0], "", unreal.MaterialProperty.MP_EMISSIVE_COLOR):
    raise RuntimeError("could not route text vertex color to emissive")
unreal.MaterialEditingLibrary.recompile_material(material)
if not unreal.EditorAssetLibrary.save_loaded_asset(material):
    raise RuntimeError("wayfinding text material save failed")
unreal.log("BOSWELLS_WAYFINDING_TEXT_MATERIAL SAVED: unlit, masked font")

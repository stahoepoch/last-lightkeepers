"""Import the four centimeter-scale case displays without overwriting assets."""

from pathlib import Path
import unreal

ROOT = Path(unreal.Paths.project_dir()) / "assets/models/boswells"
DEST = "/Game/LLK/Boswells/Fixtures"
STEMS = [f"SM_BSW_CaseGoods_{kind}_{variant}"
         for kind in ("Perfume", "Jewelry") for variant in ("A", "B")]


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_CASE_GOODS_IMPORT " + why)


for stem in STEMS:
    require((ROOT / (stem + ".obj")).is_file(), "source missing: " + stem)
    require(not unreal.EditorAssetLibrary.does_asset_exist(DEST + "/" + stem),
            "asset exists: " + stem)

for stem in STEMS:
    task = unreal.AssetImportTask()
    task.filename = str(ROOT / (stem + ".obj"))
    task.destination_path = DEST
    task.destination_name = stem
    task.automated = True
    task.replace_existing = False
    task.save = True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = unreal.EditorAssetLibrary.load_asset(DEST + "/" + stem)
    require(isinstance(mesh, unreal.StaticMesh), "missing result: " + stem)
    bounds = mesh.get_bounds().box_extent
    require(bounds.x * 2 <= 150 and bounds.y * 2 <= 45 and
            mesh.get_bounds().origin.z + bounds.z <= 99,
            "case display exceeds glass: " + stem)
    slots = mesh.get_editor_property("static_materials")
    require(len(slots) >= 4 and all(
        s.get_editor_property("material_interface") is not None
        for s in slots), "missing palette material: " + stem)
    print("BOSWELLS_CASE_GOODS_IMPORTED", stem,
          f"bounds={bounds.x*2:.1f},{bounds.y*2:.1f},{bounds.z*2:.1f}")

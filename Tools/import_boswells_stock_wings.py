"""Import the visual-only apparel wing meshes from their centimeter OBJs.

The four names are new and this command refuses to overwrite an asset. The
existing palette materials are shared with the other Boswell's fixtures.
"""

from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.project_dir())
SOURCE = ROOT / "assets/models/boswells"
DEST = "/Game/LLK/Boswells/Fixtures"
STEMS = tuple(
    f"SM_BSW_StockWing_{length}_{variant}"
    for length in ("Short", "Long") for variant in ("A", "B")
)


def require(ok, reason):
    if not ok:
        raise RuntimeError("BOSWELLS_STOCK_WINGS_IMPORT " + reason)


for stem in STEMS:
    require((SOURCE / (stem + ".obj")).is_file(), "source missing: " + stem)
    require(not unreal.EditorAssetLibrary.does_asset_exist(DEST + "/" + stem),
            "asset already exists; refusing re-import: " + stem)

for stem in STEMS:
    task = unreal.AssetImportTask()
    task.filename = str(SOURCE / (stem + ".obj"))
    task.destination_path = DEST
    task.destination_name = stem
    task.automated = True
    task.replace_existing = False
    task.save = True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = unreal.EditorAssetLibrary.load_asset(DEST + "/" + stem)
    require(isinstance(mesh, unreal.StaticMesh), "import did not produce mesh: " + stem)
    bounds = mesh.get_bounds().box_extent
    expect_length = 150 if "Short" in stem else 600
    require(bounds.x * 2 <= 45.1 and bounds.y * 2 <= expect_length + 0.1
            and bounds.z * 2 <= 170.1,
            "imported dimensions exceed collision wing: " + stem +
            f" ({bounds.x * 2:.1f},{bounds.y * 2:.1f},{bounds.z * 2:.1f})")
    print("BOSWELLS_STOCK_WINGS_IMPORTED", stem,
          f"bounds={bounds.x*2:.1f},{bounds.y*2:.1f},{bounds.z*2:.1f}")

"""Import the two children's fixtures, never reimport the old apparel kit."""

from pathlib import Path

import unreal


source_dir = Path(unreal.Paths.project_dir()) / "assets/models/boswells"
destination = "/Game/LLK/Boswells/Fixtures"
for name in ("SM_BSW_KidsRollingRack", "SM_BSW_KidsSalesDesk"):
    source = source_dir / (name + ".obj")
    if not source.is_file():
        raise RuntimeError("missing authored source: " + str(source))
    if unreal.EditorAssetLibrary.does_asset_exist(destination + "/" + name):
        print("BOSWELLS_KIDS_IMPORT EXISTS " + name)
        continue
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source))
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("destination_name", name)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)
    task.set_editor_property("replace_existing", False)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = unreal.EditorAssetLibrary.load_asset(destination + "/" + name)
    if not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError("import failed for " + name)
    print("BOSWELLS_KIDS_IMPORT {} extent={}".format(name, mesh.get_bounds().box_extent))

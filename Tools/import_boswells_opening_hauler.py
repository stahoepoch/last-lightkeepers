"""Import the authored single-mesh opening rover silhouette."""

from pathlib import Path
import unreal

source = Path(unreal.Paths.project_dir()) / "assets/models/boswells/SM_BSW_OpeningHauler.obj"
destination = "/Game/LLK/Boswells/Fixtures"
target = destination + "/SM_BSW_OpeningHauler"
assert source.is_file(), source
task = unreal.AssetImportTask()
task.set_editor_property("filename", str(source))
task.set_editor_property("destination_path", destination)
task.set_editor_property("destination_name", "SM_BSW_OpeningHauler")
task.set_editor_property("automated", True)
task.set_editor_property("save", True)
task.set_editor_property("replace_existing", True)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
mesh = unreal.EditorAssetLibrary.load_asset(target)
assert isinstance(mesh, unreal.StaticMesh), "hauler import failed"
unreal.log("BOSWELLS_OPENING_HAULER IMPORTED: " + str(mesh.get_bounds().box_extent))
print("BOSWELLS_OPENING_HAULER IMPORTED")

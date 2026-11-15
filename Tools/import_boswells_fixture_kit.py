"""Import the three authored Boswell's apparel fixture OBJ masters.

Run with Tools/run_editor_python.ps1 while Unreal Editor is closed. Reimports
only this generated kit. Mesh sources live in assets/models/boswells.
"""

from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.project_dir())
SOURCE = ROOT / "assets" / "models" / "boswells"
DEST = "/Game/LLK/Boswells/Fixtures"
NAMES = ("SM_BSW_ClothingRun", "SM_BSW_FourWay", "SM_BSW_RoundRack")


def log(message):
    print("BOSWELLS_FIXTURE_IMPORT " + message)
    unreal.log("BOSWELLS_FIXTURE_IMPORT " + message)


def main():
    for name in NAMES:
        path = DEST + "/" + name
        source = SOURCE / (name + ".obj")
        if not source.exists():
            raise RuntimeError("missing source " + str(source))
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", str(source))
        task.set_editor_property("destination_path", DEST)
        task.set_editor_property("destination_name", name)
        task.set_editor_property("automated", True)
        task.set_editor_property("save", True)
        task.set_editor_property("replace_existing", True)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        mesh = unreal.EditorAssetLibrary.load_asset(path)
        if not isinstance(mesh, unreal.StaticMesh):
            raise RuntimeError("import did not make StaticMesh: " + path)
        bounds = mesh.get_bounds()
        log("IMPORTED {} extent={} material_slots={}".format(
            path, bounds.box_extent, mesh.get_num_sections(0)))
    log("COMPLETE")


main()

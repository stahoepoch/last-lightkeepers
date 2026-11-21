"""Import the one-word machine call from the locally authored WAV.

The source is a 2026-11-21 Windows System.Speech synthesis of "Sayers": a
placeholder machine performance that needs a listening pass, not found audio.
"""

from pathlib import Path
import unreal

source = Path(unreal.Paths.project_dir()) / "assets/audio/opening-sayers-machine-v1.wav"
target = "/Game/Audio/Opening/opening-sayers-machine-v1"
assert source.is_file(), source
assert not unreal.EditorAssetLibrary.does_asset_exist(target), "voice already imported"

task = unreal.AssetImportTask()
task.set_editor_property("filename", str(source))
task.set_editor_property("destination_path", "/Game/Audio/Opening")
task.set_editor_property("destination_name", "opening-sayers-machine-v1")
task.set_editor_property("automated", True)
task.set_editor_property("save", True)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
sound = unreal.EditorAssetLibrary.load_asset(target)
assert isinstance(sound, unreal.SoundWave), "machine call import failed"
unreal.log("BOSWELLS_OPENING_VOICE IMPORTED: one-word Sayers call")
print("BOSWELLS_OPENING_VOICE IMPORTED")

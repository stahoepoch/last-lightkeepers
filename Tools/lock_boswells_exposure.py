"""Set a repeatable legacy exposure lock in the dressed Boswell's map.

The isolated LightLab's EV100-5 / 4.0 value made this much larger sales floor
nearly black. This script permits a bounded camera sweep in Boswell's itself.
Set BOSWELLS_TEST_EXPOSURE in the shell; the default 0.5 is the current candidate.
"""

import os
import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
LABEL = "BSW_EXPOSURE_LOCK"
value = float(os.environ.get("BOSWELLS_TEST_EXPOSURE", "0.5"))
if not 0.05 <= value <= 4.0:
    raise RuntimeError("exposure outside bounded test range")
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_ApparelPass":
    raise RuntimeError("wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
all_actors = actors.get_all_level_actors()
volumes = [a for a in all_actors if isinstance(a, unreal.PostProcessVolume)]
if len(volumes) > 1:
    raise RuntimeError("competing post-process volumes; refusing edit")
volume = (volumes[0] if volumes else actors.spawn_actor_from_class(
    unreal.PostProcessVolume, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0)))
if not volume:
    raise RuntimeError("volume spawn failed")
volume.set_actor_label(LABEL)
volume.set_folder_path("Boswells/Lighting")
volume.set_editor_property("unbound", True)
volume.set_editor_property("priority", 1.0)
volume.set_editor_property("blend_weight", 1.0)
settings = volume.get_editor_property("settings")
for name, value in (
        ("override_auto_exposure_min_brightness", True),
        ("override_auto_exposure_max_brightness", True),
        ("auto_exposure_min_brightness", value),
        ("auto_exposure_max_brightness", value)):
    settings.set_editor_property(name, value)
volume.set_editor_property("settings", settings)
if not unreal.EditorLoadingAndSavingUtils.save_map(world, MAP):
    raise RuntimeError("map save failed")
unreal.log("BOSWELLS_EXPOSURE_LOCK SAVED: unbound priority 1, legacy min=max={}".format(value))

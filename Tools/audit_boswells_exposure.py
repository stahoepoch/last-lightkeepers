"""Read the saved Boswell's post-process volumes and exposure values."""

import unreal


world = unreal.EditorLoadingAndSavingUtils.load_map(
    "/Game/LLK_Boswells_ApparelPass")
if not world or world.get_name() != "LLK_Boswells_ApparelPass":
    raise RuntimeError("wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
volumes = [a for a in actors if isinstance(a, unreal.PostProcessVolume)]
unreal.log("BOSWELLS_EXPOSURE_AUDIT volumes={}".format(len(volumes)))
unreal.log("BOSWELLS_EXPOSURE_AUDIT fields={}".format(
    [name for name in dir(unreal.PostProcessSettings())
     if "auto_exposure" in name]))
for volume in volumes:
    settings = volume.get_editor_property("settings")
    values = {}
    for name in ("auto_exposure_min_brightness", "auto_exposure_max_brightness",
                 "auto_exposure_method", "override_auto_exposure_min_brightness",
                 "override_auto_exposure_max_brightness"):
        try:
            values[name] = settings.get_editor_property(name)
        except Exception as exc:
            values[name] = "UNAVAILABLE: " + str(exc).splitlines()[0]
    unreal.log("BOSWELLS_EXPOSURE_AUDIT {} unbound={} priority={} {}".format(
        volume.get_actor_label(), volume.get_editor_property("unbound"),
        volume.get_editor_property("priority"), values))

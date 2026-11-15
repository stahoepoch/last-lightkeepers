"""Print true lower-light origins; spotlight bounds are offset from origins."""

import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/LLK_Boswells_ApparelPass")
if not world or world.get_name() != "LLK_Boswells_ApparelPass":
    raise RuntimeError("wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
for actor in sorted(actors, key=lambda a: a.get_actor_label()):
    label = actor.get_actor_label()
    if not (label.startswith("ANCHOR_EMERGENCY_LOWER_") or
            label.startswith("BSW_EGRESS_LOWER_")):
        continue
    if label.endswith("_Housing"):
        centre, extent = actor.get_actor_bounds(False)
        unreal.log("BOSWELLS_LOWER_HOUSING {} centre=({:.0f},{:.0f},{:.0f}) extent=({:.0f},{:.0f},{:.0f})".format(
            label, centre.x, centre.y, centre.z, extent.x, extent.y, extent.z))
        continue
    if not isinstance(actor, unreal.SpotLight):
        continue
    origin = actor.get_actor_location()
    centre, _ = actor.get_actor_bounds(False)
    comp = actor.spot_light_component
    message = ("BOSWELLS_LOWER_LIGHT {} origin=({:.0f},{:.0f},{:.0f}) "
               "bounds_z={:.0f} intensity={:.1f} radius={:.0f}".format(
                   label, origin.x, origin.y, origin.z, centre.z,
                   comp.get_editor_property("intensity"),
                   comp.get_editor_property("attenuation_radius")))
    unreal.log(message)
    print(message)

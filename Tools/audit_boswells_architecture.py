"""Read the saved apparel map's shell, atrium, columns, and lights."""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_ApparelPass":
    raise RuntimeError("wrong map")

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
unreal.log("BOSWELLS_ARCH_AUDIT actors={}".format(len(actors)))
for actor in sorted(actors, key=lambda a: a.get_actor_label()):
    label = actor.get_actor_label()
    if not isinstance(actor, (unreal.PointLight, unreal.SpotLight,
                              unreal.DirectionalLight, unreal.SkyLight)) and not any(word in label.upper() for word in (
            "CEIL", "ROOF", "UPPER", "ATRIUM", "COLUMN", "PILLAR",
            "LIGHT", "NEON", "ESCALATOR", "FLOOR", "SKY")):
        continue
    centre, extent = actor.get_actor_bounds(False)
    unreal.log("ACTOR {} {} centre=({:.0f},{:.0f},{:.0f}) extent=({:.0f},{:.0f},{:.0f})".format(
        label, actor.get_class().get_name(), centre.x, centre.y, centre.z,
        extent.x, extent.y, extent.z))
    for cls in (unreal.PointLightComponent, unreal.SpotLightComponent,
                unreal.DirectionalLightComponent, unreal.SkyLightComponent):
        for comp in actor.get_components_by_class(cls):
            details = []
            for field in ("intensity", "attenuation_radius", "source_radius",
                          "outer_cone_angle", "indirect_lighting_intensity",
                          "affects_world"):
                try:
                    details.append("{}={}".format(field, comp.get_editor_property(field)))
                except Exception:
                    pass
            rot = comp.get_world_rotation()
            unreal.log("  LIGHT {} rot=({:.0f},{:.0f},{:.0f}) {}".format(
                cls.__name__, rot.pitch, rot.yaw, rot.roll, " ".join(details)))

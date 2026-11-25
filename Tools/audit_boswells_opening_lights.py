"""Print the saved opening map's authored visual-light inventory."""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if world is None:
    raise RuntimeError("OPENING_LIGHT_AUDIT could not load map")

for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if not isinstance(actor, (unreal.PointLight, unreal.SpotLight, unreal.DirectionalLight)):
        continue
    label = actor.get_actor_label()
    component = actor.get_component_by_class(unreal.LightComponent)
    if component is None:
        continue
    location = actor.get_actor_location()
    intensity = component.get_editor_property("intensity")
    radius = component.get_editor_property("attenuation_radius") if hasattr(component, "get_editor_property") else 0
    mobility = component.get_editor_property("mobility")
    unreal.log("OPENING_LIGHT label={} class={} loc=({:.0f},{:.0f},{:.0f}) intensity={} radius={} mobility={}".format(
        label, actor.get_class().get_name(), location.x, location.y, location.z,
        intensity, radius, mobility))

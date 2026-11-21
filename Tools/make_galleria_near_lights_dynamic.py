"""Clear the unbuilt interactions from the opening concourse route lights.

The concourse builder spawned ten PointLights with Unreal's default Stationary
mobility. The opening map has no light bake; their 8 cd pools should render
dynamically, just like Boswell's emergency lights. Keep all photometry intact.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "GALLERIA_NEAR_ROUTE_LIGHT_"


def require(condition, message):
    if not condition:
        raise RuntimeError("GALLERIA_DYNAMIC_LIGHTS " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
by_label = {actor.get_actor_label(): actor for actor in actors}
require(len(by_label) == len(actors), "duplicate actor labels")
lights = [(name, actor) for name, actor in by_label.items()
          if name.startswith(PREFIX)]
require(len(lights) == 10, "expected ten concourse route lights")

for name, actor in lights:
    require(isinstance(actor, unreal.PointLight), "not a point light: " + name)
    component = actor.point_light_component
    require(component.get_editor_property("mobility") in
            (unreal.ComponentMobility.STATIC, unreal.ComponentMobility.STATIONARY,
             unreal.ComponentMobility.MOVABLE),
            "unexpected mobility: " + name)
    require(component.get_editor_property("intensity_units") ==
            unreal.LightUnits.CANDELAS and
            abs(component.get_editor_property("intensity") - 8.0) < 0.01 and
            abs(component.get_editor_property("attenuation_radius") - 720.0) < 0.01 and
            abs(component.get_editor_property("indirect_lighting_intensity") - 0.25) < 0.01,
            "authored light band changed: " + name)

for _, actor in lights:
    actor.point_light_component.set_mobility(unreal.ComponentMobility.MOVABLE)

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
print("GALLERIA_DYNAMIC_LIGHTS SAVED: ten 8 cd route lights are movable")

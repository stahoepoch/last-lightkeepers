"""Make OpeningPass's authored emergency route lights work without a light bake.

The original 31 route spots were Stationary, but this map is changing every
day and has no built lighting data. Keep their existing position, color,
intensity, cone, and radius. Only change mobility after validating the map.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_EGRESS_"


def require(condition, message):
    if not condition:
        raise RuntimeError("BOSWELLS_DYNAMIC_EGRESS " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
by_label = {actor.get_actor_label(): actor for actor in actors}
require(len(by_label) == len(actors), "duplicate actor labels")
route_lights = [(name, actor) for name, actor in by_label.items()
                if name.startswith(PREFIX)]
require(len(route_lights) == 31, "expected 31 route lights")
for name, actor in route_lights:
    require(isinstance(actor, unreal.SpotLight), "route actor is not a spotlight: " + name)
    light = actor.spot_light_component
    require(light.get_editor_property("mobility") == unreal.ComponentMobility.STATIONARY,
            "route mobility already changed: " + name)
    require(light.get_editor_property("intensity_units") == unreal.LightUnits.CANDELAS and
            abs(light.get_editor_property("intensity") - 8.0) < 0.01 and
            abs(light.get_editor_property("attenuation_radius") - 1050.0) < 0.01,
            "route light authored band changed: " + name)

for _, actor in route_lights:
    actor.spot_light_component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
print("BOSWELLS_DYNAMIC_EGRESS SAVED: 31 route spots are movable; all other settings retained")

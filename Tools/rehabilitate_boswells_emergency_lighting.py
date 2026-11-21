"""Make the opening store's emergency power legible without making it safe.

The prototype's visual lights were largely Stationary (there is no bake) and
were authored at 8 cd, so the player saw a black void interrupted by a few
unrelated pools.  This keeps the same fixtures and routes, but makes their
working subset movable and gives each route a deliberately limited read:
walkable floor, nearby rack silhouettes, and the next decision point.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"


def require(ok, message):
    if not ok:
        raise RuntimeError("BOSWELLS_LIGHT_REHAB " + message)


def tune(component, intensity, radius, color, shadows=False):
    component.set_mobility(unreal.ComponentMobility.MOVABLE)
    component.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    component.set_editor_property("intensity", intensity)
    component.set_editor_property("attenuation_radius", radius)
    component.set_editor_property("light_color", color)
    component.set_editor_property("indirect_lighting_intensity", 0.18)
    component.set_editor_property("cast_shadows", shadows)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
by_label = {actor.get_actor_label(): actor for actor in actors}
require(len(by_label) == len(actors), "duplicate labels")

# Sickly emergency-white is a navigational read, not a normal retail lighting
# palette.  The low saturation avoids turning every floor pool into a neon cue.
EGRESS = unreal.Color(189, 212, 148, 255)
SERVICE = unreal.Color(219, 194, 135, 255)

changed = []
for label, actor in by_label.items():
    if label.startswith("ANCHOR_EMERGENCY_") and isinstance(actor, unreal.SpotLight):
        tune(actor.spot_light_component, 350.0, 1600.0, EGRESS)
        changed.append(label)
    elif label.startswith("BSW_EGRESS_"):
        require(isinstance(actor, unreal.SpotLight), "egress fixture type: " + label)
        tune(actor.spot_light_component, 500.0, 1300.0, EGRESS,
             shadows=("LOWER_MAIN" in label))
        changed.append(label)
    elif label.startswith("BSW_DEPT_ROUTE_POOL_"):
        require(isinstance(actor, unreal.SpotLight), "department fixture type: " + label)
        tune(actor.spot_light_component, 700.0, 1200.0, SERVICE, shadows=True)
        changed.append(label)
    elif label.startswith("BSW_OPENING_ENTRY_POOL_") and isinstance(actor, unreal.SpotLight):
        tune(actor.spot_light_component, 900.0, 1200.0, SERVICE, shadows=True)
        changed.append(label)
    elif label.startswith("BSW_ESC_DETAIL_") and "_ROUTE_" in label:
        require(isinstance(actor, unreal.SpotLight), "escalator fixture type: " + label)
        tune(actor.spot_light_component, 300.0, 750.0, EGRESS)
        changed.append(label)
    elif label.startswith("BSW_KIDS_CEILING_FILL_"):
        require(isinstance(actor, unreal.PointLight), "kids fill fixture type: " + label)
        tune(actor.point_light_component, 250.0, 1100.0, EGRESS)
        changed.append(label)
    elif label.startswith("GALLERIA_NEAR_ROUTE_LIGHT_"):
        require(isinstance(actor, unreal.PointLight), "concourse route fixture type: " + label)
        tune(actor.point_light_component, 400.0, 1000.0, EGRESS)
        changed.append(label)
    elif label.startswith("GALLERIA_SHOP_") and label.endswith("_POOL"):
        require(isinstance(actor, unreal.PointLight), "shop pool fixture type: " + label)
        tune(actor.point_light_component, 200.0, 800.0, SERVICE)
        changed.append(label)
    elif label.startswith("GALLERIA_SHOP_") and label.endswith("_FRONT_LIGHT"):
        require(isinstance(actor, unreal.SpotLight), "shop front fixture type: " + label)
        tune(actor.spot_light_component, 400.0, 900.0, SERVICE, shadows=True)
        changed.append(label)

require(len(changed) >= 90, "unexpectedly small light inventory: {}".format(len(changed)))
require(by_label["BSW_CAPTURE_STORM_FLASH"].point_light_component.get_editor_property("intensity") == 0.0,
        "storm flash must remain off at rest")
require(by_label["BSW_CAPTURE_WITNESS_POOL"].point_light_component.get_editor_property("intensity") == 6.0,
        "capture witness beat must remain untouched")
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "save failed")
print("BOSWELLS_LIGHT_REHAB SAVED: {} visual fixtures now make emergency routes, cover silhouettes, and next choices readable".format(len(changed)))

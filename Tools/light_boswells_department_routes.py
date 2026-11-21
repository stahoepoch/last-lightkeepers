"""Add downcast emergency pools to the active OpeningPass department aisles.

The 31 inherited egress spots sit on x=0 and x=+/-2450. Their 1050 cm
attenuation leaves the new shoe, bedding, and toy routes mostly dark. These
sixteen local spots follow customer walking lanes, with merchandise off the
beam axis. They are visual lights, not gameplay lantern sources.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_DEPT_ROUTE_POOL_"


def require(condition, message):
    if not condition:
        raise RuntimeError("BOSWELLS_DEPT_LIGHT " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = subsystem.get_all_level_actors()
by_label = {actor.get_actor_label(): actor for actor in actors}
require(len(by_label) == len(actors), "duplicate labels")
require(not any(name.startswith(PREFIX) for name in by_label), "route pass already placed")
require(sum(name.startswith("BSW_EGRESS_") for name in by_label) == 31,
        "inherited egress inventory changed")
require(sum(name.endswith("_AUTHORED_VISUAL") and
            name.startswith("BSW_DEPARTMENT_") for name in by_label) == 48,
        "modeled department pass missing")

# Existing bright customer routes are x=0 and x=+/-2450. The added pools
# address the dark stock lanes between and beyond them. Upper lights sit just
# below the 1325 cm ceiling, like the inherited upper egress spots.
positions = (
    ("LOWER_SHOES", 1250, 1150, 485, 16),
    ("LOWER_SHOES", 2150, 1150, 485, 16),
    ("LOWER_SHOES", 3150, 1150, 485, 16),
    ("LOWER_APPAREL", -3150, -1150, 485, 16),
    ("LOWER_APPAREL", -3150, 1150, 485, 16),
    ("LOWER_APPAREL", -1650, -1150, 485, 16),
    ("LOWER_APPAREL", -1650, 1150, 485, 16),
    ("UPPER_HOME", -3150, 1150, 1320, 24),
    ("UPPER_HOME", -2150, 1150, 1320, 24),
    ("UPPER_HOME", -1250, 1150, 1320, 24),
    ("UPPER_TOYS", 1400, -1250, 1320, 24),
    ("UPPER_TOYS", 2400, -1250, 1320, 24),
    ("UPPER_TOYS", 3400, -1250, 1320, 24),
    ("UPPER_HOUSE", 1400, 1150, 1320, 24),
    ("UPPER_HOUSE", 2400, 1150, 1320, 24),
    ("UPPER_HOUSE", 3400, 1150, 1320, 24),
)
require(len(positions) == 16, "plan changed")

for index, (region, x, y, z, intensity) in enumerate(positions):
    label = f"{PREFIX}{index:02d}_{region}"
    actor = subsystem.spawn_actor_from_class(
        unreal.SpotLight, unreal.Vector(x, y, z),
        unreal.Rotator(pitch=-90, yaw=0, roll=0))
    require(actor is not None, "failed to spawn " + label)
    actor.set_actor_label(label)
    actor.set_folder_path("Boswells/Lighting/DepartmentRoutes")
    light = actor.spot_light_component
    light.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    light.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    light.set_editor_property("intensity", float(intensity))
    light.set_editor_property("attenuation_radius", 950.0)
    light.set_editor_property("inner_cone_angle", 30.0)
    light.set_editor_property("outer_cone_angle", 50.0)
    light.set_editor_property("source_radius", 40.0)
    light.set_editor_property("indirect_lighting_intensity", 0.25)
    light.set_editor_property("cast_shadows", True)

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "save failed")
print("BOSWELLS_DEPT_LIGHT SAVED: 16 movable downcast pools on department routes")

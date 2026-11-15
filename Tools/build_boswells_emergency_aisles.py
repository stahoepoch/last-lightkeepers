"""Add a repeatable emergency-light path to the Boswell's apparel map.

The existing 20 aisle spots are about 20 m apart across a 87 x 54 m floor.
This pass fills the *walking routes*, leaving most shelf faces outside the
direct cone. These are ordinary visual SpotLights, never gameplay lanterns.
Run headlessly while the editor is closed, then inspect the saved map in PIE.
"""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
PREFIX = "BSW_EGRESS_"


def require(condition, message):
    if not condition:
        raise RuntimeError("BOSWELLS_EGRESS " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_ApparelPass", "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {actor.get_actor_label(): actor for actor in subsystem.get_all_level_actors()}
require("ANCHOR_FULL_ROOF" in before and "ROUTE_ESCALATOR_ASCEND" in before,
        "route shell differs")
require(not any(name.startswith(PREFIX) for name in before),
        "egress lights already exist; refusing duplicate")

# Photograph-inspired ceiling rhythm along the tiled customer routes. The
# atrium positions hang below the mirrored canopy; upper x=0 positions avoid
# its open floor. Spacing is <= 8 m along the main travel direction.
positions = []
for y in (-2350, -1550, -750, 50, 850, 1650, 2350):
    positions.append(("LOWER_MAIN", 0, y, 485))
for x in (-2450, 2450):
    for y in (-2350, -1150, 50, 1250, 2350):
        positions.append(("LOWER_WING", x, y, 485))
for x in (-2450, 2450):
    for y in (-2350, -1150, 50, 1250, 2350):
        positions.append(("UPPER_WING", x, y, 1320))
for y in (-2350, -1550, 1550, 2350):
    positions.append(("UPPER_LINK", 0, y, 1320))

made = []
for index, (group, x, y, z) in enumerate(positions):
    label = "{}{}_{:02d}".format(PREFIX, group, index)
    # Unreal Python's positional Rotator constructor is roll, pitch, yaw.
    # Name the axes: -90 pitch points the spotlight at the floor.
    actor = subsystem.spawn_actor_from_class(
        unreal.SpotLight, unreal.Vector(x, y, z),
        unreal.Rotator(pitch=-90, yaw=0, roll=0))
    require(actor is not None, "could not spawn " + label)
    actor.set_actor_label(label)
    actor.set_folder_path("Boswells/Lighting/EmergencyRoutes")
    light = actor.spot_light_component
    light.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    light.set_editor_property("intensity", 8.0)
    light.set_editor_property("attenuation_radius", 1050.0)
    light.set_editor_property("inner_cone_angle", 35.0)
    light.set_editor_property("outer_cone_angle", 65.0)
    light.set_editor_property("source_radius", 40.0)
    light.set_editor_property("indirect_lighting_intensity", 0.25)
    light.set_editor_property("cast_shadows", False)
    made.append(actor)

require(len(made) == 31, "unexpected light count")
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "save failed")
after = {actor.get_actor_label(): actor for actor in subsystem.get_all_level_actors()}
require(sum(name.startswith(PREFIX) for name in after) == 31,
        "saved light count differs")
require("ROUTE_ESCALATOR_ASCEND" in after and
        "ANCHOR_LOADING_DOCK_EXIT" in after, "route lost")
message = "BOSWELLS_EGRESS SAVED 31 visual route lights; route and dock retained"
unreal.log(message)
print(message)

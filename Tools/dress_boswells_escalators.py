"""Make the opening map's inert escalators read as walkable store machinery.

The original pair is two smooth 30-degree collision ramps. This pass keeps
those proven walk surfaces, paints a tread rhythm over them with non-colliding
strips, gives each landing a comb transition, and aims emergency light along
the travel. Run only with the editor closed. It refuses a second application.
"""

import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_ESC_DETAIL_"
CUBE = "/Engine/BasicShapes/Cube.Cube"
STEEL = "/Game/Props/Boswells/Materials/steel"
OAT = "/Game/Props/Boswells/Materials/oat"


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_ESCALATORS " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {a.get_actor_label(): a for a in sub.get_all_level_actors()}
require(len(before) == len(sub.get_all_level_actors()), "duplicate actor labels")
require(not any(label.startswith(PREFIX) for label in before),
        "escalator pass already present")
for direction in ("ASCEND", "DESCEND"):
    for suffix in ("", "_FOOT", "_HEAD", "_RAIL_W", "_RAIL_E"):
        require("ROUTE_ESCALATOR_" + direction + suffix in before,
                "missing original escalator: " + direction + suffix)
mesh = unreal.EditorAssetLibrary.load_asset(CUBE)
materials = {"steel": unreal.EditorAssetLibrary.load_asset(STEEL),
             "oat": unreal.EditorAssetLibrary.load_asset(OAT)}
require(isinstance(mesh, unreal.StaticMesh), "engine cube absent")
require(all(isinstance(m, unreal.MaterialInterface) for m in materials.values()),
        "tread materials absent")


def piece(label, x, y, z, width, length, height, material, roll=0):
    actor = sub.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, z),
        unreal.Rotator(roll, 0, 0))
    require(actor is not None, "spawn failed: " + label)
    actor.set_actor_label(PREFIX + label)
    actor.set_folder_path("Boswells/Atrium/EscalatorDetail")
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(mesh), "mesh failed: " + label)
    actor.set_actor_scale3d(unreal.Vector(width / 100, length / 100,
                                          height / 100))
    comp.set_material(0, materials[material])
    actor.set_actor_enable_collision(False)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    comp.set_editor_property("can_ever_affect_navigation", False)
    return actor


def route_spot(label, x, y, z):
    actor = sub.spawn_actor_from_class(
        unreal.SpotLight, unreal.Vector(x, y, z),
        unreal.Rotator(pitch=-90, yaw=0, roll=0))
    require(actor is not None, "light spawn failed: " + label)
    actor.set_actor_label(PREFIX + label)
    actor.set_folder_path("Boswells/Atrium/EscalatorDetail")
    light = actor.spot_light_component
    light.set_mobility(unreal.ComponentMobility.MOVABLE)
    light.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    light.set_editor_property("intensity", 8.0)
    light.set_editor_property("attenuation_radius", 500.0)
    light.set_editor_property("inner_cone_angle", 28.0)
    light.set_editor_property("outer_cone_angle", 45.0)
    light.set_editor_property("source_radius", 35.0)
    light.set_editor_property("indirect_lighting_intensity", 0.25)
    light.set_editor_property("cast_shadows", False)
    return actor


# Keep the ramps as the only walk collision. Individual moving steps would
# catch the capsule and make the transition worse. These stationary escalators
# carry a tread rhythm and two flat entrance/exit comb zones instead.
for direction, x, slope, roll, low_sign in (
        ("ASCEND", -250, 0.577350269, -30, -1),
        ("DESCEND", 250, -0.577350269, 30, 1)):
    for index in range(39):
        y = -551 + index * 29
        surface_z = 344 + slope * y
        piece("%s_TREAD_%02d" % (direction, index),
              x, y, surface_z + 1.2, 166, 3, 1.2, "steel", roll)
    low_y = -650 if low_sign < 0 else 650
    high_y = -low_y
    for index, offset in enumerate((-18, 0, 18)):
        piece("%s_COMB_LOW_%d" % (direction, index),
              x, low_y + offset, 1.4, 170, 5, 1.4, "oat")
        piece("%s_COMB_HIGH_%d" % (direction, index),
              x, high_y + offset, 664, 170, 5, 1.4, "oat")
    for index, y in enumerate((-420, 0, 420)):
        # Aiming at the floor makes the route readable without lighting every
        # adjacent shelf label. They are presentation lights, never registered
        # with the gameplay light field.
        route_spot("%s_ROUTE_%d" % (direction, index),
                   x, y, 344 + slope * y + 170)


require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "save failed")
after = {a.get_actor_label(): a for a in sub.get_all_level_actors()}
require(sum(label.startswith(PREFIX) for label in after) == 96,
        "saved detail inventory differs")
print("BOSWELLS_ESCALATORS SAVED: 78 visual treads, 12 flat comb bars, "
      "6 aimed emergency spots; original ramp collision retained")

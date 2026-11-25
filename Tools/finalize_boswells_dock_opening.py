"""Tune the dock reveal and verify capsule clearance through the crossing."""

import math

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "LLK_DOCK_CHAPTER_"


def require(ok, reason):
    if not ok:
        raise RuntimeError("DOCK_OPENING_FINAL " + reason)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = {actor.get_actor_label(): actor for actor in sub.get_all_level_actors()}
require(PREFIX + "FAR_RECEIVING" in actors, "dock chapter not built")

# The lower placeholder blocks the east service doorway. The authored
# upstairs courtesy counter is the department location in the store plan.
require("BSW_SERVICE_CORE_COURTESY_COUNTER" in actors, "upper courtesy counter missing")
desk = actors["DEPT_SERVICE_DESK"]
desk.set_actor_hidden_in_game(True)
desk.set_actor_enable_collision(False)
desk.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
desk.static_mesh_component.set_editor_property("can_ever_affect_navigation", False)

# The old perfume run closed the new staff door. Relocate the intact case
# group to the end of its own department, leaving the original promenade.
for label, actor in actors.items():
    if label.startswith("BSW_ARRIVAL_PERFUME_3_"):
        location = actor.get_actor_location()
        if location.x > -3900:
            location.x -= 920
            require(actor.set_actor_location(location, False, False), "could not relocate " + label)


def clear_walk(name, path, floor_z=0):
    for index, ((x0, y0), (x1, y1)) in enumerate(zip(path, path[1:])):
        start = unreal.Vector(x0, y0, 96)
        end = unreal.Vector(x1, y1, 96)
        hit = unreal.SystemLibrary.capsule_trace_single(
            world, start, end, 34.0, 68.0, unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,
            False, [], unreal.DrawDebugTrace.NONE, True)
        if hit:
            blocked = hit.to_dict()["hit_actor"]
            require(False, f"{name} segment {index} blocked by {blocked.get_actor_label()}")
        sample_count = max(1, math.ceil(math.hypot(x1 - x0, y1 - y0) / 90))
        for i in range(sample_count + 1):
            alpha = i / sample_count
            x, y = x0 + (x1 - x0) * alpha, y0 + (y1 - y0) * alpha
            hit = unreal.SystemLibrary.line_trace_single(
                world, unreal.Vector(x, y, 70), unreal.Vector(x, y, -45),
                unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [],
                unreal.DrawDebugTrace.NONE, True)
            require(hit and abs(hit.to_dict()["impact_point"].z - floor_z) <= 24,
                    f"{name} has a floor gap at {x:.0f}, {y:.0f}")
    unreal.log_warning("DOCK_OPENING_FINAL clear: " + name)


clear_walk("dock arrival to loading mouth", ((400, 3340), (276, 3060), (25, 2750), (0, 2550)))
clear_walk("west stock door", ((-2930, 2340), (-3200, 2340), (-3200, 1940)))
clear_walk("east sales door", ((2910, 2340), (3200, 2340), (3200, 1940)))
clear_walk("staff exit into west mall shop", (
    (-3150, -2300), (-3150, -4500), (-2135, -4500), (-1460, -4390), (-680, -4130)))
clear_walk("customer concourse beyond the grille", ((0, -2780), (0, -3220)))
clear_walk("long mall route into far receiving", (
    (0, -7800), (200, -10560), (-1410, -11200), (-1410, -14100),
    (-650, -14500), (-650, -19190), (-2460, -19190), (-2460, -21410),
    (-7470, -21410), (-7470, -25400), (-7470, -33250),
    (-6175, -33880), (-6175, -34630)))
clear_walk("far wing public bypass", ((-5470, -26200), (-5470, -32450), (-6175, -33880)))

# Retain contrast in the dock and put a low pool inside the far anchor so
# the player can read the threshold and the tracks stopping at its back door.
actors[PREFIX + "DOCK_WITNESS"].spot_light_component.set_editor_property("intensity", 28.0)
actors[PREFIX + "DOCK_APPROACH"].spot_light_component.set_editor_property("intensity", 25.0)


def pool(name, position, power, radius):
    label = PREFIX + name
    if label in actors:
        return
    lamp = sub.spawn_actor_from_class(
        unreal.SpotLight, unreal.Vector(*position), unreal.Rotator(pitch=-90))
    require(lamp is not None, "spot spawn failed")
    lamp.set_actor_label(label)
    lamp.set_folder_path("Galleria/Crossing/Receiving")
    light = lamp.spot_light_component
    light.set_mobility(unreal.ComponentMobility.MOVABLE)
    light.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    light.set_editor_property("intensity", power)
    light.set_editor_property("attenuation_radius", radius)
    light.set_editor_property("inner_cone_angle", 20.0)
    light.set_editor_property("outer_cone_angle", 42.0)
    light.set_editor_property("light_color", unreal.Color(213, 201, 177, 255))


pool("FAR_THRESHOLD_POOL", (-6175, -34200, 380), 26, 650)
pool("FAR_INSIDE_POOL", (-6160, -34840, 310), 18, 540)

cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
steel = unreal.EditorAssetLibrary.load_asset("/Game/Props/Boswells/Materials/steel")
cloth = unreal.EditorAssetLibrary.load_asset("/Game/LLK/Boswells/Fixtures/burgundy")
require(cube and steel and cloth, "receiving detail kit missing")


def detail(name, position, dimensions, material, solid=False, yaw=0):
    label = PREFIX + name
    if label in actors:
        return
    actor = sub.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(*position), unreal.Rotator(yaw=yaw))
    require(actor is not None, "detail spawn failed")
    actor.set_actor_label(label)
    actor.set_folder_path("Galleria/Crossing/Receiving")
    mesh = actor.static_mesh_component
    require(mesh.set_static_mesh(cube), "detail mesh failed")
    mesh.set_material(0, material)
    actor.set_actor_scale3d(unreal.Vector(*(axis / 100 for axis in dimensions)))
    actor.set_actor_enable_collision(solid)
    mesh.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS
                               if solid else unreal.CollisionEnabled.NO_COLLISION)
    mesh.set_editor_property("can_ever_affect_navigation", solid)


detail("FAR_CLOSED_DOOR", (-6175, -35072, 150), (510, 36, 300), steel, True)
for index, x in enumerate((-6530, -5800)):
    detail(f"FAR_BUMPER_{index}", (x, -34990, 56), (60, 82, 112), steel, True)
detail("TRAIL_CLOTH_AT_DESTINATION", (-6250, -34905, 1.8), (46, 22, 3), cloth, yaw=24)
for index, position in enumerate(((180, 2940, 22.9), (-2510, 2380, 1.1), (-3210, 1870, 1.1),
                                  (-4110, 1410, 1.1), (-4060, 210, 1.1), (-4110, -1080, 1.1))):
    detail(f"STORE_TRAIL_{index}", position, (26, 70, 1.5), cloth, yaw=index * 7)
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "save failed")
unreal.log_warning("DOCK_OPENING_FINAL saved dock and receiving light; all walk probes passed")

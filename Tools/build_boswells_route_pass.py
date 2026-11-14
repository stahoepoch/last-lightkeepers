"""Create an isolated, walkable Boswell's route plate from the Anchor map.

Run with Tools/run_editor_python.ps1 while Unreal Editor is closed. The
source map is never saved. This produces /Game/LLK_Boswells_RoutePass once;
it refuses to overwrite an existing route pass.

This is architectural blockout: a frontage, a dock threshold, a two-door
service strip, and two opposed 30-degree runs in the existing atrium void.
It is not the art pass, the mall concourse, or a collision/navmesh playtest.
"""

import unreal

SOURCE = "/Game/LLK_Boswells_Anchor"
TARGET = "/Game/LLK_Boswells_RoutePass"
CUBE = "/Engine/BasicShapes/Cube.Cube"


def log(message):
    unreal.log("BOSWELLS_ROUTE " + message)
    print("BOSWELLS_ROUTE " + message)


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def actor_map():
    actors = unreal.get_editor_subsystem(
        unreal.EditorActorSubsystem).get_all_level_actors()
    return {actor.get_actor_label(): actor for actor in actors}


def box(name, x, y, z, width, length, height, roll=0.0):
    actor = unreal.get_editor_subsystem(
        unreal.EditorActorSubsystem).spawn_actor_from_class(
            unreal.StaticMeshActor, unreal.Vector(x, y, z),
            # Unreal Python's positional Rotator order is roll, pitch, yaw.
            # The third argument would yaw this deck flat in the XY plane.
            unreal.Rotator(roll, 0.0, 0.0))
    require(actor is not None, "could not spawn " + name)
    actor.set_actor_label(name)
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(unreal.load_asset(CUBE)),
            "could not assign cube to " + name)
    actor.set_actor_scale3d(unreal.Vector(width / 100.0,
                                          length / 100.0,
                                          height / 100.0))
    return actor


def span(name, xmin, xmax, ymin, ymax, bottom, top):
    return box(name, (xmin + xmax) / 2.0, (ymin + ymax) / 2.0,
               (bottom + top) / 2.0, xmax - xmin, ymax - ymin, top - bottom)


def remove(actor):
    unreal.get_editor_subsystem(unreal.EditorActorSubsystem).destroy_actor(actor)


def build():
    require(unreal.EditorAssetLibrary.does_asset_exist(SOURCE),
            "source map missing; nothing changed")
    require(not unreal.EditorAssetLibrary.does_asset_exist(TARGET),
            "route pass already exists; refusing to overwrite it")
    result = unreal.EditorAssetLibrary.duplicate_asset(SOURCE, TARGET)
    require(result is not None, "map duplication failed")
    world = unreal.EditorLoadingAndSavingUtils.load_map(TARGET)
    require(world is not None and world.get_name() == "LLK_Boswells_RoutePass",
            "did not load the duplicate map")
    old = actor_map()
    required = ("ANCHOR_FULL_WALL_SOUTH", "ANCHOR_FULL_WALL_NORTH",
                "ANCHOR_UPPER_SOUTH", "ANCHOR_UPPER_NORTH",
                "ANCHOR_UPPER_WEST", "ANCHOR_UPPER_EAST",
                "ANCHOR_PLAYERSTART_ENTRANCE", "ANCHOR_LOADING_DOCK_EXIT")
    require(all(label in old for label in required),
            "duplicate is missing expected shell actors")
    log("loaded duplicate with {} actors".format(len(old)))

    # Front grille: x +/-900 opens the solid south shell. Bars keep it shut
    # in the story while the player starts inside. A 280 cm header clears a
    # 176 cm body. These are blockout bars, not authored art.
    remove(old["ANCHOR_FULL_WALL_SOUTH"])
    span("ROUTE_SOUTH_WALL_W", -4370, -900, -2720, -2680, 0, 1500)
    span("ROUTE_SOUTH_WALL_E", 900, 4370, -2720, -2680, 0, 1500)
    span("ROUTE_SOUTH_HEADER", -900, 900, -2720, -2680, 280, 1500)
    # 30 cm clear between bars, narrower than the measured 68 cm Sweeper.
    # The keeper capsule still needs an in-editor collision check.
    for index, x in enumerate(range(-880, 881, 40)):
        box("ROUTE_MALL_GRILLE_{:02d}".format(index), x, -2700, 140,
            10, 16, 280)

    # The north dock opens to a guarded receiving pad so the route does not
    # end in another fall off the map. The existing LoadingDock zone stays.
    remove(old["ANCHOR_FULL_WALL_NORTH"])
    if "ANCHOR_LOADING_DOCK_DOOR" in old:
        remove(old["ANCHOR_LOADING_DOCK_DOOR"])
    # This old 5.5 m placeholder reaches across y=2100 and would physically
    # intersect the new service wall. Its replacement belongs to the art pass.
    if "DEPT_MENS" in old:
        remove(old["DEPT_MENS"])
    span("ROUTE_NORTH_WALL_W", -4370, -400, 2680, 2720, 0, 1500)
    span("ROUTE_NORTH_WALL_E", 400, 4370, 2680, 2720, 0, 1500)
    span("ROUTE_DOCK_HEADER", -400, 400, 2680, 2720, 400, 1500)
    span("ROUTE_DOCK_RECEIVING_PAD", -600, 600, 2680, 3300, -30, 0)
    span("ROUTE_DOCK_GUARD_W", -600, -585, 2720, 3300, 0, 115)
    span("ROUTE_DOCK_GUARD_E", 585, 600, 2720, 3300, 0, 115)
    span("ROUTE_DOCK_GUARD_N", -600, 600, 3285, 3300, 0, 115)

    # The back-of-house threshold follows audit B5: two 400 cm doors in a
    # 20 cm wall at y=2100. The centre is intentionally closed; reaching the
    # dock requires reading the store and turning through a service door.
    span("ROUTE_SERVICE_WALL_W", -4350, -3400, 2090, 2110, 0, 637.5)
    span("ROUTE_SERVICE_WALL_M", -3000, 3000, 2090, 2110, 0, 637.5)
    span("ROUTE_SERVICE_WALL_E", 3400, 4350, 2090, 2110, 0, 637.5)
    for name, left, right in (("W", -3400, -3000), ("E", 3000, 3400)):
        span("ROUTE_SERVICE_HEADER_" + name, left, right, 2090, 2110,
             300, 637.5)

    # Existing upper slabs leave a 1200 x 1800 cm void. A tilted 1325 cm
    # cube at 30 degrees rises 662.5 cm over 1147.5 cm horizontal. Separate
    # landings bridge the last 326 cm from each top to the upper slab edge.
    # Unreal positive roll drops +Y (confirmed with MathLibrary right vector);
    # ASCEND therefore takes -30 and DESCEND takes +30.
    slope_length = 1325.0
    for name, x, roll, bottom_y, top_y in (
            ("ASCEND", -250, -30.0, -696, 737),
            ("DESCEND", 250, 30.0, 696, -737)):
        box("ROUTE_ESCALATOR_" + name, x, 0, 331.25,
            180, slope_length, 22, roll)
        low_a, low_b = sorted((bottom_y - 122, bottom_y + 122))
        high_a, high_b = sorted((top_y - 163, top_y + 163))
        span("ROUTE_ESCALATOR_" + name + "_FOOT", x - 110, x + 110,
             low_a, low_b, -20, 0)
        span("ROUTE_ESCALATOR_" + name + "_HEAD", x - 110, x + 110,
             high_a, high_b, 642.5, 662.5)
        for side, rail_x in (("W", x - 105), ("E", x + 105)):
            box("ROUTE_ESCALATOR_{}_RAIL_{}".format(name, side),
                rail_x, 0, 392, 12, slope_length, 120, roll)

    # Guard the upper atrium edges. Leave one 300 cm landing opening at
    # y=+900 for ASCEND and one at y=-900 for DESCEND.
    span("ROUTE_ATRIUM_GUARD_W", -620, -600, -900, 900, 662.5, 780)
    span("ROUTE_ATRIUM_GUARD_E", 600, 620, -900, 900, 662.5, 780)
    span("ROUTE_ATRIUM_GUARD_S_W", -600, 100, -920, -900, 662.5, 780)
    span("ROUTE_ATRIUM_GUARD_S_E", 400, 600, -920, -900, 662.5, 780)
    span("ROUTE_ATRIUM_GUARD_N_W", -600, -400, 900, 920, 662.5, 780)
    span("ROUTE_ATRIUM_GUARD_N_E", -100, 600, 900, 920, 662.5, 780)

    require(unreal.EditorLoadingAndSavingUtils.save_map(world, TARGET),
            "duplicate modified in memory but save failed")
    labels = actor_map()
    expected_new = ("ROUTE_SOUTH_HEADER", "ROUTE_DOCK_RECEIVING_PAD",
                    "ROUTE_SERVICE_WALL_M", "ROUTE_ESCALATOR_ASCEND",
                    "ROUTE_ESCALATOR_DESCEND", "ROUTE_ATRIUM_GUARD_W")
    require(all(label in labels for label in expected_new),
            "saved map missing route actors")
    log("SAVED {} with {} actors; source map was not saved".format(
        TARGET, len(labels)))


build()

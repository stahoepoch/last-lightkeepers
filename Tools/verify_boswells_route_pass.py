"""Headless geometry checks for the isolated Boswell's route map.

Run: Tools/run_editor_python.ps1 -Script Tools/verify_boswells_route_pass.py
The checks query saved collision in Unreal. They do not replace a PIE walk or
prove navmesh connectivity.
"""

import os

import unreal

MAP = os.environ.get("LLK_BOSWELLS_MAP", "/Game/LLK_Boswells_RoutePass")
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if world is None or world.get_name() != MAP.rsplit("/", 1)[-1]:
    raise RuntimeError("could not load the saved route map")


def trace(start, end):
    hit = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(*start), unreal.Vector(*end),
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, True, [],
        unreal.DrawDebugTrace.NONE, True)
    return hit.to_dict() if hit else None


def expect_floor(name, x, y, expected_actor, expected_z, tolerance=4):
    result = trace((x, y, 1100), (x, y, -100))
    if not result:
        raise RuntimeError("{} has no floor collision".format(name))
    actor = result["hit_actor"]
    label = actor.get_actor_label() if actor else "None"
    z = result["location"].z
    if label != expected_actor or abs(z - expected_z) > tolerance:
        raise RuntimeError("{} hit {} at z {:.1f}; expected {} at {:.1f}".format(
            name, label, z, expected_actor, expected_z))
    unreal.log("ROUTE_VERIFY {}: {} z {:.1f}".format(name, label, z))


def expect_horizontal(name, x, start_y, end_y, z, expected_actor):
    result = trace((x, start_y, z), (x, end_y, z))
    actor = result["hit_actor"] if result else None
    label = actor.get_actor_label() if actor else "None"
    if label != expected_actor:
        raise RuntimeError("{} hit {}; expected {}".format(
            name, label, expected_actor))
    unreal.log("ROUTE_VERIFY {}: {}".format(name, label))


for y, z in ((-500, 55), (0, 344), (500, 633)):
    expect_floor("ascent {}".format(y), -250, y,
                 "ROUTE_ESCALATOR_ASCEND", z)
for y, z in ((-500, 633), (0, 344), (500, 55)):
    expect_floor("descent {}".format(y), 250, y,
                 "ROUTE_ESCALATOR_DESCEND", z)
expect_floor("upper landing", -250, 700, "ROUTE_ESCALATOR_ASCEND_HEAD", 662.5)
expect_floor("receiving pad", 0, 3000, "ROUTE_DOCK_RECEIVING_PAD", 0)
expect_horizontal("service wall", 0, 2000, 2200, 100,
                  "ROUTE_SERVICE_WALL_M")
expect_horizontal("west service door", -3200, 2000, 2200, 100, "None")
expect_horizontal("east service door", 3200, 2000, 2200, 100, "None")
expect_horizontal("dock opening", 0, 2500, 2900, 100, "None")
expect_horizontal("closed mall grille", 0, -2600, -2800, 100,
                  "ROUTE_MALL_GRILLE_22")

unreal.log("ROUTE_VERIFY PASSED: 13 saved-collision checks; PIE/nav still required")

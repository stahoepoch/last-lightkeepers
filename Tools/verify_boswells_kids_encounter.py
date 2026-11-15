"""Verify the saved upper-floor signage and hide/patrol integration."""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_ApparelPass":
    raise RuntimeError("BOSWELLS_ENCOUNTER wrong map")
actors = {a.get_actor_label(): a for a in
          unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()}


def require(ok, message):
    if not ok:
        raise RuntimeError("BOSWELLS_ENCOUNTER " + message)


signs = {n: a for n, a in actors.items() if n.startswith("BSW_KIDS_SIGN_")}
cover = {n: a for n, a in actors.items() if n.startswith("BSW_KIDS_COVER_")}
patrol = {n: a for n, a in actors.items() if n.startswith("BSW_KIDS_PATROL_")}
require(len(signs) == 20, "sign inventory changed")
require(len(cover) == 10, "cover inventory changed")
require(len(patrol) == 3 and "BSW_KIDS_SWEEPER" in actors,
        "upper encounter incomplete")
require("ANCHOR_SWEEPER_CONCOURSE" not in actors,
        "lower Sweeper would join the global upper patrol")
require("ANCHOR_NAV_BOUNDS_FULL_PLATE" in actors and
        "RecastNavMesh-Default" in actors, "navigation actors absent")
for name, actor in signs.items():
    require(not actor.get_actor_enable_collision(), "sign blocks route: " + name)
for name, actor in cover.items():
    if name.endswith("_BASE") or name.endswith("_TOP"):
        require(actor.get_actor_enable_collision(),
                "solid cover lost collision: " + name)
    else:
        require(not actor.get_actor_enable_collision(),
                "folded visual blocks route: " + name)
    center, extent = actor.get_actor_bounds(False)
    require(center.x - extent.x > -3950 and center.x + extent.x < -1060,
            "cover enters bypass: " + name)
    require((center.y - extent.y > -2050 and center.y + extent.y < -1450)
            or (center.y - extent.y > -1150 and center.y + extent.y < -150),
            "cover enters a cross aisle: " + name)

for name, actor in patrol.items():
    require(unreal.Name("LLK.Patrol") in actor.get_editor_property("tags"),
            "upper patrol lost tag: " + name)
    require(abs(actor.get_actor_location().z - 750.5) < 1,
            "upper patrol on wrong floor: " + name)
for index in (1, 2, 3):
    lower = actors["ANCHOR_PATROL_" + str(index)]
    require(unreal.Name("LLK.Patrol") not in lower.get_editor_property("tags"),
            "global patrol would send upper Sweeper downstairs")

# Same line-of-sight question the enemy asks: ECC_Visibility. At these two
# display slots the crouched centre must hit; the standing centre must clear.
# A control through the reserved cross aisle must also clear.
for y in (-1650, -500):
    for height, expected_hit in ((40, True), (88, False)):
        result = unreal.SystemLibrary.line_trace_single(
            world, unreal.Vector(-2400, y, 662.5 + height),
            unreal.Vector(-1900, y, 662.5 + height),
            unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,
            False, [], unreal.DrawDebugTrace.NONE, True)
        require((result is not None) == expected_hit,
                "cover visibility wrong at y={} height={}".format(y, height))
control = unreal.SystemLibrary.line_trace_single(
    world, unreal.Vector(-2400, -1300, 702.5),
    unreal.Vector(-1900, -1300, 702.5),
    unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,
    False, [], unreal.DrawDebugTrace.NONE, True)
require(control is None, "reserved cross aisle is blocked")

message = "BOSWELLS_ENCOUNTER PASSED: 20 signs, 2 crouch covers, 3 upper patrol points, 1 Sweeper, clear cross aisle"
unreal.log(message)
print(message)

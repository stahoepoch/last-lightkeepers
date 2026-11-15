"""Add a first upper-floor hide-and-detour encounter to ApparelPass.

Run with Tools/run_editor_python.ps1 while Unreal Editor is closed. The two
stocked displays have an opaque low base: crouching behind one actually blocks
the sight trace, while the upper aisle remains passable. One Sweeper patrols
only the upper route; the old lower markers remain for a later floor-local
patrol implementation but are temporarily untagged in this map.
"""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
PREFIX = "BSW_KIDS_COVER_"
PATROL_PREFIX = "BSW_KIDS_PATROL_"
FLOOR_Z = 662.5
RACK_X = -2150
RACK_YS = (-1650, -500)


def require(ok, message):
    if not ok:
        raise RuntimeError("BOSWELLS_STEALTH " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_ApparelPass", "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {a.get_actor_label(): a for a in subsystem.get_all_level_actors()}
require(not any(name.startswith(PREFIX) or name.startswith(PATROL_PREFIX)
                for name in before), "upper encounter already exists")
require("BSW_KIDS_SWEEPER" not in before, "Sweeper already placed")
require("ANCHOR_SWEEPER_CONCOURSE" in before,
        "lower Sweeper inventory changed; review patrol scope")
for name in ("ANCHOR_NAV_BOUNDS_FULL_PLATE", "RecastNavMesh-Default",
             "BSW_KIDS_CEILING_SALES_COLUMN", "BSW_KIDS_SALES_DESK",
             "ROUTE_ESCALATOR_ASCEND", "ROUTE_ESCALATOR_DESCEND"):
    require(name in before, "required actor absent: " + name)
for index in (1, 2, 3):
    actor = before.get("ANCHOR_PATROL_" + str(index))
    require(actor and unreal.Name("LLK.Patrol") in actor.get_editor_property("tags"),
            "lower patrol changed; review before editing")
for y in RACK_YS:
    require("BSW_KIDS_ROLLING_RACK_{}_{}".format(RACK_X, y) in before,
            "rack to replace absent at y=" + str(y))

cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
fixture_root = "/Game/LLK/Boswells/Fixtures/"
cream = unreal.EditorAssetLibrary.load_asset(fixture_root + "cream")
oat = unreal.EditorAssetLibrary.load_asset(fixture_root + "oat")
rose = unreal.EditorAssetLibrary.load_asset(fixture_root + "rose")
mint = unreal.EditorAssetLibrary.load_asset(fixture_root + "mint")
lavender = unreal.EditorAssetLibrary.load_asset(fixture_root + "lavender")
require(isinstance(cube, unreal.StaticMesh), "engine cube missing")
require(all(isinstance(m, unreal.MaterialInterface)
            for m in (cream, oat, rose, mint, lavender)),
        "fixture material missing")
require(hasattr(unreal, "LLKSweeper"), "LLKSweeper C++ class unavailable")


def block(label, x, y, z, width, length, height, material, collision):
    actor = subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator(0, 0, 0))
    require(actor is not None, "block spawn failed: " + label)
    actor.set_actor_label(PREFIX + label)
    actor.set_folder_path("Boswells/Children/Cover")
    actor.set_actor_scale3d(unreal.Vector(width / 100, length / 100,
                                          height / 100))
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(cube), "block mesh failed: " + label)
    comp.set_material(0, material)
    actor.set_actor_enable_collision(collision)
    comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS
                               if collision else unreal.CollisionEnabled.NO_COLLISION)
    if hasattr(comp, "set_can_ever_affect_navigation"):
        comp.set_can_ever_affect_navigation(collision)
    return actor


# Replace one see-through rack in each island with a visibly solid display.
# 72 cm solid plinth + 6 cm top + 4 cm folded goods = 82 cm maximum. A
# crouched centre at ~40 cm is screened, a standing centre at ~88 cm is not.
for y, side in ((-1650, "SOUTH"), (-500, "NORTH")):
    rack = before["BSW_KIDS_ROLLING_RACK_{}_{}".format(RACK_X, y)]
    require(subsystem.destroy_actor(rack), "could not replace rack " + side)
    block(side + "_BASE", RACK_X, y, FLOOR_Z + 36,
          140, 70, 72, cream, True)
    block(side + "_TOP", RACK_X, y, FLOOR_Z + 75,
          148, 78, 6, oat, True)
    for index, (offset, mat) in enumerate(((-42, rose), (0, mint), (42, lavender))):
        block(side + "_FOLDED_" + str(index), RACK_X + offset, y,
              FLOOR_Z + 80, 32, 50, 4, mat, False)

# LLKSweeper currently reads all LLK.Patrol tags in the whole UWorld. The
# three old points are downstairs. Preserve the points, but reserve their
# tag for a later floor-local code change before enabling two patrols here.
# The old lower Sweeper is removed from this map for the same reason: in the
# first live simulation it climbed upstairs and doubled the encounter. Its
# pre-pass copy is kept in the committed prior map and Saved/Temp backup.
require(subsystem.destroy_actor(before["ANCHOR_SWEEPER_CONCOURSE"]),
        "could not defer lower Sweeper")
for index in (1, 2, 3):
    actor = before["ANCHOR_PATROL_" + str(index)]
    actor.set_editor_property("tags", [unreal.Name("LLK.Patrol.Lower")])

# An L-shaped work route crosses the 300 cm aisle between the two carpet
# islands, then watches the east bypass. Both outer and atrium detours stay
# open, and the machine begins away from the escalator landing.
for index, (x, y) in enumerate(((-1750, -1300), (-850, -1300),
                               (-850, -250)), start=1):
    point = subsystem.spawn_actor_from_class(
        unreal.TargetPoint, unreal.Vector(x, y, FLOOR_Z + 88),
        unreal.Rotator(0, 0, 0))
    require(point is not None, "patrol point spawn failed " + str(index))
    point.set_actor_label(PATROL_PREFIX + str(index))
    point.set_folder_path("Boswells/Children/Encounter")
    point.set_editor_property("tags", [unreal.Name("LLK.Patrol")])

sweeper = subsystem.spawn_actor_from_class(
    unreal.LLKSweeper, unreal.Vector(-850, -1300, FLOOR_Z + 88),
    unreal.Rotator(0, 0, 0))
require(sweeper is not None, "Sweeper spawn failed")
sweeper.set_actor_label("BSW_KIDS_SWEEPER")
sweeper.set_folder_path("Boswells/Children/Encounter")

after = {a.get_actor_label(): a for a in subsystem.get_all_level_actors()}
require(sum(name.startswith(PREFIX) for name in after) == 10,
        "display actor count differs")
require(sum(name.startswith(PATROL_PREFIX) for name in after) == 3,
        "patrol count differs")
require("BSW_KIDS_SWEEPER" in after, "Sweeper absent before save")
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP),
        "map save failed")
message = "BOSWELLS_STEALTH SAVED: 2 solid displays, 3 upper patrol points, 1 Sweeper"
unreal.log(message)
print(message)

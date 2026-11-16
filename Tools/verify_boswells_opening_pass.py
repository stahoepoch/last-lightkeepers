"""Check the saved opening map after loading it in an independent process.

This verifies placements and two real sightline traces, not the capture scene,
pickup interaction, or grille motion. Those require runtime and a play pass.
"""

import configparser
import math
from pathlib import Path

import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"


def require(ok, message):
    if not ok:
        raise RuntimeError("BOSWELLS_OPENING_VERIFY " + message)


def close_to(actual, expected, tolerance=1.0):
    return all(abs(a - b) <= tolerance for a, b in zip(actual, expected))


def xyz(vector):
    return vector.x, vector.y, vector.z


settings = configparser.ConfigParser()
settings.read(Path(unreal.Paths.project_dir()) / "Config/DefaultEngine.ini",
              encoding="utf-8-sig")
for key in ("EditorStartupMap", "GameDefaultMap"):
    require(settings.get("/Script/EngineSettings.GameMapsSettings", key,
                         fallback="") == MAP,
            key + " does not open the playable opening map")


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "saved opening map did not load")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actor_list = subsystem.get_all_level_actors()
actors = {actor.get_actor_label(): actor for actor in actor_list}
require(sum(actor.get_actor_label().startswith("BSW_OPENING_LANTERN")
            for actor in actor_list) == 1,
        "pickup missing or duplicated")
start = actors.get("ANCHOR_PLAYERSTART_ENTRANCE")
pickup = actors.get("BSW_OPENING_LANTERN")
counter = actors.get("BSW_ARRIVAL_CHECKOUT_EAST_0")
require(isinstance(start, unreal.PlayerStart), "PlayerStart missing")
require(isinstance(pickup, unreal.LLKLanternPickup),
        "world pickup has the wrong class")
require(counter is not None, "entrance counter missing")
require(close_to(xyz(start.get_actor_location()), (1600.0, -2370.0, 73.0)),
        "PlayerStart moved")
start_rotation = start.get_actor_rotation()
require(abs(start_rotation.pitch) < 0.1 and
        abs(start_rotation.yaw + 162.0) < 0.1 and
        abs(start_rotation.roll) < 0.1,
        "PlayerStart must face the release without pitching the pawn")
require(close_to(xyz(pickup.get_actor_location()), (1450.0, -2260.0, 9.0)),
        "pickup moved")
require(150.0 < math.dist((1600.0, -2370.0), (1450.0, -2260.0)) < 200.0,
        "lantern is no longer a step out of cover")
require(not pickup.is_taken(), "pickup starts taken")
body = pickup.get_component_by_class(unreal.StaticMeshComponent)
require(body is not None and body.get_editor_property("static_mesh") is not None,
        "floor lantern has no visible placeholder")
require(body.get_collision_enabled() == unreal.CollisionEnabled.NO_COLLISION,
        "floor lantern blocks the escape path")
require(sum(name.startswith("ROUTE_MALL_GRILLE_") for name in actors) == 45,
        "closed grille bar inventory changed")
require(all(name in actors for name in (
    "BSW_KIDS_COVER_BAY_COLLISION", "BSW_KIDS_COVER_WEST_TABLE_COLLISION",
    "ROUTE_ESCALATOR_ASCEND", "MALL_GLASS_ENTRANCE")),
    "verified cover or opening architecture did not carry through")


def hit_label(start_xyz, end_xyz):
    hit = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(*start_xyz), unreal.Vector(*end_xyz),
        unreal.TraceTypeQuery.ECC_VISIBILITY, True, [],
        unreal.DrawDebugTrace.NONE, True)
    actor = hit.to_dict()["hit_actor"] if hit else None
    return actor.get_actor_label() if actor else None


# From behind the counter, the machine's straight approach is hidden at the
# boy's low eye. Through the aisle-end gap, the closed grille remains visible.
require(hit_label((1600, -2370, 33), (1600, -2630, 33)) ==
        "BSW_ARRIVAL_CHECKOUT_EAST_0", "counter does not shield crouched child")
grille_view_hit = hit_label((1600, -2370, 33), (0, -2700, 33))
unreal.log("BOSWELLS_OPENING_VERIFY grille view hit: " + str(grille_view_hit))
require(grille_view_hit == "ROUTE_MALL_GRILLE_23",
        "grille is not visible through the counter gap")
require(hit_label((1600, -2370, 125), (780, -2635, 168)) is None,
        "first-view release hardware sightline is blocked")

message = ("BOSWELLS_OPENING_VERIFY PASSED: saved pickup and start, "
           "closed grille, inherited cover, blocked approach and open view")
unreal.log(message)
print(message)

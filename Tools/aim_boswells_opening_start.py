"""Aim the opening camera at its clear release-hardware sightline.

The prior -90 degree start showed a dark wall while the authored release was
72 degrees left of frame. This changes the saved PlayerStart only after a
Visibility trace proves the release is in view from the child's 125 cm eye.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_OPENING_AIM " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
a = {x.get_actor_label(): x for x in
     unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()}
start = a.get("ANCHOR_PLAYERSTART_ENTRANCE")
witness = a.get("BSW_OPENING_SCENE_RELEASE_WITNESS")
require(isinstance(start, unreal.PlayerStart) and witness is not None,
        "opening scene actors missing")
loc = start.get_actor_location()
require(abs(loc.x - 1600) < 1 and abs(loc.y + 2370) < 1 and
        abs(start.get_actor_rotation().yaw + 90) < 0.1,
        "start changed since original staging")
origin = unreal.Vector(loc.x, loc.y, 125)
hit = unreal.SystemLibrary.line_trace_single(
    world, origin, witness.get_actor_location(),
    unreal.TraceTypeQuery.ECC_VISIBILITY, True, [],
    unreal.DrawDebugTrace.NONE, True)
require(not hit or not hit.to_dict().get("hit_actor"),
        "release sightline obstructed; do not turn blind")
rot = unreal.Rotator()
rot.yaw = -162
require(start.set_actor_rotation(rot, False), "rotation failed")
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
print("BOSWELLS_OPENING_AIM SAVED: first view aims at clear release sightline")

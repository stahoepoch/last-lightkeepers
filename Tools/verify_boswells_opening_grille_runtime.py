"""Fresh-process check of the saved authored side of the opening interaction."""

import unreal


def require(ok, message):
    if not ok:
        raise RuntimeError("BOSWELLS_OPENING_GRILLE_RUNTIME_VERIFY " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(
    "/Game/LLK_Boswells_OpeningPass")
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong saved map")
actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()
controllers = [a for a in actors if isinstance(a, unreal.LLKOpeningGrille)]
require(len(controllers) == 1 and
        controllers[0].get_actor_label() == "BSW_OPENING_GRILLE_CONTROLLER",
        "controller count or identity wrong")


def tagged(tag):
    return [a for a in actors if tag in [str(t) for t in a.get_editor_property("tags")]]


require(len(tagged("LLK.Opening.LanternRead")) == 1,
        "witness missing")
require(len(tagged("LLK.Opening.GrilleRelease")) == 1,
        "release missing")
segments = tagged("LLK.Opening.GrilleSegment")
require(len(segments) == 45, "grille segment count changed")
require(all(a.static_mesh_component.get_editor_property("mobility") ==
            unreal.ComponentMobility.MOVABLE for a in segments),
        "a grille segment cannot move")

witness = tagged("LLK.Opening.LanternRead")[0]
read_point = witness.get_actor_location()
read_hit = unreal.SystemLibrary.line_trace_single(
    world, unreal.Vector(780, -2470, 125), read_point,
    unreal.TraceTypeQuery.ECC_VISIBILITY, True, [],
    unreal.DrawDebugTrace.NONE, True)
read_actor = read_hit.to_dict()["hit_actor"] if read_hit else None
require(read_actor is None or read_actor == witness,
        "release witness is hidden behind blocking geometry")
# A keeper facing south at the release has the authored stable hand mount
# 42 cm forward, 25 cm right and 19 cm below the 125 cm eye.
beam_hit = unreal.SystemLibrary.line_trace_single(
    world, unreal.Vector(805, -2512, 106), read_point,
    unreal.TraceTypeQuery.ECC_VISIBILITY, True, [],
    unreal.DrawDebugTrace.NONE, True)
beam_actor = beam_hit.to_dict()["hit_actor"] if beam_hit else None
require(beam_actor is None or beam_actor == witness,
        "the stable hand mount cannot shine on the witness")


def hit_label():
    hit = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(0, -2500, 125), unreal.Vector(0, -2850, 125),
        unreal.TraceTypeQuery.ECC_VISIBILITY, True, [],
        unreal.DrawDebugTrace.NONE, True)
    actor = hit.to_dict()["hit_actor"] if hit else None
    return actor.get_actor_label() if actor else None


require(hit_label() == "ROUTE_MALL_GRILLE_22",
        "closed grille does not stop the child")
# Move the exact saved actors as the runtime controller will, then restore
# without saving. This probes the player-height crossing, not just mobility.
closed = [a.get_actor_location() for a in segments]
try:
    for actor, point in zip(segments, closed):
        actor.set_actor_location(point + unreal.Vector(0, 0, 330), False, False)
    require(hit_label() is None,
            "raised grille still blocks the child-height crossing")
finally:
    for actor, point in zip(segments, closed):
        actor.set_actor_location(point, False, False)

message = ("BOSWELLS_OPENING_GRILLE_RUNTIME_VERIFY PASSED: saved controller, "
           "read point, 45 movable bars, closed and raised crossing traces")
unreal.log(message)
print(message)

"""Verify the saved opening grille dressing in an independent Unreal process.

This proves placement, collision and selected sightlines. It cannot prove
grille motion, the capture beat, lighting read or player understanding.
"""

import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_OPENING_SCENE_"


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_OPENING_GRILLE_VERIFY " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "saved opening map did not load")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actor_list = subsystem.get_all_level_actors()
actors = {actor.get_actor_label(): actor for actor in actor_list}
scene = [actor for actor in actor_list
         if actor.get_actor_label().startswith(PREFIX)]
require(len(scene) == 27, "scene dressing count differs")
require(len(actors) == len(actor_list), "duplicate editor labels hide actors")
for actor in scene:
    require(isinstance(actor, unreal.StaticMeshActor),
            "non-mesh scene dressing: " + actor.get_actor_label())
    comp = actor.static_mesh_component
    require(comp.get_collision_enabled() == unreal.CollisionEnabled.NO_COLLISION,
            "scene dressing blocks the route: " + actor.get_actor_label())
    require(not comp.get_editor_property("can_ever_affect_navigation"),
            "scene dressing carves navigation: " + actor.get_actor_label())

for name in ("COUNTER_SAMPLE_TRAY", "COUNTER_BOTTLE_00",
             "COUNTER_JEWELRY_PAD", "RELEASE_BAR", "RELEASE_LINK",
             "RELEASE_HEADER_ROLLER",
             "RELEASE_SHADE", "RELEASE_WITNESS", "COAT_SLEEVE",
             "COAT_CUFF", "DRAG_MAIN_04", "DRAG_LOST_02"):
    require(PREFIX + name in actors, "scene landmark missing: " + name)
require(sum(name.startswith("ROUTE_MALL_GRILLE_") for name in actors) == 45,
        "the existing closed grille changed")


def hit_label(a, b):
    hit = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(*a), unreal.Vector(*b),
        unreal.TraceTypeQuery.ECC_VISIBILITY, True, [],
        unreal.DrawDebugTrace.NONE, True)
    actor = hit.to_dict()["hit_actor"] if hit else None
    return actor.get_actor_label() if actor else None


# The child can still see the release from the counter's aisle-end gap, and
# approach it; the grille still physically ends the route until runtime moves.
require(hit_label((1600, -2370, 125), (780, -2608, 111)) is None,
        "counter-to-release view is blocked")
require(hit_label((780, -2470, 125), (780, -2608, 111)) is None,
        "approach to the bar is blocked")
require(hit_label((0, -2500, 125), (0, -2850, 125)) ==
        "ROUTE_MALL_GRILLE_22", "south grille no longer stops the player")
require(hit_label((1600, -2370, 33), (1600, -2630, 33)) ==
        "BSW_ARRIVAL_CHECKOUT_EAST_0", "the opening counter lost its cover")

message = ("BOSWELLS_OPENING_GRILLE_VERIFY PASSED: 27 non-colliding scene "
           "pieces, visible bar, cover and closed grille")
unreal.log(message)
print(message)

"""Read-only structural inventory for the left-after-close dressing pass."""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_LEFTDAY_"


def require(condition, message):
    if not condition:
        raise RuntimeError("BOSWELLS_LEFTDAY_VERIFY " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None, "map unavailable")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
by_label = {actor.get_actor_label(): actor for actor in actors}
new = [actor for actor in actors if actor.get_actor_label().startswith(PREFIX)]
require(len(new) >= 150, "detail inventory too low")
for actor in new:
    require(isinstance(actor, unreal.StaticMeshActor), "unexpected detail actor")
    comp = actor.static_mesh_component
    require(comp.get_collision_enabled() == unreal.CollisionEnabled.NO_COLLISION and
            not comp.get_editor_property("can_ever_affect_navigation"),
            "closing detail blocks a route: " + actor.get_actor_label())
require(sum("APPAREL_RACK_" in actor.get_actor_label() for actor in new) == 12,
        "apparel rhythm changed")
require(sum("SERVICE_COUNTER_" in actor.get_actor_label() for actor in new) == 4,
        "service cluster changed")
require(sum("DOCK_ROLLING_RACK_" in actor.get_actor_label() for actor in new) == 6,
        "dock density changed")
for name in ("BSW_OPENING_LANTERN", "ROUTE_ESCALATOR_ASCEND",
             "ROUTE_ESCALATOR_DESCEND", "BSW_REALITY_DOCK_FLOOR"):
    require(name in by_label, "existing route actor lost: " + name)
print("BOSWELLS_LEFTDAY_VERIFY PASSED: %d non-blocking, closing-shift details; "
      "existing route and opening actors retained" % len(new))

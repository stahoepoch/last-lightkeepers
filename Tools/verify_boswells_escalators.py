"""Reload the opening map and check that escalator dressing did not break travel."""

import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_ESC_DETAIL_"


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_ESCALATORS_VERIFY " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "saved opening map did not load")
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = sub.get_all_level_actors()
by_label = {a.get_actor_label(): a for a in actors}
require(len(actors) == len(by_label), "duplicate labels")
detail = [a for a in actors if a.get_actor_label().startswith(PREFIX)]
require(len(detail) == 96, "detail count differs")
require(sum("_TREAD_" in a.get_actor_label() for a in detail) == 78,
        "tread count differs")
require(sum("_COMB_" in a.get_actor_label() for a in detail) == 12,
        "comb count differs")
require(sum("_ROUTE_" in a.get_actor_label() for a in detail) == 6,
        "route light count differs")
for actor in detail:
    if isinstance(actor, unreal.StaticMeshActor):
        require(actor.static_mesh_component.get_collision_enabled() ==
                unreal.CollisionEnabled.NO_COLLISION,
                "visual strip blocks the capsule: " + actor.get_actor_label())
    else:
        require(isinstance(actor, unreal.SpotLight),
                "unexpected detail actor: " + actor.get_actor_label())
        light = actor.spot_light_component
        require(light.get_editor_property("mobility") ==
                unreal.ComponentMobility.MOVABLE,
                "route light is baked-only")
        require(abs(light.get_editor_property("intensity") - 8) < 0.01,
                "route light intensity drifted")


def floor_at(x, y, expected_label):
    hit = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(x, y, 1100), unreal.Vector(x, y, -100),
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, True, [],
        unreal.DrawDebugTrace.NONE, True)
    require(hit is not None, "no walk floor at %s, %s" % (x, y))
    label = hit.to_dict()["hit_actor"].get_actor_label()
    require(label == expected_label,
            "walk surface at %s, %s is %s, not %s" %
            (x, y, label, expected_label))


for y in (-500, 0, 500):
    floor_at(-250, y, "ROUTE_ESCALATOR_ASCEND")
    floor_at(250, y, "ROUTE_ESCALATOR_DESCEND")
floor_at(-250, 700, "ROUTE_ESCALATOR_ASCEND_HEAD")
floor_at(250, -700, "ROUTE_ESCALATOR_DESCEND_HEAD")
require("BSW_OPENING_LANTERN" in by_label and
        "ROUTE_MALL_GRILLE_22" in by_label,
        "opening beat disappeared")
print("BOSWELLS_ESCALATORS_VERIFY PASSED: 96 unique details, visual-only "
      "treads/comb, 6 dynamic route lights, 8 original collision surfaces")

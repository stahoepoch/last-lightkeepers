"""Verify saved Boswell's stock wings, collision and lantern-release seams."""

import sys
from pathlib import Path

import unreal


sys.path.insert(0, str(Path(unreal.Paths.project_dir()) / "Tools"))
import plan_boswells_store as plan  # noqa: E402

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_CROWD_"


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_APPAREL_DENSITY_VERIFY " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "saved opening map did not load")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = subsystem.get_all_level_actors()
by_label = {actor.get_actor_label(): actor for actor in actors}
stock = [actor for actor in actors if actor.get_actor_label().startswith(PREFIX)]
require(len(stock) == 480, "saved stock inventory differs")
require(len({actor.get_actor_label() for actor in stock}) == 480,
        "duplicate stock labels")

bodies = []
for actor in stock:
    label = actor.get_actor_label()
    require(isinstance(actor, unreal.StaticMeshActor), "non-mesh stock: " + label)
    comp = actor.static_mesh_component
    solid = label.endswith("_BODY")
    require(comp.get_collision_enabled() ==
            (unreal.CollisionEnabled.QUERY_AND_PHYSICS if solid
             else unreal.CollisionEnabled.NO_COLLISION),
            "stock collision differs: " + label)
    require(comp.get_editor_property("can_ever_affect_navigation") == solid,
            "stock navigation differs: " + label)
    if solid:
        bodies.append(actor)
require(len(bodies) == 48, "structural body count differs")

for actor in bodies:
    label = actor.get_actor_label()
    center, extent = actor.get_actor_bounds(False)
    rect = plan.Rect(center.x - extent.x, center.y - extent.y,
                     center.x + extent.x, center.y + extent.y)
    require(not any(rect.overlaps(route) for route in plan.ROUTES.values()),
            "saved stock blocks a public route: " + label)
    require(center.z - extent.z >= -1 and center.z + extent.z <= 171,
            "stock collision leaves the lower floor: " + label)

for label, tag in (("BSW_OPENING_SCENE_RELEASE_WITNESS",
                    "LLK.Opening.LanternRead"),
                   ("BSW_OPENING_SCENE_RELEASE_BAR",
                    "LLK.Opening.GrilleRelease")):
    require(label in by_label, "release actor missing: " + label)
    require(tag in [str(t) for t in by_label[label].get_editor_property("tags")],
            "release runtime tag missing: " + tag)
grille = [actor for name, actor in by_label.items()
          if name.startswith("ROUTE_MALL_GRILLE_")]
require(len(grille) == 45 and all(
    "LLK.Opening.GrilleSegment" in
    [str(t) for t in actor.get_editor_property("tags")]
    for actor in grille), "grille segment tags differ")
for label in ("BSW_OPENING_LANTERN", "ANCHOR_PLAYERSTART_ENTRANCE",
              "ROUTE_ESCALATOR_ASCEND", "ROUTE_ESCALATOR_DESCEND"):
    require(label in by_label, "opening or vertical route missing: " + label)


def hit_name(a, b):
    hit = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(*a), unreal.Vector(*b),
        unreal.TraceTypeQuery.ECC_VISIBILITY, True, [],
        unreal.DrawDebugTrace.NONE, True)
    actor = hit.to_dict()["hit_actor"] if hit else None
    return actor.get_actor_label() if actor else None


hit = hit_name((-3300, -650, 125), (-3190, -650, 125))
require(hit is not None and hit.startswith(PREFIX) and hit.endswith("_BODY"),
        "stock wing does not block child-eye trace: " + str(hit))

message = ("BOSWELLS_APPAREL_DENSITY_VERIFY PASSED: 480 saved pieces, "
           "48 route-clear collision bodies, child-eye cover and release tags")
unreal.log(message)
print(message)

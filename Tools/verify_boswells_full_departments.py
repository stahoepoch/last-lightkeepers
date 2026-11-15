"""Check saved Boswell's department density against sockets and collisions.

Run in a fresh Unreal commandlet process after the dressing pass. This checks
real component bounds, which can differ from the authored dimensions. It is
not a substitute for a keeper/Sweeper walk or an art review.
"""

import csv
from pathlib import Path

import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_DEPARTMENT_"
KINDS = {"shoe_display", "shoe_bench", "bed_vignette", "home_display",
         "feature_table", "toy_gondola", "sofa_vignette", "appliance_pod"}
CSV = Path(unreal.Paths.project_dir()) / "docs/reference/boswells/BoswellsFullStore_Sockets.csv"


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_DEPARTMENTS_VERIFY " + why)


with CSV.open(newline="", encoding="utf-8") as file:
    rows = [row for row in csv.DictReader(file) if row["kind"] in KINDS]
require(len(rows) == 48, "socket plan changed")
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "saved map did not load")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actor_list = subsystem.get_all_level_actors()
actors = {actor.get_actor_label(): actor for actor in actor_list}
stock = [actor for actor in actor_list
         if actor.get_actor_label().startswith(PREFIX)]
require(len(stock) == 450, "department actor inventory differs")
require(len({actor.get_actor_label() for actor in stock}) == 450,
        "duplicate department labels hide an actor")
solids = 0
socket_counts = [0] * 48
for actor in stock:
    label = actor.get_actor_label()
    require(isinstance(actor, unreal.StaticMeshActor),
            "non-mesh department actor: " + label)
    tail = label[len(PREFIX):]
    index = int(tail[:2])
    require(0 <= index < len(rows) and tail[2] == "_",
            "invalid socket number: " + label)
    row = rows[index]
    require(tail[3:].startswith(row["kind"].upper() + "_"),
            "actor landed in wrong kind of socket: " + label)
    socket_counts[index] += 1
    xmin, ymin, xmax, ymax = (float(row[key]) for key in
                              ("xmin_cm", "ymin_cm", "xmax_cm", "ymax_cm"))
    center, extent = actor.get_actor_bounds(False)
    require(center.x - extent.x >= xmin - 2 and
            center.x + extent.x <= xmax + 2 and
            center.y - extent.y >= ymin - 2 and
            center.y + extent.y <= ymax + 2,
            "saved mesh overhangs its socket: " + label)
    comp = actor.static_mesh_component
    collision = comp.get_collision_enabled()
    nav = comp.get_editor_property("can_ever_affect_navigation")
    require(nav == (collision == unreal.CollisionEnabled.QUERY_AND_PHYSICS),
            "navigation and collision disagree: " + label)
    require(collision in (unreal.CollisionEnabled.NO_COLLISION,
                          unreal.CollisionEnabled.QUERY_AND_PHYSICS),
            "unexpected collision mode: " + label)
    solids += int(collision == unreal.CollisionEnabled.QUERY_AND_PHYSICS)

require(all(n >= 4 for n in socket_counts),
        "at least one socket is insufficiently dressed")
require(solids == 93, "structural collision inventory differs")
for label in ("ANCHOR_PLAYERSTART_ENTRANCE", "BSW_OPENING_LANTERN",
              "BSW_OPENING_SCENE_RELEASE_BAR", "ROUTE_ESCALATOR_ASCEND",
              "ROUTE_ESCALATOR_DESCEND", "BSW_KIDS_SWEEPER",
              "ANCHOR_LOADING_DOCK_EXIT"):
    require(label in actors, "opening or traversal actor disappeared: " + label)
require(sum(name.startswith("ROUTE_MALL_GRILLE_") for name in actors) == 45,
        "mall grille bars changed")

message = ("BOSWELLS_DEPARTMENTS_VERIFY PASSED: 48 furnished sockets, "
           "450 saved pieces within bounds, 93 navigation bodies; "
           "opening and vertical route actors retained")
unreal.log(message)
print(message)

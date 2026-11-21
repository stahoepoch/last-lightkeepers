"""Check saved modeled departments against sockets and original collisions.

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
require(len(stock) == 141, "department actor inventory differs")
require(len({actor.get_actor_label() for actor in stock}) == 141,
        "duplicate department labels hide an actor")
solids = 0
visuals = 0
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
    solid = collision == unreal.CollisionEnabled.QUERY_AND_PHYSICS
    solids += int(solid)
    if solid:
        require(not comp.get_editor_property("visible"),
                "original structural proxy became visible: " + label)
        require(comp.get_editor_property("static_mesh").get_path_name().startswith(
                "/Engine/BasicShapes/"),
                "original structural mesh changed: " + label)
    else:
        require(label.endswith("_AUTHORED_VISUAL") and not nav and
                not actor.get_actor_enable_collision(),
                "old cuboid decoration or colliding visual: " + label)
        mesh = comp.get_editor_property("static_mesh")
        require(mesh and mesh.get_path_name().startswith(
            "/Game/LLK/Boswells/Fixtures/SM_BSW_Department_"),
            "visual is not an authored fixture: " + label)
        require(len(mesh.get_editor_property("static_materials")) >= 3 and
                all(slot.get_editor_property("material_interface") is not None
                    for slot in mesh.get_editor_property("static_materials")),
                "authored fixture has an empty material slot: " + label)
        visuals += 1

require(all(n >= 2 for n in socket_counts),
        "at least one socket lost its structure or visual")
require(solids == 93, "structural collision inventory differs")
require(visuals == 48, "authored fixture inventory differs")
for index, row in enumerate(rows):
    group = f"BSW_DEPARTMENT_{index:02d}_{row['kind'].upper()}"
    require(group + "_AUTHORED_VISUAL" in actors,
            "socket has no authored fixture: " + row["name"])
for label in ("ANCHOR_PLAYERSTART_ENTRANCE", "BSW_OPENING_LANTERN",
              "BSW_OPENING_SCENE_RELEASE_BAR", "ROUTE_ESCALATOR_ASCEND",
              "ROUTE_ESCALATOR_DESCEND", "BSW_KIDS_SWEEPER",
              "ANCHOR_LOADING_DOCK_EXIT"):
    require(label in actors, "opening or traversal actor disappeared: " + label)
require(sum(name.startswith("ROUTE_MALL_GRILLE_") for name in actors) == 45,
        "mall grille bars changed")


def hit_label(start, end):
    hit = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(*start), unreal.Vector(*end),
        unreal.TraceTypeQuery.ECC_VISIBILITY, True, [],
        unreal.DrawDebugTrace.NONE, True)
    actor = hit.to_dict()["hit_actor"] if hit else None
    return actor.get_actor_label() if actor else None


# Two hideable silhouettes and two open alternate paths. A hidden structural
# body must keep blocking sight, while a modeled visual cannot seal the aisle.
require(hit_label((1100, 650, 70), (1400, 650, 70)) ==
        "BSW_DEPARTMENT_00_SHOE_DISPLAY_CABINET",
        "lower shoe cover lost its collision trace")
require(hit_label((-3900, 650, 695.5), (-3400, 650, 695.5)) ==
        "BSW_DEPARTMENT_12_BED_VIGNETTE_FRAME",
        "upper bed no longer covers a crouched child")
require(hit_label((850, 350, 70), (850, 950, 70)) is None,
        "lower atrium bypass is obstructed")
require(hit_label((1050, 1250, 730), (1450, 1250, 730)) is None,
        "upper cross aisle is obstructed")

message = ("BOSWELLS_DEPARTMENTS_VERIFY PASSED: 48 furnished sockets, "
           "48 modeled visuals within bounds, 93 hidden navigation bodies; "
           "two cover and two bypass traces; opening route retained")
unreal.log(message)
print(message)

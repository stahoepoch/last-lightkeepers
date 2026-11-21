"""Replace 357 visible department primitives with 48 authored fixtures.

The 93 original structural actors remain at identical transforms, with their
collision and navigation relevance unchanged. They become invisible proxies;
one non-colliding modeled fixture fills each of the 48 plan sockets. This is
a one-shot OpeningPass edit. Run with the editor closed and a clean map file.
"""

import csv
from pathlib import Path

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
CSV = Path(unreal.Paths.project_dir()) / "docs/reference/boswells/BoswellsFullStore_Sockets.csv"
ROOT = "/Game/LLK/Boswells/Fixtures/"
KINDS = {"shoe_display", "shoe_bench", "bed_vignette", "home_display",
         "feature_table", "toy_gondola", "sofa_vignette", "appliance_pod"}


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_DEPARTMENT_REPLACE " + why)


with CSV.open(newline="", encoding="utf-8") as file:
    rows = [r for r in csv.DictReader(file) if r["kind"] in KINDS]
require(len(rows) == 48, "socket plan changed")
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
all_actors = sub.get_all_level_actors()
by_label = {a.get_actor_label(): a for a in all_actors}
require(len(by_label) == len(all_actors), "duplicate actor labels")
old = {name: a for name, a in by_label.items()
       if name.startswith("BSW_DEPARTMENT_")}
require(len(old) == 450 and not any(
    name.endswith("_AUTHORED_VISUAL") for name in old),
    "expected original department inventory")
for label in ("ANCHOR_PLAYERSTART_ENTRANCE", "BSW_OPENING_LANTERN",
              "BSW_OPENING_SCENE_RELEASE_BAR", "ROUTE_ESCALATOR_ASCEND",
              "ROUTE_ESCALATOR_DESCEND", "BSW_KIDS_SWEEPER"):
    require(label in by_label, "opening or vertical actor missing: " + label)

groups = []
all_names = set()
solid_total = visual_total = 0
for i, row in enumerate(rows):
    kind = row["kind"]
    group = f"BSW_DEPARTMENT_{i:02d}_{kind.upper()}"
    members = {name: a for name, a in old.items()
               if name.startswith(group + "_")}
    require(members and not any(name in all_names for name in members),
            "missing or overlapping group: " + group)
    all_names.update(members)
    xmin, ymin, xmax, ymax = (float(row[key]) for key in
                              ("xmin_cm", "ymin_cm", "xmax_cm", "ymax_cm"))
    cx, cy = (xmin + xmax) / 2, (ymin + ymax) / 2
    floor = float(row["floor_z_cm"])
    variant = "A" if i % 2 == 0 else "B"
    if kind in ("shoe_display", "shoe_bench"):
        size = "Long" if ymax - ymin > 300 else "Short"
        stem = ("ShoeDisplay" if kind == "shoe_display" else "ShoeBench") + "_" + size
    else:
        stem = {"bed_vignette": "BedVignette", "home_display": "HomeDisplay",
                "feature_table": "FeatureTable", "toy_gondola": "ToyGondola",
                "sofa_vignette": "SofaVignette",
                "appliance_pod": "AppliancePod"}[kind]
    mesh = unreal.EditorAssetLibrary.load_asset(
        ROOT + f"SM_BSW_Department_{stem}_{variant}")
    require(isinstance(mesh, unreal.StaticMesh), "authored mesh absent: " + group)
    bounds = mesh.get_bounds()
    require(cx + bounds.origin.x - bounds.box_extent.x >= xmin - 1 and
            cx + bounds.origin.x + bounds.box_extent.x <= xmax + 1 and
            cy + bounds.origin.y - bounds.box_extent.y >= ymin - 1 and
            cy + bounds.origin.y + bounds.box_extent.y <= ymax + 1 and
            bounds.origin.z - bounds.box_extent.z >= -1,
            "visual mesh exceeds socket: " + group)
    solids = []
    decorations = []
    for name, actor in members.items():
        require(isinstance(actor, unreal.StaticMeshActor),
                "non-mesh original: " + name)
        comp = actor.static_mesh_component
        collision = comp.get_collision_enabled()
        require(comp.get_editor_property("can_ever_affect_navigation") ==
                (collision == unreal.CollisionEnabled.QUERY_AND_PHYSICS),
                "old nav differs: " + name)
        require(collision in (unreal.CollisionEnabled.NO_COLLISION,
                              unreal.CollisionEnabled.QUERY_AND_PHYSICS),
                "old collision differs: " + name)
        (solids if collision == unreal.CollisionEnabled.QUERY_AND_PHYSICS
         else decorations).append(actor)
    require(solids and decorations, "fixture missing structure or stock: " + group)
    solid_total += len(solids)
    visual_total += len(decorations)
    groups.append((row, group, cx, cy, floor, mesh, solids, decorations))

require(len(all_names) == 450 and solid_total == 93 and visual_total == 357,
        "preflight did not account for all original actors")

for row, group, cx, cy, floor, mesh, solids, decorations in groups:
    for actor in decorations:
        label = actor.get_actor_label()
        require(sub.destroy_actor(actor),
                "could not remove old decoration: " + label)
    for actor in solids:
        actor.static_mesh_component.set_visibility(False, True)
    visual = sub.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(cx, cy, floor), unreal.Rotator())
    require(visual is not None, "visual actor spawn failed: " + group)
    visual.set_actor_label(group + "_AUTHORED_VISUAL")
    visual.set_folder_path("Boswells/Departments/" + row["floor"] +
                           "/" + row["department"])
    comp = visual.static_mesh_component
    require(comp.set_static_mesh(mesh), "mesh assignment failed: " + group)
    visual.set_actor_enable_collision(False)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    comp.set_editor_property("can_ever_affect_navigation", False)

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
print("BOSWELLS_DEPARTMENT_REPLACE SAVED: 48 modeled retail fixtures, "
      "357 cuboid decorations removed, 93 unchanged collision bodies hidden")

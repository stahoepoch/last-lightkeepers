"""Give the opening map stocked apparel edges without narrowing its escape routes.

The original 36 fixture sockets occupy little of the 87 x 54 m floor. These
short, continuous merchandise wings attach to the authored racks rather than
planting obstacles in the public aisles. Every wing has visible stock and an
equal-sized collision volume; the collision is hidden so the individual pieces
can supply the silhouette. This is a one-shot map edit, run with the editor
closed. It also tags the existing release surfaces for the runtime owner.
"""

import csv
import sys
from pathlib import Path

import unreal


PROJECT = Path(unreal.Paths.project_dir())
sys.path.insert(0, str(PROJECT / "Tools"))
import plan_boswells_store as plan  # noqa: E402

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_CROWD_"
CSV = PROJECT / "docs/reference/boswells/BoswellsFullStore_Sockets.csv"
MATERIALS = "/Game/LLK/Boswells/Fixtures/"


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_APPAREL_DENSITY " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {a.get_actor_label(): a for a in subsystem.get_all_level_actors()}
require(not any(label.startswith(PREFIX) for label in before),
        "density pass already exists")
with CSV.open(newline="", encoding="utf-8") as file:
    all_rows = list(csv.DictReader(file))
rows = [r for r in all_rows if r["floor"] == "lower" and
        r["department"] in ("juniors", "ladies", "mens")]
require(len(rows) == 36, "apparel plan changed")
for row in rows:
    require("BSW_APPAREL_" + row["name"] in before,
            "base rack missing: " + row["name"])
for label in ("BSW_OPENING_SCENE_RELEASE_WITNESS",
              "BSW_OPENING_SCENE_RELEASE_BAR"):
    require(label in before, "release landmark missing: " + label)
grille = [actor for label, actor in before.items()
          if label.startswith("ROUTE_MALL_GRILLE_")]
require(len(grille) == 45, "grille segment inventory changed")

cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
palette = {name: unreal.EditorAssetLibrary.load_asset(MATERIALS + name)
           for name in ("steel", "charcoal", "navy", "denim", "oat",
                        "rose", "burgundy", "cream")}
require(isinstance(cube, unreal.StaticMesh), "engine cube missing")
require(all(isinstance(mat, unreal.MaterialInterface)
            for mat in palette.values()), "stock palette missing")


def box(row):
    return plan.Rect(*(float(row[key]) for key in
                       ("xmin_cm", "ymin_cm", "xmax_cm", "ymax_cm")))


def make(label, x, y, z, width, length, height, material,
         solid=False, hidden=False):
    actor = subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator())
    require(actor is not None, "spawn failed: " + label)
    actor.set_actor_label(PREFIX + label)
    actor.set_folder_path("Boswells/Departments/lower/StockedApparel")
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(cube), "mesh assignment failed: " + label)
    actor.set_actor_scale3d(unreal.Vector(width / 100, length / 100,
                                          height / 100))
    comp.set_material(0, palette[material])
    actor.set_actor_enable_collision(solid)
    comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS
                               if solid else unreal.CollisionEnabled.NO_COLLISION)
    comp.set_editor_property("can_ever_affect_navigation", solid)
    if hidden:
        comp.set_visibility(False)
    return actor


# Keep the outer public routes and the atrium bypass open. A wing extends a
# rack by 45 cm; even when two wings face each other across a 500 cm column
# spacing, the remaining local aisle is 230 cm wide. The 400 cm inner pair
# gets no facing wings. The public route reservations stay 300 cm clear.
columns = (-3650, -3150, -2650, -2150, -1650, -1250,
           1250, 1650, 2150, 2650, 3150, 3650)
sides = {
    -3650: (1,), -3150: (-1, 1), -2650: (-1, 1),
    -2150: (-1, 1), -1650: (-1,), -1250: (),
    1250: (), 1650: (1,), 2150: (-1, 1),
    2650: (-1, 1), 3150: (-1, 1), 3650: (-1,),
}
require(tuple(sides) == columns, "apparel column order changed")
wing_boxes = []
for row in rows:
    original = box(row)
    x = (original.x0 + original.x1) / 2
    y = (original.y0 + original.y1) / 2
    require(x in sides and abs(original.x1 - original.x0 - 180) < 1,
            "unexpected apparel socket width: " + row["name"])
    for side in sides[x]:
        wing = plan.Rect(x + side * 90 if side > 0 else x - 135,
                         y - (300 if row["kind"] == "clothing_run" else 75),
                         x + 135 if side > 0 else x - 90,
                         y + (300 if row["kind"] == "clothing_run" else 75))
        require(not any(wing.overlaps(route) for route in plan.ROUTES.values()),
                "wing enters reserved public route: " + row["name"])
        require(not any(wing.overlaps(box(other)) for other in all_rows
                        if other is not row and other["floor"] == "lower"),
                "wing overlaps another socket: " + row["name"])
        wing_boxes.append((row, side, wing))
require(len(wing_boxes) == 48, "expected 48 safe stocked wings")
for i, (source_a, _, a) in enumerate(wing_boxes):
    for source_b, _, b in wing_boxes[i + 1:]:
        if source_a is source_b:
            continue  # the rack fills the intentional gap between its wings
        if a.y0 < b.y1 and b.y0 < a.y1:
            gap = b.x0 - a.x1 if a.x1 <= b.x0 else a.x0 - b.x1
            require(gap >= 200,
                    "opposing stock narrows a side aisle below 200 cm")


created = []
for index, (row, side, wing) in enumerate(wing_boxes):
    cx = (wing.x0 + wing.x1) / 2
    cy = (wing.y0 + wing.y1) / 2
    length = wing.y1 - wing.y0
    code = "%02d_%s" % (index, "W" if side < 0 else "E")
    created.append(make(code + "_BODY", cx, cy, 85, 45, length, 170,
                        "charcoal", solid=True, hidden=True))
    created.append(make(code + "_BASE", cx, cy, 9, 45, length, 18,
                        "charcoal"))
    created.append(make(code + "_RAIL", cx, cy, 168, 11, length, 6,
                        "steel"))
    for end, ey in enumerate((wing.y0 + 6, wing.y1 - 6)):
        created.append(make(code + "_POST_%d" % end, cx, ey, 85,
                            10, 10, 170, "steel"))
    count = 8 if length > 300 else 2
    for segment in range(count):
        py = wing.y0 + (segment + 0.5) * length / count
        height = (94, 112, 103, 116)[(index + segment) % 4]
        colors = ("navy", "denim", "oat", "cream") if row["department"] == "mens" \
            else ("rose", "burgundy", "denim", "oat")
        created.append(make(code + "_GARMENT_%02d" % segment, cx, py,
                            164 - height / 2, 43, length / count - 4,
                            height, colors[(index + segment) % 4]))

require(len(created) == 480, "stock piece count changed")
require(sum(a.static_mesh_component.get_collision_enabled() ==
            unreal.CollisionEnabled.QUERY_AND_PHYSICS for a in created) == 48,
        "stock collision inventory changed")

# Stable runtime names are actor tags rather than editor-only outliner labels.
tagged = ((before["BSW_OPENING_SCENE_RELEASE_WITNESS"],
           "LLK.Opening.LanternRead"),
          (before["BSW_OPENING_SCENE_RELEASE_BAR"],
           "LLK.Opening.GrilleRelease"))
for actor, tag in tagged:
    tags = list(actor.get_editor_property("tags"))
    require(not any(str(old) == tag for old in tags), "tag already present: " + tag)
    actor.set_editor_property("tags", tags + [unreal.Name(tag)])
for actor in grille:
    tags = list(actor.get_editor_property("tags"))
    require(not any(str(old) == "LLK.Opening.GrilleSegment" for old in tags),
            "grille already tagged")
    actor.set_editor_property("tags", tags +
                              [unreal.Name("LLK.Opening.GrilleSegment")])

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "save failed")
message = ("BOSWELLS_APPAREL_DENSITY SAVED: 48 stocked wings, 480 pieces, "
           "48 collision bodies, release and grille tagged")
unreal.log(message)
print(message)

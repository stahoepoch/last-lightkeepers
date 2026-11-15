"""Fill the empty authored store sockets in LLK_Boswells_OpeningPass.

The socket plan protects the 300 cm public routes. This pass adds distinct
shoe, bedding, home, toy, furniture and appliance silhouettes inside 48
otherwise empty sockets. Structural bodies collide; merchandise and trim do
not carve navigation. Run headlessly with the editor closed. One-shot only.
"""

import csv
from pathlib import Path

import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
CSV = Path(unreal.Paths.project_dir()) / "docs/reference/boswells/BoswellsFullStore_Sockets.csv"
PREFIX = "BSW_DEPARTMENT_"
KINDS = {"shoe_display", "shoe_bench", "bed_vignette", "home_display",
         "feature_table", "toy_gondola", "sofa_vignette", "appliance_pod"}


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_DEPARTMENTS " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
old = {actor.get_actor_label(): actor
       for actor in subsystem.get_all_level_actors()}
require(not any(name.startswith(PREFIX) for name in old),
        "department pass already exists")
for name in ("ANCHOR_PLAYERSTART_ENTRANCE", "BSW_OPENING_LANTERN",
             "ROUTE_ESCALATOR_ASCEND", "ROUTE_ESCALATOR_DESCEND",
             "BSW_KIDS_SALES_DESK", "BSW_OPENING_SCENE_RELEASE_BAR"):
    require(name in old, "required route or opening actor absent: " + name)

with CSV.open(newline="", encoding="utf-8") as file:
    rows = [row for row in csv.DictReader(file) if row["kind"] in KINDS]
require(len(rows) == 48, "expected 48 empty department sockets")
require(len({row["name"] for row in rows}) == 48,
        "department socket names are duplicated")

cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
cylinder = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cylinder.Cylinder")
root = "/Game/LLK/Boswells/Fixtures/"
names = ("steel", "white_steel", "cream", "dark_wood", "denim", "rose",
         "butter", "mint", "oat", "navy", "charcoal", "burgundy")
colors = {name: unreal.EditorAssetLibrary.load_asset(root + name)
          for name in names}
require(isinstance(cube, unreal.StaticMesh) and
        isinstance(cylinder, unreal.StaticMesh), "engine primitive mesh missing")
require(all(isinstance(mat, unreal.MaterialInterface)
            for mat in colors.values()), "fixture palette incomplete")

pieces = 0
solids = 0


def make(row, group, name, dx, dy, z, width, length, height,
         material, solid=False, shape="cube", roll=0):
    global pieces, solids
    xmin, ymin, xmax, ymax = (float(row[key]) for key in
                              ("xmin_cm", "ymin_cm", "xmax_cm", "ymax_cm"))
    x = (xmin + xmax) / 2 + dx
    y = (ymin + ymax) / 2 + dy
    floor = float(row["floor_z_cm"])
    require(x - width / 2 >= xmin - 1 and x + width / 2 <= xmax + 1 and
            y - length / 2 >= ymin - 1 and y + length / 2 <= ymax + 1,
            "piece overhangs socket: " + row["name"] + "/" + name)
    rotation = unreal.Rotator()
    rotation.roll = roll
    actor = subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, floor + z), rotation)
    require(actor is not None, "spawn failed: " + group + "/" + name)
    actor.set_actor_label(PREFIX + group + "_" + name)
    actor.set_folder_path("Boswells/Departments/" + row["floor"] +
                          "/" + row["department"])
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(cube if shape == "cube" else cylinder),
            "mesh assignment failed: " + name)
    actor.set_actor_scale3d(unreal.Vector(width / 100, length / 100,
                                          height / 100))
    comp.set_material(0, colors[material])
    actor.set_actor_enable_collision(solid)
    comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS
                               if solid else unreal.CollisionEnabled.NO_COLLISION)
    comp.set_editor_property("can_ever_affect_navigation", solid)
    pieces += 1
    solids += int(solid)
    return actor


for index, row in enumerate(rows):
    kind = row["kind"]
    group = "%02d_%s" % (index, kind.upper())
    socket_length = float(row["ymax_cm"]) - float(row["ymin_cm"])

    if kind == "shoe_display":
        long_run = socket_length > 300
        make(row, group, "CABINET", 0, 0, 43,
             150 if long_run else 180, 480 if long_run else 140, 86,
             "dark_wood", solid=True)
        for tier in (87, 119):
            make(row, group, "SHELF_%d" % tier, 0, 0, tier,
                 158 if long_run else 190, 500 if long_run else 150,
                 5, "steel")
        ys = (-205, -115, -25, 65, 155, 215) if long_run else (-35, 35)
        for j, y in enumerate(ys):
            for side, x in enumerate((-38, 38)):
                make(row, group, "SHOE_%02d_%d" % (j, side), x, y, 97,
                     42, 25, 13, ("navy", "rose", "denim", "oat")[(j + side) % 4])

    elif kind == "shoe_bench":
        for side, y in enumerate((-128, 128) if socket_length > 300 else (0,)):
            make(row, group, "BENCH_%d" % side, 0, y, 22,
                 140 if socket_length > 300 else 180, 115 if socket_length > 300 else 140,
                 44, "steel", solid=True)
            make(row, group, "CUSHION_%d" % side, 0, y, 48,
                 142 if socket_length > 300 else 184,
                 117 if socket_length > 300 else 144,
                 9, "burgundy" if side == 0 else "navy")
            for xside, x in enumerate((-47, 47)):
                make(row, group, "SAMPLE_%d_%d" % (side, xside), x, y, 57,
                     37, 22, 12, "oat" if xside == 0 else "denim")

    elif kind == "bed_vignette":
        make(row, group, "RUG", 0, 0, 0.6, 200, 306, 1.2, "oat")
        make(row, group, "FRAME", 0, 0, 17, 182, 274, 34,
             "dark_wood", solid=True)
        make(row, group, "MATTRESS", 0, 0, 45, 172, 252, 22, "cream")
        make(row, group, "HEADBOARD", 0, 143, 69, 184, 18, 92,
             "dark_wood", solid=True)
        make(row, group, "DUVET", 0, -52, 60, 174, 150, 9,
             "denim" if index % 2 else "rose")
        for side, x in enumerate((-44, 44)):
            make(row, group, "PILLOW_%d" % side, x, 87, 64,
                 73, 42, 14, "oat")

    elif kind == "home_display":
        make(row, group, "SIDEBOARD", 0, 0, 43, 178, 172, 86,
             "dark_wood", solid=True)
        make(row, group, "TOP", 0, 0, 89, 190, 184, 6, "cream")
        for side, x in enumerate((-54, 54)):
            make(row, group, "LAMP_BASE_%d" % side, x, 42, 97,
                 24, 24, 12, "steel")
            make(row, group, "LAMP_STEM_%d" % side, x, 42, 122,
                 5, 5, 42, "steel")
            make(row, group, "SHADE_%d" % side, x, 42, 148,
                 38, 38, 20, "cream")
        for layer in range(3):
            make(row, group, "FOLDED_%d" % layer, 0, -43, 96 + layer * 7,
                 52 - layer * 5, 42, 6,
                 ("oat", "denim", "rose")[layer])

    elif kind == "feature_table":
        make(row, group, "TABLE", 0, 0, 35, 180, 160, 70,
             "dark_wood", solid=True)
        make(row, group, "TOP", 0, 0, 73, 194, 174, 7, "cream")
        for j, (x, y) in enumerate(((-55, -46), (0, -46), (55, -46),
                                     (-55, 42), (0, 42), (55, 42))):
            make(row, group, "TOY_%02d" % j, x, y, 92,
                 37, 33, 34 if j % 2 else 25,
                 ("butter", "mint", "rose", "denim")[j % 4])

    elif kind == "toy_gondola":
        make(row, group, "FOOT", 0, 0, 10, 158, 544, 20,
             "steel", solid=True)
        make(row, group, "SPINE", 0, 0, 87, 14, 530, 134,
             "cream", solid=True)
        for side, x in enumerate((-42, 42)):
            for tier, z in enumerate((55, 103)):
                make(row, group, "SHELF_%d_%d" % (side, tier), x, 0, z,
                     76, 520, 6, "steel")
            for j, y in enumerate((-220, -135, -50, 35, 120, 205)):
                make(row, group, "STOCK_%d_%02d" % (side, j), x, y, 76,
                     44, 42, 31,
                     ("rose", "butter", "mint", "navy")[(j + side) % 4])

    elif kind == "sofa_vignette":
        make(row, group, "RUG", 0, 0, 0.7, 208, 332, 1.4, "oat")
        make(row, group, "SEAT", 0, 0, 26, 184, 268, 52,
             "denim" if index % 2 else "burgundy", solid=True)
        make(row, group, "BACK", 0, 127, 87, 184, 30, 82,
             "denim" if index % 2 else "burgundy", solid=True)
        for side, x in enumerate((-84, 84)):
            make(row, group, "ARM_%d" % side, x, 0, 62,
                 23, 246, 72, "denim" if index % 2 else "burgundy",
                 solid=True)
            make(row, group, "PILLOW_%d" % side, x / 2, 37, 60,
                 53, 46, 17, "cream" if side == 0 else "rose")

    elif kind == "appliance_pod":
        make(row, group, "PLINTH", 0, 0, 10, 188, 120, 20,
             "charcoal", solid=True)
        for side, x in enumerate((-45, 45)):
            make(row, group, "MACHINE_%d" % side, x, 0, 72,
                 79, 86, 104, "white_steel", solid=True)
            make(row, group, "CONTROL_%d" % side, x, -44, 107,
                 65, 3, 20, "steel")
            make(row, group, "DOOR_%d" % side, x, -45, 65,
                 57, 57, 3, "charcoal", shape="cylinder", roll=90)

require(pieces >= 350 and solids >= 70,
        "fixture density fell unexpectedly: %d pieces, %d solids" %
        (pieces, solids))
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP),
        "map save failed")
message = ("BOSWELLS_DEPARTMENTS SAVED: 48 furnished sockets, %d pieces, "
           "%d colliding structural bodies" % (pieces, solids))
unreal.log(message)
print(message)

"""Dress the Juniors sockets in OpeningPass using the local Boscov's photos.

Refs 02, 03 and 09: dense color-blocked rails, folded endcaps, figure
displays and full-height wall merchandising beside an open customer aisle.
The existing socket bodies carry collision. Wall displays sit beyond the
outer aisle. Rebuild navigation after saving using MAP-NAV-WORKFLOW.md.
"""

import csv
from math import cos, radians, sin
import os
from pathlib import Path

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_JUNIORS_DRESS_"
KIT = "/Game/LLK/Boswells/Fixtures/"


def require(condition, message):
    if not condition:
        raise RuntimeError("BOSWELLS_JUNIORS " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}
lock = before["BSW_EXPOSURE_LOCK"].get_editor_property("settings")
for bound in ("min", "max"):
    key = f"auto_exposure_{bound}_brightness"
    require(lock.get_editor_property("override_" + key)
            and lock.get_editor_property(key) == 0.5, "exposure lock differs")

plan = Path(unreal.Paths.project_dir()) / "docs/reference/boswells/BoswellsFullStore_Sockets.csv"
with plan.open(newline="", encoding="utf-8") as source:
    sockets = [row for row in csv.DictReader(source)
               if row["floor"] == "lower" and row["department"] == "juniors"]
require(len(sockets) == 12, "unexpected Juniors socket count")
require(all("BSW_APPAREL_" + row["name"] in before for row in sockets), "missing socket actors")

cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
palette = {name: unreal.EditorAssetLibrary.load_asset(KIT + name) for name in (
    "dark_wood", "cream", "rose", "burgundy", "denim", "navy", "lavender",
    "charcoal", "steel", "oat", "brass")}
require(cube and all(palette.values()), "fixture kit incomplete")


def load_merchandise(kind):
    name = "SM_BSW_Juniors_" + kind
    path = KIT + name
    reimport = kind.startswith("Display") and os.getenv("LLK_REIMPORT_JUNIORS_DISPLAY") == "1"
    if reimport or not unreal.EditorAssetLibrary.does_asset_exist(path):
        source = Path(unreal.Paths.project_dir()) / "Saved/JuniorsMeshes" / (name + ".obj")
        require(source.is_file(), "run assets/models/boswells/make_juniors_merchandise.py first")
        task = unreal.AssetImportTask()
        task.filename = str(source)
        task.destination_path = KIT.rstrip("/")
        task.destination_name = name
        task.automated = True
        task.replace_existing = reimport
        task.save = True
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = unreal.EditorAssetLibrary.load_asset(path)
    require(isinstance(mesh, unreal.StaticMesh), "missing merchandise " + name)
    require(mesh.get_num_sections(0) >= 2, "merchandise trim slots missing: " + name)
    return mesh


merchandise = {kind: load_merchandise(kind) for kind in (
    "Jacket", "Cardigan", "Tee", "Folded", "DisplayA", "DisplayB")}

# This pass owns these actors. Regenerate them when the dressing recipe changes.
for label, actor in before.items():
    if label.startswith(PREFIX):
        require(actors.destroy_actor(actor), "could not replace " + label)


def spawn(cls, name, position, yaw=0):
    actor = actors.spawn_actor_from_class(cls, unreal.Vector(*position), unreal.Rotator(yaw=yaw))
    require(actor is not None, "spawn failed " + name)
    actor.set_actor_label(PREFIX + name)
    actor.set_folder_path("Boswells/Departments/Lower/Juniors")
    actor.set_actor_enable_collision(False)
    return actor


def visual_only(component):
    component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    component.set_editor_property("can_ever_affect_navigation", False)


def box(name, position, size, material, yaw=0):
    actor = spawn(unreal.StaticMeshActor, name, position, yaw)
    component = actor.static_mesh_component
    require(component.set_static_mesh(cube), "mesh assignment failed " + name)
    component.set_material(0, palette[material])
    actor.set_actor_scale3d(unreal.Vector(*(axis / 100 for axis in size)))
    visual_only(component)
    return actor


def model(name, kind, position, scale=(1, 1, 1), yaw=0, color=None):
    actor = spawn(unreal.StaticMeshActor, name, position, yaw)
    component = actor.static_mesh_component
    mesh = merchandise[kind]
    require(component.set_static_mesh(mesh), "mesh assignment failed " + name)
    actor.set_actor_scale3d(unreal.Vector(*scale))
    for index, slot in enumerate(mesh.get_editor_property("static_materials")):
        key = str(slot.material_slot_name).lower()
        require(key in palette, "unknown material " + key)
        material = color if key == "rose" and color is not None else key
        component.set_material(index, palette[material])
    visual_only(component)
    return actor


# Remove the visual blanket of identical rails; those overlays have no body.
for label, actor in before.items():
    if label.startswith(("BSW_CIRCULATION_LOWER_STOCK_", "BSW_LEFTDAY_APPAREL_RACK_")):
        loc = actor.get_actor_location()
        if loc.x < 0 and -2000 <= loc.y <= -300:
            require(not actor.get_actor_enable_collision(), "overlay unexpectedly solid")
            actor.set_actor_hidden_in_game(True)

colors = ("rose", "burgundy", "denim", "navy", "lavender", "oat")

# Older stock wings have their own collision bodies and visible frame. Swap
# just the rectangular garments, keeping the occupied silhouette and rails.
for index, (label, actor) in enumerate(sorted(before.items())):
    if not label.startswith("BSW_CROWD_") or "_GARMENT_" not in label:
        continue
    center, extent = actor.get_actor_bounds(False)
    if center.x >= 0 or not -2000 <= center.y <= -250:
        continue
    require(not actor.get_actor_enable_collision(), "stock wing garment unexpectedly solid")
    actor.set_actor_hidden_in_game(True)
    model(label.removeprefix("BSW_CROWD_") + "_WING", "Jacket",
          (center.x, center.y, center.z + extent.z), (extent.y / 40, 1.36, extent.z / 36),
          yaw=90 + (-6, 2, 5)[index % 3], color=colors[index % len(colors)])


def endcap(code, x, y, index):
    box(code + "_BACK", (x, y + 48, 74), (157, 8, 148), "dark_wood")
    for tier, z in enumerate((22, 57, 91, 125)):
        box(f"{code}_SHELF_{tier}", (x, y, z), (157, 98, 5), "cream")
        for stack, offset in enumerate((-48, 0, 48)):
            for fold in range(1 + (stack + tier) % 3):
                model(f"{code}_FOLD_{tier}_{stack}_{fold}", "Folded",
                      (x + offset + fold, y - 10, z + 6 + fold * 5.5),
                      (1, 1 - fold * .04, 1), yaw=fold * 3,
                      color=colors[(index + tier + stack) % len(colors)])
    box(code + "_CARD", (x, y - 41, 144), (103, 4, 28), "burgundy")
    actor = spawn(unreal.TextRenderActor, code + "_SALE", (x, y - 44, 144), -90)
    text = actor.text_render
    visual_only(text)
    text.set_text("SALE")
    text.set_world_size(19)
    text.set_horizontal_alignment(unreal.HorizTextAligment.EHTA_CENTER)
    text.set_vertical_alignment(unreal.VerticalTextAligment.EVRTA_TEXT_CENTER)
    text.set_text_render_color(unreal.Color(243, 234, 213, 255))


long_rows = sorted((row for row in sockets if row["kind"] == "clothing_run"),
                   key=lambda row: float(row["xmin_cm"]))
for index, row in enumerate(long_rows):
    x = (float(row["xmin_cm"]) + float(row["xmax_cm"])) / 2
    rear = float(row["ymin_cm"])
    front = float(row["ymax_cm"])
    code = f"RAIL_{index}"
    before["BSW_APPAREL_" + row["name"]].set_actor_hidden_in_game(True)
    # Taller jacket rows interrupt a standing view; shorter groups keep the
    # real retail variation. Every rail remains inside the CSV footprint.
    rail_z = (184, 151, 193, 158, 190, 174)[index]
    start = rear + (129 if index in (1, 3, 4) else 27)
    stop = front - 30
    for leg, y in enumerate((start, stop)):
        box(f"{code}_UPRIGHT_{leg}", (x, y, rail_z / 2), (6, 6, rail_z), "steel")
        box(f"{code}_FOOT_{leg}", (x, y, 5), (137, 12, 10), "charcoal")
    box(code + "_BAR", (x, (start + stop) / 2, rail_z),
        (5, stop - start + 10, 5), "steel")
    for slot, y in enumerate(range(int(start + 10 + 3 * index), int(stop - 5), 24)):
        width = (71, 86, 74, 95)[(index + slot) % 4]
        height = (60, 76, 91, 69, 82)[(slot + 2 * index) % 5]
        shift = (-9, 2, 12, -3)[slot % 4]
        color = colors[(index + slot // 4) % len(colors)]
        top = rail_z - 7 - slot % 3 * 3
        kind = ("Jacket", "Cardigan", "Tee")[(index + slot // 5) % 3]
        model(f"{code}_GARMENT_{slot}", kind, (x + shift, y, top),
              (width / 80, 1.12 + .06 * (slot % 3), height / 72),
              yaw=(-7, 2, 7, -2)[slot % 4], color=color)
    if index in (1, 3, 4):
        endcap(code + "_ENDCAP", x, rear + 56, index)

# Two dressed figures and four low racks share the compact socket bodies.
compact = sorted((row for row in sockets if row["kind"] != "clothing_run"),
                 key=lambda row: float(row["xmin_cm"]))
for index, row in enumerate(compact):
    x = (float(row["xmin_cm"]) + float(row["xmax_cm"])) / 2
    y = (float(row["ymin_cm"]) + float(row["ymax_cm"])) / 2
    before["BSW_APPAREL_" + row["name"]].set_actor_hidden_in_game(True)
    if index not in (1, 4):
        code = f"LOW_RACK_{index}"
        box(code + "_POST", (x, y, 68), (5, 5, 136), "steel")
        box(code + "_FOOT_X", (x, y, 4), (102, 8, 8), "dark_wood")
        box(code + "_FOOT_Y", (x, y, 4), (8, 102, 8), "dark_wood")
        if row["kind"] == "round_rack":
            for slot in range(16):
                yaw = slot * 22.5
                angle = radians(yaw)
                px, py = x + 49 * cos(angle), y + 49 * sin(angle)
                box(f"{code}_RING_{slot}", (px, py, 136), (20, 4, 4), "steel", yaw + 90)
                model(f"{code}_TOP_{slot}", ("Tee", "Cardigan")[slot // 4 % 2],
                      (px, py, 131), (.69, .77, .80), yaw=yaw,
                      color=colors[(index + slot // 3) % len(colors)])
        else:
            for arm in range(4):
                yaw = arm * 90
                angle = radians(yaw)
                dx, dy = cos(angle), sin(angle)
                z = 143 - 8 * (arm % 2)
                box(f"{code}_ARM_{arm}", (x + 39 * dx, y + 39 * dy, z),
                    (78, 4, 4), "steel", yaw)
                for slot, offset in enumerate((31, 46, 61, 74)):
                    model(f"{code}_ARM_{arm}_TOP_{slot}", ("Tee", "Jacket")[arm % 2],
                          (x + offset * dx, y + offset * dy, z - 5),
                          (.72, .73, .85), yaw=yaw + 90 + 4 * (slot % 2),
                          color=colors[(index + arm + slot // 2) % len(colors)])
        continue
    code = f"FIGURE_{index}"
    box(code + "_PLINTH", (x, y, 17), (148, 143, 34), "dark_wood")
    box(code + "_TOP", (x, y, 36), (152, 147, 4), "cream")
    model(code, "DisplayA" if index == 1 else "DisplayB", (x, y, 38), (.89, .89, .89),
          yaw=166 if index == 1 else 198)

# The 100 cm wall strip lies outside the reserved west customer aisle.
for bay, y in enumerate((-1870, -850, -520)):
    code = f"WALL_{bay}"
    box(code + "_BACK", (-4330, y, 131), (5, 262, 258), "dark_wood")
    box(code + "_VALANCE", (-4296, y, 247), (66, 272, 18), "cream")
    box(code + "_BASE", (-4294, y, 26), (72, 263, 12), "cream")
    box(code + "_BAR", (-4274, y, 202), (6, 235, 6), "steel")
    for slot, offset in enumerate(range(-101, 112, 23)):
        drop = (73, 88, 100, 79)[(bay + slot) % 4]
        model(f"{code}_STOCK_{slot}", ("Jacket", "Cardigan", "Tee")[slot // 4 % 3],
              (-4289, y + offset, 197), (.76, .82, drop / 72),
              yaw=(6, -4, 3)[slot % 3], color=colors[(bay + slot // 3) % len(colors)])
    for shelf, z in enumerate((49, 78)):
        box(f"{code}_SHELF_{shelf}", (-4293, y, z), (76, 252, 4), "cream")
        for stack, offset in enumerate((-92, -46, 0, 46, 92)):
            model(f"{code}_FOLDED_{shelf}_{stack}", "Folded", (-4291, y + offset, z + 5),
                  (1, 1, 1.15), yaw=90, color=colors[(bay + shelf + stack) % len(colors)])

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "save failed")
unreal.log_warning("BOSWELLS_JUNIORS_SAVED: 6 long rails, 4 low racks, 3 endcaps, 2 figures, 3 wall bays")

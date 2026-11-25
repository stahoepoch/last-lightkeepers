"""Install the dock opening and walkable Galleria crossing in OpeningPass.

The lower mall follows the reference long wing, center court, dogleg and
far wing. It is a connective greybox. Existing Boswell's and the near
concourse keep their finish, scale and established routes.
"""

from pathlib import Path

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "LLK_DOCK_CHAPTER_"
ROOT = Path(unreal.Paths.project_dir())


def require(ok, message):
    if not ok:
        raise RuntimeError("DOCK_CHAPTER " + message)


def load_scream(take):
    name = "opening-dock-scream-" + take
    folder = "/Game/Audio/Opening"
    path = folder + "/" + name
    if not unreal.EditorAssetLibrary.does_asset_exist(path):
        source = ROOT / "assets/audio" / (name + ".wav")
        require(source.is_file(), "missing scream source " + name)
        task = unreal.AssetImportTask()
        task.filename = str(source)
        task.destination_path = folder
        task.automated = True
        task.save = True
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    wave = unreal.EditorAssetLibrary.load_asset(path)
    require(isinstance(wave, unreal.SoundWave), "sound import failed: " + name)
    return wave


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {actor.get_actor_label(): actor for actor in subsystem.get_all_level_actors()}
require(not any(label.startswith(PREFIX) for label in before), "chapter already built")
cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
arch = "/Game/LLK/Boswells/Architecture/M_BSW_"
props = "/Game/Props/Boswells/Materials/"
mats = {name: unreal.EditorAssetLibrary.load_asset(path) for name, path in {
    "tile": arch + "StoneTile", "glass": arch + "MallEntranceGlass",
    "steel": props + "steel", "wall": arch + "DarkChrome",
    "mark": props + "oat", "cloth": arch + "KidsSageCarpet",
    "wood": props + "dark_wood", "trim": arch + "MallMullionTeal",
}.items()}
require(cube and all(mats.values()), "missing build materials")
controller = before["BSW_OPENING_GRILLE_CONTROLLER"]
controller.set_editor_property("dock_screams", [load_scream(take) for take in ("A", "B", "C")])


def place(cls, name, location, rotation=None, folder="Galleria/Crossing"):
    actor = subsystem.spawn_actor_from_class(
        cls, unreal.Vector(*location), rotation or unreal.Rotator())
    require(actor is not None, "could not place " + name)
    actor.set_actor_label(PREFIX + name)
    actor.set_folder_path(folder)
    return actor


def box(name, x0, x1, y0, y1, z0, z1, mat, solid=True, yaw=0):
    actor = place(unreal.StaticMeshActor, name,
                  ((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2),
                  unreal.Rotator(yaw=yaw))
    component = actor.static_mesh_component
    require(component.set_static_mesh(cube), "mesh assignment failed")
    component.set_material(0, mats[mat])
    actor.set_actor_scale3d(unreal.Vector((x1 - x0) / 100, (y1 - y0) / 100, (z1 - z0) / 100))
    actor.set_actor_enable_collision(solid)
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS
                                    if solid else unreal.CollisionEnabled.NO_COLLISION)
    component.set_editor_property("can_ever_affect_navigation", solid)
    return actor


def retire(label):
    actor = before[label]
    actor.set_actor_hidden_in_game(True)
    actor.set_actor_enable_collision(False)
    body = actor.static_mesh_component
    body.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    body.set_editor_property("can_ever_affect_navigation", False)


def move(actor, xyz, yaw=None):
    require(actor.set_actor_location(unreal.Vector(*xyz), False, False),
            "move failed " + actor.get_actor_label())
    if yaw is not None:
        actor.set_actor_rotation(unreal.Rotator(yaw=yaw), False)


def spot(name, xyz, power, radius, yaw=0, pitch=-90):
    actor = place(unreal.SpotLight, name, xyz, unreal.Rotator(yaw=yaw, pitch=pitch))
    lamp = actor.spot_light_component
    lamp.set_mobility(unreal.ComponentMobility.MOVABLE)
    lamp.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    lamp.set_editor_property("intensity", power)
    lamp.set_editor_property("attenuation_radius", radius)
    lamp.set_editor_property("outer_cone_angle", 48.0)
    lamp.set_editor_property("inner_cone_angle", 22.0)
    lamp.set_editor_property("cast_shadows", True)
    lamp.set_editor_property("light_color", unreal.Color(217, 222, 212, 255))


def sign(name, copy, xyz, yaw, size):
    actor = place(unreal.TextRenderActor, name, xyz, unreal.Rotator(yaw=yaw))
    actor.set_actor_enable_collision(False)
    text = actor.text_render
    text.set_text(copy)
    text.set_world_size(size)
    text.set_horizontal_alignment(unreal.HorizTextAligment.EHTA_CENTER)
    text.set_vertical_alignment(unreal.VerticalTextAligment.EVRTA_TEXT_CENTER)
    text.set_text_render_color(unreal.Color(185, 201, 194, 255))
    text.set_editor_property("can_ever_affect_navigation", False)


# Start behind the dock bollard, looking at the cast and the loading mouth.
move(before["ANCHOR_PLAYERSTART_ENTRANCE"], (400, 3340, 102), -139)
move(before["BSW_OPENING_LANTERN"], (276, 3060, 32))
move(before["BSW_ROUTE_REPAIR_DOCK_GRAB"], (0, 2980, 22))
cast = sorted((actor for actor in before.values()
               if any(str(tag) == "LLK.Opening.Captive" for tag in actor.tags)),
              key=lambda actor: actor.get_actor_label())
require(len(cast) == 4, "expected four opening captives")
for actor, xyz in zip(cast, ((-67, 3170, 23), (40, 3130, 23),
                            (-141, 3080, 23), (112, 3230, 23))):
    move(actor, xyz, -85)
captor = next(actor for actor in before.values()
              if any(str(tag) == "LLK.Opening.Captor" for tag in actor.tags))
move(captor, (18, 2780, 22), 90)
spot("DOCK_WITNESS", (100, 3110, 420), 71, 850)
spot("DOCK_APPROACH", (16, 2560, 445), 33, 740)
spot("WEST_SERVICE", (-3190, 2300, 442), 29, 755)
spot("EAST_SERVICE", (3190, 2240, 442), 26, 740)
sign("DOCK_SIGN", "RECEIVING", (0, 2737, 327), 90, 44)
sign("WEST_DOOR_SIGN", "STOCK / SALES", (-3200, 2142, 265), 90, 26)
sign("EAST_DOOR_SIGN", "SALES FLOOR", (3200, 2142, 265), 90, 26)

# Second exit from Boswell's: the staff passage meets the west shop row.
# The customer grille remains an independent multitool-operated exit.
retire("ROUTE_SOUTH_WALL_W")
box("STAFF_WALL_W", -4370, -3300, -2720, -2680, 0, 1500, "wall")
box("STAFF_WALL_E", -3000, -900, -2720, -2680, 0, 1500, "wall")
box("STAFF_DOOR_HEADER", -3300, -3000, -2720, -2680, 248, 1500, "wall")
box("STAFF_FLOOR_LONG", -3330, -2970, -4700, -2675, -25, 0, "steel")
box("STAFF_FLOOR_TURN", -3310, -2160, -4700, -4340, -25, 0, "steel")
box("STAFF_EDGE_W", -3330, -3300, -4700, -2700, 0, 360, "wall")
box("STAFF_EDGE_E", -3000, -2970, -4370, -2700, 0, 360, "wall")
box("STAFF_EDGE_S", -3330, -2180, -4700, -4674, 0, 360, "wall")
box("STAFF_EDGE_N", -2990, -2175, -4340, -4314, 0, 360, "wall")
box("STAFF_CEILING_LONG", -3330, -2970, -4700, -2675, 357, 380, "wall")
box("STAFF_CEILING_TURN", -3310, -2170, -4700, -4314, 357, 380, "wall")
retire("GALLERIA_NEAR_W_OUTER_WALL")
box("WEST_SHOP_BACK_N", -2212, -2188, -4340, -3100, 0, 1200, "wood")
box("WEST_SHOP_BACK_S", -2212, -2188, -8100, -4674, 0, 1200, "wood")
box("WEST_SHOP_BACK_HEAD", -2212, -2188, -4674, -4340, 249, 1200, "wood")
spot("STAFF_ENTRY", (-3140, -2740, 310), 24, 630)
spot("STAFF_TURN", (-3140, -4520, 320), 27, 610)
sign("STAFF_EXIT_SIGN", "STAFF EXIT", (-3150, -2677, 251), 90, 29)

# Keep pursuit off the concourse until the player makes their own route
# choice. The machine turns into the stock exit and out of the public view.
for label, xyz in (("FRONT_PROMENADE", (-3930, -2250, 0)),):
    move(before["BSW_ROUTE_REPAIR_DOCK_" + label], xyz)
for index, xyz in enumerate(((-3150, -2265, 0), (-3150, -3150, 0)), 7):
    marker = place(unreal.TargetPoint, f"HAUL_{index:02d}", xyz, folder="Boswells/Opening/DockSequence")
    marker.set_editor_property("tags", [unreal.Name(f"LLK.DockOpening.Haul.{index:02d}")])

# The lower crossing preserves the reference's long wing, wider center
# court, lateral turn, and far wing. Generate boundary walls only on the
# outer edges of the floor union, with no false walls at the joins.
retire("GALLERIA_NEAR_FAR_SAFETY_WALL")
rooms = (
    (-900, 900, -10900, -8085),
    (-2300, 2300, -14600, -10900),
    (-1050, 1050, -18000, -14600),
    (-3100, 3000, -22200, -18000),
    (-7900, -3100, -22200, -19800),
    (-7900, -4600, -26100, -22200),
    (-7850, -4950, -33500, -26100),
    (-7850, -4950, -35100, -33500),
)
xs = sorted({x for x0, x1, _, _ in rooms for x in (x0, x1)})
ys = sorted({y for _, _, y0, y1 in rooms for y in (y0, y1)})
cells = set()
for i, (x0, x1) in enumerate(zip(xs, xs[1:])):
    for j, (y0, y1) in enumerate(zip(ys, ys[1:])):
        if any(a <= x0 < x1 <= b and c <= y0 < y1 <= d for a, b, c, d in rooms):
            cells.add((i, j))
for index, (x0, x1, y0, y1) in enumerate(rooms):
    box(f"WING_{index}_FLOOR", x0, x1, y0, y1, -26, 0, "tile")
    ceiling = 1050 if index in (1, 3) else 680
    box(f"WING_{index}_CEILING", x0, x1, y0, y1, ceiling, ceiling + 20, "wall")
    if index in (1, 3):
        for x in (x0 + 140, x1 - 140):
            box(f"WING_{index}_BALCONY_{x}", x - 115, x + 115, y0, y1, 460, 482, "steel")
for i, j in sorted(cells):
    x0, x1, y0, y1 = xs[i], xs[i + 1], ys[j], ys[j + 1]
    for dx, dy, bounds in (
        (-1, 0, (x0 - 22, x0, y0 - 22, y1 + 22)),
        (1, 0, (x1, x1 + 22, y0 - 22, y1 + 22)),
        (0, -1, (x0 - 22, x1 + 22, y0 - 22, y0)),
        (0, 1, (x0 - 22, x1 + 22, y1, y1 + 22)),
    ):
        if (i + dx, j + dy) in cells or (dy == 1 and y1 == -8085):
            continue
        box(f"PERIMETER_{i}_{j}_{dx}_{dy}", *bounds, 0, 1100, "wall")

# Retail islands, shuttered kiosks, and a service strip produce choices
# around the open courts and along the far wing.
for index, (x, y, width, length) in enumerate((
    (0, -12030, 1660, 710), (580, -13300, 1750, 420),
    (0, -15750, 440, 460), (-1200, -20120, 1400, 1060),
    (1330, -19950, 1080, 530), (-4600, -21000, 1500, 390),
    (-6200, -23900, 675, 1380), (-6220, -28300, 1100, 1150),
    (-6160, -31050, 715, 850),
)):
    box(f"ISLAND_{index}", x - width / 2, x + width / 2,
        y - length / 2, y + length / 2, 0, 108, "wood")
    box(f"ISLAND_{index}_TRIM", x - width / 2, x + width / 2,
        y - length / 2, y + length / 2, 108, 114, "steel", solid=False)
for index, (y0, y1) in enumerate(((-27900, -26650), (-30200, -28400), (-32450, -30700))):
    box(f"SERVICE_SPINE_{index}", -7120, -7095, y0, y1, 0, 480, "wood")
    box(f"SERVICE_AWNING_{index}", -7850, -7105, y0, y1, 330, 350, "wall")
for index, (x, y) in enumerate(((300, -9940), (-1370, -11360), (1270, -13810),
                                (430, -16740), (1820, -19180), (-2550, -21320),
                                (-5300, -20500), (-7200, -21540), (-5250, -24200),
                                (-7450, -27050), (-5290, -29190), (-7410, -31420),
                                (-6050, -34170))):
    spot(f"WING_POOL_{index}", (x, y, 485), 46 if index in (4, 12) else 31, 1040)
for index, (x, y, yaw) in enumerate((
    (-3350, -1900, 57), (-3160, -2550, 90), (-3135, -3560, 90), (-2450, -4500, 0),
    (-700, -5870, 90), (300, -8820, 90), (-1370, -11360, 90),
    (-90, -14560, 90), (670, -17010, 90), (-2530, -21150, 0),
    (-5230, -21600, 0), (-7290, -22720, 90), (-7410, -27050, 90),
    (-7350, -31300, 90), (-6250, -34000, 90),
)):
    for side in (-1, 1):
        box(f"DRAG_SCUFF_{index}_{side}", x + side * 22 - 45, x + side * 22 + 45,
            y - 3 + side * 7, y + 3 + side * 7, .45, .8, "mark", solid=False, yaw=yaw)
box("RECEIVING_FRONT_W", -7850, -6525, -34300, -34270, 0, 430, "trim")
box("RECEIVING_FRONT_E", -5825, -4950, -34300, -34270, 0, 430, "trim")
box("RECEIVING_HEADER", -6525, -5825, -34300, -34270, 262, 430, "steel")
sign("FAR_RECEIVING_SIGN", "RECEIVING", (-6175, -34255, 310), 90, 46)
endpoint = place(unreal.TargetPoint, "FAR_RECEIVING", (-6175, -34600, 95))
endpoint.set_editor_property("tags", [unreal.Name("LLK.DockOpening.FarReceiving")])

# Include the dock, both store exits, and the whole new route in the same
# navigation volume. The strict readiness gate runs after the save.
nav = before["ANCHOR_NAV_BOUNDS_FULL_PLATE"]
old_center, old_extent = nav.get_actor_bounds(False)
old_scale = nav.get_actor_scale3d()
nav.set_actor_scale3d(unreal.Vector(old_scale.x * (6800 / old_extent.x),
                                   old_scale.y * (20400 / old_extent.y), old_scale.z))
move(nav, (-2400, -15300, old_center.z))
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
unreal.log_warning("DOCK_CHAPTER_SAVED: dock start, drag route, two store exits, far receiving")

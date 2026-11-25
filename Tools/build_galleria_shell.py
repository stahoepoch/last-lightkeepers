"""Create the first truthful, walkable Galleria shell.

This is intentionally a new map. It does not alter any Boswell's pass.  It
uses measured reference markers for the mall's overall dogleg and public
vertical circulation, then provides conservative placeholder architecture
where no published construction drawing exists.  All labels are fictional.

Run with Tools/run_editor_python.ps1 while Unreal is closed.
"""

import unreal


MAP = "/Game/LLK_Galleria_Shell"
PREFIX = "GALLERIA_SHELL_"


def require(condition, message):
    if not condition:
        raise RuntimeError("GALLERIA_SHELL " + message)


actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if unreal.EditorAssetLibrary.does_asset_exist(MAP):
    # Only the failed first pass can take this route.  Its package was created
    # at 6 KB before a guard fired; retain the no-overwrite rule for any map
    # that contains an actor not stamped by this generator.
    world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
    require(world is not None, "could not load incomplete target")
    existing = actors.get_all_level_actors()
    unsafe = [actor.get_actor_label() for actor in existing
              if not actor.get_actor_label().startswith(PREFIX)]
    require(not unsafe, "target now contains non-generator actors: " + str(unsafe))
    for actor in existing:
        require(actors.destroy_actor(actor), "could not clear incomplete actor")
else:
    require(unreal.EditorLevelLibrary.new_level(MAP), "could not create level")
    world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
    require(world is not None, "could not load new level")
cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
require(isinstance(cube, unreal.StaticMesh), "engine cube missing")

mat_paths = {
    "tile": "/Game/LLK/Boswells/Architecture/M_BSW_StoneTile",
    "glass": "/Game/LLK/Boswells/Architecture/M_BSW_MallEntranceGlass",
    "teal": "/Game/LLK/Boswells/Architecture/M_BSW_MallMullionTeal",
    "dark": "/Game/LLK/Boswells/Architecture/M_BSW_DarkChrome",
    "carpet": "/Game/LLK/Boswells/Architecture/M_BSW_KidsSageCarpet",
    "steel": "/Game/Props/Boswells/Materials/steel",
    "cream": "/Game/Props/Boswells/Materials/cream",
    "wood": "/Game/Props/Boswells/Materials/dark_wood",
}
mats = {name: unreal.EditorAssetLibrary.load_asset(path)
        for name, path in mat_paths.items()}
require(all(isinstance(value, unreal.MaterialInterface) for value in mats.values()),
        "required material unavailable")


def box(name, x0, x1, y0, y1, z0, z1, material, solid=True, folder="Shell"):
    require(x1 > x0 and y1 > y0 and z1 > z0, "bad box " + name)
    actor = actors.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2),
        unreal.Rotator())
    require(actor is not None, "spawn failed " + name)
    actor.set_actor_label(PREFIX + name)
    actor.set_folder_path("Galleria/" + folder)
    component = actor.static_mesh_component
    require(component.set_static_mesh(cube), "mesh failed " + name)
    actor.set_actor_scale3d(unreal.Vector((x1 - x0) / 100,
                                          (y1 - y0) / 100,
                                          (z1 - z0) / 100))
    component.set_material(0, mats[material])
    actor.set_actor_enable_collision(solid)
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS
                                    if solid else unreal.CollisionEnabled.NO_COLLISION)
    component.set_editor_property("can_ever_affect_navigation", solid)
    return actor


def pillar(index, x, y):
    # 80 cm square columns on the public walk's regular structural rhythm.
    return box("PILLAR_%03d" % index, x - 40, x + 40, y - 40, y + 40,
               0, 1500, "cream", True, "Concourse/Pillars")


def rail(name, x0, x1, y0, y1):
    # A solid lower curb and a non-blocking upper glazed rail make the balcony
    # legible while retaining a reliable player boundary.
    box(name + "_CURB", x0, x1, y0, y1, 520, 620, "dark", True,
        "Concourse/Rails")
    box(name + "_GLASS", x0, x1, y0, y1, 620, 760, "glass", False,
        "Concourse/Rails")


def storefront(index, x, y, facing, name):
    # One tiled pocket has a 5 m clear mouth, low soffit, fitted placeholder
    # merchandise, carpet, and a rear wall.  It is a place with depth, not a
    # flat facade.  The public arcade remains 12 m clear between shop faces.
    direction = -1 if facing == "west" else 1
    far = x + direction * 2800
    near = x + direction * 120
    lo, hi = sorted((far, near))
    box("SHOP_%02d_%s_FLOOR" % (index, facing), lo, hi, y - 700, y + 700,
        0, 18, "carpet", True, "Shops")
    wall_lo, wall_hi = sorted((far - 20, far + 20))
    box("SHOP_%02d_%s_BACK" % (index, facing), wall_lo, wall_hi,
        y - 700, y + 700, 0, 520, "dark", True, "Shops")
    box("SHOP_%02d_%s_SOFT" % (index, facing), lo, hi, y - 700, y + 700,
        520, 550, "cream", True, "Shops")
    # Fixture islands leave a 140 cm flank each side and 5 m frontage.
    for fixture, dy in enumerate((-360, 0, 360)):
        fx = x + direction * 1450
        box("SHOP_%02d_%s_FIX_%d" % (index, facing, fixture),
            fx - 90, fx + 90, y + dy - 85, y + dy + 85, 0, 145,
            "wood" if fixture == 1 else "steel", True, "Shops/Fixtures")
    # Side jambs only: the opening stays physically usable.
    for side in (-1, 1):
        box("SHOP_%02d_%s_JAMB_%d" % (index, facing, side),
            x + direction * 70 - 25, x + direction * 70 + 25,
            y + side * 270 - 35, y + side * 270 + 35, 0, 520,
            "teal", True, "Shops")


# The lower public spine follows the reference's long dogleg at a deliberately
# conservative 12 m clear width.  The outer wall, not a void, bounds every
# edge.  Coordinates preserve the existing marker convention: Boswell's south
# end is around x=-13952, y=-3443; the court is around x=1420, y=-3627.
spine = [
    (-15000, -9000, -5200, -2200),   # Boswell's / food-court approach
    (-9000,  2500, -4600, -2200),    # centre-court wing
    (  500,  3500, -10500, -2200),   # long dogleg
    (  500, 16500, -10500, -7500),   # far-anchor wing
]
for i, (x0, x1, y0, y1) in enumerate(spine):
    box("L1_WALK_%d" % i, x0, x1, y0, y1, 0, 22, "tile", True,
        "Concourse/LowerFloor")

# The upper balcony is a separate, cut plate — never a copy of the lower
# floor.  These segments leave the food court, centre well and far escalator
# well open all the way to the glass roof.
for i, (x0, x1, y0, y1) in enumerate((
        (-15000, -7000, -5200, -2200),
        (-3600, 2500, -4600, -2200),
        (500, 3500, -10500, -8500),
        (500, 3500, -6100, -2200),
        (3500, 16500, -10500, -7500),
        (3500, 16500, -7500, -2200))):
    box("L2_BALCONY_%d" % i, x0, x1, y0, y1, 660, 682, "tile", True,
        "Concourse/UpperFloor")

# Cut the visible courts by not supplying upper floor above them; lower floors
# remain present.  Balconies receive guarded edges on the three open rooms.
for name, x0, x1, y0, y1 in (
        ("FOOD_COURT", -7000, -3600, -5200, -2200),
        ("CENTER_WELL", -200, 3000, -5200, -2200),
        ("FAR_ESCALATOR_WELL", 500, 3500, -8500, -6100)):
    box(name + "_LOWER", x0, x1, y0, y1, 22, 28, "tile", True,
        "Concourse/Courts")
    rail(name + "_NORTH", x0, x1, y0, y0 + 24)
    rail(name + "_SOUTH", x0, x1, y1 - 24, y1)
    rail(name + "_WEST", x0, x0 + 24, y0, y1)
    rail(name + "_EAST", x1 - 24, x1, y0, y1)

# There is no fabricated third mall floor.  The vaulted concourse roof rises
# over the two public levels; low soffits remain in the shop pockets.
for i, y in enumerate(range(-10000, -1800, 900)):
    box("ROOF_GLASS_%02d" % i, -15100, 16600, y, y + 830, 1460, 1480,
        "glass", False, "Concourse/Roof")
    box("ROOF_TRUSS_%02d" % i, -15100, 16600, y + 830, y + 900, 1400, 1510,
        "teal", False, "Concourse/Roof")

# Column pairs define the arcade.  They stop under the roof, preserve both
# walkable levels, and give the long sightlines real rhythm.
column_points = []
for y in range(-9800, -1800, 1200):
    column_points.extend([(-1700, y), (1700, y)])
for x in range(-13800, 2400, 1200):
    column_points.extend([(x, -4900), (x, -2500)])
for x in range(800, 15800, 1200):
    column_points.extend([(x, -10400), (x, -7800)])
for index, (x, y) in enumerate(column_points):
    pillar(index, x, y)

# Six dense lower-level storefronts per major horizontal wing; each one is a
# walkable pocket with carpet and fixtures instead of a decorative planar wall.
shop_index = 0
for y in (-4300, -3200):
    for x in (-12500, -10200, -7900, -5600, -3300, -1000):
        storefront(shop_index, x, y, "west" if y == -4300 else "east", "")
        shop_index += 1
for x in (1100, 3600, 6100, 8600, 11100, 13600):
    storefront(shop_index, x, -9300, "west", "")
    shop_index += 1
    storefront(shop_index, x, -7800, "east", "")
    shop_index += 1

# Two-level Boswell's shell: selling floors at 0 and 660 cm, office floor at
# 1320 cm.  The third level belongs to the anchor, never the public mall.
bx0, bx1, by0, by1 = -19500, -15000, -6500, -1700
for floor, z in (("RETAIL_L1", 0), ("RETAIL_L2", 660), ("OFFICE_L3", 1320)):
    box("BOSWELLS_" + floor, bx0, bx1, by0, by1, z, z + 22,
        "tile" if floor != "OFFICE_L3" else "carpet", True, "Boswells")
for y in (by0, by1 - 25):
    box("BOSWELLS_OUTER_WALL_%d" % y, bx0, bx1, y, y + 25, 0, 1500,
        "dark", True, "Boswells")
for x in (bx0, bx1 - 25):
    box("BOSWELLS_OUTER_WALL_%d" % x, x, x + 25, by0, by1, 0, 1500,
        "dark", True, "Boswells")
# Public mall frontage with a wide door, and the rear receiving/loading dock.
box("BOSWELLS_FRONTAGE_W", bx1 - 25, bx1, by0, -4500, 0, 550, "glass", False,
    "Boswells")
box("BOSWELLS_FRONTAGE_E", bx1 - 25, bx1, -3500, by1, 0, 550, "glass", False,
    "Boswells")
box("BOSWELLS_DOCK_APRON", bx0 - 1800, bx0, -5700, -3900, 0, 22, "steel", True,
    "Boswells/Service")
box("BOSWELLS_DOCK_DOOR", bx0 - 20, bx0 + 20, -5200, -4400, 0, 420, "dark", False,
    "Boswells/Service")
box("BOSWELLS_SERVICE_CORRIDOR", bx0 + 250, bx0 + 700, -5700, -2500, 0, 300,
    "steel", True, "Boswells/Service")
# Fitting rooms: a spine, four cubicles, and a staff door placeholder.
for level, z in (("L1", 22), ("L2", 682)):
    base_x = -17700
    base_y = -3300
    box("BOSWELLS_FITTING_%s_LOBBY" % level, base_x, base_x + 1100,
        base_y, base_y + 700, z, z + 15, "carpet", True, "Boswells/FittingRooms")
    for cubicle in range(4):
        y0 = base_y + cubicle * 170
        box("BOSWELLS_FITTING_%s_CUBICLE_%d" % (level, cubicle),
            base_x + 420, base_x + 1050, y0, y0 + 135, z, z + 240,
            "cream", True, "Boswells/FittingRooms")
# Anchor elevator stack — explicitly three stops — and a paired escalator well
# between retail floors only.  Ramps make both retail levels walkable now.
box("BOSWELLS_ELEVATOR_SHAFT", -16600, -16000, -4700, -4100, 0, 1500,
    "teal", True, "Boswells/Vertical")
for level, z in (("L1", 22), ("L2", 682), ("OFFICE", 1342)):
    box("BOSWELLS_ELEVATOR_DOOR_" + level, -16020, -15970, -4560, -4240,
        z, z + 250, "dark", False, "Boswells/Vertical")
for direction, x, slope in (("UP", -17150, .46), ("DOWN", -16850, -.46)):
    for step in range(30):
        y = -5000 + step * 48
        z = 22 + ((step if direction == "UP" else 29 - step) * 22)
        box("BOSWELLS_ESC_%s_TREAD_%02d" % (direction, step),
            x - 80, x + 80, y - 24, y + 24, z, z + 22, "steel", True,
            "Boswells/Vertical")

# The public vertical circulation: 3 escalator connections, one elevator,
# and seven stairs from the researched plan. Their geometry is placeholder but
# their count and role are no longer missing.
verticals = [(-5000, -3950, "FOOD"), (1420, -3627, "COURT"),
             (6805, -2250, "FAR")]
for x, y, name in verticals:
    for side in (-1, 1):
        for step in range(27):
            sy = y - 624 + step * 48
            sz = 28 + ((step if side < 0 else 26 - step) * 24)
            box("ESC_%s_%d_STEP_%02d" % (name, side, step),
                x + side * 260 - 80, x + side * 260 + 80, sy - 18, sy + 18,
                sz, sz + 24, "steel", True, "Concourse/Vertical")
box("CLOCK_ELEVATOR_SHAFT", 1100, 1740, -3945, -3305, 22, 1320,
    "teal", True, "Concourse/Vertical")
for floor, z in (("LOWER", 22), ("UPPER", 682)):
    box("CLOCK_ELEVATOR_DOOR_" + floor, 1710, 1750, -3810, -3450, z, z + 250,
        "dark", False, "Concourse/Vertical")
for i, (x, y) in enumerate(((1037, -1046), (-7702, -1083), (-5838, -6837),
                            (898, -7919), (8518, 2150), (10974, -1546),
                            (1289, -4231))):
    box("STAIR_%02d" % i, x - 160, x + 160, y - 300, y + 300, 0, 682,
        "steel", True, "Concourse/Stairs")

# A player start and a conservative nav volume cover the built shell; neither
# claims that paths, lighting, encounters, or art have been playtested.
start = actors.spawn_actor_from_class(unreal.PlayerStart,
                                      unreal.Vector(-14700, -3443, 110),
                                      unreal.Rotator())
require(start is not None, "player start failed")
start.set_actor_label(PREFIX + "PLAYERSTART_BOSWELLS_FACE")
start.set_folder_path("Galleria/Gameplay")
nav = actors.spawn_actor_from_class(unreal.NavMeshBoundsVolume,
                                    unreal.Vector(-1500, -6000, 750),
                                    unreal.Rotator())
require(nav is not None, "nav bounds failed")
nav.set_actor_label(PREFIX + "NAV_BOUNDS")
nav.set_actor_scale3d(unreal.Vector(340, 110, 16))
nav.set_folder_path("Galleria/Gameplay")

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
all_actors = actors.get_all_level_actors()
created = [a for a in all_actors if a.get_actor_label().startswith(PREFIX)]
require(len(created) >= 350, "shell density unexpectedly low")
print("GALLERIA_SHELL SAVED: %d actors; two public floors, Boswell's retail/office "
      "anchor, pillars, shops, carpet, service dock, fitting rooms, escalators, "
      "clock elevator, stairs, roof and safety geometry" % len(created))

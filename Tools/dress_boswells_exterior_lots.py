"""Put something in the lots, and a sky behind the moon.

Chris, choosing between the two things
`docs/FINDINGS-THE-STORE-HAD-NO-OUTSIDE.md` left open:

    "Lots and sky"

That file's own list of what it did not do is the brief:

    **You can walk out but there is nothing to walk to.** The lots are flat
    asphalt with lamps - no cars, no kerbs, no cart corrals, no treeline, no
    mall beyond.

    **The sky is black.** There is no `SkyAtmosphere` and no stars - the moon
    is a directional light with nothing behind it.

## The one rule this tool inherits

**The inside of the store must not move.** The complaint that began this week
was that the interior was too bright, and a sky is the single most plausible
way to undo that by accident - a `SkyAtmosphere` feeds a sky light, and a sky
light reaches anywhere the envelope leaks. The exterior pass measured the two
interior poses at 16.39 and 40.16 and this one has to leave them there. That
is checked by render afterwards, not by this script, and the numbers are
written down in the findings file either way.

## What a lot full of cars says

Seven cars on a lot that holds two hundred is not set dressing. An anchor
store at night with a handful of cars left in it is a building with people
still inside, and that is the only story this pass tells. They are parked
near the entrance, the way the last cars of an evening are.
"""

import math
import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
CUBE = "/Engine/BasicShapes/Cube.Cube"
CYLINDER = "/Engine/BasicShapes/Cylinder.Cylinder"
CONE = "/Engine/BasicShapes/Cone.Cone"
PREFIX = "BSW_LOT_"
MATERIAL_DIR = "/Game/LLK/Boswells/Architecture"
FIXTURE_PARENT = MATERIAL_DIR + "/M_LLK_LightFixture"
GLASS = MATERIAL_DIR + "/M_BSW_SmokedMirror"

NEW_MATERIALS = {
    "MI_LLK_Lot_Stripe":   (0.46, 0.44, 0.36),   # paint, worn, not white
    # First pass had these at a third of this and the cars read as dark
    # plinths under a bright glass slab: the storefront glass was doing the
    # cabins, and under a sodium lamp it was the brightest thing on the lot.
    # Smoked mirror for the cabins, and paint that catches a lamp.
    "MI_LLK_Lot_CarA":     (0.26, 0.08, 0.08),   # maroon
    "MI_LLK_Lot_CarB":     (0.48, 0.49, 0.50),   # silver
    "MI_LLK_Lot_CarC":     (0.11, 0.16, 0.26),   # navy
    "MI_LLK_Lot_Bark":     (0.08, 0.07, 0.06),
    "MI_LLK_Lot_Canopy":   (0.05, 0.09, 0.05),
}

# Read off the saved map rather than recomputed - these are the actors
# Tools/build_boswells_exterior.py made, and if one of them moves this tool
# must follow it rather than carry a stale copy of its coordinates. The
# exterior verifier had exactly that bug and reported two clear doors blocked.
LOTS = ("BSW_EXT_LOT_EAST", "BSW_EXT_LOT_WEST_LOWER")
DOORS = ("BSW_EXT_DOOR_EAST_GLASS", "BSW_EXT_DOOR_WEST_GLASS")
HILL_EDGE = "BSW_EXT_HILL_W_2"

STALL_DEPTH = 520.0        # how far a car sticks out from the aisle
STALL_WIDTH = 270.0
STRIPE_W, STRIPE_T = 12.0, 2.0
ISLAND_W = 240.0

CAR_L, CAR_W, CAR_H = 450.0, 186.0, 78.0
CABIN_L, CABIN_W, CABIN_H = 236.0, 172.0, 58.0

TREE_COUNT = 14
TREE_TRUNK_H = (320.0, 520.0)
TREE_CANOPY_H = (380.0, 620.0)

SKY_ATMOSPHERE_LABEL = "SKY_ATMOSPHERE"


def require(condition, message):
    if not condition:
        raise RuntimeError("LOTS " + message)


def say(message):
    unreal.log("LOTS| " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None, "could not load " + MAP)
actors_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

meshes = {}
for key, path in (("cube", CUBE), ("cylinder", CYLINDER), ("cone", CONE)):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    require(isinstance(asset, unreal.StaticMesh), "engine mesh missing: " + path)
    meshes[key] = asset

by_label = {}
for actor in actors_sub.get_all_level_actors():
    by_label.setdefault(actor.get_actor_label(), []).append(actor)


def bounds_of(label):
    group = by_label.get(label, [])
    require(len(group) == 1,
            "%s is not in the level exactly once, so the exterior this tool "
            "dresses is not the one it was written against" % label)
    return group[0].get_actor_bounds(False)


# ---------------------------------------------------------------- materials

def ensure_material(name, colour):
    path = MATERIAL_DIR + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        existing = unreal.EditorAssetLibrary.load_asset(path)
        if isinstance(existing, unreal.MaterialInstanceConstant):
            return existing
    parent = unreal.EditorAssetLibrary.load_asset(FIXTURE_PARENT)
    require(isinstance(parent, unreal.Material), "parent material missing")
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    mat = tools.create_asset(name, MATERIAL_DIR,
                             unreal.MaterialInstanceConstant,
                             unreal.MaterialInstanceConstantFactoryNew())
    require(mat is not None, "could not create " + path)
    unreal.MaterialEditingLibrary.set_material_instance_parent(mat, parent)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        mat, "BaseColour", unreal.LinearColor(colour[0], colour[1], colour[2], 1.0))
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
        mat, "Emissive", 0.0)
    unreal.MaterialEditingLibrary.update_material_instance(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)
    say("created material %s" % name)
    return mat


MATS = {name: ensure_material(name, colour)
        for name, colour in NEW_MATERIALS.items()}
glass = unreal.EditorAssetLibrary.load_asset(GLASS)
require(isinstance(glass, unreal.MaterialInterface), "missing " + GLASS)
MATS["glass"] = glass    # the cabins: smoked, not the storefront's clear glass
for name in ("MI_LLK_Ext_Asphalt", "MI_LLK_Ext_Concrete", "MI_LLK_Ext_Turf",
             "MI_LLK_Ext_Metal"):
    asset = unreal.EditorAssetLibrary.load_asset(MATERIAL_DIR + "/" + name)
    require(isinstance(asset, unreal.MaterialInterface),
            "%s is missing - run Tools/build_boswells_exterior.py first" % name)
    MATS[name] = asset


# ---------------------------------------------------------------- placing

placed = []


def clear_previous():
    gone = 0
    for label, group in by_label.items():
        if label.startswith(PREFIX):
            for actor in group:
                if actors_sub.destroy_actor(actor):
                    gone += 1
    return gone


def mesh_of(actor):
    comps = actor.get_components_by_class(unreal.StaticMeshComponent)
    require(comps, "%s has no static mesh component" % actor.get_actor_label())
    return comps[0]


def piece(label, mesh_key, centre, scale, material, yaw=0.0, collision=True):
    full = PREFIX + label
    actor = actors_sub.spawn_actor_from_class(
        unreal.StaticMeshActor, centre, unreal.Rotator(0.0, 0.0, yaw))
    require(actor is not None, "could not spawn " + full)
    actor.set_actor_label(full)
    comp = mesh_of(actor)
    comp.set_mobility(unreal.ComponentMobility.STATIC)
    mesh = meshes[mesh_key]
    if comp.static_mesh != mesh:
        require(comp.set_static_mesh(mesh), "mesh assignment failed for " + full)
    actor.set_actor_scale3d(scale)
    comp.set_material(0, material)
    if not collision:
        actor.set_actor_enable_collision(False)
        comp.set_collision_profile_name("NoCollision")
        comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    placed.append(full)
    return actor


def box(label, x0, x1, y0, y1, z0, z1, material, collision=True):
    require(x1 > x0 and y1 > y0 and z1 > z0, "degenerate box " + label)
    return piece(label, "cube",
                 unreal.Vector((x0 + x1) / 2.0, (y0 + y1) / 2.0, (z0 + z1) / 2.0),
                 unreal.Vector((x1 - x0) / 100.0, (y1 - y0) / 100.0,
                               (z1 - z0) / 100.0),
                 material, collision=collision)


gone = clear_previous()
say("cleared %d actor(s) from a previous run" % gone)


# ---------------------------------------------------------------- the lots

def stall_rows(name, lot_label, door_y, rows):
    """Stripes, islands and the cars that are left.

    `rows` is a list of (aisle_x, facing) - the x of the drive aisle and which
    way the cars nose. Every stall is measured from the lot's own bounds, so a
    lot that is resized does not leave paint hanging off the end of it.
    """
    origin, extent = bounds_of(lot_label)
    top = origin.z + extent.z
    y_lo, y_hi = origin.y - extent.y + 120.0, origin.y + extent.y - 120.0
    made = 0
    for index, (aisle_x, facing) in enumerate(rows):
        x0 = aisle_x
        x1 = aisle_x + facing * STALL_DEPTH
        require(min(x0, x1) >= origin.x - extent.x - 1.0
                and max(x0, x1) <= origin.x + extent.x + 1.0,
                "%s row %d runs off the edge of the lot" % (name, index))
        y = y_lo
        while y <= y_hi:
            box("%s_STRIPE_%d_%d" % (name, index, made),
                min(x0, x1), max(x0, x1), y, y + STRIPE_W,
                top, top + STRIPE_T, MATS["MI_LLK_Lot_Stripe"],
                collision=False)
            made += 1
            y += STALL_WIDTH
        # the planted island at the head of the row
        island_x = aisle_x + facing * (STALL_DEPTH + ISLAND_W / 2.0)
        if origin.x - extent.x < island_x < origin.x + extent.x:
            box("%s_ISLAND_%d" % (name, index),
                min(island_x - ISLAND_W / 2.0, island_x + ISLAND_W / 2.0),
                max(island_x - ISLAND_W / 2.0, island_x + ISLAND_W / 2.0),
                door_y - 1600.0, door_y + 1600.0, top, top + 18.0,
                MATS["MI_LLK_Ext_Concrete"])
            box("%s_ISLAND_%d_TURF" % (name, index),
                island_x - ISLAND_W / 2.0 + 20.0, island_x + ISLAND_W / 2.0 - 20.0,
                door_y - 1580.0, door_y + 1580.0, top + 18.0, top + 24.0,
                MATS["MI_LLK_Ext_Turf"], collision=False)
    say("%s: %d stall lines across %d row(s)" % (name, made, len(rows)))
    return top


CAR_PAINTS = ("MI_LLK_Lot_CarA", "MI_LLK_Lot_CarB", "MI_LLK_Lot_CarC")


def car(label, x, y, z, yaw, paint):
    """Two boxes and a band of glass. A car at 40 m in the dark is a mass with
    a windscreen, and this level is never going to see one closer than that
    without a reason to."""
    rad = math.radians(yaw)
    body_c = unreal.Vector(x, y, z + CAR_H / 2.0)
    piece(label + "_BODY", "cube", body_c,
          unreal.Vector(CAR_L / 100.0, CAR_W / 100.0, CAR_H / 100.0),
          MATS[paint], yaw=yaw)
    cabin_c = unreal.Vector(x - math.cos(rad) * 28.0, y - math.sin(rad) * 28.0,
                            z + CAR_H + CABIN_H / 2.0)
    piece(label + "_CABIN", "cube", cabin_c,
          unreal.Vector(CABIN_L / 100.0, CABIN_W / 100.0, CABIN_H / 100.0),
          MATS["glass"], yaw=yaw)


def park(name, top, aisle_x, facing, door_y, offsets, seed):
    for index, offset in enumerate(offsets):
        car("%s_CAR_%d" % (name, index),
            aisle_x + facing * (STALL_DEPTH / 2.0),
            door_y + offset,
            top,
            0.0 if facing > 0 else 180.0,
            CAR_PAINTS[(index + seed) % len(CAR_PAINTS)])
    say("%s: %d car(s) left on the lot" % (name, len(offsets)))


east_origin, east_extent = bounds_of("BSW_EXT_LOT_EAST")
east_door_y = bounds_of("BSW_EXT_DOOR_EAST_GLASS")[0].y
east_x0 = east_origin.x - east_extent.x
east_top = stall_rows("EAST", "BSW_EXT_LOT_EAST", east_door_y,
                      [(east_x0 + 240.0, +1.0),
                       (east_x0 + 240.0 + STALL_DEPTH + ISLAND_W + 700.0, +1.0)])
park("EAST", east_top, east_x0 + 240.0, +1.0, east_door_y,
     (-820.0, -270.0, 0.0, 540.0, 1360.0, 2180.0), seed=0)

west_origin, west_extent = bounds_of("BSW_EXT_LOT_WEST_LOWER")
west_door_y = bounds_of("BSW_EXT_DOOR_WEST_GLASS")[0].y
west_x1 = west_origin.x + west_extent.x
west_top = stall_rows("WEST", "BSW_EXT_LOT_WEST_LOWER", west_door_y,
                      [(west_x1 - 240.0, -1.0)])
park("WEST", west_top, west_x1 - 240.0, -1.0, west_door_y,
     (-1100.0, 320.0), seed=1)


# A cart corral by the east entrance, because that is the one piece of a car
# park that is unmistakably a car park even as a silhouette.
corral_x = east_x0 + 240.0 + STALL_DEPTH + 90.0
for index, offset in enumerate((-60.0, 60.0)):
    box("CORRAL_RAIL_%d" % index, corral_x, corral_x + 300.0,
        east_door_y + 700.0 + offset, east_door_y + 706.0 + offset,
        east_top + 90.0, east_top + 96.0, MATS["MI_LLK_Ext_Metal"])
for index, (dx, dy) in enumerate(((0.0, -60.0), (0.0, 60.0),
                                  (300.0, -60.0), (300.0, 60.0))):
    box("CORRAL_POST_%d" % index, corral_x + dx - 5.0, corral_x + dx + 5.0,
        east_door_y + 700.0 + dy - 5.0, east_door_y + 700.0 + dy + 5.0,
        east_top, east_top + 110.0, MATS["MI_LLK_Ext_Metal"])


# ---------------------------------------------------------------- the trees

# Along the shoulder of the hill, which is the one place a treeline reads from
# inside the building: you see it under the west canopy, below you.
hill_origin, hill_extent = bounds_of(HILL_EDGE)
for index in range(TREE_COUNT):
    t = index / float(TREE_COUNT - 1)
    # Jittered, not evenly spaced. Fourteen identical cones at a fixed
    # interval read as a fence, which is the opposite of a treeline.
    y = (hill_origin.y - hill_extent.y + t * (hill_extent.y * 2.0)
         + ((index * 37 % 11) - 5) * 46.0)
    # and kept on the hill: the first jitter threw the end tree 230 cm past
    # the edge of it, where the ground trace found nothing at all.
    y = max(hill_origin.y - hill_extent.y + 120.0,
            min(hill_origin.y + hill_extent.y - 120.0, y))
    # On the slope itself, not past the bottom of it: a treeline standing on
    # the car park is not a treeline. And the ground is traced for rather
    # than computed - the first run seated every trunk 17 cm inside the lower
    # lot's own slab, where a downward trace starts inside the collider and
    # finds nothing, which is what the floating check caught.
    x = hill_origin.x - hill_extent.x + 150.0 + (index % 3) * 220.0
    ground = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(x, y, 400.0), unreal.Vector(x, y, -2200.0),
        unreal.TraceTypeQuery.ECC_VISIBILITY, False, [],
        unreal.DrawDebugTrace.NONE, True)
    require(ground is not None,
            "tree %d at (%.0f, %.0f) has no ground under it at all" % (index, x, y))
    z = ground.to_tuple()[4].z - 20.0
    trunk = TREE_TRUNK_H[0] + (TREE_TRUNK_H[1] - TREE_TRUNK_H[0]) * ((index * 7 % 5) / 4.0)
    canopy = TREE_CANOPY_H[0] + (TREE_CANOPY_H[1] - TREE_CANOPY_H[0]) * ((index * 3 % 4) / 3.0)
    piece("TREE_%d_TRUNK" % index, "cylinder",
          unreal.Vector(x, y, z + trunk / 2.0),
          unreal.Vector(0.30, 0.30, trunk / 100.0), MATS["MI_LLK_Lot_Bark"])
    spread = 1.9 + (index * 5 % 7) * 0.13
    piece("TREE_%d_CANOPY" % index, "cone",
          unreal.Vector(x, y, z + trunk + canopy / 2.0),
          unreal.Vector(spread, spread, canopy / 100.0),
          MATS["MI_LLK_Lot_Canopy"], collision=False)
say("%d trees along the hill shoulder" % TREE_COUNT)


# ---------------------------------------------------------------- the sky

# A SkyAtmosphere is what the moon has been missing: without one the
# directional light is a lamp in a void and every frame outside ends in black.
# It also feeds the sky light, which is why the interior gets re-measured.
existing_sky = by_label.get(PREFIX + SKY_ATMOSPHERE_LABEL, [])
for actor in existing_sky:
    actors_sub.destroy_actor(actor)
sky = actors_sub.spawn_actor_from_class(unreal.SkyAtmosphere,
                                        unreal.Vector(0.0, 0.0, 0.0),
                                        unreal.Rotator())
require(sky is not None, "could not spawn the sky atmosphere")
sky.set_actor_label(PREFIX + SKY_ATMOSPHERE_LABEL)
placed.append(PREFIX + SKY_ATMOSPHERE_LABEL)

# The sky light has to recapture or it keeps lighting the level with the black
# it was built against.
for label in ("BSW_EXT_SKYLIGHT",):
    group = by_label.get(label, [])
    require(group, "%s is missing - run Tools/build_boswells_exterior.py "
                   "first" % label)
    for actor in group:
        comp = actor.get_component_by_class(unreal.SkyLightComponent)
        comp.set_editor_property("real_time_capture", True)
        comp.set_editor_property("source_type",
                                 unreal.SkyLightSourceType.SLS_CAPTURED_SCENE)
        comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
        say("%s now captures the sky in real time" % label)


# ---------------------------------------------------------------- checks

after = {}
for actor in actors_sub.get_all_level_actors():
    after.setdefault(actor.get_actor_label(), []).append(actor)
for label in placed:
    require(len(after.get(label, [])) == 1,
            "%s is not in the level exactly once after the run" % label)

# Nothing dressed into a lot may stand inside the store, and nothing may float.
store_x, store_y, roof = 4310.0, 2660.0, 1488.0
floating, intruding = [], []
for label in placed:
    actor = after[label][0]
    if not isinstance(actor, unreal.StaticMeshActor):
        continue
    o, e = actor.get_actor_bounds(False)
    if abs(o.x) + e.x < store_x and abs(o.y) + e.y < store_y and o.z < roof:
        intruding.append(label)
    down = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(o.x, o.y, o.z - e.z + 6.0),
        unreal.Vector(o.x, o.y, o.z - e.z - 220.0),
        unreal.TraceTypeQuery.ECC_VISIBILITY, False, [actor],
        unreal.DrawDebugTrace.NONE, True)
    if down is None:
        floating.append(label)
require(not intruding, "these lot pieces stand inside the store: %s" % intruding)
require(not floating,
        "these lot pieces have nothing under them: %s" % floating[:12])

say("placed %d actor(s)" % len(placed))
saved = unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
say("SAVED=%s" % bool(saved))

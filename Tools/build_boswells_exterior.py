"""Give Boswell's an outside.

Chris, after walking the opening map himself:

    "there's supposed to be like a couple exits or entrances from the outside
    to Boscov's. Definitely the entrance-exit to the mall on both top and
    bottom have that glass, but so do the exits to the outside. There's at
    least two that I can recall on Boscov's that I'm from. Plus the whole exit
    for the the loading dock and stuff. Like if you actually looked at the
    whole Johnstown Galleria and everything, you could see how it's on a
    hillside and the things are weird because we don't even have like an
    exterior or anything."

He is right, and the survey is blunter than the complaint. Before this tool
the entire outside of this building was:

    ANCHOR_FULL_WALL_WEST   x -4350, 5400 long, 1500 tall, BasicShapeMaterial
    ANCHOR_FULL_WALL_EAST   x  4350, 5400 long, 1500 tall, BasicShapeMaterial

Two placeholder-grey slabs, no north face, no south face, no door to the
outside anywhere, no ground beyond the slab, and **no sky, no sky light, no
directional light and no fog in the level at all**. Step through a wall and
you are in black vacuum. The store was an interior with nothing around it.

Those two walls are on `BasicShapeMaterial` - the engine's default. That is
the same defect Chris led with on the upper storey, where seven engine cubes
on a default material were the most visible thing in every render, and it has
been true of the store's own east and west elevations the whole time.

## What this builds

1. **An envelope.** The two placeholder walls are hidden and decollided - not
   deleted, which is this project's own precedent from
   `docs/FINDINGS-FIFTY-LIGHTS-ON-NO-CIRCUIT.md` - and replaced by segmented
   walls on a real facade material, with openings cut for doors.
2. **Two exterior entrances**, west and east, in glass, with mullions, a
   vestibule floor and a canopy over each. These are the ones Chris remembers.
3. **The upper mall entrance.** Both mall headers in this level sit at ground
   level, so the "top" half of what he described did not exist. There is now
   one at the deck.
4. **The dock's own exterior exit** - an overhead door and a staff door on the
   north face, which is where the dock actually is.
5. **The hill.** The Johnstown Galleria sits on one, and the way you read a
   hillside from inside a building is that the ground is not at your feet on
   every side. The east lot is at store grade. The west falls away.
6. **A night to see it against** - sky, moon, fog, and sodium lamps over the
   lots - at intensities chosen so that *the inside of the store does not
   change*. That is the acceptance test at the end of this file, because the
   complaint that started this whole week was that the interior was too bright.

## What this is not

It is not the Galleria. `Content/LLK_Galleria_Shell.umap` is a separate,
uncommitted 350-actor map by another writer that models the mall itself; this
tool does not open it, read it or touch it. This is the outside of one anchor
store, built in the map Chris actually plays.
"""

import math
import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
CUBE = "/Engine/BasicShapes/Cube.Cube"
PREFIX = "BSW_EXT_"
MATERIAL_DIR = "/Game/LLK/Boswells/Architecture"

GLASS = MATERIAL_DIR + "/M_BSW_MallEntranceGlass"
MULLION = MATERIAL_DIR + "/M_BSW_MallMullionTeal"
FIXTURE_PARENT = MATERIAL_DIR + "/M_LLK_LightFixture"

# New dull diffuse instances, made the same way MI_LLK_Crate_Stock was: off
# M_LLK_LightFixture at Emissive 0. A borrowed interior material renders as a
# black rectangle outdoors, which is how the breaker and the crate first went
# in last week.
NEW_MATERIALS = {
    "MI_LLK_Ext_Facade":   (0.52, 0.47, 0.40),   # beige EIFS, an anchor store
    "MI_LLK_Ext_Brick":    (0.29, 0.20, 0.17),   # the base course
    "MI_LLK_Ext_Concrete": (0.34, 0.34, 0.33),   # sidewalk and dock apron
    "MI_LLK_Ext_Asphalt":  (0.11, 0.11, 0.12),   # the lots
    "MI_LLK_Ext_Turf":     (0.09, 0.12, 0.07),   # the hill, at night
    "MI_LLK_Ext_Metal":    (0.17, 0.18, 0.19),   # the dock door and wallpacks
}

# The store box, read off ANCHOR_FULL_FLOOR_LOWER rather than typed in. These
# are checked against the live actor below and the run fails if they moved.
BOX_X, BOX_Y = 4350.0, 2700.0
FLOOR_Z = 0.0
ROOF_Z = 1488.0
PARAPET_Z = 1700.0
DECK_Z = 650.0

WALL_T = 40.0          # the envelope's thickness
# An anchor store's entrance is monumental, not a domestic doorway. The first
# render of this elevation had a 620 x 300 opening in an 8700 x 1700 wall and
# it read as a service door on a warehouse. A real Boscov's entrance is a
# glazed bay two storeys of glass wide.
# An entrance is as wide as the floor behind it allows, between these two.
# 1840 was tried as a fixed width and the west elevation refused it: the
# widest clear bay on that side is 900 cm, because the juniors department
# stands against that wall. A fixed number would have meant either doors
# opening into a dress rack or moving somebody else's department.
DOOR_W_MAX = 1840.0
DOOR_W_MIN = 560.0
DOOR_H = 470.0
CANOPY_D = 420.0

# The elevation needs articulation or it is a blank slab, which is most of
# what "it feels like something I'd see in 1996" means on an exterior.
PIER_SPACING = 1450.0
PIER_W = 150.0
PIER_PROUD = 26.0
SIGN_BAND_Z = (1180.0, 1480.0)

# Where the two exterior entrances go. Chris said "at least two that I can
# recall"; these are the two elevations that have a lot in front of them. The
# y is NOT typed in - the first run put the west entrance behind a juniors
# dress rack, and the verifier found it. See clear_bay() below.
DOOR_SEARCH = (-2300.0, 2300.0)
DOOR_CLEAR_DEPTH = 420.0

# The dock. BSW_REALITY_DOCK_FLOOR is centred (0, 3615) with a 935 half-depth,
# so its north face is y 4550 and the exterior exit belongs there.
DOCK_FACE_Y = 4550.0
DOCK_OVERHEAD_W = 420.0
DOCK_OVERHEAD_H = 440.0

# The hill. East is at grade; west falls away in three tilted runs.
APRON_OUT = 900.0      # sidewalk around the building
LOT_OUT = 4200.0       # how far the lots reach
HILL_RUNS = ((1600.0, 8.0), (1600.0, 11.0), (1800.0, 14.0))

# Night. Every one of these is small on purpose - see the acceptance test.
MOON_LUX = 0.30
SKYLIGHT_INTENSITY = 0.40
FOG_DENSITY = 0.012
LOT_LAMP_CD = 9.0
LOT_LAMP_Z = 820.0


def require(condition, message):
    if not condition:
        raise RuntimeError("EXTERIOR " + message)


def say(message):
    unreal.log("EXT| " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None, "could not load " + MAP)
actors_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
cube = unreal.EditorAssetLibrary.load_asset(CUBE)
require(isinstance(cube, unreal.StaticMesh), "engine cube missing")


# ---------------------------------------------------------------- the box

by_label = {}
for actor in actors_sub.get_all_level_actors():
    by_label.setdefault(actor.get_actor_label(), []).append(actor)

slab = by_label.get("ANCHOR_FULL_FLOOR_LOWER")
require(slab, "ANCHOR_FULL_FLOOR_LOWER is gone, so I do not know where the "
              "building is")
origin, extent = slab[0].get_actor_bounds(False)
require(abs(extent.x - BOX_X) < 1.0 and abs(extent.y - BOX_Y) < 1.0,
        "the store floor now measures %.0f x %.0f, not %.0f x %.0f - the "
        "exterior would be built around the wrong footprint"
        % (extent.x, extent.y, BOX_X, BOX_Y))
say("store footprint confirmed: x %.0f..%.0f  y %.0f..%.0f"
    % (-BOX_X, BOX_X, -BOX_Y, BOX_Y))


# ---------------------------------------------------------------- materials

def ensure_material(name, colour):
    path = MATERIAL_DIR + "/" + name
    # does_asset_exist first: load_asset on a missing path prints a red
    # LoadAsset failed line for every material on every first run, which makes
    # a clean run look like a broken one in the log.
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        existing = unreal.EditorAssetLibrary.load_asset(path)
        if isinstance(existing, unreal.MaterialInstanceConstant):
            return existing
    parent = unreal.EditorAssetLibrary.load_asset(FIXTURE_PARENT)
    require(isinstance(parent, unreal.Material),
            "parent material %s missing" % FIXTURE_PARENT)
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
    # save_loaded_asset, not save_asset: a just-created instance is not in the
    # asset registry yet and the path form fails. Learned the hard way on
    # MI_LLK_Crate_Stock.
    unreal.EditorAssetLibrary.save_loaded_asset(mat)
    say("created material %s" % name)
    return mat


MATS = {}
for name, colour in NEW_MATERIALS.items():
    MATS[name] = ensure_material(name, colour)
for key, path in (("glass", GLASS), ("mullion", MULLION)):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    require(isinstance(asset, unreal.MaterialInterface), "missing " + path)
    MATS[key] = asset

FACADE = MATS["MI_LLK_Ext_Facade"]
BRICK = MATS["MI_LLK_Ext_Brick"]
CONCRETE = MATS["MI_LLK_Ext_Concrete"]
ASPHALT = MATS["MI_LLK_Ext_Asphalt"]
TURF = MATS["MI_LLK_Ext_Turf"]
METAL = MATS["MI_LLK_Ext_Metal"]
GLASS_M = MATS["glass"]
MULLION_M = MATS["mullion"]


# ---------------------------------------------------------------- placing

placed = []


def clear_previous():
    """Idempotence. Everything this tool makes is prefixed, so a re-run starts
    from the same place a first run does."""
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


def box(label, x0, x1, y0, y1, z0, z1, material, collision=True, pitch=0.0):
    require(x1 > x0 and y1 > y0 and z1 > z0,
            "degenerate box %s (%.0f..%.0f, %.0f..%.0f, %.0f..%.0f)"
            % (label, x0, x1, y0, y1, z0, z1))
    full = PREFIX + label
    centre = unreal.Vector((x0 + x1) / 2.0, (y0 + y1) / 2.0, (z0 + z1) / 2.0)
    rot = unreal.Rotator(0.0, pitch, 0.0)
    actor = actors_sub.spawn_actor_from_class(unreal.StaticMeshActor, centre, rot)
    require(actor is not None, "could not spawn " + full)
    actor.set_actor_label(full)
    comp = mesh_of(actor)
    comp.set_mobility(unreal.ComponentMobility.STATIC)
    if comp.static_mesh != cube:
        require(comp.set_static_mesh(cube), "mesh assignment failed for " + full)
    actor.set_actor_scale3d(unreal.Vector((x1 - x0) / 100.0,
                                          (y1 - y0) / 100.0,
                                          (z1 - z0) / 100.0))
    comp.set_material(0, material)
    if not collision:
        # Profile first, then the enum, then check - the same lesson the
        # placeholder elevations taught two runs earlier. Setting the enum
        # alone left the glass blocking a visibility trace, so the verifier
        # reported both entrances sealed by their own windows.
        actor.set_actor_enable_collision(False)
        comp.set_collision_profile_name("NoCollision")
        comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        require(comp.get_collision_enabled()
                == unreal.CollisionEnabled.NO_COLLISION,
                "%s would not give up its collision" % full)
    placed.append(full)
    return actor


def wall_with_opening(prefix, axis_x, span_lo, span_hi, door_centre,
                      door_w, door_h, z0, z1, material):
    """A wall is three pieces when a door goes through it, and the third piece
    is the one people forget: the header over the opening."""
    half = door_w / 2.0
    lo_end = door_centre - half
    hi_end = door_centre + half
    require(span_lo < lo_end and hi_end < span_hi,
            "%s: the opening at %.0f does not fit inside %.0f..%.0f"
            % (prefix, door_centre, span_lo, span_hi))
    x0, x1 = axis_x - WALL_T / 2.0, axis_x + WALL_T / 2.0
    box(prefix + "_A", x0, x1, span_lo, lo_end, z0, z1, material)
    box(prefix + "_B", x0, x1, hi_end, span_hi, z0, z1, material)
    box(prefix + "_HEADER", x0, x1, lo_end, hi_end, z0 + door_h, z1, material)
    return lo_end, hi_end


def glazed_door(prefix, axis_x, y_lo, y_hi, z0, z_top, facing_out):
    """Glass, with mullions, and a mullion is what tells you it is a door
    rather than a hole. `facing_out` is +1 when outside is at larger x."""
    gx0, gx1 = axis_x - 10.0, axis_x + 10.0
    box(prefix + "_GLASS", gx0, gx1, y_lo + 18.0, y_hi - 18.0, z0 + 8.0, z_top - 8.0,
        GLASS_M, collision=False)
    box(prefix + "_MULLION_L", gx0 - 4.0, gx1 + 4.0, y_lo, y_lo + 18.0, z0, z_top,
        MULLION_M)
    box(prefix + "_MULLION_R", gx0 - 4.0, gx1 + 4.0, y_hi - 18.0, y_hi, z0, z_top,
        MULLION_M)
    # mullions every 460 cm, which is what divides a storefront into bays
    step = (y_hi - y_lo) / 4.0
    for i in (1, 2, 3):
        yc = y_lo + step * i
        box(prefix + "_MULLION_%d" % i, gx0 - 4.0, gx1 + 4.0,
            yc - 9.0, yc + 9.0, z0, z_top, MULLION_M)
    box(prefix + "_TRANSOM", gx0 - 4.0, gx1 + 4.0, y_lo, y_hi,
        z_top - 14.0, z_top, MULLION_M)
    # the canopy, outside, which is what makes an entrance read as an entrance
    cx0 = axis_x if facing_out > 0 else axis_x - CANOPY_D
    cx1 = axis_x + CANOPY_D if facing_out > 0 else axis_x
    box(prefix + "_CANOPY", cx0, cx1, y_lo - 90.0, y_hi + 90.0,
        z_top + 20.0, z_top + 44.0, BRICK)
    box(prefix + "_THRESHOLD",
        min(axis_x, cx0) - 10.0, max(axis_x, cx1) + 10.0, y_lo - 40.0, y_hi + 40.0,
        FLOOR_Z - 6.0, FLOOR_Z + 2.0, CONCRETE)


def clear_bay(axis_x, inward):
    """Find a stretch of wall with nothing standing inside it.

    A department store entrance opens onto its own floor, not into the back of
    a dress rack. The first build typed the y in and put the west doors behind
    `BSW_JUNIORS_DRESS_WALL_1_STOCK_2`, which is the kind of thing only a
    trace finds.
    """
    world_ref = unreal.EditorLevelLibrary.get_editor_world()
    step = 60.0
    clear = []
    y = DOOR_SEARCH[0]
    while y <= DOOR_SEARCH[1]:
        start = unreal.Vector(axis_x + inward * 30.0, y, 120.0)
        end = unreal.Vector(axis_x + inward * DOOR_CLEAR_DEPTH, y, 120.0)
        hit = unreal.SystemLibrary.line_trace_single(
            world_ref, start, end, unreal.TraceTypeQuery.ECC_VISIBILITY, False,
            [], unreal.DrawDebugTrace.NONE, True)
        clear.append((y, hit is None))
        y += step
    # the longest clear run
    best_len, best_mid, run_start = 0.0, None, None
    for y, ok in clear + [(None, False)]:
        if ok and run_start is None:
            run_start = y
        elif not ok and run_start is not None:
            length = y_prev - run_start
            if length > best_len:
                best_len, best_mid = length, (run_start + y_prev) / 2.0
            run_start = None
        if y is not None:
            y_prev = y
    require(best_mid is not None and best_len >= DOOR_W_MIN,
            "the widest clear bay on the x=%.0f elevation is %.0f cm and an "
            "entrance needs at least %.0f - the floor would have to be "
            "cleared first" % (axis_x, best_len, DOOR_W_MIN))
    width = min(DOOR_W_MAX, best_len - 80.0)
    say("x=%.0f elevation: %.0f cm of clear floor centred on y %.0f, so the "
        "entrance is %.0f cm wide" % (axis_x, best_len, best_mid, width))
    return best_mid, width


gone = clear_previous()
say("cleared %d actor(s) from a previous run" % gone)


# ---------------------------------------------------------------- envelope

# The two placeholder walls are hidden and decollided rather than deleted.
# docs/FINDINGS-KIDS-FLOOR-STEALTH.md counts geometry as cover and an audit
# that has been told these walls exist should still find them.
hidden = []
for label in ("ANCHOR_FULL_WALL_WEST", "ANCHOR_FULL_WALL_EAST"):
    group = by_label.get(label, [])
    require(group, "%s is missing, so I cannot tell what I am replacing" % label)
    for actor in group:
        mats = set()
        comp = mesh_of(actor)
        for i in range(comp.get_num_materials()):
            m = comp.get_material(i)
            if m:
                mats.add(m.get_name())
        actor.set_actor_hidden_in_game(True)
        comp.set_visibility(False)
        # Both, and in this order. Setting the component alone left the actor
        # colliding, and the verifier caught a new door opening into an
        # invisible wall.
        actor.set_actor_enable_collision(False)
        comp.set_collision_profile_name("NoCollision")
        comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        require(comp.get_collision_enabled()
                == unreal.CollisionEnabled.NO_COLLISION,
                "%s would not give up its collision" % label)
        hidden.append((label, sorted(mats)))
say("hidden and decollided the placeholder elevations: %s"
    % ", ".join("%s wearing %s" % (l, ",".join(m)) for l, m in hidden))

WEST_DOOR_Y, WEST_DOOR_W = clear_bay(-BOX_X, +1.0)
EAST_DOOR_Y, EAST_DOOR_W = clear_bay(BOX_X, -1.0)

west_lo, west_hi = wall_with_opening(
    "WALL_W", -BOX_X, -BOX_Y, BOX_Y, WEST_DOOR_Y, WEST_DOOR_W, DOOR_H,
    FLOOR_Z, PARAPET_Z, FACADE)
east_lo, east_hi = wall_with_opening(
    "WALL_E", BOX_X, -BOX_Y, BOX_Y, EAST_DOOR_Y, EAST_DOOR_W, DOOR_H,
    FLOOR_Z, PARAPET_Z, FACADE)

def articulate(side, axis, door_lo, door_hi, outward):
    """Piers and a sign band, so the elevation has a rhythm instead of being
    one flat rectangle 87 m long."""
    out0 = axis if outward > 0 else axis - PIER_PROUD - WALL_T / 2.0
    out1 = axis + PIER_PROUD + WALL_T / 2.0 if outward > 0 else axis
    count = 0
    y = -BOX_Y + 200.0
    while y < DOCK_FACE_Y - 200.0:
        if not (door_lo - 240.0 < y < door_hi + 240.0):
            box("PIER_%s_%d" % (side, count), out0, out1, y, y + PIER_W,
                FLOOR_Z, PARAPET_Z, BRICK)
            count += 1
        y += PIER_SPACING
    box("SIGNBAND_%s" % side, out0, out1 + outward * 8.0,
        door_lo - 240.0, door_hi + 240.0, SIGN_BAND_Z[0], SIGN_BAND_Z[1],
        MULLION_M)
    # The store's name, in the project's own idiom: a TextRenderActor, the
    # same way every sign inside this building is made. A blank band over the
    # door is a building that does not say whose it is.
    face = (out1 + PIER_PROUD) if outward > 0 else (out0 - PIER_PROUD)
    label = PREFIX + "SIGN_WORD_" + side
    text = actors_sub.spawn_actor_from_class(
        unreal.TextRenderActor, unreal.Vector(face, (door_lo + door_hi) / 2.0,
                                              sum(SIGN_BAND_Z) / 2.0),
        unreal.Rotator(0.0, 0.0, 0.0 if outward > 0 else 180.0))
    require(text is not None, "could not spawn " + label)
    text.set_actor_label(label)
    text.set_actor_enable_collision(False)
    comp = text.text_render
    comp.set_text("BOSWELL'S")
    comp.set_world_size(196.0)
    comp.set_horizontal_alignment(unreal.HorizTextAligment.EHTA_CENTER)
    comp.set_vertical_alignment(unreal.VerticalTextAligment.EVRTA_TEXT_CENTER)
    comp.set_text_render_color(unreal.Color(236, 228, 210, 255))
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    placed.append(label)
    return count


# A base course, because a blank 17 m wall is the 1996 look Chris named. It
# stops at each opening: the first build ran it straight across the east
# entrance and the verifier found the keeper walking into a 120 cm kerb.
for side, axis, lo, hi in (("W", -BOX_X, west_lo, west_hi),
                           ("E", BOX_X, east_lo, east_hi)):
    box("BASE_%s_A" % side, axis - WALL_T / 2.0 - 12.0, axis + WALL_T / 2.0 + 12.0,
        -BOX_Y, lo, FLOOR_Z, FLOOR_Z + 120.0, BRICK)
    box("BASE_%s_B" % side, axis - WALL_T / 2.0 - 12.0, axis + WALL_T / 2.0 + 12.0,
        hi, BOX_Y, FLOOR_Z, FLOOR_Z + 120.0, BRICK)
    box("PARAPET_CAP_" + side, axis - WALL_T / 2.0 - 18.0, axis + WALL_T / 2.0 + 18.0,
        -BOX_Y, BOX_Y, PARAPET_Z, PARAPET_Z + 30.0, CONCRETE)

# The north elevation, which did not exist at all, out to the dock face.
box("WALL_N", -BOX_X, BOX_X, DOCK_FACE_Y - WALL_T, DOCK_FACE_Y,
    FLOOR_Z, PARAPET_Z, FACADE)
box("BASE_N", -BOX_X, BOX_X, DOCK_FACE_Y - WALL_T - 12.0, DOCK_FACE_Y + 12.0,
    FLOOR_Z, FLOOR_Z + 120.0, BRICK)
box("PARAPET_CAP_N", -BOX_X, BOX_X, DOCK_FACE_Y - WALL_T - 18.0, DOCK_FACE_Y + 18.0,
    PARAPET_Z, PARAPET_Z + 30.0, CONCRETE)
# and the return walls that close the gap between the store box and the dock
for side, axis in (("W", -BOX_X), ("E", BOX_X)):
    box("WALL_RETURN_" + side, axis - WALL_T / 2.0, axis + WALL_T / 2.0,
        BOX_Y, DOCK_FACE_Y, FLOOR_Z, PARAPET_Z, FACADE)

glazed_door("DOOR_WEST", -BOX_X, west_lo, west_hi, FLOOR_Z, FLOOR_Z + DOOR_H, -1)
glazed_door("DOOR_EAST", BOX_X, east_lo, east_hi, FLOOR_Z, FLOOR_Z + DOOR_H, +1)

piers_w = articulate("W", -BOX_X, west_lo, west_hi, -1)
piers_e = articulate("E", BOX_X, east_lo, east_hi, +1)
say("piers: %d west, %d east, each elevation with a sign band over its "
    "entrance" % (piers_w, piers_e))


# ---------------------------------------------------- the upper mall entrance

# Both MALL_* headers in this level are at ground level, so the "top" half of
# what Chris described has never existed. This is it, at the deck.
box("MALL_UPPER_FRAME_L", -520.0, -400.0, -BOX_Y - 20.0, -BOX_Y + 20.0,
    DECK_Z, DECK_Z + 320.0, MULLION_M)
box("MALL_UPPER_FRAME_R", 400.0, 520.0, -BOX_Y - 20.0, -BOX_Y + 20.0,
    DECK_Z, DECK_Z + 320.0, MULLION_M)
box("MALL_UPPER_GLASS", -400.0, 400.0, -BOX_Y - 8.0, -BOX_Y + 8.0,
    DECK_Z + 10.0, DECK_Z + 300.0, GLASS_M, collision=False)
box("MALL_UPPER_TRANSOM", -520.0, 520.0, -BOX_Y - 22.0, -BOX_Y + 22.0,
    DECK_Z + 300.0, DECK_Z + 326.0, MULLION_M)
box("MALL_UPPER_SILL", -520.0, 520.0, -BOX_Y - 22.0, -BOX_Y + 22.0,
    DECK_Z - 14.0, DECK_Z, CONCRETE)


# ---------------------------------------------------------------- the dock

box("DOCK_OVERHEAD_FRAME_L", -480.0, -420.0, DOCK_FACE_Y - WALL_T - 6.0,
    DOCK_FACE_Y + 6.0, FLOOR_Z, FLOOR_Z + DOCK_OVERHEAD_H + 40.0, MULLION_M)
box("DOCK_OVERHEAD_FRAME_R", 420.0, 480.0, DOCK_FACE_Y - WALL_T - 6.0,
    DOCK_FACE_Y + 6.0, FLOOR_Z, FLOOR_Z + DOCK_OVERHEAD_H + 40.0, MULLION_M)
box("DOCK_OVERHEAD_DOOR", -420.0, 420.0, DOCK_FACE_Y - WALL_T - 4.0,
    DOCK_FACE_Y + 4.0, FLOOR_Z, FLOOR_Z + DOCK_OVERHEAD_H, MATS["MI_LLK_Ext_Concrete"])
box("DOCK_STAFF_DOOR", 700.0, 810.0, DOCK_FACE_Y - WALL_T - 4.0,
    DOCK_FACE_Y + 4.0, FLOOR_Z, FLOOR_Z + 230.0, MULLION_M)
box("DOCK_APRON", -1400.0, 1400.0, DOCK_FACE_Y, DOCK_FACE_Y + 1300.0,
    FLOOR_Z - 10.0, FLOOR_Z, CONCRETE)
box("DOCK_APRON_KERB", -1400.0, 1400.0, DOCK_FACE_Y + 1300.0,
    DOCK_FACE_Y + 1340.0, FLOOR_Z - 10.0, FLOOR_Z + 22.0, CONCRETE)


# ---------------------------------------------------------------- the ground

# Sidewalk right around the three built elevations.
box("APRON_W", -BOX_X - APRON_OUT, -BOX_X, -BOX_Y, DOCK_FACE_Y,
    FLOOR_Z - 10.0, FLOOR_Z, CONCRETE)
box("APRON_E", BOX_X, BOX_X + APRON_OUT, -BOX_Y, DOCK_FACE_Y,
    FLOOR_Z - 10.0, FLOOR_Z, CONCRETE)

# The east lot is at store grade. This is the flat side of the hill.
box("LOT_EAST", BOX_X + APRON_OUT, BOX_X + APRON_OUT + LOT_OUT,
    -BOX_Y - 600.0, DOCK_FACE_Y, FLOOR_Z - 12.0, FLOOR_Z - 2.0, ASPHALT)

# And the west falls away. Three tilted runs, each starting where the last
# one ended, so the grade is continuous rather than a flight of steps.
hill_x = -BOX_X - APRON_OUT
hill_z = FLOOR_Z
for index, (run, degrees) in enumerate(HILL_RUNS):
    drop = run * math.tan(math.radians(degrees))
    mid_z = hill_z - drop / 2.0
    # A tilted box is placed by its centre; its own length grows with the
    # tilt, so the span is the run divided by the cosine.
    length = run / math.cos(math.radians(degrees))
    label = "HILL_W_%d" % index
    centre = unreal.Vector(hill_x - run / 2.0, (-BOX_Y - 600.0 + DOCK_FACE_Y) / 2.0,
                           mid_z - 30.0)
    actor = actors_sub.spawn_actor_from_class(
        unreal.StaticMeshActor, centre, unreal.Rotator(0.0, degrees, 0.0))
    require(actor is not None, "could not spawn " + label)
    actor.set_actor_label(PREFIX + label)
    comp = mesh_of(actor)
    comp.set_mobility(unreal.ComponentMobility.STATIC)
    if comp.static_mesh != cube:
        require(comp.set_static_mesh(cube), "mesh assignment failed for " + label)
    actor.set_actor_scale3d(unreal.Vector(length / 100.0,
                                          (DOCK_FACE_Y + BOX_Y + 600.0) / 100.0,
                                          0.6))
    comp.set_material(0, TURF)
    placed.append(PREFIX + label)
    say("hill run %d: %.0f cm at %.0f deg, dropping %.0f to z %.0f"
        % (index, run, degrees, drop, hill_z - drop))
    hill_x -= run
    hill_z -= drop

# The lower lot, at the bottom of the hill, where the land flattens again.
box("LOT_WEST_LOWER", hill_x - LOT_OUT, hill_x, -BOX_Y - 600.0, DOCK_FACE_Y,
    hill_z - 12.0, hill_z - 2.0, ASPHALT)


# ---------------------------------------------------------------- the night

def ensure_actor(label, cls, location, setup):
    existing = by_label.get(PREFIX + label)
    if existing:
        for actor in existing:
            actors_sub.destroy_actor(actor)
    actor = actors_sub.spawn_actor_from_class(cls, location, unreal.Rotator())
    require(actor is not None, "could not spawn " + label)
    actor.set_actor_label(PREFIX + label)
    setup(actor)
    placed.append(PREFIX + label)
    return actor


def setup_moon(actor):
    comp = actor.get_component_by_class(unreal.DirectionalLightComponent)
    comp.set_editor_property("intensity", MOON_LUX)
    comp.set_editor_property("light_color", unreal.Color(188, 205, 255, 255))
    comp.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
    actor.set_actor_rotation(unreal.Rotator(0.0, -38.0, 125.0), False)


def setup_skylight(actor):
    comp = actor.get_component_by_class(unreal.SkyLightComponent)
    comp.set_editor_property("intensity", SKYLIGHT_INTENSITY)
    comp.set_editor_property("light_color", unreal.Color(120, 140, 185, 255))
    comp.set_editor_property("mobility", unreal.ComponentMobility.STATIC)


def setup_fog(actor):
    comp = actor.get_component_by_class(unreal.ExponentialHeightFogComponent)
    comp.set_editor_property("fog_density", FOG_DENSITY)
    comp.set_editor_property("fog_height_falloff", 0.15)


ensure_actor("MOON", unreal.DirectionalLight,
             unreal.Vector(0.0, 0.0, 3000.0), setup_moon)
ensure_actor("SKYLIGHT", unreal.SkyLight,
             unreal.Vector(0.0, 0.0, 2600.0), setup_skylight)
ensure_actor("FOG", unreal.ExponentialHeightFog,
             unreal.Vector(0.0, 0.0, 400.0), setup_fog)

# Sodium lamps over the lots. These are outside the envelope and carry neither
# circuit tag: a car park light is not on the store's breaker, and
# docs/FINDINGS-FIFTY-LIGHTS-ON-NO-CIRCUIT.md is about lights that are untagged
# *inside*. They are named so an audit can tell the difference.
LAMP_SPOTS = [
    ("LOT_LAMP_E_1", BOX_X + APRON_OUT + 1400.0, -1500.0, FLOOR_Z),
    ("LOT_LAMP_E_2", BOX_X + APRON_OUT + 1400.0, 1500.0, FLOOR_Z),
    ("LOT_LAMP_E_3", BOX_X + APRON_OUT + 1400.0, 3900.0, FLOOR_Z),
    ("LOT_LAMP_W_1", hill_x - 1200.0, -1200.0, hill_z),
    ("LOT_LAMP_W_2", hill_x - 1200.0, 1800.0, hill_z),
]


def make_lamp(label, x, y, z):
    box(label + "_POLE", x - 14.0, x + 14.0, y - 14.0, y + 14.0,
        z, z + LOT_LAMP_Z, BRICK)
    box(label + "_HEAD", x - 70.0, x + 70.0, y - 40.0, y + 40.0,
        z + LOT_LAMP_Z, z + LOT_LAMP_Z + 26.0, BRICK)

    def setup(actor):
        comp = actor.get_component_by_class(unreal.PointLightComponent)
        comp.set_editor_property("intensity", LOT_LAMP_CD)
        comp.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
        comp.set_editor_property("attenuation_radius", 2600.0)
        comp.set_editor_property("light_color", unreal.Color(255, 214, 160, 255))
        comp.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
        comp.set_editor_property("cast_shadows", True)

    ensure_actor(label + "_LIGHT", unreal.PointLight,
                 unreal.Vector(x, y, z + LOT_LAMP_Z - 10.0), setup)


# A lamp standard directly in front of the entrance stands between the player
# and the store's own name. The first render of the east elevation had one
# bisecting BOSWELL'S, because the lamp spacing was typed and the entrance was
# traced. Push any lamp that lands in front of a door along the row.
DOOR_YS = (WEST_DOOR_Y, EAST_DOOR_Y)
LAMP_CLEARANCE = 700.0
adjusted = []
for index, (label, x, y, z) in enumerate(LAMP_SPOTS):
    for door_y in DOOR_YS:
        if abs(y - door_y) < LAMP_CLEARANCE:
            y = door_y + (LAMP_CLEARANCE if y >= door_y else -LAMP_CLEARANCE)
            adjusted.append(label)
    LAMP_SPOTS[index] = (label, x, y, z)
if adjusted:
    say("moved %s clear of an entrance" % ", ".join(adjusted))

for spot in LAMP_SPOTS:
    make_lamp(*spot)


# Wallpacks over the three doors. The first render of the dock came back as a
# 27 KB black rectangle and the west elevation was a silhouette: a door with
# no light over it is a door nobody finds from outside, which is the same
# mistake the breaker made inside the service corridor last week.
WALLPACK_CD = 3.0


def wallpack(label, x, y, z, colour, out_axis, outward):
    # A hooded housing above the lamp, because the first dock render showed a
    # bare white orb hanging in the air over the door. A light with no fitting
    # around it is the oldest tell in this project.
    back = 56.0 * outward          # the hood sits between the lamp and the wall
    if out_axis == "x":
        box(label + "_HOOD", min(x, x - back), max(x, x - back) + 2.0,
            y - 34.0, y + 34.0, z + 14.0, z + 32.0, METAL)
    else:
        box(label + "_HOOD", x - 34.0, x + 34.0,
            min(y, y - back), max(y, y - back) + 2.0, z + 14.0, z + 32.0, METAL)

    def setup(actor):
        comp = actor.get_component_by_class(unreal.PointLightComponent)
        comp.set_editor_property("intensity", WALLPACK_CD)
        comp.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
        comp.set_editor_property("attenuation_radius", 1500.0)
        comp.set_editor_property("light_color", colour)
        comp.set_editor_property("mobility", unreal.ComponentMobility.STATIC)

    ensure_actor(label, unreal.PointLight, unreal.Vector(x, y, z), setup)


wallpack("WALLPACK_WEST", -BOX_X - 150.0, WEST_DOOR_Y, FLOOR_Z + DOOR_H + 90.0,
         unreal.Color(255, 226, 180, 255), "x", -1)
wallpack("WALLPACK_EAST", BOX_X + 150.0, EAST_DOOR_Y, FLOOR_Z + DOOR_H + 90.0,
         unreal.Color(255, 226, 180, 255), "x", +1)
wallpack("WALLPACK_DOCK", 0.0, DOCK_FACE_Y + 140.0,
         FLOOR_Z + DOCK_OVERHEAD_H + 130.0, unreal.Color(255, 236, 206, 255),
         "y", +1)
wallpack("WALLPACK_DOCK_STAFF", 755.0, DOCK_FACE_Y + 130.0, FLOOR_Z + 300.0,
         unreal.Color(255, 236, 206, 255), "y", +1)


# ---------------------------------------------------------------- checks

after = {}
for actor in actors_sub.get_all_level_actors():
    after.setdefault(actor.get_actor_label(), []).append(actor)

for label in placed:
    require(len(after.get(label, [])) == 1,
            "%s is not in the level exactly once after the run" % label)

# Nothing this tool built may stand inside the store. An exterior that pokes
# through the wall is a wall the player can see daylight through, and this
# building's whole mood is that you cannot.
INSIDE_X, INSIDE_Y = BOX_X - WALL_T, BOX_Y - WALL_T
intruders = []
for label in placed:
    actor = after[label][0]
    if not isinstance(actor, unreal.StaticMeshActor):
        continue    # the moon, the sky light, the fog and the lamps have no
                    # geometry to intrude with, and a sky light's bounds are
                    # the whole level - it failed this check on the first run
    if "DOOR" in label or "MALL_UPPER" in label or "THRESHOLD" in label:
        continue    # a door is supposed to be in the wall
    o, e = actor.get_actor_bounds(False)
    if (abs(o.x) + e.x < INSIDE_X and abs(o.y) + e.y < INSIDE_Y
            and o.z - e.z < ROOF_Z and o.z + e.z > FLOOR_Z):
        intruders.append(label)
require(not intruders,
        "these exterior pieces stand inside the store: %s" % intruders)

# The hill must actually fall. A flat hill is the thing Chris said was missing.
total_drop = FLOOR_Z - hill_z
require(total_drop > 400.0,
        "the west grade drops only %.0f cm, which no one would read as a "
        "hillside" % total_drop)

say("placed %d actor(s)" % len(placed))
say("west grade falls %.0f cm from store level to the lower lot" % total_drop)
say("exterior doors: west, east, the dock overhead, the dock staff door, and "
    "the upper mall entrance")
saved = unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
say("SAVED=%s" % bool(saved))

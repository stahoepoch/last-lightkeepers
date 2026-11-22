"""Close the black gap over Boswell's main aisle, and finish what is above it.

Chris, 2026-11-22, choosing the next thing after the floors and fixtures:
*"The ceiling beams."*

## What the beams actually are

`Tools/audit_boswells_ceiling.py` counted what is overhead and a rendered
shot from the west wing settled it. The sales floor has a suspended tile
ceiling at 5.14 m - `BSW_LOWER_CEILING_WEST_FIELD` and `..._EAST_FIELD`,
`M_BSW_LowerCeilingTile`, and it reads correctly - but the two fields stop at
x = -900 and x = 900. **The 18 m strip between them, which is the main aisle,
has no ceiling at all.**

So from anywhere in a wing you look across a black trench, with the near
field's 4 cm edge in front of it and the far field's beyond. That edge is the
"beam". There is no beam: there is a hole, and the thing Chris read as a
blade is the lit ceiling ending at one.

**The gap is exactly the route.** The racetrack laid on 2026-11-21 runs up
that strip; it is where the chapter walks from Boswell's to Sayers, and it is
the one part of the store with nothing overhead.

## What this lays

A **painted bulkhead** over the aisle rather than more tile. Real stores
change the ceiling over the racetrack - the departments get lay-in grid, the
main run gets drywall - and that change overhead is the same signal the
carpet gives underfoot, read the other way up. A player who looks up should
be able to tell whether they are on the route or in a department, and after
this they can.

**The rule is "no part of the sales floor is unroofed", not "the aisle is
18 m wide".** It walks the whole floor and fills whatever is under the
mezzanine and not already under a tile field, because the aisle is only the
largest hole - the fields also stop 6 m short of the north wall, and a tool
that had the aisle's width typed into it would have closed one gap and left
the other. That is how this one got here.

**The atrium stays open.** It is derived from the map rather than typed here:
whatever rectangle the four `ANCHOR_UPPER_*` decks leave uncovered is the
void, and the soffit skips it. An anchor store's atrium is open to the roof
on purpose - it is where the building admits it has an upstairs, and capping
it would cost the one piece of verticality this floor has. The escalators are
skipped the same way, by their own bounds, so a plane cannot be laid through
one.

And **nothing a player looks up at should wear the engine placeholder.**
`ANCHOR_FULL_ROOF` - 4,698 m2, the thing you see from the atrium - is on
`BasicShapeMaterial`, and so are the four `ANCHOR_ATRIUM_POST`s, which
through the gap read as an enormous black X. Those are repainted by
material rather than by name, because a list of labels goes stale the next
time somebody adds a post.

Idempotent: everything it creates is `BSW_AISLE_SOFFIT_*` and is removed
first. Materials are reassigned in place and are not actors.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_AISLE_SOFFIT_"
MATERIAL_DIR = "/Game/LLK/Boswells/Architecture"
MASTER_NAME = "M_LLK_CeilingSoffit"

# The tile fields, from the audit: underside 514, top 518, centre 516.
# The bulkhead sits at the same height and is 2 cm deeper, so its edge shows
# just below the grid - which is what a drywall bulkhead a tile ceiling dies
# into actually looks like, and keeps two coplanar faces from fighting.
SOFFIT_Z = 516.0
SOFFIT_DEPTH_CM = 6.0

FLOOR_HALF_X = 4350.0
FLOOR_HALF_Y = 2700.0
PLANE_SIZE_CM = 100.0
CELL_CM = 300.0             # small enough to cut round the atrium and escalators
CLEARANCE_CM = 120.0        # kept back from an opening's edge

world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("SOFFIT map unavailable")

subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()
actors = subsystem.get_all_level_actors()

# ---- 1. the master and its instances ------------------------------------
master_path = "%s/%s" % (MATERIAL_DIR, MASTER_NAME)
master = unreal.EditorAssetLibrary.load_asset(master_path)
if master is None:
    master = tools.create_asset(MASTER_NAME, MATERIAL_DIR,
                                unreal.Material, unreal.MaterialFactoryNew())
    colour = unreal.MaterialEditingLibrary.create_material_expression(
        master, unreal.MaterialExpressionVectorParameter, -400, 0)
    colour.set_editor_property("parameter_name", "BaseColour")
    colour.set_editor_property("default_value",
                               unreal.LinearColor(0.34, 0.33, 0.31, 1.0))
    unreal.MaterialEditingLibrary.connect_material_property(
        colour, "", unreal.MaterialProperty.MP_BASE_COLOR)

    rough = unreal.MaterialEditingLibrary.create_material_expression(
        master, unreal.MaterialExpressionScalarParameter, -400, 220)
    rough.set_editor_property("parameter_name", "Roughness")
    # Flat latex on drywall. A ceiling that catches a highlight reads as
    # sheet metal, and this one is meant to read as paint.
    rough.set_editor_property("default_value", 0.92)
    unreal.MaterialEditingLibrary.connect_material_property(
        rough, "", unreal.MaterialProperty.MP_ROUGHNESS)

    unreal.MaterialEditingLibrary.recompile_material(master)
    unreal.EditorAssetLibrary.save_asset(master_path)
    print("SOFFIT created_master=%s" % master_path)
else:
    print("SOFFIT master_exists=%s" % master_path)


def instance(name, rgb):
    path = "%s/%s" % (MATERIAL_DIR, name)
    existing = unreal.EditorAssetLibrary.load_asset(path)
    if existing is not None:
        return existing
    inst = tools.create_asset(name, MATERIAL_DIR,
                              unreal.MaterialInstanceConstant,
                              unreal.MaterialInstanceConstantFactoryNew())
    unreal.MaterialEditingLibrary.set_material_instance_parent(inst, master)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        inst, "BaseColour", unreal.LinearColor(rgb[0], rgb[1], rgb[2], 1.0))
    unreal.EditorAssetLibrary.save_asset(path)
    print("SOFFIT created_instance=%s rgb=%s" % (name, rgb))
    return inst


# The bulkhead is a shade warmer and darker than the tile so the transition is
# legible without being a stripe. The roof is darker still: it is 15 m up in a
# building with no mains power, and a bright roof over a dark floor is the
# tell that a ceiling is unlit rather than high.
aisle_mat = instance("MI_LLK_Soffit_Aisle", (0.30, 0.29, 0.27))
structure_mat = instance("MI_LLK_Soffit_Structure", (0.13, 0.13, 0.14))

# ---- 2. the openings, derived rather than typed --------------------------
decks = []
for actor in actors:
    if not actor.get_actor_label().startswith("ANCHOR_UPPER_"):
        continue
    centre, extent = actor.get_actor_bounds(False)
    decks.append((centre, extent))
if not decks:
    raise RuntimeError("SOFFIT no ANCHOR_UPPER_ decks; atrium cannot be derived")
print("SOFFIT decks=%d" % len(decks))

openings = []
for actor in actors:
    label = actor.get_actor_label().upper()
    if "ESCALATOR" not in label and "ESC_" not in label:
        continue
    centre, extent = actor.get_actor_bounds(False)
    if extent.x < 60.0 and extent.y < 60.0:
        continue
    openings.append((centre, extent))
print("SOFFIT escalator_pieces=%d" % len(openings))

# The suspended tile fields that already roof the wings. Read from the map
# rather than typed, because the rule this lays is **no part of the sales
# floor is unroofed** and not "the aisle is 18 m wide". The aisle is only the
# biggest hole; the fields also stop 6 m short of the north wall, and a tool
# that knew the aisle's width by heart would have roofed one and left the
# other - which is how the first gap got here.
fields = []
for actor in actors:
    if not actor.get_actor_label().startswith("BSW_LOWER_CEILING_"):
        continue
    centre, extent = actor.get_actor_bounds(False)
    fields.append((centre, extent))
print("SOFFIT existing_tile_fields=%d" % len(fields))
if not fields:
    raise RuntimeError("SOFFIT no BSW_LOWER_CEILING_ fields; this map's "
                       "suspended ceiling is gone, not merely gapped")


def under_a_deck(x, y):
    """Is this point roofed by the mezzanine? If not, it is the atrium void."""
    return any(abs(x - c.x) <= e.x and abs(y - c.y) <= e.y for c, e in decks)


def over_an_escalator(x, y):
    return any(abs(x - c.x) <= e.x + CLEARANCE_CM and
               abs(y - c.y) <= e.y + CLEARANCE_CM for c, e in openings)


def already_tiled(x, y):
    """Is this cell WHOLLY under a suspended tile field?

    Wholly, not at its centre. A cell whose centre is on a field's edge is
    half over the gap, and skipping it would leave a 1.5 m strip of void
    down each side of the aisle - the same off-by-one that widened the main
    aisle by a whole tile when the carpet was laid, two commits ago.

    Laying it instead overlaps the tile by up to half a cell, which is the
    right way round: the bulkhead's underside is at 513 and the field's at
    514, so the paint reads in front of the grid and nothing is coplanar.
    """
    half = CELL_CM * 0.5
    return any(abs(x - c.x) + half <= e.x and abs(y - c.y) + half <= e.y
               for c, e in fields)


# ---- 3. clear and lay ----------------------------------------------------
removed = 0
for actor in actors:
    if actor.get_actor_label().startswith(PREFIX):
        subsystem.destroy_actor(actor)
        removed += 1
print("SOFFIT removed=%d" % removed)

cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
if cube is None:
    raise RuntimeError("SOFFIT /Engine/BasicShapes/Cube missing")

placed = 0
skipped_void = 0
skipped_esc = 0
skipped_tiled = 0
x = -FLOOR_HALF_X + CELL_CM * 0.5
while x <= FLOOR_HALF_X - CELL_CM * 0.5 + 1.0:
    y = -FLOOR_HALF_Y + CELL_CM * 0.5
    while y <= FLOOR_HALF_Y - CELL_CM * 0.5 + 1.0:
        if already_tiled(x, y):
            skipped_tiled += 1
            y += CELL_CM
            continue
        if not under_a_deck(x, y):
            skipped_void += 1
            y += CELL_CM
            continue
        if over_an_escalator(x, y):
            skipped_esc += 1
            y += CELL_CM
            continue
        actor = subsystem.spawn_actor_from_class(
            unreal.StaticMeshActor, unreal.Vector(x, y, SOFFIT_Z))
        if actor is None:
            raise RuntimeError("SOFFIT could not spawn a bulkhead piece")
        actor.set_actor_label("%s%d_%d" % (PREFIX, int(x), int(y)))
        comp = actor.static_mesh_component
        comp.set_mobility(unreal.ComponentMobility.MOVABLE)
        comp.set_static_mesh(cube)
        actor.set_actor_scale3d(unreal.Vector(
            CELL_CM / PLANE_SIZE_CM, CELL_CM / PLANE_SIZE_CM,
            SOFFIT_DEPTH_CM / PLANE_SIZE_CM))
        comp.set_material(0, aisle_mat)
        # A ceiling is looked at, never walked into. The deck above already
        # carries the collision, and a collider here would be one more thing
        # the navmesh has to be told to ignore.
        comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        if hasattr(comp, "set_can_ever_affect_navigation"):
            comp.set_can_ever_affect_navigation(False)
        placed += 1
        y += CELL_CM
    x += CELL_CM

print("SOFFIT placed=%d m2=%.0f skipped_already_tiled=%d skipped_atrium_void=%d "
      "skipped_escalator=%d" % (
          placed, placed * (CELL_CM / 100.0) ** 2,
          skipped_tiled, skipped_void, skipped_esc))

# ---- 4. everything overhead still wearing the engine's default -----------
# A second shot, from 4 m up looking down the aisle, showed the gap was not
# the only thing wrong with the view: `ANCHOR_FULL_ROOF` (4,698 m2) and the
# four `ANCHOR_ATRIUM_POST`s all wear `BasicShapeMaterial`, and through the
# gap the posts read as an enormous black X. So the rule is **nothing a
# player looks up at wears the engine placeholder**, applied by material
# rather than by name - a list of labels is a thing that goes stale the next
# time somebody adds a post.
PLACEHOLDERS = ("BasicShapeMaterial", "WorldGridMaterial", "DefaultMaterial")
OVERHEAD_CM = 400.0

# **Two narrow rules, after two runs of a broad one were wrong.**
#
# A first version tested `centre.z + extent.z > 400`, which is true of
# anything tall, and repainted 66 actors including the store's west and east
# walls. Testing the object's bottom instead cut that to 49 - and the
# remainder were `UPPER_TOYS`, `UPPER_KIDS`, `UPPER_BEDDING`: merchandise
# standing on the mezzanine deck, whose feet are above head height for the
# simple reason that the mezzanine is. Painting a store's stock the colour of
# a roof is not finishing a ceiling.
#
# There is no height that separates "hangs above you" from "stands on the
# floor above you", so this stops trying to find one. It repaints what it can
# justify: **lids**, which are flat and wide and overhead by measurement, and
# **the atrium posts**, which are named because a screenshot showed them -
# four columns reading as an enormous black X through the gap, and the only
# structure the atrium shows a player on the ground. Everything else on a
# placeholder material is counted and reported, not quietly recoloured.
LID_MAX_THICKNESS_CM = 40.0
LID_MIN_SPAN_CM = 300.0
NAMED_STRUCTURE = ("ANCHOR_ATRIUM_POST",)

repainted = 0
repainted_actors = []
left_alone = []
for actor in actors:
    label = actor.get_actor_label()
    centre, extent = actor.get_actor_bounds(False)
    if centre.z - extent.z < OVERHEAD_CM:
        continue

    is_lid = (extent.z <= LID_MAX_THICKNESS_CM
              and extent.x >= LID_MIN_SPAN_CM and extent.y >= LID_MIN_SPAN_CM)
    is_named = any(label.startswith(n) for n in NAMED_STRUCTURE)

    for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
        if comp.static_mesh is None:
            continue
        for slot, mat in enumerate(comp.get_materials()):
            if (mat.get_name() if mat else "<none>") not in PLACEHOLDERS:
                continue
            if not (is_lid or is_named):
                if label not in left_alone:
                    left_alone.append(label)
                continue
            comp.set_material(slot, structure_mat)
            repainted += 1
            if label not in repainted_actors:
                repainted_actors.append(label)

print("SOFFIT placeholder_slots_repainted=%d actors=%d %s" % (
    repainted, len(repainted_actors), repainted_actors[:8]))
print("SOFFIT placeholder_left_alone=%d %s" % (
    len(left_alone), left_alone[:10]))
if repainted == 0:
    raise RuntimeError("SOFFIT no overhead lid or atrium post wore a "
                       "placeholder material - ANCHOR_FULL_ROOF and the four "
                       "ANCHOR_ATRIUM_POSTs did when this was written")

unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
print("SOFFIT saved=True")

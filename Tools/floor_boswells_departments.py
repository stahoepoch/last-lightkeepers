"""Give Boswell's a racetrack of hard floor and carpeted departments.

Chris, 2026-11-22, twice:

    "There's walkways that are linoleum or ceramic, I don't even know what
    they are, but there's walkways. And then there's carpeted areas."

    "Go after the floors, the walkways, the carpeted areas."

**The measurement.** `Tools/census_boswells_floor.py` found the inverse of
that: **76.3% of everything you walk on is one material**, `M_BSW_StoneTile`,
16,207 m2 of it, with 12.6% carpet that is **all the same carpet** - the Kids
sage, reused even on the Galleria shop floors. A real department store is
mostly carpet, broken by a hard-surface racetrack that leads you around it,
and the carpet changes when the department does. That change underfoot is one
of the strongest signals a store gives you that you have walked somewhere
else, and this map gives none of it.

## What this lays

A **racetrack** of the existing stone tile, left uncovered: a main aisle up
the middle on X, a cross aisle through the centre on Y, and a perimeter lane
against the walls. That shape is not decoration - it is how department stores
move people, and it doubles as the chapter's legible route.

**Carpeted departments** in the blocks between, one material each, named and
coloured from the department signs already hanging in the map rather than
from a list invented here. A store whose carpet says `LADIES` where the sign
says `LADIES` is a store somebody authored; one with four arbitrary colours
is a colour-blocking exercise.

The old `BSW_REALITY_CARPET_ZONE_*` planes are removed. They are four patches
of the Kids sage carpet standing in for every department in the building, and
leaving them under the new ones would mean two floors disagreeing wherever
the new zoning does not exactly cover the old.

## Materials

`M_LLK_DeptCarpet` is created once as a master with a `BaseColour` vector
parameter and a `Roughness` scalar, and each department gets an instance of
it. Instances rather than separate materials so a retune is one parameter on
one asset, and so the next department costs nothing. The existing
`M_BSW_KidsSageCarpet` could not be reused this way: it is a plain `Material`
with no parameters at all.

Idempotent: every actor it creates is `BSW_DEPT_CARPET_*` and those are
removed first.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_DEPT_CARPET_"
MATERIAL_DIR = "/Game/LLK/Boswells/Architecture"
MASTER_NAME = "M_LLK_DeptCarpet"

# Sales floor, from the census: centre (0,0), extent 4350 x 2700 cm.
FLOOR_HALF_X = 4350.0
FLOOR_HALF_Y = 2700.0

# The racetrack. Hard floor, left as the stone tile underneath.
MAIN_AISLE_HALF_X = 900.0      # up the middle, 18 m across
CROSS_AISLE_HALF_Y = 500.0     # through the centre, 10 m across
PERIMETER_CM = 250.0           # a lane against the walls

CARPET_Z = 3.0                 # just above the slab at -15 and the old zones at 2
PLANE_SIZE_CM = 100.0          # /Engine/BasicShapes/Plane is 100 cm square

# Department carpets, as a real anchor store runs them: muted, dark enough to
# hide traffic, and different enough from each other to be felt underfoot.
#
# **These are the departments this building has on its GROUND floor**, which
# `Tools/audit_boswells_departments.py` established by counting merchandise
# rather than by reading the palette that used to be typed here. HOME and
# FABRICS are gone: HOME has 20 actors and all 20 are on the mezzanine, and
# FABRICS was never in this map at all. ELECTRONICS is new and is a real
# ground-floor department, with a sign at (1600, 650).
PALETTE = {
    "LADIES": (0.19, 0.13, 0.17),
    "SHOES": (0.17, 0.15, 0.13),
    "JUNIORS": (0.12, 0.16, 0.18),
    "MENS": (0.11, 0.12, 0.16),
    "ELECTRONICS": (0.13, 0.13, 0.15),
}
DEFAULT_COLOUR = (0.15, 0.14, 0.14)

# The mezzanine deck's underside. Anything at or above this is upstairs and
# says nothing about what a player is standing on down here.
MEZZANINE_UNDERSIDE_CM = 638.0

world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("DEPTFLOOR map unavailable")

subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()

# ---- 1. the master material, once ---------------------------------------
master_path = "%s/%s" % (MATERIAL_DIR, MASTER_NAME)
master = unreal.EditorAssetLibrary.load_asset(master_path)
if master is None:
    master = tools.create_asset(MASTER_NAME, MATERIAL_DIR,
                                unreal.Material, unreal.MaterialFactoryNew())
    colour = unreal.MaterialEditingLibrary.create_material_expression(
        master, unreal.MaterialExpressionVectorParameter, -400, 0)
    colour.set_editor_property("parameter_name", "BaseColour")
    colour.set_editor_property("default_value", unreal.LinearColor(0.15, 0.14, 0.14, 1.0))
    unreal.MaterialEditingLibrary.connect_material_property(
        colour, "", unreal.MaterialProperty.MP_BASE_COLOR)

    rough = unreal.MaterialEditingLibrary.create_material_expression(
        master, unreal.MaterialExpressionScalarParameter, -400, 220)
    rough.set_editor_property("parameter_name", "Roughness")
    # Carpet is almost entirely diffuse. A floor that catches a highlight
    # reads as vinyl, which is the surface this one exists to differ from.
    rough.set_editor_property("default_value", 0.95)
    unreal.MaterialEditingLibrary.connect_material_property(
        rough, "", unreal.MaterialProperty.MP_ROUGHNESS)

    unreal.MaterialEditingLibrary.recompile_material(master)
    unreal.EditorAssetLibrary.save_asset(master_path)
    print("DEPTFLOOR created_master=%s" % master_path)
else:
    print("DEPTFLOOR master_exists=%s" % master_path)


_carpet_cache = {}


def carpet_for(name):
    """One material instance per department, kept in step with the palette.

    **It writes the colour whether or not the instance already exists.** The
    first version returned an existing instance untouched, which means an
    edit to `PALETTE` would have changed nothing and the run would have
    reported success either way - the same shape as the migrate in
    `HANDOFFS.md` M15, where a default that skipped was indistinguishable
    from a migrate that worked. A retune has to be one run of this file.
    """
    if name in _carpet_cache:
        return _carpet_cache[name]

    inst_name = "MI_LLK_Carpet_%s" % name.title()
    inst_path = "%s/%s" % (MATERIAL_DIR, inst_name)
    inst = unreal.EditorAssetLibrary.load_asset(inst_path)
    verb = "updated"
    if inst is None:
        verb = "created"
        inst = tools.create_asset(inst_name, MATERIAL_DIR,
                                  unreal.MaterialInstanceConstant,
                                  unreal.MaterialInstanceConstantFactoryNew())
        unreal.MaterialEditingLibrary.set_material_instance_parent(inst, master)

    r, g, b = PALETTE.get(name.upper(), DEFAULT_COLOUR)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        inst, "BaseColour", unreal.LinearColor(r, g, b, 1.0))
    unreal.EditorAssetLibrary.save_asset(inst_path)
    print("DEPTFLOOR %s_instance=%s rgb=(%.2f,%.2f,%.2f)" % (
        verb, inst_name, r, g, b))
    _carpet_cache[name] = inst
    return inst


# ---- 2. where the departments are, from the stock rather than the signs --
#
# **Corrected 2026-11-22. The first version read the nearest `TextRenderActor`
# in XY and ignored Z**, and produced a floor that was the inverse of the
# store above it:
#
#   | department | ground-floor stock actors | carpet it got |
#   | JUNIORS    | 477                       | 11%           |
#   | SHOES      |  29                       | 38%           |
#
# Three faults, not one. It counted signs **upstairs** - HOME is on the
# mezzanine, and so are TOYS, SEASONAL and FURNITURE. It counted signs
# **outside the store** - the Galleria shopfronts at y = -4050 carry SHOES
# and TOYS and were voting on Boswell's carpet. And it treated the
# `BSW_LOWER_WAYFINDING_*` signs, which hang on the aisle edge pointing *into*
# a department, as if they marked its centre, which drags every assignment
# toward the middle of the building.
#
# So the evidence is now the merchandise. A department is where its stock
# stands, which is the thing a player actually sees, and a carpet that agrees
# with the racking is a carpet somebody laid for a reason. Signs are kept as a
# second source for a department that has signage and no distinctly labelled
# stock - ELECTRONICS is exactly that, one sign and nothing else.
#
# Centroids rather than every point: nearest-point against 477 juniors
# fixtures and 18 ladies ones interleaves them into a speckle, and real
# departments are contiguous.
actors = subsystem.get_all_level_actors()

evidence = {}


# Height matters here too, one storey smaller. A `BSW_DEPT_ROUTE_POOL_..._
# LOWER_SHOES` is a ceiling light that names a department, which makes it a
# fine *mark* and a terrible *footprint*: six of them in a row at z = 490
# gave SHOES a 19 x 32 m block that is mostly empty floor, and 39% of the
# carpet. Only things standing on the floor say how far a department reaches.
FIXTURE_MAX_Z_CM = 300.0


def note(name, x, y, z):
    if name in PALETTE:
        evidence.setdefault(name, []).append((x, y, z))


for actor in actors:
    here = actor.get_actor_location()
    # Ground floor, and inside Boswell's. Both filters earned their place.
    if here.z >= MEZZANINE_UNDERSIDE_CM:
        continue
    if abs(here.x) > FLOOR_HALF_X or abs(here.y) > FLOOR_HALF_Y:
        continue

    label = actor.get_actor_label().upper()
    if label.startswith("BSW_DEPT_CARPET_"):
        continue
    # The wayfinding signs point into departments from the aisle; they are
    # not where the department is, and letting them speak was most of the
    # drag toward the middle.
    if "WAYFINDING" in label:
        continue

    for name in PALETTE:
        if name in label:
            note(name, here.x, here.y, here.z)
            break
    else:
        if isinstance(actor, unreal.TextRenderActor):
            comp = actor.text_render
            text = str(comp.text).strip().upper() if comp else ""
            note(text, here.x, here.y, here.z)

# A department with racking standing in a block and a department with one
# sign on a wall are not the same kind of evidence, and reducing both to a
# centroid compares them as though they were. JUNIORS has 468 fixtures across
# the west-south quadrant; LADIES, MENS and ELECTRONICS have a sign each,
# because their stock is labelled `BSW_APPAREL_LOWER_*` and names no
# department at all.
#
# So: a department with real racking claims **the footprint its racking
# occupies**. The rest of the floor goes to whichever department's mark is
# nearest. Standing among the juniors rails is standing in juniors, whatever
# is nearest in a straight line.
FOOTPRINT_MIN_POINTS = 5
TRIM = 0.05   # ignore the outermost 5% each way, so one stray rail two
              # departments over cannot stretch a block across the store


def span(values):
    ordered = sorted(values)
    lo = ordered[int(len(ordered) * TRIM)]
    hi = ordered[min(len(ordered) - 1, int(len(ordered) * (1.0 - TRIM)))]
    return lo, hi


centres = {}
footprints = {}
for name, pts in sorted(evidence.items(), key=lambda kv: -len(kv[1])):
    centres[name] = (sum(p[0] for p in pts) / len(pts),
                     sum(p[1] for p in pts) / len(pts))
    line = "DEPTFLOOR evidence %-12s n=%3d centre=(%6.0f,%6.0f)" % (
        name, len(pts), centres[name][0], centres[name][1])
    standing = [p for p in pts if p[2] < FIXTURE_MAX_Z_CM]
    if len(standing) >= FOOTPRINT_MIN_POINTS:
        x0, x1 = span([p[0] for p in standing])
        y0, y1 = span([p[1] for p in standing])
        footprints[name] = (x0, x1, y0, y1)
        line += " standing=%d footprint x=%6.0f..%6.0f y=%6.0f..%6.0f" % (
            len(standing), x0, x1, y0, y1)
    print(line)
if not centres:
    raise RuntimeError("DEPTFLOOR no ground-floor department evidence; the "
                       "carpet would be one colour, which is the state this "
                       "tool exists to leave behind")


def department_at(x, y):
    """Whose racking is this point standing in, or failing that, nearest."""
    # Insertion order is by stock count, so the better-evidenced department
    # wins where two footprints overlap.
    for name, (x0, x1, y0, y1) in footprints.items():
        if x0 <= x <= x1 and y0 <= y <= y1:
            return name
    return min(centres,
               key=lambda n: (centres[n][0] - x) ** 2 + (centres[n][1] - y) ** 2)


# ---- 3. clear the placeholder carpet and lay the new zoning -------------
removed = 0
for actor in actors:
    label = actor.get_actor_label()
    if label.startswith(PREFIX) or label.startswith("BSW_REALITY_CARPET_ZONE_"):
        subsystem.destroy_actor(actor)
        removed += 1
print("DEPTFLOOR removed=%d" % removed)

plane = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Plane")
if plane is None:
    raise RuntimeError("DEPTFLOOR /Engine/BasicShapes/Plane missing")

# 450, down from 650. The cell size is what decides how much of the space
# between the aisles actually gets carpeted: a 6.5 m cell cannot fit into the
# last 5 m against a wall, so it leaves bare tile there. Smaller cells pack
# the same area more closely and raise coverage without moving a single aisle
# - which matters now, because the aisle width is shared with the bulkhead
# ceiling laid over it in 86cbdfd, and narrowing one would silently
# un-align the other.
TILE_CM = 450.0
placed = 0
counts = {}

x = -FLOOR_HALF_X + PERIMETER_CM + TILE_CM * 0.5
while x <= FLOOR_HALF_X - PERIMETER_CM - TILE_CM * 0.5:
    y = -FLOOR_HALF_Y + PERIMETER_CM + TILE_CM * 0.5
    while y <= FLOOR_HALF_Y - PERIMETER_CM - TILE_CM * 0.5:
        # Leave the racetrack bare: the tile underneath is the walkway.
        # The cell CENTRE against the aisle, not its near edge. Testing the
        # edge widened the main aisle by a whole tile either side - 24.5 m
        # instead of 18 - and left only 21% of the floor carpeted, which is
        # the same warehouse the zoning exists to fix.
        in_main = abs(x) < MAIN_AISLE_HALF_X
        in_cross = abs(y) < CROSS_AISLE_HALF_Y
        if in_main or in_cross:
            y += TILE_CM
            continue

        name = department_at(x, y)
        # `spawn_actor_from_class` and then assign the mesh, rather than
        # `spawn_actor_from_object`, which returned None here for every call
        # and reported nothing about why.
        actor = subsystem.spawn_actor_from_class(
            unreal.StaticMeshActor, unreal.Vector(x, y, CARPET_Z))
        if actor is None:
            raise RuntimeError("DEPTFLOOR could not spawn a carpet actor")
        actor.set_actor_label("%s%s_%d_%d" % (PREFIX, name, int(x), int(y)))
        comp = actor.static_mesh_component
        # Mobility before the mesh: a StaticMeshActor spawns static, and a
        # static component refuses to be given geometry at runtime.
        comp.set_mobility(unreal.ComponentMobility.MOVABLE)
        comp.set_static_mesh(plane)
        actor.set_actor_scale3d(
            unreal.Vector(TILE_CM / PLANE_SIZE_CM, TILE_CM / PLANE_SIZE_CM, 1.0))
        comp.set_material(0, carpet_for(name))
        # Carpet is walked on, not collided with: the slab beneath already
        # carries the collision and the navmesh is built from it.
        comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        counts[name] = counts.get(name, 0) + 1
        placed += 1
        y += TILE_CM
    x += TILE_CM

print("DEPTFLOOR placed=%d by_department=%s" % (placed, counts))
floor_m2 = (FLOOR_HALF_X * 2.0) * (FLOOR_HALF_Y * 2.0) / 10000.0
carpet_m2 = placed * (TILE_CM / 100.0) ** 2
print("DEPTFLOOR carpet_m2=%.0f of sales_floor_m2=%.0f (%.0f%%) "
      "aisle_left_as_tile=yes" % (
          carpet_m2, floor_m2, 100.0 * carpet_m2 / floor_m2))

# The claim this tool has to be able to make, checked rather than asserted.
#
# **Every department with stock on this floor gets floor.** The first version
# gave JUNIORS - 477 fixtures, the largest department in the building - 11% of
# the carpet while SHOES took 38% on 29, and nothing noticed, because nothing
# compared the floor to the store standing on it. A department with racking
# and no carpet under it is a department the player cannot tell they are in.
for name in sorted(centres):
    if counts.get(name, 0) > 0:
        continue
    raise RuntimeError(
        "DEPTFLOOR %s has stock on this floor and got no carpet at all. Its "
        "centre is (%.0f, %.0f); either that is inside an aisle or the "
        "evidence for it is one stray actor somewhere it does not belong."
        % (name, centres[name][0], centres[name][1]))

biggest = max(counts, key=lambda n: counts[n])
print("DEPTFLOOR largest_share=%s %d of %d (%.0f%%) stock_rank=%s" % (
    biggest, counts[biggest], placed, 100.0 * counts[biggest] / placed,
    [n for n in sorted(evidence, key=lambda k: -len(evidence[k]))]))

unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
print("DEPTFLOOR saved=True")

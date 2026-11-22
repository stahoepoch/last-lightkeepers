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
PALETTE = {
    "LADIES": (0.19, 0.13, 0.17),
    "SHOES": (0.17, 0.15, 0.13),
    "JUNIORS": (0.12, 0.16, 0.18),
    "FABRICS": (0.16, 0.15, 0.11),
    "MENS": (0.11, 0.12, 0.16),
    "HOME": (0.18, 0.16, 0.13),
}
DEFAULT_COLOUR = (0.15, 0.14, 0.14)

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


def carpet_for(name):
    """One material instance per department, created on first use."""
    inst_name = "MI_LLK_Carpet_%s" % name.title()
    inst_path = "%s/%s" % (MATERIAL_DIR, inst_name)
    existing = unreal.EditorAssetLibrary.load_asset(inst_path)
    if existing is not None:
        return existing

    inst = tools.create_asset(inst_name, MATERIAL_DIR,
                              unreal.MaterialInstanceConstant,
                              unreal.MaterialInstanceConstantFactoryNew())
    unreal.MaterialEditingLibrary.set_material_instance_parent(inst, master)
    r, g, b = PALETTE.get(name.upper(), DEFAULT_COLOUR)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        inst, "BaseColour", unreal.LinearColor(r, g, b, 1.0))
    unreal.EditorAssetLibrary.save_asset(inst_path)
    print("DEPTFLOOR created_instance=%s rgb=(%.2f,%.2f,%.2f)" % (inst_name, r, g, b))
    return inst


# ---- 2. where the departments are, from the signs already in the map ----
actors = subsystem.get_all_level_actors()
signs = []
for actor in actors:
    if not isinstance(actor, unreal.TextRenderActor):
        continue
    comp = actor.text_render
    text = str(comp.text).strip().upper() if comp else ""
    if text and text in PALETTE:
        here = actor.get_actor_location()
        signs.append((text, here.x, here.y))
print("DEPTFLOOR signs_found=%d %s" % (len(signs), [s[0] for s in signs]))


def department_at(x, y):
    """The nearest department sign to a point, or None if the map has none."""
    if not signs:
        return None
    best = min(signs, key=lambda s: (s[1] - x) ** 2 + (s[2] - y) ** 2)
    return best[0]


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

TILE_CM = 650.0  # one carpet plane covers 6.5 m, small enough to follow the aisles
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

        name = department_at(x, y) or "HOME"
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
print("DEPTFLOOR carpet_m2=%.0f aisle_left_as_tile=yes" % (
    placed * (TILE_CM / 100.0) ** 2))

unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
print("DEPTFLOOR saved=True")

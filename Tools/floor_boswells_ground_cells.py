"""Lay the ground floor from the authored cell plan, not from guesswork.

Runs headlessly with `Tools/run_editor_python.ps1`.

## Why this replaces `Tools/floor_boswells_departments.py`

That tool had to **infer** where the departments were, because nothing had
authored it: it read merchandise centroids, built footprints from them, and
gave every remaining point to the nearest mark. It was the right answer to
the question it was asked, and the census of what it produced shows the cost
of asking that question at all:

    CENSUS material MI_LLK_Carpet_Shoes=31
    CENSUS material MI_LLK_Carpet_Juniors=21
    CENSUS material MI_LLK_Carpet_Mens=21
    CENSUS material MI_LLK_Carpet_Electronics=18
    CENSUS material MI_LLK_Carpet_Ladies=7

**LADIES is the largest department on this floor and it has the least
carpet.** It has 571 m2 in the plan and 7 planes in the map, while
ELECTRONICS, which has 18, is not a ground-floor department at all - section
13 of `docs/THE-MALL-IS-STILL-TRADING.md` puts the lower floor as juniors,
jewelry, mens, perfume, ladies, shoes, and the big CRT sets sit with juniors.
The inference was voting with the only evidence it had, which was racking
that is all labelled `BSW_APPAREL_LOWER_*` and names no department.

There is now an authored plan - `docs/reference/boswells/make_groundfloor_
plan.py` and the 27 cells it writes to `BoswellsGroundFloor_Cells.csv` - so
the floor can simply be laid where the plan says. That is decision row 81's
licence to derive: the derivation is the plan, and it is written down.

**The ELECTRONICS conflict, cited rather than quietly dropped.** The palette
in the old tool, the `MI_LLK_Carpet_Electronics` asset, and one sign at
(1600, 650) all say ELECTRONICS is here. Section 13 says it is not. This tool
follows section 13 and leaves the material asset and the sign alone: no actor
it places uses that carpet, and nothing is deleted that a decision has not
covered.

## What it lays

**One plane per cell, sized to the cell**, rather than a global grid of
450 cm tiles. The grid could only approximate a rectangle, which is why the
old tool left bare tile against the walls and needed a `TILE_CM` tuned for
coverage. A cell is a rectangle and the plan gives its corners, so the carpet
is exact, the walkway between two cells is bare tile *by construction*, and
the whole floor is 20 actors instead of 98.

That is what makes the walkway rule visible underfoot. The aisles in
`docs/plans/BOSWELLS-WALKWAY-AND-CEILING-RULES.md` are three child steps -
180 cm - and because the carpet stops exactly at the cell edge, the player
sees 180 cm of hard floor between departments. Nothing has to draw the aisle;
it is the gap.

**The hard-floor cells are a real-life detail, not an omission.** `kind` on
each cell decides the surface. Cosmetics, perfume, jewelry and accessories
counters stand on polished hard floor in every anchor store there has ever
been - bright, specular, wipe-clean - and so does a lift lobby, a service
desk and a stock room. Only apparel, shoes and the fitting rooms get carpet.
So of the 27 cells, 20 are carpeted and 7 are left as the stone tile
underneath, and the front-of-house band the player reaches last changes
underfoot.

Unverified, and worth saying: the carpet is still a flat colour. `M_LLK_
DeptCarpet` has a `BaseColour` and a `Roughness` and no texture, so these
read as matte colour fields rather than as weave. That was already on the
list before this tool and it still is.

Idempotent: everything it creates is `BSW_GROUND_CARPET_*`, and it removes
those plus the two older generations - `BSW_DEPT_CARPET_*` from the inferred
pass and `BSW_REALITY_CARPET_ZONE_*` from the placeholder pass - so two
floors can never disagree where the zoning changed.
"""

import csv
from pathlib import Path

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_GROUND_CARPET_"
SUPERSEDED = ("BSW_DEPT_CARPET_", "BSW_REALITY_CARPET_ZONE_")
MATERIAL_DIR = "/Game/LLK/Boswells/Architecture"
MASTER_NAME = "M_LLK_DeptCarpet"
CSV_PATH = (Path(unreal.Paths.project_dir()) /
            "docs/reference/boswells/BoswellsGroundFloor_Cells.csv")

CARPET_Z = 3.0            # above the slab at -15 and the old zones at 2
PLANE_SIZE_CM = 100.0     # /Engine/BasicShapes/Plane is 100 cm square

# Which cells are carpeted. A counter hall, a lift lobby, a service desk and
# a stock room are hard floor in a real store, so they are hard floor here.
CARPETED_KINDS = ("apparel", "shoes", "fitting")

# Muted, dark enough to hide traffic, different enough to be felt underfoot.
# The first four are the colours the old tool used, kept so that this is a
# re-zoning and not also a re-colouring - one change at a time.
PALETTE = {
    "ladies":  (0.19, 0.13, 0.17),
    "shoes":   (0.17, 0.15, 0.13),
    "juniors": (0.12, 0.16, 0.18),
    "mens":    (0.11, 0.12, 0.16),
    "service": (0.14, 0.14, 0.13),
}
DEFAULT_COLOUR = (0.15, 0.14, 0.14)

# The plan's own numbers, so this tool fails loudly if the plan moves under
# it rather than laying a floor nobody checked.
EXPECT_CELLS = 27
PLATE = (-4350.0, -2700.0, 4350.0, 2090.0)
WALKWAY = 180.0


def require(condition, message):
    if not condition:
        raise RuntimeError("GROUNDFLOOR " + message)


# ---- 1. the plan -------------------------------------------------------
require(CSV_PATH.is_file(), "cell plan missing at %s" % CSV_PATH)
with CSV_PATH.open(newline="", encoding="utf-8") as handle:
    cells = list(csv.DictReader(handle))
require(len(cells) == EXPECT_CELLS,
        "expected %d cells, plan has %d" % (EXPECT_CELLS, len(cells)))
require(len({row["name"] for row in cells}) == len(cells),
        "duplicate cell names in the plan")

for row in cells:
    x0, y0 = float(row["xmin_cm"]), float(row["ymin_cm"])
    x1, y1 = float(row["xmax_cm"]), float(row["ymax_cm"])
    require(x1 > x0 and y1 > y0, "cell %s has no area" % row["name"])
    require(PLATE[0] <= x0 and x1 <= PLATE[2] and
            PLATE[1] <= y0 and y1 <= PLATE[3],
            "cell %s falls outside the sales plate" % row["name"])

laying = [row for row in cells if row["kind"] in CARPETED_KINDS]
require(laying, "no cell has a carpeted kind; nothing would be laid")

# Every department with apparel on this floor has to end up with floor under
# it. This is the check the old tool learned to make the hard way, kept.
wanted = {row["department"] for row in laying}

world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "map unavailable")

subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()

# ---- 2. the master material, reused ------------------------------------
master_path = "%s/%s" % (MATERIAL_DIR, MASTER_NAME)
master = unreal.EditorAssetLibrary.load_asset(master_path)
require(master is not None,
        "master material %s missing; Tools/floor_boswells_departments.py "
        "creates it and this tool deliberately does not duplicate that"
        % master_path)
print("GROUNDFLOOR master=%s" % master_path)

_cache = {}


def carpet_for(department):
    """One material instance per department, written every run.

    Written rather than returned-if-present, for the reason the old tool
    records: an instance that is reused untouched makes a palette edit
    indistinguishable from a run that did nothing.
    """
    if department in _cache:
        return _cache[department]

    name = "MI_LLK_Carpet_%s" % department.title()
    path = "%s/%s" % (MATERIAL_DIR, name)
    inst = unreal.EditorAssetLibrary.load_asset(path)
    verb = "updated"
    if inst is None:
        verb = "created"
        inst = tools.create_asset(name, MATERIAL_DIR,
                                  unreal.MaterialInstanceConstant,
                                  unreal.MaterialInstanceConstantFactoryNew())
        unreal.MaterialEditingLibrary.set_material_instance_parent(inst, master)
    require(inst is not None, "could not make %s" % path)

    r, g, b = PALETTE.get(department, DEFAULT_COLOUR)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        inst, "BaseColour", unreal.LinearColor(r, g, b, 1.0))
    unreal.EditorAssetLibrary.save_asset(path)
    print("GROUNDFLOOR %s %s rgb=(%.2f,%.2f,%.2f)" % (verb, name, r, g, b))
    _cache[department] = inst
    return inst


# ---- 3. clear the older generations ------------------------------------
removed = {}
for actor in subsystem.get_all_level_actors():
    label = actor.get_actor_label()
    for stale in (PREFIX,) + SUPERSEDED:
        if label.startswith(stale):
            subsystem.destroy_actor(actor)
            removed[stale] = removed.get(stale, 0) + 1
            break
print("GROUNDFLOOR removed=%s" % (removed or "{}"))

# ---- 4. lay one plane per cell -----------------------------------------
plane = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Plane")
require(plane is not None, "/Engine/BasicShapes/Plane missing")

placed = {}
carpet_m2 = 0.0
for row in laying:
    x0, y0 = float(row["xmin_cm"]), float(row["ymin_cm"])
    x1, y1 = float(row["xmax_cm"]), float(row["ymax_cm"])
    width, length = x1 - x0, y1 - y0
    centre = unreal.Vector((x0 + x1) * 0.5, (y0 + y1) * 0.5, CARPET_Z)

    actor = subsystem.spawn_actor_from_class(unreal.StaticMeshActor, centre)
    require(actor is not None, "could not spawn carpet for %s" % row["name"])
    slug = "".join(ch if ch.isalnum() else "_" for ch in row["name"].upper())
    actor.set_actor_label("%s%s_%s" % (PREFIX, row["department"].upper(), slug))

    comp = actor.static_mesh_component
    # Mobility before the mesh: a StaticMeshActor spawns static, and a static
    # component refuses to be given geometry.
    comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    comp.set_static_mesh(plane)
    actor.set_actor_scale3d(unreal.Vector(width / PLANE_SIZE_CM,
                                          length / PLANE_SIZE_CM, 1.0))
    comp.set_material(0, carpet_for(row["department"]))
    # Carpet is walked on, not collided with: the slab beneath carries the
    # collision and the navmesh is built from it.
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)

    placed[row["department"]] = placed.get(row["department"], 0) + 1
    carpet_m2 += width * length / 10000.0

plate_m2 = ((PLATE[2] - PLATE[0]) * (PLATE[3] - PLATE[1])) / 10000.0
print("GROUNDFLOOR placed=%d cells by_department=%s" % (len(laying), placed))
print("GROUNDFLOOR carpet_m2=%.0f of plate_m2=%.0f (%.0f%%)" % (
    carpet_m2, plate_m2, 100.0 * carpet_m2 / plate_m2))
print("GROUNDFLOOR hard_floor_cells=%s" % [
    row["name"] for row in cells if row["kind"] not in CARPETED_KINDS])

# ---- 5. the claims, checked rather than asserted -----------------------
for department in sorted(wanted):
    require(placed.get(department, 0) > 0,
            "%s has cells in the plan and got no carpet" % department)

# No carpet may cross into a walkway. Two cells in the plan are a walkway
# apart, so two carpets that touch would mean a plane is the wrong size -
# which is the one way this tool can silently undo the rule it exists to
# make visible.
boxes = [(row["name"],
          float(row["xmin_cm"]), float(row["ymin_cm"]),
          float(row["xmax_cm"]), float(row["ymax_cm"]))
         for row in laying]
for i, a in enumerate(boxes):
    for b in boxes[i + 1:]:
        overlap_x = min(a[3], b[3]) - max(a[1], b[1])
        overlap_y = min(a[4], b[4]) - max(a[2], b[2])
        require(overlap_x <= 0.5 or overlap_y <= 0.5,
                "carpets %s and %s overlap" % (a[0], b[0]))

print("GROUNDFLOOR walkway=%.0f cm shows as bare tile between every cell"
      % WALKWAY)

unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
print("GROUNDFLOOR saved=True")

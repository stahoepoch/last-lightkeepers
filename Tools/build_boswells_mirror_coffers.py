"""Put the four mirrored diamonds in the lower ceiling.

Runs headlessly with `Tools/run_editor_python.ps1`.

Chris, 2026-11-23, twice:

    "The glass should be in each four corners of the square, and the middle
    part is where the escalators are, the high ceiling with the chandelier
    ... an oblongish square of glass in each corner, like a diamond. So it's
    a square with diamonds."

    "Mirrored ceiling!!! My bad. Boscovs mirror ceiling."

Drawn in `docs/reference/boswells/BoswellsGroundFloor_Ceiling.svg`, specified
in `docs/plans/BOSWELLS-WALKWAY-AND-CEILING-RULES.md`, and until now that is
all it was.

## What this builds, and what it does not

**Four mirror panels, 1400 x 950 cm, turned 45 degrees, at (+/-2625,
+/-1200), flush in the suspended ceiling plane.** 133 m2 each, 532 m2 in all,
set in the white tile with the troffers and light housings hanging below them
where the mirror will double them - which is what
`docs/reference/boswells/source-photos/07-mirrored-atrium.jpg` shows.

**It does not build the 124 cm recess or the sloped facets**, and that is a
decision rather than an omission. The lower ceiling is one slab per wing -
`BSW_LOWER_CEILING_WEST_FIELD` and `_EAST_FIELD`, 3450 x 4800 x 4 cm at
z 516 from `Tools/build_boswells_lower_ceiling.py` - so a coffer that rises
into the plenum needs a hole in it, and a hole needs that slab replaced by a
ring of thirteen pieces per wing. Worse, the opening is a rotated rectangle
and the slab is axis aligned, so the ring leaves four triangles at each
corner that neither an axis-aligned nor a 45-degree piece can fill without
overlapping, and `ALLKTestGameMode::ReportFloorWithoutCeiling` would be right
to call each one a hole. That is a real change to geometry another pass owns,
with its own way of going wrong, and it belongs in its own run with its own
check rather than riding along inside this one.

Flush costs almost nothing in the meantime. At a 16.6 m span across the
diamond a 124 cm recess is a shallow dish; what the player gets from the
corners is the reflection, and the reflection is the same either way.

## Why this is worth building before anything else in the ceiling

The mirror is the only thing on this floor that does work without a script.
The keeper's lantern is the one moving light in the building, and 532 m2 of
mirror overhead gives it a twin that moves when the player moves and lights
nothing - decided in BOSWELLS-WALKWAY-AND-CEILING-RULES.md section 3, which
keeps `LLKLightField` lantern-only because a real mirror redirects rather
than emits. And a diamond ahead of the player reflects the aisle behind them.

## Material

`M_BSW_SmokedMirror`, which the atrium canopy already uses, rather than a new
bright mirror. Two reasons: the corners and the middle are then one ceiling
family rather than two, and a dark store wants a mirror that gives back less
than it gets. If it reads too dark in a field check it is one instance.

Idempotent: everything it creates is `BSW_CEILING_MIRROR_*` and those are
removed first.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_CEILING_MIRROR_"
MIRROR = "/Game/LLK/Boswells/Architecture/M_BSW_SmokedMirror"

# From docs/reference/boswells/make_groundfloor_plan.py, which is the plan of
# record. Repeated here rather than imported because that module is not on the
# editor's path, and checked below against the geometry in the map.
DIAMOND = (1400.0, 950.0)
DIAMOND_AT = ((-2625.0, -1200.0), (2625.0, -1200.0),
              (-2625.0, 1200.0), (2625.0, 1200.0))
TILE_FIELDS = ((-4350.0, -2700.0, -900.0, 2100.0),
               (900.0, -2700.0, 4350.0, 2100.0))

SALES_CEILING = 514.0      # the tile underside
MIRROR_Z = 513.0           # 1 cm clear below it, so the mirror is what you see
THICKNESS = 1.0
YAW = 45.0

# The slabs this tool deliberately leaves intact.
FIELD_LABELS = ("BSW_LOWER_CEILING_WEST_FIELD", "BSW_LOWER_CEILING_EAST_FIELD")


def require(condition, message):
    if not condition:
        raise RuntimeError("MIRROR " + message)


# ---- 1. the plan has to fit the building -------------------------------
#
# A panel is a rotated rectangle, so what has to sit inside a tile field is
# its turned bounding box, not its width and length. 1400 x 950 turned 45
# degrees spans 1662 cm each way.
HALF = (DIAMOND[0] + DIAMOND[1]) * 0.5 * 0.70710678
for cx, cy in DIAMOND_AT:
    inside = [f for f in TILE_FIELDS
              if f[0] <= cx - HALF and cx + HALF <= f[2]
              and f[1] <= cy - HALF and cy + HALF <= f[3]]
    require(len(inside) == 1,
            "a mirror at (%.0f, %.0f) spans %.0f cm each way and is not "
            "wholly inside one tile field; it would hang over the painted "
            "aisle bulkhead or off the end of the ceiling"
            % (cx, cy, 2 * HALF))

world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "map unavailable")

subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
existing = {a.get_actor_label(): a for a in subsystem.get_all_level_actors()}

# The ceiling this hangs in has to actually be there. If the fields were ever
# cut into a ring, the premise of this tool changed and it should be rewritten
# rather than run.
for label in FIELD_LABELS:
    require(label in existing,
            "%s is missing; this tool assumes the lower ceiling is one slab "
            "per wing and must be revisited if that changed" % label)
    centre = existing[label].get_actor_location()
    require(abs(centre.z - 516.0) < 1.0,
            "%s is at z=%.1f, not 516; the mirror plane would be wrong"
            % (label, centre.z))

plane = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
require(isinstance(plane, unreal.StaticMesh), "cube source missing")
mirror = unreal.EditorAssetLibrary.load_asset(MIRROR)
require(isinstance(mirror, unreal.MaterialInterface),
        "mirror material missing at %s" % MIRROR)

# ---- 2. clear any earlier run ------------------------------------------
removed = 0
for label, actor in existing.items():
    if label.startswith(PREFIX):
        subsystem.destroy_actor(actor)
        removed += 1
print("MIRROR removed=%d" % removed)

# ---- 3. hang the diamonds ----------------------------------------------
placed = 0
for cx, cy in DIAMOND_AT:
    actor = subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(cx, cy, MIRROR_Z),
        unreal.Rotator(pitch=0.0, yaw=YAW, roll=0.0))
    require(actor is not None, "spawn failed at (%.0f, %.0f)" % (cx, cy))
    actor.set_actor_label("%s%d_%d" % (PREFIX, int(cx), int(cy)))
    actor.set_folder_path("Boswells/LowerSales/Ceiling")

    comp = actor.static_mesh_component
    comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    require(comp.set_static_mesh(plane), "mesh failed at (%.0f, %.0f)" % (cx, cy))
    actor.set_actor_scale3d(unreal.Vector(DIAMOND[0] / 100.0,
                                          DIAMOND[1] / 100.0,
                                          THICKNESS / 100.0))
    comp.set_material(0, mirror)
    # A ceiling is looked at, not walked into, and the navmesh should never
    # hear about it.
    actor.set_actor_enable_collision(False)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    if hasattr(comp, "set_can_ever_affect_navigation"):
        comp.set_can_ever_affect_navigation(False)
    placed += 1

each_m2 = DIAMOND[0] * DIAMOND[1] / 10000.0
print("MIRROR placed=%d each=%.0fx%.0f cm yaw=%.0f at z=%.1f (tile %.1f)"
      % (placed, DIAMOND[0], DIAMOND[1], YAW, MIRROR_Z, SALES_CEILING))
print("MIRROR area_m2=%.0f (%.0f m2 each) span_each_way_cm=%.0f"
      % (placed * each_m2, each_m2, 2 * HALF))
print("MIRROR recess=none this pass; the wing fields are one slab each and "
      "are left intact")

# ---- 4. the claims, checked --------------------------------------------
require(placed == len(DIAMOND_AT),
        "placed %d of %d panels" % (placed, len(DIAMOND_AT)))

after = [a for a in subsystem.get_all_level_actors()
         if a.get_actor_label().startswith(PREFIX)]
require(len(after) == len(DIAMOND_AT),
        "map holds %d mirrors after the run, not %d" % (len(after),
                                                        len(DIAMOND_AT)))
# Nothing may sit between a mirror and the floor at the ceiling plane, or the
# player looks up at the back of some other piece of ceiling instead.
for actor in after:
    here = actor.get_actor_location()
    require(here.z < SALES_CEILING,
            "a mirror at (%.0f, %.0f) is at z=%.1f, at or above the tile "
            "underside; it would be hidden" % (here.x, here.y, here.z))

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP),
        "map save failed")
print("MIRROR saved=True")

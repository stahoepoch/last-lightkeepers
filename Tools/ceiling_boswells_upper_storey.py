"""Give the upper storey a ceiling at 400 cm, and re-hang what it would bury.

Chris, 2026-11-23, answering the one question
`docs/FINDINGS-FIFTY-LIGHTS-ON-NO-CIRCUIT.md` left open: **"Build it at 400."**

## What was there

Nothing. Not a low ceiling - that complaint turned out to be a bad
measurement of mine, and the deck's median headroom was **838 cm**. The real
defect was that the upper selling floor had no ceiling *height*:

- **89%** of the deck open to `ANCHOR_FULL_ROOF`, 838 cm overhead
- **10%** under `BSW_SERVICE_CORE_SERVICE_CEILING` at 375 cm
- the kids department under a tile field at 448 cm, **with no collision**, so
  nothing could measure it and nothing could be stopped by it

Three heights and a void on one floor. 400 cm is a real anchor's upper
selling floor, the service core's 375 is a service drop and stays, and the
kids field comes down to the new datum so the storey has one.

## The thing that nearly made this a black floor

The probe written for this change found what the lighting pass missed:
**the mains and emergency grid exists twice.** `BSW_MAINS_*` and
`BSW_EMERGENCY_FILL_*` hang once below the deck, lighting the ground floor -
which is the copy yesterday's lighting pass measured - and once at **z 1462**,
812 cm above the deck in the roof space, under the same family names. The
high copy is the upper storey's lighting, and it has been lighting the
mezzanine from the roof all along, which is exactly right while that deck is
open to the sky. Only their z tells the two storeys apart.

So a ceiling laid at 1050 with nothing else done would have put every light
the upper floor has above it, and Chris would have opened the map to a
department lit by nothing at all. Every light over a cell this tool roofs is
re-hung 26 cm under the new ceiling - the drop this project already uses
everywhere - and its housing mesh moves with it.

**A light over the atrium stays where it is.** The atrium is open to the roof
on purpose, and a light in an atrium hangs from the roof; moving those down
would also drop them 438 cm nearer the ground floor, through the one hole in
its ceiling, and brighten a floor Chris has just finished reviewing.

Nothing's intensity, colour or circuit tag is touched here. The lighting
change was the previous commit and is Chris's to judge against a picture;
this tool only moves things in z.

Idempotent: everything it creates is `BSW_UPPER_CEILING_*` and is destroyed
first, and every move is to an absolute height rather than by an offset.
Saves the map, so it needs `-AllowUnsafeMapSave`.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_UPPER_CEILING_"
TILE_MATERIAL = "/Game/LLK/Boswells/Architecture/M_BSW_LowerCeilingTile"

DECK = 650.0                 # top of ANCHOR_UPPER_*
CLEAR_CM = 400.0             # Chris, 2026-11-23
UNDERSIDE = DECK + CLEAR_CM  # 1050
THICKNESS_CM = 6.0

# The kids field comes down to the datum too, but 4 cm higher than the general
# field, so where the two meet the new tile reads in front of the old grid
# instead of fighting it for the same plane. Same trick as the aisle soffit.
KIDS_UNDERSIDE = UNDERSIDE + 4.0
KIDS_FAMILIES = ("BSW_KIDS_CEILING_TILE_FIELD", "BSW_KIDS_CEILING_GRID_")

# A fitting hangs 26 cm under the ceiling it belongs to. Already the drop used
# by the ground floor and by yesterday's kids-fitting repair.
FITTING_DROP_CM = 26.0
HANG_Z = UNDERSIDE - FITTING_DROP_CM

CELL_CM = 300.0
PLANE_SIZE_CM = 100.0
ESCALATOR_CLEARANCE_CM = 120.0


def say(message):
    unreal.log("UPPERCEIL " + message)
    print("UPPERCEIL " + message)


def require(ok, message):
    if not ok:
        raise RuntimeError("UPPERCEIL " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = subsystem.get_all_level_actors()

tile = unreal.EditorAssetLibrary.load_asset(TILE_MATERIAL)
require(tile is not None,
        "the ground floor's ceiling tile material is gone: " + TILE_MATERIAL)
cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
require(cube is not None, "/Engine/BasicShapes/Cube missing")


# =====================================================================
# 1. The footprint, derived from the map rather than typed
# =====================================================================

decks = []
for actor in actors:
    if actor.get_actor_label().startswith("ANCHOR_UPPER_"):
        decks.append(actor.get_actor_bounds(False))
require(len(decks) >= 4, "fewer than four ANCHOR_UPPER_ decks; the atrium "
        "void cannot be derived and this tool would cap it")

escalators = []
for actor in actors:
    label = actor.get_actor_label().upper()
    if "ESCALATOR" not in label and "ESC_" not in label:
        continue
    centre, extent = actor.get_actor_bounds(False)
    if extent.x < 60.0 and extent.y < 60.0:
        continue
    escalators.append((centre, extent))

# Lids that already roof part of the deck. One below the new datum is a
# service drop and keeps its cells; one above it is the kids field and comes
# down, so its cells are skipped as well.
lower_lids = []
kids = []
for actor in actors:
    label = actor.get_actor_label()
    centre, extent = actor.get_actor_bounds(False)
    base = centre.z - extent.z
    if label.startswith(KIDS_FAMILIES):
        kids.append(actor)
        continue
    if base < DECK + 200.0 or extent.z > 60.0:
        continue
    if extent.x < 300.0 or extent.y < 300.0:
        continue
    if base > UNDERSIDE:
        continue
    # On this deck, not somewhere else in the level. The first run of this
    # counted `LLK_DOCK_CHAPTER_WING_1_CEILING` and `_WING_3_CEILING` as lids
    # over the mezzanine; they are 150 m south of it, at y -22200..-10900, in
    # another chapter's dock. They could never have claimed a cell, but a
    # report that names them is a report that cannot be checked.
    if (centre.x - extent.x > 4350.0 or centre.x + extent.x < -4350.0
            or centre.y - extent.y > 2700.0 or centre.y + extent.y < -2700.0):
        continue
    lower_lids.append((centre, extent, label, base))

for centre, extent, label, base in lower_lids:
    say("existing lid kept, it hangs lower than the new ceiling: %s at %.0f cm "
        "(%.0f clear over the deck)" % (label, base, base - DECK))

kids_box = None
for actor in kids:
    centre, extent = actor.get_actor_bounds(False)
    box = (centre.x - extent.x, centre.y - extent.y,
           centre.x + extent.x, centre.y + extent.y)
    if kids_box is None:
        kids_box = list(box)
    else:
        kids_box[0] = min(kids_box[0], box[0])
        kids_box[1] = min(kids_box[1], box[1])
        kids_box[2] = max(kids_box[2], box[2])
        kids_box[3] = max(kids_box[3], box[3])
require(kids_box is not None, "the kids ceiling field is gone; it was "
        "x -3900..-1100 y -2050..-250 at 1098 when this was written")
say("kids ceiling field: x %.0f..%.0f y %.0f..%.0f, %d actors"
    % (kids_box[0], kids_box[2], kids_box[1], kids_box[3], len(kids)))

bounds = [1e9, 1e9, -1e9, -1e9]
for centre, extent in decks:
    bounds[0] = min(bounds[0], centre.x - extent.x)
    bounds[1] = min(bounds[1], centre.y - extent.y)
    bounds[2] = max(bounds[2], centre.x + extent.x)
    bounds[3] = max(bounds[3], centre.y + extent.y)
say("deck footprint: x %.0f..%.0f y %.0f..%.0f"
    % (bounds[0], bounds[2], bounds[1], bounds[3]))


def on_deck(x, y):
    return any(abs(x - c.x) <= e.x and abs(y - c.y) <= e.y for c, e in decks)


def over_escalator(x, y):
    return any(abs(x - c.x) <= e.x + ESCALATOR_CLEARANCE_CM and
               abs(y - c.y) <= e.y + ESCALATOR_CLEARANCE_CM
               for c, e in escalators)


def wholly_under_a_lid(x, y):
    half = CELL_CM * 0.5
    return any(abs(x - c.x) + half <= e.x and abs(y - c.y) + half <= e.y
               for c, e, lab, base in lower_lids)


def centre_in_kids(x, y):
    return (kids_box[0] <= x <= kids_box[2]
            and kids_box[1] <= y <= kids_box[3])


# =====================================================================
# 2. Lay it
# =====================================================================

removed = 0
for actor in actors:
    if actor.get_actor_label().startswith(PREFIX):
        subsystem.destroy_actor(actor)
        removed += 1
say("cleared %d pieces from a previous run" % removed)

# What roofs each cell, and at what height. A cell under the service core's
# lid is roofed too, just 25 cm lower, and the first run of this treated it as
# open sky - which left four emergency fills hanging at 1462 above a lid at
# 1025, in the one part of the floor that is NOT open to the roof. "Is there a
# ceiling here" and "did this tool lay it" are different questions.
cover = {}
roofed = []
placed = skipped_void = skipped_esc = skipped_lid = skipped_kids = 0
x = bounds[0] + CELL_CM * 0.5
while x <= bounds[2] - CELL_CM * 0.5 + 1.0:
    y = bounds[1] + CELL_CM * 0.5
    while y <= bounds[3] - CELL_CM * 0.5 + 1.0:
        skip = True
        if not on_deck(x, y):
            skipped_void += 1
        elif over_escalator(x, y):
            skipped_esc += 1
        elif wholly_under_a_lid(x, y):
            skipped_lid += 1
            cover[(x, y)] = min(base for c, e, lab, base in lower_lids
                                if abs(x - c.x) <= e.x and abs(y - c.y) <= e.y)
        elif centre_in_kids(x, y):
            skipped_kids += 1
            cover[(x, y)] = KIDS_UNDERSIDE
        else:
            skip = False
        if skip:
            y += CELL_CM
            continue
        actor = subsystem.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(x, y, UNDERSIDE + THICKNESS_CM * 0.5))
        require(actor is not None, "could not spawn a ceiling piece")
        actor.set_actor_label("%s%d_%d" % (PREFIX, int(x), int(y)))
        comp = actor.static_mesh_component
        comp.set_mobility(unreal.ComponentMobility.MOVABLE)
        comp.set_static_mesh(cube)
        actor.set_actor_scale3d(unreal.Vector(
            CELL_CM / PLANE_SIZE_CM, CELL_CM / PLANE_SIZE_CM,
            THICKNESS_CM / PLANE_SIZE_CM))
        comp.set_material(0, tile)
        # Query-only, not none. A ceiling 400 cm up is never walked into by a
        # 134 cm child, so this blocks nothing in practice - but the kids
        # field at 448 had NO_COLLISION, which meant no trace could find it
        # and every audit of this floor reported that department as open to
        # the sky. A ceiling nobody can measure is a ceiling nobody can prove
        # is there.
        comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_ONLY)
        if hasattr(comp, "set_can_ever_affect_navigation"):
            comp.set_can_ever_affect_navigation(False)
        roofed.append((x, y))
        cover[(x, y)] = UNDERSIDE
        placed += 1
        y += CELL_CM
    x += CELL_CM

say("laid %d pieces, %.0f m2 at %.0f cm underside (%.0f clear over the deck)"
    % (placed, placed * (CELL_CM / 100.0) ** 2, UNDERSIDE, CLEAR_CM))
say("skipped: %d atrium void, %d escalator, %d already under a lower lid, "
    "%d kids field" % (skipped_void, skipped_esc, skipped_lid, skipped_kids))
require(placed > 0, "nothing was roofed")


# =====================================================================
# 3. Bring the kids field down to the datum
# =====================================================================

kids_moved = 0
for actor in kids:
    centre, extent = actor.get_actor_bounds(False)
    base = centre.z - extent.z
    delta = KIDS_UNDERSIDE - base
    if abs(delta) < 0.5:
        continue
    loc = actor.get_actor_location()
    actor.set_actor_location(
        unreal.Vector(loc.x, loc.y, loc.z + delta), False, True)
    kids_moved += 1
    if actor.get_actor_label().startswith("BSW_KIDS_CEILING_TILE_FIELD"):
        # Same argument as above: it had no collision, so no audit of this
        # floor could see it. The grid bars stay uncollided - they are 2 cm of
        # trim, and the field behind them is what a trace should find.
        for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
            comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_ONLY)
            if hasattr(comp, "set_can_ever_affect_navigation"):
                comp.set_can_ever_affect_navigation(False)
        actor.set_actor_enable_collision(True)
say("kids ceiling brought down to %.0f cm underside (%.0f clear): %d of %d "
    "actors moved" % (KIDS_UNDERSIDE, KIDS_UNDERSIDE - DECK, kids_moved,
                      len(kids)))


# =====================================================================
# 4. Re-hang every light the new ceiling would bury
# =====================================================================

def ceiling_at(x, y):
    """The underside of whatever roofs this point, or None for open sky."""
    half = CELL_CM * 0.5
    found = None
    for (cx, cy), z in cover.items():
        if abs(x - cx) <= half and abs(y - cy) <= half:
            found = z if found is None else min(found, z)
    if found is None and centre_in_kids(x, y):
        return KIDS_UNDERSIDE
    return found


def is_roofed(x, y):
    return ceiling_at(x, y) is not None


by_label = {}
for actor in actors:
    by_label.setdefault(actor.get_actor_label(), []).append(actor)

rehung = 0
rehung_fixtures = 0
left_in_the_roof = []
for actor in actors:
    if not isinstance(actor, unreal.Light):
        continue
    loc = actor.get_actor_location()
    label = actor.get_actor_label()
    lid = ceiling_at(loc.x, loc.y)
    if lid is None:
        # Over the atrium or an escalator well: both are open to the roof,
        # and a light in an atrium hangs from the roof.
        if loc.z >= UNDERSIDE:
            left_in_the_roof.append((label, loc.z))
        continue
    hang = lid - FITTING_DROP_CM
    if loc.z < lid:
        continue
    delta = hang - loc.z
    actor.set_actor_location(unreal.Vector(loc.x, loc.y, hang), False, True)
    rehung += 1
    # The housing mesh moves the same distance, or the pool appears four
    # metres below the fitting that is supposed to be making it.
    for candidate in ("BSW_FIXTURE_" + label, label + "_Housing"):
        for mesh in by_label.get(candidate, []):
            mloc = mesh.get_actor_location()
            mesh.set_actor_location(
                unreal.Vector(mloc.x, mloc.y, mloc.z + delta), False, True)
            rehung_fixtures += 1

say("lights re-hung %.0f cm under the new ceiling (z %.0f): %d lights, "
    "%d housing meshes" % (FITTING_DROP_CM, HANG_Z, rehung, rehung_fixtures))
say("left in the roof space, because they are over the open atrium or an "
    "escalator well: %d" % len(left_in_the_roof))
for label, z in left_in_the_roof[:12]:
    say("  roof %-46s z=%.0f" % (label, z))
# Not "something moved" - that fails the second time this is run, and a tool
# that cannot be run twice is a tool nobody dares run once. The thing worth
# asserting is the end state: no light under a ceiling is above it.
stranded = []
for actor in actors:
    if not isinstance(actor, unreal.Light):
        continue
    loc = actor.get_actor_location()
    lid = ceiling_at(loc.x, loc.y)
    if lid is not None and loc.z >= lid:
        stranded.append((actor.get_actor_label(), loc.z, lid))
for label, z, lid in stranded[:10]:
    say("  STRANDED %-44s z=%.0f, ceiling %.0f" % (label, z, lid))
require(not stranded, "%d light(s) are still above the ceiling over them, so "
        "part of this floor is lit by nothing" % len(stranded))


# =====================================================================
# 5. Hanging signage hangs FROM the ceiling, so it cannot go through one
# =====================================================================
#
# The first run of this left `BSW_KIDS_SIGN_DIRECTORY_PANEL` spanning
# 965..1115 under a ceiling at 1054 - a directory board with its top 61 cm
# inside the tile, which is the first thing anyone would see on walking into
# that department. A sign is moved as a whole assembly, by the one drop its
# worst member needs, or the board and its lettering come apart.
SIGN_FAMILY = "BSW_KIDS_SIGN_"
SIGN_GAP_CM = 2.0

signs = {}
for actor in actors:
    label = actor.get_actor_label()
    if not label.startswith(SIGN_FAMILY):
        continue
    centre, extent = actor.get_actor_bounds(False)
    if centre.z - extent.z < DECK - 50.0:
        continue
    signs.setdefault("_".join(label.split("_")[:4]), []).append(
        (actor, centre.z + extent.z))

signs_moved = 0
for key, members in sorted(signs.items()):
    drop = 0.0
    for actor, top in members:
        loc = actor.get_actor_location()
        lid = ceiling_at(loc.x, loc.y)
        if lid is None:
            continue
        drop = min(drop, (lid - SIGN_GAP_CM) - top)
    if drop > -0.5:
        continue
    for actor, top in members:
        loc = actor.get_actor_location()
        actor.set_actor_location(
            unreal.Vector(loc.x, loc.y, loc.z + drop), False, True)
        signs_moved += 1
    say("sign %s dropped %.0f cm to hang under the ceiling (%d pieces)"
        % (key, -drop, len(members)))
say("hanging signage: %d pieces moved across %d assemblies"
    % (signs_moved, len(signs)))


# =====================================================================
# 6. What still pokes through, reported rather than resized
# =====================================================================

lid_labels = set(lab for c, e, lab, base in lower_lids)
through = []
for actor in actors:
    label = actor.get_actor_label()
    if label.startswith(PREFIX) or label.startswith(KIDS_FAMILIES):
        continue
    if label in lid_labels:
        continue    # a lid reporting itself as poking through itself
    centre, extent = actor.get_actor_bounds(False)
    base = centre.z - extent.z
    top = centre.z + extent.z
    if base < DECK - 50.0:
        continue
    lid = ceiling_at(centre.x, centre.y)
    if lid is None or base > lid or top <= lid + THICKNESS_CM:
        continue
    through.append((label, base, top))
say("standing on the deck and now poking through the ceiling: %d"
    % len(through))
for label, base, top in sorted(through, key=lambda r: -r[2])[:12]:
    say("  through %-44s %.0f..%.0f cm" % (label, base, top))

saved = unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
say("SAVED=%s" % bool(saved))

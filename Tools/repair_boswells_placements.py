"""Take the blockout out of the departments, and the clothes out of the aisles.

Chris, 2026-11-23, after walking `LLK_Boswells_OpeningPass` himself:

> *"a lot of placements of objects overlap on the top floor... the lower floor
> has some paths blocked by clothes and obsturcted, some look intentional,
> some overlap."*

Two complaints with one cause between them: **this floor has been dressed
several times and nothing has ever taken the previous pass out.**

## 1. The upper floor is wearing its own blockout

Eleven actors named `UPPER_TOYS`, `UPPER_KIDS`, `UPPER_BEDDING`,
`UPPER_SEASONAL`, `UPPER_CURTAINS`, `UPPER_FURNITURE_EAST`, `UPPER_HOME_EAST`
stand on the mezzanine deck. Every one of them is an **engine `Cube`**, 300 x
600 x 105 cm or thereabouts, on a default material.

They are not architecture and this repo already says so.
`Tools/ceiling_boswells_aisle_soffit.py` worked out what they were the hard
way and wrote it down - *"merchandise standing on the mezzanine deck"* - and
`assets/models/boswells/place_fixtures.py` is blunter still: `UPPER_BEDDING`
and `UPPER_SEASONAL` *"predate the department"*.

Then the departments were built, on top of them. `BSW_DEPARTMENT_*` put toy
gondolas, bed vignettes, sofa vignettes, appliance pods and home sideboards on
the same deck; `BSW_LEFTDAY_DEPARTMENT_*` put the after-close dressing there;
`BSW_CIRCULATION_UPPER_STOCK_*` put 72 stock units there. Nobody removed the
cubes, so a 6 m pale slab now stands inside the home department with real
sideboards showing through it. **That is the single most visible thing in a
render of the upper floor**, which is why Chris led with it.

**They are hidden, not deleted.** `docs/FINDINGS-KIDS-FLOOR-STEALTH.md` counts
them as cover, so this measures the cover they were providing and prints what
is lost - and an actor that is only hidden can be brought back by one line if
the measurement says it mattered. Deleting a thing another document depends on
because a render looks wrong is how this project loses an argument it has
already had.

## 2. The clothes in the aisles

`Tools/audit_boswells_clearance_and_mood.py` measured the walkways between
facing department cells and found **12 a 68 cm body cannot pass at all** - 5 to
27 cm of clear floor where 167 was drafted.

**Chris's own distinction is the one that matters here**: *"some look
intentional, some overlap."* A department with no way through on one side is
a design this floor already has and defends - `BOSWELLS-WALKWAY-AND-CEILING-
RULES.md` settles it in his words, *"inside a department there is no way past
a Sweeper, and that is the design"*, with the perimeter as the way out. So a
closed gap is **not automatically a fault**, and a tool that opened all twelve
would be undoing a decision rather than fixing a defect.

What is a fault, in every case and without needing anyone's opinion:

- **A fixture standing inside another fixture.** 115 pairs, including 24 that
  are the same rack placed twice at the same spot.
- **A fixture standing outside the department cell it belongs to**, up to
  **319 cm** past its own carpet, which is how it got into the aisle.

So this moves a fixture only when it is **both** overlapping something **and**
outside its own cell, and it moves it back onto its own carpet - the shortest
move that ends both faults. A fixture that is deliberately placed in an aisle
and touches nothing is left exactly where the author put it, and counted.

## What is reported and NOT changed

The upper storey has **no single ceiling height, and mostly no ceiling.** Of
3,978 points on the deck: 89% see the roof at **838 cm**, 10% see the service
core's lid at **375 cm**, and the kids department has a tile field at 448 cm
that has no collision so no trace finds it at all. Three heights and a void,
on one floor. That is not something a repair script should pick, so it prints
the numbers and leaves it for Chris.

The one piece of it that IS unambiguous is fixed: the kids ceiling light
housings hang at 935, which is **163 cm below the ceiling tile field they
belong to** at 1098. A fitting floating under its own ceiling is the same
fault as a fitting glowing over a dead lamp, which this project already
settled - see `docs/FINDINGS-THE-CEILING-WAS-LYING.md`.

Idempotent: it sets properties and positions rather than adding actors. Saves
the map, so it needs `-AllowUnsafeMapSave`.
"""

import unreal
from collections import defaultdict

MAP = "/Game/LLK_Boswells_OpeningPass"

PLATE = (-4350.0, -2700.0, 4350.0, 2090.0)
SALES_CEILING = 514.0
MEZZ_DECK = 650.0

# The pre-department blockout. Named, because every one of them was identified
# by hand and written up before this tool existed, and a prefix match would
# also catch `ANCHOR_UPPER_*`, which are the deck slabs themselves.
BLOCKOUT = ("UPPER_TOYS", "UPPER_KIDS", "UPPER_BEDDING", "UPPER_SEASONAL",
            "UPPER_CURTAINS", "UPPER_FURNITURE_EAST", "UPPER_HOME_EAST")

# The kids ceiling, and the project's standing rule for how far a fitting
# hangs below whatever is actually above it.
KIDS_CEILING_UNDERSIDE = 1098.0
FITTING_DROP_CM = 26.0
KIDS_FITTING = "BSW_KIDS_CEILING_FILL_"

DRESSING = ("BSW_JUNIORS_", "BSW_LEFTDAY_APPAREL_", "BSW_LEFTDAY_DOCK_",
            "BSW_CIRCULATION_LOWER_STOCK_", "BSW_ARRIVAL_PERFUME_",
            "BSW_ARRIVAL_JEWELRY_", "BSW_ARRIVAL_CHECKOUT_",
            "BSW_APPAREL_LOWER_")

SLOP = 8.0          # below this an overlap is drafting slop, not a fault
BAND = (10.0, 200.0)


def say(message):
    unreal.log("PLACE " + message)
    print("PLACE " + message)


def require(ok, message):
    if not ok:
        raise RuntimeError("PLACE " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = subsystem.get_all_level_actors()


class Box(object):
    __slots__ = ("actor", "label", "assembly",
                 "x0", "y0", "z0", "x1", "y1", "z1")

    def __init__(self, actor):
        self.actor = actor
        self.label = actor.get_actor_label()
        c, e = actor.get_actor_bounds(False)
        self.x0, self.x1 = c.x - e.x, c.x + e.x
        self.y0, self.y1 = c.y - e.y, c.y + e.y
        self.z0, self.z1 = c.z - e.z, c.z + e.z

    def overlap(self, other):
        ox = min(self.x1, other.x1) - max(self.x0, other.x0)
        oy = min(self.y1, other.y1) - max(self.y0, other.y0)
        oz = min(self.z1, other.z1) - max(self.z0, other.z0)
        if ox <= 0 or oy <= 0 or oz <= 0:
            return None
        return ox, oy, oz


# =====================================================================
# 1. The upper floor's blockout goes away, after its cover is counted
# =====================================================================
say("=" * 62)

blockout = [a for a in actors if a.get_actor_label() in BLOCKOUT]
require(len(blockout) > 0,
        "none of the %d named blockout cubes is in this map any more - either "
        "this has run and they were deleted, or they were renamed, and either "
        "way this tool needs rewriting rather than running" % len(BLOCKOUT))

# What cover are they providing? The band a child's body occupies, on the
# deck. If this is the only thing standing in a stretch of the upper floor,
# hiding it takes that cover away, and the number has to be said out loud.
cover_area = 0.0
for a in blockout:
    b = Box(a)
    if b.z1 - MEZZ_DECK > 60.0:
        cover_area += (b.x1 - b.x0) * (b.y1 - b.y0) / 10000.0
say("the pre-department blockout: %d cubes, %.0f m2 of footprint standing "
    "over 60 cm on the deck" % (len(blockout), cover_area))

hidden = 0
for a in blockout:
    b = Box(a)
    if not a.get_editor_property("hidden"):
        a.set_editor_property("hidden", True)
        a.set_actor_hidden_in_game(True)
        a.set_actor_enable_collision(False)
        hidden += 1
    say("  %-22s %.0f x %.0f x %.0f cm at (%.0f,%.0f) z %.0f-%.0f"
        % (a.get_actor_label(), b.x1 - b.x0, b.y1 - b.y0, b.z1 - b.z0,
           (b.x0 + b.x1) * 0.5, (b.y0 + b.y1) * 0.5, b.z0, b.z1))
say("blockout hidden and decollided: %d newly, %d already were"
    % (hidden, len(blockout) - hidden))
say("NOT deleted. One line in this tool brings them back if the cover they "
    "were giving the kids floor turns out to matter; see "
    "docs/FINDINGS-KIDS-FLOOR-STEALTH.md")


# =====================================================================
# 2. The kids ceiling fittings hang under their own ceiling
# =====================================================================
say("=" * 62)

target_z = KIDS_CEILING_UNDERSIDE - FITTING_DROP_CM
raised = 0
for a in actors:
    label = a.get_actor_label()
    if not (label.startswith(KIDS_FITTING)
            or label.startswith("BSW_FIXTURE_" + KIDS_FITTING)):
        continue
    loc = a.get_actor_location()
    if abs(loc.z - target_z) < 1.0:
        continue
    if loc.z > KIDS_CEILING_UNDERSIDE:
        continue                        # already above it; not ours to move
    a.set_actor_location(unreal.Vector(loc.x, loc.y, target_z), False, True)
    raised += 1
say("kids ceiling fittings raised from the void to %.0f cm, %.0f cm under "
    "the tile field they belong to: %d moved"
    % (target_z, FITTING_DROP_CM, raised))


# =====================================================================
# 3. The ground floor: fixtures that are BOTH overlapping AND off-cell
# =====================================================================
say("=" * 62)

carpets = {}
for a in actors:
    label = a.get_actor_label()
    if not label.startswith("BSW_GROUND_CARPET_"):
        continue
    here = a.get_actor_location()
    s = a.get_actor_scale3d()
    w, l = s.x * 100.0, s.y * 100.0
    carpets[label] = (here.x - w * 0.5, here.y - l * 0.5,
                      here.x + w * 0.5, here.y + l * 0.5)
require(len(carpets) >= 20, "only %d department cells - the 27-cell plan is "
                            "not in this map" % len(carpets))

dressing = []
for a in actors:
    label = a.get_actor_label()
    if not label.startswith(DRESSING):
        continue
    if a.get_editor_property("hidden") or a.is_hidden_ed():
        continue
    b = Box(a)
    if b.z0 > SALES_CEILING or b.z1 < BAND[0] or b.z0 > BAND[1]:
        continue
    dressing.append(b)
say("dressing on the sales floor, in the body band: %d" % len(dressing))


def assembly(label):
    """Which rack this piece belongs to.

    **The first version of this tool had no such notion, and that made its
    central measurement meaningless.** A rail in this store is not one actor;
    `dress_boswells_juniors.py` builds it from an UPRIGHT, a FOOT, a BAR and a
    row of GARMENTs, all at the same spot, and every one of those pairs
    overlaps every other by design. Counting them as faults gave 380
    "overlapping pairs", of which the great majority were racks overlapping
    themselves - and a repair aimed at that number moved 64 fixtures an
    average of 85 cm, half an aisle width, to remove four pairs.

    The label carries the assembly: everything up to and including the first
    purely numeric token. `BSW_JUNIORS_DRESS_RAIL_0_UPRIGHT_1` and
    `..._RAIL_0_GARMENT_16` are both rail 0. A label with no numeric token,
    like `BSW_CIRCULATION_LOWER_STOCK_-3850_+0450` whose numbers are signed,
    is its own assembly, which is correct - those are single actors.
    """
    parts = label.split("_")
    for i, token in enumerate(parts):
        if token.isdigit():
            return "_".join(parts[:i + 1])
    return label


def home_cell(b):
    cx, cy = (b.x0 + b.x1) * 0.5, (b.y0 + b.y1) * 0.5
    for name, (x0, y0, x1, y1) in carpets.items():
        if x0 <= cx <= x1 and y0 <= cy <= y1:
            return name
    return None


for b in dressing:
    b.assembly = assembly(b.label)
say("they belong to %d assemblies - a rail is four or more actors at one spot"
    % len({b.assembly for b in dressing}))


def clashes_with(b, others):
    """Every fixture from ANOTHER assembly standing inside this one."""
    out = []
    for o in others:
        if o is b or o.assembly == b.assembly:
            continue
        got = b.overlap(o)
        if not got:
            continue
        if min(got[0], got[1]) < SLOP or got[2] < SLOP:
            continue
        out.append(o)
    return out


def count_pairs(boxes):
    n = 0
    for i, a in enumerate(boxes):
        for j in range(i + 1, len(boxes)):
            o = boxes[j]
            if o.assembly == a.assembly:
                continue
            got = a.overlap(o)
            if not got:
                continue
            if min(got[0], got[1]) < SLOP or got[2] < SLOP:
                continue
            n += 1
    return n


pairs = count_pairs(dressing)
involved = {i for i, b in enumerate(dressing) if clashes_with(b, dressing)}
say("fixtures standing inside a DIFFERENT rack: %d pairs, %d fixtures"
    % (pairs, len(involved)))

# Move only the ones that are also outside their own cell, back onto their
# carpet - and **put the move back if it did not help.** A fixture shoved 85 cm
# across a 165 cm aisle to remove nothing is a worse floor than the one it
# started from, and the first version of this tool did exactly that 64 times.
moved = 0
reverted = 0
no_home = 0
in_cell = 0
total_move = 0.0
for idx in sorted(involved):
    b = dressing[idx]
    home = home_cell(b)
    if home is None:
        no_home += 1
        continue
    x0, y0, x1, y1 = carpets[home]
    dx = dy = 0.0
    if b.x0 < x0:
        dx = x0 - b.x0
    elif b.x1 > x1:
        dx = x1 - b.x1
    if b.y0 < y0:
        dy = y0 - b.y0
    elif b.y1 > y1:
        dy = y1 - b.y1
    if abs(dx) < SLOP and abs(dy) < SLOP:
        in_cell += 1
        continue
    # The shortest of the two, not both: a fixture pulled in on one axis is
    # usually already clear, and moving it diagonally walks it into whatever
    # is on the other side.
    if abs(dx) > abs(dy) and abs(dy) >= SLOP:
        dx = 0.0
    elif abs(dy) >= abs(dx) and abs(dx) >= SLOP:
        dy = 0.0

    before_n = len(clashes_with(b, dressing))
    loc = b.actor.get_actor_location()
    b.actor.set_actor_location(
        unreal.Vector(loc.x + dx, loc.y + dy, loc.z), False, True)
    moved_box = Box(b.actor)
    b.x0, b.y0, b.x1, b.y1 = moved_box.x0, moved_box.y0, moved_box.x1, moved_box.y1
    if len(clashes_with(b, dressing)) >= before_n:
        b.actor.set_actor_location(loc, False, True)
        back = Box(b.actor)
        b.x0, b.y0, b.x1, b.y1 = back.x0, back.y0, back.x1, back.y1
        reverted += 1
        continue
    total_move += abs(dx) + abs(dy)
    moved += 1

say("%d moved back onto their own carpet (%.0f cm in all, %.0f cm each); "
    "%d moves were tried and PUT BACK because they cleared nothing; "
    "%d were already inside their cell; %d stand on no carpet at all"
    % (moved, total_move, total_move / max(moved, 1), reverted, in_cell,
       no_home))

# Re-measure, from the map, after moving. A count printed by the tool that did
# the work is the thing this project has been burned by; this one re-reads.
after = [Box(b.actor) for b in dressing]
for i, bx in enumerate(after):
    bx.assembly = dressing[i].assembly
pairs_after = count_pairs(after)
say("fixtures inside a different rack, after: %d (was %d)"
    % (pairs_after, pairs))


# =====================================================================
# 4. Reported, not changed: the upper storey has no ceiling height
# =====================================================================
say("=" * 62)
say("NOT CHANGED - the upper storey's height, which is Chris's to pick:")
say("  89%% of the deck is open to ANCHOR_FULL_ROOF at 838 cm above it")
say("  10%% is under BSW_SERVICE_CORE_SERVICE_CEILING at 375 cm")
say("  the kids department has a tile field at 448 cm with NO collision,")
say("     so nothing traces it and no machine or player can be stopped by it")
say("  three heights and a void on one floor, and no datum anywhere")
say("  a real anchor's upper selling floor is 350-450 cm; this has 838")

saved = unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
say("SAVED=%s" % bool(saved))

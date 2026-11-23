"""Read only: what Chris saw when he opened the opening pass, measured.

Chris, 2026-11-23, after walking `LLK_Boswells_OpeningPass` himself - the
first time anybody has looked at this floor in the editor rather than at a
render of it:

> *"a lot of placements of objects overlap on the top floor, and the entire
> area has a ceiling that also drops down super low. we need to fix some of
> the height issues on that upper floor, the lower floor has some paths
> blocked by clothes and obsturcted, some look intentional, some overlap. so
> overall, also, the lighting is still to bright and not emergency looking."*

Three complaints, and this script turns each into a number before anything is
changed. It writes nothing and saves no map.

**Every previous pass on this floor measured it from the sales floor, at a
nine-year-old's eye, and photographed it from there too.** Both halves of that
discipline stopped at the mezzanine deck, and the upper storey has never been
measured at all - `audit_boswells_ground_lighting.py` explicitly classifies
anything above 650 as *"the upper storey's"* and drops it. So complaint one is
in a place no tool in this repo has ever looked.

## A. The upper floor: headroom and overlap

Headroom is measured where somebody stands, not as an average. For a grid of
points on the deck it traces up to the first thing overhead and reports the
clear height. A nine-year-old is 134 cm, so anything under about 200 cm reads
as oppressive and anything under 134 is impassable.

Overlap is pairwise box intersection on actor bounds. Bounds are generous for
a rotated or irregular mesh, so an overlap this finds is a candidate rather
than a verdict - which is why it reports the intersection's own dimensions and
sorts by volume. Two rails sharing 2 cm is drafting slop; two sharing 60 cm is
one standing inside the other, and that is what Chris saw.

## B. The lower floor: what the clothes did to the walkway

The 27-cell plan gives every department a carpet and leaves 165 cm of bare
floor between them - three steps of a nine-year-old, decided twice in Chris's
own words. **The dressing tools were never told about that number.** They
place fixtures relative to their department's cell, so a rail whose footprint
runs past the cell edge is standing in the walkway, and nothing has ever
checked.

This measures the clear gap left in every walkway after the dressing went in,
against three thresholds: 165 cm as drafted, 136 cm for two 68 cm bodies to
pass, and 68 cm for one body to get through at all. Below 68 the aisle is
closed, and *"some look intentional, some overlap"* is exactly the ambiguity
a number resolves - an aisle deliberately dead-ended reads the same as one
somebody's rail fell into, until you can see which cell the intruder belongs
to.

## C. The light: why 100% coverage was the defect, not the win

`docs/FINDINGS-THE-CEILING-WAS-LYING.md` reports *"shapes-or-better 87% ->
100%"* as this floor's lighting win. Chris looked at the result and said it is
too bright and does not read as emergency. **He is right and that table is
the reason.** Emergency lighting in a real building is not dim room light; it
is a handful of isolated pools on the egress route with unlit dark between
them, because its entire job is to show you the way out and nothing else.
A uniform 1500 cm grid that reaches every square metre of a 4,167 m2 plate is
a floor that is evenly, weakly lit everywhere - which is the one thing
emergency lighting never looks like.

So this prints the distribution rather than the mean: how much of the floor is
in a pool, how much is between pools, and how dark the darkest part gets. A
floor that reads as emergency has a bimodal histogram. A floor that reads as
dim has a hump.

It also prints every light's colour, because that half has never been
measured. Emergency fittings are cold - a 6500 K LED head or the green of an
exit sign - and if these are warm white the floor will read as "the lights are
on low", whatever the arithmetic says.
"""

import math
from collections import defaultdict

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"

PLATE = (-4350.0, -2700.0, 4350.0, 2090.0)
SALES_CEILING = 514.0
MEZZ_SOFFIT = 638.0
MEZZ_DECK = 650.0          # the upper floor people walk on
ROOF = 1488.0
ATRIUM = (-600.0, -900.0, 600.0, 900.0)

EYE_NINE = 124.0           # a nine-year-old's eye on the floor it stands on
STATURE_NINE = 134.0
OPPRESSIVE = 200.0         # under this a room reads low to a child
WALKWAY = 165.0            # three child steps of 55
TWO_BODIES = 136.0         # two 68 cm bodies
ONE_BODY = 68.0

HEAD_GRID = 100.0          # upper-floor headroom sample pitch
LIGHT_GRID = 75.0          # floor illuminance sample pitch

# Overlap below this in any axis is drafting slop, not a placement fault.
SLOP = 8.0

DRESSING = ("BSW_JUNIORS_", "BSW_LEFTDAY_APPAREL_", "BSW_LEFTDAY_DOCK_",
            "BSW_CIRCULATION_LOWER_STOCK_", "BSW_ARRIVAL_PERFUME_",
            "BSW_ARRIVAL_JEWELRY_", "BSW_ARRIVAL_CHECKOUT_",
            "BSW_APPAREL_LOWER_", "BSW_DEPT_")


def say(message):
    unreal.log("MOOD " + message)
    print("MOOD " + message)


def assembly(label):
    """Which rack a piece belongs to: everything up to the first purely
    numeric token.

    **Added 2026-11-23, because the first run of this script without it was
    wrong.** A rail in this store is several actors at one spot - an UPRIGHT,
    a FOOT, a BAR and a row of GARMENTs - so every one of those overlaps every
    other by construction. Counting them gave 380 "overlapping pairs" on the
    ground floor, the great majority of them racks overlapping themselves, and
    a repair aimed at that number moved 64 fixtures an average of 85 cm, half
    an aisle width, to remove four pairs. An overlap only means anything
    between two DIFFERENT assemblies.
    """
    parts = label.split("_")
    for i, token in enumerate(parts):
        if token.isdigit():
            return "_".join(parts[:i + 1])
    return label


def family(label):
    """`BSW_JUNIORS_RAIL_12` -> `BSW_JUNIORS_RAIL`. A hundred rails are one
    decision, not a hundred, and a report that lists them individually is a
    report nobody reads."""
    return label.rstrip("0123456789").rstrip("_")


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("MOOD map unavailable")
actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()
say("map=%s actors=%d" % (MAP, len(actors)))


# =====================================================================
# Gather once. Bounds queries are the expensive part of this script.
# =====================================================================

class Piece(object):
    __slots__ = ("label", "assembly", "x0", "y0", "z0", "x1", "y1", "z1",
                 "hidden")

    def __init__(self, label, centre, extent, hidden):
        self.label = label
        self.assembly = assembly(label)
        self.x0, self.x1 = centre.x - extent.x, centre.x + extent.x
        self.y0, self.y1 = centre.y - extent.y, centre.y + extent.y
        self.z0, self.z1 = centre.z - extent.z, centre.z + extent.z
        self.hidden = hidden

    def overlap(self, other):
        ox = min(self.x1, other.x1) - max(self.x0, other.x0)
        oy = min(self.y1, other.y1) - max(self.y0, other.y0)
        oz = min(self.z1, other.z1) - max(self.z0, other.z0)
        if ox <= 0 or oy <= 0 or oz <= 0:
            return None
        return ox, oy, oz


pieces = []
lights = []
for actor in actors:
    if isinstance(actor, unreal.Light):
        comp = actor.light_component
        if comp is None:
            continue
        loc = actor.get_actor_location()
        lights.append((actor, comp, loc))
        continue
    comps = actor.get_components_by_class(unreal.StaticMeshComponent)
    if not comps or not any(c.static_mesh is not None for c in comps):
        continue
    centre, extent = actor.get_actor_bounds(False)
    if extent.x <= 0 or extent.y <= 0:
        continue
    pieces.append(Piece(actor.get_actor_label(), centre, extent,
                        bool(actor.get_editor_property("hidden"))
                        or actor.is_hidden_ed()))

say("meshes=%d lights=%d" % (len(pieces), len(lights)))


# =====================================================================
# A. THE UPPER FLOOR
# =====================================================================
say("=" * 62)
say("A. THE UPPER FLOOR - nobody has measured this storey before")

# Anything standing on the deck. A piece whose base is within 20 cm of 650 is
# on the upper floor; one whose base is well above it is hanging from the
# upper floor's own ceiling and is overhead, not a prop.
on_deck = [p for p in pieces
           if MEZZ_DECK - 20.0 <= p.z0 < MEZZ_DECK + STATURE_NINE
           and not p.hidden]
overhead = [p for p in pieces
            if p.z0 >= MEZZ_DECK + STATURE_NINE and not p.hidden]
say("on the deck: %d props   overhead above it: %d pieces"
    % (len(on_deck), len(overhead)))

# --- A1. headroom, sampled where somebody walks ----------------------
#
# Traced rather than taken from bounds, because what matters is the first
# thing over your head and not the lowest thing in the building.
low = defaultdict(int)
heights = []
culprits = defaultdict(list)
x = PLATE[0] + HEAD_GRID * 0.5
while x < PLATE[2]:
    y = PLATE[1] + HEAD_GRID * 0.5
    while y < PLATE[3]:
        if not (ATRIUM[0] < x < ATRIUM[2] and ATRIUM[1] < y < ATRIUM[3]):
            # From ABOVE the deck props. The first run started at
            # MEZZ_DECK + 10, inside the furniture, and reported a
            # 13 cm "ceiling" that was the top face of a sofa.
            start = unreal.Vector(x, y, MEZZ_DECK + 250.0)
            end = unreal.Vector(x, y, ROOF + 200.0)
            hit = unreal.SystemLibrary.line_trace_single(
                world, start, end, unreal.TraceTypeQuery.ECC_VISIBILITY,
                True, [], unreal.DrawDebugTrace.NONE, True)
            if hit:
                d = hit.to_dict()
                clear = d["location"].z - MEZZ_DECK
                heights.append(clear)
                label = (d["hit_actor"].get_actor_label()
                         if d["hit_actor"] else "<none>")
                if clear < OPPRESSIVE:
                    low[family(label)] += 1
                    if len(culprits[family(label)]) < 3:
                        culprits[family(label)].append(
                            "(%.0f,%.0f) %.0f cm" % (x, y, clear))
        y += HEAD_GRID
    x += HEAD_GRID

if heights:
    heights.sort()
    n = len(heights)
    say("headroom on the deck, %d samples:" % n)
    say("  lowest=%.0f  p10=%.0f  median=%.0f  p90=%.0f  highest=%.0f cm"
        % (heights[0], heights[n // 10], heights[n // 2],
           heights[(9 * n) // 10], heights[-1]))
    say("  under %.0f cm (a nine-year-old cannot pass): %d  (%.0f%%)"
        % (STATURE_NINE, sum(1 for h in heights if h < STATURE_NINE),
           100.0 * sum(1 for h in heights if h < STATURE_NINE) / n))
    say("  under %.0f cm (reads oppressive): %d  (%.0f%%)"
        % (OPPRESSIVE, sum(1 for h in heights if h < OPPRESSIVE),
           100.0 * sum(1 for h in heights if h < OPPRESSIVE) / n))
    for key in sorted(low, key=lambda k: -low[k]):
        say("  low ceiling owner %s: %d samples  e.g. %s"
            % (key, low[key], "; ".join(culprits[key])))
else:
    say("NOTHING overhead the deck at all - no trace hit anything, so either "
        "the upper storey has no ceiling or it has no collision")

# --- A2. overlapping props on the deck -------------------------------
clashes = []
for i, a in enumerate(on_deck):
    for b in on_deck[i + 1:]:
        if a.assembly == b.assembly:
            continue
        got = a.overlap(b)
        if not got:
            continue
        ox, oy, oz = got
        if min(ox, oy, oz) < SLOP:
            continue
        clashes.append((ox * oy * oz, a.label, b.label, ox, oy, oz))
clashes.sort(reverse=True)
say("overlapping props on the deck: %d pairs past %.0f cm in every axis"
    % (len(clashes), SLOP))
pairs = defaultdict(int)
for _v, a, b, _x, _y, _z in clashes:
    key = tuple(sorted((family(a), family(b))))
    pairs[key] += 1
for key in sorted(pairs, key=lambda k: -pairs[k])[:12]:
    say("  %s x %s: %d pairs" % (key[0], key[1], pairs[key]))
for _v, a, b, ox, oy, oz in clashes[:10]:
    say("  worst: %s into %s by %.0f x %.0f x %.0f cm" % (a, b, ox, oy, oz))


# =====================================================================
# B. THE LOWER FLOOR - what the clothes did to the walkway
# =====================================================================
say("=" * 62)
say("B. THE LOWER FLOOR - the clothes against the 165 cm walkway")

carpets = {}
for actor in actors:
    label = actor.get_actor_label()
    if not label.startswith("BSW_GROUND_CARPET_"):
        continue
    here = actor.get_actor_location()
    scale = actor.get_actor_scale3d()
    w, l = scale.x * 100.0, scale.y * 100.0
    carpets[label] = (here.x - w * 0.5, here.y - l * 0.5,
                      here.x + w * 0.5, here.y + l * 0.5)
say("department cells: %d" % len(carpets))


def cell_of(px, py):
    for name, (x0, y0, x1, y1) in carpets.items():
        if x0 <= px <= x1 and y0 <= py <= y1:
            return name
    return None


# Every dressing piece standing in the body band, and how far it runs outside
# the cell it belongs to. A fixture whose own centre is in no cell is loose on
# the floor and reported separately - that is a different fault.
BAND = (10.0, 200.0)
intruders = []
loose = []
dressing = []
for p in pieces:
    if not p.label.startswith(DRESSING) or p.hidden:
        continue
    if p.z0 > SALES_CEILING or p.z1 < BAND[0] or p.z0 > BAND[1]:
        continue
    dressing.append(p)
    cx, cy = (p.x0 + p.x1) * 0.5, (p.y0 + p.y1) * 0.5
    home = cell_of(cx, cy)
    if home is None:
        loose.append(p)
        continue
    x0, y0, x1, y1 = carpets[home]
    out = max(x0 - p.x0, p.x1 - x1, y0 - p.y0, p.y1 - y1)
    if out > SLOP:
        intruders.append((out, p.label, home))
intruders.sort(reverse=True)
say("dressing in the body band: %d" % len(dressing))
say("standing on no department carpet at all: %d" % len(loose))
for p in loose[:8]:
    say("  loose: %s at (%.0f,%.0f) z %.0f-%.0f"
        % (p.label, (p.x0 + p.x1) * 0.5, (p.y0 + p.y1) * 0.5, p.z0, p.z1))
say("overhanging its own cell by more than %.0f cm: %d" % (SLOP, len(intruders)))
by_family = defaultdict(lambda: [0, 0.0])
for out, label, _home in intruders:
    row = by_family[family(label)]
    row[0] += 1
    row[1] = max(row[1], out)
for key in sorted(by_family, key=lambda k: -by_family[k][0]):
    say("  %s: %d pieces, worst %.0f cm into the walkway"
        % (key, by_family[key][0], by_family[key][1]))

# --- B2. so how wide is the aisle, really? ---------------------------
#
# Walked rather than inferred. Along every gap between two cells, step the
# centre line and measure the clear span between the nearest obstruction on
# each side, at a child's chest height.
def clear_span(px, py, axis, limit=400.0):
    """Clear width across `axis` ('x' or 'y') at (px, py), chest height."""
    z = 80.0
    out = []
    for sign in (-1.0, 1.0):
        if axis == "x":
            end = unreal.Vector(px + sign * limit, py, z)
        else:
            end = unreal.Vector(px, py + sign * limit, z)
        hit = unreal.SystemLibrary.line_trace_single(
            world, unreal.Vector(px, py, z), end,
            unreal.TraceTypeQuery.ECC_VISIBILITY, True, [],
            unreal.DrawDebugTrace.NONE, True)
        if hit:
            d = hit.to_dict()
            loc = d["location"]
            out.append(abs((loc.x - px) if axis == "x" else (loc.y - py)))
        else:
            out.append(limit)
    return out[0] + out[1]


# The gaps: for each pair of cells that face each other across a walkway,
# the centre line of that gap.
gaps = []
names = sorted(carpets)
for i, a in enumerate(names):
    ax0, ay0, ax1, ay1 = carpets[a]
    for b in names[i + 1:]:
        bx0, by0, bx1, by1 = carpets[b]
        # facing across x: y ranges overlap, x ranges adjacent
        oy = min(ay1, by1) - max(ay0, by0)
        if oy > 100.0:
            d = max(bx0 - ax1, ax0 - bx1)
            if 0 < d < 400.0:
                mid = (ax1 + bx0) * 0.5 if ax1 < bx0 else (bx1 + ax0) * 0.5
                gaps.append(("x", mid, max(ay0, by0), min(ay1, by1), a, b, d))
        ox = min(ax1, bx1) - max(ax0, bx0)
        if ox > 100.0:
            d = max(by0 - ay1, ay0 - by1)
            if 0 < d < 400.0:
                mid = (ay1 + by0) * 0.5 if ay1 < by0 else (by1 + ay0) * 0.5
                gaps.append(("y", mid, max(ax0, bx0), min(ax1, bx1), a, b, d))

say("walkways between facing cells: %d" % len(gaps))
closed = []
tight = []
for axis, mid, lo, hi, a, b, drafted in gaps:
    spans = []
    t = lo + 40.0
    while t < hi - 40.0:
        if axis == "x":
            spans.append((clear_span(mid, t, "x"), t))
        else:
            spans.append((clear_span(t, mid, "y"), t))
        t += 60.0
    if not spans:
        continue
    spans.sort()
    worst, at = spans[0]
    med = spans[len(spans) // 2][0]
    if worst < ONE_BODY:
        closed.append((worst, axis, mid, at, a, b, drafted, med))
    elif worst < TWO_BODIES:
        tight.append((worst, axis, mid, at, a, b, drafted, med))
closed.sort()
tight.sort()
say("walkways a %.0f cm body cannot pass at all: %d" % (ONE_BODY, len(closed)))
for worst, axis, mid, at, a, b, drafted, med in closed[:14]:
    say("  CLOSED %s=%.0f at %.0f: %.0f cm clear (drafted %.0f, median %.0f) "
        "between %s and %s" % (axis, mid, at, worst, drafted, med, a, b))
say("walkways two bodies cannot pass: %d" % len(tight))
for worst, axis, mid, at, a, b, drafted, med in tight[:10]:
    say("  TIGHT %s=%.0f at %.0f: %.0f cm clear (drafted %.0f, median %.0f) "
        "between %s and %s" % (axis, mid, at, worst, drafted, med, a, b))

# --- B3. dressing overlapping dressing -------------------------------
dress_clash = []
for i, a in enumerate(dressing):
    for b in dressing[i + 1:]:
        if a.assembly == b.assembly:
            continue
        got = a.overlap(b)
        if not got:
            continue
        ox, oy, oz = got
        if min(ox, oy) < SLOP or oz < SLOP:
            continue
        dress_clash.append((ox * oy * oz, a.label, b.label, ox, oy, oz))
dress_clash.sort(reverse=True)
say("dressing pieces standing inside each other: %d pairs" % len(dress_clash))
dpairs = defaultdict(int)
for _v, a, b, _x, _y, _z in dress_clash:
    dpairs[tuple(sorted((family(a), family(b))))] += 1
for key in sorted(dpairs, key=lambda k: -dpairs[k])[:12]:
    say("  %s x %s: %d pairs" % (key[0], key[1], dpairs[key]))
for _v, a, b, ox, oy, oz in dress_clash[:8]:
    say("  worst: %s into %s by %.0f x %.0f x %.0f cm" % (a, b, ox, oy, oz))


# =====================================================================
# C. THE LIGHT - distribution and colour, not the mean
# =====================================================================
say("=" * 62)
say("C. THE LIGHT - Chris: too bright, and not emergency looking")

lower = []
for actor, comp, loc in lights:
    if loc.z > MEZZ_DECK:
        continue
    tags = [str(t) for t in actor.tags]
    try:
        temp = comp.get_editor_property("temperature")
        use_temp = comp.get_editor_property("use_temperature")
    except Exception:
        temp, use_temp = 0.0, False
    colour = comp.get_editor_property("light_color")
    lower.append((actor.get_actor_label(), tags,
                  comp.get_editor_property("intensity"),
                  (colour.r, colour.g, colour.b), temp, use_temp,
                  bool(comp.get_editor_property("visible")), loc))

say("lights below the mezzanine deck: %d" % len(lower))
fam = defaultdict(lambda: [0, [], set(), set(), 0])
for label, tags, inten, rgb, temp, use_temp, vis, loc in lower:
    row = fam[family(label)]
    row[0] += 1
    row[1].append(inten)
    row[2].add("%dK" % round(temp) if use_temp else "rgb%s" % (rgb,))
    row[3].update(tags)
    if vis:
        row[4] += 1
for key in sorted(fam, key=lambda k: -fam[k][0]):
    n, ints, cols, tags, vis = fam[key]
    ints.sort()
    say("  %s n=%d visible=%d intensity %.1f-%.1f (median %.1f) "
        "colour=%s tags=%s"
        % (key, n, vis, ints[0], ints[-1], ints[len(ints) // 2],
           ",".join(sorted(cols)), ",".join(sorted(tags)) or "-"))

lit_at_boot = [r for r in lower if r[2] > 0.0 and r[6]]
say("on at boot with any output: %d of %d" % (len(lit_at_boot), len(lower)))

# --- C2. the distribution on the floor -------------------------------
#
# E = I / d^2 with a Lambert term, same arithmetic as
# audit_boswells_ground_lighting.py, and the same caveat: it is a relative
# measure, not photometry, because BSW_EXPOSURE_LOCK pins what the camera
# makes of it. What this is for is the SHAPE of the histogram.
samples = []
x = PLATE[0] + LIGHT_GRID * 0.5
while x < PLATE[2]:
    y = PLATE[1] + LIGHT_GRID * 0.5
    while y < PLATE[3]:
        e = 0.0
        for label, tags, inten, rgb, temp, use_temp, vis, loc in lit_at_boot:
            dx, dy, dz = loc.x - x, loc.y - y, loc.z - EYE_NINE
            d2 = dx * dx + dy * dy + dz * dz
            if d2 < 1.0 or d2 > 4.0e6:
                continue
            e += inten * abs(dz) / (d2 * math.sqrt(d2))
        samples.append(e)
        y += LIGHT_GRID
    x += LIGHT_GRID

samples.sort()
n = len(samples)
say("floor samples: %d at %.0f cm pitch" % (n, LIGHT_GRID))
say("  p01=%.4f p10=%.4f p25=%.4f median=%.4f p75=%.4f p90=%.4f p99=%.4f max=%.4f"
    % (samples[n // 100], samples[n // 10], samples[n // 4], samples[n // 2],
       samples[(3 * n) // 4], samples[(9 * n) // 10],
       samples[(99 * n) // 100], samples[-1]))

# The shape question. Emergency light is pools and dark: a wide ratio between
# the bright part and the dark part. Dim room light is a narrow ratio.
if samples[n // 10] > 0:
    say("  p90/p10 contrast ratio = %.1f   (pools-and-dark is high; a floor "
        "lit evenly and weakly is near 1)" % (samples[(9 * n) // 10]
                                              / samples[n // 10]))
else:
    say("  p10 is zero, so a tenth of this floor is black")

# Histogram, printed so the shape is visible rather than described.
top = samples[-1] or 1.0
BUCKETS = 12
counts = [0] * BUCKETS
for s in samples:
    counts[min(BUCKETS - 1, int(BUCKETS * s / top))] += 1
say("  histogram of floor brightness, %d equal bands from 0 to %.4f:" % (
    BUCKETS, top))
for i, c in enumerate(counts):
    say("   %5.1f%%-%5.1f%% of max | %-40s %d (%.0f%%)"
        % (100.0 * i / BUCKETS, 100.0 * (i + 1) / BUCKETS,
           "#" * int(40.0 * c / max(counts)), c, 100.0 * c / n))

say("=" * 62)
say("read only: nothing changed, no map saved")

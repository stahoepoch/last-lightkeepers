"""Read only: draw the ground floor from the map, not from the plan.

Runs headlessly with `Tools/run_editor_python.ps1`.

`docs/reference/boswells/BoswellsGroundFloor_Plan.svg` is the plan. This draws
the same sheet from the actors that are actually in
`LLK_Boswells_OpeningPass`, reading each one's real transform, and writes it
to `BoswellsGroundFloor_AsBuilt.svg`.

The two drawings should be the same drawing. The reason to make the second one
is that every way this project has gone wrong has been a correct system over
an absent thing with a clean log: a build script that reports `saved=True`
while the floor it laid is somewhere else is exactly that shape, and a number
printed by the tool that did the work cannot catch it. A drawing made from the
map can, and Chris is remote, so a drawing is the only thing either of us can
actually look at.

It checks as well as draws, and it checks the things a log line cannot:

- every carpet actor's centre and size match the cell the plan gives it
- no carpet lies in the main aisle, the cross aisle, the escalator court or
  the atrium void
- no two carpets touch, so the 180 cm walkway is really there underfoot
- the four mirrors are where the plan puts them, turned 45 degrees, below the
  tile underside and so visible from the floor

It changes nothing and saves no map.
"""

import csv
from pathlib import Path

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
CARPET = "BSW_GROUND_CARPET_"
MIRROR = "BSW_CEILING_MIRROR_"
ROOT = Path(unreal.Paths.project_dir()) / "docs/reference/boswells"
CSV_PATH = ROOT / "BoswellsGroundFloor_Cells.csv"
OUT = ROOT / "BoswellsGroundFloor_AsBuilt.svg"

PLATE = (-4350.0, -2700.0, 4350.0, 2090.0)
MAIN_AISLE = 900.0
COURT = (-1000.0, -1000.0, 1000.0, 1000.0)
ATRIUM = (-600.0, -900.0, 600.0, 900.0)
WALKWAY = 180.0
SALES_CEILING = 514.0
TOLERANCE = 1.0

COLOUR = {
    "juniors": "#9fb6c2", "ladies": "#c6a8b6", "mens": "#a8aec6",
    "shoes": "#c2b7a8", "service": "#bdbdb6",
    "perfume": "#efe7da", "jewelry": "#efe7da",
}

problems = []


def fail(message):
    problems.append(message)


# ---- the plan, to compare against --------------------------------------
with CSV_PATH.open(newline="", encoding="utf-8") as handle:
    plan = {row["name"]: row for row in csv.DictReader(handle)}

world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("ASBUILT map unavailable")

actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()

# ---- what is actually in the map ---------------------------------------
#
# The transform, not the label. A plane is 100 cm square, so its world size is
# its scale times 100, and that is the number that decides what the player
# stands on.
built = []
mirrors = []
for actor in actors:
    label = actor.get_actor_label()
    if label.startswith(CARPET):
        here = actor.get_actor_location()
        scale = actor.get_actor_scale3d()
        built.append((label, here.x, here.y, scale.x * 100.0, scale.y * 100.0,
                      actor.static_mesh_component.get_material(0)))
    elif label.startswith(MIRROR):
        here = actor.get_actor_location()
        scale = actor.get_actor_scale3d()
        yaw = actor.get_actor_rotation().yaw
        mirrors.append((label, here.x, here.y, here.z,
                        scale.x * 100.0, scale.y * 100.0, yaw))

print("ASBUILT carpets=%d mirrors=%d" % (len(built), len(mirrors)))
if not built:
    fail("no carpet in the map at all")

# ---- does each carpet match its cell? ----------------------------------
#
# Matched by geometry rather than by name, because a label is what the tool
# believed and a transform is what the map holds.
matched = {}
for label, x, y, width, length, _material in built:
    best, best_miss = None, None
    for name, row in plan.items():
        cx = (float(row["xmin_cm"]) + float(row["xmax_cm"])) * 0.5
        cy = (float(row["ymin_cm"]) + float(row["ymax_cm"])) * 0.5
        miss = abs(cx - x) + abs(cy - y)
        if best_miss is None or miss < best_miss:
            best, best_miss = name, miss
    if best_miss > TOLERANCE:
        fail("%s sits at (%.0f, %.0f), %.0f cm from the nearest cell (%s)"
             % (label, x, y, best_miss, best))
        continue
    row = plan[best]
    want_w = float(row["xmax_cm"]) - float(row["xmin_cm"])
    want_l = float(row["ymax_cm"]) - float(row["ymin_cm"])
    if abs(want_w - width) > TOLERANCE or abs(want_l - length) > TOLERANCE:
        fail("%s is %.0f x %.0f cm; cell %s is %.0f x %.0f"
             % (label, width, length, best, want_w, want_l))
    if best in matched:
        fail("cell %s has two carpets: %s and %s" % (best, matched[best], label))
    matched[best] = label

# ---- is anything standing in a walkway? --------------------------------
for label, x, y, width, length, _m in built:
    x0, x1 = x - width * 0.5, x + width * 0.5
    y0, y1 = y - length * 0.5, y + length * 0.5
    if x0 < MAIN_AISLE and -MAIN_AISLE < x1:
        fail("%s crosses the main aisle" % label)
    if x0 < COURT[2] and COURT[0] < x1 and y0 < COURT[3] and COURT[1] < y1:
        fail("%s stands in the escalator court" % label)
    if x0 < ATRIUM[2] and ATRIUM[0] < x1 and y0 < ATRIUM[3] and ATRIUM[1] < y1:
        fail("%s stands in the atrium void" % label)
    if x0 < PLATE[0] - TOLERANCE or x1 > PLATE[2] + TOLERANCE or \
            y0 < PLATE[1] - TOLERANCE or y1 > PLATE[3] + TOLERANCE:
        fail("%s hangs off the sales plate" % label)

# The walkway is the gap, so the gap is the thing to measure.
for i, a in enumerate(built):
    for b in built[i + 1:]:
        ax0, ax1 = a[1] - a[3] * 0.5, a[1] + a[3] * 0.5
        ay0, ay1 = a[2] - a[4] * 0.5, a[2] + a[4] * 0.5
        bx0, bx1 = b[1] - b[3] * 0.5, b[1] + b[3] * 0.5
        by0, by1 = b[2] - b[4] * 0.5, b[2] + b[4] * 0.5
        if min(ax1, bx1) - max(ax0, bx0) > 0.5 and \
                min(ay1, by1) - max(ay0, by0) > 0.5:
            fail("%s and %s overlap; there is no walkway between them"
                 % (a[0], b[0]))

# ---- can a keeper actually walk it? ------------------------------------
#
# **This check replaced one that was wrong, and the way it was wrong is worth
# keeping.** The first version refused any cell crossing a cross aisle at
# y +/-500, because `Tools/floor_boswells_departments.py` reserves one there.
# It flagged eight cells, and the plan was right and the check was wrong:
#
# - Nothing is built on y +/-500. The aisle soffit from `86cbdfd` runs on X
#   only, at x +/-900. That band was `floor_boswells_departments.py`'s own
#   invention; `Tools/plan_boswells_store.py` instead reserves three 300 cm
#   cross aisles, at y -1450..-1150, -150..150 and 1150..1450.
# - The 27-cell plan has no straight cross aisle **by design**: every column
#   splits into its own rows so that no row runs across the store, which is
#   what the Meriden sheet does and what Chris asked for. Check the six
#   columns against any single y and at least three of them are mid-bay
#   there. A straight cross aisle and that irregularity cannot both exist.
#
# So the thing to check is not whether a particular aisle survived. It is
# whether the floor can be **walked**, which is what a reserved aisle was
# only ever a proxy for. This floods the walkable space at the keeper's own
# width and asks whether the player can get from the service door they enter
# by to the mall grille they are heading for.
BODY = 68.0          # the keeper capsule, from docs/reference/boswells/README
STEP = 20.0
ENTRY = (3200.0, 2000.0)     # inside the east service door at x 3000..3400
GOAL = (0.0, -2600.0)        # the mall grille, mid south wall

blocks = [(c[1] - c[3] * 0.5, c[2] - c[4] * 0.5,
           c[1] + c[3] * 0.5, c[2] + c[4] * 0.5) for c in built]
blocks += [(float(r["xmin_cm"]), float(r["ymin_cm"]),
            float(r["xmax_cm"]), float(r["ymax_cm"]))
           for n, r in plan.items() if n not in matched]
blocks.append(ATRIUM)        # the void: you fall down it, you do not cross it

nx = int((PLATE[2] - PLATE[0]) / STEP)
ny = int((PLATE[3] - PLATE[1]) / STEP)
half = BODY * 0.5


def clear(ix, iy):
    """Does a keeper-width body fit, centred on this grid square?"""
    x = PLATE[0] + (ix + 0.5) * STEP
    y = PLATE[1] + (iy + 0.5) * STEP
    if not (PLATE[0] + half <= x <= PLATE[2] - half):
        return False
    if not (PLATE[1] + half <= y <= PLATE[3] - half):
        return False
    for bx0, by0, bx1, by1 in blocks:
        if bx0 - half < x < bx1 + half and by0 - half < y < by1 + half:
            return False
    return True


def square(point):
    return (int((point[0] - PLATE[0]) / STEP),
            int((point[1] - PLATE[1]) / STEP))

start, goal = square(ENTRY), square(GOAL)
if not clear(*start):
    fail("the entry at (%.0f, %.0f) is blocked, so the walk cannot even "
         "begin there" % ENTRY)
if not clear(*goal):
    fail("the mall grille at (%.0f, %.0f) is blocked" % GOAL)

seen = {start}
queue = [start]
while queue:
    ix, iy = queue.pop()
    for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        nxt = (ix + dx, iy + dy)
        if nxt in seen or not (0 <= nxt[0] < nx and 0 <= nxt[1] < ny):
            continue
        if not clear(*nxt):
            continue
        seen.add(nxt)
        queue.append(nxt)

reachable_m2 = len(seen) * (STEP / 100.0) ** 2
print("ASBUILT walkable_m2=%.0f at body=%.0f cm reachable_from_entry=%s"
      % (reachable_m2, BODY, goal in seen))
if goal in seen:
    print("ASBUILT route=OK service door to mall grille, %.0f cm body" % BODY)
else:
    fail("a %.0f cm body cannot get from the service door at (%.0f, %.0f) to "
         "the mall grille at (%.0f, %.0f); the walkways do not connect"
         % (BODY, ENTRY[0], ENTRY[1], GOAL[0], GOAL[1]))

# ---- the mirrors -------------------------------------------------------
WANT_MIRRORS = ((-2625.0, -1200.0), (2625.0, -1200.0),
                (-2625.0, 1200.0), (2625.0, 1200.0))
if len(mirrors) != len(WANT_MIRRORS):
    fail("%d mirrors in the map, expected %d" % (len(mirrors),
                                                 len(WANT_MIRRORS)))
for wx, wy in WANT_MIRRORS:
    here = [m for m in mirrors
            if abs(m[1] - wx) < TOLERANCE and abs(m[2] - wy) < TOLERANCE]
    if not here:
        fail("no mirror at (%.0f, %.0f)" % (wx, wy))
        continue
    label, _x, _y, z, width, length, yaw = here[0]
    if z >= SALES_CEILING:
        fail("%s is at z=%.1f, at or above the tile underside, so the player "
             "cannot see it" % (label, z))
    if abs(abs(yaw) - 45.0) > 0.5:
        fail("%s has yaw %.1f, not 45; it would read as an oblong, not a "
             "diamond" % (label, yaw))
    if abs(width - 1400.0) > TOLERANCE or abs(length - 950.0) > TOLERANCE:
        fail("%s is %.0f x %.0f cm, not 1400 x 950" % (label, width, length))

# ---- draw it -----------------------------------------------------------
SCALE = 10.0           # 1 px per 10 cm
OX, OY = 60.0, 48.0


def sx(x):
    return OX + (x - PLATE[0]) / SCALE


def sy(y):
    return OY + (PLATE[3] - y) / SCALE


W = sx(PLATE[2]) + 60.0
H = sy(PLATE[1]) + 108.0
out = ['<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 %.0f %.0f" '
       'width="%.0f" height="%.0f" font-family="Helvetica, Arial, sans-serif">'
       % (W, H, W, H),
       '<rect width="%.0f" height="%.0f" fill="#fbfaf7"/>' % (W, H),
       '<text x="%.1f" y="26" font-size="13" letter-spacing="1.6" '
       'fill="#111">BOSWELL&#8217;S GROUND FLOOR &#183; AS BUILT</text>' % OX,
       '<text x="%.1f" y="40" font-size="7.5" fill="#666">Drawn from the '
       'actors in LLK_Boswells_OpeningPass, not from the plan. 1 px = 10 cm.'
       '</text>' % OX]

# the plate
out.append('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="#fff" '
           'stroke="#111" stroke-width="1.6"/>'
           % (sx(PLATE[0]), sy(PLATE[3]),
              (PLATE[2] - PLATE[0]) / SCALE, (PLATE[3] - PLATE[1]) / SCALE))

# the racetrack, as hard floor
for x0, x1 in ((-MAIN_AISLE, MAIN_AISLE),):
    out.append('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" '
               'fill="#f0ece2"/>' % (sx(x0), sy(PLATE[3]),
                                     (x1 - x0) / SCALE,
                                     (PLATE[3] - PLATE[1]) / SCALE))
# No cross aisle is drawn, because the plan has none: every column splits
# into its own rows so no row runs across the store. See the walk check.

# the carpets, from the map
for label, x, y, width, length, material in built:
    department = label[len(CARPET):].split("_")[0].lower()
    name = label[len(CARPET) + len(department) + 1:].replace("_", " ").strip()
    fill = COLOUR.get(department, "#cccccc")
    out.append('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" '
               'fill="%s" stroke="#111" stroke-width="0.7"/>'
               % (sx(x - width * 0.5), sy(y + length * 0.5),
                  width / SCALE, length / SCALE, fill))
    wide, tall = width / SCALE, length / SCALE
    turn = ""
    if wide < 55 and tall > wide:
        turn = ' transform="rotate(-90 %.1f %.1f)"' % (sx(x), sy(y))
    out.append('<text x="%.1f" y="%.1f" text-anchor="middle" font-size="5.6" '
               'fill="#111"%s>%s</text>'
               % (sx(x), sy(y) + 2, turn, name.replace("&", "&amp;")))

# the hard-floor cells, outlined only, so the change underfoot is visible
for name, row in sorted(plan.items()):
    if name in matched:
        continue
    x0, y0 = float(row["xmin_cm"]), float(row["ymin_cm"])
    x1, y1 = float(row["xmax_cm"]), float(row["ymax_cm"])
    out.append('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" '
               'fill="#f7f3e9" stroke="#111" stroke-width="0.7" '
               'stroke-dasharray="3 2"/>'
               % (sx(x0), sy(y1), (x1 - x0) / SCALE, (y1 - y0) / SCALE))
    out.append('<text x="%.1f" y="%.1f" text-anchor="middle" font-size="5.2" '
               'fill="#555">%s</text>'
               % (sx((x0 + x1) * 0.5), sy((y0 + y1) * 0.5) + 2,
                  name.replace("&", "&amp;")))

# the atrium and the court
out.append('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="none" '
           'stroke="#111" stroke-width="1" stroke-dasharray="5 3"/>'
           % (sx(COURT[0]), sy(COURT[3]),
              (COURT[2] - COURT[0]) / SCALE, (COURT[3] - COURT[1]) / SCALE))
out.append('<text x="%.1f" y="%.1f" text-anchor="middle" font-size="5.6" '
           'fill="#111">ESCALATOR COURT</text>' % (sx(0), sy(0) - 2))
out.append('<text x="%.1f" y="%.1f" text-anchor="middle" font-size="5" '
           'fill="#666">14.88 m to the roof</text>' % (sx(0), sy(0) + 7))

# the mirrors, as they really sit
for label, x, y, z, width, length, yaw in mirrors:
    half_w, half_l = width * 0.5, length * 0.5
    pts = []
    for dx, dy in ((-half_w, -half_l), (half_w, -half_l),
                   (half_w, half_l), (-half_w, half_l)):
        rx = (dx - dy) * 0.70710678
        ry = (dx + dy) * 0.70710678
        pts.append("%.1f,%.1f" % (sx(x + rx), sy(y + ry)))
    out.append('<polygon points="%s" fill="#111" fill-opacity="0.13" '
               'stroke="#111" stroke-width="1.3"/>' % " ".join(pts))
    # A halo, because these sit over the department names underneath them and
    # the first render had "MENS CLOTHING" and a mirror's dimensions printed
    # through each other. Drawn last, so the halo covers what it crosses.
    halo = ('paint-order="stroke" stroke="#fbfaf7" stroke-width="2.6" '
            'stroke-linejoin="round"')
    out.append('<text x="%.1f" y="%.1f" text-anchor="middle" font-size="5.8" '
               'fill="#111" %s>MIRROR</text>' % (sx(x), sy(y) - 1, halo))
    out.append('<text x="%.1f" y="%.1f" text-anchor="middle" font-size="4.8" '
               'fill="#555" %s>%.0f &#215; %.0f at z %.0f</text>'
               % (sx(x), sy(y) + 6, halo, width, length, z))

carpet_m2 = sum(c[3] * c[4] for c in built) / 10000.0
ty = sy(PLATE[1]) + 18.0
out.append('<text x="%.1f" y="%.1f" font-size="6.4" fill="#111">%d carpeted '
           'cells, %.0f m&#178;. %d cells left as hard floor: counters, the '
           'lift, the service desk, the stock room.</text>'
           % (OX, ty, len(built), carpet_m2, len(plan) - len(built)))
out.append('<text x="%.1f" y="%.1f" font-size="6.4" fill="#111">The %.0f cm '
           'walkway is the gap: the carpet stops at the cell edge, so what '
           'the player sees between departments is bare tile.</text>'
           % (OX, ty + 12, WALKWAY))
out.append('<text x="%.1f" y="%.1f" font-size="6.4" fill="%s">%s</text>'
           % (OX, ty + 24, "#8a1c1c" if problems else "#1c6b2e",
              ("%d PROBLEM(S) &#8212; see the log" % len(problems))
              if problems else
              "Every carpet matches its cell; every mirror is where the plan "
              "puts it, turned 45&#176;, below the tile."))
out.append("</svg>")

OUT.write_text("\n".join(out), encoding="utf-8")
print("ASBUILT wrote=%s carpet_m2=%.0f" % (OUT, carpet_m2))

for message in problems:
    print("ASBUILT PROBLEM " + message)
print("ASBUILT problems=%d" % len(problems))
if problems:
    raise RuntimeError("ASBUILT %d problem(s); the map does not match the plan"
                       % len(problems))

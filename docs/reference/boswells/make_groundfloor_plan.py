"""Boswell's ground floor as a store directory: many cells, not four blocks.

Chris, 2026-11-23, on the four-block proposal and the Meriden reference
photographs: *"This is what the floor plan looks like. What you're showing me
are like four giant blocks. It's not like that. There's all kinds of intricate
little paths in between all of the different areas."*

He is right, and the reference was already in this repo. Two things in it
decide this file:

1. `docs/THE-MALL-IS-STILL-TRADING.md` section 13 is the department authority.
   Chris's lower floor, in his order: **juniors, jewelry, mens, perfume,
   ladies, shoes**, with the teens area and the big CRT sets sitting with
   juniors. Everything else he named is upstairs. So electronics is not a
   ground-floor department, and jewelry and perfume are - which the four-block
   proposal had backwards in both directions.
2. The CT Mommy Blog photographs of the Meriden Boscov's first-floor diagram
   (`source-photos/`, and the links in README.md). They are too low-resolution
   to read as a measured plan, and Meriden is a different building. What they
   do show is the *grain*: roughly two dozen named cells of very different
   sizes and shapes, narrow strips beside wide blocks, counters ringing the
   escalator well, fitting rooms bitten out of the apparel cells, and aisles
   that do not line up across the store.

So every cell here is a sub-department of one of Chris's six, or a service.
Nothing invents a department he did not name. The sub-names (misses
sportswear, petites, young mens, ties and belts) are how a 1990s anchor
directory breaks those six down, which is the pattern the photographs show.

This is a plan, not a build. It writes the cell table and the drawing:

    py docs/reference/boswells/make_groundfloor_plan.py

Geometry is read from the map, not typed from memory:
`ROUTE_MALL_GRILLE_*` puts the customer entrance in the south wall at
x +/-900; `ROUTE_SERVICE_WALL_*` puts the back-of-house wall at y 2090 with
400 cm doors at x -3400..-3000 and 3000..3400; the dock is north; the atrium
void is x +/-600, y +/-900.
"""

from __future__ import annotations

import csv
import math
from pathlib import Path

OUT = Path(__file__).resolve().parent
CSV_PATH = OUT / "BoswellsGroundFloor_Cells.csv"
SVG_PATH = OUT / "BoswellsGroundFloor_Plan.svg"

# --- the plate, from the map -------------------------------------------------

PLATE = (-4350.0, -2700.0, 4350.0, 2090.0)   # sales floor, x0 y0 x1 y1

# Chris, 2026-11-23: *"the pacing should be three child-like steps in width of
# a walkway."* Decision row 81 licenses deriving a number rather than picking
# one, provided the derivation sits beside it. Walking step length is about
# 0.41 of stature; the keeper is a child of 145 cm, so a step is 59.5 cm,
# taken as 60. Three of them is the walkway.
CHILD_STEP = 60.0
WALKWAY = 3 * CHILD_STEP                      # 180 cm, every aisle and the walk
WALK = WALKWAY                                # perimeter walk, same rule

# What 180 costs, recorded rather than smoothed over: the measured side aisle
# in `README.md` is 168 cm, "two 68 cm bodies pass", so 180 still passes two.
# But the escape gap in the same table is 200 cm, and 180 is under it. In a
# department aisle you can no longer slip past a Sweeper. The escapes are the
# cross aisles, the court and the main spine, which keep their widths.
ESCAPE_GAP = 200.0
TWO_BODIES = 168.0
GRILLE = (-900.0, 900.0)                      # mall entrance opening, south
ATRIUM = (-600.0, -900.0, 600.0, 900.0)       # void, open to the roof
SERVICE_DOORS = ((-3400.0, -3000.0), (3000.0, 3400.0))
COURT = (-1000.0, -1000.0, 1000.0, 1000.0)    # escalator court
CHAMFER = 250.0                               # angled corners on the court

# Chris's six, plus the things a floor needs that are not departments.
DEPARTMENTS = {"juniors", "jewelry", "mens", "perfume", "ladies", "shoes"}
SERVICES = {"service"}

# name, department, kind, xmin, ymin, xmax, ymax
#
# Six columns, and every column splits into its own rows. That is the whole
# trick: the Meriden sheet has no row that runs across the store, so a shopper
# leaving one department never faces the mouth of the one opposite. Every gap
# here is exactly 250 cm, the project's measured aisle; the irregularity comes
# from where the gaps fall, not from how wide they are.
CELLS = [
    # --- front counter band, either side of the mall entrance ---------------
    ("HANDBAGS",              "perfume", "counter",  -4100, -2450, -3000, -2100),
    ("PERFUME & COSMETICS",   "perfume", "counter",  -2750, -2450, -1200, -2100),
    ("JEWELRY",               "jewelry", "counter",   1200, -2450,  2750, -2100),
    ("ACCESSORIES",           "jewelry", "counter",   3000, -2450,  4100, -2100),

    # --- west column A, the deep wall bays ----------------------------------
    ("JUNIORS",               "juniors", "apparel",  -4100, -1800, -2900,  -600),
    ("MISSES SPORTSWEAR",     "ladies",  "apparel",  -4100,  -350, -2900,   700),
    ("WOMENS",                "ladies",  "apparel",  -4100,   950, -2900,  1350),
    ("LADIES FITTING ROOMS",  "service", "fitting",  -4100,  1600, -2900,  1840),

    # --- west column B, four shallower bays, none aligned with column A -----
    ("JUNIORS DRESSES",       "juniors", "apparel",  -2650, -1800, -1700, -1000),
    ("TEENS",                 "juniors", "apparel",  -2650,  -750, -1700,  -100),
    ("LADIES DRESSES",        "ladies",  "apparel",  -2650,   150, -1700,  1050),
    ("COATS",                 "ladies",  "apparel",  -2650,  1300, -1700,  1840),

    # --- west column C, the narrow strips facing the court ------------------
    ("PETITES",               "ladies",  "apparel",  -1450, -1800, -1000,  -700),
    ("ACTIVEWEAR",            "ladies",  "apparel",  -1450,  -450, -1000,   450),
    ("INTIMATE & SLEEPWEAR",  "ladies",  "apparel",  -1450,   700, -1000,  1840),

    # --- east column A, narrow strips facing the court ----------------------
    ("TIES & BELTS",          "mens",    "apparel",   1000, -1800,  1450, -1050),
    ("MENS FURNISHINGS",      "mens",    "apparel",   1000,  -800,  1450,   100),
    ("SHOE STOCK",            "shoes",   "service",   1000,   350,  1450,  1200),

    # --- east column B ------------------------------------------------------
    ("MENS CLOTHING",         "mens",    "apparel",   1700, -1800,  2800,  -700),
    ("MENS SPORTSWEAR",       "mens",    "apparel",   1700,  -450,  2800,   600),
    ("LADIES SHOES",          "shoes",   "shoes",     1700,   850,  2800,  1840),

    # --- east column C, four bays against the far wall ----------------------
    ("YOUNG MENS",            "mens",    "apparel",   3050, -1800,  4100,  -900),
    ("MENS FITTING ROOMS",    "service", "fitting",   3050,  -650,  4100,  -100),
    ("MENS SHOES",            "shoes",   "shoes",     3050,   150,  4100,  1050),
    ("SEASONAL PROMOTION",    "ladies",  "apparel",   3050,  1300,  4100,  1840),

    # --- behind the court, where the back aisle runs to both service doors --
    ("CUSTOMER SERVICE",      "service", "desk",       -750,  1300,  -150,  1840),
    ("ELEVATOR",              "service", "vertical",    100,  1300,   500,  1840),
]

# Small counter islands ringing the escalator well, the way the Meriden sheet
# rings its well with ladies accessories.
COURT_ISLANDS = [
    ("ACCESSORIES", -900, -820, -640, -560),
    ("ACCESSORIES",   640, -820,  900, -560),
    ("WATCHES",     -900,  560, -640,  820),
    ("ACCESSORIES",   640,  560,  900,  820),
]


def widen() -> None:
    """Grow every bay until its aisle is exactly one walkway.

    The table above is the grammar - which department sits beside which, and
    where the gaps fall. It is written with generous gaps so the grammar is
    readable. This closes them to `WALKWAY`, so that changing one constant
    repacks the whole floor instead of needing 27 rows retyped. A cell facing
    another cell moves half the excess, because the other cell moves too; a
    cell facing the shell or the main spine moves all of it, because they do
    not move at all.
    """
    inner = (PLATE[0] + WALKWAY, PLATE[1] + WALKWAY,
             PLATE[2] - WALKWAY, PLATE[3] - WALKWAY)
    spine = (COURT[0], COURT[2])      # the central corridor, x, at every y

    def nearest(cell, axis, side):
        """(gap, moves_too) to the closest thing on one side of one cell."""
        lo, hi = (3, 5) if axis == "x" else (4, 6)
        olo, ohi = (4, 6) if axis == "x" else (3, 5)
        best, shared = None, False
        for other in CELLS:
            if other is cell:
                continue
            if not (other[olo] < cell[ohi] and cell[olo] < other[ohi]):
                continue
            gap = (cell[lo] - other[hi]) if side < 0 else (other[lo] - cell[hi])
            if gap >= 0 and (best is None or gap < best):
                best, shared = gap, True
        if axis == "x":
            edge = inner[0] if side < 0 else inner[2]
            gap = (cell[lo] - edge) if side < 0 else (edge - cell[hi])
        else:
            edge = inner[1] if side < 0 else inner[3]
            gap = (cell[lo] - edge) if side < 0 else (edge - cell[hi])
        if gap >= 0 and (best is None or gap < best):
            best, shared = gap, False
        if axis == "x":   # the spine never yields
            gap = (cell[lo] - spine[1]) if side < 0 else (spine[0] - cell[hi])
            if gap >= 0 and (best is None or gap < best):
                best, shared = gap, False
        elif cell[3] < COURT[2] and COURT[0] < cell[5]:
            # Standing over the court's own x range, so the court is the
            # obstacle on this axis. It does not yield either.
            gap = (cell[4] - COURT[3]) if side < 0 else (COURT[1] - cell[6])
            if gap >= 0 and (best is None or gap < best):
                best, shared = gap, False
        return best, shared

    for _ in range(3):
        for i, cell in enumerate(CELLS):
            box = list(cell)
            for axis, side, index in (("x", -1, 3), ("x", 1, 5),
                                      ("y", -1, 4), ("y", 1, 6)):
                gap, shared = nearest(CELLS[i], axis, side)
                if gap is None or gap <= WALKWAY:
                    continue
                move = (gap - WALKWAY) / (2.0 if shared else 1.0)
                box[index] += -move if side < 0 else move
            CELLS[i] = tuple(box)


def checks() -> list[str]:
    """Refuse to draw a plan that would not stand up."""
    problems = []
    x0, y0, x1, y1 = PLATE
    inner = (x0 + WALK, y0 + WALK, x1 - WALK, y1 - WALK)

    for name, dept, _kind, cx0, cy0, cx1, cy1 in CELLS:
        if dept not in DEPARTMENTS | SERVICES:
            problems.append("%s: %r is not one of Chris's six" % (name, dept))
        if not (inner[0] <= cx0 < cx1 <= inner[2]
                and inner[1] <= cy0 < cy1 <= inner[3]):
            problems.append("%s: outside the perimeter walk" % name)
        if cx0 < COURT[2] and COURT[0] < cx1 and cy0 < COURT[3] and COURT[1] < cy1:
            problems.append("%s: stands in the escalator court" % name)
        if cx0 < ATRIUM[2] and ATRIUM[0] < cx1 and cy0 < ATRIUM[3] and ATRIUM[1] < cy1:
            problems.append("%s: stands in the atrium void" % name)

    for i, a in enumerate(CELLS):
        for b in CELLS[i + 1:]:
            if a[3] < b[5] and b[3] < a[5] and a[4] < b[6] and b[4] < a[6]:
                problems.append("%s overlaps %s" % (a[0], b[0]))
            # A gap you cannot walk down is worse than no gap at all: it is a
            # slot a body wedges in. 250 cm is this project's measured aisle.
            if a[4] < b[6] and b[4] < a[6]:
                gap = max(b[3] - a[5], a[3] - b[5])
                if 0 < gap < WALKWAY - 0.5:
                    problems.append("%s and %s leave a %.0f cm slot in x"
                                    % (a[0], b[0], gap))
            if a[3] < b[5] and b[3] < a[5]:
                gap = max(b[4] - a[6], a[4] - b[6])
                if 0 < gap < WALKWAY - 0.5:
                    problems.append("%s and %s leave a %.0f cm slot in y"
                                    % (a[0], b[0], gap))

    # Every door and the grille must open onto something walkable, not a cell.
    for lo, hi in (GRILLE,) + SERVICE_DOORS:
        mid = (lo + hi) / 2.0
        for name, _d, _k, cx0, cy0, cx1, cy1 in CELLS:
            if cx0 < mid < cx1 and (cy0 <= y0 + WALK or cy1 >= y1 - WALK):
                if cy1 >= y1 - WALK and not (cy1 <= y1 - WALK):
                    problems.append("%s blocks a doorway at x %.0f" % (name, mid))

    # Every glass panel must sit wholly inside a tile field, clear of the
    # aisle bulkhead: a diamond half in drywall is not a ceiling detail.
    reach = (DIAMOND[0] + DIAMOND[1]) / 2.0 * 0.70710678   # turned corner
    for cx, cy in DIAMOND_AT:
        if not any(fx0 <= cx - reach and cx + reach <= fx1
                   and fy0 <= cy - reach and cy + reach <= fy1
                   for fx0, fy0, fx1, fy1 in TILE_FIELDS):
            problems.append("glass panel at (%.0f, %.0f) is not wholly in a "
                            "tile field" % (cx, cy))

    missing = DEPARTMENTS - {c[1] for c in CELLS}
    if missing:
        problems.append("no cell for %s" % ", ".join(sorted(missing)))
    return problems


def write_csv() -> None:
    with CSV_PATH.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(["name", "department", "kind",
                         "xmin_cm", "ymin_cm", "xmax_cm", "ymax_cm", "area_m2"])
        for name, dept, kind, cx0, cy0, cx1, cy1 in CELLS:
            area = (cx1 - cx0) * (cy1 - cy0) / 10000.0
            writer.writerow([name, dept, kind,
                             int(cx0), int(cy0), int(cx1), int(cy1),
                             "%.0f" % area])


# --- drawing -----------------------------------------------------------------

S = 0.1          # px per cm
OX, OY = 70.0, 56.0


def sx(x: float) -> float:
    return OX + (x - PLATE[0]) * S


def sy(y: float) -> float:
    return OY + (PLATE[3] - y) * S


def rect(x0, y0, x1, y1, **kw) -> str:
    attrs = " ".join('%s="%s"' % (k.replace("_", "-"), v) for k, v in kw.items())
    return '<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" %s/>' % (
        sx(x0), sy(y1), (x1 - x0) * S, (y1 - y0) * S, attrs)


def label(name, x0, y0, x1, y1, size=6.2) -> str:
    """Centre the name, wrapping to two lines, turned 90 degrees if narrow."""
    cx, cy = (sx(x0) + sx(x1)) / 2.0, (sy(y0) + sy(y1)) / 2.0
    wide = (x1 - x0) * S
    tall = (y1 - y0) * S
    turn = wide < 55 and tall > wide
    words = name.split()
    lines, line = [], ""
    limit = (tall if turn else wide) / (size * 0.56)
    for word in words:
        trial = (line + " " + word).strip()
        if len(trial) > limit and line:
            lines.append(line)
            line = word
        else:
            line = trial
    lines.append(line)
    out = ['<text x="%.1f" y="%.1f" text-anchor="middle" font-size="%.1f"'
           ' letter-spacing="0.6" fill="#111"%s>' % (
               cx, cy - (len(lines) - 1) * size * 0.55, size,
               ' transform="rotate(-90 %.1f %.1f)"' % (cx, cy) if turn else "")]
    for i, text in enumerate(lines):
        out.append('<tspan x="%.1f" dy="%s">%s</tspan>' % (
            cx, "0" if i == 0 else "%.1f" % (size * 1.1),
            text.replace("&", "&amp;")))
    out.append("</text>")
    return "".join(out)


def draw() -> None:
    x0, y0, x1, y1 = PLATE
    p = []
    add = p.append
    add('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1010 760" '
        'width="1010" height="760" font-family="Helvetica, Arial, sans-serif">')
    add('<rect x="0" y="0" width="1010" height="760" fill="#ffffff"/>')

    # the shell
    add(rect(x0, y0, x1, y1, fill="none", stroke="#111", stroke_width="2.6"))
    # back-of-house wall and its two doors
    add('<g stroke="#111" stroke-width="2">')
    add('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (
        sx(x0), sy(2090), sx(SERVICE_DOORS[0][0]), sy(2090)))
    add('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (
        sx(SERVICE_DOORS[0][1]), sy(2090), sx(SERVICE_DOORS[1][0]), sy(2090)))
    add('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (
        sx(SERVICE_DOORS[1][1]), sy(2090), sx(x1), sy(2090)))
    add("</g>")

    # the mall entrance: a splayed vestibule, the way the sheet draws one
    add('<path d="M %.1f %.1f L %.1f %.1f L %.1f %.1f L %.1f %.1f" '
        'fill="none" stroke="#111" stroke-width="1.4"/>' % (
            sx(-1700), sy(-2450), sx(GRILLE[0]), sy(-2620),
            sx(GRILLE[1]), sy(-2620), sx(1700), sy(-2450)))
    add('<g stroke="#111" stroke-width="1">')
    for i in range(13):
        gx = GRILLE[0] + i * (GRILLE[1] - GRILLE[0]) / 12.0
        add('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (
            sx(gx), sy(-2700), sx(gx), sy(-2620)))
    add("</g>")

    # the escalator court: chamfered, like the Meriden well enclosure
    c0, c1, c2, c3 = COURT
    add('<path d="M %.1f %.1f L %.1f %.1f L %.1f %.1f L %.1f %.1f L %.1f %.1f '
        'L %.1f %.1f L %.1f %.1f L %.1f %.1f Z" fill="none" stroke="#111" '
        'stroke-width="1.4"/>' % (
            sx(c0 + CHAMFER), sy(c1), sx(c2 - CHAMFER), sy(c1),
            sx(c2), sy(c1 + CHAMFER), sx(c2), sy(c3 - CHAMFER),
            sx(c2 - CHAMFER), sy(c3), sx(c0 + CHAMFER), sy(c3),
            sx(c0), sy(c3 - CHAMFER), sx(c0), sy(c1 + CHAMFER)))
    # the void and the criss-cross escalators
    add(rect(*ATRIUM, fill="none", stroke="#111", stroke_width="1",
             stroke_dasharray="3 2"))
    add('<g stroke="#111" stroke-width="3.4" stroke-linecap="round" fill="none">')
    add('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (
        sx(-420), sy(-760), sx(380), sy(760)))
    add('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (
        sx(420), sy(-760), sx(-380), sy(760)))
    add("</g>")
    for name, ix0, iy0, ix1, iy1 in COURT_ISLANDS:
        add(rect(ix0, iy0, ix1, iy1, fill="none", stroke="#111",
                 stroke_width="1.1"))
        add(label(name, ix0, iy0, ix1, iy1, size=4.2))

    # the cells
    for name, _dept, kind, cx0, cy0, cx1, cy1 in CELLS:
        add(rect(cx0, cy0, cx1, cy1, fill="none", stroke="#111",
                 stroke_width="1.2"))
        text_y0, text_y1 = cy0, cy1
        if kind == "fitting":
            # Cubicles across the top of the bay, like the sheet draws them.
            depth = min(220.0, (cy1 - cy0) * 0.42)
            step = (cx1 - cx0) / 5.0
            add('<g stroke="#111" stroke-width="0.7">')
            for i in range(1, 5):
                add('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (
                    sx(cx0 + i * step), sy(cy1), sx(cx0 + i * step),
                    sy(cy1 - depth)))
            add('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (
                sx(cx0), sy(cy1 - depth), sx(cx1), sy(cy1 - depth)))
            add("</g>")
            text_y1 = cy1 - depth
        if kind == "counter":
            # The cash wrap sits in the lower half, so keep the name clear.
            text_y0 = cy0 + (cy1 - cy0) * 0.45
        add(label(name, cx0, text_y0, cx1, text_y1))

    # the two cash wraps that are already built into the map, drawn where the
    # map actually has them so a clash with a cell would show
    for cwx0, cwx1 in ((-3500, -3100), (3100, 3500)):
        add(rect(cwx0, -2290, cwx1, -2210, fill="#111", fill_opacity="0.18",
                 stroke="#111", stroke_width="0.9"))

    # edge callouts
    add('<g font-size="7" fill="#111" letter-spacing="1">')
    add('<text x="%.1f" y="%.1f" text-anchor="middle">MALL ENTRANCE</text>' % (
        sx(0), sy(-2700) + 15))
    add('<text x="%.1f" y="%.1f" text-anchor="middle">RECEIVING / DOCK</text>' % (
        sx(0), sy(2090) - 24))
    add('<text x="%.1f" y="%.1f" text-anchor="middle">STOCK / SALES</text>' % (
        sx(-3200), sy(2090) - 6))
    add('<text x="%.1f" y="%.1f" text-anchor="middle">SALES FLOOR</text>' % (
        sx(3200), sy(2090) - 6))
    add("</g>")

    # title block, bottom left, the way the sheet carries one
    ty = sy(PLATE[1]) + 46
    add('<text x="%.1f" y="%.1f" font-size="26" font-weight="700" '
        'letter-spacing="1" fill="#111">BOSWELL\'S</text>' % (OX, ty + 22))
    add('<text x="%.1f" y="%.1f" font-size="11" letter-spacing="2" '
        'fill="#111">JOHNSTOWN GALLERIA &#183; 1ST FLOOR</text>' % (OX, ty + 40))
    add('<text x="%.1f" y="%.1f" font-size="7.5" fill="#555">'
        'Proposed, 2026-11-23. 87 &#215; 48 m. Departments from '
        'THE-MALL-IS-STILL-TRADING.md section 13; grain from the Meriden '
        'first-floor diagram. Every aisle 250 cm. Not built.</text>'
        % (OX, ty + 56))
    add('<text x="%.1f" y="%.1f" font-size="7.5" fill="#555">'
        'The 18 m main aisle and the escalator court are inherited from the '
        'built map; narrowing the aisle means re-cutting the bulkhead '
        'ceiling laid in 86cbdfd.</text>' % (OX, ty + 68))
    add("</svg>")
    SVG_PATH.write_text("\n".join(p), encoding="utf-8")


# --- the section, because "high ceilings" is a vertical question ------------

# Heights as this session's ceiling audit read them out of the map, in cm.
SALES_CEILING = 514.0     # suspended tile underside, both wings
AISLE_SOFFIT = 513.0      # painted bulkhead over the main aisle, 86cbdfd
DECK_UNDERSIDE = 638.0    # mezzanine soffit
DECK_TOP = 650.0          # upper selling floor
ROOF_UNDERSIDE = 1488.0   # the only high ceiling in the building
DECKS_Y = ((-2700.0, -900.0), (900.0, 2700.0))   # at x = 0, north and south
SECTION_PATH = OUT / "BoswellsGroundFloor_Section.svg"


def draw_section() -> None:
    """A cut at x = 0, looking west. Chris, 2026-11-23: *'the ceilings are
    high, high ceilings with the glass on the ceilings. That's the most
    important thing.'* The cut is the honest answer to that: the building
    only has a high ceiling where it has no second storey, and at x = 0 the
    second storey stops only for the atrium."""
    y0, y1 = PLATE[1], 2700.0
    sc, left, base = 0.145, 70.0, 300.0

    def px(y):
        return left + (y - y0) * sc

    def pz(z):
        return base - z * sc

    p = ['<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 900 400" '
         'width="900" height="400" '
         'font-family="Helvetica, Arial, sans-serif">',
         '<rect x="0" y="0" width="900" height="400" fill="#ffffff"/>']
    add = p.append

    def line(ya, za, yb, zb, w=1.2, dash=""):
        add('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="#111" '
            'stroke-width="%s"%s/>' % (px(ya), pz(za), px(yb), pz(zb), w,
                                       ' stroke-dasharray="%s"' % dash
                                       if dash else ""))

    def note(y, z, text, anchor="start", size=7.5, fill="#111"):
        add('<text x="%.1f" y="%.1f" font-size="%s" text-anchor="%s" '
            'fill="%s">%s</text>' % (px(y), pz(z), size, anchor, fill, text))

    # floor, roof, outer walls
    line(y0, 0, y1, 0, 2.4)
    line(y0, ROOF_UNDERSIDE, y1, ROOF_UNDERSIDE, 2.4)
    line(y0, 0, y0, ROOF_UNDERSIDE, 2.4)
    line(y1, 0, y1, ROOF_UNDERSIDE, 2.4)

    # the second storey, and the drop ceiling tucked under it
    for a, b in DECKS_Y:
        add('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="#111" '
            'fill-opacity="0.12" stroke="#111" stroke-width="1.2"/>' % (
                px(a), pz(DECK_TOP), (b - a) * sc,
                (DECK_TOP - DECK_UNDERSIDE) * sc))
        line(a, SALES_CEILING, b, SALES_CEILING, 1.2)
        add('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="#111" '
            'fill-opacity="0.05" stroke="none"/>' % (
                px(a), pz(SALES_CEILING), (b - a) * sc, SALES_CEILING * sc))

    # the atrium: the one slot with no storey over it
    line(-900, 0, -900, ROOF_UNDERSIDE, 1.0, "4 3")
    line(900, 0, 900, ROOF_UNDERSIDE, 1.0, "4 3")
    add('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="#111" '
        'fill-opacity="0.04" stroke="none"/>' % (
            px(-900), pz(ROOF_UNDERSIDE), 1800 * sc, ROOF_UNDERSIDE * sc))
    line(-900, 1400, 900, 1400, 2.6)
    note(0, 1430, "GLASS / MIRRORED CANOPY + CHANDELIER", "middle", 7)
    note(0, 760, "the only high ceiling in the building:", "middle", 8)
    note(0, 640, "18 m wide, 14.9 m to the roof", "middle", 8)

    # dimension flags
    note(-2600, SALES_CEILING + 40, "suspended tile, 5.14 m")
    note(-2600, DECK_TOP + 60, "upper selling floor, 6.50 m")
    note(2650, ROOF_UNDERSIDE + 50, "roof, 14.88 m", "end")
    note(-2650, 40, "MALL ENTRANCE", "start", 7, "#555")
    note(2650, 40, "DOCK", "end", 7, "#555")

    add('<text x="70" y="348" font-size="15" font-weight="700" fill="#111">'
        'SECTION AT X = 0, LOOKING WEST</text>')
    add('<text x="70" y="366" font-size="7.5" fill="#555">'
        'A second storey and a high ceiling cannot share a floor. The decks '
        'cover everything except the 18 m atrium, so that slot is the whole '
        'of the high, glass-roofed room.</text>')
    add('<text x="70" y="379" font-size="7.5" fill="#555">'
        'Heights as the 2026-11-22 ceiling audit read them from '
        'LLK_Boswells_OpeningPass. Widening the glass means shrinking the '
        'mezzanine; nothing else moves it.</text>')
    add("</svg>")
    SECTION_PATH.write_text("\n".join(p), encoding="utf-8")


# --- the reflected ceiling plan ---------------------------------------------

# Chris, 2026-11-23: *"The glass should be in each four corners of the square,
# and the middle part is where the escalators are, the high ceiling with the
# chandelier. So there'll be a small patch on each part of the ceiling on the
# bottom floor only of glass that is an oblongish square of glass in each
# corner, like a diamond. So it's a square with diamonds."*
#
# "Bottom floor only" is the part that makes this buildable. The glass does
# not cut through the storey above - it cannot, there is a deck up there. It
# is a luminous panel let into the 5.14 m tile, lit from inside the 124 cm
# plenum between the tile and the mezzanine soffit. That is also exactly what
# a 1990s anchor did with a glazed ceiling coffer.
TILE_FIELDS = ((-4350.0, -2700.0, -900.0, 2100.0),
               (900.0, -2700.0, 4350.0, 2100.0))
PLENUM = DECK_UNDERSIDE - SALES_CEILING        # 124 cm to light it from
DIAMOND_AT = ((-2625.0, -1200.0), (2625.0, -1200.0),
              (-2625.0, 1200.0), (2625.0, 1200.0))
# Chris, 2026-11-23: *"The glass panels are much larger than that. Look at
# some reference pictures of Costco's glass ceiling."* The Costco interior is
# big bright rectangles repeated across the whole ceiling, each one reading as
# a panel of sky rather than a fitting. 500 x 350 was a light fitting. This is
# 16.6 m corner to corner, nearly half the width of its own tile field.
DIAMOND = (1400.0, 950.0)                      # oblong, then turned 45 degrees
CEILING_PATH = OUT / "BoswellsGroundFloor_Ceiling.svg"


def draw_ceiling() -> None:
    """Looking up. The square, its four diamonds, and the hole in the middle."""
    x0, y0, x1, y1 = PLATE
    p = ['<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1010 700" '
         'width="1010" height="700" '
         'font-family="Helvetica, Arial, sans-serif">',
         '<rect x="0" y="0" width="1010" height="700" fill="#ffffff"/>']
    add = p.append

    add(rect(x0, y0, x1, y1, fill="none", stroke="#111", stroke_width="2.6"))

    # the suspended tile, drawn as its grid so the panels read as let into it
    for fx0, fy0, fx1, fy1 in TILE_FIELDS:
        add(rect(fx0, fy0, fx1, fy1, fill="#111", fill_opacity="0.04",
                 stroke="#111", stroke_width="1.1"))
        add('<g stroke="#111" stroke-width="0.25" stroke-opacity="0.45">')
        step = 300.0
        gx = fx0 + step
        while gx < fx1:
            add('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (
                sx(gx), sy(fy0), sx(gx), sy(fy1)))
            gx += step
        gy = fy0 + step
        while gy < fy1:
            add('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (
                sx(fx0), sy(gy), sx(fx1), sy(gy)))
            gy += step
        add("</g>")

    # the painted bulkhead over the main aisle, a different ceiling on purpose
    add(rect(-900, -2700, 900, 2100, fill="#111", fill_opacity="0.10",
             stroke="#111", stroke_width="1.1"))
    add('<text x="%.1f" y="%.1f" text-anchor="middle" font-size="6.5" '
        'fill="#111" transform="rotate(-90 %.1f %.1f)" letter-spacing="1.4">'
        'PAINTED BULKHEAD 5.13 m</text>' % (
            sx(0), sy(-1900), sx(0), sy(-1900)))

    # the four diamonds
    half_w, half_h = DIAMOND[0] / 2.0, DIAMOND[1] / 2.0
    for cx, cy in DIAMOND_AT:
        pts = []
        for dx, dy in ((-half_w, -half_h), (half_w, -half_h),
                       (half_w, half_h), (-half_w, half_h)):
            # the corners of the oblong, turned 45 degrees about its centre,
            # so a rectangle reads as a diamond
            rx = (dx - dy) * 0.70710678
            ry = (dx + dy) * 0.70710678
            pts.append("%.1f,%.1f" % (sx(cx + rx), sy(cy + ry)))
        add('<polygon points="%s" fill="#111" fill-opacity="0.16" '
            'stroke="#111" stroke-width="1.5"/>' % " ".join(pts))
        add('<text x="%.1f" y="%.1f" text-anchor="middle" font-size="6" '
            'fill="#111">GLASS</text>' % (sx(cx), sy(cy) - 1))
        add('<text x="%.1f" y="%.1f" text-anchor="middle" font-size="5.2" '
            'fill="#555">%.0f &#215; %.0f cm</text>' % (
                sx(cx), sy(cy) + 7, DIAMOND[0], DIAMOND[1]))

    # the hole in the middle
    add(rect(*ATRIUM, fill="#ffffff", stroke="#111", stroke_width="1.8"))
    add('<circle cx="%.1f" cy="%.1f" r="13" fill="none" stroke="#111" '
        'stroke-width="1.2"/>' % (sx(0), sy(0)))
    add('<circle cx="%.1f" cy="%.1f" r="5" fill="#111"/>' % (sx(0), sy(0)))
    add('<g stroke="#111" stroke-width="0.6">')
    for i in range(12):
        ang = i * 3.14159265 / 6.0
        add('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (
            sx(0) + 5.5 * math.cos(ang),
            sy(0) + 5.5 * math.sin(ang),
            sx(0) + 12.5 * math.cos(ang),
            sy(0) + 12.5 * math.sin(ang)))
    add("</g>")
    add('<text x="%.1f" y="%.1f" text-anchor="middle" font-size="6.5" '
        'fill="#111">CHANDELIER</text>' % (sx(0), sy(-300)))
    add('<text x="%.1f" y="%.1f" text-anchor="middle" font-size="6" '
        'fill="#555">HIGH CEILING, 14.88 m</text>' % (sx(0), sy(-520)))
    add('<text x="%.1f" y="%.1f" text-anchor="middle" font-size="6" '
        'fill="#555">UNDER THE SMOKED MIRROR CANOPY</text>' % (sx(0), sy(500)))

    ty = sy(PLATE[1]) + 46
    add('<text x="%.1f" y="%.1f" font-size="22" font-weight="700" '
        'fill="#111">REFLECTED CEILING PLAN</text>' % (OX, ty + 20))
    add('<text x="%.1f" y="%.1f" font-size="10" letter-spacing="2" '
        'fill="#111">A SQUARE WITH DIAMONDS</text>' % (OX, ty + 36))
    add('<text x="%.1f" y="%.1f" font-size="7.5" fill="#555">'
        'Suspended tile at 5.14 m, 300 cm grid. Four glass panels %.0f '
        '&#215; %.0f cm turned 45&#176;, at (&#177;%.0f, &#177;%.0f), lit '
        'from the %.0f cm plenum between the tile and the mezzanine soffit '
        'at 6.38 m.</text>' % (OX, ty + 52, DIAMOND[0], DIAMOND[1],
                               abs(DIAMOND_AT[0][0]), abs(DIAMOND_AT[0][1]),
                               PLENUM))
    add('<text x="%.1f" y="%.1f" font-size="7.5" fill="#555">'
        'The glass is in the bottom floor\'s ceiling only; it does not cut '
        'the storey above, because a deck is up there. The middle is the one '
        'place that is open to the roof.</text>' % (OX, ty + 64))
    add("</svg>")
    CEILING_PATH.write_text("\n".join(p), encoding="utf-8")


def main() -> None:
    widen()
    problems = checks()
    if problems:
        for line in problems:
            print("GROUNDFLOOR refusing: %s" % line)
        raise SystemExit(1)
    write_csv()
    draw()
    draw_section()
    draw_ceiling()
    selling = sum((c[5] - c[3]) * (c[6] - c[4]) for c in CELLS
                  if c[1] in DEPARTMENTS) / 10000.0
    plate = (PLATE[2] - PLATE[0]) * (PLATE[3] - PLATE[1]) / 10000.0
    print("GROUNDFLOOR cells=%d departments=%d services=%d" % (
        len(CELLS),
        sum(1 for c in CELLS if c[1] in DEPARTMENTS),
        sum(1 for c in CELLS if c[1] in SERVICES)))
    for dept in sorted(DEPARTMENTS):
        cells = [c for c in CELLS if c[1] == dept]
        area = sum((c[5] - c[3]) * (c[6] - c[4]) for c in cells) / 10000.0
        print("GROUNDFLOOR %-9s %2d cells %5.0f m2  %s" % (
            dept, len(cells), area, ", ".join(c[0].title() for c in cells)))
    print("GROUNDFLOOR selling=%.0f m2 of plate=%.0f m2 (%.0f%%), "
          "circulation and court are the rest" % (
              selling, plate, 100.0 * selling / plate))
    print("GROUNDFLOOR walkway=%.0f cm (3 child steps of %.0f), escape gap %.0f"
          " is no longer met inside a department" % (WALKWAY, CHILD_STEP, ESCAPE_GAP))
    print("GROUNDFLOOR ceiling tile=%.2f m plenum=%.0f cm glass=4 x %.0fx%.0f"
          " at (+/-%.0f, +/-%.0f), atrium open to %.2f m" % (
              SALES_CEILING / 100.0, PLENUM, DIAMOND[0], DIAMOND[1],
              abs(DIAMOND_AT[0][0]), abs(DIAMOND_AT[0][1]),
              ROOF_UNDERSIDE / 100.0))
    print("GROUNDFLOOR wrote %s, %s, %s and %s" % (
        CSV_PATH.name, SVG_PATH.name, SECTION_PATH.name, CEILING_PATH.name))


if __name__ == "__main__":
    main()

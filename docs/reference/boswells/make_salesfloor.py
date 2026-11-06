"""Block-in of the lower sales floor, inside the room that already exists.

Units are centimeters. Axes are Unreal's: X/Y plan, Z up. The room is the
measured interior of LLK_Boswells_FirstRun, 2800 by 1800, walls at
x -1406/+1394 and y -906/+894. This file does not build those walls, the
dock volume, or the roof. It builds the fixtures a sales floor is made of.

Clear on purpose:
- |x| < 180 is the center walk. The Sweeper's patrol runs that line.
- y > 600 and |x| < 260 is the LoadingDock trigger. Nothing solid goes there.
- The gondola runs break for 200 cm at y = 0, so a side aisle can be reached.
- Wall runs sit on the wall. A gap under 68 cm is not a corridor, so none is left behind a counter.

The walk check pushes a 68 cm disc (capsule radius 34) from the customer
door through every aisle, the three patrol points, and the dock mouth.

Regenerate with: py docs/reference/boswells/make_salesfloor.py
"""

from __future__ import annotations

import csv
from pathlib import Path

OUT = Path(__file__).resolve().parent
OBJ = OUT / "BoswellsSalesFloor_Greybox.obj"
CSV = OUT / "BoswellsSalesFloor_Fixtures.csv"
SVG = OUT / "BoswellsSalesFloor_Plan.svg"

# Measured interior. FINDINGS-BOSWELLS-GEOMETRY.md.
X_MIN, X_MAX = -1406.0, 1394.0
Y_MIN, Y_MAX = -906.0, 894.0
ROOM_W = X_MAX - X_MIN  # 2800
ROOM_D = Y_MAX - Y_MIN  # 1800

# Sweeper capsule, same finding. The disc is what has to fit.
RADIUS = 34.0
BODY = 68.0

# Patrol spine. Points are y -200, +250, +650 along x = 0.
SPINE = 180.0
# LoadingDock trigger: x -252..252, y 612..948, z to 212.
DOCK = (-252.0, 612.0, 252.0, 948.0, 212.0)

# Eye 152 clears a 137 cm shelf. The lantern at rest, 133, does not.
# FINDINGS-LANTERN-ANCHOR-ORIGIN.md.
GONDOLA_H = 137.0
ISLAND = 110.0
WALL_D = 60.0
AISLE_MIN = 200.0  # the escape gap the dressing pass measured

# name, xmin, ymin, zmin, xmax, ymax, zmax, kind
BOXES: list[tuple] = []


def box(name, x0, y0, z0, x1, y1, z1, kind):
    BOXES.append((name, x0, y0, z0, x1, y1, z1, kind))


def islands():
    """Three clothing islands each side of the spine. Aisles stay >= 200."""
    west = X_MIN
    # 60 cm wall + three 110 cm islands, four equal aisles, landing on the spine.
    west_budget = -SPINE - west
    west_aisle = (west_budget - WALL_D - 3 * ISLAND) / 4
    if west_aisle < AISLE_MIN:
        raise SystemExit(f"west aisle {west_aisle:.1f} is under {AISLE_MIN}")
    cursor = west + WALL_D
    spans = []
    for i in range(3):
        cursor += west_aisle
        spans.append((f"W_{i}", cursor, cursor + ISLAND))
        cursor += ISLAND
    if abs(cursor + west_aisle - (-SPINE)) > 0.1:
        raise SystemExit("west stack does not meet the spine")

    east_budget = X_MAX - SPINE
    east_aisle = (east_budget - WALL_D - 3 * ISLAND) / 4
    if east_aisle < AISLE_MIN:
        raise SystemExit(f"east aisle {east_aisle:.1f} is under {AISLE_MIN}")
    cursor = SPINE
    for i in range(3):
        cursor += east_aisle
        spans.append((f"E_{i}", cursor, cursor + ISLAND))
        cursor += ISLAND
    if abs(cursor + east_aisle + WALL_D - X_MAX) > 0.1:
        raise SystemExit("east stack does not meet the wall")

    # 200 cm cross at y = 0. Each run is 300 cm, so the far end is 150 cm from the gap.
    for side, x0, x1 in spans:
        for i, (y0, y1) in enumerate(((-400.0, -100.0), (100.0, 400.0))):
            box(f"GONDOLA_{side}_{i}", x0, y0, 0, x1, y1, GONDOLA_H, "clothing")
    return west_aisle, east_aisle, spans


def glass_run():
    """Jewelry and perfume, on the west wall. No slot behind them."""
    x0, x1 = X_MIN, X_MIN + WALL_D
    y = -400.0
    i = 0
    while y + 140 <= 280:
        box(f"GLASS_{i}", x0, y, 0, x1, y + 140, 100, "jewelry")
        y += 180
        i += 1


def shoes():
    """Low tables on the east wall. 45 cm is under the hop, so the path goes in front."""
    x0 = X_MAX - WALL_D
    for i, y0 in enumerate((-300.0, -40.0, 220.0)):
        box(f"SHOE_TABLE_{i}", x0, y0, 0, X_MAX, y0 + 80, 45, "shoes")


def checkout():
    """Counters flank the walk. 90 cm is over the hop. The vault stays closed."""
    # North face sits 168 cm south of the gondolas, so two bodies can pass.
    box("CHECKOUT_W", -620, -648, 0, -320, -568, 90, "checkout")
    box("CHECKOUT_E", 320, -648, 0, 620, -568, 90, "checkout")


def doors_and_alarms():
    """Customer door at -Y. EAS pedestals inside it. Dock header is overhead."""
    box("DOOR_POST_W", -140, Y_MIN, 0, -90, -870, 220, "door")
    box("DOOR_POST_E", 90, Y_MIN, 0, 140, -870, 220, "door")
    box("DOOR_HEADER", -140, Y_MIN, 210, 140, -870, 240, "door")
    # 160 cm between the inner faces. The 68 cm disc passes one at a time.
    box("EAS_W", -110, -800, 0, -80, -760, 150, "alarm")
    box("EAS_E", 80, -800, 0, 110, -760, 150, "alarm")
    box("DOCK_HEADER", -240, 860, 260, 240, 890, 300, "door")


def racks(spans):
    """Round racks on the outer islands, in the end bays, not in an aisle."""
    outers = [s for s in spans if s[0] in ("W_0", "E_2")]
    n = 0
    for _name, x0, x1 in outers:
        cx = (x0 + x1) / 2
        for cy in (-480.0, 480.0):
            box(
                f"RACK_{n}",
                cx - 45,
                cy - 45,
                0,
                cx + 45,
                cy + 45,
                160,
                "clothing",
            )
            n += 1


def floor_boxes():
    """Solids a 176 cm body cannot walk through. Overhead headers are not those."""
    return [b for b in BOXES if b[3] < 170]


def hits(cx, cy, x0, y0, x1, y1):
    nx = min(max(cx, x0), x1)
    ny = min(max(cy, y0), y1)
    dx, dy = cx - nx, cy - ny
    return dx * dx + dy * dy <= RADIUS * RADIUS


def blocked(cx, cy, solids):
    if cx - RADIUS < X_MIN or cx + RADIUS > X_MAX:
        return True
    if cy - RADIUS < Y_MIN or cy + RADIUS > Y_MAX:
        return True
    return any(hits(cx, cy, b[1], b[2], b[4], b[5]) for b in solids)


def walk(west_aisle, east_aisle, spans):
    solids = floor_boxes()
    step = 10.0
    xs = []
    x = X_MIN + RADIUS
    while x <= X_MAX - RADIUS:
        xs.append(round(x, 1))
        x += step
    ys = []
    y = Y_MIN + RADIUS
    while y <= Y_MAX - RADIUS:
        ys.append(round(y, 1))
        y += step
    free = set()
    for cx in xs:
        for cy in ys:
            if not blocked(cx, cy, solids):
                free.add((cx, cy))

    def nearest(px, py):
        gx = min(xs, key=lambda v: abs(v - px))
        gy = min(ys, key=lambda v: abs(v - py))
        if (gx, gy) not in free:
            raise SystemExit(f"no stance at ({px:.0f}, {py:.0f})")
        return gx, gy

    start = nearest(0, -850)
    # Straight patrol, y -200 through the dock approach. x = 0.
    y = -200.0
    while y <= 700:
        nearest(0, y)
        y += step

    goals = {
        "patrol south": (0, -200),
        "patrol mid": (0, 250),
        "patrol dock": (0, 650),
        "dock mouth": (0, 780),
        "door": (0, -890),
    }
    # One sample in each north-south aisle, on the cross at y = 0.
    bands = []
    cursor = X_MIN + WALL_D
    for i in range(4):
        bands.append((cursor, cursor + west_aisle))
        cursor += west_aisle + (ISLAND if i < 3 else 0)
    cursor = SPINE
    for i in range(4):
        bands.append((cursor, cursor + east_aisle))
        cursor += east_aisle + (ISLAND if i < 3 else 0)
    for i, (a, b) in enumerate(bands):
        goals[f"aisle {i}"] = ((a + b) / 2, 0)

    want = {name: nearest(px, py) for name, (px, py) in goals.items()}
    seen = {start}
    queue = [start]
    while queue:
        cx, cy = queue.pop()
        for nx, ny in ((cx + step, cy), (cx - step, cy), (cx, cy + step), (cx, cy - step)):
            key = (round(nx, 1), round(ny, 1))
            if key in free and key not in seen:
                seen.add(key)
                queue.append(key)
    missing = [name for name, cell in want.items() if cell not in seen]
    if missing:
        raise SystemExit("disc cannot reach " + ", ".join(missing))
    return len(seen)


def assert_clear():
    if abs(ROOM_W - 2800) > 0.1 or abs(ROOM_D - 1800) > 0.1:
        raise SystemExit("room is not the measured 2800 by 1800")
    for name, x0, y0, z0, x1, y1, z1, _kind in BOXES:
        if x0 < X_MIN - 0.1 or x1 > X_MAX + 0.1 or y0 < Y_MIN - 0.1 or y1 > Y_MAX + 0.1:
            raise SystemExit(f"{name} leaves the room")
        if z0 < 160 and y1 > -250 and y0 < 700 and x0 < SPINE and x1 > -SPINE:
            raise SystemExit(f"{name} blocks the center walk")
        dx0, dy0, dx1, dy1, dz = DOCK
        if z0 < dz and x0 < dx1 and x1 > dx0 and y0 < dy1 and y1 > dy0:
            raise SystemExit(f"{name} sits in the LoadingDock volume")
        if y1 - y0 < 1 or x1 - x0 < 1 or z1 - z0 < 1:
            raise SystemExit(f"{name} has no size")
        # A slot you can see and cannot enter. Overhead pieces are not slots.
        if z0 < 1:
            behind = min(x0 - X_MIN, X_MAX - x1, y0 - Y_MIN, Y_MAX - y1)
            if 1 < behind < BODY:
                raise SystemExit(f"{name} leaves a {behind:.0f} cm slot")


def write_obj():
    lines = ["# Boswell's sales-floor fixtures. Centimeters, Unreal axes, Z up."]
    v = 1
    for name, x0, y0, z0, x1, y1, z1, _kind in BOXES:
        lines.append(f"o {name}")
        corners = [
            (x0, y0, z0),
            (x1, y0, z0),
            (x1, y1, z0),
            (x0, y1, z0),
            (x0, y0, z1),
            (x1, y0, z1),
            (x1, y1, z1),
            (x0, y1, z1),
        ]
        for x, y, z in corners:
            lines.append(f"v {x:.1f} {y:.1f} {z:.1f}")
        quads = (
            (0, 3, 2, 1),
            (4, 5, 6, 7),
            (0, 1, 5, 4),
            (1, 2, 6, 5),
            (2, 3, 7, 6),
            (3, 0, 4, 7),
        )
        for a, b, c, d in quads:
            lines.append(f"f {v+a} {v+b} {v+c}")
            lines.append(f"f {v+a} {v+c} {v+d}")
        v += 8
    OBJ.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")


def write_csv():
    with CSV.open("w", encoding="utf-8", newline="\n") as f:
        w = csv.writer(f)
        w.writerow("name xmin_cm ymin_cm zmin_cm xmax_cm ymax_cm zmax_cm kind".split())
        for row in BOXES:
            w.writerow(
                [
                    row[0],
                    f"{row[1]:.1f}",
                    f"{row[2]:.1f}",
                    f"{row[3]:.1f}",
                    f"{row[4]:.1f}",
                    f"{row[5]:.1f}",
                    f"{row[6]:.1f}",
                    row[7],
                ]
            )


def write_svg(west_aisle, east_aisle):
    scale = 0.12
    pad = 36

    def px(x, y):
        return pad + (x - X_MIN) * scale, pad + (Y_MAX - y) * scale

    width = pad * 2 + ROOM_W * scale
    height = pad * 2 + ROOM_D * scale
    fill = {
        "clothing": "#c4b49a",
        "jewelry": "#9bb7c9",
        "shoes": "#b7a48e",
        "checkout": "#d2c4a2",
        "door": "#8d8a84",
        "alarm": "#c47b7b",
    }
    parts = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width:.0f}" height="{height:.0f}" viewBox="0 0 {width:.0f} {height:.0f}">',
        "<style>text{font:12px sans-serif;fill:#222}</style>",
        f'<rect x="{pad}" y="{pad}" width="{ROOM_W * scale:.1f}" height="{ROOM_D * scale:.1f}" fill="#f4f1ea" stroke="#222"/>',
    ]
    for name, x0, y0, _z0, x1, y1, _z1, kind in BOXES:
        a, b = px(x0, y1)
        parts.append(
            f'<rect x="{a:.1f}" y="{b:.1f}" width="{(x1 - x0) * scale:.1f}" height="{(y1 - y0) * scale:.1f}" fill="{fill[kind]}" stroke="#444" stroke-width="0.6"><title>{name}</title></rect>'
        )
    labels = (
        (0, 200, "patrol"),
        (0, 760, "dock"),
        (0, -830, "door"),
        (-1240, 0, "jewelry aisle"),
        (1230, 0, "shoe aisle"),
    )
    for x, y, text in labels:
        sx, sy = px(x, y)
        parts.append(f'<text x="{sx:.0f}" y="{sy:.0f}" text-anchor="middle">{text}</text>')
    parts.append(
        f'<text x="{pad}" y="24">Boswell sales floor, cm. West aisle {west_aisle:.0f}. East aisle {east_aisle:.0f}. Spine {SPINE * 2:.0f}. North is up.</text>'
    )
    parts.append("</svg>")
    SVG.write_text("\n".join(parts) + "\n", encoding="utf-8", newline="\n")


def main():
    west_aisle, east_aisle, spans = islands()
    glass_run()
    shoes()
    checkout()
    doors_and_alarms()
    racks(spans)
    assert_clear()
    cells = walk(west_aisle, east_aisle, spans)
    write_obj()
    write_csv()
    write_svg(west_aisle, east_aisle)
    print(f"fixtures {len(BOXES)}")
    print(f"west aisle {west_aisle:.1f}")
    print(f"east aisle {east_aisle:.1f}")
    print(f"walk cells {cells}")
    print(f"obj {OBJ.stat().st_size}")


if __name__ == "__main__":
    main()

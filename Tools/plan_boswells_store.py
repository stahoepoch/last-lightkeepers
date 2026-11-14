"""Generate a two-floor fixture plan for the existing Boswell's Anchor plate.

This is a spatial plan and a set of asset-placement sockets, not a saved Unreal
map or a claim to reproduce Boscov's private interior blueprint. Dimensions
come from docs/PLAN-AUDIT-BOSWELLS-ANCHOR.md; department adjacency follows
Chris's recollection recorded there. The official mall directory establishes
the anchor and two levels, not the store's internal fixture positions.

Run: python Tools/plan_boswells_store.py
"""

from __future__ import annotations

import csv
from collections import deque
from dataclasses import dataclass
from html import escape
from pathlib import Path


OUT = Path(__file__).resolve().parents[1] / "docs/reference/boswells"
PLAN = OUT / "BoswellsFullStore_Plan.svg"
SOCKETS = OUT / "BoswellsFullStore_Sockets.csv"
X0, X1, Y0, Y1 = -4350, 4350, -2700, 2700  # cm; audited Anchor plate
UPPER_Z = 662.5


@dataclass(frozen=True)
class Rect:
    x0: float
    y0: float
    x1: float
    y1: float

    def overlaps(self, other: "Rect") -> bool:
        return (self.x0 < other.x1 and other.x0 < self.x1 and
                self.y0 < other.y1 and other.y0 < self.y1)

    def contains(self, other: "Rect") -> bool:
        return (self.x0 <= other.x0 and self.y0 <= other.y0 and
                other.x1 <= self.x1 and other.y1 <= self.y1)

    @property
    def area(self) -> float:
        return (self.x1 - self.x0) * (self.y1 - self.y0)


PLATE = Rect(X0, Y0, X1, Y1)
ATRIUM = Rect(-600, -900, 600, 900)

# These are reserved clear floor, not painted suggested walks. Any fixture
# socket that intersects one fails generation. They form loops around the
# atrium and connect the retail wings at three Y bands. Widths are >= 300 cm,
# before any real mesh is placed. The later Unreal pass must trace collision.
ROUTES = {
    "front promenade": Rect(-4000, -2350, 4000, -2050),
    "west outer aisle": Rect(-4250, -2050, -3950, 2050),
    "east outer aisle": Rect(3950, -2050, 4250, 2050),
    "west atrium bypass": Rect(-1060, -2050, -700, 2050),
    "east atrium bypass": Rect(700, -2050, 1060, 2050),
    "south cross aisle": Rect(-4250, -1450, 4250, -1150),
    "middle cross aisle west": Rect(-4250, -150, -600, 150),
    "middle cross aisle east": Rect(600, -150, 4250, 150),
    "north cross aisle": Rect(-4250, 1150, 4250, 1450),
    "service approach": Rect(-4250, 2050, 4250, 2350),
    "south entry": Rect(-300, -2700, 300, -900),
    "north spine": Rect(-300, 900, 300, 2350),
}


@dataclass(frozen=True)
class Socket:
    floor: str
    name: str
    department: str
    kind: str
    box: Rect
    z: float


def department(floor: str, x: float, y: float) -> str:
    if floor == "lower":
        if x < 0:
            return "juniors" if y < 0 else "ladies"
        return "mens" if y < 0 else "shoes"
    if x < 0:
        return "kids_babies" if y < 0 else "bedding_home"
    return "toys_seasonal" if y < 0 else "furniture_appliances"


def build_sockets() -> list[Socket]:
    sockets = []
    # The axes repeat for construction speed, but the objects do not: each
    # department has its own fixture vocabulary. The real mesh pass must
    # turn each socket into a dressed arrangement, not into another cube.
    columns = (-3650, -3150, -2650, -2150, -1650, -1250,
               1250, 1650, 2150, 2650, 3150, 3650)
    runs = (-1850, -650, 650, 1850)
    for floor in ("lower", "upper"):
        z = 0.0 if floor == "lower" else UPPER_Z
        for column_index, x in enumerate(columns):
            for y in runs:
                dept = department(floor, x, y)
                edge = abs(y) > 1000
                if dept in ("juniors", "ladies", "mens", "kids_babies"):
                    kind = ("round_rack" if column_index % 2 else "four_way") if edge else "clothing_run"
                    width, length = (180, 180) if edge else (180, 700)
                elif dept == "shoes":
                    kind = "shoe_bench" if column_index % 2 else "shoe_display"
                    width, length = (220, 180) if edge else (180, 580)
                elif dept == "toys_seasonal":
                    kind = "feature_table" if edge else "toy_gondola"
                    width, length = (220, 220) if edge else (180, 630)
                elif dept == "bedding_home":
                    kind = "bed_vignette" if not edge else "home_display"
                    width, length = (220, 320) if not edge else (220, 220)
                else:
                    kind = "sofa_vignette" if not edge else "appliance_pod"
                    width, length = (220, 360) if not edge else (220, 220)
                box = Rect(x - width / 2, y - length / 2,
                           x + width / 2, y + length / 2)
                sockets.append(Socket(floor, f"{floor.upper()}_{kind.upper()}_{x:+05d}_{y:+05d}",
                                      dept, kind, box, z))

    special = [
        ("lower", "CHECKOUT_W", "checkout", Rect(-2100, -2600, -1500, -2400)),
        ("lower", "CHECKOUT_E", "checkout", Rect(1500, -2600, 2100, -2400)),
        ("lower", "PERFUME_CASE", "perfume_jewelry", Rect(-3800, -2600, -2800, -2400)),
        ("lower", "JEWELRY_CASE", "perfume_jewelry", Rect(2800, -2600, 3800, -2400)),
        ("upper", "CANDY_AT_ESCALATOR", "candy", Rect(1100, -1130, 1700, -1040)),
        ("upper", "COURTESY_DESK", "services", Rect(-3500, 2400, -2600, 2600)),
        ("upper", "OPTICAL_HEARING", "services", Rect(-2200, 2400, -1100, 2600)),
        ("upper", "RESTROOM_GIFTWRAP", "services", Rect(1100, 2400, 2600, 2600)),
    ]
    for floor, name, dept, box in special:
        sockets.append(Socket(floor, name, dept, "counter_or_room",
                              box, 0.0 if floor == "lower" else UPPER_Z))
    return sockets


def validate(sockets: list[Socket]) -> None:
    seen = set()
    for item in sockets:
        if (item.floor, item.name) in seen:
            raise ValueError(f"duplicate socket {item.name}")
        seen.add((item.floor, item.name))
        if not PLATE.contains(item.box):
            raise ValueError(f"{item.name} leaves the plate")
        if item.floor == "upper" and item.box.overlaps(ATRIUM):
            raise ValueError(f"{item.name} crosses the upper atrium void")
        for route_name, route in ROUTES.items():
            if item.box.overlaps(route):
                raise ValueError(f"{item.name} blocks {route_name}")
    for i, a in enumerate(sockets):
        for b in sockets[i + 1:]:
            if a.floor == b.floor and a.box.overlaps(b.box):
                raise ValueError(f"{a.name} overlaps {b.name}")
    for floor in ("lower", "upper"):
        validate_horizontal_reach(floor, sockets)


def validate_horizontal_reach(floor: str, sockets: list[Socket]) -> None:
    """Check a 50 cm grid with 50 cm clearance around plan obstacles.

    This tests the drawn plan's connected floor. It cannot validate actual
    Unreal collision or the still-unbuilt vertical route.
    """
    cell = 50
    clearance = 50
    width = (X1 - X0) // cell
    height = (Y1 - Y0) // cell
    obstacles = [s.box for s in sockets if s.floor == floor]
    if floor == "upper":
        obstacles.append(ATRIUM)

    def point(index: tuple[int, int]) -> tuple[float, float]:
        ix, iy = index
        return X0 + (ix + 0.5) * cell, Y0 + (iy + 0.5) * cell

    def clear(index: tuple[int, int]) -> bool:
        x, y = point(index)
        if not (X0 + clearance <= x <= X1 - clearance and
                Y0 + clearance <= y <= Y1 - clearance):
            return False
        return all(not (box.x0 - clearance <= x <= box.x1 + clearance and
                        box.y0 - clearance <= y <= box.y1 + clearance)
                   for box in obstacles)

    def index(x: int, y: int) -> tuple[int, int]:
        return (int((x - X0) // cell), int((y - Y0) // cell))

    start = index(0, -2500) if floor == "lower" else index(800, -1000)
    if not clear(start):
        raise ValueError(f"{floor} entry or escalator landing is obstructed")
    reached = {start}
    frontier = deque([start])
    while frontier:
        x, y = frontier.popleft()
        for neighbor in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
            if (neighbor not in reached and 0 <= neighbor[0] < width and
                    0 <= neighbor[1] < height and clear(neighbor)):
                reached.add(neighbor)
                frontier.append(neighbor)
    # Four sales wings and the service end must all connect to the entry.
    targets = ((-3850, -750), (3850, -750), (-3850, 750),
               (3850, 750), (0, 2250))
    for x, y in targets:
        if index(x, y) not in reached:
            raise ValueError(f"{floor} route cannot reach ({x}, {y})")


def write_csv(sockets: list[Socket]) -> None:
    with SOCKETS.open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(("floor", "name", "department", "kind", "xmin_cm",
                         "ymin_cm", "xmax_cm", "ymax_cm", "floor_z_cm"))
        for s in sockets:
            writer.writerow((s.floor, s.name, s.department, s.kind,
                             s.box.x0, s.box.y0, s.box.x1, s.box.y1, s.z))


COLORS = {
    "juniors": "#876b89", "ladies": "#ad7189", "mens": "#647f91",
    "shoes": "#7295a5", "kids_babies": "#b28e68",
    "bedding_home": "#789281", "toys_seasonal": "#b69366",
    "furniture_appliances": "#648782", "checkout": "#c5a36c",
    "perfume_jewelry": "#af8bb5", "candy": "#d39f67",
    "services": "#7796a5",
}


def write_svg(sockets: list[Socket]) -> None:
    scale = 0.095
    panel_w, panel_h = 900, 630
    chunks = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{2*panel_w+30}" '
              f'height="{panel_h}" viewBox="0 0 {2*panel_w+30} {panel_h}">',
              '<rect width="100%" height="100%" fill="#111a22"/>',
              '<style>text{font-family:Arial,sans-serif;fill:#e7e8dc} '
              '.small{font-size:12px;fill:#bac8cb}</style>']

    def rect(r: Rect, ox: float, fill: str, opacity=1.0, stroke="none"):
        px = ox + (r.x0 - X0) * scale
        py = 50 + (Y1 - r.y1) * scale
        chunks.append(f'<rect x="{px:.1f}" y="{py:.1f}" '
                      f'width="{(r.x1-r.x0)*scale:.1f}" '
                      f'height="{(r.y1-r.y0)*scale:.1f}" '
                      f'fill="{fill}" fill-opacity="{opacity}" '
                      f'stroke="{stroke}"/>')

    for floor, ox in (("lower", 35), ("upper", panel_w + 50)):
        chunks.append(f'<text x="{ox}" y="28" font-size="20" font-weight="bold">'
                      f'BOSWELL\'S  /  {floor.upper()} SALES</text>')
        rect(PLATE, ox, "#35444b", stroke="#b8c9c7")
        for route in ROUTES.values():
            rect(route, ox, "#8fabb1", 0.28)
        if floor == "upper":
            rect(ATRIUM, ox, "#111a22", stroke="#e1bd74")
        else:
            rect(ATRIUM, ox, "#695f50", 0.45, "#e1bd74")
        for s in sockets:
            if s.floor == floor:
                rect(s.box, ox, COLORS[s.department], stroke="#121d22")
        labels = (("JUNIORS", -2500, -900), ("LADIES", -2500, 900),
                  ("MENS", 2500, -900), ("SHOES", 2500, 900)) if floor == "lower" else (
                  ("KIDS / BABIES", -2500, -900),
                  ("BEDDING / HOME", -2500, 900),
                  ("TOYS / SEASONAL", 2500, -900),
                  ("FURNITURE / APPLIANCES", 2500, 900))
        for label, x, y in labels:
            px = ox + (x - X0) * scale
            py = 50 + (Y1 - y) * scale
            chunks.append(f'<text x="{px:.1f}" y="{py:.1f}" text-anchor="middle" '
                          f'font-size="13" font-weight="bold" stroke="#17232b" '
                          f'stroke-width="3" paint-order="stroke">{escape(label)}</text>')
        for label, x, y in (("MALL / ENTRY", 0, -2480),
                            ("ESCALATOR WELL", 0, 0),
                            ("SERVICE / DOCK", 0, 2500)):
            px = ox + (x - X0) * scale
            py = 50 + (Y1 - y) * scale
            chunks.append(f'<text x="{px:.1f}" y="{py:.1f}" '
                          f'text-anchor="middle" class="small">{escape(label)}</text>')
    chunks.append('<text x="35" y="606" class="small">87 x 54 m per plate · 300 cm reserved route bands · colored blocks are fixture sockets, not finished meshes</text>')
    chunks.append('</svg>')
    PLAN.write_text("\n".join(chunks) + "\n", encoding="utf-8")


def main() -> None:
    sockets = build_sockets()
    validate(sockets)
    OUT.mkdir(parents=True, exist_ok=True)
    write_csv(sockets)
    write_svg(sockets)
    for floor in ("lower", "upper"):
        floor_sockets = [s for s in sockets if s.floor == floor]
        coverage = sum(s.box.area for s in floor_sockets) / PLATE.area * 100
        print(f"{floor}: {len(floor_sockets)} sockets; {coverage:.1f}% fixture footprint")
    print(f"Wrote {SOCKETS} and {PLAN}; 300 cm route bands stay clear in plan.")
    print("Unreal collision, child-scale sightlines, escalator and art still need editor verification.")


if __name__ == "__main__":
    main()

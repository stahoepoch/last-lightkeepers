"""Build centimeter-scale retail fixture masters for 48 Boswell's sockets.

The map's existing structural cubes remain as hidden collision proxies. These
meshes supply the visible store, including details that read at child height.
No mesh extends outside its authored socket. Re-run to reproduce the OBJs.
"""

from math import cos, pi, sin

from make_fixture_kit import Mesh


def bevel_box(m, x, y, z, sx, sy, sz, color, bevel=5):
    """An eight-corner soft furnishing instead of an engine cube."""
    hx, hy = sx / 2, sy / 2
    outline = [(-hx + bevel, -hy), (hx - bevel, -hy),
               (hx, -hy + bevel), (hx, hy - bevel),
               (hx - bevel, hy), (-hx + bevel, hy),
               (-hx, hy - bevel), (-hx, -hy + bevel)]
    low = [(x + u, y + v, z - sz / 2) for u, v in outline]
    high = [(u, v, z + sz / 2) for u, v, _ in low]
    m.face(list(reversed(low)), color)
    m.face(high, color)
    for i in range(8):
        j = (i + 1) % 8
        m.face([low[i], low[j], high[j], high[i]], color)


def shoe(m, x, y, z, color):
    # Seven-point sole and raised heel; length along X, width along Y.
    profile = [(-20, 0), (22, 0), (24, 4), (18, 8),
               (0, 10), (-11, 17), (-21, 15)]
    front = [(x + u, y - 10, z + v) for u, v in profile]
    back = [(u, y + 10, v) for u, _, v in front]
    m.face(list(reversed(front)), color)
    m.face(back, color)
    for i in range(len(front)):
        j = (i + 1) % len(front)
        m.face([front[i], front[j], back[j], back[i]], color)
    m.box(x + 1, y, z + 1.5, 46, 22, 3, "charcoal")
    m.box(x - 11, y, z + 12, 15, 18, 2, "cream")


def disk_on_front(m, x, y, z, radius, depth, color, segments=18):
    """A shallow round appliance door or control facing the customer aisle."""
    front = [(x + radius * cos(2 * pi * i / segments), y - depth / 2,
              z + radius * sin(2 * pi * i / segments))
             for i in range(segments)]
    back = [(u, y + depth / 2, v) for u, _, v in front]
    m.face(list(reversed(front)), color)
    m.face(back, color)
    for i in range(segments):
        j = (i + 1) % segments
        m.face([front[i], front[j], back[j], back[i]], color)


def shoe_display(long, variant):
    m = Mesh()
    length = 500 if long else 150
    width = 158 if long else 190
    m.box(0, 0, 43, width - 8, length - 14, 86, "dark_wood")
    for z in (87, 119):
        m.box(0, 0, z, width, length, 5, "steel")
        m.box(0, -length / 2 + 2, z + 3, width, 4, 3, "brass")
    positions = (-205, -115, -25, 65, 155, 215) if long else (-35, 35)
    colors = ("navy", "rose", "denim", "oat") if variant == 0 else ("pine", "cream", "burgundy", "sky")
    for row, y in enumerate(positions):
        for side, x in enumerate((-38, 38)):
            shoe(m, x, y, 92, colors[(row + side) % 4])
            # Display card is deliberately blank: a route fixture must not
            # turn the label into readable gameplay information.
            m.box(x, y - 19, 115, 22, 2, 9, "cream")
    return m


def shoe_bench(long, variant):
    m = Mesh()
    places = (-128, 128) if long else (0,)
    width, depth = (140, 115) if long else (180, 140)
    for y in places:
        for x in (-width / 2 + 11, width / 2 - 11):
            for yy in (y - depth / 2 + 11, y + depth / 2 - 11):
                m.box(x, yy, 21, 5, 5, 42, "steel")
        m.box(0, y, 43, width, depth, 6, "dark_wood")
        bevel_box(m, 0, y, 49, width - 6, depth - 6, 9,
                  "burgundy" if variant == 0 else "navy", 9)
        for x, col in ((-46, "oat"), (46, "denim")):
            shoe(m, x, y, 55, col)
    return m


def bed_vignette(variant):
    m = Mesh()
    m.box(0, 0, 1, 200, 306, 2, "oat")
    for x in (-75, 75):
        for y in (-119, 119):
            m.box(x, y, 17, 12, 12, 34, "dark_wood")
    m.box(0, 0, 32, 182, 274, 10, "dark_wood")
    bevel_box(m, 0, 0, 45, 172, 252, 22, "cream", 8)
    m.box(0, 143, 69, 184, 18, 92, "dark_wood")
    for x in (-61, 61):
        m.box(x, 134, 71, 46, 3, 70, "brass")
    bevel_box(m, 0, -52, 60, 170, 148, 9,
              "denim" if variant else "rose", 9)
    m.box(0, -126, 60, 168, 5, 7, "cream")
    for x in (-45, 45):
        bevel_box(m, x, 87, 65, 71, 40, 15, "oat", 10)
        m.box(x, 87, 73, 51, 2, 2, "cream")
    return m


def home_display(variant):
    m = Mesh()
    m.box(0, 0, 43, 178, 172, 86, "dark_wood")
    m.box(0, 0, 89, 190, 184, 6, "cream")
    for x in (-57, 0, 57):
        m.box(x, -87, 42, 1.5, 2, 70, "brass")
        m.box(x + 15, -88, 45, 4, 2, 2, "brass")
    for x in (-54, 54):
        m.cylinder(x, 42, 93, 105, 10, "steel", 12)
        m.cylinder(x, 42, 105, 139, 2.5, "brass", 10)
        m.cylinder(x, 42, 139, 158, 19, "cream", 12)
        m.cylinder(x, 42, 158, 160, 11, "brass", 12)
    for layer in range(3):
        bevel_box(m, 0, -43, 96 + layer * 7,
                  52 - layer * 5, 42, 6,
                  ("oat", "denim", "rose")[(layer + variant) % 3], 4)
    return m


def feature_table(variant):
    m = Mesh()
    for x in (-75, 75):
        for y in (-65, 65):
            m.box(x, y, 34, 7, 7, 68, "dark_wood")
    m.box(0, 0, 69, 180, 160, 7, "dark_wood")
    m.box(0, 0, 74, 194, 174, 5, "cream")
    for j, (x, y) in enumerate(((-55, -46), (0, -46), (55, -46),
                                (-55, 42), (0, 42), (55, 42))):
        color = ("butter", "mint", "rose", "denim")[(j + variant) % 4]
        if j % 3 == 0:
            # Rockets with fins and a contrasting nose.
            m.cylinder(x, y, 78, 101, 8, color, 10)
            m.cylinder(x, y, 101, 108, 4, "brass", 10)
            for dx in (-8, 8):
                m.box(x + dx, y, 83, 3, 12, 9, "charcoal")
        elif j % 3 == 1:
            m.box(x, y, 89, 30, 25, 20, color)
            m.box(x, y - 13, 90, 22, 1, 12, "cream")
            m.cylinder(x, y, 99, 105, 5, "brass", 8)
        else:
            bevel_box(m, x, y, 88, 32, 28, 19, color, 5)
            m.box(x, y - 15, 88, 24, 2, 4, "charcoal")
    return m


def toy_gondola(variant):
    m = Mesh()
    m.box(0, 0, 10, 158, 544, 20, "steel")
    m.box(0, 0, 87, 14, 530, 134, "cream")
    for x in (-42, 42):
        for z in (55, 103):
            m.box(x, 0, z, 76, 520, 6, "steel")
            m.box(x + (39 if x > 0 else -39), 0, z + 3,
                  3, 520, 4, "brass")
        for j, y in enumerate((-220, -165, -110, -55, 0, 55, 110, 165, 220)):
            color = ("rose", "butter", "mint", "navy")[(j + variant + (x > 0)) % 4]
            m.box(x, y, 77, 42, 35, 34, color)
            m.box(x + (21 if x > 0 else -21), y, 79, 1, 23, 17, "cream")
            m.box(x, y, 114, 39, 32, 28, color)
    return m


def sofa_vignette(variant):
    m = Mesh()
    cloth = "denim" if variant else "burgundy"
    m.box(0, 0, 1, 208, 332, 2, "oat")
    for x in (-72, 72):
        for y in (-105, 105):
            m.box(x, y, 11, 9, 9, 22, "dark_wood")
    m.box(0, 0, 27, 184, 268, 28, "dark_wood")
    for y in (-85, 0, 85):
        bevel_box(m, 0, y, 50, 149, 80, 20, cloth, 10)
        m.box(0, y + 39, 52, 145, 2, 8, "charcoal")
    m.box(0, 127, 88, 184, 30, 82, cloth)
    for x in (-84, 84):
        bevel_box(m, x, 0, 62, 23, 246, 72, cloth, 7)
        bevel_box(m, x / 2, 38, 65, 48, 42, 17,
                  "cream" if x < 0 else "rose", 6)
    return m


def appliance_pod(variant):
    m = Mesh()
    m.box(0, 0, 10, 188, 120, 20, "charcoal")
    for x in (-45, 45):
        m.box(x, 0, 72, 79, 86, 104, "white_steel")
        m.box(x, -43, 105, 66, 4, 20, "steel")
        disk_on_front(m, x, -45, 68, 27, 3, "charcoal")
        disk_on_front(m, x, -47, 68, 22, 3, "steel")
        disk_on_front(m, x, -49, 68, 17, 2,
                      "navy" if variant else "charcoal")
        for dx in (-22, -11, 0, 11, 22):
            disk_on_front(m, x + dx, -47, 108, 2.5, 2, "brass", 8)
        m.box(x + 27, -45, 90, 10, 3, 3, "charcoal")
    return m


MAKERS = {
    "ShoeDisplay_Long": lambda v: shoe_display(True, v),
    "ShoeDisplay_Short": lambda v: shoe_display(False, v),
    "ShoeBench_Long": lambda v: shoe_bench(True, v),
    "ShoeBench_Short": lambda v: shoe_bench(False, v),
    "BedVignette": bed_vignette,
    "HomeDisplay": home_display,
    "FeatureTable": feature_table,
    "ToyGondola": toy_gondola,
    "SofaVignette": sofa_vignette,
    "AppliancePod": appliance_pod,
}


if __name__ == "__main__":
    for name, maker in MAKERS.items():
        for variant in (0, 1):
            maker(variant).write(
                f"SM_BSW_Department_{name}_{'A' if variant == 0 else 'B'}")

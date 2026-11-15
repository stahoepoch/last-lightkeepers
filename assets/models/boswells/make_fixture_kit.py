"""Generate portable, centimeter-scale Boswell's retail fixture masters.

These are deliberately simple authored retail silhouettes for the first
traversal slice. Re-run this script to rebuild the OBJ/MTL sources.
"""

from math import cos, pi, sin
from pathlib import Path


ROOT = Path(__file__).resolve().parent
COLORS = {
    "steel": (0.33, 0.37, 0.39),
    "brass": (0.40, 0.30, 0.16),
    "dark_wood": (0.18, 0.12, 0.09),
    "navy": (0.12, 0.17, 0.25),
    "burgundy": (0.31, 0.13, 0.15),
    "pine": (0.16, 0.27, 0.23),
    "oat": (0.57, 0.50, 0.40),
    "cream": (0.72, 0.66, 0.55),
    "denim": (0.25, 0.34, 0.47),
    "white_steel": (0.68, 0.70, 0.67),
    "rose": (0.53, 0.27, 0.35),
    "sky": (0.30, 0.47, 0.58),
    "mint": (0.33, 0.52, 0.43),
    "lavender": (0.49, 0.40, 0.58),
    "butter": (0.66, 0.54, 0.27),
    "sage_sign": (0.33, 0.43, 0.28),
    "charcoal": (0.06, 0.08, 0.09),
}
GARMENTS = ("navy", "burgundy", "pine", "oat", "denim", "cream")


class Mesh:
    def __init__(self):
        self.vertices = []
        self.uvs = []
        self.faces = []

    def vertex(self, point, uv):
        self.vertices.append(point)
        self.uvs.append(uv)
        return len(self.vertices)

    def face(self, points, material):
        count = len(points)
        if count == 4:
            uvs = ((0, 0), (1, 0), (1, 1), (0, 1))
        else:
            uvs = [(0.5 + 0.5 * cos(2 * pi * i / count),
                    0.5 + 0.5 * sin(2 * pi * i / count))
                   for i in range(count)]
        self.faces.append((material, [self.vertex(p, uv)
                                      for p, uv in zip(points, uvs)]))

    def box(self, cx, cy, cz, sx, sy, sz, material, angle=0.0):
        ca, sa = cos(angle), sin(angle)
        p = []
        for z in (-sz / 2, sz / 2):
            for y in (-sy / 2, sy / 2):
                for x in (-sx / 2, sx / 2):
                    p.append((cx + x * ca - y * sa,
                              cy + x * sa + y * ca, cz + z))
        for ids in ((0, 2, 3, 1), (4, 5, 7, 6), (0, 1, 5, 4),
                    (2, 6, 7, 3), (0, 4, 6, 2), (1, 3, 7, 5)):
            self.face([p[i] for i in ids], material)

    def cylinder(self, cx, cy, bottom, top, radius, material, segments=12):
        low = [(cx + radius * cos(2 * pi * i / segments),
                cy + radius * sin(2 * pi * i / segments), bottom)
               for i in range(segments)]
        high = [(x, y, top) for x, y, _ in low]
        self.face(list(reversed(low)), material)
        self.face(high, material)
        for i in range(segments):
            j = (i + 1) % segments
            self.face([low[i], low[j], high[j], high[i]], material)

    def shirt(self, x, y, color, angle=0.0):
        # Lightweight sale-floor cloth with sleeves, hem, and hanger.
        self.box(x, y, 105, 43, 5, 59, color, angle)
        self.box(x - 27 * cos(angle), y - 27 * sin(angle), 123,
                 14, 5, 27, color, angle)
        self.box(x + 27 * cos(angle), y + 27 * sin(angle), 123,
                 14, 5, 27, color, angle)
        self.box(x, y, 75, 42, 6, 3, "dark_wood", angle)
        self.box(x, y, 145, 37, 3, 2, "dark_wood", angle)

    def write(self, stem):
        obj = ROOT / (stem + ".obj")
        lines = ["# Boswell's retail fixture, centimeters, Z up",
                 "mtllib BoswellsFixtures.mtl", "o " + stem]
        for x, y, z in self.vertices:
            lines.append(f"v {x:.4f} {y:.4f} {z:.4f}")
        for u, v in self.uvs:
            lines.append(f"vt {u:.4f} {v:.4f}")
        last = None
        for material, indices in sorted(self.faces, key=lambda face: face[0]):
            if material != last:
                lines.append("usemtl " + material)
                last = material
            lines.append("f " + " ".join(f"{i}/{i}" for i in indices))
        obj.write_text("\n".join(lines) + "\n", encoding="utf-8")
        print(obj.name, len(self.vertices), "vertices")


def clothing_run():
    mesh = Mesh()
    for x in (-51, 51):
        for y in (-285, 285):
            mesh.box(x, y, 82, 5, 5, 164, "steel")
    for x in (-51, 51):
        mesh.box(x, 0, 157, 4, 570, 4, "steel")
        mesh.box(x, 0, 8, 18, 570, 5, "brass")
    for i in range(17):
        y = -260 + i * 32.5
        mesh.shirt(-51, y, GARMENTS[i % len(GARMENTS)])
        mesh.shirt(51, y, GARMENTS[(i + 3) % len(GARMENTS)])
    mesh.write("SM_BSW_ClothingRun")


def four_way():
    mesh = Mesh()
    mesh.cylinder(0, 0, 0, 166, 5, "steel")
    mesh.cylinder(0, 0, 2, 6, 54, "brass")
    mesh.box(0, 0, 154, 162, 4, 4, "steel")
    mesh.box(0, 0, 154, 4, 162, 4, "steel")
    for i in range(4):
        angle = i * pi / 2
        for j in range(3):
            d = 33 + j * 20
            x, y = d * cos(angle), d * sin(angle)
            mesh.shirt(x, y, GARMENTS[(i * 2 + j) % len(GARMENTS)], angle)
    mesh.write("SM_BSW_FourWay")


def round_rack():
    mesh = Mesh()
    mesh.cylinder(0, 0, 0, 156, 5, "steel")
    mesh.cylinder(0, 0, 2, 6, 54, "brass")
    for i in range(16):
        a = 2 * pi * i / 16
        x, y = 61 * cos(a), 61 * sin(a)
        mesh.box(x, y, 153, 24, 4, 4, "steel", a + pi / 2)
        mesh.shirt(x, y, GARMENTS[i % len(GARMENTS)], a + pi / 2)
    mesh.write("SM_BSW_RoundRack")


def kids_rolling_rack():
    """Low, wheeled children\'s rail like the photographed sales floor."""
    mesh = Mesh()
    for x in (-67, 67):
        for y in (-30, 30):
            mesh.cylinder(x, y, 0, 7, 5, "steel", 8)
            mesh.box(x, y, 8, 12, 12, 3, "dark_wood")
    for x in (-65, 65):
        mesh.box(x, 0, 76, 4, 58, 138, "white_steel")
    mesh.box(0, 0, 139, 132, 5, 5, "white_steel")
    mesh.box(0, 0, 15, 132, 45, 4, "white_steel")
    colors = ("rose", "sky", "mint", "lavender", "butter", "cream")
    for i, x in enumerate((-52, -31, -10, 11, 32, 53)):
        color = colors[i]
        mesh.box(x, 0, 105, 5, 30, 48, color)
        mesh.box(x, -21, 120, 5, 15, 25, color)
        mesh.box(x, 21, 120, 5, 15, 25, color)
        mesh.box(x, 0, 130, 6, 20, 3, "dark_wood")
    mesh.write("SM_BSW_KidsRollingRack")


def kids_sales_desk():
    mesh = Mesh()
    mesh.box(0, 0, 42, 160, 74, 84, "cream")
    mesh.box(0, -38, 90, 166, 9, 10, "dark_wood")
    mesh.box(0, 0, 91, 174, 84, 6, "oat")
    mesh.box(-45, 7, 102, 38, 24, 16, "charcoal")
    mesh.box(52, 14, 96, 25, 20, 4, "white_steel")
    # The short sign mast is structural wayfinding, not game information.
    for x in (-59, 59):
        mesh.box(x, 30, 148, 3, 3, 105, "white_steel")
    mesh.box(0, 30, 193, 122, 4, 32, "sage_sign")
    mesh.write("SM_BSW_KidsSalesDesk")


# ---------------------------------------------------------------------------
# Cover fixtures. Added 2026-11-14 against docs/FINDINGS-KIDS-FLOOR-STEALTH.md.
#
# Sight is traced between capsule CENTRES - 88 cm standing, 40 cm crouched -
# while the keeper looks out from 152 cm. The rolling racks block only between
# 88 and 130 cm, so crouching behind one drops the traced point below the cover
# and makes the keeper MORE visible. The racks stay as they are: they are an
# honest open rack and they should keep reading as open.
#
# These two are the deliberate answer, and they are solid to the floor so there
# is no band to fall below.
TRACE_CROUCHED_CM = 40.0
TRACE_STANDING_CM = 88.0
EYE_HEIGHT_CM = 152.0


def folded_goods_table():
    """A 30-inch folded-stock table. Blocks a crouched keeper and NOT a
    standing one: you have to duck behind it, and because the whole silhouette
    stays under chest height it never looks like something you could stand
    behind. That misreading is what the racks already cost us."""
    mesh = Mesh()
    mesh.box(0, 0, 5, 134, 64, 10, "steel")             # toe recess, 0-10
    mesh.box(0, 0, 37, 150, 80, 54, "white_steel")      # painted plinth, 10-64
    mesh.box(0, 0, 68, 160, 90, 8, "oat")               # laminate top, 64-72
    for x, y, colour in ((-38, -20, "denim"), (38, -20, "cream"),
                         (-38, 20, "burgundy"), (38, 20, "mint")):
        mesh.box(x, y, 76, 36, 34, 8, colour)           # folded stock, 72-80
    mesh.write("SM_BSW_FoldedGoodsTable")
    return mesh


def service_counter():
    """A 40-inch service counter. Blocks the chest at 88 and stays fifty
    centimetres below the eye at 152, so a keeper stands behind it, watches the
    machine over the top, and is not seen. The findings found that affordance
    by accident in the racks and asked for it to be made deliberate before
    someone 'fixes' the trace to use eye height and deletes it."""
    mesh = Mesh()
    mesh.box(0, 6, 5, 168, 49, 10, "charcoal")          # toe recess, 0-10
    mesh.box(0, 2, 53, 180, 57, 86, "cream")            # carcass, 10-96
    mesh.box(0, 0, 92, 182, 67, 8, "white_steel")       # apron, 88-96
    mesh.box(0, 0, 99, 186, 71, 6, "dark_wood")         # laminate top, 96-102
    mesh.write("SM_BSW_ServiceCounter")
    return mesh


def wall_merch_bay():
    """A double-sided merchandise gondola. Blocks at every traced height AND
    above the eye, so it is the third kind of cover and the only one that
    costs you something: you are completely hidden and completely blind.

    The findings' second recommendation was *"give the floor more than one
    column"* - one dependable blocker in a department this size means one
    hiding place. This is a blocker that is furniture rather than structure,
    so it can be placed where the stealth needs it instead of where the
    building happens to hold the roof up.

    Long on purpose. A 240 cm run breaks a sightline and makes the player
    choose an end to go round, which is a decision; a narrow blocker is just
    a pillar you strafe past."""
    mesh = Mesh()
    mesh.box(0, 0, 5, 228, 62, 10, "charcoal")           # plinth, 0-10
    mesh.box(0, 0, 100, 240, 70, 180, "white_steel")     # carcass, 10-190
    # Shelf bands, both faces. These are what make it read as merchandising
    # rather than a wall, and they catch the lantern at different heights.
    for z in (46, 86, 126, 166):
        for y in (-34, 34):
            mesh.box(0, y, z, 236, 6, 5, "steel")
    # Faced stock on the shelves, alternating so neither side reads as empty.
    palette = ("denim", "oat", "rose", "sky", "mint", "butter")
    for index, z in enumerate((54, 94, 134, 174)):
        for side, y in ((0, -30), (1, 30)):
            for slot in range(4):
                x = -90 + slot * 60
                mesh.box(x, y, z, 52, 10, 14, palette[(index * 2 + side + slot) % len(palette)])
    mesh.write("SM_BSW_WallMerchBay")
    return mesh


def cover_check(table, counter, bay=None):
    """These two exist for a rule, so the rule is checked here rather than
    trusted. A fixture that silently stops being cover is the failure the
    racks already demonstrated, and it cost a live editor session to find."""
    def extent(mesh):
        zs = [z for _, _, z in mesh.vertices]
        return min(zs), max(zs)

    problems = []
    t_low, t_high = extent(table)
    c_low, c_high = extent(counter)

    if t_low > 0.01:
        problems.append("the table does not reach the floor, so a crouched "
                        "keeper is visible beneath it")
    if t_high <= TRACE_CROUCHED_CM:
        problems.append(f"the table tops at {t_high:.0f} cm and does not hide a "
                        f"crouched keeper traced at {TRACE_CROUCHED_CM:.0f} cm")
    if t_high >= TRACE_STANDING_CM:
        problems.append(f"the table tops at {t_high:.0f} cm and hides a STANDING "
                        f"keeper traced at {TRACE_STANDING_CM:.0f} cm. It is "
                        f"meant to be the fixture that makes you duck")
    if c_low > 0.01:
        problems.append("the counter does not reach the floor")
    if c_high <= TRACE_STANDING_CM:
        problems.append(f"the counter tops at {c_high:.0f} cm and does not hide "
                        f"a standing keeper traced at {TRACE_STANDING_CM:.0f} cm")
    if c_high >= EYE_HEIGHT_CM:
        problems.append(f"the counter tops at {c_high:.0f} cm, at or above the "
                        f"{EYE_HEIGHT_CM:.0f} cm eye, so the keeper can no "
                        f"longer see over it")

    if bay is not None:
        b_low, b_high = extent(bay)
        if b_low > 0.01:
            problems.append("the merchandise bay does not reach the floor")
        if b_high <= EYE_HEIGHT_CM:
            problems.append(f"the bay tops at {b_high:.0f} cm, at or below the "
                            f"{EYE_HEIGHT_CM:.0f} cm eye. Its whole job is to be "
                            f"the cover you cannot see over - if the player can "
                            f"watch from behind it, it is a tall counter and the "
                            f"floor has two of those instead of three kinds")

    if problems:
        raise SystemExit("cover rule broken:\n  - " + "\n  - ".join(problems))

    line = (f"cover ok: table 0-{t_high:.0f} (duck), "
            f"counter 0-{c_high:.0f} (stand and watch)")
    if bay is not None:
        line += f", bay 0-{extent(bay)[1]:.0f} (hidden and blind)"
    print(line + f", against crouch {TRACE_CROUCHED_CM:.0f} / stand "
                 f"{TRACE_STANDING_CM:.0f} / eye {EYE_HEIGHT_CM:.0f}")


def main():
    with (ROOT / "BoswellsFixtures.mtl").open("w", encoding="utf-8") as file:
        file.write("# Authored palette for Boswell's fixture masters\n")
        for name, (r, g, b) in COLORS.items():
            file.write(f"newmtl {name}\nKd {r} {g} {b}\nKa 0 0 0\nKs 0.08 0.08 0.08\nNs 12\n\n")
    clothing_run()
    four_way()
    round_rack()
    kids_rolling_rack()
    kids_sales_desk()
    cover_check(folded_goods_table(), service_counter(), wall_merch_bay())


if __name__ == "__main__":
    main()

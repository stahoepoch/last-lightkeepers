"""Generate portable, centimeter-scale Boswell's apparel fixture masters.

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


def main():
    with (ROOT / "BoswellsFixtures.mtl").open("w", encoding="utf-8") as file:
        file.write("# Authored palette for Boswell's fixture masters\n")
        for name, (r, g, b) in COLORS.items():
            file.write(f"newmtl {name}\nKd {r} {g} {b}\nKa 0 0 0\nKs 0.08 0.08 0.08\nNs 12\n\n")
    clothing_run()
    four_way()
    round_rack()


if __name__ == "__main__":
    main()

"""Generate the Juniors clothing and display forms in centimeters.

Photo references 02 and 09 inform the color-grouped stock, shoulder shapes
and dressed figures. The source models are authored here; no photo pixels
or retail branding are included.
"""

from math import cos, pi, sin
from pathlib import Path
import shutil

import make_fixture_kit as kit


OUTPUT = Path(__file__).resolve().parents[3] / "Saved" / "JuniorsMeshes"


def loft(mesh, profile, material, sides=20, drape=0):
    """Closed cloth or display-form shell along a near-vertical centerline."""
    rings = []
    for level, (x, y, z, width, depth) in enumerate(profile):
        ring = []
        for side in range(sides):
            angle = side * 2 * pi / sides
            fold = 1 + drape * (cos(5 * angle + level * .58) + .4 * sin(3 * angle))
            ring.append((x + width * cos(angle), y + depth * sin(angle) * fold, z))
        rings.append(ring)
    mesh.face(list(reversed(rings[0])), material)
    for lower, upper in zip(rings, rings[1:]):
        for side in range(sides):
            nxt = (side + 1) % sides
            mesh.face((lower[side], lower[nxt], upper[nxt], upper[side]), material)
    mesh.face(rings[-1], material)


def shell(mesh, x, y, z, rx, ry, rz, material):
    loft(mesh, [(x, y, z + rz * sin(t), rx * cos(t), ry * cos(t))
                for t in (step * pi / 16 for step in range(-7, 8))], material, 24)


def seam(mesh, points, material, half_width=.24):
    for (x0, y0, z0), (x1, y1, z1) in zip(points, points[1:]):
        points = ((x0 - half_width, y0, z0), (x1 - half_width, y1, z1),
                  (x1 + half_width, y1, z1), (x0 + half_width, y0, z0))
        mesh.face(points if z1 < z0 else tuple(reversed(points)), material)


def top(mesh, z, color, trim, kind, display=False):
    """Rounded body, two sleeve shells, and collar with local front at -Y."""
    long_sleeve = kind != "Tee"
    length = 64 if display else 72
    depth = 12 if display else 7.5
    hem = z - length
    loft(mesh, [(0, 0, hem, 24, depth * .86),
                (-1, 0, hem + 3, 24.4, depth),
                (1, 0, hem + 15, 23, depth),
                (0, 0, z - 29, 22.5 if kind == "Cardigan" else 24, depth * 1.14),
                (0, 0, z - 11, 24, depth),
                (0, 0, z - 5, 20, depth * .88),
                (0, 0, z, 10, depth * .75)], color, drape=.13)
    for side in (-1, 1):
        end = z - (57 if long_sleeve else 27)
        tip = side * (34 if long_sleeve else 32)
        loft(mesh, [(tip, 0, end, 5.8, 5),
                    (tip, 0, end + 2, 6.1, 5.4),
                    (side * 31, 0, z - 21, 9.1, 7),
                    (side * 23, 0, z - 8, 8.8, 7.4),
                    (side * 20, 0, z - 5, 5, 5)], color, drape=.16)
        if long_sleeve:
            loft(mesh, [(tip, 0, end - 1, 5.9, 5.3),
                        (tip, 0, end + 3, 6, 5.5)], trim)
    loft(mesh, [(0, 0, z - 3, 10.5, depth * .79),
                (0, 0, z + 2, 8.7, depth * .71)], trim)
    front = -depth * 1.18
    if kind != "Tee":
        seam(mesh, [(0, front, hem + 4), (.6, front - 1.4, z - 25),
                    (0, -depth * .9, z - 4)], trim, .65)
        for drop in (16, 27, 38, 49, 59):
            shell(mesh, .8, front - 1.7, z - drop, 1, .48, 1, "brass")
        seam(mesh, [(10, front, z - 39), (16, front + .7, z - 34)], trim, .68)
    else:
        seam(mesh, [(-17, front + 1.2, z - 34), (0, front - .1, z - 35),
                    (17, front + 1.2, z - 33)], trim, 1.3)


def hanger(mesh):
    # A narrow wooden triangular hanger and a curved wire hook.
    for side in (-1, 1):
        points = [(0, -1.5, 1), (side * 20, -1.5, -5),
                  (side * 20, -1.5, -7), (0, -1.5, -1)]
        mesh.face(points if side > 0 else list(reversed(points)), "dark_wood")
        mesh.face(list(reversed(points)) if side > 0 else points, "dark_wood")
    curve = [(1.4 + 3.5 * cos(i * pi / 12), 0, 4 + 3.5 * sin(i * pi / 12))
             for i in range(15)]
    seam(mesh, curve, "steel", .55)


def folded():
    mesh = kit.Mesh()
    loft(mesh, [(0, 0, -2.7, 16, 23), (0, 0, -1.8, 17.5, 24),
                (.5, -.3, 1.1, 18, 24.5), (1, .4, 2.8, 16.5, 23)], "rose", drape=.09)
    for x in (-6, 3, 10):
        seam(mesh, [(x, -23, 1.4), (x + 1.5, -18, 2.7), (x + .5, -10, 2.8)], "burgundy")
    return mesh


def mannequin(jacket):
    mesh = kit.Mesh()
    color, trim, pants = ("burgundy", "rose", "navy") if jacket else ("denim", "navy", "charcoal")
    top(mesh, 139, color, trim, "Cardigan" if jacket else "Jacket", display=True)
    loft(mesh, [(0, 0, 137, 6.6, 6.3), (0, 0, 150, 6, 6),
                (0, -.1, 153, 6.7, 6.3)], "cream", 24)
    loft(mesh, [(0, -1, 149, 4, 5), (0, -1, 153, 6.2, 6.4),
                (0, 0, 157, 7.8, 7.5), (0, 0, 166, 8.1, 8.1),
                (0, 1, 173, 6.8, 7.3), (0, 1, 176, 3.6, 4.3),
                (0, 1, 177, .9, 1.1)], "cream", 28)
    shell(mesh, 0, -7.1, 161, 1.65, 2.15, 2.5, "cream")
    for side in (-1, 1):
        shell(mesh, side * 7.6, .8, 160, 1.25, 1.8, 2.7, "cream")
        shell(mesh, side * 34, -.5, 77, 4.2, 3.8, 8.6, "cream")
        foot_x = side * (11 if side < 0 else 14)
        foot_y = -4 if side < 0 else 5
        loft(mesh, [(foot_x, foot_y, 7, 6.5, 6.9),
                    (foot_x - side, foot_y + 1, 22, 7.1, 7.4),
                    (side * 12, -1, 46, 8.4, 8.3),
                    (side * 11, 1, 64, 9.8, 8.4),
                    (side * 9, 0, 77, 9.2, 6.5),
                    (side * 8, 0, 83, 8, 5.5)], pants, drape=.10)
        loft(mesh, [(foot_x, foot_y - 4, 0, 7.6, 13),
                    (foot_x, foot_y - 4, 2.8, 7.8, 13.1)], "oat")
        loft(mesh, [(foot_x, foot_y - 4, 2.7, 7.5, 12.8),
                    (foot_x, foot_y - 2, 6, 7.1, 11.4),
                    (foot_x, foot_y + 1, 10, 6.4, 7)], "dark_wood")
    return mesh


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(kit.ROOT / "BoswellsFixtures.mtl", OUTPUT / "BoswellsFixtures.mtl")
    kit.ROOT = OUTPUT
    for kind in ("Jacket", "Cardigan", "Tee"):
        mesh = kit.Mesh()
        top(mesh, 0, "rose", "burgundy", kind)
        hanger(mesh)
        mesh.write("SM_BSW_Juniors_" + kind)
    folded().write("SM_BSW_Juniors_Folded")
    mannequin(True).write("SM_BSW_Juniors_DisplayA")
    mannequin(False).write("SM_BSW_Juniors_DisplayB")


if __name__ == "__main__":
    main()

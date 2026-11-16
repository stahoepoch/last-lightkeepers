"""Make four compact hanging-stock meshes for Boswell's apparel wings.

Dimensions are centimeters. The existing 45 cm wide collision proxies stay
authoritative in the map; these meshes are visual only. A long wing is 600 cm
and a short wing is 150 cm. Re-run to reproduce the OBJ sources exactly.
"""

from make_fixture_kit import Mesh


COLORS = ("navy", "denim", "oat", "burgundy", "pine", "cream")


def garment(mesh, y, height, color, variation):
    top = 162.0
    hem = top - height
    shoulder = 22 + variation % 3
    body = 23 + variation % 2
    sleeve = 32 + variation % 4
    x = (-5, 0, 5)[variation % 3]
    front = [
        (-8, top), (-shoulder, top - 4), (-sleeve, top - 24),
        (-sleeve + 3, top - 45), (-body - 1, top - 43),
        (-body, hem + 5), (-body + 3, hem),
        (body - 3, hem + (variation % 3) * 2), (body, hem + 5),
        (body + 1, top - 43), (sleeve - 3, top - 45),
        (sleeve, top - 24), (shoulder, top - 4), (8, top),
    ]
    # The cloth has real thickness and an uneven hem. Front, back and all
    # perimeter faces share positions, so a grazing aisle light catches folds
    # instead of illuminating a flat rectangle.
    back = [(x - 3.0, y + oy, z) for oy, z in front]
    face = [(x + 3.0, y + oy, z) for oy, z in front]
    mesh.face(list(reversed(back)), color)
    mesh.face(face, color)
    for i in range(len(face)):
        j = (i + 1) % len(face)
        mesh.face([back[i], back[j], face[j], face[i]], color)
    # Hanger, open collar, seam, pocket and cuffs supply retail-scale detail.
    mesh.box(x, y, top + 3, 7, 35, 2, "dark_wood")
    mesh.box(x, y, top + 4, 4, 4, 8, "steel")
    mesh.box(x + 3.6, y, top - 24, 1, 2, 29, "charcoal")
    for side in (-1, 1):
        mesh.box(x + 3.7, y + side * 13, top - 48, 1, 12, 2,
                 "dark_wood")
        mesh.box(x + 3.7, y + side * (sleeve - 2), top - 44, 1, 8, 2,
                 "dark_wood")


def wing(long, variant):
    mesh = Mesh()
    length = 600 if long else 150
    count = 8 if long else 2
    step = length / count
    mesh.box(0, 0, 9, 44, length - 4, 18, "charcoal")
    mesh.box(0, 0, 166, 7, length - 10, 5, "steel")
    for end in (-1, 1):
        mesh.box(0, end * (length / 2 - 5), 85, 8, 8, 170, "steel")
    for i in range(count):
        y = -length / 2 + step * (i + 0.5)
        height = (94, 112, 103, 116)[(i + variant) % 4]
        color = COLORS[(i * 2 + variant * 3) % len(COLORS)]
        garment(mesh, y, height, color, i + variant * 3)
    stem = f"SM_BSW_StockWing_{'Long' if long else 'Short'}_{'A' if variant == 0 else 'B'}"
    mesh.write(stem)


if __name__ == "__main__":
    for is_long in (False, True):
        for variant in (0, 1):
            wing(is_long, variant)

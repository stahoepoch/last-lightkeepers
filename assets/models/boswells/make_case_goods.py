"""Model stocked perfume and jewelry displays for the Boswell's entry cases.

One local-space mesh replaces three visual cubes per case. The existing
plinth and glazed case retain their collision and silhouette. Dimensions are
centimeters; all stock fits inside a 186 x 90 x 100 cm case.
"""

from make_fixture_kit import Mesh


def perfume(variant):
    m = Mesh()
    colors = ("rose", "mint", "lavender") if variant == "A" else ("butter", "sky", "burgundy")
    for i, x in enumerate((-52, 0, 52)):
        depth = (-5, 6, -3)[i] if variant == "A" else (4, -6, 3)[i]
        body_height = (20, 25, 18)[i] if variant == "A" else (25, 18, 22)[i]
        width = (17, 13, 20)[i]
        m.box(x, depth, 61, 31, 25, 2, "dark_wood")
        m.box(x, depth, 62 + body_height / 2, width, 12, body_height, colors[i])
        m.box(x, depth - 6.25, 71, width - 4, 0.5, 7, "cream")
        m.box(x, depth, 62 + body_height + 2, 7, 7, 4, "brass")
        m.box(x, depth, 62 + body_height + 6.5, 10, 10, 5, "charcoal")
        # A low display riser and price-card lip; the face is tilted away
        # from ceiling light and carries no legible product text.
        m.box(x, depth - 15, 63, 33, 2, 4, "brass")
    return m


def jewelry(variant):
    m = Mesh()
    colors = ("rose", "mint", "butter") if variant == "A" else ("lavender", "sky", "oat")
    for i, x in enumerate((-52, 0, 52)):
        y = (3, -4, 5)[i] if variant == "A" else (-5, 4, -3)[i]
        m.box(x, y, 63, 36, 27, 5, "dark_wood")
        m.box(x, y, 66, 31, 22, 1, colors[i])
        if i == 1:
            # A shallow necklace bust, not a generic bright cube.
            m.box(x, y + 2, 78, 19, 7, 23, "charcoal")
            m.box(x, y + 2, 91, 11, 6, 5, "charcoal")
            for dx, dz in ((-8, 82), (-5, 77), (0, 74), (5, 77), (8, 82)):
                m.cylinder(x + dx, y - 2, dz, dz + 1.5, 1.4, "brass", 8)
        else:
            for dx in (-9, 9):
                m.box(x + dx, y, 70, 8, 8, 7, "oat")
                m.cylinder(x + dx, y, 74, 76, 4, "brass", 10)
                m.cylinder(x + dx, y, 76, 78, 2, colors[i], 8)
        m.box(x, y - 15, 64, 35, 2, 3, "brass")
    return m


if __name__ == "__main__":
    for kind, maker in (("Perfume", perfume), ("Jewelry", jewelry)):
        for variant in ("A", "B"):
            maker(variant).write(f"SM_BSW_CaseGoods_{kind}_{variant}")

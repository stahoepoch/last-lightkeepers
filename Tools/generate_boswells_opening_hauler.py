"""Author a low floor rover silhouette for the witnessed opening.

Centimetres in a local OBJ. It is a single mesh so the runtime haul controller
can move and hide exactly one actor. This is blockout art, not the live Sweeper.
"""

from math import cos, pi, sin
from pathlib import Path

out = Path(__file__).resolve().parents[1] / "assets/models/boswells/SM_BSW_OpeningHauler.obj"
out.parent.mkdir(parents=True, exist_ok=True)
vertices = []
faces = []


def box(x0, x1, y0, y1, z0, z1):
    first = len(vertices) + 1
    vertices.extend(((x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0),
                     (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)))
    for ids in ((1, 4, 3, 2), (5, 6, 7, 8), (1, 2, 6, 5),
                (2, 3, 7, 6), (3, 4, 8, 7), (4, 1, 5, 8)):
        faces.append(tuple(first + i - 1 for i in ids))


def wheel(x0, x1, y, z, radius=20, sides=12):
    first = len(vertices) + 1
    for x in (x0, x1):
        for n in range(sides):
            a = 2 * pi * n / sides
            vertices.append((x, y + radius * cos(a), z + radius * sin(a)))
    faces.append(tuple(first + n for n in reversed(range(sides))))
    faces.append(tuple(first + sides + n for n in range(sides)))
    for n in range(sides):
        nxt = (n + 1) % sides
        faces.append((first + n, first + nxt,
                      first + sides + nxt, first + sides + n))


# Wheels, sloped-front hood, waist-height haul frame and sensor mast make a
# purposeful machine silhouette without an oversized, blinding beacon.
box(-42, 42, -61, 56, 21, 67)
box(-38, 38, -71, -57, 27, 52)
box(-32, 32, -47, 26, 66, 80)
box(-8, 8, -44, -29, 78, 119)
box(-27, 27, -52, -32, 112, 125)
box(-47, 47, 44, 53, 65, 74)
box(-47, -39, 44, 82, 65, 118)
box(39, 47, 44, 82, 65, 118)
for y in (-40, 32):
    wheel(-55, -40, y, 24)
    wheel(40, 55, y, 24)

with out.open("w", encoding="utf-8") as stream:
    stream.write("# Opening haul machine, centimetres, front toward -Y\n")
    stream.write("o SM_BSW_OpeningHauler\n")
    for x, y, z in vertices:
        stream.write(f"v {x:.4f} {y:.4f} {z:.4f}\n")
    stream.write("vt 0.0 0.0\n")
    for face in faces:
        stream.write("f " + " ".join(f"{i}/1" for i in face) + "\n")
print(out, len(vertices), "vertices", len(faces), "faces")

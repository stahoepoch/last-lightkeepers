"""A ladder of deliberately boring meshes, to find what the cover fixtures
cross that the counter does not.

WHY THIS EXISTS
---------------
Two of the three Boswell's cover fixtures are missed by a Visibility trace
everywhere, and the third blocks everywhere. Eliminated by direct measurement
so far: position, height, trace geometry, trace length, start distance,
process type as a sole cause, `set_collision_profile_name`, box count, box
extents, collision-enabled, profile, re-saving from a full editor, Nanite,
triangle count, winding, manifoldness and degenerate faces.

What survives all of that is **complexity**, and the three fixtures happen to
sit on a ladder:

    SM_BSW_ServiceCounter      4 boxes,  4 materials   BLOCKS
    SM_BSW_FoldedGoodsTable    7 boxes,  7 materials   misses
    SM_BSW_WallMerchBay       42 boxes,  9 materials   misses

Three points is not enough to say whether it is the piece count, the material
count, or neither - they move together in all three fixtures, which is
exactly the confound that makes them useless as evidence. So this emits
meshes that vary ONE of them at a time.

HOW TO USE IT
-------------
Generate, import the probe meshes the same way the fixtures are imported, and
trace each one across its short axis at 40 cm. Then read the two rows:

  - if the PIECES ladder fails above some N and the MATERIALS ladder does not,
    it is the piece count
  - if the MATERIALS ladder fails and PIECES does not, it is the section count
  - if both fail at the same total, it is probably neither and the real
    variable is something they both drag along, which is worth knowing too
  - if neither fails, the fixtures differ by something not on this ladder and
    this file has done its job by removing two more candidates

Every probe is the same outer size, 160 x 90 x 80 cm, sitting on z = 0, built
from the same welded primitive as the real kit. The only thing that changes
along each ladder is the one variable in its name.

This is a diagnostic, not a shipping asset. Nothing here goes in a map.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SIZE = (160.0, 90.0, 80.0)

# Reuses the real kit's palette so the materials resolve on import rather
# than arriving as missing references and adding a variable of their own.
PALETTE = ["oat", "cream", "dark_wood", "white_steel", "steel", "charcoal",
           "denim", "burgundy", "mint", "rose", "sky", "butter"]


def box(cx, cy, cz, sx, sy, sz):
    corners = []
    for z in (cz - sz / 2, cz + sz / 2):
        for y in (cy - sy / 2, cy + sy / 2):
            for x in (cx - sx / 2, cx + sx / 2):
                corners.append((x, y, z))
    faces = ((0, 2, 3, 1), (4, 5, 7, 6), (0, 1, 5, 4),
             (2, 6, 7, 3), (0, 4, 6, 2), (1, 3, 7, 5))
    return corners, faces


def write(stem, pieces, materials):
    """`pieces` boxes stacked inside the same envelope, cycling through
    `materials` palette entries. Welded positions and flat normals, same as
    the shipping kit - the point is to vary one thing, not to reintroduce a
    defect that was just removed."""
    verts, index, normals, normal_index, out = [], {}, [], {}, []

    def vid(p):
        key = tuple(round(c, 4) for c in p)
        if key not in index:
            verts.append(key)
            index[key] = len(verts)
        return index[key]

    def nid(n):
        key = tuple(round(c, 4) for c in n)
        if key not in normal_index:
            normals.append(key)
            normal_index[key] = len(normals)
        return normal_index[key]

    # NESTED, not stacked. The first version stacked the pieces face to face,
    # which put the top corners of one box at the same positions as the
    # bottom corners of the next; welding then merged them and every edge on
    # that plane was shared by four faces. That makes the probe non-manifold,
    # which is a second variable, in a file whose entire purpose is to vary
    # one. Caught by self-checking the probes before using them - 8 of 9 came
    # back open or non-manifold.
    #
    # Nesting keeps every piece a separate closed solid with its own
    # vertices, keeps the outer envelope and the z = 0 base exactly the same
    # across the whole ladder, and keeps all pieces spanning the 40 cm trace
    # height so the trace meets the outermost one in every case.
    for i in range(pieces):
        corners, faces = box(0.0, 0.0, SIZE[2] / 2,
                             SIZE[0] - i, SIZE[1] - i, SIZE[2] - i)
        material = materials[i % len(materials)]
        for ids in faces:
            pts = [corners[j] for j in ids]
            (ax, ay, az), (bx, by, bz), (cx, cy, cz) = pts[0], pts[1], pts[2]
            ux, uy, uz = bx - ax, by - ay, bz - az
            vx, vy, vz = cx - ax, cy - ay, cz - az
            nx, ny, nz = (uy * vz - uz * vy, uz * vx - ux * vz,
                          ux * vy - uy * vx)
            length = (nx * nx + ny * ny + nz * nz) ** 0.5 or 1.0
            n = nid((nx / length, ny / length, nz / length))
            out.append((material, [(vid(p), n) for p in pts]))

    lines = [f"# collision probe: {pieces} pieces, {len(set(materials))} "
             f"materials, centimeters, Z up",
             "mtllib BoswellsFixtures.mtl", "o " + stem]
    lines += [f"v {x:.4f} {y:.4f} {z:.4f}" for x, y, z in verts]
    lines.append("vt 0.0000 0.0000")
    lines += [f"vn {x:.4f} {y:.4f} {z:.4f}" for x, y, z in normals]
    last = None
    for material, corners in sorted(out, key=lambda f: f[0]):
        if material != last:
            lines.append("usemtl " + material)
            last = material
        lines.append("f " + " ".join(f"{v}/1/{n}" for v, n in corners))
    (ROOT / (stem + ".obj")).write_text("\n".join(lines) + "\n",
                                        encoding="utf-8")
    return stem, pieces, len(set(materials))


def main():
    rows = []
    # Ladder A: piece count varies, material count pinned at 1.
    for pieces in (1, 4, 7, 12, 42):
        rows.append(write(f"SM_BSW_PROBE_P{pieces:02d}_M01", pieces,
                          PALETTE[:1]))
    # Ladder B: material count varies, piece count pinned at 12 so there are
    # always enough pieces to carry the materials.
    for mats in (1, 4, 7, 9, 12):
        rows.append(write(f"SM_BSW_PROBE_P12_M{mats:02d}", 12,
                          PALETTE[:mats]))

    print(f"{'mesh':34} {'pieces':>7} {'materials':>10}")
    for stem, pieces, mats in rows:
        print(f"{stem:34} {pieces:7} {mats:10}")
    print("\nImport these, then trace each across its short axis at 40 cm.")
    print("The counter blocks at 4 pieces / 4 materials; the table misses at")
    print("7 / 7. If a ladder has a step where BLOCK becomes clear, that is")
    print("the variable. If neither does, it is neither, which is also an")
    print("answer and removes two candidates.")


if __name__ == "__main__":
    main()

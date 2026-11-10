"""Print every actor's label, class and transform from a .umap, with no editor.

    py Tools/read_map_transforms.py Content/LLK_Boswells_Anchor.umap
    py Tools/read_map_transforms.py Content/LLK_Boswells_Anchor.umap --csv

This is how `docs/PLAN-AUDIT-BOSWELLS-ANCHOR.md` was written. It exists so the
audit is reproducible by anyone, on any checkout, without opening Unreal and
without the MCP bridge being up.

**What it reads, and the limit that matters.** A saved level carries one
descriptor per actor in its asset-registry payload: a semicolon-separated
list of `path,class,base64,None` records, where the base64 holds the actor's
guid, its class, a transform and its editor label and folder. This decodes
those. It is therefore:

- **an index, not the actors.** The authoritative actor data is in the
  package's export table. These records are written from it on save, so they
  agree with it on a map saved by the editor - but a map edited by anything
  that does not refresh them would read stale here and be right in game.
- **not collision, and not bounds.** `FINDINGS-BOSWELLS-GEOMETRY.md` already
  records that bounds are not collision; authored transforms are a third
  thing again, and all three can disagree. Heights printed below are
  `scale * 100`, which is only a size because every box in these maps is
  `/Engine/BasicShapes/Cube`, a 100 cm cube with its origin at its centre.
  **Trace for anything that has to be true.**

Non-OFPA maps only. A World Partition map keeps its actors in
`Content/__ExternalActors__/<map>/`, one package each, and this would read
the descriptors while every actor lived somewhere else.
"""

import base64
import csv
import math
import re
import struct
import sys

RECORD = re.compile(
    r"PersistentLevel\.([A-Za-z0-9_\-]+),/Script/([A-Za-z0-9_.]+),"
    r"([A-Za-z0-9+/=]+)")
LABEL = re.compile(rb"[A-Z][A-Z0-9_]{5,}")


def read(path):
    with open(path, "rb") as handle:
        data = handle.read()

    rows = []
    seen = set()
    for run in re.findall(rb"[ -~]{200,}", data):
        for match in RECORD.finditer(run.decode()):
            name, cls, blob = match.groups()
            if name in seen:
                continue
            try:
                raw = base64.b64decode(blob)
            except Exception:
                continue
            short = cls.split(".")[-1]
            # The class name is stored as a length-prefixed string; the guid
            # follows it, then the transform as ten doubles: the quaternion
            # (x, y, z, w), the translation, and the 3D scale.
            at = raw.find(short.encode() + b"\x00")
            if at < 0:
                continue
            start = at + len(short) + 1 + 16
            if start + 80 > len(raw):
                continue
            qx, qy, qz, qw, tx, ty, tz, sx, sy, sz = struct.unpack_from(
                "<10d", raw, start)
            found = LABEL.findall(raw)
            label = found[0].decode() if found else name
            seen.add(name)
            rows.append({
                "label": label,
                "name": name,
                "class": short,
                "x": tx, "y": ty, "z": tz,
                "sx": sx, "sy": sy, "sz": sz,
                "roll": math.degrees(math.atan2(
                    2.0 * (qw * qx + qy * qz),
                    1.0 - 2.0 * (qx * qx + qy * qy))),
                "pitch": math.degrees(math.asin(
                    max(-1.0, min(1.0, 2.0 * (qw * qy - qz * qx))))),
                "yaw": math.degrees(math.atan2(
                    2.0 * (qw * qz + qx * qy),
                    1.0 - 2.0 * (qy * qy + qz * qz))),
            })
    rows.sort(key=lambda row: row["label"])
    return rows


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    path = argv[1]
    rows = read(path)
    if not rows:
        print("no actor descriptors found in {0}. If this is a World "
              "Partition map, its actors are in Content/__ExternalActors__/ "
              "and this tool cannot read them.".format(path))
        return 1

    if "--csv" in argv:
        out = csv.DictWriter(sys.stdout, fieldnames=list(rows[0].keys()),
                            lineterminator="\n")
        out.writeheader()
        out.writerows(rows)
        return 0

    print("{0:34s} {1:20s} {2:>8s} {3:>8s} {4:>8s} {5:>7s} {6:>7s} {7:>7s} "
          "{8:>7s} {9:>7s}".format(
              "label", "class", "X", "Y", "Z", "sclX", "sclY", "sclZ",
              "pitch", "yaw"))
    for row in rows:
        print("{label:34s} {class:20s} {x:8.1f} {y:8.1f} {z:8.1f} "
              "{sx:7.2f} {sy:7.2f} {sz:7.2f} {pitch:7.1f} "
              "{yaw:7.1f}".format(**row))
    print("{0} actors in {1}".format(len(rows), path))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

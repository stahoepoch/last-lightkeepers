"""Generate one subtle 115 x 120 cm suspended-ceiling tile for the lower wings."""

from pathlib import Path

import numpy as np
from PIL import Image


OUTPUT = (Path(__file__).resolve().parents[1] /
          "assets/generated/boswells/T_BSW_LowerCeilingTile.png")
SIZE = 256
rng = np.random.default_rng(915160)
noise = rng.normal(0, 1.7, (SIZE, SIZE, 1))
base = np.array([177, 176, 169], dtype=np.float32)
tile = np.broadcast_to(base, (SIZE, SIZE, 3)).copy() + noise

# The repeat edge is the recessed T-bar. The bevel gives it a little shape
# without baking a lamp pool or a direction of light into the texture.
tile[:3, :, :] = [65, 69, 68]
tile[:, :3, :] = [65, 69, 68]
tile[3:5, 5:, :] += 5
tile[5:, 3:5, :] += 5

OUTPUT.parent.mkdir(parents=True, exist_ok=True)
Image.fromarray(np.clip(tile, 0, 255).astype(np.uint8), "RGB").save(
    OUTPUT, optimize=True)
print("BOSWELLS_CEILING_TILE generated {} ({} bytes)".format(
    OUTPUT, OUTPUT.stat().st_size))

"""Generate the project's own subdued 60 cm store tile texture.

This is a four-by-four seamless repeat: 1024 pixels represent 240 cm. It is
not a photograph or a licensed texture. The edge grout hides each repeat;
variation is restrained so emergency pools and shadows remain legible.
"""

from pathlib import Path

import numpy as np
from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "assets/generated/boswells/T_BSW_StoneTile_4x4.png"
SIZE = 1024
TILE = 256
rng = np.random.default_rng(841711)


def smooth_noise(size, amplitude):
    pixels = rng.integers(0, 256, (size, size), dtype=np.uint8)
    small = Image.fromarray(pixels, mode="L")
    field = np.asarray(small.resize((SIZE, SIZE), Image.Resampling.BICUBIC),
                       dtype=np.float32)
    return (field - 127.5) * amplitude / 127.5


large = smooth_noise(18, 5.5)
medium = smooth_noise(76, 3.0)
fine = smooth_noise(300, 1.3)
stone = large + medium + fine
tile_variation = rng.integers(-7, 8, (4, 4), dtype=np.int16)
y, x = np.indices((SIZE, SIZE))
per_tile = tile_variation[y // TILE, x // TILE]
base = np.array([151, 148, 139], dtype=np.float32)
image = base + (stone + per_tile)[..., None]
image[..., 0] += 2.0 * smooth_noise(22, 1.0)
image[..., 2] -= 1.5 * smooth_noise(31, 1.0)

# About 1 cm of dark grout, including the repeat edge. A faint bevel makes
# the grid read as real tile under the 8 cd emergency pools without a loud
# black outline or a texture that supplies fake light.
local_x = x % TILE
local_y = y % TILE
grout = (local_x < 4) | (local_y < 4)
bevel = (local_x == 4) | (local_y == 4)
image[grout] = np.array([72, 75, 72], dtype=np.float32)
image[bevel] = image[bevel] + 7

OUTPUT.parent.mkdir(parents=True, exist_ok=True)
Image.fromarray(np.clip(image, 0, 255).astype(np.uint8), mode="RGB").save(
    OUTPUT, optimize=True)
print("BOSWELLS_TILE generated {} ({} bytes)".format(OUTPUT, OUTPUT.stat().st_size))

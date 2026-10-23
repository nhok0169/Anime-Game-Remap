"""Builds ChangliTextureBlindfoldOnNymph.dds, the Laurel Nymph's blindfold diffuse in Changli's own black.

The Laurel Nymph draws its own blindfold (vb0 646aa163, diffuse d3563476 at ps-t1): white lace with a gold
laurel. Changli's base outfit draws a different mesh (vb0 baaa6262) in plain dark cloth, whose UVs do not
match, so its texture cannot simply be bound on the skin's. This repaints the skin's own texture: every
unsaturated (lace) texel takes the base cloth's colour, its luminance kept as a little texture; the gold
laurel is left alone, and the lace is made opaque inside its outline.

    py -3 changliNymphBlindfold.py <skin diffuse .dds> <base diffuse .dds> <out .dds>

The two inputs are frame-dump textures (BC7_UNORM_SRGB); the output is re-encoded in the skin's format, mip
chain included.
"""

import os
import shutil
import sys

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "..",
                                "Anime Game Remap (for all users)", "api", "src", "py"))
from FixRaidenBoss2 import CppTextureFile  # noqa: E402

# Below this saturation a texel is lace, not the gold laurel
LACE_SATURATION = 0.2
# The lace's own luminance survives as a faint weave: shade + texture * luminance, times the base cloth
LACE_SHADE = 0.35
LACE_TEXTURE = 0.25
# Lace texels at least this opaque become fully opaque
LACE_SOLID_FROM = 128


def pixelsOf(path: str) -> np.ndarray:
    tex = CppTextureFile(path)
    tex.open()
    assert tex.hasImage, path
    return np.frombuffer(tex.getPixels(), dtype=np.uint8).reshape(tex.height, tex.width, 4)


def main(skinPath: str, basePath: str, outPath: str):
    base = pixelsOf(basePath).astype(float)
    opaque = base[..., 3] > 0
    cloth = base[..., :3][opaque & (base[..., :3].max(axis=2) < 200)].mean(axis=0)
    print("base cloth colour", cloth.round(1))

    shutil.copyfile(skinPath, outPath)
    tex = CppTextureFile(outPath)
    tex.open()
    skin = np.frombuffer(tex.getPixels(), dtype=np.uint8).reshape(tex.height, tex.width, 4).astype(float)

    rgb = skin[..., :3]
    hi = rgb.max(axis=2)
    sat = (hi - rgb.min(axis=2)) / np.maximum(hi, 1)
    lace = sat < LACE_SATURATION
    lum = rgb.mean(axis=2, keepdims=True) / 255.0

    out = skin.copy()
    out[..., :3] = np.where(lace[..., None], cloth * (LACE_SHADE + LACE_TEXTURE * lum), rgb)
    # the lace is see-through (alpha ~150-200 over the eyes) where her cloth is solid: inside the band's own
    # outline it is made opaque, the outline itself (the low-alpha fringe) left as it is
    solid = lace & (skin[..., 3] >= LACE_SOLID_FROM)
    out[..., 3] = np.where(solid, 255, skin[..., 3])
    print(f"lace texels {lace.mean():.3f}")

    # opened as BC7_UNORM_SRGB, so the library has set its sRGB gamma: the file is written as BC7_UNORM,
    # pre-corrected, which the game samples to the same colour
    print("gamma", tex.gamma)
    tex.setPixels(out.clip(0, 255).astype(np.uint8).tobytes(), tex.width, tex.height)
    tex.save(True, True)

    check = pixelsOf(outPath).astype(float)
    lin = 255.0 * (out[..., :3] / 255.0) ** (1 / 2.2)
    print("round trip vs pre-corrected: mean error", np.abs(check[..., :3] - lin).mean().round(2))


if __name__ == "__main__":
    main(*sys.argv[1:4])

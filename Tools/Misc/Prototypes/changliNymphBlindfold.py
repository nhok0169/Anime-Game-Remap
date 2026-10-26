"""Builds the two blindfold downloads of Changli <-> ChangliNymph, each outfit's blindfold in the OTHER's colours.

Changli's base outfit draws her blindfold as its own mesh (vb0 baaa6262, diffuse b53e6c59 at ps-t1): plain
black cloth. The Laurel Nymph draws another (vb0 646aa163, diffuse d3563476 at ps-t1): white lace with a gold
laurel. The two meshes' UVs differ, so neither texture can be bound on the other's mesh; each fix repaints
the TARGET's own texture instead.

    py -3 changliNymphBlindfold.py forward <skin diffuse .dds> <base diffuse .dds> <out .dds>
    py -3 changliNymphBlindfold.py reverse <skin diffuse .dds> <base diffuse .dds> <out .dds>

forward  (Changli -> ChangliNymph, ChangliTextureBlindfoldOnNymph.dds): the skin's texture, every unsaturated
         (lace) texel in the base cloth's colour with its luminance kept as a faint weave, the gold laurel
         left alone, and the lace made opaque inside its outline.
reverse  (ChangliNymph -> Changli, ChangliNymphTextureBlindfoldOnChangli.dds): Changli's texture, every texel
         in the skin's lace colour with its folds kept; alpha is not opacity on her shader (it tops out at
         127) and is left as it is.

The two inputs are frame-dump textures (BC7_UNORM_SRGB); the output is re-encoded in the repainted texture's
format, mip chain included.
"""

import os
import shutil
import sys

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "..",
                                "Anime Game Remap (for all users)", "api", "src", "py"))
from FixRaidenBoss2 import CppTextureFile  # noqa: E402

# Below this saturation a skin texel is lace, not the gold laurel
LACE_SATURATION = 0.2
# The lace's own luminance survives as a faint weave: shade + texture * luminance, times the base cloth
LACE_SHADE = 0.35
LACE_TEXTURE = 0.25
# Lace texels at least this opaque become fully opaque
LACE_SOLID_FROM = 128
# Her cloth's folds survive in white: shade + texture * (luminance / its 95th percentile), times the lace
CLOTH_SHADE = 0.75
CLOTH_TEXTURE = 0.25


def pixelsOf(path: str) -> np.ndarray:
    tex = CppTextureFile(path)
    tex.open()
    assert tex.hasImage, path
    return np.frombuffer(tex.getPixels(), dtype=np.uint8).reshape(tex.height, tex.width, 4)


def laceOf(pixels: np.ndarray) -> np.ndarray:
    rgb = pixels[..., :3]
    hi = rgb.max(axis=2)
    return (hi - rgb.min(axis=2)) / np.maximum(hi, 1) < LACE_SATURATION


def forward(skin: np.ndarray, base: np.ndarray) -> np.ndarray:
    cloth = base[..., :3][(base[..., 3] > 0) & (base[..., :3].max(axis=2) < 200)].mean(axis=0)
    print("base cloth colour", cloth.round(1))

    rgb = skin[..., :3]
    lace = laceOf(skin)
    lum = rgb.mean(axis=2, keepdims=True) / 255.0

    out = skin.copy()
    out[..., :3] = np.where(lace[..., None], cloth * (LACE_SHADE + LACE_TEXTURE * lum), rgb)
    # the lace is see-through (alpha ~150-200 over the eyes) where her cloth is solid: inside the band's own
    # outline it is made opaque, the outline itself (the low-alpha fringe) left as it is
    solid = lace & (skin[..., 3] >= LACE_SOLID_FROM)
    out[..., 3] = np.where(solid, 255, skin[..., 3])
    print(f"lace texels {lace.mean():.3f}")
    return out


def reverse(skin: np.ndarray, base: np.ndarray) -> np.ndarray:
    lace = skin[..., :3][laceOf(skin) & (skin[..., 3] > 0)].mean(axis=0)
    print("skin lace colour", lace.round(1))

    lum = base[..., :3].mean(axis=2, keepdims=True)
    folds = lum / max(np.percentile(lum, 95), 1.0)

    out = base.copy()
    out[..., :3] = lace * (CLOTH_SHADE + CLOTH_TEXTURE * folds.clip(0, 1))
    return out


def main(mode: str, skinPath: str, basePath: str, outPath: str):
    target = skinPath if mode == "forward" else basePath
    other = pixelsOf(basePath if mode == "forward" else skinPath).astype(float)

    shutil.copyfile(target, outPath)
    tex = CppTextureFile(outPath)
    tex.open()
    own = np.frombuffer(tex.getPixels(), dtype=np.uint8).reshape(tex.height, tex.width, 4).astype(float)

    out = forward(own, other) if mode == "forward" else reverse(other, own)

    # opened as BC7_UNORM_SRGB, so the library has set its sRGB gamma: the file is written as BC7_UNORM,
    # pre-corrected, which the game samples to the same colour
    print("gamma", tex.gamma)
    tex.setPixels(out.clip(0, 255).astype(np.uint8).tobytes(), tex.width, tex.height)
    tex.save(True, True)


if __name__ == "__main__":
    main(*sys.argv[1:5])

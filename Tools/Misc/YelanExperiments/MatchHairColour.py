"""
Colour-matches the hair painted in the BODY diffuse to the hair painted in the HEAD diffuse, and
saves the result as a new body diffuse.

Why: the mod's crown hair is the Head object, drawn from Yelan's own head texture; its lower hair is
the Body object, drawn from the mod's body texture, whose hair the author painted lighter and bluer.
Yelan's shader flattens that difference, YelanTranquil's hair material does not, so the lower third
of the hair reads brighter blue with a visible boundary.

The hair pixels of each texture are found by colour (navy: blue-dominant and dark), and the body's
are moved onto the head's distribution channel by channel (mean and spread), which keeps the
strands' own variation. Alpha is left as the source has it (use the *Final* body diffuse, whose hair
alpha is already 255).

    py -3 MatchHairColour.py                                   yelanBodyDiffuseFinal.dds -> yelanBodyDiffuseHairMatched.dds
    py -3 MatchHairColour.py --src X.dds --ref yelanHeadDiffuse.dds --dest Y.dds
    py -3 MatchHairColour.py --strength 0.5                    half-way between the mod's colour and the head's
"""

import argparse
import os
import sys

import numpy as np

APISrc = r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss\Anime Game Remap (for all users)\api\src\py"
if (APISrc not in sys.path):
    sys.path.insert(0, APISrc)

Here = os.path.dirname(os.path.abspath(__file__))


def hairMask(rgb: np.ndarray) -> np.ndarray:
    r, g, b = rgb[..., 0].astype(int), rgb[..., 1].astype(int), rgb[..., 2].astype(int)
    return (b > r + 15) & (r < 90) & (g < 120) & (b > 50)


def load(FRB, path):
    tex = FRB.TextureFile(path)
    tex.open()
    if (not tex.hasImage):
        raise SystemExit(f"could not read '{path}'")
    px = np.frombuffer(tex.getPixels(), dtype = np.uint8).reshape(tex.height, tex.width, 4).copy()
    return tex, px


if (__name__ == "__main__"):
    parser = argparse.ArgumentParser(description = __doc__, formatter_class = argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--src", default = os.path.join(Here, "yelanBodyDiffuseFinal.dds"))
    parser.add_argument("--ref", default = os.path.join(Here, "yelanHeadDiffuse.dds"))
    parser.add_argument("--dest", default = os.path.join(Here, "yelanBodyDiffuseHairMatched.dds"))
    parser.add_argument("--leftHalf", action = "store_true", help = "also edit the left half of the body texture (default: the right half only, the china-dress UV)")
    parser.add_argument("--strength", type = float, default = 1.0, help = "0 = untouched, 1 = fully on the head's distribution (default: %(default)s)")
    parser.add_argument("--noCompress", action = "store_true")
    args = parser.parse_args()

    import FixRaidenBoss2 as FRB

    tex, body = load(FRB, args.src)
    _, head = load(FRB, args.ref)
    bodyMask = hairMask(body[..., :3])
    if (not args.leftHalf):
        bodyMask[:, :body.shape[1] // 2] = False
    headMask = hairMask(head[..., :3])

    src = body[..., :3][bodyMask].astype(np.float64)
    ref = head[..., :3][headMask].astype(np.float64)
    print(f"body hair pixels {len(src)}: mean {src.mean(0).round(1)} std {src.std(0).round(1)}")
    print(f"head hair pixels {len(ref)}: mean {ref.mean(0).round(1)} std {ref.std(0).round(1)}")

    matched = (src - src.mean(0)) * (ref.std(0) / np.maximum(src.std(0), 1e-6)) + ref.mean(0)
    blended = src + (matched - src) * args.strength
    out = body.copy()
    out[..., :3][bodyMask] = np.clip(np.round(blended), 0, 255).astype(np.uint8)
    print(f"body hair after: mean {out[..., :3][bodyMask].mean(0).round(1)} std {out[..., :3][bodyMask].astype(float).std(0).round(1)}")

    tex.setPixels(out.tobytes(), tex.width, tex.height)
    tex.src = args.dest
    tex.save(compress = not args.noCompress)
    print(f"wrote: {args.dest} ({os.path.getsize(args.dest)} bytes)" if (os.path.isfile(args.dest)) else f"FAILED: '{args.dest}' was not written")

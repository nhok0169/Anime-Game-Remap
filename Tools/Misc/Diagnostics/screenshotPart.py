#
# ===== screenshotPart =====
#
# Compare ONE PART of a character across several in-game screenshots -- the base against each round
# of a remap -- without the answer depending on which pixels got selected or how bright the scene
# happened to be.
#
#   python screenshotPart.py base.png fix1.png fix2.png [--sat 0.45] [--x 0.15 0.55] [--preview out.png]
#
# Two things it does that a hand-written crop does not, and both of them changed a conclusion here
# (2026-09-20, the Chisa ribbon):
#
#   1. IT SHOWS YOU THE PIXELS IT USED. `--preview` paints the selection green over a dimmed copy
#      of each shot. The first version of this measurement took every reddish pixel in the left 45%
#      of the frame, which on two of five screenshots also swallowed the character-portrait card in
#      the corner; the second took the largest connected red component, which took the ear and neck
#      on four of them and the WEAPON BLADE on the fifth. Both produced a tidy table of numbers,
#      and the table said the base had three times the remap's highlights -- the opposite of the
#      truth. A statistic over the wrong region reads exactly like one over the right region.
#
#   2. IT NORMALISES AGAINST THE SAME SHOT. Screenshots are exposures: the scene lighting, the
#      camera and the time of day all move between them. The character's HAIR (dark, desaturated)
#      and SKIN (bright, warm) are in every shot and no remap round touches them, so the part's
#      luminance is reported as a RATIO to each. A part statistic that moves while both controls
#      move with it is the scene, not the fix.
#
# Selection is by SATURATION rather than by hue alone, because skin and a saturated fabric overlap
# badly in hue and not at all in saturation (measured: skin ~0.25, the ribbon ~0.55). `--x` is the
# horizontal band the part's centroid must fall in, which is what separates it from anything else
# of the same colour elsewhere in frame.
#

import argparse
import os

import numpy as np
from PIL import Image, ImageDraw
from scipy import ndimage

Luma = np.array([0.299, 0.587, 0.114])


def part(rgb, sat, xBand, channel):
    """The largest saturated blob of the wanted channel whose centroid sits in the x band"""
    height, width, _ = rgb.shape
    high, low = rgb.max(axis = 2), rgb.min(axis = 2)
    saturation = (high - low) / np.maximum(high, 1)
    wanted = (rgb[..., channel] > 55) & (rgb[..., channel] == high) & (saturation > sat)
    joined = ndimage.binary_dilation(wanted, np.ones((9, 9), bool))    # a part's strands are one thing
    labels, count = ndimage.label(joined)
    best, bestSize = None, 0
    for index in range(1, count + 1):
        selected = wanted & (labels == index)
        size = int(selected.sum())
        if (size < 500):
            continue
        centre = np.flatnonzero(selected.any(axis = 0)).mean() / width
        if (xBand[0] < centre < xBand[1] and size > bestSize):
            best, bestSize = selected, size
    return best, saturation


def controls(rgb, saturation):
    """The hair and the skin of the same shot: dark and desaturated, bright and warm"""
    height, width, _ = rgb.shape
    lum = rgb @ Luma
    middle = np.zeros(lum.shape, bool)
    middle[int(height * 0.1):int(height * 0.85), int(width * 0.3):int(width * 0.75)] = True
    hair = middle & (saturation < 0.35) & (lum > 8) & (lum < 60)
    skin = middle & (saturation > 0.10) & (saturation < 0.34) & (lum > 120) & (rgb[..., 0] > rgb[..., 2] + 12)
    return hair, skin


def main():
    parser = argparse.ArgumentParser(description = "one part of a character, across several screenshots")
    parser.add_argument("shots", nargs = "+", help = "the screenshots, the reference one first")
    parser.add_argument("--sat", type = float, default = 0.45, help = "how saturated the part is (skin is about 0.25)")
    parser.add_argument("--x", nargs = 2, type = float, default = [0.15, 0.55], metavar = ("LO", "HI"),
                        help = "the horizontal band the part's centroid sits in, as a fraction of the width")
    parser.add_argument("--channel", choices = ["r", "g", "b"], default = "r", help = "the part's dominant channel")
    parser.add_argument("--preview", default = None, help = "write a contact sheet showing the pixels used")
    args = parser.parse_args()

    channel = {"r": 0, "g": 1, "b": 2}[args.channel]
    tiles, first = [], None
    print(f"{'':22s} {'px':>6s} {'R':>6s} {'G':>6s} {'B':>6s} {'lum':>6s} {'sat':>5s} | "
          f"{'hair':>5s} {'skin':>5s} | {'part/hair':>9s} {'part/skin':>9s} | {'vs the first':>12s}")
    for path in args.shots:
        rgb = np.asarray(Image.open(path).convert("RGB")).astype(np.float64)
        selected, saturation = part(rgb, args.sat, args.x, channel)
        if (selected is None):
            print(f"{os.path.basename(path)[:22]:22s} nothing matched -- widen --x or lower --sat")
            continue
        hair, skin = controls(rgb, saturation)
        px = rgb[selected]
        median = np.median(px, axis = 0)
        lum = float(np.median(px @ Luma))
        hairLum, skinLum = float(np.median((rgb @ Luma)[hair])), float(np.median((rgb @ Luma)[skin]))
        sat = float((median.max() - median.min()) / max(median.max(), 1))
        first = first if (first is not None) else (lum / hairLum, median)
        drift = ", ".join(f"{(median[c] - first[1][c]):+.0f}" for c in range(3))
        print(f"{os.path.basename(path)[:22]:22s} {int(selected.sum()):6d} {median[0]:6.1f} {median[1]:6.1f} "
              f"{median[2]:6.1f} {lum:6.1f} {sat:5.3f} | {hairLum:5.1f} {skinLum:5.1f} | "
              f"{lum / hairLum:9.2f} {lum / skinLum:9.3f} | {drift:>12s}")
        if (args.preview):
            shown = (rgb * 0.25).astype(np.uint8)
            shown[selected] = [0, 255, 0]
            tiles.append((Image.fromarray(shown, "RGB"), os.path.basename(path)))

    if (args.preview and tiles):
        scaled = [(t.resize((300, int(300 * t.size[1] / t.size[0]))), n) for t, n in tiles]
        height = max(t.size[1] for t, _ in scaled)
        sheet = Image.new("RGB", (len(scaled) * 304, height + 18), (20, 20, 20))
        draw = ImageDraw.Draw(sheet)
        for index, (tile, name) in enumerate(scaled):
            sheet.paste(tile, (index * 304, 0))
            draw.text((index * 304 + 2, height + 2), name[:38], fill = (255, 255, 255))
        sheet.save(args.preview)
        print(f"\nwrote {args.preview} -- LOOK AT IT before believing the table above")


if (__name__ == "__main__"):
    main()

r"""WHICH pixels differ between two screenshots -- painted, not counted.

A share tells you nothing about where, and this guide's own history is a list of statistics that
measured the weather, the leaf litter or the character-portrait card. The painted output is the
claim: if the red is on the hem, the change is on the hem, and if it is on the sky it is the scene.

    py -3 whereDiff.py a.png b.png out.png [--crop x y w h] [--threshold 18]
"""
import argparse
import pathlib
import sys

from PIL import Image

sys.stdout.reconfigure(errors="replace")

ap = argparse.ArgumentParser()
ap.add_argument("first")
ap.add_argument("second")
ap.add_argument("out")
ap.add_argument("--crop", nargs=4, type=float, default=[0.0, 0.0, 1.0, 1.0])
ap.add_argument("--threshold", type=int, default=18)
args = ap.parse_args()

a = Image.open(args.first).convert("RGB")
b = Image.open(args.second).convert("RGB")
assert a.size == b.size, "NOTHING WAS CHECKED: %s and %s differ in size" % (a.size, b.size)

x, y, w, h = args.crop
box = (int(x * a.width), int(y * a.height), int((x + w) * a.width), int((y + h) * a.height))
a = a.crop(box)
b = b.crop(box)

pa = a.load()
pb = b.load()
shot = b.copy()
sp = shot.load()
changed = 0
for j in range(a.height):
    for i in range(a.width):
        d = max(abs(pa[i, j][k] - pb[i, j][k]) for k in range(3))
        if (d >= args.threshold):
            changed += 1
            sp[i, j] = (255, 0, 0)

total = a.width * a.height
print("%d of %d pixels differ by %d or more (%.2f%%)" % (changed, total, args.threshold,
                                                         100.0 * changed / total))
scale = max(1, 760 // max(1, shot.width))
shot.resize((shot.width * scale, shot.height * scale), Image.NEAREST).save(args.out)
print("wrote %s" % args.out)

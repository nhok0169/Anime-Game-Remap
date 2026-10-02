r"""How much does the render move between reloads? The acceptance test for the ordering fix.

Remapping inside each draw read a partly merged skeleton, so the picture differed from reload to
reload. If the ordering fix worked, repeated reloads of the same files now agree -- whatever they
agree ON. That is a different question from whether the model is right, and it is the one this
answers.

Compares only the model region (the right-hand two thirds), since the menu chrome is identical in
every shot and would swamp the difference.

  py -3 shotSpread.py <shot>...
"""
import itertools
import pathlib
import sys

from PIL import Image, ImageChops, ImageStat

paths = [pathlib.Path(p) for p in sys.argv[1:]]
assert len(paths) >= 2, "NOTHING WAS CHECKED: give at least two shots"
for p in paths:
    assert p.is_file(), "missing %s" % p

ims = []
for p in paths:
    im = Image.open(p).convert("RGB")
    w, h = im.size
    ims.append((p.name, im.crop((w // 3, 0, w, h))))

print("mean absolute difference over the model region, 0-255:\n")
worst = 0.0
for (an, a), (bn, b) in itertools.combinations(ims, 2):
    if (a.size != b.size):
        print("  %s vs %s: DIFFERENT SIZES, not compared" % (an, bn))
        continue
    stat = ImageStat.Stat(ImageChops.difference(a, b))
    d = sum(stat.mean) / 3.0
    worst = max(worst, d)
    print("  %-28s vs %-28s  %6.2f" % (an[:28], bn[:28], d))

print("\nworst pair: %.2f" % worst)
print("  < 1    the render is stable across reloads")
print("  > 5    it still differs run to run")

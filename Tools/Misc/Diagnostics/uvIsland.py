"""Crop the UV island an object occupies out of a texture and save it as a PNG, so it can be LOOKED at.

    python uvIsland.py <mod folder> <Texcoord.buf> <texture.dds> <out.png> [--stride N] [--no-marks]

**A texture bug is a picture; get a picture.** One in-game symptom produced three confident,
mechanical, mutually exclusive diagnoses -- the texture binding, then the material band remap, then
the geometry -- each refuted by a measurement within minutes of being written down, and a fourth
hypothesis (a global scaling factor) that measurement also ruled out. What ended it was taking the
object's own UVs, cropping exactly that rectangle out of the ``.dds``, and looking: the iris is a
circle in PIXELS on an atlas twice as wide as it is tall, so the island is a 2:1 rectangle holding
a circle -- correct, and something no numeric check had said anything about.

Prefer this to opening the whole atlas. A 2048x1024 sheet tells you nothing; a 64x64 crop of the
island in question tells you everything. Each vertex is marked in red (``--no-marks`` to turn that
off) and the crop is nearest-neighbour upscaled, so single pixels stay visible.

``<Texcoord.buf>`` and ``<texture.dds>`` are matched as a substring of the file name anywhere under
``<mod folder>``, so partial names are fine.

**The stride is not guessed.** Candidate layouts are tried and every one whose UVs land in [0, 1]
is reported; if more than one does, the tool STOPS and asks for ``--stride``. A range check alone
is not enough to identify a layout -- one 1440-byte eye buffer reads as 180 vertices at stride 8
and 120 at stride 12, and BOTH give UVs inside [0, 1]. Picking the first silently crops the wrong
rectangle, which is the same class of mistake this tool exists to end. Read the real stride off
the mod's own ``.ini`` (its ``Texcoord`` resource carries a ``stride``) when asked.

Set ``AG_REMAP_REPO`` to find the API elsewhere.

Needs ``numpy`` and ``Pillow``, and a built API for ``.dds`` decoding."""
import os
import sys

import numpy as np
from PIL import Image

Repo = os.environ.get("AG_REMAP_REPO", r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss")
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
sys.path.insert(0, APISrc)
if (hasattr(os, "add_dll_directory") and os.path.isdir(os.path.join(APISrc, "FixRaidenBoss2"))):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))

import FixRaidenBoss2 as FRB

folder, texcoordName, textureName, out = sys.argv[1:5]
marks = "--no-marks" not in sys.argv
forced = int(sys.argv[sys.argv.index("--stride") + 1]) if "--stride" in sys.argv else None

# GIMI's texcoord layouts: 8 = two floats; 12 = COLOR (4 bytes) + two floats; 16/20 add a second
# TEXCOORD. The UV pair we want is the FIRST one after any colour.
Layouts = {8: 0, 12: 4, 16: 4, 20: 4}


def find(name):
    hits = []
    for root, _, files in os.walk(folder):
        for f in files:
            if (name.lower() in f.lower()):
                hits.append(os.path.join(root, f))
    if (not hits):
        sys.exit(f"nothing matching {name!r} under {folder}")
    return sorted(hits, key = len)[0]


texcoordPath = find(texcoordName)
raw = open(texcoordPath, "rb").read()

viable = []
for candidate in ([forced] if forced else sorted(Layouts)):
    offset = Layouts.get(candidate)
    if (offset is None or not raw or len(raw) % candidate):
        continue

    count = len(raw) // candidate
    lines = np.frombuffer(raw, dtype = np.uint8).reshape(count, candidate)
    guess = lines[:, offset:offset + 8].copy().view(np.float32).reshape(count, 2)
    # A wrong layout reinterprets unrelated bytes as floats and produces numbers that look fine one
    # at a time, so the range is necessary -- but it is NOT sufficient, see the module docstring.
    if (np.isfinite(guess).all() and guess.min() >= -0.1 and guess.max() <= 1.1):
        viable.append((candidate, guess))

if (not viable):
    sys.exit(f"{os.path.basename(texcoordPath)}: no layout gives UVs in [0, 1] -- "
             f"{len(raw)} bytes; pass --stride to force one")

if (len(viable) > 1):
    print(f"{os.path.basename(texcoordPath)}   {len(raw)} bytes -- AMBIGUOUS, "
          f"{len(viable)} layouts all give UVs in [0, 1]:")
    # The ranges themselves are the tell: ONE object occupies a small island, so a layout whose UVs
    # span the whole sheet is reading unrelated bytes. Ranked, but still not chosen automatically.
    def area(guess):
        return (guess[:, 0].max() - guess[:, 0].min()) * (guess[:, 1].max() - guess[:, 1].min())

    for candidate, guess in sorted(viable, key = lambda pair: area(pair[1])):
        print(f"    stride {candidate:>3}  {len(guess):>6} vertices   "
              f"u [{guess[:, 0].min():.4f}, {guess[:, 0].max():.4f}]   "
              f"v [{guess[:, 1].min():.4f}, {guess[:, 1].max():.4f}]   "
              f"island {area(guess):.3f} of the sheet")
    sys.exit("Re-run with --stride N. The mod's own .ini carries the real one on the Texcoord "
             "resource; guessing here crops the wrong rectangle and looks entirely plausible.")

stride, uv = viable[0]
print(f"{os.path.basename(texcoordPath)}   {len(uv)} vertices, stride {stride}")
print(f"  u in [{uv[:, 0].min():.4f}, {uv[:, 0].max():.4f}]   v in [{uv[:, 1].min():.4f}, {uv[:, 1].max():.4f}]")

texPath = find(textureName)
tex = FRB.CppTextureFile(texPath)
tex.open()
# `hasImage` is a PROPERTY on the binding, not a method -- calling it raises rather than returning
# a truthy object, so the mistake is at least loud.
if (not tex.hasImage):
    sys.exit(f"{os.path.basename(texPath)}: could not be decoded")

pixels = np.frombuffer(bytes(tex.getPixels()), dtype = np.uint8).reshape(tex.height, tex.width, 4)

pad = 8
x0 = max(0, int(uv[:, 0].min() * tex.width) - pad)
x1 = min(tex.width, int(uv[:, 0].max() * tex.width) + pad)
y0 = max(0, int(uv[:, 1].min() * tex.height) - pad)
y1 = min(tex.height, int(uv[:, 1].max() * tex.height) + pad)
patch = pixels[y0:y1, x0:x1, :3].copy()

if (marks):
    for u, v in uv:
        x = int(u * tex.width) - x0
        y = int(v * tex.height) - y0
        if (0 <= x < patch.shape[1] and 0 <= y < patch.shape[0]):
            patch[max(0, y - 1):y + 2, max(0, x - 1):x + 2] = [255, 0, 0]

image = Image.fromarray(patch, "RGB")
scale = max(1, 360 // max(1, max(patch.shape[0], patch.shape[1])))
image = image.resize((patch.shape[1] * scale, patch.shape[0] * scale), Image.NEAREST)
image.save(out)

aspect = (x1 - x0) / max(1, y1 - y0)
print(f"{os.path.basename(texPath)}   {tex.width}x{tex.height}")
print(f"  island {x1 - x0}x{y1 - y0} px at ({x0}, {y0}), aspect {aspect:.2f}, upscaled {scale}x  ->  {out}")

r"""Does a fixed WuWa mod's remapped `Blend.buf` name the bones its vertices are weighted to?

Past 256 merged bones a WWMI mod's blend indices do not fit the byte `Blend.buf` holds, so the real
16-bit ids live in the companion `...BlendRemapVertexVG.buf` and WWMI's `BlendRemapper` compute shader
overwrites the byte at run time with `ReverseMap[trueId]` -- the LOCAL id of a <= 512 entry window.

That makes the bytes on disk a placeholder, and a placeholder is still what the GPU draws on any frame
the compute pass does not land. Writing the merged id TRUNCATED there is wrong under every
circumstance: Chisa's bone 409 becomes 153, a live bone elsewhere on the body, so the mesh renders
scrambled rather than absent and every offline measurement of the inputs still comes back clean. The
right placeholder is the local id, which is what the pass computes anyway.

This reports which of the three a fixed mod's bytes actually hold, per weighted influence slot:

  local      the local id -- idempotent with the compute pass, correct without it
  merged     the merged id, which happens to fit a byte (so is also the local id's cousin, not it)
  truncated  a merged id past 255 cut down to a byte: NAMES THE WRONG BONE

  py -3 wwmiBlendPlaceholder.py <fixed mod folder> [--prefix <fix name>]

Exits 1 if any slot is truncated or unaccounted for, so it can gate a remap.
"""
import argparse
import collections
import pathlib
import struct
import sys

parser = argparse.ArgumentParser()
parser.add_argument("mod", help="the fixed mod folder (its Meshes/ holds the generated buffers)")
parser.add_argument("--prefix", default = None,
                    help="the fix's name, if several remaps live in one folder")
args = parser.parse_args()

root = pathlib.Path(args.mod)
if (not root.is_dir()):
    sys.exit(f"NOTHING WAS CHECKED: {root} is not a folder")

# Find the generated set by the one file only this path writes, so a folder holding several remaps
# (or the host mod's own WWMI-Tools buffers) cannot be mistaken for it.
# The generated set is `<base>.buf`, `<base>RemapForward.buf`, `<base>RemapReverse.buf` and
# `<base>RemapVertexVG.buf`, where <base> already ends in "Blend" -- so the stem to strip is
# "RemapReverse.buf", not "BlendRemapReverse.buf".
Suffix = "RemapReverse.buf"
reverses = sorted(p for p in root.rglob("*BlendRemapReverse.buf")
                  if args.prefix is None or args.prefix in p.name)
if (not reverses):
    sys.exit("NOTHING WAS CHECKED: no *BlendRemapReverse.buf -- this mod has no blend remap")

bad = 0
for reversePath in reverses:
    stem = reversePath.name[: -len(Suffix)]
    blendPath = reversePath.with_name(stem + ".buf")
    vgPath = reversePath.with_name(stem + "RemapVertexVG.buf")
    if (not blendPath.is_file() or not vgPath.is_file()):
        print(f"{stem}: INCOMPLETE -- missing {blendPath.name if not blendPath.is_file() else vgPath.name}")
        bad += 1
        continue

    raw = reversePath.read_bytes()
    reverse = list(struct.unpack(f"<{len(raw) // 2}H", raw))
    blend = blendPath.read_bytes()
    vg = vgPath.read_bytes()
    verts = len(blend) // 16
    if (len(vg) != verts * 16):
        print(f"{stem}: its VertexVG holds {len(vg) // 16} vertices, its Blend.buf {verts}")
        bad += 1
        continue

    kinds = collections.Counter()
    worst = []
    for v in range(verts):
        full = struct.unpack_from("<8H", vg, v * 16)
        for k in range(8):
            if (not blend[v * 16 + 8 + k]):
                continue                                   # a weight-zero slot names nothing
            m = full[k]
            disk = blend[v * 16 + k]
            if (m < len(reverse) and disk == reverse[m]):
                kinds["local"] += 1
            elif (disk == m & 0xFF):
                kinds["truncated" if m > 255 else "merged"] += 1
                if (m > 255 and len(worst) < 5):
                    worst.append((v, m, disk, reverse[m] if m < len(reverse) else None))
            else:
                kinds["other"] += 1

    total = sum(kinds.values())
    print(f"{stem}  ({verts} vertices, {total} weighted influence slots)")
    for kind in ("local", "merged", "truncated", "other"):
        note = "   <-- NAMES THE WRONG BONE" if (kind in ("truncated", "other") and kinds[kind]) else ""
        print(f"    {kind:<10} {kinds[kind]:>9}{note}")
    for v, m, disk, want in worst:
        print(f"      vertex {v}: weighted to merged {m}, byte says {disk}, local would be {want}")
    if (kinds["truncated"] or kinds["other"]):
        bad += 1

sys.exit(1 if bad else 0)

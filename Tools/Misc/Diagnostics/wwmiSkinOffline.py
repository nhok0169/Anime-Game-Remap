r"""Skin a fixed WuWa mod offline against the TARGET's own skeleton, and say where its vertices land.

The point is independence. A frame dump of the FIX's own draws can only confirm whatever the fix
already did, and three diagnoses of the ChisaParfait -> Chisa remap were taken that way and were
wrong. This rebuilds the skeleton from a dump of the TARGET CHARACTER instead:

  * each of the target's component draws dumps an index buffer whose .txt sidecar gives
    `first index` and `index count`, which names the component
  * that draw's `vs-cb4=...` buffer is that component's bone data, LOCAL and 0-based, 3 x float4 per
    bone (WWMI's SkeletonMerger.hlsl)
  * so merged[vgOffset + i] = cb4[i] over every window -> the target's whole merged skeleton

Then the fix's own forward map takes that to the window the remapped draw addresses
(SkeletonRemapper.hlsl: remapped[local] = merged[forward[local]]), the mod's Position.buf is skinned
with the local ids and weights in the remapped Blend.buf, and the result is summarised.

A healthy remap puts every vertex in one character-sized cluster. Vertices far outside it stretch
across the scene; a cluster collapsed to a point is a mod that renders as nothing.

  py -3 wwmiSkinOffline.py --mod <fixed mod folder> --dump <frame analysis of the target>
                           --target <the target's identity mod folder>

Exits 1 if anything lands outside the body's envelope or the mesh has collapsed.
"""
import argparse
import collections
import glob
import os
import pathlib
import re
import struct
import sys

parser = argparse.ArgumentParser()
parser.add_argument("--mod", required=True, help="the FIXED mod folder")
parser.add_argument("--dump", required=True, help="a frame analysis folder of the TARGET character")
parser.add_argument("--target", required=True,
                    help="the target's identity mod folder, for its component windows")
parser.add_argument("--cb", default="vs-cb4", help="which bone-data slot to read (vs-cb4 / vs-cb3)")
args = parser.parse_args()

mod = pathlib.Path(args.mod)
dump = pathlib.Path(args.dump)
target = pathlib.Path(args.target)
for p, what in ((mod, "mod"), (dump, "dump"), (target, "target")):
    if (not p.is_dir()):
        sys.exit("NOTHING WAS CHECKED: no %s at %s" % (what, p))

# ---- the fix's generated set, found by the one file only this path writes ---------------------
reverses = sorted(mod.rglob("*BlendRemapReverse.buf"))
if (not reverses):
    sys.exit("NOTHING WAS CHECKED: no *BlendRemapReverse.buf -- this mod has no blend remap")
mesh = reverses[0].parent
stem = reverses[0].name[: -len("RemapReverse.buf")]              # e.g. "ChisaRemapBlend"

# ---- the target's windows, from its identity mod ----------------------------------------------
inis = sorted(target.rglob("*.ini"))
if (not inis):
    sys.exit("NOTHING WAS CHECKED: no .ini under %s" % target)
ident = inis[0].read_bytes().decode("utf-8", "replace").replace("\r\n", "\n")
windows = []
for body in re.findall(r"^\[TextureOverrideComponent\d+\]\n(.*?)(?=\n\[)", ident, re.S | re.M):
    first = re.search(r"match_first_index = (\d+)", body)
    count = re.search(r"match_index_count = (\d+)", body)
    off = re.search(r"vg_offset = (\d+)", body)
    cnt = re.search(r"vg_count = (\d+)", body)
    if (first and count and off and cnt):
        windows.append((int(first.group(1)), int(count.group(1)),
                        int(off.group(1)), int(cnt.group(1))))
if (not windows):
    sys.exit("NOTHING WAS CHECKED: no component windows in %s" % inis[0].name)
bones = max(o + c for _, _, o, c in windows)
print("target: %d components, %d merged bones" % (len(windows), bones))

# ---- the merged skeleton, from the dump -------------------------------------------------------
byRange = {(f, c): (o, n) for f, c, o, n in windows}
merged = [None] * bones
seen = set()
for txt in glob.glob(os.path.join(str(dump), "*-ib=*.txt")):
    head = open(txt, encoding="utf-8", errors="replace").read(400)
    first = re.search(r"first index: (\d+)", head)
    count = re.search(r"index count: (\d+)", head)
    if (not first or not count):
        continue
    key = (int(first.group(1)), int(count.group(1)))
    if (key not in byRange or key in seen):
        continue
    drawId = os.path.basename(txt).split("-")[0]
    cbs = glob.glob(os.path.join(str(dump), "%s-%s=*.buf" % (drawId, args.cb)))
    if (not cbs):
        continue
    raw = open(cbs[0], "rb").read()
    rows = [struct.unpack_from("<4f", raw, i * 16) for i in range(len(raw) // 16)]
    off, cnt = byRange[key]
    for i in range(cnt):
        if ((i * 3 + 2) < len(rows)):
            merged[off + i] = (rows[i * 3], rows[i * 3 + 1], rows[i * 3 + 2])
    seen.add(key)

absent = [b for b in range(bones) if merged[b] is None]
print("windows found in the dump: %d of %d; merged bones with no matrix: %d\n"
      % (len(seen), len(windows), len(absent)))
if (len(seen) < len(windows)):
    print("  (a window the dump does not cover is not the fix's fault -- take a dump where the")
    print("   whole character is on screen before reading anything below)\n")

# ---- the mesh --------------------------------------------------------------------------------
fwdRaw = (mesh / (stem + "RemapForward.buf")).read_bytes()
forward = list(struct.unpack("<%dH" % (len(fwdRaw) // 2), fwdRaw))
pos = (mesh / "Position.buf").read_bytes()
blend = (mesh / (stem + ".buf")).read_bytes()
verts = len(pos) // 12
if (len(blend) // 16 != verts):
    sys.exit("NOTHING WAS CHECKED: %s has %d vertices, Position.buf %d"
             % (stem, len(blend) // 16, verts))
print("mesh: %d vertices\n" % verts)

placed = []
noInfluence = 0
for v in range(verts):
    x, y, z = struct.unpack_from("<3f", pos, v * 12)
    out = [0.0, 0.0, 0.0]
    live = False
    for k in range(8):
        w = blend[v * 16 + 8 + k]
        if (not w):
            continue
        local = blend[v * 16 + k]
        m = merged[forward[local]] if (local < len(forward) and forward[local] < bones) else None
        if (m is None):
            continue
        live = True
        f = w / 255.0
        for r in range(3):
            out[r] += f * (m[r][0] * x + m[r][1] * y + m[r][2] * z + m[r][3])
    if (not live):
        noInfluence += 1
    placed.append(out)

dists = sorted((p[0] ** 2 + p[1] ** 2 + p[2] ** 2) ** 0.5 for p in placed)
med = dists[len(dists) // 2]
print("distance from the origin: median %.2f, 99th %.2f, max %.2f" % (med, dists[int(len(dists) * 0.99)], dists[-1]))

xs = [p[0] for p in placed]
ys = [p[1] for p in placed]
zs = [p[2] for p in placed]
span = max(max(xs) - min(xs), max(ys) - min(ys), max(zs) - min(zs))
print("bounding box: %.2f x %.2f x %.2f  (largest span %.2f)"
      % (max(xs) - min(xs), max(ys) - min(ys), max(zs) - min(zs), span))
print("vertices with NO live influence (they sit at the origin): %d  (%.2f%%)\n"
      % (noInfluence, 100.0 * noInfluence / verts))

limit = med * 3.0 + 1.0
far = [v for v in range(verts) if (placed[v][0] ** 2 + placed[v][1] ** 2 + placed[v][2] ** 2) ** 0.5 > limit]
bad = False
if (far):
    bad = True
    print("FLYING OFF: %d vertices past %.1f (3x the median)" % (len(far), limit))
    blame = collections.Counter()
    for v in far:
        for k in range(8):
            if (blend[v * 16 + 8 + k]):
                blame[forward[blend[v * 16 + k]]] += 1
    for b, n in blame.most_common(10):
        print("    merged bone %-4d: %d far vertices" % (b, n))
elif (span < 1.0):
    bad = True
    print("COLLAPSED: the whole mesh spans %.3f units -- it would render as nothing" % span)
else:
    print("every vertex lands inside one character-sized cluster")

sys.exit(1 if (bad or noInfluence) else 0)

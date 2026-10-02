r"""Which vertices of the remapped mesh land far from the body, and what are they weighted to?

Everything here is offline and independent of the fix's own output, which is the point: three
earlier diagnoses were taken from dumps of the fix's own skeleton and so could only confirm
themselves.

The skeleton is rebuilt from a frame dump of CHISA HERSELF:

  * each of her component draws dumps an index buffer whose .txt sidecar gives `first index` and
    `index count`, which names the component
  * that draw's `vs-cb4=<boneDataHash>` buffer is that component's bone data, LOCAL and 0-based,
    3 x float4 per bone (SkeletonMerger.hlsl)
  * so merged[(vgOffset + i)] = cb4[i] for i < vgCount, over all seven windows -> Chisa's 420 bones

Then the fix's own forward map takes that to the 180-bone window the draw addresses
(SkeletonRemapper.hlsl: remapped[local] = merged[forward[local]]), the mod's Position.buf is skinned
with the local ids and weights in the remapped Blend.buf, and the result is compared against the
bulk of the mesh.

  py -3 flyAway.py [--dump <frame analysis folder>]
"""
import argparse
import collections
import glob
import os
import pathlib
import re
import struct

parser = argparse.ArgumentParser()
parser.add_argument("--dump", default=r"C:\Users\AlexX\Documents\Games\Mods"
                                       r"\XXMI-Launcher-Portable-v1.8.5\WWMI"
                                       r"\FrameAnalysis-Chisa-Max-LOD-2026-09-21-080231")
parser.add_argument("--cb", default="vs-cb4", help="vs-cb4 (primary) or vs-cb3 (the extra skeleton)")
args = parser.parse_args()

W = pathlib.Path(r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI")
dump = pathlib.Path(args.dump)
assert dump.is_dir(), "NOTHING WAS CHECKED: no dump at %s" % dump
mod = next(p for p in (W / "Mods" / "ChisaParfaitIdentity", W / "ChisaParfaitIdentity")
           if (p / "Meshes").is_dir())
mesh = mod / "Meshes"

# ---- Chisa's windows, from her own identity mod -----------------------------------------------
ident = (W / "ChisaIdentity" / "mod.ini").read_bytes().decode("utf-8", "replace").replace("\r\n", "\n")
windows = []                                                # (firstIndex, indexCount, vgOffset, vgCount)
for body in re.findall(r"^\[TextureOverrideComponent\d+\]\n(.*?)(?=\n\[)", ident, re.S | re.M):
    first = re.search(r"match_first_index = (\d+)", body)
    count = re.search(r"match_index_count = (\d+)", body)
    off = re.search(r"vg_offset = (\d+)", body)
    cnt = re.search(r"vg_count = (\d+)", body)
    if (first and count and off and cnt):
        windows.append((int(first.group(1)), int(count.group(1)),
                        int(off.group(1)), int(cnt.group(1))))
assert windows, "NOTHING WAS CHECKED: no component windows in ChisaIdentity's .ini"
bones = max(o + c for _, _, o, c in windows)
print("Chisa: %d components, %d merged bones\n" % (len(windows), bones))

# ---- the merged skeleton, from the dump -------------------------------------------------------
byRange = {(f, c): (o, n) for f, c, o, n in windows}
merged = [None] * bones
found = {}
for txt in glob.glob(os.path.join(str(dump), "*-ib=*.txt")):
    head = open(txt, encoding="utf-8", errors="replace").read(400)
    first = re.search(r"first index: (\d+)", head)
    count = re.search(r"index count: (\d+)", head)
    if (not first or not count):
        continue
    key = (int(first.group(1)), int(count.group(1)))
    if (key not in byRange or key in found):
        continue
    drawId = os.path.basename(txt).split("-")[0]
    cbs = glob.glob(os.path.join(str(dump), "%s-%s=*.buf" % (drawId, args.cb)))
    if (not cbs):
        continue
    raw = open(cbs[0], "rb").read()
    rows = [struct.unpack_from("<4f", raw, i * 16) for i in range(len(raw) // 16)]
    off, cnt = byRange[key]
    for i in range(cnt):
        if (i * 3 + 2) < len(rows):
            merged[off + i] = (rows[i * 3], rows[i * 3 + 1], rows[i * 3 + 2])
    found[key] = (drawId, off, cnt)

print("%-28s %-8s %s" % ("component draw", "window", "bone data"))
for (f, c), (drawId, off, cnt) in sorted(found.items()):
    print("  first %-8d count %-8d [%3d, %3d)  draw %s" % (f, c, off, off + cnt, drawId))
missing = [w for w in windows if (w[0], w[1]) not in found]
for f, c, off, cnt in missing:
    print("  first %-8d count %-8d [%3d, %3d)  NOT IN THE DUMP" % (f, c, off, off + cnt))

empty = [b for b in range(bones) if merged[b] is None]
print("\nmerged bones with no matrix: %d" % len(empty))

# ---- the fix's forward map, and the remapped window --------------------------------------------
fwdRaw = (mesh / "ChisaRemapBlendRemapForward.buf").read_bytes()
forward = list(struct.unpack("<%dH" % (len(fwdRaw) // 2), fwdRaw))
revRaw = (mesh / "ChisaRemapBlendRemapReverse.buf").read_bytes()
reverse = list(struct.unpack("<%dH" % (len(revRaw) // 2), revRaw))

pos = (mesh / "Position.buf").read_bytes()
blend = (mesh / "ChisaRemapBlend.buf").read_bytes()
vg = (mesh / "ChisaRemapBlendRemapVertexVG.buf").read_bytes()
verts = len(pos) // 12
assert len(blend) // 16 == verts, "Blend.buf has %d vertices, Position.buf %d" % (len(blend) // 16, verts)
print("mesh: %d vertices\n" % verts)


def skin(v):
    """The vertex's skinned position, and the local ids that had weight."""
    x, y, z = struct.unpack_from("<3f", pos, v * 12)
    out = [0.0, 0.0, 0.0]
    total = 0
    used = []
    for k in range(8):
        w = blend[v * 16 + 8 + k]
        if (not w):
            continue
        local = blend[v * 16 + k]
        total += w
        used.append(local)
        m = merged[forward[local]] if forward[local] < bones else None
        if (m is None):
            continue                                   # a bone with no matrix contributes nothing
        f = w / 255.0
        for r in range(3):
            out[r] += f * (m[r][0] * x + m[r][1] * y + m[r][2] * z + m[r][3])
    return out, used, total


dists = []
for v in range(verts):
    p, used, total = skin(v)
    dists.append((p[0] * p[0] + p[1] * p[1] + p[2] * p[2]) ** 0.5)

ordered = sorted(dists)
med = ordered[len(ordered) // 2]
p99 = ordered[int(len(ordered) * 0.99)]
print("distance from the origin: median %.2f, 99th %.2f, max %.2f" % (med, p99, ordered[-1]))

# The body is the bulk; anything far outside it is what stretches across the scene.
limit = med * 3.0 + 1.0
far = [v for v in range(verts) if dists[v] > limit]
print("vertices past %.1f (3x the median): %d  (%.2f%%)\n"
      % (limit, len(far), 100.0 * len(far) / verts))

blame = collections.Counter()
blameBone = collections.Counter()
for v in far:
    _, used, _ = skin(v)
    for local in set(used):
        blame[local] += 1
        blameBone[forward[local]] += 1

if (far):
    print("%-8s %-10s %-9s %s" % ("local", "merged", "has matrix", "far vertices on it"))
    for local, n in blame.most_common(15):
        m = forward[local]
        print("  %-6d %-10d %-9s %d" % (local, m, "no" if (m >= bones or merged[m] is None) else "yes", n))
else:
    print("nothing flies off: every vertex lands inside the body's envelope")

# The other half of the question: bones with no matrix that the mesh actually uses.
usedBones = collections.Counter()
for v in range(verts):
    for k in range(8):
        if (blend[v * 16 + 8 + k]):
            usedBones[forward[blend[v * 16 + k]]] += 1
dead = {b: n for b, n in usedBones.items() if b >= bones or merged[b] is None}
print("\nbones the mesh is weighted to that have NO matrix in the rebuilt skeleton: %d" % len(dead))
for b in sorted(dead, key=lambda k: -dead[k])[:10]:
    print("  merged bone %-4d: %d weighted slots" % (b, dead[b]))

r"""Which triangle edges does the remap stretch? The pose-robust form of "is a part mis-skinned".

Comparing skinned POSITIONS between two dumps is confounded by pose: the two characters stand
differently in their own frames, so a centroid alignment leaves a median residual of 4.6 units and
drowns the thing being looked for. Edge LENGTH does not care about pose -- a part whose bones have
real counterparts keeps its edges (the guides measure median stretch 1.00, p90 1.15), and a region
riding a bone that sits elsewhere tears away from its neighbours.

So: skin the mesh twice, once with the mod's own ids under the source's merged skeleton and once
with the remapped ids under the target's, and report the edges whose length ratio blows up --
grouped by the source bone the stretched vertices ride, which names the vertex group rows.
"""
import collections
import glob
import os
import pathlib
import re
import struct
import sys

sys.stdout.reconfigure(errors="replace")
W = pathlib.Path(r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI")
mod = next(c for c in (W / "Mods" / "ChisaParfait1", W / "ChisaParfait1") if c.is_dir())
mesh = next(p.parent for p in mod.rglob("*BlendRemapReverse.buf"))
stem = next(p.name[: -len("RemapReverse.buf")] for p in mod.rglob("*BlendRemapReverse.buf"))


def windowsOf(identity):
    ini = sorted((W / identity).rglob("*.ini"))[0]
    text = ini.read_bytes().decode("utf-8", "replace").replace("\r\n", "\n")
    out = {}
    for body in re.findall(r"^\[TextureOverrideComponent\d+\]\n(.*?)(?=\n\[)", text, re.S | re.M):
        g = [re.search(p, body) for p in (r"match_first_index = (\d+)", r"match_index_count = (\d+)",
                                          r"vg_offset = (\d+)", r"vg_count = (\d+)")]
        if (all(g)):
            out[(int(g[0].group(1)), int(g[1].group(1)))] = (int(g[2].group(1)), int(g[3].group(1)))
    return out


def skeletonOf(dump, windows):
    bones = max(o + c for o, c in windows.values())
    merged = [None] * bones
    for txt in glob.glob(os.path.join(dump, "*-ib=*.txt")):
        head = open(txt, encoding="utf-8", errors="replace").read(400)
        f, c = re.search(r"first index: (\d+)", head), re.search(r"index count: (\d+)", head)
        if (not f or not c or (int(f.group(1)), int(c.group(1))) not in windows):
            continue
        drawId = os.path.basename(txt).split("-")[0]
        cbs = glob.glob(os.path.join(dump, "%s-vs-cb4=*.buf" % drawId))
        if (not cbs):
            continue
        raw = open(cbs[0], "rb").read()
        rows = [struct.unpack_from("<4f", raw, i * 16) for i in range(len(raw) // 16)]
        off, cnt = windows[(int(f.group(1)), int(c.group(1)))]
        for i in range(cnt):
            if ((i * 3 + 2) < len(rows)):
                merged[off + i] = (rows[i * 3], rows[i * 3 + 1], rows[i * 3 + 2])
    return merged


srcSkel = skeletonOf(sorted(glob.glob(str(W / "FrameAnalysis-ChisaParfait-*")))[-1],
                     windowsOf("ChisaParfaitIdentity"))
tgtSkel = skeletonOf(str(W / "FrameAnalysis-Chisa-Max-LOD-2026-09-21-080231"), windowsOf("ChisaIdentity"))

posRaw = (mesh / "Position.buf").read_bytes()
verts = len(posRaw) // 12
pos = [struct.unpack_from("<3f", posRaw, v * 12) for v in range(verts)]
origBlend = (mesh / "Blend.buf").read_bytes()
newBlend = (mesh / (stem + ".buf")).read_bytes()
fwdRaw = (mesh / (stem + "RemapForward.buf")).read_bytes()
forward = list(struct.unpack("<%dH" % (len(fwdRaw) // 2), fwdRaw))
vgPath = mesh / "BlendRemapVertexVG.buf"
vgRaw = vgPath.read_bytes() if vgPath.is_file() else None

srcIds, newIds, weights = [], [], []
for v in range(verts):
    srcIds.append(struct.unpack_from("<8H", vgRaw, v * 16) if vgRaw is not None
                  else tuple(origBlend[v * 16: v * 16 + 8]))
    newIds.append(tuple(newBlend[v * 16: v * 16 + 8]))
    weights.append(tuple(origBlend[v * 16 + 8: v * 16 + 16]))


def skin(ids, skel, fwd=None):
    out = []
    for v in range(verts):
        x, y, z = pos[v]
        p, live = [0.0, 0.0, 0.0], False
        for k in range(8):
            w = weights[v][k]
            if (not w):
                continue
            b = ids[v][k]
            if (fwd is not None):
                b = fwd[b] if b < len(fwd) else len(skel)
            m = skel[b] if b < len(skel) else None
            if (m is None):
                continue
            live = True
            f = w / 255.0
            for r in range(3):
                p[r] += f * (m[r][0] * x + m[r][1] * y + m[r][2] * z + m[r][3])
        out.append(tuple(p) if live else None)
    return out


a, b = skin(srcIds, srcSkel), skin(newIds, tgtSkel, forward)

idx = (mesh / "Index.buf").read_bytes()
tris = struct.unpack("<%dI" % (len(idx) // 4), idx)
edges = set()
for t in range(0, len(tris) - 2, 3):
    i, j, k = tris[t], tris[t + 1], tris[t + 2]
    for p, q in ((i, j), (j, k), (k, i)):
        edges.add((p, q) if p < q else (q, p))
print("%d vertices, %d unique edges" % (verts, len(edges)))


def dist(p, q):
    return sum((p[i] - q[i]) ** 2 for i in range(3)) ** 0.5


ratios = []
for p, q in edges:
    if (a[p] is None or a[q] is None or b[p] is None or b[q] is None):
        continue
    la, lb = dist(a[p], a[q]), dist(b[p], b[q])
    if (la < 1e-4):
        continue
    ratios.append((lb / la, p, q))
ratios.sort(reverse=True)
vals = [r for r, _, _ in ratios]
print("edge stretch: median %.3f  p90 %.3f  p99 %.3f  max %.1f\n"
      % (vals[len(vals) // 2], vals[int(len(vals) * 0.10)], vals[int(len(vals) * 0.01)], vals[0]))

bad = [(r, p, q) for r, p, q in ratios if r > 3.0]
print("edges stretched more than 3x: %d of %d (%.3f%%)\n" % (len(bad), len(ratios), 100.0 * len(bad) / len(ratios)))
if (bad):
    blame = collections.Counter()
    tgtOf = {}
    for _, p, q in bad:
        for v in (p, q):
            for k in range(8):
                if (weights[v][k]):
                    s = srcIds[v][k]
                    blame[s] += 1
                    tgtOf.setdefault(s, forward[newIds[v][k]] if newIds[v][k] < len(forward) else -1)
    print("%-14s %-12s %s" % ("source bone", "-> target", "stretched edge ends on it"))
    for s, n in blame.most_common(15):
        print("  %-12d %-12s %d" % (s, tgtOf.get(s, "?"), n))

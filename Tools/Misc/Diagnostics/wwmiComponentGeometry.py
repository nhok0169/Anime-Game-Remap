#
# ===== wwmiComponentGeometry =====
#
# Which draw slot of one WuWa character is which draw slot of another, measured from the GEOMETRY.
#
# The standing rule for a WuWa remap is that SHADER FAMILIES decide the slot mapping, because a
# merged skeleton makes every bone reachable from every draw so the bones say nothing. That rule
# exists for a pair of unrelated characters. When the two are SKINS OF ONE CHARACTER they stand in
# the same rest pose, and then the honest measurement is per-component overlap -- which settled
# Chisa <-> ChisaParfait in minutes after reading shader names had not: three of her components came
# out at IoU 1.00, literally the same mesh, and the shader families then only had to confirm it.
#
# Overlap is reported three ways, because each can mislead alone:
#   * IoU of the axis-aligned bounding boxes -- cheap, and 1.00 means the same box, not the same mesh
#   * centroid distance, which separates two parts that share a box (a bodice and the skin under it)
#   * VERTEX overlap: the share of one component's vertices with a vertex of the other within a
#     small radius, which is the only one of the three that says "the same mesh" rather than
#     "the same region"
#
#   py -3 wwmiComponentGeometry.py <mod A> <mod B> [--radius 0.25]
#
# The two arguments are folders holding a WWMI mod each -- the two IDENTITY mods, normally, since a
# real mod's mesh is the author's and not the character's. Read the component windows off each mod's
# own `.ini`, never off the other's.
#

import argparse
import os
import re
import sys

import numpy as np


def findIni(folder):
    for root, _, files in os.walk(folder):
        for name in sorted(files):
            if (not name.lower().endswith(".ini")):
                continue
            path = os.path.join(root, name)
            if ("[TextureOverrideComponent" in open(path, "r", encoding = "utf-8",
                                                    errors = "replace").read()):
                return path
    return None


def readComponents(iniPath):
    text = open(iniPath, "r", encoding = "utf-8", errors = "replace").read()
    out = {}
    for block in re.finditer(r'\[TextureOverrideComponent(\d+)\](.*?)(?=\n\[|\Z)', text, re.S):
        body = block.group(2)
        first = re.search(r'^\s*match_first_index\s*=\s*(\d+)', body, re.M)
        count = re.search(r'^\s*match_index_count\s*=\s*(\d+)', body, re.M)
        if (first and count):
            out[int(block.group(1))] = (int(first.group(1)), int(count.group(1)))
    return out


def readBuf(folder, name):
    for root, _, files in os.walk(folder):
        if (name in files):
            return os.path.join(root, name)
    return None


def load(folder):
    ini = findIni(folder)
    if (ini is None):
        raise SystemExit(f"no .ini with component sections under {folder}")
    index = np.fromfile(readBuf(folder, "Index.buf"), dtype = "<u4")
    position = np.fromfile(readBuf(folder, "Position.buf"), dtype = "<f4").reshape(-1, 3)
    comps = readComponents(ini)
    out = {}
    for c, (first, count) in sorted(comps.items()):
        verts = np.unique(index[first: first + count])
        verts = verts[verts < len(position)]
        out[c] = position[verts]
    return out, os.path.basename(os.path.normpath(folder))


def box(p):
    return p.min(axis = 0), p.max(axis = 0)


def iou(a, b):
    lo = np.maximum(a[0], b[0])
    hi = np.minimum(a[1], b[1])
    inter = np.prod(np.maximum(0.0, hi - lo))
    va = np.prod(a[1] - a[0])
    vb = np.prod(b[1] - b[0])
    union = va + vb - inter
    return 0.0 if (union <= 0) else inter / union


def nearShare(a, b, radius):
    """the share of a's vertices with a vertex of b within `radius` -- on a voxel grid, so it is
    linear in the vertex count rather than quadratic"""
    cell = radius
    keys = set(map(tuple, np.floor(b / cell).astype(np.int64)))
    mine = np.floor(a / cell).astype(np.int64)
    hits = 0
    offsets = [(dx, dy, dz) for dx in (-1, 0, 1) for dy in (-1, 0, 1) for dz in (-1, 0, 1)]
    for v in mine:
        for dx, dy, dz in offsets:
            if ((v[0] + dx, v[1] + dy, v[2] + dz) in keys):
                hits += 1
                break
    return hits / len(a)


parser = argparse.ArgumentParser(description = "per-component geometry overlap of two WuWa mods")
parser.add_argument("modA")
parser.add_argument("modB")
parser.add_argument("--radius", type = float, default = 0.25)
args = parser.parse_args()

A, nameA = load(args.modA)
B, nameB = load(args.modB)

print(f"  A = {nameA}: {len(A)} components, {sum(len(p) for p in A.values())} vertices")
print(f"  B = {nameB}: {len(B)} components, {sum(len(p) for p in B.values())} vertices")
print(f"\n  {'A':>3} {'verts':>7}  {'B':>3} {'verts':>7}   {'boxIoU':>7} {'dCentroid':>9}"
      f" {'A in B':>7} {'B in A':>7}")

best = {}
for ca, pa in sorted(A.items()):
    rows = []
    for cb, pb in sorted(B.items()):
        i = iou(box(pa), box(pb))
        d = float(np.linalg.norm(pa.mean(axis = 0) - pb.mean(axis = 0)))
        rows.append((i, -d, cb, pb))
    rows.sort(reverse = True)
    for i, negD, cb, pb in rows[:3]:
        ab = nearShare(pa, pb, args.radius)
        ba = nearShare(pb, pa, args.radius)
        mark = "  <-" if (cb == rows[0][2]) else ""
        print(f"  {ca:>3} {len(pa):>7}  {cb:>3} {len(pb):>7}   {i:7.2f} {-negD:9.2f}"
              f" {ab:7.1%} {ba:7.1%}{mark}")
        if (cb == rows[0][2]):
            best[ca] = (cb, i, -negD, ab, ba)
    print()

print("  best-box pairing, and whether the vertex overlap agrees with it:")
for ca, (cb, i, d, ab, ba) in sorted(best.items()):
    verdict = ("the SAME mesh" if (ab > 0.9 and ba > 0.9)
               else "the same region, different mesh" if (ab > 0.3 or ba > 0.3)
               else "only the same box -- treat as unpaired")
    print(f"    A {ca} -> B {cb}   boxIoU {i:.2f}  centroid {d:.2f}  {verdict}")

"""Is the remapped blend ANATOMICALLY sane -- and is Chisa17's as sane as a mod that works?

A conversion from per-component LOCAL bone ids to merged ones cannot be checked by looking at the
numbers: any wrong-but-in-range table gives ids that span the right interval, preserve the weights
and leave ~1% unchanged. What it cannot fake is GEOMETRY. A vertex on the left shoulder must end up
on a target bone whose own vertices are near the left shoulder.

So: for every vertex, take its dominant (highest-weight) target bone, look up where THAT bone's
vertices sit on the target's own mesh, and measure the distance. A correct table gives a small
distance; a table off by a component's offset gives a large one.

Chisa16 is the control -- same fix, same target, confirmed working in game. If Chisa17's distances
are in the same range, the conversion is sound and the slant is elsewhere. If they are much worse,
the conversion is the bug.
"""
import json
import os
import sys

import numpy as np

Repo = r"C:/Users/AlexX/Documents/Games/Mods/Repos/Anime-Game-Remap/.claude/worktrees/add-chisa-worktree"
WWMI = r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI"
Tgt = os.path.join(Repo, "Data", "Mod Downloads", "WuWa", "ChisaParfait", "3_5")

# ---- where each of the TARGET's merged bones lives, from her own geometry ----------------------
tp = np.fromfile(os.path.join(Tgt, "ChisaParfaitPosition.buf"), dtype = np.uint8).view(np.float32).reshape(-1, 3)
tb = np.fromfile(os.path.join(Tgt, "ChisaParfaitBlend.buf"), dtype = np.uint8)
tw = tb.size // tp.shape[0] // 2
tRows = tb.reshape(tp.shape[0], 2 * tw)
tIds, tWts = tRows[:, :tw].astype(int), tRows[:, tw:].astype(int)

centroid = {}
for bone in np.unique(tIds[tWts > 0]):
    hit = ((tIds == bone) & (tWts > 0)).any(axis = 1)
    if (hit.sum() >= 5):
        centroid[int(bone)] = tp[hit].mean(axis = 0)
print(f"target bones with a centroid: {len(centroid)}\n")

scale = float(np.linalg.norm(tp.max(axis = 0) - tp.min(axis = 0)))
print(f"the target's bounding-box diagonal: {scale:.1f} units (distances below are in the same units)\n")


def measure(label, folder):
    meshes = []
    for root, _d, names in os.walk(folder):
        if ("Position.buf" in names):
            blend = next((n for n in names if (n.lower().endswith("remapblend.buf"))), None)
            if (blend):
                meshes.append((os.path.join(root, "Position.buf"), os.path.join(root, blend)))
    if (not meshes):
        print(f"{label:12s} no remapped blend found under {folder}")
        return
    for pPath, bPath in meshes:
        P = np.fromfile(pPath, dtype = np.uint8).view(np.float32).reshape(-1, 3)
        raw = np.fromfile(bPath, dtype = np.uint8)
        stride = raw.size // P.shape[0]
        if (stride not in (8, 16)):
            print(f"{label:12s} blend stride {stride} for {P.shape[0]} vertices -- skipped")
            continue
        w = stride // 2
        rows = raw[:P.shape[0] * stride].reshape(P.shape[0], stride)
        ids, wts = rows[:, :w].astype(int), rows[:, w:].astype(int)

        dom = ids[np.arange(P.shape[0]), wts.argmax(axis = 1)]
        known = np.array([b in centroid for b in dom])
        cent = np.array([centroid.get(int(b), (np.nan,) * 3) for b in dom])
        d = np.linalg.norm(P - cent, axis = 1)
        good = known & np.isfinite(d)
        pct = 100.0 * (~known).sum() / len(dom)
        print(f"{label:12s} {P.shape[0]:>7} verts  stride {stride}   "
              f"median {np.median(d[good]):6.2f}   mean {d[good].mean():6.2f}   "
              f"90th {np.percentile(d[good], 90):6.2f}   "
              f"beyond 1/4 diagonal: {100.0 * (d[good] > scale / 4).mean():5.1f}%   "
              f"bone unknown to the target: {pct:.1f}%")


print(f"{'mod':12s} {'vertices':>13}            distance from each vertex to its dominant target bone's centroid")
measure("Chisa16", os.path.join(WWMI, "Chisa16"))
measure("Chisa17", os.path.join(WWMI, "Mods", "Chisa17"))
measure("Identity", os.path.join(WWMI, "ChisaIdentity"))

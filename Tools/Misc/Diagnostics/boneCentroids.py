#
# ===== boneCentroids =====
#
# Where every vertex group of a character sits: the blend-weighted centroid, total weight and the
# number of vertices it dominates, per group -- read straight out of a raw 3dmigoto frame analysis
# (the game's own skin), a dump folder in the GI-Model-Importer-Assets layout, or a mod's raw files,
# through Tools/VGRemapFinder's own readers.
#
#   python boneCentroids.py <FrameAnalysis folder> --hashes <position> <blend> <ib> [--component Body]
#   python boneCentroids.py <PlayerCharacterData/Name>            a dump folder (hash.json names the components)
#   python boneCentroids.py <mod folder>                          a mod's *Position.buf / *Blend.buf / *.ib
#   ... --near X Y Z --radius 0.15                                only the groups whose centroid is within the radius
#   ... --groups 11 63 64 68                                      only those groups
#
# Why: when a mod rides on vertex groups the target has no counterpart for (a cape on Yelan's
# jacket chains), the shipped row's entries for them are the finder's per-bone nearest guesses that
# no run ever exercised. The fix is to read the SOURCE groups' centroids off the mod
# (modTally.py --centroids) and the TARGET's off its frame analysis (this script), and pick a
# symmetric, rigid anchor -- one chain root landed on the target's upper arm once and the cape hung
# crooked (2026-09-12).
#

import argparse
import os
import sys

import numpy as np

OnWindows = (sys.platform == "win32")


def winToPosix(path: str) -> str:
    if (OnWindows or (len(path) < 2) or (path[1] != ":") or (not path[0].isalpha())):
        return path
    return "/mnt/" + path[0].lower() + path[2:].replace("\\", "/")


Here = os.path.dirname(os.path.abspath(__file__))
Repo = os.environ.get("AG_REMAP_REPO") or os.path.abspath(os.path.join(Here, "..", "..", ".."))
Finder = os.path.join(Repo, "Tools", "VGRemapFinder")
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
for p in (APISrc, Finder):
    if (not os.path.isdir(p)):
        raise SystemExit(f"not found: {p}; set AG_REMAP_REPO")
    sys.path.insert(0, p)
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))

from src.VGRemapFinder.DumpMod import DumpMod     # noqa: E402


def centroids(dump: DumpMod):
    pos, idx, w = dump.positions, dump.blendIndices, dump.blendWeights
    n = pos.shape[0]
    pts = np.repeat(pos, idx.shape[1], axis = 0).reshape(n, idx.shape[1], 3)
    rows = []
    for g in range(int(idx.max()) + 1):
        m = (idx == g) & (w > 0)
        ww = w[m]
        if (ww.sum() == 0):
            continue
        c = (pts[m] * ww[:, None]).sum(0) / ww.sum()
        rows.append((g, float(ww.sum()), int(((idx == g) & (w >= 0.5)).any(1).sum()), c))
    return rows


def main():
    parser = argparse.ArgumentParser(description = "per-vertex-group centroids of a character or mod")
    parser.add_argument("folder", help = "a frame analysis, a dump folder, or a mod folder")
    parser.add_argument("--hashes", nargs = 3, metavar = ("POSITION", "BLEND", "IB"), default = None, help = "for a frame analysis: the component's position / blend / ib hashes")
    parser.add_argument("--component", default = "", help = "the component name, for a frame analysis or to pick one of a dump folder's")
    parser.add_argument("--name", default = None, help = "the character name (default: from the folder)")
    parser.add_argument("--near", nargs = 3, type = float, metavar = ("X", "Y", "Z"), default = None, help = "only groups whose centroid is within --radius of this point")
    parser.add_argument("--radius", type = float, default = 0.15)
    parser.add_argument("--groups", nargs = "*", type = int, default = None, help = "only these groups")
    args = parser.parse_args()
    folder = winToPosix(args.folder)

    if (args.hashes):
        dumps = {args.component: DumpMod.fromFrameAnalysis(folder, *args.hashes, name = args.name, silent = True, component = args.component)}
    else:
        character = DumpMod.fromFolder(folder, name = args.name, silent = True)
        dumps = character.components if hasattr(character, "components") else {"": character}
        if (args.component):
            dumps = {args.component: dumps[args.component]}

    for comp, dump in dumps.items():
        pos = dump.positions
        print(f"== {dump.name}{(' ' + comp) if comp else ''}: {pos.shape[0]} vertices, y {pos[:, 1].min():.3f}..{pos[:, 1].max():.3f}, z {pos[:, 2].min():+.3f}..{pos[:, 2].max():+.3f}")
        for g, weight, dominant, c in centroids(dump):
            if (args.groups is not None and g not in args.groups):
                continue
            if (args.near is not None and np.linalg.norm(c - np.array(args.near)) > args.radius):
                continue
            print(f"  {g:4d}: weight {weight:8.1f}  dominant on {dominant:5d} verts  centroid x {c[0]:+.3f} y {c[1]:.3f} z {c[2]:+.3f}")


if (__name__ == "__main__"):
    main()

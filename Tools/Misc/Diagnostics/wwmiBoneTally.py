#
# ===== wwmiBoneTally =====
#
# A per-bone tally of a WWMI vertex group remap, read off two WWMI mods' buffers: for every vertex
# group the SOURCE mod's Blend.buf uses, where its vertices sit (the weighted centroid over the mod's
# Position.buf, in the model's own units -- centimetres, for a WuWa character), which TARGET bone the
# remap sends it to, where THAT bone's vertices sit on the target mod, and the distance between the
# two -- plus every chain of consecutive source groups and the targets it lands on. The WuWa form of
# `modTally.py --remap` (see Creating Remaps' "The Yelan lessons", point 2 and point 5).
#
#   python wwmiBoneTally.py <source WWMI mod> <target WWMI mod> --library Sanhua SanhuaExorcist
#   python wwmiBoneTally.py <source WWMI mod> <target WWMI mod> --json remap.json
#   python wwmiBoneTally.py <source WWMI mod> <target WWMI mod> --sheet Data/RemapDrafts/SanhuaRemapDraft.xlsx "2.5 -Sanhua to SanhuaExorcist"
#
# The two mods are ordinary WWMI mod folders (Meshes/Position.buf as R32G32B32_FLOAT, Meshes/Blend.buf
# as four R8 indices + four R8 weights): the identity mods wwmiIdentityMod.py builds are the natural
# pair, since they use every bone. `--library` reads the remap the API ships (needs the API's Python,
# `py -3` here); `--sheet` reads a draft workbook (needs openpyxl: `py -3.11` here); `--json` reads a
# {source group: target group} table, which is also what sanhuaExorcistFix.py's --vgRemap takes.
#
# What to read off it (the first in-game run of Sanhua -> SanhuaExorcist, 2026-09-19): a chain whose
# members land on bones from DIFFERENT target chains, or on a bone several centimetres from where the
# source group's vertices sit, crumples in game once that chain bends -- Sanhua's back ribbons and
# belt tassel, at 5-20 cm, curled at her hands while the finger bones (distance 0.0) and the skirt
# chains (mapped chain to chain, in order) were fine. The remedy is to pin such a chain to its root
# (the prototype's --anchor), not to search for a nearer bone.
#

import argparse
import json
import os
import sys

import numpy as np


def loadGroups(mod):
    """{group: (centroid, vertex count, min, max)} over every group the mod's blend uses with a weight"""
    pos = np.fromfile(os.path.join(mod, "Meshes", "Position.buf"), dtype = np.float32).reshape(-1, 3)
    blend = np.fromfile(os.path.join(mod, "Meshes", "Blend.buf"), dtype = np.uint8).reshape(-1, 8)
    if (len(blend) != len(pos)):
        raise SystemExit(f"{mod}: Position.buf holds {len(pos)} vertices and Blend.buf {len(blend)}")
    idx, w = blend[:, :4].astype(int), blend[:, 4:].astype(np.float64) / 255.0
    groups = {}
    for slot in range(4):
        for g in np.unique(idx[:, slot]):
            m = (idx[:, slot] == g) & (w[:, slot] > 0)
            if (m.any()):
                d = groups.setdefault(int(g), [np.zeros(3), 0.0, 0, np.full(3, np.inf), np.full(3, -np.inf)])
                d[0] += (pos[m] * w[m, slot, None]).sum(0)
                d[1] += w[m, slot].sum()
                d[2] += int(m.sum())
                d[3] = np.minimum(d[3], pos[m].min(0))
                d[4] = np.maximum(d[4], pos[m].max(0))
    return {g: (c / ws, n, lo, hi) for g, (c, ws, n, lo, hi) in groups.items()}, pos


def loadRemap(args):
    if (args.json):
        with open(args.json, "r", encoding = "utf-8") as f:
            return {int(k): int(v) for k, v in json.load(f).items()}, {}, f"--json {args.json}"
    if (args.sheet):
        import openpyxl
        wb = openpyxl.load_workbook(args.sheet[0], data_only = True)
        rows = [r for r in wb[args.sheet[1]].iter_rows(min_row = 2, values_only = True) if r[0] is not None]
        return ({int(r[0]): int(r[1]) for r in rows if r[1] is not None},
                {int(r[0]): str(r[3] or "") for r in rows if len(r) > 3}, f"sheet {args.sheet[1]!r}")
    if (args.library):
        repo = os.environ.get("AG_REMAP_REPO") or os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
        apiSrc = os.path.join(repo, "Anime Game Remap (for all users)", "api", "src", "py")
        sys.path.insert(0, apiSrc)
        if (hasattr(os, "add_dll_directory")):
            os.add_dll_directory(os.path.join(apiSrc, "FixRaidenBoss2"))
        import FixRaidenBoss2 as FRB
        modType = next((m for m in FRB.CppGlobalModTypes.all() if m.name == args.library[0]), None)
        if (modType is None):
            raise SystemExit(f"the library has no mod type named {args.library[0]}")
        row = modType.getVGRemap(args.library[1])
        if (row is None):
            raise SystemExit(f"the library has no vertex group remap {args.library[0]} -> {args.library[1]}")
        return {int(k): int(v) for k, v in dict(row.remap).items()}, {}, f"the library's {args.library[0]} -> {args.library[1]} row"
    raise SystemExit("give one of --library, --json or --sheet")


def main():
    parser = argparse.ArgumentParser(description = "per-bone tally of a WWMI vertex group remap over two WWMI mods' buffers")
    parser.add_argument("source", help = "the WWMI mod folder of the character being remapped (its identity mod, ideally)")
    parser.add_argument("target", help = "the WWMI mod folder of the character remapped onto")
    parser.add_argument("--library", nargs = 2, metavar = ("FROM", "TO"), help = "read the remap the API ships for this pair (by mod type name)")
    parser.add_argument("--json", help = "read the remap from a {source group: target group} json file")
    parser.add_argument("--sheet", nargs = 2, metavar = ("XLSX", "SHEET"), help = "read the remap from a draft workbook's sheet")
    parser.add_argument("--far", type = float, default = 3.0, help = "list every row whose target bone sits more than this far from the source group's vertices (default: %(default)s)")
    args = parser.parse_args()

    src, srcPos = loadGroups(args.source)
    dst, dstPos = loadGroups(args.target)
    remap, comments, label = loadRemap(args)
    print(f"source {args.source}: {len(srcPos)} vertices, {len(src)} groups used, extents {srcPos.min(0).round(1)} .. {srcPos.max(0).round(1)}")
    print(f"target {args.target}: {len(dstPos)} vertices, {len(dst)} groups used, extents {dstPos.min(0).round(1)} .. {dstPos.max(0).round(1)}")
    print(f"remap: {label}, {len(remap)} rows")

    rows = []
    for g in sorted(src):
        c, n, lo, hi = src[g]
        t = remap.get(g)
        if (t is None):
            rows.append((g, n, None, None, c)); continue
        if (t not in dst):
            rows.append((g, n, t, float("nan"), c)); continue
        rows.append((g, n, t, float(np.linalg.norm(c - dst[t][0])), c))

    unmapped = [(g, n) for g, n, t, d, c in rows if t is None]
    if (unmapped):
        print(f"\nUNMAPPED source groups (an unmapped group is written as a NEGATIVE bone index): {unmapped}")
    empty = [(g, t) for g, n, t, d, c in rows if (t is not None and d != d)]
    if (empty):
        print(f"\ntarget bones no vertex of the target mod uses (a duplicate slot of the merged skeleton, or a wrong row): {empty}")

    far = sorted([r for r in rows if (r[3] is not None and r[3] == r[3] and r[3] > args.far)], key = lambda r: -r[1] * r[3])
    print(f"\n{len(far)} rows whose target bone sits more than {args.far} from the source group's vertices, by vertices * distance:")
    print(f"{'src':>4} {'verts':>6}   {'dst':>4} {'dist':>6}   source centroid            comment")
    for g, n, t, d, c in far:
        print(f"{g:>4} {n:>6} -> {t:>4} {d:>6.2f}   {c[0]:>7.1f} {c[1]:>7.1f} {c[2]:>7.1f}   {comments.get(g, '')[:60]}")

    print("\nchains (runs of consecutive source groups, 3 or more) and the targets each lands on:")
    gs = sorted(src)
    i = 0
    while (i < len(gs)):
        j = i
        while (j + 1 < len(gs) and gs[j + 1] == gs[j] + 1):
            j += 1
        if (j - i + 1 >= 3):
            print(f"  {gs[i]}..{gs[j]} -> {[remap.get(g) for g in gs[i:j + 1]]}")
        i = j + 1


if (__name__ == "__main__"):
    main()

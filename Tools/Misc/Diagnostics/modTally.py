#
# ===== modTally =====
#
# The first thing to run over a GIMI mod folder before believing anything about it: per object, what
# the geometry is, which vertex groups it rides on, and which lightmap band its pixels sit on -- read
# at the object's own vertices, with the diffuse colour under each band, which is how a band is told
# to be skin, hair, fur or cloth for THIS author (a mod's legend is the author's, not the skin's).
#
#   python modTally.py <mod folder>                                 the tally
#   python modTally.py <mod folder> --against <other mod folder>    ... plus the vertex groups one uses and the other does not
#   python modTally.py <mod folder> --remap Yelan YelanTranquil Body  ... plus the groups with NO entry in the API's shipped
#                                                                   remap row (source component "" -> target component)
#   python modTally.py <mod folder> --centroids 0 1 2 3             ... plus the weighted centroid of those groups per object
#
# What it found the day it was written (2026-09-12, as three one-off scripts): a cape rigged to
# vertex groups the reference mod never used (the "--against" list), grey stockings painted on a
# band the target shades as skin (the band tally), a port whose hair sat on the source's skin band
# (the diffuse mean under the band), and the real legend of the skin itself (run it over the
# identity mod, Tools/Misc/Prototypes/identityMod.py).
#
# Needs the API (textures are BC7): the repo root is found from this file's location, or set
# AG_REMAP_REPO. Runs on Windows or Linux / WSL; a Windows-form path is translated on Linux.
#

import argparse
import glob
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
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
if (not os.path.isdir(APISrc)):
    raise SystemExit(f"the API's package folder is not at {APISrc}; set AG_REMAP_REPO")
sys.path.insert(0, APISrc)
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))

import FixRaidenBoss2 as FRB     # noqa: E402

Bands = [(0, 0), (1, 49), (50, 99), (100, 114), (115, 127), (128, 139), (140, 159), (160, 199), (200, 239), (240, 254), (255, 255)]


class Mod:
    """A GIMI mod's raw files, found by name: <prefix>Position.buf / Blend.buf / Texcoord.buf, <prefix><Obj>.ib, <prefix><Obj>Diffuse / LightMap.dds"""

    def __init__(self, folder: str):
        self.folder = folder
        positions = [p for p in glob.glob(os.path.join(folder, "*Position.buf")) if ("Remap" not in os.path.basename(p))]
        if (not positions):
            raise SystemExit(f"no *Position.buf in {folder}")
        self.prefix = os.path.basename(positions[0])[:-len("Position.buf")]
        self.name = self.prefix
        raw = np.fromfile(positions[0], dtype = np.uint8)
        blend = np.fromfile(os.path.join(folder, self.prefix + "Blend.buf"), dtype = np.uint8)
        self.n = blend.size // 32
        self.positionStride = raw.size // self.n
        self.pos = raw.reshape(self.n, self.positionStride)[:, :12].copy().view(np.float32)
        self.weights = blend.reshape(self.n, 32)[:, :16].copy().view(np.float32)
        self.indices = blend.reshape(self.n, 32)[:, 16:].copy().view(np.int32)
        tcPath = os.path.join(folder, self.prefix + "Texcoord.buf")
        self.uv = None
        self.colour = None
        if (os.path.exists(tcPath)):
            tc = np.fromfile(tcPath, dtype = np.uint8)
            self.texcoordStride = tc.size // self.n
            tc = tc.reshape(self.n, self.texcoordStride)
            self.colour = tc[:, :4]
            self.uv = tc[:, 4:12].copy().view(np.float32)
        self.objects = {}
        for ib in sorted(glob.glob(os.path.join(folder, self.prefix + "*.ib"))):
            stem = os.path.basename(ib)[len(self.prefix):-3]
            if ("Remap" in stem or not stem):
                continue
            data = np.fromfile(ib, dtype = np.uint32)
            self.objects[stem] = data
        order = {"Head": 0, "Body": 1, "Dress": 2, "Extra": 3}
        self.objects = dict(sorted(self.objects.items(), key = lambda kv: (order.get(kv[0], 9), kv[0])))

    def texture(self, obj: str, kind: str):
        path = os.path.join(self.folder, f"{self.prefix}{obj}{kind}.dds")
        if (not os.path.exists(path)):
            return None
        t = FRB.TextureFile(path, readPillowImg = True)
        t.open()
        return np.asarray(t.img.convert("RGBA")) if t.hasImage else None

    def sample(self, img, verts):
        h, w = img.shape[:2]
        uv = self.uv[verts]
        x = np.clip((uv[:, 0] % 1.0 * w).astype(int), 0, w - 1)
        y = np.clip((uv[:, 1] % 1.0 * h).astype(int), 0, h - 1)
        return img[y, x]

    def groupsOf(self, verts):
        live = self.weights[verts] > 0
        g, c = np.unique(self.indices[verts][live], return_counts = True)
        return dict(zip(g.tolist(), c.tolist()))


def bandLabel(lo, hi):
    return f"{lo}" if (lo == hi) else f"{lo}-{hi}"


def tally(mod: Mod, centroids):
    print(f"== {mod.name} ({mod.folder}): {mod.n} vertices, position stride {mod.positionStride}, "
          f"y {mod.pos[:, 1].min():.3f}..{mod.pos[:, 1].max():.3f}, groups used {len(mod.groupsOf(np.arange(mod.n)))}, max group {int(mod.indices[mod.weights > 0].max())}")
    if (mod.colour is not None):
        print(f"   vertex colour mean {mod.colour.mean(0).round(1)} (both game models say 128 for G / B)")
    for obj, ib in mod.objects.items():
        if (not ib.size):
            print(f"  {obj}: empty ib (hidden by the mod)")
            continue
        v = np.unique(ib)
        p = mod.pos[v]
        groups = mod.groupsOf(v)
        print(f"  {obj}: {ib.size // 3} triangles, {v.size} vertices, x {p[:, 0].min():+.3f}..{p[:, 0].max():+.3f} y {p[:, 1].min():.3f}..{p[:, 1].max():.3f} z {p[:, 2].min():+.3f}..{p[:, 2].max():+.3f}")
        print(f"     groups ({len(groups)}): " + " ".join(f"{g}:{c}" for g, c in groups.items()))
        if (mod.uv is None):
            continue
        light = mod.texture(obj, "LightMap")
        diffuse = mod.texture(obj, "Diffuse")
        if (light is None or diffuse is None):
            continue
        l = mod.sample(light, v)
        d = mod.sample(diffuse, v)
        rows = []
        for lo, hi in Bands:
            m = (l[:, 3] >= lo) & (l[:, 3] <= hi)
            if (m.sum()):
                rows.append(f"{bandLabel(lo, hi)}: {m.sum()} verts, diffuse {d[m, :3].mean(0).round(0).astype(int).tolist()}")
        print(f"     lightmap alpha bands at its vertices ({light.shape[1]}x{light.shape[0]}): " + "; ".join(rows))
        da = d[:, 3]
        print(f"     diffuse alpha at its vertices: " + " ".join(f"[{bandLabel(lo, hi)}]:{((da >= lo) & (da <= hi)).sum()}" for lo, hi in Bands if ((da >= lo) & (da <= hi)).sum()))
        for g in centroids:
            m = (mod.indices[v] == g) & (mod.weights[v] > 0)
            ww = mod.weights[v][m]
            if (ww.sum() == 0):
                continue
            pts = np.repeat(p, 4, axis = 0).reshape(v.size, 4, 3)[m]
            c = (pts * ww[:, None]).sum(0) / ww.sum()
            print(f"     group {g}: weight {ww.sum():.1f}, centroid x {c[0]:+.3f} y {c[1]:.3f} z {c[2]:+.3f}")


def main():
    parser = argparse.ArgumentParser(description = "per-object geometry, vertex groups and lightmap bands of a GIMI mod")
    parser.add_argument("mod", help = "the mod folder (the one holding *Position.buf)")
    parser.add_argument("--against", default = None, help = "another mod folder of the same character: list the vertex groups each uses that the other does not")
    parser.add_argument("--remap", nargs = 3, metavar = ("FROM", "TO", "TOCOMPONENT"), default = None, help = "the API's shipped remap row to check every used group against (source component '')")
    parser.add_argument("--versions", nargs = 2, default = ["1.0", "5.7"], help = "the row's (from, to) versions for --remap (default: %(default)s)")
    parser.add_argument("--centroids", nargs = "*", type = int, default = [], help = "vertex groups whose weighted centroid to print per object")
    args = parser.parse_args()

    mod = Mod(winToPosix(args.mod))
    tally(mod, args.centroids)
    if (args.against):
        other = Mod(winToPosix(args.against))
        print()
        tally(other, args.centroids)
        print(f"\n== {mod.name} vs {other.name}, per object:")
        for obj in set(mod.objects) | set(other.objects):
            a = mod.groupsOf(np.unique(mod.objects[obj])) if (obj in mod.objects and mod.objects[obj].size) else {}
            b = other.groupsOf(np.unique(other.objects[obj])) if (obj in other.objects and other.objects[obj].size) else {}
            print(f"  {obj}: groups only the first uses: {sorted(set(a) - set(b))}; only the second uses: {sorted(set(b) - set(a))}")
    if (args.remap):
        src, dst, comp = args.remap
        table = FRB.CppGlobalModTypes.all()[0].vgRemaps
        row = dict(table.get([src, "", dst, comp], list(args.versions)).remap)
        used = set(mod.groupsOf(np.arange(mod.n)))
        missing = sorted(g for g in used if (g not in row))
        print(f"\n== remap {src} -> {dst} {comp} ({len(row)} entries): used groups with NO entry: {missing if missing else 'none'}")
        byTarget = {}
        for g in sorted(used):
            byTarget.setdefault(row.get(g), []).append(g)
        shared = {t: gs for t, gs in byTarget.items() if (t is not None and len(gs) > 1)}
        if (shared):
            print(f"   targets several used groups share (fine for chains the target lacks, suspicious for limbs): {shared}")


if (__name__ == "__main__"):
    main()

#
# ===== chisaHashHistory =====
#
# Chisa's texture hash HISTORY, built from the mods themselves. WuWa rehashes a texture between game
# versions and a mod carries whatever hash its author dumped, so a fix that knows only today's hashes
# cannot say what a mod's `Components-3 t=2970cef1.dds` is -- and guessing it from the picture is
# fragile (a qipao's saturated diffuse read as a normal map; a pale or tanned skin read as cloth).
# There is no hash history for Chisa anywhere (WWMI-Assets has no Chisa; the community hash maps stop
# at game version 2.3), so this derives one, keeping a hash ONLY on hash-level evidence:
#
#   * a mod ships it UNMODIFIED: its pixels are identical to Chisa's CURRENT texture of that role
#     (correlation >= IdentityMin with it, < IdentityGap with every other -- the prototype's own
#     thresholds), and the role's component is in the file's `Components-<list>`, which WWMI's
#     exporter writes from where the game used the texture. Repaints under a hash are no evidence
#     either way; they simply do not vote.
#   * a mod ships a FRAME-DUMP file naming it (`000134-ps-t2=2970cef1-...`): the register it was bound to.
#   * it is byte-identical to a hash so identified, inside the same component list.
#
# A hash whose evidence disagrees is reported and not filed. The output is the evidence file
# Data/Mod Downloads/WuWa/Chisa/2_8/ChisaHashLineage.json, and its rows go into core's HashData.cpp
# (Chisa's texture rows, typed by role: current at 3.6, older at 3.0).
#
#   py -3 chisaHashHistory.py <WWMI folder holding the Chisa mods> [--out <json>]
#
# Mods are the folders named Chisa* directly under the WWMI folder and under its Mods/ (the maintainer
# moves them between the two); ChisaParfait* and *Identity* folders are skipped.
#

import argparse
import collections
import glob
import hashlib
import json
import os
import re
import sys

import numpy as np

Here = os.path.dirname(os.path.abspath(__file__))
Repo = os.path.normpath(os.path.join(Here, "..", "..", ".."))
sys.path.insert(0, os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py"))
sys.path.insert(0, os.path.join(Repo, "Tools", "Misc", "Prototypes"))
import FixRaidenBoss2 as FRB                                   # noqa: E402
from PIL import Image                                          # noqa: E402
import chisaParfaitFix as C                                    # noqa: E402

Downloads = os.path.join(Repo, "Data", "Mod Downloads", "WuWa", "Chisa", "2_8")
_cache = {}


def rgb(path):
    """A texture's RGB at 128 x 128, decoded by the library (not through a PNG: that blanks zero-alpha maps)"""
    if (path not in _cache):
        try:
            tex = FRB.TextureFile(path)
            tex.open()
            px = np.frombuffer(tex.getPixels(), dtype = np.uint8).reshape(tex.height, tex.width, 4)[..., :3]
            _cache[path] = np.asarray(Image.fromarray(px).resize((128, 128), Image.BOX)).astype(np.float64)
        except Exception:
            _cache[path] = None
    return _cache[path]


def corr(x, y):
    x, y = x - x.mean(), y - y.mean()
    norm = np.linalg.norm(x) * np.linalg.norm(y)
    return float((x * y).sum() / norm) if norm else 0.0


def md5(path):
    with open(path, "rb") as f:
        return hashlib.md5(f.read()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description = "Chisa's texture hash history, from the mods")
    parser.add_argument("wwmi", help = "the WWMI folder holding the Chisa mods (and its Mods/)")
    parser.add_argument("--out", default = os.path.join(Downloads, "ChisaHashLineage.json"), help = "the evidence file to write")
    args = parser.parse_args()

    refs = {h: rgb(os.path.join(Downloads, f"ChisaTexture{h}.dds")) for h in C.Roles}
    refs = {h: v for h, v in refs.items() if (v is not None)}
    mods = sorted({os.path.normpath(d) for d in glob.glob(os.path.join(args.wwmi, "Chisa*")) + glob.glob(os.path.join(args.wwmi, "Mods", "Chisa*"))
                   if (os.path.isdir(d) and "parfait" not in os.path.basename(d).lower() and "identity" not in os.path.basename(d).lower())})
    print("mods:", [os.path.basename(m) for m in mods])

    identified = collections.defaultdict(list)
    files = []
    for mod in mods:
        for path in glob.glob(os.path.join(mod, "**", "*.dds"), recursive = True):
            if ("Remap" in path or "DISABLED" in path.upper()):
                continue
            base = os.path.basename(path)
            dump = re.match(r"^\d{6}-ps-t(\d+)=([0-9a-fA-F]{8})", base)
            named = re.search(r"Components-([0-9-]+) t=([0-9a-fA-F]{8})\.dds$", base)
            if (dump):
                # a frame-dump file names the register; the prototype's own map says what that register is
                identified[dump.group(2).lower()].append((None, f"dump ps-t{dump.group(1)}", os.path.basename(mod), "dump"))
                continue
            if (not named):
                continue
            h, comps = named.group(2).lower(), named.group(1)
            files.append((path, h, comps))
            if (h in C.Roles):
                continue
            x = rgb(path)
            if (x is None):
                continue
            allowed = [r for r in refs if (str(C.RoleComponent.get(C.Roles[r])) in comps.split("-"))]
            scores = sorted(((corr(x, refs[r]), r) for r in allowed), reverse = True)
            if (scores and scores[0][0] >= C.IdentityMin and (len(scores) == 1 or scores[1][0] < C.IdentityGap)):
                identified[h].append((C.Roles[scores[0][1]], f"{scores[0][0]:.3f}", os.path.basename(mod), comps))

    # A dump file's register -> role, by the pass it was dumped from. Only the old UPPER-body pass has
    #   been seen in a mod (Chisa5's draw 134, ps dce450ef), and it binds its normal at ps-t0 and its
    #   diffuse at ps-t2 exactly as her current upper pass (42721e1d) does.
    DumpRegisterRoles = {"ps-t0": "upperNormal", "ps-t2": "upperDiffuse"}
    for h, ev in identified.items():
        identified[h] = [((DumpRegisterRoles.get(e[1].split()[-1]) if (e[0] is None) else e[0]),) + tuple(e[1:]) for e in ev]
        identified[h] = [e for e in identified[h] if (e[0] is not None)]

    history, conflicts = {}, {}
    for h, ev in identified.items():
        if (ev):
            (history if (len({e[0] for e in ev}) == 1) else conflicts)[h] = ev
    byMd5 = collections.defaultdict(set)
    for path, h, comps in files:
        byMd5[md5(path)].add((h, comps))
    inherited = {}
    for path, h, comps in files:
        if (h in history or h in C.Roles):
            role = history[h][0][0] if (h in history) else C.Roles[h]
            for other, otherComps in byMd5[md5(path)]:
                if (other != h and other not in C.Roles and other not in history and otherComps == comps):
                    inherited.setdefault(other, (role, h))

    older = {h: {"role": ev[0][0], "evidence": sorted({f"{e[2]}:{e[1]}" for e in ev})} for h, ev in history.items()}
    older.update({h: {"role": role, "evidence": [f"byte-identical to {via}"]} for h, (role, via) in inherited.items()})
    for h, v in sorted(older.items(), key = lambda kv: kv[1]["role"]):
        print(f"  {h} -> {v['role']:18s} {', '.join(v['evidence'])}")
    if (conflicts):
        print("NOT FILED, evidence disagrees:", conflicts)
    with open(args.out, "w", encoding = "utf-8") as f:
        json.dump({"_about": "Chisa's texture hashes by role. current: the download folder's (game 3.6). older: hashes seen in mods, "
                             "each with the evidence that fixes its role -- <mod>:<correlation with the current texture>, "
                             "<mod>:dump ps-tN, or byte-identical to another identified hash. Generated by "
                             "Tools/Misc/Diagnostics/chisaHashHistory.py; filed in core's HashData.cpp at 3.6 and 3.0.",
                   "current": dict(C.Roles), "older": older}, f, indent = 1, sort_keys = True)
    print(f"{len(older)} older hashes -> {args.out}")


if (__name__ == "__main__"):
    main()

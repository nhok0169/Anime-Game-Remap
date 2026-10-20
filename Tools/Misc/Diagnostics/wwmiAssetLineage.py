#
# ===== wwmiAssetLineage =====
#
# A WuWa character's texture hash LINEAGE out of WWMI-Assets' own git history. WuWa rehashes a texture
# between game versions and a mod carries whatever hash its author dumped, so a fix that knows only today's
# hashes downloads the GAME's texture over a mod's own art for every older export (Changli1's whole outfit,
# 2026-10-20). WWMI-Assets keeps each character's textures as `Components-<list> t=<hash>.dds`, so when a
# version rehashes a texture git records a RENAME, old hash -> new hash, with its similarity. For each one
# this prints:
#
#   * the new hash's role in the character's fixer config (`<Char>Textures.cpp`'s `facts.roles`, read as
#     text), and failing that through the live-hash file's twins;
#   * the OLD hash's register usage in the TextureUsage.json of the commit before the rename -- the check
#     that the role is right: it should be the register the successor's role reads today.
#
# It is hash-level evidence, unlike correlating pixels (Tools/Misc/Diagnostics/wwmiHashHistory.py), and it
# covers only versions WWMI-Assets carried the character at. A hash with no successor, or whose successor
# has no role, is reported and is the caller's to judge. The result goes into HashData.cpp as rows typed by
# role at an older version (Changli's 2.5 / 2.6 rows), with the --out JSON as the evidence file.
#
#   py -3 wwmiAssetLineage.py --char Changli [--assets <WWMI-Assets checkout>] [--out <json>]
#

import argparse
import json
import os
import re
import subprocess

Repo = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
DefaultAssets = os.path.abspath(os.path.join(Repo, "..", "WuWa", "WWMI-Assets"))


def git(assets, *args):
    return subprocess.run(["git", "-C", assets, *args], capture_output = True, text = True, encoding = "utf-8").stdout


def configRoles(char):
    path = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "cpp", "core", "src", "data",
                        "IniFixData", char, char + "Textures.cpp")
    with open(path, encoding = "utf-8") as f:
        text = f.read()
    return dict(re.findall(r'\{"([0-9a-f]{8})", "(\w+)"\}', text.split("facts.registerRoles")[0]))


def liveTwins(char):
    path = os.path.join(Repo, "Data", "Mod Downloads", "WuWa", char, char + "LiveHashes.json")
    if (not os.path.isfile(path)):
        return {}
    with open(path, encoding = "utf-8") as f:
        data = json.load(f)
    return {k: v for k, v in data.items() if isinstance(v, str) and re.fullmatch(r"[0-9a-f]{8}", k)}


def usage(assets, folder, commit, h):
    try:
        u = json.loads(git(assets, "show", f"{commit}^:{folder}/TextureUsage.json"))
    except ValueError:
        return []
    out = []
    for comp, regs in u.items():
        for reg, uses in regs.items():
            out += [f"{comp.split()[-1]}:{reg}:{use.split('ps=')[1][:8]}" for use in uses if use.startswith(h)]
    return out


def main():
    parser = argparse.ArgumentParser(description = "A WuWa character's texture hash lineage from WWMI-Assets' git history")
    parser.add_argument("--char", required = True, help = "the character's folder name in WWMI-Assets and in IniFixData")
    parser.add_argument("--assets", default = DefaultAssets, help = "the WWMI-Assets checkout")
    parser.add_argument("--out", default = None, help = "write the lineage as JSON here")
    args = parser.parse_args()

    folder = f"PlayerCharacterData/{args.char}"
    roles = configRoles(args.char)
    twins = liveTwins(args.char)

    def roleOf(h):
        return roles.get(h) or roles.get(twins.get(h, ""))

    commit = None
    result = {}
    for line in git(args.assets, "log", "-M", "--name-status", "--format=== %h %s", "--", folder).splitlines():
        if (line.startswith("== ")):
            commit = line.split()[1]
            continue
        parts = line.split("\t")
        if (not parts[0].startswith("R") or not parts[1].endswith(".dds")):
            continue
        old = re.search(r"t=([0-9a-f]{8})", parts[1]).group(1)
        new = re.search(r"t=([0-9a-f]{8})", parts[2]).group(1)
        if (old == new):
            continue
        comps = re.search(r"Components-([\d-]+)", parts[1]).group(1)
        entry = {"new": new, "similarity": parts[0], "commit": commit, "components": comps,
                 "role": roleOf(new), "oldUsage": usage(args.assets, folder, commit, old)}
        result[old] = entry
        print(f"{commit} {parts[0]} {old} -> {new}  components {comps:16s} role {entry['role']!s:14s} "
              f"old usage {entry['oldUsage']}")

    if (args.out):
        with open(args.out, "w", encoding = "utf-8") as f:
            json.dump(result, f, indent = 4)


if (__name__ == "__main__"):
    main()

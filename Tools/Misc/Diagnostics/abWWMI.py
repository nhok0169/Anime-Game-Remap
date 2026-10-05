"""A/B a WuWa mod through the PROTOTYPE and through the COMPILED tables, then diff every file.

    py -3 abWWMI.py <mod folder> [--proto <prototype script>] [--scratch <folder>]

Two scratch copies of the mod are made, both are undone with the prototype first (so a fix already in the
folder, whichever spelling wrote its copies, is taken out), then one is fixed by the prototype and the other
by RemapService over the compiled tables alone. Buffers and textures are compared by md5, .ini files line
by line without their carriage returns; the compiled fixer's own log is printed. This is the WuWa
counterpart of runCompiled.py + diff, and what proved makeWWMIFixer against sanhuaExorcistFix.py on four
mods (2026-09-19): every remapped section identical, every RemapBlend byte-identical, the copies differing
only in shape (the prototype's carry the whole file, the API's the mod's text plus their own group).
Set AG_REMAP_REPO for another checkout. Windows paths; the API's Python (`py -3` here)."""
import argparse
import difflib
import hashlib
import os
import shutil
import subprocess
import sys

Repo = os.environ.get("AG_REMAP_REPO") or os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
sys.path.insert(0, os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py"))
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py", "FixRaidenBoss2"))


def runCompiled(folder):
    import FixRaidenBoss2 as FRB
    FRB.CppStrategyOverrides.clear()
    service = FRB.RemapService(path = folder, keepBackups = False, logger = FRB.Logger())
    service.fix()
    stats = service.stats
    print(f"  compiled: .ini fixed {len(stats.ini.fixed)}, skipped {len(stats.ini.skipped)}")
    for path, error in stats.ini.skipped.items():
        print(f"    SKIPPED {os.path.relpath(path, folder)}: {error}")
    for label in ("blend", "position", "texcoord", "buf", "texEdit", "texAdd"):
        s = getattr(stats, label, None)
        if (s is not None and (s.fixed or s.skipped)):
            print(f"  compiled {label}: fixed {len(s.fixed)}, skipped {len(s.skipped)}")
            for path, error in s.skipped.items():
                print(f"    SKIPPED {os.path.relpath(path, folder)}: {error}")


def md5(path):
    with open(path, "rb") as f:
        return hashlib.md5(f.read()).hexdigest()


def iniLines(path):
    with open(path, "r", encoding = "utf-8", errors = "replace", newline = "") as f:
        return [line.rstrip("\r") for line in f.read().split("\n")]


def main():
    parser = argparse.ArgumentParser(description = "A/B a WuWa mod: the prototype against the compiled tables")
    parser.add_argument("mod", help = "the mod folder")
    parser.add_argument("--proto", default = os.path.join(Repo, "Tools", "Misc", "Prototypes", "sanhuaExorcistFix.py"),
                        help = "the prototype script (default: the Sanhua one)")
    parser.add_argument("--scratch", default = os.path.join(os.environ.get("TEMP", "."), "agr_ab"), help = "where the two copies go")
    args = parser.parse_args()

    src = os.path.abspath(args.mod)
    name = os.path.basename(src.rstrip("\\/"))
    a = os.path.join(args.scratch, name + "_proto")
    b = os.path.join(args.scratch, name + "_compiled")
    for d in (a, b):
        if (os.path.isdir(d)):
            shutil.rmtree(d)
        shutil.copytree(src, d)
        subprocess.run([sys.executable, args.proto, d, "--undo"], capture_output = True, text = True)

    print("#### prototype")
    r = subprocess.run([sys.executable, args.proto, a], capture_output = True, text = True)
    print("\n".join("  " + line for line in (r.stdout + r.stderr).splitlines() if line.strip()))
    print("#### compiled")
    runCompiled(b)

    print("#### diff")
    filesA = {os.path.relpath(os.path.join(root, f), a) for root, _, names in os.walk(a) for f in names}
    filesB = {os.path.relpath(os.path.join(root, f), b) for root, _, names in os.walk(b) for f in names}
    # The two sides place and name some fix-written files differently -- a download's folder, and
    # whether an edited texture is named after the EDIT or after the source resource -- and neither
    # changes what the game loads, since the .ini names the file either way. Paired by exact path
    # those show up as two unpaired files each, which on one mod was 46 of 61 and buried the
    # differences that mattered. So pair the leftovers by CONTENT first.
    #
    # .ini files are not paired this way (two of them legitimately differ), nor are empty ones
    # (every empty file has the same hash).
    def pairable(root, rel):
        return not rel.lower().endswith(".ini") and os.path.getsize(os.path.join(root, rel)) > 0

    byHash = {}
    for f in sorted(filesA - filesB):
        if (pairable(a, f)):
            byHash.setdefault(md5(os.path.join(a, f)), []).append(f)

    moved, onlyB = [], []
    for f in sorted(filesB - filesA):
        candidates = byHash.get(md5(os.path.join(b, f))) if (pairable(b, f)) else None
        if (candidates):
            moved.append((candidates.pop(0), f))
        else:
            onlyB.append(f)

    paired = {was for was, _now in moved}
    onlyA = [f for f in sorted(filesA - filesB) if (f not in paired)]
    for was, now in moved:
        print(f"  same bytes, moved/renamed: {was}  ->  {now}")
    for f in onlyA:
        print(f"  only in prototype: {f}")
    for f in onlyB:
        print(f"  only in compiled:  {f}")
    same = differ = 0
    for f in sorted(filesA & filesB):
        pa, pb = os.path.join(a, f), os.path.join(b, f)
        if (f.lower().endswith(".ini")):
            la, lb = iniLines(pa), iniLines(pb)
            if (la == lb):
                same += 1
                continue
            differ += 1
            diff = list(difflib.unified_diff(la, lb, "proto/" + f, "compiled/" + f, lineterm = "", n = 1))
            print(f"  DIFF {f}: {sum(1 for d in diff if d.startswith('+') and not d.startswith('+++'))} added, "
                  f"{sum(1 for d in diff if d.startswith('-') and not d.startswith('---'))} removed lines")
            for line in diff[:80]:
                print("    " + line)
        elif (md5(pa) == md5(pb)):
            same += 1
        else:
            differ += 1
            print(f"  DIFF {f}: {os.path.getsize(pa)} vs {os.path.getsize(pb)} bytes")
    print(f"  {same} identical, {differ} differ, {len(moved)} moved/renamed with the same bytes, "
          f"{len(onlyA)} only prototype, {len(onlyB)} only compiled")


if (__name__ == "__main__"):
    main()

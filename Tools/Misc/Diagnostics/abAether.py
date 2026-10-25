#
# ===== abAether =====
#
# The compiled Aether -> AetherHeaven fix against its prototype (Tools/Misc/Prototypes/aetherHeavenFix.py), per mod:
# two scratch copies of each mod, one fixed by the prototype (its parser / fixers on CppStrategyOverrides) and one by
# the compiled rows (a plain RemapService), compared file for file. Generated file names carry run-specific suffixes
# (Lumine point 9), so the binaries are compared as a multiset of md5s and the .ini files by their text with those
# suffixes normalised. Each side runs in its own process, so neither sees the other's overrides.
#
#   py -3 abAether.py <work folder> <mod folder> [<mod folder> ...] [--download disabled]
#
# Prints SAME / DIFF per mod and exits 1 on any DIFF; a DIFF keeps both copies to read.
#
import argparse
import hashlib
import os
import re
import shutil
import subprocess
import sys

Here = os.path.dirname(os.path.abspath(__file__))
Repo = os.environ.get("AG_REMAP_REPO") or os.path.abspath(os.path.join(Here, "..", "..", ".."))
Prototype = os.path.join(Repo, "Tools", "Misc", "Prototypes", "aetherHeavenFix.py")

Compiled = r'''
import os, sys
api = os.path.join(sys.argv[1], "Anime Game Remap (for all users)", "api", "src", "py")
sys.path.insert(0, api); os.add_dll_directory(os.path.join(api, "FixRaidenBoss2"))
import FixRaidenBoss2 as FRB
kwargs = {"downloadMode": sys.argv[3]} if sys.argv[3] != "default" else {}
s = FRB.RemapService(path = sys.argv[2], keepBackups = False, **kwargs)
s.fix()
print("fixed", len(s.stats.ini.fixed), "skipped", len(s.stats.ini.skipped))
'''


def tree(folder):
    out = {}
    for d, _, fs in os.walk(folder):
        for f in fs:
            p = os.path.join(d, f)
            out[os.path.relpath(p, folder)] = open(p, "rb").read()
    return out


def normIni(data):
    t = data.decode("utf-8", "replace").replace("\r", "")
    return re.sub(r"_[A-Za-z0-9]{2,3}(?=[_.\]\s])", "_X", t).rstrip()


def summary(files):
    bins = sorted(hashlib.md5(v).hexdigest() for k, v in files.items() if not k.lower().endswith(".ini"))
    inis = sorted(normIni(v) for k, v in files.items() if k.lower().endswith(".ini"))
    return bins, inis


def main():
    parser = argparse.ArgumentParser(description = "the compiled Aether -> AetherHeaven fix against its prototype")
    parser.add_argument("work")
    parser.add_argument("mods", nargs = "+")
    parser.add_argument("--download", default = "default")
    parser.add_argument("--protoArg", action = "append", default = [], help = "an extra prototype flag (eg. --noHeadAlpha) -- to prove the A/B can FAIL")
    args = parser.parse_args()
    env = {**os.environ, "AG_REMAP_REPO": Repo, "PYTHONIOENCODING": "utf-8"}
    bad = 0
    for mod in args.mods:
        name = os.path.basename(os.path.normpath(mod))
        a, b = os.path.join(args.work, name + "_proto"), os.path.join(args.work, name + "_compiled")
        for d in (a, b):
            shutil.rmtree(d, ignore_errors = True)
            shutil.copytree(mod, d)
        protoArgs = [sys.executable, Prototype, a] + ([] if args.download == "default" else ["--download", args.download]) + args.protoArg
        ra = subprocess.run(protoArgs, capture_output = True, text = True, env = env, encoding = "utf-8", errors = "replace")
        rb = subprocess.run([sys.executable, "-c", Compiled, Repo, b, args.download], capture_output = True, text = True, env = env, encoding = "utf-8", errors = "replace")
        fa, fb = tree(a), tree(b)
        (ba, ia), (bb, ib) = summary(fa), summary(fb)
        if (ba == bb and ia == ib and len(fa) == len(fb)):
            print(f"SAME {name}: {len(fa)} files")
            shutil.rmtree(a); shutil.rmtree(b)
            continue
        bad += 1
        onlyA = sorted(set(ba) - set(bb)); onlyB = sorted(set(bb) - set(ba))
        print(f"DIFF {name}: files {len(fa)} / {len(fb)}, binaries only in prototype {len(onlyA)}, only compiled {len(onlyB)}, ini equal {ia == ib}")
        for label, r in (("prototype", ra), ("compiled", rb)):
            tail = (r.stdout + r.stderr).strip().splitlines()[-3:]
            print(f"   {label}: {' | '.join(tail)}")
    sys.exit(1 if bad else 0)


if (__name__ == "__main__"):
    main()

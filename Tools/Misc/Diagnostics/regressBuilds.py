"""Old build vs new build over a corpus of real mod folders: which folders does a change move?

    py -3 regressBuilds.py <corpus.txt> <workDir> --a <pkgA> --b <pkgB> [--jobs 3] [--download disabled]

<pkgA> / <pkgB> are folders that each CONTAIN a `FixRaidenBoss2` package (the built `.pyd` and its DLLs
included). Make <pkgA> by copying `Anime Game Remap (for all users)/api/src/py/FixRaidenBoss2` into a
scratch folder BEFORE you rebuild (Overview habit 56: after the rebuild the old output is gone), and
<pkgB> the same way after it. Each side is fixed in its own process, so both builds run from one shell.

<corpus.txt> is one mod folder per line (a tab and anything after it are ignored, so you can tag lines).
A folder that is not there prints MISSING and the run goes on -- the maintainer moves folders between
`Mods` and its parent while you work (GameView's guide).

Every folder is copied twice into <workDir>/A and <workDir>/B, fixed with each build, and compared file
by file by md5. It prints one line per folder IN CORPUS ORDER: SAME (and the copies are deleted), DIFF
with the first differing files (the copies are kept to read), or ERR. Ordered output means one slow
folder holds every later line back -- a large library folder (11k files) takes about an hour PER SIDE,
so put it first or run it separately.

`--download` defaults to `disabled` so the comparison does not depend on the network. That is NOT what
a user runs: with downloads on, a mod missing a component can take a different path through the fix
(Overview habit 122). For any folder that moved, re-check it with `--download normal` before calling it
a user-visible change either way.

`--ini-unordered` compares an .ini file as the MULTISET of its sections rather than by md5: each section
with the comment lines right above it and without trailing blank lines, the preamble before the first
section kept in place. For a change that only moves where the fix writes its sections, which 3dmigoto does
not care about -- a SAME then still says every section's text is the same, and nothing else moved.

Read every DIFF; a SAME is proof. Then run the Integration Tester too (habit 119): its hand-written
fixtures hold shapes no real library has.
"""
import argparse
import concurrent.futures as cf
import hashlib
import os
import shutil
import subprocess
import sys

RUN = r'''
import os, sys
sys.path.insert(0, sys.argv[1]); os.add_dll_directory(os.path.join(sys.argv[1], "FixRaidenBoss2"))
import FixRaidenBoss2 as FRB
s = FRB.RemapService(path = sys.argv[2], downloadMode = sys.argv[3], keepBackups = False)
s.fix()
print("fixed", len(s.stats.ini.fixed), "skipped", len(s.stats.ini.skipped))
'''


def iniSections(data):
    """An .ini file's text as (its preamble, the sorted list of its sections, its tail) -- see --ini-unordered.
    Blank lines are ignored; a comment block touching a section's header belongs to that section, any
    other comment to what it follows."""
    lines = data.decode("utf-8", errors = "replace").splitlines()
    preamble, sections, current, pending = [], [], None, []

    def flush():
        (current if current is not None else preamble).extend(pending)
        pending.clear()

    for line in lines:
        stripped = line.strip()
        if stripped == "":
            flush()
        elif stripped.startswith("["):
            if current is not None:
                sections.append(current)
            current = list(pending) + [line]
            pending.clear()
        elif stripped.startswith(";"):
            pending.append(line)
        else:
            flush()
            (current if current is not None else preamble).append(line)

    tail = list(pending)
    if current is not None:
        sections.append(current)
    newline = chr(10)
    return newline.join(preamble), sorted(newline.join(s) for s in sections), newline.join(tail)


def tree(folder, iniUnordered = False):
    result = {}
    for dirpath, _dirs, files in os.walk(folder):
        for name in files:
            path = os.path.join(dirpath, name)
            with open(path, "rb") as f:
                data = f.read()
            if iniUnordered and name.lower().endswith(".ini"):
                data = repr(iniSections(data)).encode("utf-8")
            result[os.path.relpath(path, folder)] = hashlib.md5(data).hexdigest()
    return result


def one(src, work, pkgs, download, iniUnordered = False):
    name = os.path.basename(src.rstrip("/\\"))
    if not os.path.isdir(src):
        return f"MISSING {src}"

    results = {}
    for side in "AB":
        dst = os.path.join(work, side, name)
        shutil.rmtree(dst, ignore_errors = True)
        shutil.copytree(src, dst)
        run = subprocess.run([sys.executable, "-c", RUN, pkgs[side], dst, download],
                             capture_output = True, text = True, timeout = 4 * 3600)
        if run.returncode != 0:
            err = run.stderr.strip().splitlines()
            return f"ERR  {name} [{side}] {err[-1] if err else run.returncode}"
        results[side] = (tree(dst, iniUnordered), run.stdout.strip())

    a, b = results["A"][0], results["B"][0]
    diff = sorted(k for k in set(a) | set(b) if a.get(k) != b.get(k))
    if not diff:
        for side in "AB":
            shutil.rmtree(os.path.join(work, side, name), ignore_errors = True)
        return f"SAME {name} ({len(a)} files; {results['B'][1]})"
    return f"DIFF {name} ({len(diff)} of {len(a)} files): " + "; ".join(diff[:8])


def main():
    parser = argparse.ArgumentParser(description = __doc__, formatter_class = argparse.RawDescriptionHelpFormatter)
    parser.add_argument("corpus")
    parser.add_argument("work")
    parser.add_argument("--a", required = True, help = "folder containing the OLD build's FixRaidenBoss2")
    parser.add_argument("--b", required = True, help = "folder containing the NEW build's FixRaidenBoss2")
    parser.add_argument("--jobs", type = int, default = 3)
    parser.add_argument("--download", default = "disabled", choices = ["disabled", "normal", "always"])
    parser.add_argument("--ini-unordered", action = "store_true",
                        help = "compare an .ini as the multiset of its sections, ignoring where each one is written")
    args = parser.parse_args()

    pkgs = {"A": os.path.abspath(args.a), "B": os.path.abspath(args.b)}
    for side, pkg in pkgs.items():
        if not os.path.isdir(os.path.join(pkg, "FixRaidenBoss2")):
            sys.exit(f"--{side.lower()} {pkg} has no FixRaidenBoss2 folder")

    with open(args.corpus, encoding = "utf-8") as f:
        folders = [line.split("\t")[0].strip() for line in f if line.strip()]

    os.makedirs(args.work, exist_ok = True)
    with cf.ThreadPoolExecutor(args.jobs) as pool:
        for line in pool.map(lambda folder: one(folder, args.work, pkgs, args.download, args.ini_unordered), folders):
            print(line, flush = True)


if __name__ == "__main__":
    main()

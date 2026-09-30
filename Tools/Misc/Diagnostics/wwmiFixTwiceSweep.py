r"""EVERY WuWa mod folder on disk, fixed TWICE and then UNDONE, against a pristine copy.

The regression harness excluded folders by name and only ever fixed ONCE. This does not:

  coverage     every directory under WWMI and WWMI/Mods that holds a .ini, with no name filter --
               the excluded ones (Jinhsi1, the two `(backup ...)` copies) are mods too.
  idempotency  fix, then fix AGAIN, and require the file set and every byte to match. A folder is
               handled one .ini at a time, undo then fix, so a second pass is a different code path
               (NeuvilletteMelusent, 2026-09-26).
  undo         then undo, and require the folder back to the pristine copy byte for byte -- no
               leftover file, no lost file, no edited .ini.
  refs         after each pass: every `filename =` exists, every Resource reference is declared.

  py -3 finalSweep.py <tag>
"""
import glob
import hashlib
import os
import re
import shutil
import subprocess
import sys

Launcher = (r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5"
            r"\WWMI\Mods\FixRaidenBoss7.py")
Wwmi = r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI"
Work = (r"C:\Users\AlexX\AppData\Local\Temp\claude"
        r"\C--Users-AlexX-Documents-Games-Mods-Repos-Anime-Game-Remap"
        r"\86121637-104e-4025-b42e-d3a2d9b768b2\scratchpad\finalSweep")

Skip = re.compile(r"^(FrameAnalysis|Shader(Cache|Fixes|Backups)|Core|Mods)$", re.I)
Section = re.compile(r"^\s*\[([^\]]+)\]\s*$")
Assign = re.compile(r"^\s*([^=;\[]+?)\s*=\s*(.+?)\s*$")
ResRef = re.compile(r"^(?:ref\s+)?(Resource[\w.\\]*)$", re.I)
BindKey = re.compile(r"^(ps-t\d+|vb\d+|ib|this)$", re.I)


def modFolders():
    out = []
    for root in (Wwmi, os.path.join(Wwmi, "Mods")):
        for name in sorted(os.listdir(root)):
            path = os.path.join(root, name)
            if not os.path.isdir(path) or Skip.match(name):
                continue

            if glob.glob(os.path.join(path, "**", "*.ini"), recursive = True):
                out.append((name if root == Wwmi else "Mods/" + name, path))

    return out


def snap(folder):
    out = {}
    for root, _dirs, files in os.walk(folder):
        for name in files:
            path = os.path.join(root, name)
            with open(path, "rb") as f:
                out[os.path.relpath(path, folder).replace(os.sep, "/")] = hashlib.md5(f.read()).hexdigest()

    return out


def refs(folder):
    bad = []
    for root, _dirs, files in os.walk(folder):
        inis = [os.path.join(root, n) for n in files if n.lower().endswith(".ini")]
        if not inis:
            continue

        declared = set()
        for ini in inis:
            with open(ini, encoding = "utf-8", errors = "replace") as f:
                declared |= {m.group(1).lower() for m in map(Section.match, f) if m}

        for ini in inis:
            with open(ini, encoding = "utf-8", errors = "replace") as f:
                for line in f:
                    a = Assign.match(line.rstrip())
                    if not a:
                        continue

                    key, val = a.group(1).strip().lower(), a.group(2).strip()
                    if key == "filename":
                        if not os.path.exists(os.path.join(root, val.replace("\\", os.sep))):
                            bad.append(f"missing file {val}")
                    elif BindKey.match(key):
                        m = ResRef.match(val)
                        if m and m.group(1).lower() not in declared:
                            bad.append(f"undeclared {val}")

    return bad


def fix(path, undo = False):
    env = {k: v for k, v in os.environ.items() if k != "AG_REMAP_REPO"}
    args = [sys.executable, Launcher, "-s", path] + (["-u"] if undo else [])
    subprocess.run(args, capture_output = True, stdin = subprocess.DEVNULL,
                   cwd = os.path.dirname(Launcher), env = env)


tag = sys.argv[1]
out = os.path.join(Work, tag)
shutil.rmtree(out, ignore_errors = True)
os.makedirs(out)

folders = modFolders()
print(f"{len(folders)} mod folder(s)\n")

notIdempotent, notUndone, newRefs, manifest = [], [], [], {}
for name, src in folders:
    work = os.path.join(out, name.replace("/", "__"))
    shutil.copytree(src, work)
    fix(work, undo = True)          # the source may already be fixed; the baseline is UNFIXED
    pristine = snap(work)
    baseRefs = set(refs(work))

    fix(work)
    first, firstRefs = snap(work), set(refs(work))
    for rel, h in first.items():
        manifest[name + "/" + rel] = h

    fix(work)
    second = snap(work)
    if first != second:
        gone = sorted(set(first) - set(second))
        added = sorted(set(second) - set(first))
        moved = sorted(k for k in set(first) & set(second) if first[k] != second[k])
        notIdempotent.append((name, gone, added, moved))

    fix(work, undo = True)
    back = snap(work)
    if back != pristine:
        gone = sorted(set(pristine) - set(back))
        added = sorted(set(back) - set(pristine))
        moved = sorted(k for k in set(pristine) & set(back) if pristine[k] != back[k])
        notUndone.append((name, gone, added, moved))

    fresh = firstRefs - baseRefs
    if fresh:
        newRefs.append((name, sorted(fresh)))

    shutil.rmtree(work, ignore_errors = True)          # the copies are large

with open(os.path.join(out, "manifest.txt"), "w", encoding = "utf-8") as f:
    for k in sorted(manifest):
        f.write(f"{manifest[k]}  {k}\n")

report = open(os.path.join(out, "report.txt"), "w", encoding = "utf-8")
def say(s = ""):
    print(s)
    report.write(s + chr(10))

say(f"=== NOT IDEMPOTENT: {len(notIdempotent)} folder(s) ===")
for name, gone, added, moved in notIdempotent:
    say(f"  {name}")
    for k in gone:
        say(f"    - {k}")
    for k in added:
        say(f"    + {k}")
    for k in moved:
        say(f"    ~ {k}")

say(f"\n=== UNDO DID NOT RESTORE: {len(notUndone)} folder(s) ===")
for name, gone, added, moved in notUndone:
    say(f"  {name}")
    for k in gone:
        say(f"    lost  {k}")
    for k in added:
        say(f"    left  {k}")
    for k in moved:
        say(f"    edited {k}")

say(f"\n=== NEW BROKEN REFERENCES: {len(newRefs)} folder(s) ===")
for name, items in newRefs:
    say(f"  {name}")
    for k in items:
        say(f"    {k}")

say(f"\n{len(manifest)} fixed files hashed into {tag}/manifest.txt")

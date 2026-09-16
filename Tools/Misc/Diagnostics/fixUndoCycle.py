"""Fix a scratch copy of a mod, undo it, and require the .ini files to come back to the original.

    python fixUndoCycle.py <original mod folder> <scratch folder>

An undo is only as complete as what the fix wrote INSIDE its own boilerplate block: the default
remover takes everything between those lines, and outside them only a `Remap`-named section it can
attribute by hash. This is the check for a new template's output (see Creating Remaps' "Undo is only
as complete as what the fix wrote INSIDE its block").

After the FIX it reports where every `[TextureOverride...Hide]` section landed, inside or outside
the remap block. After the UNDO it compares every .ini of the original with the scratch copy section
by section: leftover sections, lost sections, differing bodies and any `remap` text left. Exits 1 if
any file is not restored.

The scratch folder is deleted and re-copied first -- never point it at a real mod. Uses the compiled
tables alone, like runCompiled.py; Linux / WSL paths; set AG_REMAP_REPO for another repo.
"""
import os
import re
import shutil
import sys

Repo = os.environ.get("AG_REMAP_REPO", "/mnt/e/Computer/Games/Genshin/Repos/Repos/Fix-Raiden-Boss")
sys.path.insert(0, os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py"))
import FixRaidenBoss2 as FRB

orig, scratch = sys.argv[1], sys.argv[2]
if os.path.abspath(orig) == os.path.abspath(scratch):
    raise SystemExit("the scratch folder must not be the original")

shutil.rmtree(scratch, ignore_errors = True)
shutil.copytree(orig, scratch)


def inis(root):
    for folder, _, names in os.walk(root):
        for n in names:
            if n.lower().endswith(".ini"):
                yield os.path.relpath(os.path.join(folder, n), root)


def read(path):
    return open(path, "rb").read().decode("utf-8", "replace").replace("\r\n", "\n")


def sections(text):
    return {m.group(1): m.group(2).strip() for m in re.finditer(r"^\[([^\]]+)\]\n((?:(?!^\[).*\n?)*)", text, re.M)}


FRB.CppStrategyOverrides.clear()
service = FRB.RemapService(path = scratch, keepBackups = False)
service.fix()
print(f"FIX: .ini fixed {len(service.stats.ini.fixed)}, skipped {len(service.stats.ini.skipped)}")

for rel in sorted(inis(scratch)):
    lines = read(os.path.join(scratch, rel)).split("\n")
    opens = [i for i, l in enumerate(lines) if re.match(r"^; -+ \w+ Remap -+$", l)]
    closes = [i for i, l in enumerate(lines) if re.match(r"^; -+$", l)]
    for h, line in ((i, l) for i, l in enumerate(lines) if re.match(r"^\[TextureOverride\w+Hide\]$", l)):
        inside = any(o < h and any(c > h for c in closes) for o in opens)
        print(f"  {rel}: {line} at line {h + 1}, {'INSIDE' if inside else 'OUTSIDE'} the remap block")

service = FRB.RemapService(path = scratch, keepBackups = False, undoOnly = True)
service.fix()
print(f"UNDO: .ini fixed {len(service.stats.ini.fixed)}, skipped {len(service.stats.ini.skipped)}")

bad = 0
for rel in sorted(inis(orig)):
    a = read(os.path.join(orig, rel))
    path = os.path.join(scratch, rel)
    if not os.path.isfile(path):
        print(f"  MISSING after undo: {rel}")
        bad += 1
        continue

    b = read(path)
    sa, sb = sections(a), sections(b)
    leftovers = [n for n in sb if n not in sa]
    lost = [n for n in sa if n not in sb]
    differ = [n for n in sa if n in sb and sa[n] != sb[n]]
    remapText = len(re.findall("remap", b, re.I))
    ok = not leftovers and not lost and not differ and remapText == 0
    bad += 0 if ok else 1
    print(f"  {rel}: {'OK' if ok else 'BAD'}  text identical (ignoring trailing whitespace): {a.rstrip() == b.rstrip()}; "
          f"leftover sections {leftovers}, lost {lost[:5]}, bodies differ {differ[:5]}, 'remap' mentions {remapText}")

print("ALL RESTORED" if bad == 0 else f"{bad} FILE(S) NOT RESTORED")
sys.exit(0 if bad == 0 else 1)

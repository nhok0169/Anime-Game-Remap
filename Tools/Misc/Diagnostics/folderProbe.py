r"""Does the fix still build a folder the mod does not have?

Fixes a scratch copy of one mod and reports every DIRECTORY the fix created that the pristine mod did
not have, plus every `filename =` in the fixed `.ini` files that does not resolve to a file on disk --
the dangling-reference check that found the mod-manager mod rendering nothing but its weapon.

  py -3 folderProbe.py <mod folder name>
"""
import os
import pathlib
import re
import shutil
import subprocess
import sys

Launcher = (r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5"
            r"\WWMI\Mods\FixRaidenBoss7.py")
Wwmi = pathlib.Path(r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI")
Work = pathlib.Path(__file__).parent / "folderProbe"

mod = sys.argv[1]
sys.stdout.reconfigure(errors = "replace")

source = Wwmi / mod
if not source.is_dir():
    source = Wwmi / "Mods" / mod

assert source.is_dir(), "no such mod: " + mod

if Work.is_dir():
    shutil.rmtree(Work)

Work.mkdir(parents = True)
dest = Work / mod.replace("/", "_")
shutil.copytree(source, dest)

before = {str(p.relative_to(dest)).replace("\\", "/") for p in dest.rglob("*") if p.is_dir()}

run = subprocess.run(["py", "-3", Launcher, "--src", str(dest), "--download", "disabled"],
                     capture_output = True, text = True, errors = "replace")

after = {str(p.relative_to(dest)).replace("\\", "/") for p in dest.rglob("*") if p.is_dir()}
made = sorted(after - before)

print("=== directories the FIX created ({}) ===".format(len(made)))
for d in made:
    holds = sorted(p.name for p in (dest / d).iterdir())
    print("   {}   <- {}".format(d, ", ".join(holds)[:70]))

if not made:
    print("   none -- the fix wrote beside the mod's own files")

# every `filename =` must resolve
dangling = []
total = 0
for ini in dest.rglob("*.ini"):
    folder = ini.parent
    for line in ini.read_text(encoding = "utf-8", errors = "replace").split("\n"):
        kv = re.match(r"\s*([^=;\[]+?)\s*=\s*(.+?)\s*$", line)
        if not kv or kv.group(1).strip().lower() != "filename":
            continue

        total += 1
        target = folder / kv.group(2).strip().replace("\\", "/")
        if not target.exists():
            dangling.append("{}: {}".format(ini.name, kv.group(2).strip()))

print("\n=== `filename =` references: {} checked, {} dangling ===".format(total, len(dangling)))
for d in dangling[:12]:
    print("   " + d)

print("\n=== the fix's own new files, where they landed ===")
for p in sorted(dest.rglob("*Remap*")):
    if p.is_file():
        print("   " + str(p.relative_to(dest)).replace("\\", "/"))

"""Fix a mod folder through the COMPILED tables alone -- no runtime overrides -- and print every stats bucket.

    python runCompiled.py <mod folder> [--verbose]

The compiled counterpart of a prototype run: fix a scratch copy with each and diff the folders (buffers by
bytes, .ini files without their carriage returns). Linux / WSL paths; set AG_REMAP_REPO for another repo."""
import os
import sys

Repo = os.environ.get("AG_REMAP_REPO", "/mnt/e/Computer/Games/Genshin/Repos/Repos/Fix-Raiden-Boss")
sys.path.insert(0, os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py"))
import FixRaidenBoss2 as FRB

folder = sys.argv[1]
verbose = "--verbose" in sys.argv
FRB.CppStrategyOverrides.clear()
service = FRB.RemapService(path = folder, keepBackups = False, logger = FRB.Logger() if verbose else None)
service.fix()
stats = service.stats
print(f".ini fixed: {len(stats.ini.fixed)}, skipped: {len(stats.ini.skipped)}")
for path, error in stats.ini.skipped.items():
    print(f"  SKIPPED {os.path.relpath(path, folder)}: {error}")
for label in ("blend", "position", "texcoord", "buf", "texEdit", "texAdd", "download"):
    s = getattr(stats, label, None)
    if s is None:
        continue
    print(f"{label}: fixed {len(s.fixed)}, skipped {len(s.skipped)}")
    for path in sorted(s.fixed):
        print(f"  {os.path.relpath(path, folder)}")
    for path, error in s.skipped.items():
        print(f"  SKIPPED {os.path.relpath(path, folder)}: {error}")

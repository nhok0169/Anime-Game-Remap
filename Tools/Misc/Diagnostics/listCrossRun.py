"""listCrossRun.py <mod folder> <its mod type> <out folder>: a pair's list of edits (Tools/Misc/Prototypes/editList.py, its
pair from AGREMAP_LIST_PAIR="Source,Target"), unchanged, run on ANOTHER character's mod, of any game.

The list's parser and fixer are registered for <its mod type> (fixing to the data's TARGET), the mod is copied to
<out folder>/<name>, RemapService fixes the copy, and what happened is printed: a traceback, what the service skipped and
why, and the files the run added, changed or removed. A list that is generic writes nothing for a mod with nothing of its
source in it."""
import hashlib
import os
import shutil
import sys
import traceback

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Prototypes"))
import editList as L                                      # noqa: E402

FRB = L.FRB
src, modType, outRoot = sys.argv[1], sys.argv[2], sys.argv[3]
out = os.path.join(outRoot, os.path.basename(os.path.normpath(src)))
if os.path.exists(out):
    shutil.rmtree(out)
shutil.copytree(src, out)


def snapshot(folder):
    result = {}
    for base, _, names in os.walk(folder):
        for name in names:
            path = os.path.join(base, name)
            with open(path, "rb") as f:
                result[os.path.relpath(path, folder)] = hashlib.md5(f.read()).hexdigest()
    return result


before = snapshot(out)
FRB.CppStrategyOverrides.setParser(modType, lambda ini, modTypeId: L.makeParser(ini, modTypeId))
FRB.CppStrategyOverrides.setFixer(modType, L.TARGET, L.makeFixer)
try:
    service = FRB.RemapService(path = out, downloadMode = FRB.DownloadMode.Disabled,
                               fromModTypeIds = {getattr(FRB.ModTypeId, modType)},
                               toModTypeIds = {getattr(FRB.ModTypeId, L.TARGET)})
    service.fix()
    for path, error in service.stats.ini.skipped.items():
        print("SKIPPED", os.path.relpath(path, out), "--", error)
except Exception:
    traceback.print_exc()
finally:
    FRB.CppStrategyOverrides.clear()

after = snapshot(out)
for rel in sorted(set(before) | set(after)):
    if before.get(rel) != after.get(rel):
        print("ADDED  " if rel not in before else "REMOVED" if rel not in after else "CHANGED", rel)
written = sum(1 for rel in after if rel.lower().endswith(".ini")
              and L.TARGET + "RemapFix" in open(os.path.join(out, rel), encoding = "utf-8", errors = "replace").read())
print("FIX SECTIONS OF", L.TARGET, "WRITTEN IN", written, ".ini file(s)")

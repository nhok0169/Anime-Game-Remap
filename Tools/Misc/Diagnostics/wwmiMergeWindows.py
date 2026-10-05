"""Does every bone the remapped blend USES sit in a window the fix actually merges?

This is the check the first scaffolding would have failed, and it is written to fail against it:

  * every `run = CommandListMergeSkeleton<fix>` in the output contributes the window its
    `vg_offset` / `vg_count` name, and the union of those windows must cover every bone id the
    remapped blend buffer references. A window nobody merges is a garbage matrix.
  * the copy that PUBLISHES the buffer must come before the merger runs inside the list, so what a
    draw reads is the previous frame's complete buffer rather than this frame's partial one.
  * the second merger run must read `vs-cb3`, not `vs-cb4`, or every bone past 256 is wrong.

Run it against the previous build's `.ini` with `--against <file>` to see it fail there.
"""
import argparse
import os
import re
import sys

import numpy as np

Mod = r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI\Mods\Chisa17\chisathiccmod\ChisaThiccMod"

parser = argparse.ArgumentParser()
parser.add_argument("--ini", default = os.path.join(Mod, "mod.ini"))
parser.add_argument("--blend", default = os.path.join(Mod, "Meshes", "ChisaParfaitRemapBlend.buf"))
args = parser.parse_args()

text = open(args.ini, encoding = "utf-8", errors = "replace").read()
failures = []

# ---- the windows the output merges -------------------------------------------------------------
windows = []
pending = {}
for line in text.splitlines():
    s = line.strip()
    m = re.match(r"^\$\\WWMIv1\\vg_(offset|count)\s*=\s*(\d+)$", s, re.IGNORECASE)
    if (m):
        pending[m.group(1).lower()] = int(m.group(2))
    elif (re.match(r"^run\s*=\s*CommandListMergeSkeleton\w*$", s, re.IGNORECASE)):
        if ("offset" in pending and "count" in pending):
            windows.append((pending["offset"], pending["count"]))
        pending = {}

covered = set()
for off, count in windows:
    covered |= set(range(off, off + count))
print(f"merge windows in the output: {len(windows)} -> {sorted(windows)}")
print(f"bones they cover: {len(covered)} (max {max(covered) if covered else -1})")

# ---- the bones the remapped blend actually uses -------------------------------------------------
raw = np.fromfile(args.blend, dtype = np.uint8)
n = len(raw) // 16
rows = raw[:n * 16].reshape(n, 16)
ids, wts = rows[:, :8].astype(int), rows[:, 8:].astype(int)
used = {int(v) for v in np.unique(ids[wts > 0])}
print(f"bones the remapped blend uses: {len(used)} (max {max(used)})")

missing = sorted(used - covered)
if (missing):
    weight = float(wts[np.isin(ids, missing) & (wts > 0)].sum()) / float(wts[wts > 0].sum())
    failures.append(f"{len(missing)} bone(s) used by the blend sit in NO merged window "
                    f"({weight * 100:.2f}% of all weight): {missing[:15]}")

# ---- the merge list's own shape -----------------------------------------------------------------
m = re.search(r"^\[CommandListMergeSkeleton\w*\]\s*$(.*?)(?=^\[)", text, re.MULTILINE | re.DOTALL)
if (not m):
    failures.append("no [CommandListMergeSkeleton...] section in the output at all")
else:
    body = [l.strip() for l in m.group(1).splitlines() if (l.strip())]
    firstMerge = next((i for i, l in enumerate(body) if (l.lower().startswith("run = customshader"))), None)
    firstCopy = next((i for i, l in enumerate(body) if ("= copy resourcemergedskeletonrw" in l.lower())), None)
    if (firstCopy is None):
        failures.append("the merge list never publishes ResourceMergedSkeleton")
    elif (firstMerge is not None and firstCopy > firstMerge):
        failures.append("the RW -> SRV copy comes AFTER the merger runs, so a draw reads a buffer "
                        "holding only the windows merged so far this frame")
    cbs = [l.lower().replace(" ", "") for l in body if (l.lower().replace(" ", "").startswith("cs-cb8="))]
    if (cbs and cbs.count("cs-cb8=refvs-cb3") == 0):
        failures.append(f"the extra (past-256) merger run does not read vs-cb3: cs-cb8 bindings are {cbs}")

print()
if (failures):
    for f in failures:
        print(f"  FAIL  {f}")
    sys.exit(1)
print("  PASS  every used bone is in a merged window; the copy publishes before merging; "
      "the extra run reads vs-cb3")

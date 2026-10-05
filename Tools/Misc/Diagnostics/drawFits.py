##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

# Every explicit `drawindexed = <count>, <start>, 0` of a REMAPPED section in a fixed mod must fit inside the index
# buffer that section binds -- a fixer whose .ini takes its counts from one split while the buffers come from another
# overruns the buffer and draws garbage (Lumine2's dress: 67224 indices drawn out of a 50763-index buffer, 2026-09-30).
#   py -3 drawFits.py <mod folder>
# Prints every overrun and a `checked N overruns M` line; `checked 0` means the mod draws only `auto` and says nothing.
import os, re, sys
if (len(sys.argv) != 2 or not os.path.isdir(sys.argv[1])):
    raise SystemExit(f"no such mod folder: {sys.argv[1:]}")   # a walk over nothing would say `checked 0`
bad = checked = 0
for root, _, files in os.walk(sys.argv[1]):
    for f in files:
        if not f.lower().endswith(".ini"): continue
        text = open(os.path.join(root, f), encoding="utf-8", errors="replace").read()
        res = {}
        for m in re.finditer(r"^\[(Resource[^\]]+)\]\s*\n((?:(?!^\[).*\n?)*)", text, re.M):
            fn = re.search(r"^filename\s*=\s*(.+)$", m.group(2), re.M)
            fmt = re.search(r"^format\s*=\s*(.+)$", m.group(2), re.M)
            if fn: res[m.group(1).lower()] = (fn.group(1).strip(), 2 if fmt and "R16" in fmt.group(1) else 4)
        for m in re.finditer(r"^\[(TextureOverride[^\]]+)\]\s*\n((?:(?!^\[).*\n?)*)", text, re.M):
            ib = None
            for line in m.group(2).splitlines():
                s = line.strip()
                mi = re.match(r"ib\s*=\s*(\S+)", s)
                if mi: ib = mi.group(1).lower()
                md = re.match(r"drawindexed\s*=\s*(\d+)\s*,\s*(\d+)", s)
                if md and ib and ib in res and "remap" in ib:
                    path = os.path.join(root, res[ib][0].replace("\\", os.sep))
                    if not os.path.exists(path): continue
                    n = os.path.getsize(path) // res[ib][1]
                    checked += 1
                    if int(md.group(1)) + int(md.group(2)) > n:
                        bad += 1; print("OVERRUN", f, m.group(1), s, "ib has", n)
print("checked", checked, "overruns", bad)

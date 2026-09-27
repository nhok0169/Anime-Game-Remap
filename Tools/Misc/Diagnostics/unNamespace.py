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

# Control for the namespace A/B: strips a namespace-merged sub-mod's `if $\<X>\Master\swapvar==n`
# wrapper (and its matching endif) from every section, leaving the same keys unconditional.
#   py -3 unNamespace.py <ini>          (rewrites in place; keep a copy)
import re, sys
p = sys.argv[1]
raw = open(p, "rb").read().decode("utf-8", errors="replace")
nl = "\r\n" if "\r\n" in raw else "\n"
out, depth, stripped = [], [], 0
for line in raw.replace("\r\n", "\n").split("\n"):
    s = line.strip().lower()
    if re.match(r"^if \$\\.*\\master\\swapvar\s*==", s):
        depth.append(True); stripped += 1; continue
    if s.startswith("if "):
        depth.append(False)
    elif s == "endif" and depth:
        if depth.pop():
            continue
    out.append(line)
open(p, "wb").write(nl.join(out).encode("utf-8"))
print("stripped", stripped, "namespace ifs from", p)

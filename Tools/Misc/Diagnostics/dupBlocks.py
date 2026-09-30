r"""Repeated runs of code in one file -- the shape the two fallback-registration loops have.

Near-duplicates are what "expand the existing module rather than adding another ad-hoc helper" is
about, and they are easy to miss by reading because the two copies are usually far apart. This finds
the longest runs of identical non-trivial lines that appear more than once, ignoring indentation and
comments so a copy that was re-indented or re-commented still matches.

  py -3 dupBlocks.py <file> [min lines]
"""
import collections
import pathlib
import re
import sys

path = pathlib.Path(sys.argv[1])
least = int(sys.argv[2]) if len(sys.argv) > 2 else 4
sys.stdout.reconfigure(errors = "replace")

raw = path.read_bytes().replace(b"\r\n", b"\n").decode("utf-8")
lines = raw.split("\n")

# (normalised line, original line number), dropping comments, braces alone, and blanks -- a run of
# `}` matches everywhere and says nothing
keep = []
for i, line in enumerate(lines, 1):
    s = line.strip()
    if not s or s.startswith("//") or s.startswith("*") or s.startswith("/*") or s in ("{", "}", "};"):
        continue

    keep.append((re.sub(r"\s+", " ", s), i))

best = {}
n = len(keep)
for i in range(n):
    for j in range(i + 1, n):
        run = 0
        while (i + run < j and j + run < n and keep[i + run][0] == keep[j + run][0]):
            run += 1

        if run >= least:
            key = tuple(keep[i + k][0] for k in range(run))
            here = (keep[i][1], keep[j][1], run)
            if key not in best or run > best[key][2]:
                best[key] = here

# drop a run that is wholly inside a longer reported one
found = sorted(best.items(), key = lambda kv: -kv[1][2])
shown = []
for key, (a, b, run) in found:
    if any(a >= x and a + run <= x + r and b >= y and b + run <= y + r for x, y, r in shown):
        continue

    shown.append((a, b, run))

print("{}: {} repeated run(s) of {}+ significant lines\n".format(path.name, len(shown), least))
for a, b, run in shown[:12]:
    print("=== {} lines, at {} and again at {}".format(run, a, b))
    for k in range(min(run, 6)):
        print("      " + keep[[i for i, (_, ln) in enumerate(keep) if ln == a][0] + k][0][:100])

    if run > 6:
        print("      ... {} more".format(run - 6))

    print()

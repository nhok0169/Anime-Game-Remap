"""Compare every generated .buf / .dds between two fixed trees, by content hash.

Enumerates from BOTH sides so a file only one of them produced shows up, and never builds a
filename by hand -- assembling one wrong is how a comparison harness reports mismatches that are
really its own (see Overview's habit 10).
"""
import hashlib, io, os, sys

old, new = sys.argv[1], sys.argv[2]
# Matched anywhere in the name, NOT as a suffix. An edited texture's name ENDS in a hash --
# "AyakaBodyRemapTexMzY IMY.dds" -- so an endswith() test silently skipped every one of them, which
# is how a texture bug survived several "0 differ" runs. If a check can be structurally blind to the
# files the change is about, it will be.
GENERATED = ("remapblend", "remaptex", "remapdl", "remapposition")
EXTS = (".buf", ".dds", ".ib")


def collect(root):
    found = {}
    for dirpath, _dirs, files in os.walk(root):
        for f in files:
            low = f.lower()
            if not low.endswith(EXTS) or not any(s in low for s in GENERATED):
                continue
            p = os.path.join(dirpath, f)
            found[os.path.relpath(p, root).replace(os.sep, "/")] = \
                hashlib.sha256(io.open(p, "rb").read()).hexdigest()
    return found


o, n = collect(old), collect(new)
if not o and not n:
    print("  no generated binaries on either side")
    sys.exit(0)

same = diff = onlyOld = onlyNew = 0
for k in sorted(set(o) | set(n)):
    if k not in n:
        print("  ONLY OLD  %s" % k); onlyOld += 1
    elif k not in o:
        print("  ONLY NEW  %s" % k); onlyNew += 1
    elif o[k] == n[k]:
        same += 1
    else:
        print("  DIFFERS   %s" % k); diff += 1

print("  %d identical, %d differ, %d only-old, %d only-new" % (same, diff, onlyOld, onlyNew))

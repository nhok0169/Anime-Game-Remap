"""Does every resource the fixed .ini names actually exist on disk?

The check the A/B harness was missing. A section diff compares .ini TEXT, so an .ini that names a
resource the run never produced -- or produced and then deleted -- looks perfect. In game it is a
missing mesh.

Run it on a tree that was fixed WITHOUT undoing first, which is the state a real user's folder is
in: a mod that already carries an old fix, possibly across several .ini files.
"""
import os, re, sys

root = sys.argv[1]
bad = 0
checked = 0

for dirpath, _dirs, files in os.walk(root):
    for name in files:
        if not name.lower().endswith(".ini") or name.startswith("DISABLED"):
            continue

        path = os.path.join(dirpath, name)
        section = None
        for line in open(path, encoding="utf-8", errors="replace"):
            line = line.rstrip("\r\n")
            m = re.match(r"^\[([^\]]+)\]", line)
            if m:
                section = m.group(1)
                continue
            if section is None or line.startswith(";"):
                continue

            m = re.match(r"^\s*filename\s*=\s*(.+?)\s*$", line, re.I)
            if not m:
                continue

            checked += 1
            target = os.path.normpath(os.path.join(dirpath, m.group(1).replace("\\", os.sep)))
            if not os.path.isfile(target):
                bad += 1
                print("  DANGLING  [%s] -> %s" % (section, m.group(1)))
                print("            in %s" % os.path.relpath(path, root))

print("%s: %d filename reference(s) checked, %d dangling"
      % ("FAIL" if bad else "OK", checked, bad))
sys.exit(1 if bad else 0)

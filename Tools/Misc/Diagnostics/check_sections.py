"""Every Resource* a fixed .ini REFERENCES must be a section the .ini (or its siblings) DEFINES.

The companion to check_dangling.py, and it catches a class that one cannot see. check_dangling
follows `filename = ...` to the disk; this follows `<register> = Resource...` to the section list.
A register naming a section nobody defines leaves check_dangling perfectly happy -- there is no
filename to check -- and in game that register is simply unbound.

Found by CherryHuTao: two texture edits on one object shared a resource graph, so the .ini said
    ps-t1 = ResourceHuTaoCherryBodyLightMapHuTaoOpaqueBodyLightMapRemapTex
and never defined it. The body drew with no lightmap and looked flat.

    py -3 check_sections.py <fixed mod folder>
"""
import os
import re
import sys

SECTION = re.compile(r"^\[([^\]]+)\]")

# A value that names a resource section. 3dmigoto resolves these by section name, so anything
# starting with "Resource" is a reference -- the ".0"/".1" suffixes are part of the name.
REFERENCE = re.compile(r"^\s*[\w\-]+\s*=\s*(Resource[\w.]*)\s*$", re.IGNORECASE)


def iniFiles(folder):
    for root, _dirs, files in os.walk(folder):
        for name in files:
            if name.lower().endswith(".ini"):
                yield os.path.join(root, name)


def scan(folder):
    # Defined names are pooled PER FOLDER, not per file: a merged mod splits its resources across
    # <name>.ini and <name>RemapFix1.ini and the importer loads all of them together.
    defined = {}
    referenced = []

    for path in iniFiles(folder):
        try:
            text = open(path, encoding="utf-8", errors="replace").read()
        except OSError:
            continue

        folderKey = os.path.dirname(path)
        for line in text.splitlines():
            section = SECTION.match(line)
            if section is not None:
                defined.setdefault(folderKey, set()).add(section.group(1).lower())
                continue

            reference = REFERENCE.match(line)
            if reference is not None:
                referenced.append((folderKey, path, reference.group(1)))

    missing = [(path, name) for folderKey, path, name in referenced
               if name.lower() not in defined.get(folderKey, set())]

    for path, name in missing:
        print("  MISSING SECTION  [%s]" % name)
        print("                   referenced in %s" % os.path.basename(path))

    print("%s: %d resource reference(s) checked, %d undefined"
          % ("FAIL" if missing else "OK", len(referenced), len(missing)))
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(scan(sys.argv[1]))

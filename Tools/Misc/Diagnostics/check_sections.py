r"""Every section a fixed .ini REFERENCES must be one the .ini (or its siblings) DEFINES.

The companion to check_dangling.py, and it catches a class that one cannot see. check_dangling
follows `filename = ...` to the disk; this follows a reference to the section list. A name nobody
defines leaves check_dangling perfectly happy -- there is no filename to check -- and in game the
register is simply unbound, or the call does nothing.

TWO KINDS OF REFERENCE, and the second was missing until 2026-09-23:

* `<register> = Resource...`. Found by CherryHuTao: two texture edits on one object shared a
  resource graph, so the .ini said
      ps-t1 = ResourceHuTaoCherryBodyLightMapHuTaoOpaqueBodyLightMapRemapTex
  and never defined it. The body drew with no lightmap and looked flat.
* `run = <section>`. A call to a section nobody defines does NOTHING, which is worse than an unbound
  register -- a whole block of setup silently does not happen. Chisa17's remap emitted
      run = CommandListMergeSkeletonChisaParfaitRemapFix
  and defined it nowhere, so its draws were never skinned with the merged skeleton and the model
  came out smeared. This tool said "OK: 63 resource reference(s) checked, 0 undefined".

A `run =` naming a NAMESPACED section -- RabbitFX's `CommandList\RabbitFX\Run`, WWMI's
`CustomShader\WWMIv1\SkeletonMerger` -- lives in another mod's .ini, so those are counted and
reported separately rather than called missing.

    py -3 check_sections.py <fixed mod folder>
"""
import os
import re
import sys

SECTION = re.compile(r"^\[([^\]]+)\]")

# A value that names a resource section. 3dmigoto resolves these by section name, so anything
# starting with "Resource" is a reference -- the ".0"/".1" suffixes are part of the name.
REFERENCE = re.compile(r"^\s*[\w\-]+\s*=\s*(Resource[\w.]*)\s*$", re.IGNORECASE)

# `run = <section>`. The value may carry a `ref ` prefix, and a name containing a backslash is
# namespaced into another mod's .ini (an external library), which this folder cannot be expected
# to define.
RUN = re.compile(r"^\s*run\s*=\s*(?:ref\s+)?(\S+)\s*$", re.IGNORECASE)


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
    external = []

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
                continue

            call = RUN.match(line)
            if call is not None:
                target = call.group(1)
                if "\\" in target or "/" in target:
                    external.append(target)
                else:
                    referenced.append((folderKey, path, target))

    missing = [(path, name) for folderKey, path, name in referenced
               if name.lower() not in defined.get(folderKey, set())]

    for path, name in missing:
        print("  MISSING SECTION  [%s]" % name)
        print("                   referenced in %s" % os.path.basename(path))

    print("%s: %d reference(s) checked (registers and `run =`), %d undefined%s"
          % ("FAIL" if missing else "OK", len(referenced), len(missing),
             ", %d external/namespaced left alone" % len(set(external)) if external else ""))
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(scan(sys.argv[1]))

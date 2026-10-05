"""Did every texture the fix EDITED end up bound by a section that draws?

A texture edit has three independent halves, and the summary line reports only the first:

  1. the edit ran and wrote a file       -- ``editted 4 *.dds files and skipped 0``
  2. a resource section names that file
  3. a section that DRAWS binds that resource

On 2026-09-25 the compiled WuWa fix did (1) on four files and neither (2) nor (3) on any of them,
and the summary said exactly what a working run says. The A/B against the prototype was blind to
it as well, because both sides wrote the same orphans. Nothing in the pipeline asks this question,
so ask it separately.

Reports, per fixed folder:

  * ORPHAN     -- a file the fix wrote that no resource section names
  * UNBOUND    -- a resource section naming one that no other line references
  * and the count that passed, because a checker that cannot say "nothing was checked" will
    report a tidy zero over a path that shattered (Overview habit 66)

Generated files are recognised by this library's own keywords in the NAME (``RemapTex``,
``RemapDL``, ``RemapBlend`` and friends), which is what it marks its own output with; a mod's own
files are never reported.

``--prove`` copies a fixed folder, deletes one binding line from its ``.ini``, and requires the
check to FAIL on the copy -- a check that has never failed is not a check (habit 34).

Usage::

    py -3 Tools/Misc/Diagnostics/editedButUnbound.py <fixed folder>... [--prove]
"""
import os
import re
import shutil
import sys
import tempfile

#: What this library names its own generated files with. A mod's own file carries none of these.
Generated = ("remaptex", "remapdl", "remapblend", "remapposition", "remaptexcoord", "remapib",
             "remapshapekey", "remapvector", "remapcolor")

Section = re.compile(r"^\s*\[([^\]]+)\]")
Assign = re.compile(r"^\s*([^=;\[]+?)\s*=\s*(.+?)\s*$")


def isGenerated(name):
    low = os.path.basename(name).lower()
    return any(k in low for k in Generated)


def readIni(path):
    """{section: [(key, value)]} -- repeats kept, since a section may bind a register twice."""
    out = {}
    current = None
    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.rstrip("\r\n")
            m = Section.match(line)
            if m:
                current = m.group(1)
                out.setdefault(current, [])
                continue

            if current is None or line.lstrip().startswith(";"):
                continue

            a = Assign.match(line)
            if a:
                out[current].append((a.group(1).strip(), a.group(2).strip()))
    return out


def check(root, quiet=False):
    """(orphans, unbound, boundCount, iniCount)"""
    orphans = []
    unbound = []
    bound = 0
    iniCount = 0

    # BINDING is file-scoped in 3dmigoto -- a section can only reference a resource its own .ini
    # declares -- but the FILE on disk is shared, and a merge writes several .ini files beside each
    # other. So "is this resource bound" is asked per .ini and "does anything declare this file" is
    # asked over the whole tree. Asking both per .ini reports every buffer of a merged mod as an
    # orphan once per sibling .ini, which is how this first ran.
    declaredAnywhere = set()
    for dirpath, _dirs, files in os.walk(root):
        for name in files:
            if not name.lower().endswith(".ini") or name.startswith("DISABLED"):
                continue

            for _section, pairs in readIni(os.path.join(dirpath, name)).items():
                for key, value in pairs:
                    if key.lower() == "filename":
                        declaredAnywhere.add(os.path.basename(value.replace("\\", "/")).lower())

    for dirpath, _dirs, files in os.walk(root):
        for name in files:
            if not name.lower().endswith(".ini") or name.startswith("DISABLED"):
                continue

            iniCount += 1
            path = os.path.join(dirpath, name)
            ini = readIni(path)
            rel = os.path.relpath(path, root)

            # which sections declare a generated file, and what every other line references
            declaring = {}
            referenced = set()
            for section, pairs in ini.items():
                for key, value in pairs:
                    if key.lower() == "filename":
                        if isGenerated(value):
                            declaring[section.lower()] = (section, value)
                    else:
                        for token in re.split(r"[\s,=]+", value):
                            token = token.strip()
                            if token:
                                referenced.add(token.lower())

            for lowName, (section, filename) in declaring.items():
                if lowName in referenced:
                    bound += 1
                else:
                    unbound.append((rel, section, filename))

            # ...and every generated file beside it that NO .ini in the tree declares
            for sibling in os.listdir(dirpath):
                if not isGenerated(sibling):
                    continue
                if sibling.lower().endswith(".ini") or sibling.lower().endswith(".txt"):
                    continue
                if sibling.lower() not in declaredAnywhere:
                    entry = (os.path.relpath(dirpath, root), sibling)
                    if entry not in orphans:
                        orphans.append(entry)

    if not quiet:
        print(f"  {root}")
        print(f"    {iniCount} .ini file(s), {bound} generated resource(s) bound")
        for rel, section, filename in unbound:
            print(f"    UNBOUND  [{section}] -> {filename}   in {rel}")
        for folder, sibling in orphans:
            print(f"    ORPHAN   {sibling}   in {folder}   (no .ini in this mod declares it)")

    return orphans, unbound, bound, iniCount


def prove(root):
    """Unbind one generated resource on a copy and require the check to notice.

    EVERY line that references it, not the first: a resource is normally bound by several sections
    (one per branch of a toggle, one per copy), so cutting one occurrence leaves the rest and the
    check passes -- correctly. The first version of this cut one line, reported PROVE FAILED, and
    was right to: it had unbound nothing.
    """
    tmp = tempfile.mkdtemp(prefix="editedButUnbound-")
    try:
        work = os.path.join(tmp, "mod")
        shutil.copytree(root, work)

        # find a generated resource that IS bound, and cut the line that binds it
        cut = None
        for dirpath, _dirs, files in os.walk(work):
            for name in files:
                if not name.lower().endswith(".ini") or name.startswith("DISABLED"):
                    continue

                path = os.path.join(dirpath, name)
                ini = readIni(path)
                declared = {s.lower() for s, pairs in ini.items()
                            for k, v in pairs if k.lower() == "filename" and isGenerated(v)}
                if not declared:
                    continue

                with open(path, encoding="utf-8", errors="replace", newline="") as f:
                    lines = f.readlines()

                target = None
                for line in lines:
                    a = Assign.match(line.rstrip("\r\n"))
                    if a and a.group(1).strip().lower() != "filename" \
                            and a.group(2).strip().lower() in declared:
                        target = a.group(2).strip()
                        break

                if target is not None:
                    cut = (path, lines, target)
                    break
            if cut:
                break

        if cut is None:
            print("PROVE INCONCLUSIVE: this folder has no bound generated resource to unbind")
            return 2

        path, lines, what = cut
        kept = [line for line in lines
                if not (Assign.match(line.rstrip("\r\n"))
                        and Assign.match(line.rstrip("\r\n")).group(1).strip().lower() != "filename"
                        and Assign.match(line.rstrip("\r\n")).group(2).strip().lower() == what.lower())]
        print(f"  (unbinding {what!r}: {len(lines) - len(kept)} reference line(s) removed)")
        with open(path, "w", encoding="utf-8", newline="") as f:
            f.writelines(kept)

        _o, unbound, _b, _n = check(work, quiet=True)
        if unbound:
            print(f"PROVE OK: unbinding {what!r} is reported ({len(unbound)} UNBOUND)")
            return 0

        print(f"PROVE FAILED: unbound {what!r} and the check still passed")
        return 1
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


def main():
    args = [a for a in sys.argv[1:] if a != "--prove"]
    if not args:
        print(__doc__)
        return 2

    if "--prove" in sys.argv:
        return prove(args[0])

    bad = 0
    total = 0
    for root in args:
        orphans, unbound, bound, iniCount = check(root)
        bad += len(orphans) + len(unbound)
        total += iniCount

    if total == 0:
        print("NOTHING WAS CHECKED: no .ini files under any of those paths")
        return 2

    print(f"\n{'FAIL' if bad else 'OK'}: {total} .ini file(s), {bad} problem(s)")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())

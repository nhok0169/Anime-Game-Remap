"""A/B two fixed mod folders' ``.ini`` files SEMANTICALLY -- what the game sees, not what the text says.

    python abIni.py <folder A> <folder B>

**Byte-identical buffers say nothing about the ``.ini``.** A compiled fix's first run produced
buffers that were byte-for-byte a verified prototype's, and an ``.ini`` file with ``HashNotFound``
in all ten sections and the SOURCE's ``match_first_index`` in four of them. A mod like that is
well-formed, loads without a warning, and draws nothing.

It cannot be a text diff either: this library suffixes a graph id onto every resource it generates
(``...RemapBlend0_0_0_0`` -> ``..._B8g.buf``) where a prototype names them by hand, and it wraps a
collected register in ``if 1 ... endif``. So each ``TextureOverride`` is reduced to what actually
reaches the game:

  * every value naming a ``Resource`` section is replaced by the **md5 of the file** that resource
    names, plus the resource's other keys -- so two differently-named resources pointing at
    identical bytes compare equal;
  * ``if`` / ``endif`` lines are dropped (``if 1`` is always taken);
  * key ORDER is kept, because the last binding of a register wins.

Everything that reaches the game -- the hashes, the indices, the registers, their order and the
bytes behind each one -- is then compared exactly. Roughly thirty lines of the below is the whole
idea; the rest is reporting.

Exits non-zero on any difference. Needs nothing but the folders -- no API, no build."""
import hashlib
import os
import sys

Marker = "Remap ---"


def iniPath(folder):
    hits = []
    for root, _, files in os.walk(folder):
        for f in files:
            if (f.lower().endswith(".ini")):
                hits.append(os.path.join(root, f))
    if (len(hits) != 1):
        sys.exit(f"expected exactly 1 .ini under {folder}, found {len(hits)}"
                 + ("" if hits else " -- an empty folder is not a match, it is a failed run"))
    return hits[0]


def parse(path):
    text = open(path, encoding = "utf-8", errors = "replace").read().replace("\r\n", "\n")
    if (Marker not in text):
        sys.exit(f"{path}: no remap block -- the fix did not run on it")

    sections, name = {}, None
    for line in text.split(Marker, 1)[1].splitlines():
        s = line.strip()
        if (not s or s.startswith(";")):
            continue
        if (s.startswith("[") and s.endswith("]")):
            name = s[1:-1]
            sections[name] = []
        elif (s == "endif" or s.startswith("if ") or s.startswith("else")):
            continue
        elif (name is not None and "=" in s):
            key, _, value = s.partition("=")
            sections[name].append((key.strip(), value.strip()))
    return sections, os.path.dirname(path)


def md5(path):
    if (not os.path.isfile(path)):
        return f"<missing file {os.path.basename(path)}>"
    return "md5:" + hashlib.md5(open(path, "rb").read()).hexdigest()


def resolve(sections, folder):
    out = {}
    for name, kvps in sections.items():
        if (name.startswith("Resource")):
            continue

        resolved = []
        for key, value in kvps:
            target = sections.get(value)
            if (target is not None):
                extras, fileName = [], None
                for tk, tv in target:
                    if (tk == "filename"):
                        fileName = tv
                    else:
                        extras.append(f"{tk}={tv}")
                value = md5(os.path.join(folder, fileName.replace("\\", os.sep))) if fileName else "<no filename>"
                if (extras):
                    value += " [" + ", ".join(sorted(extras)) + "]"
            resolved.append((key, value))
        out[name] = resolved
    return out


a = resolve(*parse(iniPath(sys.argv[1])))
b = resolve(*parse(iniPath(sys.argv[2])))

problems = []
for name in sorted(set(a) - set(b)):
    problems.append(f"section only in A: [{name}]")
for name in sorted(set(b) - set(a)):
    problems.append(f"section only in B: [{name}]")

for name in sorted(set(a) & set(b)):
    if (a[name] == b[name]):
        print(f"  ok   [{name}]")
        continue

    print(f"  DIFF [{name}]")
    for line in a[name]:
        if (line not in b[name]):
            print(f"         A only: {line[0]} = {line[1]}")
    for line in b[name]:
        if (line not in a[name]):
            print(f"         B only: {line[0]} = {line[1]}")
    if (sorted(a[name]) == sorted(b[name])):
        print("         (same keys and values, different ORDER -- the last binding of a register wins)")
    problems.append(f"[{name}] differs")

print()
print("PROBLEMS:" if problems else "THE TWO .ini FILES ARE SEMANTICALLY IDENTICAL")
for problem in problems:
    print("  -", problem)
sys.exit(1 if problems else 0)

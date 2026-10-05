r"""Side-effect sweep over two whole fixed corpora, and over every toggle VARIANT in the final one.

Three things the fix-written manifest cannot see, and the first is what hid the red kimono for a
whole round:

  tree      EVERY file of every mod folder, the mod's OWN .ini included -- it is edited in place,
            so it is not a "fix-written file" and regress.py never compared it.
  variants  per section, per toggle BRANCH: which registers are bound and to what. A defect that
            only shows on `$hair == 1` is invisible to a whole-file diff, because the file changed
            for a reason you already accepted.
  refs      every Resource reference resolves, and every `filename =` exists, per folder.

  py -3 sweep.py <old snapshot> <new snapshot>
"""
import collections
import hashlib
import os
import re
import sys

Section = re.compile(r"^\s*\[([^\]]+)\]\s*$")
Assign = re.compile(r"^(\s*)([^=;\[]+?)\s*=\s*(.+?)\s*$")
Cond = re.compile(r"^\s*(if|else if|elif|else|endif)\b(.*)$", re.I)
ResRef = re.compile(r"^(?:ref\s+)?(Resource[\w.\\]*)$", re.I)
BindKey = re.compile(r"^(ps-t\d+|vb\d+|ib|this|run)$", re.I)


def readIni(path):
    with open(path, encoding = "utf-8", errors = "replace") as f:
        return [line.rstrip("\n").rstrip("\r") for line in f]


def sections(lines):
    """{section name: [lines]}, concatenating a name declared more than once."""
    out = collections.OrderedDict()
    current = None
    for line in lines:
        m = Section.match(line)
        if m:
            current = m.group(1)
            out.setdefault(current, [])
            continue

        if current is not None:
            out[current].append(line)

    return out


def branches(body):
    """[(condition path, [(key, value)])] -- one entry per path through the section's if/else tree."""
    paths = [([], [])]
    stack = []
    for line in body:
        c = Cond.match(line)
        if c:
            word = c.group(1).lower()
            rest = c.group(2).strip()
            if word == "if":
                stack.append([rest])
            elif word in ("else if", "elif"):
                if stack:
                    stack[-1].append(rest)
            elif word == "else":
                if stack:
                    stack[-1].append("else")
            elif word == "endif":
                if stack:
                    stack.pop()

            # a new path starts at every branch boundary
            paths.append((list(cond[-1] for cond in stack), []))
            continue

        a = Assign.match(line)
        if a and not line.strip().startswith(";"):
            paths[-1][1].append((a.group(2).strip().lower(), a.group(3).strip()))

    return [p for p in paths if p[1]]


def variantMap(folder):
    """{(ini, section, condition path, key): value} over every mod .ini of a folder."""
    out = {}
    for root, _dirs, files in os.walk(folder):
        for name in files:
            if not name.lower().endswith(".ini"):
                continue

            rel = os.path.relpath(os.path.join(root, name), folder).replace(os.sep, "/")
            for sec, body in sections(readIni(os.path.join(root, name))).items():
                for i, (cond, kvps) in enumerate(branches(body)):
                    seen = {}
                    for key, val in kvps:
                        if BindKey.match(key):
                            # a branch may carry SEVERAL `run =` lines, and keying by the key alone
                            # collapses them -- which read as "a binding was replaced" on Chisa7
                            n = seen.get(key, 0)
                            seen[key] = n + 1
                            out[(rel, sec, " && ".join(cond), i, key if n == 0 else f"{key}#{n}")] = val

    return out


def declared(folder):
    names, files = set(), {}
    for root, _dirs, fs in os.walk(folder):
        for name in fs:
            if not name.lower().endswith(".ini"):
                continue

            path = os.path.join(root, name)
            for sec, body in sections(readIni(path)).items():
                names.add(sec.lower())
                for line in body:
                    a = Assign.match(line)
                    if a and a.group(2).strip().lower() == "filename":
                        files[(root, sec.lower())] = a.group(3).strip()

    return names, files


def tree(snapshot):
    out = {}
    for game in ("gi", "wuwa"):
        base = os.path.join(snapshot, game)
        if not os.path.isdir(base):
            continue

        for root, _dirs, files in os.walk(base):
            for name in files:
                path = os.path.join(root, name)
                with open(path, "rb") as f:
                    out[os.path.relpath(path, snapshot).replace(os.sep, "/")] = \
                        hashlib.md5(f.read()).hexdigest()

    return out


def folders(snapshot):
    for game in ("gi", "wuwa"):
        base = os.path.join(snapshot, game)
        if not os.path.isdir(base):
            continue

        for name in sorted(os.listdir(base)):
            path = os.path.join(base, name)
            if os.path.isdir(path):
                yield game + "/" + name, path


old, new = sys.argv[1], sys.argv[2]

# ---- 1. the whole tree, the mod's own .ini included ------------------------------------------
a, b = tree(old), tree(new)
changed = sorted(k for k in set(a) & set(b) if a[k] != b[k])
onlyOld = sorted(set(a) - set(b))
onlyNew = sorted(set(b) - set(a))
print(f"TREE  {len(set(a) & set(b)) - len(changed)} identical, {len(changed)} changed, "
      f"{len(onlyOld)} only in {os.path.basename(old)}, {len(onlyNew)} only in {os.path.basename(new)}")
for k in changed + ["- " + k for k in onlyOld] + ["+ " + k for k in onlyNew]:
    print("   ", k)

# ---- 2. every toggle VARIANT's bindings -------------------------------------------------------
print()
oldFolders = dict(folders(old))
moved = 0
for name, path in folders(new):
    if name not in oldFolders:
        continue

    va, vb = variantMap(oldFolders[name]), variantMap(path)
    gone = {k: va[k] for k in set(va) - set(vb)}
    added = {k: vb[k] for k in set(vb) - set(va)}
    diff = {k: (va[k], vb[k]) for k in set(va) & set(vb) if va[k] != vb[k]}
    if not (gone or added or diff):
        continue

    moved += 1
    print(f"VARIANTS {name}")
    for k, v in sorted(gone.items()):
        print(f"    -  {k[0]} [{k[1]}] if({k[2]}) #{k[3]}  {k[4]} = {v}")
    for k, v in sorted(added.items()):
        print(f"    +  {k[0]} [{k[1]}] if({k[2]}) #{k[3]}  {k[4]} = {v}")
    for k, (x, y) in sorted(diff.items()):
        print(f"    ~  {k[0]} [{k[1]}] if({k[2]}) #{k[3]}  {k[4]}: {x} -> {y}")

print(f"\n{moved} folder(s) whose per-variant bindings moved")

# ---- 3. does every reference resolve, in the NEW corpus ---------------------------------------
print()
bad = 0
for name, path in folders(new):
    names, files = declared(path)
    for (root, sec), fn in sorted(files.items()):
        if not os.path.exists(os.path.join(root, fn.replace("\\", os.sep))):
            bad += 1
            print(f"REFS {name}: [{sec}] -> missing file {fn}")

    for key, val in sorted(variantMap(path).items()):
        m = ResRef.match(val)
        if m and m.group(1).lower() not in names:
            bad += 1
            print(f"REFS {name}: {key[0]} [{key[1]}] {key[4]} = {val}  (no such section)")

print(f"\n{bad} unresolved reference(s) in {os.path.basename(new)}")

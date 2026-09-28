r"""For every per-variant binding that CHANGED, compare the BYTES each side's resource names.

A binding moving from `ResourceFooSanhuaRemapRef` to `ResourceTexture7` is cosmetic when the two
sections name the same file, and a real change when they do not. Nothing in a section diff says
which, and "the fix now uses the mod's own resource" is a claim about the NAME.

  py -3 resolveDiff.py <old snapshot> <new snapshot>
"""
import hashlib
import os
import re
import sys

Section = re.compile(r"^\s*\[([^\]]+)\]\s*$")
Assign = re.compile(r"^(\s*)([^=;\[]+?)\s*=\s*(.+?)\s*$")
Cond = re.compile(r"^\s*(if|else if|elif|else|endif)\b(.*)$", re.I)
ResRef = re.compile(r"^(?:ref\s+)?(Resource[\w.\\]*)$", re.I)
BindKey = re.compile(r"^(ps-t\d+|vb\d+|ib|this)$", re.I)


def readIni(path):
    with open(path, encoding = "utf-8", errors = "replace") as f:
        return [line.rstrip("\n").rstrip("\r") for line in f]


def sections(lines):
    out, current = {}, None
    for line in lines:
        m = Section.match(line)
        if m:
            current = m.group(1)
            out.setdefault(current, [])
            continue

        if current is not None:
            out[current].append(line)

    return out


def fileOfResource(folder):
    """{resource name lower: md5 of the file it names, or a marker}"""
    out = {}
    for root, _dirs, files in os.walk(folder):
        for name in files:
            if not name.lower().endswith(".ini"):
                continue

            for sec, body in sections(readIni(os.path.join(root, name))).items():
                for line in body:
                    a = Assign.match(line)
                    if a and a.group(2).strip().lower() == "filename":
                        path = os.path.join(root, a.group(3).strip().replace("\\", os.sep))
                        if os.path.exists(path):
                            with open(path, "rb") as f:
                                out[sec.lower()] = hashlib.md5(f.read()).hexdigest()
                        else:
                            out[sec.lower()] = "MISSING:" + a.group(3).strip()

    return out


def bindings(folder):
    out = {}
    for root, _dirs, files in os.walk(folder):
        for name in files:
            if not name.lower().endswith(".ini"):
                continue

            rel = os.path.relpath(os.path.join(root, name), folder).replace(os.sep, "/")
            for sec, body in sections(readIni(os.path.join(root, name))).items():
                stack, i = [], 0
                for line in body:
                    c = Cond.match(line)
                    if c:
                        word = c.group(1).lower()
                        if word == "if":
                            stack.append(c.group(2).strip())
                        elif word in ("else if", "elif", "else"):
                            if stack:
                                stack[-1] = c.group(2).strip() or "else"
                        elif word == "endif" and stack:
                            stack.pop()

                        i += 1
                        continue

                    a = Assign.match(line)
                    if a and BindKey.match(a.group(2).strip().lower()):
                        out[(rel, sec, " && ".join(stack), i, a.group(2).strip().lower())] = a.group(3).strip()

    return out


old, new = sys.argv[1], sys.argv[2]
same = diff = unknown = 0
for game in ("gi", "wuwa"):
    base = os.path.join(new, game)
    if not os.path.isdir(base):
        continue

    for folder in sorted(os.listdir(base)):
        a, b = os.path.join(old, game, folder), os.path.join(base, folder)
        if not os.path.isdir(a):
            continue

        ba, bb = bindings(a), bindings(b)
        fa, fb = fileOfResource(a), fileOfResource(b)
        for key in sorted(set(ba) & set(bb)):
            if ba[key] == bb[key]:
                continue

            ma, mb = ResRef.match(ba[key]), ResRef.match(bb[key])
            if not (ma and mb):
                continue

            ha = fa.get(ma.group(1).lower())
            hb = fb.get(mb.group(1).lower())
            if ha is None or hb is None:
                unknown += 1
                print(f"?? {game}/{folder} {key[1]} {key[4]}: {ba[key]} -> {bb[key]}  (a side names no file)")
            elif ha == hb:
                same += 1
            else:
                diff += 1
                print(f"!! {game}/{folder} {key[1]} {key[4]}: {ba[key]} -> {bb[key]}")
                print(f"     old {ha}")
                print(f"     new {hb}")

print(f"\n{same} renamed to the SAME bytes, {diff} to DIFFERENT bytes, {unknown} undecidable")

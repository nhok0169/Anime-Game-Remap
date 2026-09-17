"""Splices chosen top-level classes of a freshly generated core.pyi into the tracked one, leaving every
other class exactly as it is.

    python pyiSplice.py <generated core.pyi>                         list the classes that differ
    python pyiSplice.py <generated core.pyi> --show A B ...          unified diff of those classes
    python pyiSplice.py <generated core.pyi> --apply A B ...         replace those classes in place

Why not copy the generated file over: a checkout several agents build is never all yours, and the
module you generated from carries their bindings too (or lacks them). The rule is "keep it only if
the diff is your classes" -- run with no class names first; any differing class you did not touch
is someone else's, and stays out.

Generate on Linux from a copy of the package on the Linux filesystem (importing across /mnt/e is
slow and flaky), with the pinned pybind11 (3.0.4):

    rsync -a --exclude __pycache__ <api/src/py/FixRaidenBoss2> /tmp/stubsrc/
    cd /tmp/stubsrc && PYTHONPATH=/tmp/stubsrc python -m pybind11_stubgen FixRaidenBoss2.core -o /tmp/stub --root-suffix ""

Line endings of the tracked file are kept. A class is its `class X` line through the last indented
line before the next unindented one; a new class (only in the generated file) or `__all__` is not
handled -- add those by hand.
"""
import argparse, difflib, os, re, sys

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", ".."))
TRACKED = os.path.join(REPO, "Anime Game Remap (for all users)", "api", "src", "py", "FixRaidenBoss2", "core.pyi")


def read(path):
    with open(path, "rb") as f:
        data = f.read()
    return data.decode("utf-8").replace("\r\n", "\n"), b"\r\n" in data


def blocks(txt):
    lines = txt.split("\n")
    result = {}
    for i, line in enumerate(lines):
        m = re.match(r"class (\w+)\b", line)
        if not m:
            continue
        end = i + 1
        while end < len(lines) and (lines[end] == "" or lines[end][0] in " \t"):
            end += 1
        while end > i + 1 and lines[end - 1] == "":
            end -= 1
        result[m.group(1)] = (i, end)
    return lines, result


parser = argparse.ArgumentParser(description = __doc__, formatter_class = argparse.RawDescriptionHelpFormatter)
parser.add_argument("generated")
parser.add_argument("--show", nargs = "*")
parser.add_argument("--apply", nargs = "*")
parser.add_argument("--tracked", default = TRACKED)
args = parser.parse_args()

tracked, crlf = read(args.tracked)
generated, _ = read(args.generated)
tLines, tBlocks = blocks(tracked)
gLines, gBlocks = blocks(generated)

changed = sorted(n for n in tBlocks.keys() & gBlocks.keys() if tLines[slice(*tBlocks[n])] != gLines[slice(*gBlocks[n])])
print(f"{len(changed)} classes differ:", changed)
print("only in tracked:", sorted(tBlocks.keys() - gBlocks.keys()))
print("only in generated:", sorted(gBlocks.keys() - tBlocks.keys()))

for name in (args.show or []):
    if name not in changed:
        print(f"\n===== {name}: no difference")
        continue
    print(f"\n===== {name}")
    sys.stdout.writelines(l + "\n" for l in difflib.unified_diff(tLines[slice(*tBlocks[name])], gLines[slice(*gBlocks[name])], lineterm = "", n = 2))

if args.apply:
    missing = [n for n in args.apply if n not in tBlocks or n not in gBlocks]
    if missing:
        raise SystemExit(f"not a class in both files: {missing}")
    for name in sorted(args.apply, key = lambda n: tBlocks[n][0], reverse = True):
        start, end = tBlocks[name]
        tLines[start:end] = gLines[slice(*gBlocks[name])]
    text = "\n".join(tLines)
    if crlf:
        text = text.replace("\n", "\r\n")
    data = text.encode("utf-8")
    with open(args.tracked, "wb") as f:
        f.write(data)
    print("spliced:", sorted(args.apply))

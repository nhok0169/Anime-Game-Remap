r"""Every function in a C++ file, with its line span and size -- so an audit can be EXHAUSTIVE.

Two audits of `WWMIFixer.cpp` in one session both reported it clean and both missed things, because
both were pattern searches: grep for `std::filesystem`, grep for `ofstream`, grep for identical
repeated lines. A pattern search finds the mechanisms you thought of. It cannot find "this function
re-implements a library concept", because that has no textual signature -- the third copy of the
blend-stride derivation survived a duplicate-block scan purely by carrying a different error message.

So: list every function, and walk the list. Brace-depth based, so it does not care what a function is
called or what it contains.

  py -3 fnInventory.py <file> [min lines]
"""
import pathlib
import re
import sys

path = pathlib.Path(sys.argv[1])
least = int(sys.argv[2]) if len(sys.argv) > 2 else 1
sys.stdout.reconfigure(errors = "replace")

text = path.read_bytes().replace(b"\r\n", b"\n").decode("utf-8")
lines = text.split("\n")

# a signature line: ends in `{` and has a `(` before it, and is not a control statement
sig = re.compile(r"^(\s*)(?!(?:if|for|while|switch|catch|else|do|return|struct|class|namespace|enum)\b)"
                 r"([A-Za-z_~][\w:<>,&*\s\[\]]*\([^;]*)\s*(?:const\s*)?(?:noexcept\s*)?(?:->[^{]*)?\{\s*$")

found = []
depth = 0
openAt = None
openSig = None
openIndent = 0
for i, line in enumerate(lines, 1):
    stripped = re.sub(r"//.*$", "", line)
    if openAt is None:
        m = sig.match(stripped)
        if m and stripped.count("{") >= 1:
            openAt, openSig, openIndent = i, " ".join(m.group(2).split())[:78], len(m.group(1))
            depth = stripped.count("{") - stripped.count("}")
            if depth <= 0:
                openAt = None
            continue

    if openAt is not None:
        depth += stripped.count("{") - stripped.count("}")
        if depth <= 0:
            found.append((openAt, i, i - openAt + 1, openIndent, openSig))
            openAt = None

found = [f for f in found if f[2] >= least]
print("{}: {} function(s) of {}+ lines, {} lines total\n".format(
    path.name, len(found), least, len(lines)))
for start, end, size, indent, name in found:
    print("{:>5}-{:<5} {:>4}L  {}{}".format(start, end, size, " " * (indent // 4), name))

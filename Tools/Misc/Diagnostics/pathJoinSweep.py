"""Every ``std::filesystem`` path JOIN whose right operand is a narrow ``std::string``.

``parent / someNarrowString`` reads that string as the ACTIVE CODE PAGE on Windows, exactly like
``std::filesystem::path(str)`` does. Architecture's "Never use ``path.string()``..." section has
two greps for the CONSTRUCTOR and STREAM forms; neither can see the join form, because a join has
no keyword in it -- which is how three of them sat in shared code until 2026-09-28.

What they cost: a mod whose ``.ini`` is named ``mod-<non-Latin>.ini`` got its generated copy named
from a mangled round trip (``IniFileFixContext::fixedFilePath``), so ``RemapService::_origIniPath``
could not map it back and the copy SURVIVED EVERY UNDO -- still carrying remapped sections, so the
mod went on drawing on the target with no fix installed. The other two named the backup
(``IniFile::disableIni`` and ``IniFileRemoveContext::removeBackup``), which worked only because the
two agreed with each other about the wrong name.

Two things this got wrong on its first pass, both worth knowing before writing any sweep:

* it skipped any operand whose NAME contained ``path``, which skipped ``pathToStr(...)`` -- a
  function that RETURNS A NARROW STRING. A word in a name says nothing about a type.
* it worked line by line, and the bug's own join was written across TWO lines with the ``/`` at the
  end of the first. Statements here wrap; a line-based reader cannot see them.

So it reads STATEMENTS (comments and string literals stripped, split on ``;``). Expect false
positives -- it cannot tell a ``std::filesystem::path`` variable from a ``std::string`` one, and it
reports integer division whose statement happens to mention a path. A hit is READ, not trusted.

``--prove`` runs it over the broken form of the line it was written for and requires a hit: habit
34, against a check whose first two versions would both have passed the build they were meant to
fail.

Usage::

    py -3 Tools/Misc/Diagnostics/pathJoinSweep.py [--prove]
"""
import pathlib
import re
import sys

Repo = pathlib.Path(__file__).resolve().parents[3]
Cpp = Repo / "Anime Game Remap (for all users)" / "api" / "src" / "cpp"

Join = re.compile(r"/\s*\(?\s*([A-Za-z_][A-Za-z0-9_:]*)\s*(\(|\)|\+|,|$)")

#: The right operand is safe only when what is on the right is ALREADY a ``std::filesystem::path``.
SafeCall = ("strToPath",)

#: Accessors of ``std::filesystem::path`` itself, which really do return a path.
SafeAccessor = ("stem", "filename", "extension", "parent_path", "root_path", "relative_path")

Comment = re.compile(r"//[^\n]*")
Block = re.compile(r"/\*.*?\*/", re.S)
String = re.compile(r'"(?:[^"\\]|\\.)*"')

#: The exact shape ``IniFileFixContext::fixedFilePath`` had before 2026-09-28 -- the broken build
#: this sweep has to be able to fail against.
Broken = """
        std::filesystem::path copy = path.parent_path() /
            (FileService::pathToStr(path.stem()) + IniKeywords::RemapFix
             + std::to_string(groupInd) + FileService::pathToStr(path.extension()));
"""


def statements(text):
    """Cut `text` into (line number, statement), comments and string literals removed."""
    text = Block.sub(" ", text)
    out = []
    line = 1
    buf = []
    start = 1
    for ch in text:
        if ch == "\n":
            line += 1
        if ch == ";":
            out.append((start, "".join(buf)))
            buf = []
            start = line
            continue
        buf.append(ch)
    if buf:
        out.append((start, "".join(buf)))

    cleaned = []
    for at, stmt in out:
        stmt = Comment.sub(" ", stmt)
        stmt = String.sub('""', stmt)
        cleaned.append((at, " ".join(stmt.split())))
    return cleaned


def interesting(stmt):
    """The right operand of the first unsafe join in `stmt`, or None."""
    if "/" not in stmt:
        return None
    if not any(w in stmt for w in ("path", "Path", "folder", "Folder", "dir", "Dir")):
        return None

    for match in Join.finditer(stmt):
        name = match.group(1)
        leaf = name.split("::")[-1]
        if leaf in SafeCall or leaf in SafeAccessor:
            continue
        return name
    return None


def prove():
    """The sweep has to SEE the broken build before it means anything on the fixed one."""
    hits = [h for h in (interesting(s) for _at, s in statements(Broken)) if h]
    if hits:
        print(f"PROVE OK: the broken join is reported (right operand {hits[0]!r})")
        return 0

    print("PROVE FAILED: the sweep does not see the very line it was written for")
    return 1


def main():
    if "--prove" in sys.argv:
        return prove()

    if not Cpp.is_dir():
        print(f"no C++ tree at {Cpp}")
        return 1

    hits = 0
    for suffix in ("*.cpp", "*.h", "*.tpp"):
        for f in sorted(Cpp.rglob(suffix)):
            if "extern" in f.parts or "build" in f.parts:
                continue

            raw = f.read_bytes().replace(b"\r\n", b"\n").decode("utf-8", "replace")
            for at, stmt in statements(raw):
                name = interesting(stmt)
                if name is None:
                    continue

                hits += 1
                print(f"{f.relative_to(Cpp)}:{at}: right operand {name!r}")
                print(f"    {stmt[:190]}")

    print(f"\n{hits} join(s) to read")
    return 0


if __name__ == "__main__":
    sys.exit(main())

"""Does a mod fix and undo the same way when its ``.ini`` file has a NON-LATIN NAME?

Fixes two scratch copies of one mod -- one untouched, one with every ``.ini`` renamed to ASCII --
and requires the two runs to produce the same set of files (modulo the rename) and to leave nothing
behind after ``--undo``. The ASCII copy is the CONTROL: it isolates the file's own name from the
folder's, from the mod's contents, and from whatever the fix happens to do for that character.

Why it exists. A path-shaped ``std::string`` in ``core/`` is UTF-8, and on Windows handing one
straight to ``std::filesystem`` reads it as the ACTIVE CODE PAGE instead -- so a UTF-8 name goes in
and a DIFFERENT name comes out. Architecture's greps catch the ``fs::path(str)`` and ``ofstream(str)``
spellings; ``Tools/Misc/Diagnostics/pathJoinSweep.py`` catches ``folder / narrowString``. This
catches what neither can: the behaviour.

It has found the class twice.

* **2026-09-11** -- a Korean-named mod FOLDER had all 7 of its ``.ini`` files skipped, then reported
  ``editted 18 *.dds files and skipped 0`` having written none of them.
* **2026-09-28** -- a ChisaParfait mod whose ``.ini`` is ``mod-<non-Latin>.ini`` had its generated
  copy written as ``mod-<mojibake>RemapFix1.ini``, which the undo could not map back to the ``.ini``
  it belongs to. **The copy survived every undo**, still carrying remapped sections, so the mod went
  on drawing on the target with no fix installed. The folder name was fine; only the file's was not,
  which is why the 2026-09-11 round did not cover it.

Both arms are copied fresh every run, so one can never read the other's output.

A backup (``RemapBKUP*``) is SUPPOSED to survive an undo -- removing it is what ``--deleteBackup``
is for -- so it is not counted as a leftover. Its NAME is checked, though: two more sites of this
bug were in ``IniFile::disableIni`` and ``IniFileRemoveContext::removeBackup``, which mangled the
backup's name identically and so agreed with each other while both being wrong.

Usage::

    py -3 Tools/Misc/Diagnostics/nonLatinNames.py <mod folder> [--out <scratch>] [--keep]

Exits non-zero on any difference, and on a mod with no non-Latin ``.ini`` name at all -- a check
that cannot say "nothing was checked" will report a tidy zero over nothing (Overview habit 66).
"""
import argparse
import os
import pathlib
import shutil
import subprocess
import sys

#: Where FixRaidenBoss7.py lives, if the caller does not say. The launcher, not the API: it is what
#: a real run goes through, and it resolves the repo the same way the maintainer's own command does.
DefaultLauncher = (r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5"
                   r"\WWMI\Mods\FixRaidenBoss7.py")


def isAscii(text):
    return all(ord(ch) < 128 for ch in text)


def run(launcher, folder, *args):
    proc = subprocess.run([sys.executable, str(launcher), "-s", str(folder), *args],
                          capture_output=True, text=True, encoding="utf-8", errors="replace",
                          env=dict(os.environ, PYTHONIOENCODING="utf-8"))
    return proc


def names(folder):
    """Every file under `folder`, relative, as text -- so a mangled name shows up AS ITSELF.

    Never a glob or a `find` for an expected name: a mojibake name still matches `*RemapFix*.ini`,
    which is exactly how this bug passed for two weeks.
    """
    out = set()
    for root, _dirs, files in os.walk(folder):
        for f in files:
            out.add(str(pathlib.Path(root, f).relative_to(folder)))
    return out


def arm(src, out, tag, launcher, toAscii):
    work = out / tag
    if work.exists():
        shutil.rmtree(work)
    work.parent.mkdir(parents=True, exist_ok=True)
    shutil.copytree(src, work)

    renames = {}
    if toAscii:
        for ini in sorted(work.rglob("*.ini")):
            if isAscii(ini.name):
                continue

            ascii_ = f"mod{len(renames)}.ini"
            renames[ini.name] = ascii_
            ini.rename(ini.with_name(ascii_))

    before = names(work)

    fix = run(launcher, work)
    if fix.returncode != 0:
        print(f"{tag}: the fix exited {fix.returncode}")
        print(fix.stdout[-2000:])
        return None

    made = sorted(names(work) - before)

    undo = run(launcher, work, "-u")
    if undo.returncode != 0:
        print(f"{tag}: the undo exited {undo.returncode}")
        print(undo.stdout[-2000:])
        return None

    left = sorted(n for n in names(work) - before
                  if not pathlib.Path(n).name.startswith("RemapBKUP"))
    return made, left, renames


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("mod", help="the mod folder to test, copied fresh into --out")
    parser.add_argument("--out", default=None, help="scratch folder (default: <mod>-nonLatin)")
    parser.add_argument("--launcher", default=DefaultLauncher, help="FixRaidenBoss7.py")
    parser.add_argument("--keep", action="store_true", help="leave both arms on disk afterwards")
    args = parser.parse_args()

    src = pathlib.Path(args.mod).resolve()
    out = pathlib.Path(args.out).resolve() if args.out else src.with_name(src.name + "-nonLatin")

    nonLatin = [p for p in src.rglob("*.ini") if not isAscii(p.name)]
    if not nonLatin:
        print(f"NOTHING CHECKED: no .ini under {src} has a non-Latin name, so this proves nothing")
        return 2

    print(f"{len(nonLatin)} .ini file(s) with a non-Latin name:")
    for p in nonLatin:
        print(f"    {p.relative_to(src)}")

    plain = arm(src, out, "nonLatin", args.launcher, False)
    control = arm(src, out, "ascii", args.launcher, True)
    if plain is None or control is None:
        return 1

    madeB, leftB, _ = plain
    madeA, leftA, renames = control

    ok = True

    # 1. the two arms made the SAME set of files, once the rename is undone. Map each ASCII stem
    #    back to the real one rather than comparing counts: a count matches while the names are
    #    mangled, which is the whole failure being looked for.
    mapped = set()
    for m in madeA:
        for real, ascii_ in renames.items():
            realStem = real[:-len(".ini")]
            asciiStem = ascii_[:-len(".ini")]
            m = m.replace(asciiStem, realStem)
        mapped.add(m)

    onlyControl = sorted(mapped - set(madeB))
    onlyPlain = sorted(set(madeB) - mapped)
    if onlyControl or onlyPlain:
        ok = False
        print("\nFAIL: the two arms wrote different files")
        for n in onlyControl:
            print(f"    only with an ASCII name: {n}")
        for n in onlyPlain:
            print(f"    only with the real name: {n}")
    else:
        print(f"\nOK   both arms wrote the same {len(madeB)} files")

    # 2. neither arm leaves anything behind
    for left, tag in ((leftA, "ascii"), (leftB, "nonLatin")):
        if left:
            ok = False
            print(f"FAIL {tag}: {len(left)} file(s) survived the undo")
            for n in left:
                print(f"    ! {n}")
        else:
            print(f"OK   {tag}: the undo removed everything the fix wrote")

    if not args.keep:
        shutil.rmtree(out, ignore_errors=True)

    print("PASS" if ok else "FAILED")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())

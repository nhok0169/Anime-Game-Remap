"""Score the FILE route against the hash history, over a folder of mods.

    py -3 audit.py "<a folder holding mod folders>"

WHY THIS EXISTS, AND WHY IT SHOULD BE RUN AFTER ANY CHANGE TO `TextureTyper`. Typing a texture from
its file is an inference, and an inference that writes a hash into somebody's mod has to be held to
something. The history is that something: every hash it explains is a labelled example nobody had to
produce, so running the file route on those same files and comparing is a real test at no cost.

The route is allowed to ABSTAIN as often as it likes -- it is a fallback, and a hash it declines is
reported exactly as it was before. A DISAGREEMENT is a defect, because the same reasoning applied one
file along would write the wrong hash and the mod would render wrong in a way nothing here could
explain.

It has already earned its keep twice. The first version of the route typed each file on its own and
disagreed 24 times over 76 mod folders -- a hair normal read as a hair mask, a lower diffuse as a
lower normal -- because its thresholds had been calibrated on the GAME's textures rather than on
repaints of them. The second, pairing a component's files against its roles by layout, got that to 1.
Only the third reached 0, and without this it would have shipped at 24.
"""
import argparse
import collections
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ModHashFixer import ModHashFixer                          # noqa: E402
from ModHashFixer.TextureTyper import TextureTyper             # noqa: E402


def modFolders(root: str):
    """Every mod folder under `root`, one level deep and at the root itself"""
    out = []
    for name in sorted(os.listdir(root)):
        path = os.path.join(root, name)
        if (os.path.isdir(path) and not name.upper().startswith("DISABLED")):
            out.append(path)
    return out


def main():
    parser = argparse.ArgumentParser(description = __doc__.split("\n")[0])
    parser.add_argument("root", help = "a folder holding mod folders")
    parser.add_argument("-r", "--recursive", action = "store_true",
                        help = "also treat each subfolder's subfolders as mods")
    args = parser.parse_args()

    mods = modFolders(args.root)
    if (args.recursive):
        for mod in list(mods):
            mods += modFolders(mod)

    counts = collections.Counter()
    rows = []
    for mod in mods:
        try:
            fixer = ModHashFixer(mod)
            name, _ = fixer.detect()
            if (not name):
                continue
            modType = fixer.characters()[name]
            typer = fixer.typer(modType)
            if (not typer):
                continue
            files = fixer._fileOfSection()
            typed = typer.assign(fixer._textureFiles())
        except Exception as e:
            print(f"  {os.path.basename(mod)}: {type(e).__name__}: {e}", file = sys.stderr)
            continue
        for path, raw in fixer._texts.items():
            for section, _, value in fixer._hashesIn(raw):
                known = fixer._resolve(modType, value)
                file = files.get(section.lower())
                if (not known or not file or known[0] in ModHashFixer.GeometryTypes):
                    continue
                guess = typed.get(TextureTyper.key(file)) if (fixer._fileIsFor(file, value)) else None
                if (not guess):
                    counts["abstain"] += 1
                elif (guess[0] == known[0]):
                    counts["agree"] += 1
                else:
                    counts["disagree"] += 1
                    rows.append((os.path.basename(mod), value, known[0], guess[0], os.path.basename(file)))

    print(f"\nfile route vs the hash history, over {len(mods)} mod folder(s):")
    print(f"  agree     {counts['agree']}")
    print(f"  abstain   {counts['abstain']}   (left unrecognised, which is the safe answer)")
    print(f"  DISAGREE  {counts['disagree']}")
    for mod, value, expected, got, file in rows:
        print(f"    {mod:26s} {value}  history says {expected:18s} file route says {got:18s} {file}")
    if (counts["disagree"]):
        print("\nA DISAGREEMENT IS A DEFECT: the same reasoning one file along writes a wrong hash.")
    sys.exit(1 if (counts["disagree"]) else 0)


if (__name__ == "__main__"):
    main()

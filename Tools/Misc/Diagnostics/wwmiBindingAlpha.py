#
# ===== wwmiBindingAlpha =====
#
# Every `ps-t` binding a fixed WuWa mod makes, resolved to the file it names, with that file's
# ALPHA CEILING -- the check that answers "is a glow waiting on some toggle?" without the game.
#
# On a WuWa body pass the diffuse's alpha is NOT opacity, it is a scalar the shader reads. Every
# body diffuse Chisa or ChisaParfait ships sits at a mode of 102 with a ceiling of 134; a mod whose
# author saved the texture fully opaque carries 255, which reads as that term at full and blooms
# the whole character red. `ChisaParfaitFixer::diffuseAlphaClampFilter` clamps it, but a clamp only
# reaches the bindings the fix knows about -- and the defect came back twice, each time through a
# binding nobody had listed:
#
#   * the first time through the mod's own carried `ps-t` line, which runs after the fix's list;
#   * the second through the OTHER branch of a toggle, where the edit had been built from one
#     variant and the other kept the raw file. Pressing the key brought the aura straight back.
#
# Both would have been one run of this. It reads the fixed `.ini` rather than the config, so it sees
# what the game will see: the fix's list, the carried lines, the copies, the downloads and every
# branch of every toggle.
#
# A raw binding above the band is not automatically a defect -- a later line on the same register
# may overwrite it before any draw. The script says where each binding sits so that can be judged,
# and `--strict` fails only on one that is still live at a draw.
#
# THE BAND AND THE REGISTER ARE PER CHARACTER, AND THE DEFAULTS HERE ARE CHISA'S. Measure them for
# the target before trusting a run: every body diffuse Chisa ships tops out at 119 and every one of
# ChisaParfait's at 134, but SANHUA's own textures carry alpha 255 with a mode of 255 -- 14 of her
# 18 large ones are above this default band. Run with Chisa's numbers against a Sanhua mod and it
# reports four defects that are not there. One pass over the target's download folder gives the
# real ceiling, and `--reg` has to name the register the TARGET reads its diffuse at, which is not
# the one the source binds it at.
#
#   py -3 wwmiBindingAlpha.py "<mod folder>"
#   py -3 wwmiBindingAlpha.py "<mod folder>" --fix ChisaRemapFix --band 134 --strict
#
# Written after the ChisaParfait3 red-aura rounds, 2026-10-03. See
# `AI Agent Help/CreatingRemaps/CLAUDE.md`, "A BODY DIFFUSE'S ALPHA IS A SCALAR" and "A TEXTURE
# TOGGLE HAS TWO SHAPES".
#

import argparse
import os
import pathlib
import re
import sys


def apiSrc():
    here = pathlib.Path(__file__).resolve()
    for parent in here.parents:
        src = parent / "Anime Game Remap (for all users)" / "api" / "src" / "py"
        if (src.is_dir()):
            return src

    raise SystemExit("could not find the API's package above %s" % here)


sys.path.insert(0, str(apiSrc()))
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(str(apiSrc() / "FixRaidenBoss2"))

from FixRaidenBoss2 import TextureFile     # noqa: E402


SectionLine = re.compile(r"^\[(.+)\]$")
FileLine = re.compile(r"^filename\s*=\s*(.+)$", re.IGNORECASE)
BindLine = re.compile(r"^(ps-t\d+)\s*=\s*(\S+)")
CondLine = re.compile(r"^(if|else|elif|endif)\b", re.IGNORECASE)
DrawLine = re.compile(r"^drawindexed\b", re.IGNORECASE)


def readMod(root):
    """Every resource's file, and every `ps-t` binding, in the order the `.ini` writes them."""
    files, binds = {}, []
    for path in sorted(pathlib.Path(root).rglob("*.ini")):
        try:
            lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        except OSError:
            continue

        section, drew = "", False
        path_, nextId = [], 0
        for line in lines:
            text = line.strip()
            found = SectionLine.match(text)
            if (found is not None):
                section, drew = found.group(1), False
                path_ = []
                continue

            found = FileLine.match(text)
            if (found is not None):
                files[section.lower()] = (path.parent / found.group(1).strip().replace("\\", "/"))
                continue

            found = CondLine.match(text)
            if (found is not None):
                word = found.group(1).lower()
                if (word == "if"):
                    nextId += 1
                    path_.append([nextId, 0])
                elif (word in ("else", "elif") and path_):
                    path_[-1][1] += 1
                elif (word == "endif" and path_):
                    path_.pop()

                continue

            if (DrawLine.match(text) is not None):
                drew = True
                continue

            found = BindLine.match(text)
            if (found is not None):
                binds.append({"section": section, "reg": found.group(1),
                              "resource": found.group(2), "drewBefore": drew,
                              "file": path.name,
                              "path": tuple(tuple(x) for x in path_)})

    return files, binds


def alphaCeiling(files, resource, cache):
    key = resource.lower()
    if (key in cache):
        return cache[key]

    path = files.get(key)
    top = None
    if (path is not None and path.is_file() and path.suffix.lower() == ".dds"):
        try:
            tex = TextureFile(str(path))
            tex.read()
            pixels = tex.getPixels()
            top = max(pixels[3::4]) if pixels else None
        except Exception:
            top = None

    cache[key] = top
    return top


def main():
    parser = argparse.ArgumentParser(description="alpha ceiling of every ps-t binding a fix writes")
    parser.add_argument("mod", help="the fixed mod's folder")
    parser.add_argument("--fix", default="RemapFix",
                        help="only sections whose name contains this (default: RemapFix)")
    parser.add_argument("--band", type=int, default=134,
                        help="the target's alpha ceiling for a body diffuse (default: 134)")
    parser.add_argument("--reg", default="ps-t2",
                        help="the register the target reads its diffuse at (default: ps-t2)")
    parser.add_argument("--all", action="store_true", help="print in-band bindings too")
    parser.add_argument("--strict", action="store_true",
                        help="exit 1 if an out-of-band binding is still live at a draw")
    args = parser.parse_args()

    print("band %d on %s -- measured for Chisa; see the header before using these on another pair"
          % (args.band, args.reg))

    files, binds = readMod(args.mod)
    binds = [b for b in binds if args.fix in b["section"]]
    if (not binds):
        raise SystemExit("no binding in a section containing %r -- is this mod fixed?" % args.fix)

    # WHICH WRITE A DRAW ACTUALLY SEES, WHICH IS NOT SIMPLY THE LAST ONE (2026-10-03). The first
    # cut of this took the last write per register per section and PASSED against the very bug it
    # was written for: the raw file sat in `if $key2 == 1` and the clamped copy in the `else`, so
    # the later line looked like it overwrote the earlier one when the two are alternatives and
    # only ever one of them runs.
    #
    # A later write B kills an earlier A only if B happens whenever A does -- that is, if B's
    # branch path is a PREFIX of A's. A sibling branch diverges at some conditional and kills
    # nothing; a write nested deeper than A is itself conditional and kills nothing either.
    live = set()
    for section in dict.fromkeys(b["section"] for b in binds):
        inSection = [b for b in binds if b["section"] == section]
        for i, bind in enumerate(inSection):
            killed = False
            for later in inSection[i + 1:]:
                if (later["reg"] != bind["reg"]):
                    continue

                if (bind["path"][:len(later["path"])] == later["path"]):
                    killed = True
                    break

            if (not killed):
                live.add(id(bind))

    cache, bad, shown = {}, 0, 0
    for bind in binds:
        top = alphaCeiling(files, bind["resource"], cache)
        over = (top is not None and top > args.band and bind["reg"] == args.reg)
        isLive = id(bind) in live
        if (over and isLive):
            bad += 1

        if (over or args.all):
            shown += 1
            note = ""
            if (over and isLive):
                note = "   <-- OUT OF BAND, and live at a draw on %s" % bind["reg"]
            elif (over):
                note = "   <-- out of band, but overwritten before any draw sees it"

            print("  %-7s %-50s alpha %s%s"
                  % (bind["reg"], bind["resource"][:50],
                     top if top is not None else "-", note))

    if (not shown):
        print("  every binding is inside the band (ceiling %d)" % args.band)

    print("\n%d binding(s) above the band on %s and still live" % (bad, args.reg))
    if (args.strict and bad):
        raise SystemExit(1)


if (__name__ == "__main__"):
    main()

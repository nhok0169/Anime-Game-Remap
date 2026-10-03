#
# ===== wwmiDrawArt =====
#
# Which BINDING GENERATION each `drawindexed` of a fixed WuWa mod belongs to, and whether the fix
# gave that generation the right art.
#
# A WWMI component section can hold two generations. ChisaParfait3 draws its hat, its chest
# bandages, its bows and its sleeves BEFORE the section binds a single `ps-t`:
#
#     [TextureOverrideComponent3]
#         run = CommandListOverrideSharedResources
#         if $key_4 == 1
#             drawindexed = 6009, 122481, 0      <- generation 0: whatever the GAME had bound
#         endif
#         ...
#         ps-t0 = ResourceTexture3_0             <- generation 1 starts here
#         ps-t3 = ResourceTexture3_3
#         drawindexed = 140979, 142263, 0        <- the custom body, on the MOD's art
#
# Generation 0 renders with the textures the game had bound when it matched the draw -- the SOURCE
# character's own atlas -- because the mod declares no `TextureOverride` by hash for them either.
# After a remap those are the TARGET's textures at the source's UVs, so the fix has to download the
# source's own and bind them. A fix that hangs ONE list off the shared-resource override gives every
# draw the mod's art instead, and the accessories render with the body atlas.
#
# WHAT THIS CHECKS, AND WHAT IT DOES NOT. It is structural: a generation-0 draw must be left on a
# file the fix derived from the source's DOWNLOAD, and anything else is reported. Deciding that from
# the pixels was tried first and does not work -- a texture edit is written uncompressed and the
# gamma round trip moves every pixel of it (mean |diff| 26 of 255 on ChisaParfait3's body atlas), so
# the same island measured through two edits is not comparable, and the island-contrast test that
# looked obvious PASSED against the build it was written to catch.
#
#   py -3 wwmiDrawArt.py "<fixed mod folder>" --fix ChisaRemapFix
#   py -3 wwmiDrawArt.py "<fixed mod folder>" --fix ChisaRemapFix --strict
#
# The names it matches (`RemapDL`, `RemapTex`) are the fix's OWN boilerplate, not a third party's --
# which is the one condition under which a keyword test over names is safe here.
#
# TWO THINGS IT GETS WRONG, BOTH MEASURED RATHER THAN GUESSED AT:
#
#   * `--reg` defaults to `ps-t2`, which is CHISA's diffuse register. Run it on the other direction
#     without changing that and it reads whatever sits at `ps-t2` there -- on Chisa17 that is the
#     flat detail map the fix invents, and all 16 of its draws are reported as defects that are not.
#     Set `--reg` to the register the TARGET of the run reads its diffuse at.
#   * a texture the fix CREATES (a flat neutral stood in for a role the source lacks) carries
#     neither mark and reads as the mod's art.
#
# What it gets right, on the corpus it was written against: 21 draws on ChisaParfait3 before the fix
# and 1 after, 0 on all 11 Sanhua mods and on 7 of the 9 Chisa ones, both before and after. The
# remaining one is ChisaParfait2, which declares no texture override by hash AND binds nothing in
# its sections -- 37 draws the game textured and the fix gives the mod's art. That is the same
# defect in its other shape and is NOT fixed yet: this change covers only a section that binds after
# it has already drawn.
#
# Written 2026-10-03. See `AI Agent Help/CreatingRemaps/CLAUDE.md`, "TWO BINDING GENERATIONS IN ONE
# SECTION".
#

import argparse
import os
import pathlib
import re
import sys


SectionLine = re.compile(r"^\[(.+)\]$")
FileLine = re.compile(r"^filename\s*=\s*(.+)$", re.I)
BindLine = re.compile(r"^(ps-t\d+)\s*=\s*(\S+)", re.I)
RunLine = re.compile(r"^run\s*=\s*(\S+)", re.I)
DrawLine = re.compile(r"^drawindexed\s*=\s*(\d+)\s*,\s*(\d+)", re.I)


def readIni(root):
    """{section: [(kind, payload)]} and {resource: file path}, over every .ini of the mod."""
    sections, files = {}, {}
    for path in sorted(pathlib.Path(root).rglob("*.ini")):
        section = ""
        for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
            text = line.strip()
            found = SectionLine.match(text)
            if (found is not None):
                section = found.group(1).lower()
                sections.setdefault(section, [])
                continue

            found = FileLine.match(text)
            if (found is not None):
                files[section] = path.parent / found.group(1).strip().replace("\\", "/")
                continue

            if (text.lower().startswith("hash")):
                sections.setdefault(section, []).append(("hash", ()))
                continue

            if (text.lower().startswith("this")):
                sections.setdefault(section, []).append(("this", ()))
                continue

            for kind, pattern in (("bind", BindLine), ("run", RunLine), ("draw", DrawLine)):
                found = pattern.match(text)
                if (found is not None):
                    sections.setdefault(section, []).append((kind, found.groups()))
                    break

    return sections, files


def replacesByHash(sections, slotPrefix):
    """Does the mod swap the game's textures by hash?

    `CheckTextureOverride = ps-tN` fires a `TextureOverride` keyed on the hash of whatever is bound
    there, and that section's `this =` puts the mod's file in its place -- so a mod declaring those
    gets its own art on every draw, bound or not, and has only ONE generation however its component
    sections are written. Every Sanhua mod declares 20 to 72; ChisaParfait3 declares none.
    """
    for name, items in sections.items():
        if (not name.startswith("textureoverride")
                or name.startswith("textureoverride" + slotPrefix.lower())):
            continue

        kinds = set(kind for kind, _ in items)
        if ("hash" in kinds and "this" in kinds):
            return True

    return False


def walk(sections, name, state, draws, own, depth=0):
    """Register state in file order, following `run =` into the lists.

    `own` tracks whether the SECTION ITSELF has bound a register yet -- a `run =` is the fix's own
    list and does not start generation 1, a direct `ps-t` line is the mod's and does.

    Conditions are ignored on purpose: this asks which file a draw is REACHED with, and an `if`
    choosing between two files of one role is still choosing between two files of one role.
    """
    if (depth > 8):
        return

    for kind, payload in sections.get(name.lower(), []):
        if (kind == "bind"):
            state[payload[0].lower()] = payload[1]
            if (depth == 0):
                own[0] = True
        elif (kind == "run"):
            walk(sections, payload[0], state, draws, own, depth + 1)
        elif (kind == "draw"):
            draws.append((int(payload[0]), int(payload[1]), dict(state), own[0]))


def main():
    parser = argparse.ArgumentParser(
        description="the binding generation of every draw a fixed WuWa mod makes")
    parser.add_argument("mod", help="the fixed mod's folder")
    parser.add_argument("--fix", default="RemapFix", help="the fix suffix (default: RemapFix)")
    parser.add_argument("--reg", default="ps-t2",
                        help="the target's diffuse register (default: ps-t2)")
    parser.add_argument("--all", action="store_true", help="print generation-1 draws too")
    parser.add_argument("--slot-prefix", dest="slotPrefix", default="Component",
                        help="what the source's slot sections are named (default: Component)")
    parser.add_argument("--strict", action="store_true",
                        help="exit 1 if a generation-0 draw is left on the mod's own art")
    args = parser.parse_args()

    sections, files = readIni(args.mod)
    roots = [s for s in sections if s.startswith("textureoverride")
             and args.fix.lower() in s and "commandlist" not in s]
    if (not roots):
        raise SystemExit("no remapped section containing " + args.fix + " -- is this mod fixed?")

    byHash = replacesByHash(sections, args.slotPrefix)
    if (byHash):
        print("this mod swaps the game's textures BY HASH, so every draw gets its own art "
              "whatever its sections bind -- there is only one generation here")
        print("")

    rows, bad = [], 0
    for root in sorted(roots):
        draws = []
        walk(sections, root, {}, draws, [False])
        for count, start, state, bound in draws:
            resource = state.get(args.reg.lower())
            path = files.get(resource.lower()) if resource is not None else None
            name = os.path.basename(str(path)) if path is not None else "(nothing bound)"

            # The fix's own two kinds of written file: a download of the source's game texture, and
            # an edit of one. An edit of the MOD's file carries neither mark.
            fromSource = "remapdl" in name.lower() or "game" in name.lower()
            wrong = (not bound) and resource is not None and not fromSource and not byHash
            bad += 1 if wrong else 0
            if (not bound) or args.all:
                rows.append((0 if not bound else 1, root, count, start, name, wrong))

    print("%-3s %-30s %8s %9s  %s" % ("gen", "section", "count", "start", "the file on " + args.reg))
    for gen, root, count, start, name, wrong in rows:
        print("%-3d %-30s %8d %9d  %s%s"
              % (gen, root[len("textureoverride"):][:30], count, start, name,
                 "   <-- the MOD's art on a draw the game textured" if wrong else ""))

    print("\n%d draw(s) before their section binds anything are left on the mod's own art" % bad)
    if (args.strict and bad):
        raise SystemExit(1)


if (__name__ == "__main__"):
    main()

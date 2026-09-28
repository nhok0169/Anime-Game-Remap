#
# ===== wwmiShaderTags =====
#
# Do two WuWa remaps disagree about a shader's `filter_index`?
#
# A WWMI fix tags the shaders of the passes it binds textures on -- `[ShaderOverride] hash = <shader>
# / filter_index = <value>` -- so its per-component texture command list can ask `if vs == <value>`
# and bind only on those draws. 3dmigoto keys a `[ShaderOverride]` by its shader hash across EVERY
# loaded `.ini`, and a shader holds ONE filter_index. So if two fixed mods tag the same shader with
# different values, whichever file 3dmigoto loads last wins and the other mod's condition never
# matches: its textures stay silently unbound, on a mod that was verified in game on its own.
#
# The guides record this between the two DIRECTIONS of one pair (the hair, face and eye shaders are
# usually the same on a skin and its character). It reaches further than that, because WuWa's are
# MATERIAL shaders and different characters share them -- measured: Chisa's face pixel shader
# `374a4f8fc9a5ea6a` is also Sanhua's.
#
# It reads FIXED `.ini` files rather than the configs, for two reasons: the value a config derives
# from `filterBase` / `filterStep` is not written in the config at all, and the artifact cannot drift
# from the template the way a re-derivation of its loop can.
#
#   py -3 wwmiShaderTags.py <folder>...          # any folders holding fixed mods, searched recursively
#   py -3 wwmiShaderTags.py <folder> --drawn <draw table>...
#
# `--drawn` takes the output of `wwmiDrawTable.py` and additionally reports every tagged shader that
# character is DRAWN with -- which is how the cross-character case is found rather than guessed, and
# which also answers "what value must a new direction use for this shader" in one line.
#
# It prints `NOTHING WAS READ` rather than a clean zero when it finds no tags, since a path that
# shattered and a corpus with no conflicts are otherwise the same output (Overview habit 66).
#

import argparse
import collections
import os
import re
import sys

ShaderOverride = re.compile(r'^\s*\[([^\]]*ShaderOverride[^\]]*)\]\s*$', re.I)
Section = re.compile(r'^\s*\[([^\]]+)\]\s*$')
Hash = re.compile(r'^\s*hash\s*=\s*([0-9a-fA-F]+)\s*$', re.I)
Filter = re.compile(r'^\s*filter_index\s*=\s*([0-9.]+)\s*$', re.I)
# a draw table row: draw, comp, startIdx, count, vs, ps
DrawRow = re.compile(r'^\s*\d+\s+\S+\s+\d+\s+\d+\s+([0-9a-f]{16})\s+([0-9a-f]{16})')


def readTags(path):
    """[(shader, filter index)] for every [ShaderOverride...] section of one .ini"""
    out = []
    name = None
    shader = None
    value = None

    def flush():
        if (name is not None and shader is not None and value is not None):
            out.append((shader.lower(), value))

    for line in open(path, "r", encoding = "utf-8", errors = "replace"):
        section = Section.match(line)
        if (section):
            flush()
            name = section.group(1) if (ShaderOverride.match(line)) else None
            shader = None
            value = None
            continue
        if (name is None):
            continue
        hit = Hash.match(line)
        if (hit):
            shader = hit.group(1)
            continue
        hit = Filter.match(line)
        if (hit):
            value = hit.group(1)
    flush()
    return out


def main():
    parser = argparse.ArgumentParser(description = "filter_index conflicts between WuWa remaps")
    parser.add_argument("folders", nargs = "+", help = "folders holding fixed mods")
    parser.add_argument("--drawn", nargs = "*", default = [],
                        help = "wwmiDrawTable.py outputs, to report which shaders a character draws with")
    args = parser.parse_args()

    # shader -> {value: [the .ini files saying so]}
    tags = collections.defaultdict(lambda: collections.defaultdict(list))
    files = 0
    for folder in args.folders:
        if (not os.path.isdir(folder)):
            print(f"  !! not a folder: {folder}")
            continue
        for root, dirs, names in os.walk(folder):
            dirs[:] = [d for d in dirs if (not d.upper().startswith("DISABLED"))]
            for name in names:
                if (not name.lower().endswith(".ini")):
                    continue
                path = os.path.join(root, name)
                found = readTags(path)
                if (found):
                    files += 1
                for shader, value in found:
                    tags[shader][value].append(path)

    if (not tags):
        print("  NOTHING WAS READ -- no [ShaderOverride] with a hash and a filter_index was found, "
              "which is not a pass. Check the folders are fixed mods.")
        return 1

    print(f"  {len(tags)} shaders tagged across {files} .ini file(s)")

    conflicts = {s: v for s, v in tags.items() if (len(v) > 1)}
    print(f"\n=== {len(conflicts)} shader(s) tagged with MORE THAN ONE value ===")
    for shader in sorted(conflicts):
        print(f"  {shader}")
        for value, paths in sorted(conflicts[shader].items()):
            print(f"      {value:<12} {len(paths)} file(s), e.g. {os.path.basename(os.path.dirname(paths[0]))}")
    if (not conflicts):
        print("  none -- every tagged shader carries one value everywhere")

    for table in args.drawn:
        drawn = set()
        for line in open(table, "r", encoding = "utf-8", errors = "replace"):
            hit = DrawRow.match(line)
            if (hit):
                drawn.add(hit.group(1))
                drawn.add(hit.group(2))
        shared = sorted(drawn & set(tags))
        print(f"\n=== {os.path.basename(table)}: {len(shared)} of its shaders are already tagged ===")
        for shader in shared:
            values = ", ".join(sorted(tags[shader]))
            print(f"  {shader}  ->  {values}")
        if (not shared):
            print("  none -- this character shares no shader with anything already fixed")

    return 1 if (conflicts) else 0


if (__name__ == "__main__"):
    sys.exit(main())

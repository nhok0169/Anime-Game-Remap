#
# ===== wwmiRabbitPatched =====
#
# Which pixel shaders does RabbitFX patch, and is a WuWa fix tagging any of them?
#
# A WWMI fix tags the shaders of the passes it binds textures on -- `[ShaderOverride] hash = <shader>
# / filter_index = <value>` -- so its texture command lists can ask `if ps == <value>` (or `if vs ==`,
# `WWMIFixerConfig::passVertexShaders`) and bind only on those draws. RabbitFX tags every pixel shader
# its `[ShaderRegex...]` patches `filter_index = 1718.1`, and `CommandList\RabbitFX\Run` /
# `...\SetTextures` act only `if ps == 1718.1`. 3dmigoto keys a `[ShaderOverride]` by shader hash
# across EVERY loaded `.ini` and a shader holds ONE filter_index -- so a fix that tags one of
# RabbitFX's pixel shaders switches RabbitFX off on it for every mod drawing with it, not only for the
# remapped one. That is what `passVertexShaders` exists to avoid.
#
# This answers it off the dumps already on disk rather than in game. A frame dump names every resource
# a command list bound AFTER that list, so a file
#
#   000057.0-[ShaderRegex_RabbitFX_Main]-ps-t0.dsc
#
# says draw 57 ran RabbitFX's regex command list, and that draw's own files carry
# `-vs=<hash>-ps=<hash>`. **Presence is proof; absence is not** -- all but one of RabbitFX's regexes
# have their `dump` lines commented out, so a patched pass can leave no trace in any dump.
#
#   py -3 wwmiRabbitPatched.py <WWMI folder or FrameAnalysis folder>...
#   py -3 wwmiRabbitPatched.py <WWMI folder> --tags <folder of fixed mods>...
#   py -3 wwmiRabbitPatched.py <WWMI folder> --shader 0f1752e476c3804d --shader ed1c0f8b2ba08ac4
#
# `--tags` reads the `[ShaderOverride]`s out of fixed `.ini` files (what a fix actually wrote, not
# what a config says) and reports every tagged shader RabbitFX is known to patch. With no `--tags`
# and no `--shader` it just lists what RabbitFX patches.
#
# `--vs` additionally prints, per pixel shader, the vertex shaders it is drawn with in each dump --
# which is the `passVertexShaders` map a config needs, and the reason to read it per dump is that a
# pass can run on a different vertex shader per component or per scene.
#

import argparse
import collections
import os
import re
import sys

Draw = re.compile(r"^(\d{6})[-.]")
Shaders = re.compile(r"-vs=([0-9a-f]{16})-ps=([0-9a-f]{16})")
RegexFile = re.compile(r"^(\d{6})\.\d+-\[(ShaderRegex_[A-Za-z0-9_]+)\]")
Section = re.compile(r"^\s*\[([^\]]+)\]\s*$")
HashLine = re.compile(r"^\s*hash\s*=\s*([0-9a-fA-F]+)\s*$", re.I)
FilterLine = re.compile(r"^\s*filter_index\s*=\s*([0-9.]+)\s*$", re.I)


def dumpFolders(paths):
    """Every FrameAnalysis folder under the given paths, and the paths that are ones themselves."""
    out = []
    for path in paths:
        name = os.path.basename(os.path.normpath(path))
        if (name.startswith("FrameAnalysis")):
            out.append(path)
            continue

        try:
            entries = sorted(os.listdir(path))
        except OSError as error:
            print(f"  cannot read {path}: {error}", file = sys.stderr)
            continue

        out += [os.path.join(path, entry) for entry in entries
                if entry.startswith("FrameAnalysis") and os.path.isdir(os.path.join(path, entry))]
    return out


def readDump(folder):
    """(draw -> (vs, ps), draw -> {regex names}) for one dump."""
    shaders, regexes = {}, collections.defaultdict(set)
    try:
        names = os.listdir(folder)
    except OSError:
        return shaders, regexes

    for name in names:
        drawn = Draw.match(name)
        if (not drawn):
            continue

        pair = Shaders.search(name)
        if (pair):
            shaders[drawn.group(1)] = (pair.group(1), pair.group(2))

        regex = RegexFile.match(name)
        if (regex):
            regexes[regex.group(1)].add(regex.group(2))

    return shaders, regexes


def taggedShaders(folders, withDisabled = False):
    """{shader hash: {filter_index: [.ini files]}} out of every fixed `.ini` under 'folders'.

    A `DISABLED*.ini` is skipped unless asked for: 3DMigoto does not load one, so a tag in it is not
    live -- and a parked copy of an older fix is exactly where a stale tag sits.
    """
    tags = collections.defaultdict(lambda: collections.defaultdict(list))
    for folder in folders:
        for root, _, files in os.walk(folder):
            for name in files:
                if (not name.lower().endswith(".ini")):
                    continue

                if (name.upper().startswith("DISABLED") and not withDisabled):
                    continue

                path = os.path.join(root, name)
                section, shader = "", None
                try:
                    lines = open(path, encoding = "utf-8", errors = "replace").read().splitlines()
                except OSError:
                    continue

                for line in lines:
                    header = Section.match(line)
                    if (header):
                        section, shader = header.group(1), None
                        continue

                    if ("shaderoverride" not in section.lower()):
                        continue

                    found = HashLine.match(line)
                    if (found):
                        shader = found.group(1).lower()
                        continue

                    value = FilterLine.match(line)
                    if (value and shader):
                        tags[shader][value.group(1)].append(path)

    return tags


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("dumps", nargs = "+",
                        help = "the WWMI folder (its FrameAnalysis-* subfolders are read), or single dump folders")
    parser.add_argument("--tags", nargs = "*", default = [],
                        help = "folders of FIXED mods, whose [ShaderOverride]s are checked against what RabbitFX patches")
    parser.add_argument("--shader", action = "append", default = [],
                        help = "a pixel shader hash to report on (repeatable)")
    parser.add_argument("--include-disabled", action = "store_true", dest = "withDisabled",
                        help = "also read DISABLED*.ini files, which 3DMigoto never loads")
    parser.add_argument("--vs", action = "store_true",
                        help = "also print the vertex shaders each pixel shader is drawn with, per dump")
    args = parser.parse_args()

    folders = dumpFolders(args.dumps)
    if (not folders):
        sys.exit("NOTHING WAS READ: no FrameAnalysis folder under " + ", ".join(args.dumps))

    patchedBy = collections.defaultdict(set)                      # ps -> {regex name}
    patchedIn = collections.defaultdict(set)                      # ps -> {dump}
    drawnIn = collections.defaultdict(set)                        # ps -> {dump}
    asVertex = collections.defaultdict(set)                       # vs -> {dump}
    vertexOf = collections.defaultdict(lambda: collections.defaultdict(set))   # ps -> dump -> {vs}
    files = 0

    for folder in folders:
        shaders, regexes = readDump(folder)
        files += len(shaders)
        dump = os.path.basename(os.path.normpath(folder))
        for draw, (vs, ps) in shaders.items():
            drawnIn[ps].add(dump)
            asVertex[vs].add(dump)
            vertexOf[ps][dump].add(vs)
            if (draw in regexes):
                patchedBy[ps] |= regexes[draw]
                patchedIn[ps].add(dump)

    if (not files):
        sys.exit("NOTHING WAS READ: no draw files in " + ", ".join(folders))

    print(f"{len(folders)} dump(s), {len(drawnIn)} distinct pixel shaders drawn, "
          f"{len(patchedBy)} of them seen running a RabbitFX regex command list")

    print("\n=== every pixel shader a dump shows RabbitFX patching ===")
    for ps in sorted(patchedBy):
        print(f"  {ps}  {','.join(sorted(patchedBy[ps]))}  in {len(patchedIn[ps])} dump(s)")

    asked = [s.lower() for s in args.shader]
    tags = taggedShaders(args.tags, args.withDisabled) if args.tags else {}
    for shader in tags:
        if (shader not in asked):
            asked.append(shader)

    if (not asked):
        return

    print("\n=== the shaders asked about ===")
    collisions = 0
    for ps in sorted(asked):
        if (ps in patchedBy):
            state = "RABBITFX PATCHES IT -- tagging its pixel shader switches RabbitFX off"
            collisions += 1 if ps in tags else 0
        elif (ps in drawnIn):
            state = "drawn, no RabbitFX file (not proof: most of its regexes do not dump)"
        elif (ps in asVertex):
            state = "a VERTEX shader -- RabbitFX patches pixel shaders, so tagging it is safe"
        else:
            state = "not in any dump read"

        values = ", ".join(sorted(tags.get(ps, {}))) if ps in tags else ""
        print(f"  {ps}  {state}")
        if (values):
            where = sorted({os.path.basename(f) for v in tags[ps].values() for f in v})
            print(f"        tagged {values} in {', '.join(where[:4])}"
                  f"{' ...' if len(where) > 4 else ''}")
        if (args.vs and ps in vertexOf):
            for dump in sorted(vertexOf[ps]):
                print(f"        {dump}: vs {', '.join(sorted(vertexOf[ps][dump]))}")

    if (tags):
        print(f"\n{collisions} of the {len(tags)} tagged shader(s) are ones RabbitFX patches"
              + ("" if collisions else " -- none, which is what a config using passVertexShaders looks like"))


if (__name__ == "__main__"):
    main()

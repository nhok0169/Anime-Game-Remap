#
# ===== wwmiCheckBlendRemap =====
#
# Is a WWMI mod's BLEND REMAP self-consistent -- does every weighted slot of every vertex end up on
# the bone the mod says it should?
#
# A WWMI `Blend.buf` names a bone in 8 bits, so a character whose merged skeleton passes 256 slots
# (Chisa: 420, Iuno: 413, Augusta: 375) cannot address her own bones. WWMI's answer is a per-component
# blend remap: the component's distinct bones, at most 256, get local ids 0..n-1;
# `BlendRemapForward.buf` holds 512 uint16 per remapped component (local -> merged),
# `BlendRemapReverse.buf` 512 per remap (merged -> local), and `BlendRemapVertexVG.buf` every
# vertex's FULL 16-bit ids. At load `BlendRemapper.hlsl` rewrites a private copy of `Blend.buf`
# through the reverse map and `SkeletonRemapper.hlsl` gathers that component's skeleton through the
# forward one, so the two maps have to be exact inverses over the bones the component uses.
#
# This runs both shaders' arithmetic over the written files and requires every weighted slot to land
# back on the bone `BlendRemapVertexVG.buf` names. That is the only check that distinguishes a remap
# which happens to be present from one that is right: the buffers are all the correct SIZE whatever
# is in them, and a swapped reverse entry moves a limb in game and nothing else.
#
# It is also the acceptance check for a FIX that writes a blend remap of its own -- a remap onto a
# target past 256 bones has to build all three buffers, and every part of it is silent when wrong.
#
#   py -3 wwmiCheckBlendRemap.py <mod folder>...
#   py -3 wwmiCheckBlendRemap.py <mod folder> --blend <name>      # a fix's own remapped blend
#
# `--blend` names the blend file to check instead of `Blend.buf` (a fix writes e.g.
# `ChisaRemapBlend.buf` beside the mod's own), and `--vertexVg` / `--forward` / `--reverse` the same
# for the three remap buffers.
#
# Exit code 0 only when something was actually checked and every check passed; a mod with NO blend
# remap says so and is not a pass or a failure (Overview habit 66).
#

import argparse
import glob
import os
import re
import sys

import numpy as np

RemapSize = 512          # entries per remap in the forward and reverse buffers


def findFile(folder, name):
    for root, _, files in os.walk(folder):
        if (name in files):
            return os.path.join(root, name)

    # WWMI Tools may prefix every buffer with the vb0 hash (`b41c509e-Index.buf`)
    for root, _, files in os.walk(folder):
        prefixed = [f for f in files if f.endswith("-" + name)]
        if (len(prefixed) == 1):
            return os.path.join(root, prefixed[0])
    return None


def findIni(folder):
    for root, _, files in os.walk(folder):
        for name in sorted(files):
            if (not name.lower().endswith(".ini")):
                continue
            path = os.path.join(root, name)
            if ("[TextureOverrideComponent" in open(path, "r", encoding = "utf-8",
                                                    errors = "replace").read()):
                return path
    return None


def readComponents(iniPath):
    """[(section name, [(index offset, index count)], the remap resource it selects or None)]

    The ranges are the section's own `drawindexed` lines, NOT `match_first_index` /
    `match_index_count`. Those two identify the GAME's draw call -- which component 3DMigoto is
    about to draw -- so they are identical in every mod of a character and address the GAME's index
    buffer, not the mod's. Slicing the mod's buffer with them reads an arbitrary set of vertices
    (2026-10-04): the game's windows end at 287358 indices, `Chisa1` has 328074 and draws its
    component 3 at offset 217983, `Chisa13` has 845550 and draws it at 153387 and 254379.

    **`ChisaIdentity` passed throughout and could not have failed** -- the identity mod IS the
    game's model, so the game's windows happen to address its own buffer, and its single
    `drawindexed = 108702, 63894, 0` is the match window exactly. One more check that the usual
    input cannot test.

    A component may carry SEVERAL draws (an author splitting it into toggled parts), and every
    toggle state has to be right, so all of them are taken. A commented-out draw is not one.

    Keyed by the section's full NAME, not by the component number in it: a FIXED mod carries both
    its own `[TextureOverrideComponent3]` and the fix's `[TextureOverrideComponent3<Target>RemapFix]`,
    on DIFFERENT windows, and keying by the number let the second silently replace the first --
    which reported a mod's windows as the other character's and read as "this mod selects no remap".
    """
    text = open(iniPath, "r", encoding = "utf-8", errors = "replace").read()
    out = []
    for block in re.finditer(r'\[([^\]]*TextureOverrideComponent\d+[^\]]*)\](.*?)(?=\n\[|\Z)',
                             text, re.S):
        name, body = block.group(1), block.group(2)
        # `drawindexed = <count>, <first>, 0` -- count FIRST. `auto` and anything non-numeric is
        # skipped rather than guessed at.
        draws = [(int(first), int(count)) for count, first in
                 re.findall(r'^[^;\n]*?\bdrawindexed\s*=\s*(\d+)\s*,\s*(\d+)\s*,',
                            body, re.M)]
        # which remap the section selects, if any
        override = re.search(r'ResourceBlendBufferOverride\s*=\s*ref\s+(\S+)', body, re.I)
        # A section with NO active draw is kept when it selects a remap: its slot is its POSITION
        # among the selecting sections, so dropping it would shift every slot after it -- and a
        # component whose only draw the author commented out is worth saying out loud rather than
        # omitting. One with neither a draw nor a resource is not a component.
        if (draws or override):
            out.append((name, draws, override.group(1) if (override) else None))
    return out


def drawsBypassingRemap(folder):
    """([remapped sections that draw], [those that never reach BlendRemapper]).

    Only meaningful for a blend written as LOCAL ids: such a blend is what `BlendRemapper` would
    write, so a draw that goes through it is served correctly, and a draw that reads the written
    buffer DIRECTLY indexes the merged skeleton with a local id -- a wrong bone, silently.

    Pools every .ini of the mod, because a `run =` crosses files: the fix splits the extra sections
    of a draw window into `<stem>RemapFix<n>.ini` beside the mod's own.
    """
    sections = {}
    for path in glob.glob(os.path.join(folder, "**", "*.ini"), recursive = True):
        base = os.path.basename(path)
        if (base.lower().startswith("disabled") or "BKUP" in base):
            continue

        current = None
        for line in open(path, "r", encoding = "utf-8", errors = "replace"):
            line = line.rstrip("\r\n")
            head = re.match(r'^\s*\[([^\]]+)\]\s*$', line)
            if (head):
                current = head.group(1)
                sections.setdefault(current, [])
                continue

            if (current is not None and not line.lstrip().startswith(";")):
                sections[current].append(line)

    def reaches(name, seen):
        if (name in seen):
            return False
        seen.add(name)

        for line in sections.get(name, []):
            if (re.search(r'run\s*=\s*CustomShader\\WWMIv1\\BlendRemapper', line, re.I)):
                return True
            called = re.match(r'^\s*run\s*=\s*(.+?)\s*$', line, re.I)
            if (called and reaches(called.group(1), seen)):
                return True
        return False

    drawing = [n for n, lines in sections.items()
               if ("remapfix" in n.lower()
                   and any(re.match(r'^\s*drawindexed\s*=', l, re.I) for l in lines))]
    return drawing, [n for n in drawing if (not reaches(n, set()))]


def check(folder, names, wanted = None):
    label = os.path.basename(os.path.normpath(folder))
    ini = findIni(folder)
    if (ini is None):
        print(f"  {label}: NOTHING WAS CHECKED -- no .ini with component sections")
        return None

    blendPath = findFile(folder, names["blend"])
    vgPath = findFile(folder, names["vertexVg"])
    fwdPath = findFile(folder, names["forward"])
    revPath = findFile(folder, names["reverse"])
    if (vgPath is None and fwdPath is None and revPath is None):
        print(f"  {label}: no blend remap (no {names['vertexVg']}) -- nothing to check, "
              f"which is correct for a character whose merged skeleton fits 8 bits")
        return None
    for what, path in (("blend", blendPath), ("vertexVg", vgPath), ("forward", fwdPath),
                       ("reverse", revPath)):
        if (path is None):
            print(f"  {label}: FAIL -- {names[what]} is missing while the others are present")
            return False

    blend = np.fromfile(blendPath, dtype = np.uint8)
    vertexVg = np.fromfile(vgPath, dtype = "<u2")
    forward = np.fromfile(fwdPath, dtype = "<u2")
    reverse = np.fromfile(revPath, dtype = "<u2")
    index = np.fromfile(findFile(folder, "Index.buf"), dtype = "<u4")

    if (len(blend) * 2 != len(vertexVg) * 2):
        # Blend.buf is (influences) index bytes + (influences) weight bytes a vertex; VertexVG is
        # (influences) uint16 a vertex -- so the two files are the SAME byte length
        pass
    if (len(blend) != len(vertexVg) * 2):
        print(f"  {label}: FAIL -- {names['blend']} is {len(blend)} bytes and "
              f"{names['vertexVg']} is {len(vertexVg) * 2}; they must be equal "
              f"(n indices + n weights of one byte against n uint16)")
        return False

    remapCount = len(forward) // RemapSize
    if (remapCount == 0 or len(forward) % RemapSize or len(reverse) != len(forward)):
        print(f"  {label}: FAIL -- forward is {len(forward)} entries and reverse {len(reverse)}; "
              f"both must be a whole multiple of {RemapSize}")
        return False

    comps = readComponents(ini)
    # a vertex's influence count: VertexVG is (influences) uint16 a vertex, and Blend.buf twice that
    # in bytes, so it comes from the vertex count -- which the index buffer's largest id bounds
    vertices = int(index.max()) + 1
    if (len(vertexVg) % vertices):
        print(f"  {label}: FAIL -- {len(vertexVg)} ids over {vertices} vertices is not a whole "
              f"number of influences a vertex")
        return False
    influences = len(vertexVg) // vertices
    ids = vertexVg.reshape(vertices, influences).astype(np.int64)
    lanes = blend.reshape(vertices, influences * 2)
    truncated = lanes[:, :influences].astype(np.int64)
    weights = lanes[:, influences:]

    print(f"  {label}: {vertices} vertices x {influences} influences, "
          f"{remapCount} remap(s), {len(comps)} component section(s)")

    ok = True
    # TWO legal forms, and asserting only the first failed every fix-written remap (2026-10-04).
    #
    #   * the TRUNCATED merged id (`ids & 0xFF`) -- the WWMI Tools convention, and what a component
    #     with NO remap reads, so it is safe unconditionally.
    #   * the LOCAL id (`reverse[trueId]`) -- what `BlendRemapper` recomputes into the private
    #     buffer, so its write is idempotent over it. A fix that owns one remap over the whole mesh
    #     writes this. Safe only while every remapped draw goes THROUGH the remapper, since one
    #     reading the written buffer directly would index the merged skeleton with a local id.
    #
    # Neither is "the" right answer; a blend matching neither is the bug.
    live = weights > 0
    total = int(live.sum())
    asTruncated = int(((truncated == (ids & 0xFF)) & live).sum())

    asLocal = -1
    if (remapCount == 1 and total):
        r = reverse.astype(np.int64)
        safe = ids.copy()
        safe[safe >= RemapSize] = 0            # out of range is caught separately, below
        asLocal = int(((truncated == r[safe]) & live).sum())

    if (total == 0):
        print(f"      NOTHING WAS CHECKED -- no weighted slot in {names['blend']}")
        return None

    if (asTruncated == total):
        print(f"      ok: every weighted slot of {names['blend']} is its 16-bit id truncated")
    elif (asLocal == total):
        drawing, bypassing = drawsBypassingRemap(folder)
        if (bypassing):
            ok = False
            print(f"      FAIL: {names['blend']} holds LOCAL ids, but {len(bypassing)} of "
                  f"{len(drawing)} remapped section(s) that draw never reach BlendRemapper, so "
                  f"they index the merged skeleton with a local id: "
                  f"{', '.join(bypassing[:3])}"
                  f"{' ...' if (len(bypassing) > 3) else ''}")
        elif (not drawing):
            print(f"      ok: every weighted slot of {names['blend']} is its LOCAL id "
                  f"(reverse[id]) -- but no remapped section draws in this folder, so whether "
                  f"they all reach BlendRemapper was NOT checked")
        else:
            print(f"      ok: every weighted slot of {names['blend']} is its LOCAL id "
                  f"(reverse[id]), and all {len(drawing)} remapped section(s) that draw reach "
                  f"BlendRemapper, which writes the same value")
    else:
        ok = False
        best = max(asTruncated, asLocal)
        print(f"      FAIL: {total - best} of {total} weighted slots of {names['blend']} are "
              f"NEITHER the truncated merged id ({asTruncated} match) nor the local id "
              f"reverse[id] ({asLocal if (asLocal >= 0) else 'n/a, several remaps'} match)")

    # ONE remap covering the whole mesh is the simpler and stronger case: every weighted slot of
    # every vertex has to round-trip through it, with no component windows involved at all. A fix
    # that shares one remap between components is doing this, and checking it per component would
    # only ever look at a subset.
    if (remapCount == 1):
        f = forward.astype(np.int64)
        r = reverse.astype(np.int64)
        live = weights > 0
        mine = ids[live]
        if (mine.size == 0):
            print("      NOTHING WAS CHECKED -- no weighted slot in the whole mesh")
            return None
        if (int(mine.max()) >= RemapSize):
            print(f"      FAIL: the mesh weights merged bone {int(mine.max())}, past the "
                  f"{RemapSize} a remap can address")
            return False

        wrong = int((f[r[mine]] != mine).sum())
        distinct = int(np.unique(mine).size)
        if (wrong):
            example = mine[f[r[mine]] != mine][:4]
            print(f"      FAIL: {wrong} of {mine.size} weighted slots of the MESH do not "
                  f"round-trip (e.g. bones {list(example)}), {distinct} distinct bones")
            return False

        print(f"      ok: one remap over the whole mesh, {distinct} distinct bones, "
              f"max {int(mine.max())}, every weighted slot of every vertex round-trips")
        return ok

    # and each remapped component must round-trip: forward[reverse[id]] == id, for the ids IT uses.
    # Which remap a section uses is the ORDER its ResourceRemappedBlendBufferComponent<n> appears in
    # CommandListInitializeBlendRemaps, which is the order the sections do -- so take them in file
    # order and read the remap slot off the resource name's own number where the file gives one.
    remapped = [(name, draws, res) for name, draws, res in comps if (res)]
    if (wanted is not None):
        # Named explicitly: the .ini does not select the remaps yet, so the set cannot be read off
        # it. Matched by the component NUMBER in the section's name, which is how a WWMI mod names
        # its slots.
        byNumber = {}
        for name, draws, res in comps:
            hit = re.search(r'TextureOverrideComponent(\d+)', name)
            if (hit and int(hit.group(1)) not in byNumber):
                byNumber[int(hit.group(1))] = (name, draws, res)
        remapped = [byNumber[c] for c in wanted if (c in byNumber)]
        missing = [c for c in wanted if (c not in byNumber)]
        if (missing):
            print(f"      !! no component section for {missing} -- not checked")

    if (not remapped):
        ok = False
        print(f"      FAIL: the three remap buffers are written and NO component section selects a "
              f"ResourceBlendBufferOverride -- every component reads the truncated ids")
    for slot, (name, draws, res) in enumerate(remapped):
        if (slot >= remapCount):
            ok = False
            print(f"      FAIL: [{name}] selects remap {slot}, past the {remapCount} "
                  f"the buffers hold")
            continue
        comp = name
        if (not draws):
            print(f"      [{name}] selects remap {slot} but makes no active draw "
                  f"(commented out?) -- NOT CHECKED")
            continue

        # the union over every draw the section makes -- each toggle state has to be right
        verts = np.unique(np.concatenate([index[f: f + c] for f, c in draws]))
        verts = verts[verts < vertices]
        used = ids[verts]
        live = weights[verts] > 0
        f = forward[slot * RemapSize:(slot + 1) * RemapSize].astype(np.int64)
        r = reverse[slot * RemapSize:(slot + 1) * RemapSize].astype(np.int64)

        mine = used[live]
        if (mine.size == 0):
            print(f"      component {comp} -> remap {slot}: no weighted bone, nothing to check")
            continue
        if (int(mine.max()) >= RemapSize):
            ok = False
            print(f"      FAIL: component {comp} weights merged bone {int(mine.max())}, past the "
                  f"{RemapSize} a remap can address")
            continue

        back = f[r[mine]]
        wrong = int((back != mine).sum())
        distinct = int(np.unique(mine).size)
        if (wrong):
            ok = False
            example = mine[back != mine][:4]
            print(f"      FAIL: component {comp} -> remap {slot}: {wrong} of {mine.size} weighted "
                  f"slots do not round-trip (e.g. bones {list(example)}), {distinct} distinct bones")
        else:
            print(f"      ok: component {comp} -> remap {slot}, {distinct} distinct bones, "
                  f"max {int(mine.max())}, every weighted slot round-trips")

    return ok


parser = argparse.ArgumentParser(description = "is a WWMI mod's blend remap self-consistent")
parser.add_argument("mods", nargs = "+")
parser.add_argument("--blend", default = "Blend.buf")
parser.add_argument("--vertexVg", default = "BlendRemapVertexVG.buf")
parser.add_argument("--forward", default = "BlendRemapForward.buf")
parser.add_argument("--reverse", default = "BlendRemapReverse.buf")
parser.add_argument("--components", default = None,
                    help = "comma-separated component numbers that carry a remap, in order -- for a "
                           "fix whose .ini wiring is not written yet, where no section selects a "
                           "ResourceBlendBufferOverride to read the set off")
args = parser.parse_args()

names = {"blend": args.blend, "vertexVg": args.vertexVg, "forward": args.forward,
         "reverse": args.reverse}
wanted = None
if (args.components):
    wanted = [int(c) for c in args.components.split(",") if (c.strip())]

results = [check(mod, names, wanted) for mod in args.mods]
checked = [r for r in results if (r is not None)]
if (not checked):
    print("\n  NOTHING WAS CHECKED -- no mod here carries a blend remap. That is not a pass.")
    sys.exit(2)

print(f"\n  {sum(1 for r in checked if r)} of {len(checked)} mod(s) with a blend remap passed")
sys.exit(0 if (all(checked)) else 1)

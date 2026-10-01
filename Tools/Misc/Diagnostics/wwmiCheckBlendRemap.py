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
import os
import re
import sys

import numpy as np

RemapSize = 512          # entries per remap in the forward and reverse buffers


def findFile(folder, name):
    for root, _, files in os.walk(folder):
        if (name in files):
            return os.path.join(root, name)
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
    """[(section name, index offset, index count, the remap resource it selects or None)]

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
        first = re.search(r'^\s*match_first_index\s*=\s*(\d+)', body, re.M)
        count = re.search(r'^\s*match_index_count\s*=\s*(\d+)', body, re.M)
        # which remap the section selects, if any
        override = re.search(r'ResourceBlendBufferOverride\s*=\s*ref\s+(\S+)', body, re.I)
        if (first and count):
            out.append((name, int(first.group(1)), int(count.group(1)),
                        override.group(1) if (override) else None))
    return out


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
    # Blend.buf must hold the merged ids TRUNCATED -- that is what a component with no remap reads
    bad = int(((truncated != (ids & 0xFF)) & (weights > 0)).sum())
    if (bad):
        ok = False
        print(f"      FAIL: {bad} weighted slots where {names['blend']} is not "
              f"{names['vertexVg']} & 0xFF")
    else:
        print(f"      ok: every weighted slot of {names['blend']} is its 16-bit id truncated")

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
    remapped = [(name, first, count, res) for name, first, count, res in comps if (res)]
    if (wanted is not None):
        # Named explicitly: the .ini does not select the remaps yet, so the set cannot be read off
        # it. Matched by the component NUMBER in the section's name, which is how a WWMI mod names
        # its slots.
        byNumber = {}
        for name, first, count, res in comps:
            hit = re.search(r'TextureOverrideComponent(\d+)', name)
            if (hit and int(hit.group(1)) not in byNumber):
                byNumber[int(hit.group(1))] = (name, first, count, res)
        remapped = [byNumber[c] for c in wanted if (c in byNumber)]
        missing = [c for c in wanted if (c not in byNumber)]
        if (missing):
            print(f"      !! no component section for {missing} -- not checked")

    if (not remapped):
        ok = False
        print(f"      FAIL: the three remap buffers are written and NO component section selects a "
              f"ResourceBlendBufferOverride -- every component reads the truncated ids")
    for slot, (name, first, count, res) in enumerate(remapped):
        if (slot >= remapCount):
            ok = False
            print(f"      FAIL: [{name}] selects remap {slot}, past the {remapCount} "
                  f"the buffers hold")
            continue
        comp = name
        verts = np.unique(index[first: first + count])
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

#
# ===== identityVsDump =====
#
# The acceptance test for an identity mod (Tools/Misc/Prototypes/identityMod.py): every buffer it wrote
# is compared BYTE FOR BYTE with the game's own buffer of the same hash in a frame dump -- never with
# the asset dump text the writer itself parsed, which would validate it in a circle (Vertex Group
# Remaps' Yelan recipe). For each component of the character's hash.json:
#
#   <Name><Comp>Position.buf   vs the dump's  *-vb0=<position_vb>*.buf   (the pre-skinning pass)
#   <Name><Comp>Blend.buf      vs             *-vb1=<blend_vb>*.buf
#   <Name><Comp>Texcoord.buf   vs             *-vb1=<texcoord_vb>*.buf   (a drawn pass)
#   <Name><Comp><Obj>.ib       vs             *-ib=<ib>*.buf, the object's own range, widened to R32
#
#   python identityVsDump.py <hash.json> <identity mod folder> <FrameAnalysis folder> [--name <Name>]
#
# Exits non-zero on any difference, and ALSO when nothing could be compared (a missing dump file is
# reported, never skipped): "0 differ" over zero files is not a pass.
#

import argparse
import glob
import json
import os
import re
import sys

import numpy as np


def dumpFile(folder: str, slot: str, value: str):
    hits = sorted(glob.glob(os.path.join(glob.escape(folder), f"*-{slot}={value}*.buf")))
    return hits[0] if hits else None


def ibFormat(folder: str, bufPath: str) -> str:
    txt = bufPath[:-4] + ".txt"
    if os.path.exists(txt):
        head = open(txt, encoding="utf-8", errors="replace").read(400)
        m = re.search(r"format:\s*(\S+)", head)
        if m:
            return m.group(1)
    return "DXGI_FORMAT_R32_UINT"


def main():
    p = argparse.ArgumentParser(description="compare an identity mod's buffers with a frame dump's")
    p.add_argument("hashJson"); p.add_argument("mod"); p.add_argument("dump")
    p.add_argument("--name", default=None, help="the identity mod's file prefix (default: the mod's .ini name)")
    a = p.parse_args()

    name = a.name or os.path.splitext(os.path.basename(glob.glob(os.path.join(glob.escape(a.mod), "*.ini"))[0]))[0]
    entries = [e for e in json.load(open(a.hashJson, encoding="utf-8")) if e.get("position_vb") and e.get("blend_vb")]
    same = differ = missing = 0

    def check(label, ours, theirs):
        nonlocal same, differ, missing
        if ours is None or theirs is None:
            missing += 1
            print(f"MISSING  {label}: {'our file' if ours is None else 'dump file'} not found")
            return
        if ours == theirs:
            same += 1
            print(f"same     {label} ({len(ours)} bytes)")
        else:
            differ += 1
            print(f"DIFFER   {label}: ours {len(ours)} bytes, game {len(theirs)} bytes")

    def ours(file):
        path = os.path.join(a.mod, file)
        return open(path, "rb").read() if os.path.exists(path) else None

    for e in entries:
        comp = e.get("component_name", "")
        for part, slot, key in (("Position", "vb0", "position_vb"), ("Blend", "vb1", "blend_vb"), ("Texcoord", "vb1", "texcoord_vb")):
            f = dumpFile(a.dump, slot, e[key])
            check(f"{comp}{part} ({key} {e[key]})", ours(f"{name}{comp}{part}.buf"), open(f, "rb").read() if f else None)
        f = dumpFile(a.dump, "ib", e["ib"])
        game = None
        if f:
            dtype = np.uint16 if "R16" in ibFormat(a.dump, f) else np.uint32
            game = np.frombuffer(open(f, "rb").read(), dtype=dtype).astype(np.uint32)
        firsts = list(e["object_indexes"])
        counts = e.get("object_index_counts")
        for i, obj in enumerate(e["object_classifications"]):
            mine = ours(f"{name}{comp}{obj}.ib")
            theirs = None
            if game is not None and mine is not None:
                first = firsts[i]
                count = counts[i] if counts else ((firsts[i + 1] if i + 1 < len(firsts) else len(game)) - first)
                theirs = game[first:first + count].tobytes()
            check(f"{comp}{obj}.ib (ib {e['ib']} from {firsts[i]})", mine, theirs)

    print(f"\n{same} same, {differ} differ, {missing} missing")
    if same == 0:
        print("NOTHING WAS COMPARED -- that is not a pass")
    sys.exit(0 if (same and not differ and not missing) else 1)


if __name__ == "__main__":
    main()

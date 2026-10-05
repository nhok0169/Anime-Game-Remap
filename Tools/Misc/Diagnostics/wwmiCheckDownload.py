#
# ===== wwmiCheckDownload =====
#
# Verifies a WuWa download folder a SECOND way -- against the frame dump's own bytes, not against the
# reader that built it. Creating Remaps' "Proving a NEW download folder" asks for exactly this, and
# the reason is that `wwmiDownloadFolder.py --check` re-runs `wwmiIdentityMod.py` over WWMI Tools'
# extraction, so it can only ever say that the same reader read the same dump the same way.
#
# 3DMigoto writes each bound vertex and index buffer WHOLE, once per draw call, as
# `<call>-vb<N>=<hash>-vs=..-ps=...buf`, and every draw of one character binds the same ones. So the
# character's own buffers must appear byte for byte among the slot files of the calls that bind its
# vb0 -- with three deliberate exceptions the tool names rather than fails on:
#
#   Blend.buf   the extractor rewrites each component's LOCAL bone ids into the merged skeleton, so
#               it is not the raw `vb4` (47.7% of ChisaParfait's bytes differ from it)
#   ShapeKey*   rebuilt sparse from the dense per-vertex offset stream
#
# It also prints which slot each buffer came from and that slot's hash, which is what a `HashData`
# row needs -- on ChisaParfait at 3.7 that is `vb0=95ecef77` (the row that moved), `vb1=d3e7581e`,
# `vb2=c21e0513`, `vb3=ee32541e`, `ib=65023833`.
#
#   py -3 wwmiCheckDownload.py <FrameAnalysis folder> <vb0 hash> <download folder> --name ChisaParfait
#

import argparse
import hashlib
import os
import re

# what is derived rather than bound, and why -- reported, not counted as a failure
Derived = {"Blend.buf": "the extractor remaps each component's local bone ids into the merged skeleton",
           "ShapeKeyOffset.buf": "rebuilt sparse from the dense per-vertex stream",
           "ShapeKeyVertexId.buf": "rebuilt sparse from the dense per-vertex stream",
           "ShapeKeyVertexOffset.buf": "rebuilt sparse from the dense per-vertex stream",
           "BlendRemapVertexVG.buf": "WWMI's own blend remap, built from the merged vg map",
           "BlendRemapForward.buf": "WWMI's own blend remap, built from the merged vg map",
           "BlendRemapReverse.buf": "WWMI's own blend remap, built from the merged vg map"}
Buffers = ("Position.buf", "Vector.buf", "Color.buf", "Texcoord.buf", "Index.buf", "Blend.buf",
           "ShapeKeyOffset.buf", "ShapeKeyVertexId.buf", "ShapeKeyVertexOffset.buf",
           "BlendRemapVertexVG.buf", "BlendRemapForward.buf", "BlendRemapReverse.buf")


def digestOf(path: str) -> str:
    with open(path, "rb") as handle:
        return hashlib.md5(handle.read()).hexdigest()


def slotsOfObject(dump: str, vb0: str):
    """{md5 of the bytes: {(slot, hash)}} over every buffer the draws of `vb0` bound"""
    calls = {match.group(1) for match in
             (re.match(r"^(\d{6})-vb0=([0-9a-f]{8})-", name) for name in os.listdir(dump))
             if (match is not None and match.group(2) == vb0)}
    if (not calls):
        raise SystemExit(f"no draw call in '{dump}' binds vb0={vb0} -- is this the right dump, and did "
                         f"the character's hash move?")

    byDigest = {}
    for name in sorted(os.listdir(dump)):
        match = re.match(r"^(\d{6})-(vb\d|ib)=([0-9a-f]{8})[^-]*-", name)
        if (match is None or match.group(1) not in calls or not name.endswith(".buf")):
            continue
        byDigest.setdefault(digestOf(os.path.join(dump, name)), set()).add((match.group(2), match.group(3)))
    return len(calls), byDigest


def main():
    parser = argparse.ArgumentParser(description = "a WuWa download folder against the frame dump's own bytes")
    parser.add_argument("dump", help = "the FrameAnalysis-* folder the download folder was built from")
    parser.add_argument("vb0", help = "the character's vb0 hash, as the extraction's subfolder names it")
    parser.add_argument("folder", help = "the download folder, e.g. 'Data/Mod Downloads/WuWa/ChisaParfait/3_7'")
    parser.add_argument("--name", required = True, help = "the character's ModType name, the prefix of every file")
    args = parser.parse_args()

    calls, byDigest = slotsOfObject(args.dump, args.vb0.lower())
    print(f"{calls} draw call(s) bind vb0={args.vb0.lower()}; {len(byDigest)} distinct buffer(s) among their slots\n")

    matched, derived, unexplained = 0, 0, []
    for suffix in Buffers:
        path = os.path.join(args.folder, args.name + suffix)
        if (not os.path.isfile(path)):
            continue

        where = byDigest.get(digestOf(path))
        if (where is not None):
            matched += 1
            print(f"  OK       {suffix:26s} = {', '.join(sorted(slot + '=' + value for slot, value in where))}")
        elif (suffix in Derived):
            derived += 1
            print(f"  derived  {suffix:26s}   {Derived[suffix]}")
        else:
            unexplained.append(suffix)
            print(f"  UNKNOWN  {suffix:26s}   matches no slot the dump bound, and is not a derived buffer")

    print(f"\n{matched} buffer(s) byte-identical to a slot the dump bound, {derived} derived by the "
          f"extractor, {len(unexplained)} unexplained")
    if (unexplained):
        raise SystemExit(f"FAIL: {', '.join(unexplained)}")


if (__name__ == "__main__"):
    main()

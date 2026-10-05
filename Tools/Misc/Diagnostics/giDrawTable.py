#
# ===== giDrawTable =====
#
# The per-draw SLOT TABLE of a GI frame dump (GIMI's FrameAnalysis-* folder): for every DrawIndexed
# whose index buffer is one of the given hashes, its start index / index count, the shaders, the
# vertex buffers and the texture bound in each of ps-t0..ps-t3 -- read off the dump's own FILE NAMES
# (000072-ps-t2=f485d1ea-vs=...-ps=....dds) and log.txt's DrawIndexed line, never reasoned about.
# The GI form of wwmiDrawTable.py, and the instrument Creating Remaps' "A TextureOverride binds
# registers only for the draw its hash matches" asks for.
#
# Also lists the character's PRE-SKINNING passes (the draws on the pose shader, vs=653c63ba4a73ca8b
# for GI characters): their vb0 is the position buffer and vb1 the blend buffer a GIMI mod's
# [TextureOverride...Position] / [...Blend] match, so a component whose own draws show no blend_vb
# is skinned here or not at all.
#
#   python giDrawTable.py <FrameAnalysis folder> --ib f117984b --ib d44b2c85 ...
#   python giDrawTable.py <FrameAnalysis folder> --ib f117984b --distinct     one row per distinct binding
#
# The dump writes a texture to a file only when the draw's shader reads it, and 3DMigoto names what
# is bound AT that draw, so a slot missing from a row was not bound to anything the dump recorded.
#

import argparse
import os
import re
from collections import OrderedDict

FilePattern = re.compile(r"^(?P<draw>\d{6})-(?P<slot>ib|vb\d|ps-t\d+|vs-t\d+)=(?P<hash>[0-9a-f]{8}|!S!=[0-9a-f]{8})-vs=(?P<vs>[0-9a-f]{16})(?:-ps=(?P<ps>[0-9a-f]{16}))?\.(?:txt|buf|dds|jpg)$")
DrawIndexedPattern = re.compile(r"^(\d{6}) DrawIndexed\(IndexCount:(\d+), StartIndexLocation:(\d+), BaseVertexLocation:(-?\d+)\)")
PoseShader = "653c63ba4a73ca8b"
TexSlots = ("ps-t0", "ps-t1", "ps-t2", "ps-t3")


def readDraws(folder: str):
    draws = OrderedDict()
    for name in sorted(os.listdir(folder)):
        m = FilePattern.match(name)
        if (m is None):
            continue
        d = draws.setdefault(m.group("draw"), {"vs": m.group("vs"), "ps": m.group("ps"), "slots": {}})
        d["slots"].setdefault(m.group("slot"), m.group("hash").replace("!S!=", ""))
    with open(os.path.join(folder, "log.txt"), "r", encoding = "utf-8", errors = "replace") as f:
        for line in f:
            m = DrawIndexedPattern.match(line)
            if (m and (m.group(1) in draws)):
                draws[m.group(1)]["drawIndexed"] = (int(m.group(2)), int(m.group(3)), int(m.group(4)))
    return draws


def main():
    parser = argparse.ArgumentParser(description = "a GI frame dump's per-draw slot table")
    parser.add_argument("frame", help = "the FrameAnalysis-* folder")
    parser.add_argument("--ib", action = "append", default = [], help = "an index buffer hash to report (repeatable)")
    parser.add_argument("--distinct", action = "store_true", help = "one row per distinct (ib, start, shaders, buffers, textures), with how many draws share it")
    args = parser.parse_args()
    draws = readDraws(args.frame)

    print("pre-skinning passes (vs=%s):" % PoseShader)
    poses = OrderedDict()
    for num, d in draws.items():
        if ((d["vs"] == PoseShader) and ("ib" not in d["slots"])):
            key = (d["slots"].get("vb0"), d["slots"].get("vb1"), d["slots"].get("vb2"))
            poses.setdefault(key, []).append(num)
    for (vb0, vb1, vb2), nums in poses.items():
        print(f"  vb0={vb0} vb1={vb1} vb2={vb2}  draws {', '.join(nums[:6])}{' ...' if len(nums) > 6 else ''}")

    wanted = set(args.ib)
    rows = OrderedDict()
    for num, d in draws.items():
        ib = d["slots"].get("ib")
        if ((ib is None) or (wanted and (ib not in wanted))):
            continue
        count, start, base = d.get("drawIndexed", (None, None, None))
        tex = " ".join(f"{s[3:]}={d['slots'].get(s, '-')}" for s in TexSlots)
        vbs = " ".join(f"{s}={d['slots'][s]}" for s in ("vb0", "vb1", "vb2") if s in d["slots"])
        key = (ib, start, count, d["vs"], d["ps"], vbs, tex) if args.distinct else num
        rows.setdefault(key, {"nums": [], "row": (ib, start, count, d["vs"], d["ps"], vbs, tex)})["nums"].append(num)
    print("\nindexed draws:")
    for entry in rows.values():
        ib, start, count, vs, ps, vbs, tex = entry["row"]
        nums = entry["nums"]
        label = f"{nums[0]}" + (f" (+{len(nums) - 1})" if (len(nums) > 1) else "")
        print(f"  {label:<12} ib={ib} first={start} count={count} vs={vs} ps={ps}  {vbs}  {tex}")


if (__name__ == "__main__"):
    main()

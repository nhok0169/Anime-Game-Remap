r"""Is an anchor chain SAFE for this part -- i.e. are the bones it would flatten the part's own?

`AnchorChains` remaps every member to whatever the root maps to, and its key space is the SOURCE
skeleton GLOBALLY, not per component. So a member that another component also weights is pinned in
that component too. The field's two worked examples (Chisa's fox mask and her back skirt panel) are
parts with bones of their own; nothing checks that precondition, and the failure is silent and
total -- the torso welded to one bone.

For each part, prints how much of the chain's weight is shared with another drawn component, and
refuses to recommend a chain when any is.

  py -3 anchorSafety.py <fixed mod folder> --part <start> <count> --others <start> <count> ...
"""
import argparse
import os
import struct
import sys


def bonesOf(mod, start, count, inf=8):
    meshes = os.path.join(mod, "Meshes")
    blend = open(os.path.join(meshes, "Blend.buf"), "rb").read()
    index = open(os.path.join(meshes, "Index.buf"), "rb").read()
    stride = inf * 2
    vertices = len(blend) // stride

    used = {}
    for k in range(count):
        at = start + k
        if 4 * at + 4 > len(index):
            continue
        v = struct.unpack_from("<I", index, 4 * at)[0]
        if v >= vertices:
            continue
        for b in range(inf):
            w = blend[v * stride + inf + b]
            if w:
                used[blend[v * stride + b]] = used.get(blend[v * stride + b], 0) + w
    return used


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("mod")
    ap.add_argument("--part", nargs=2, type=int, required=True)
    ap.add_argument("--others", nargs="+", type=int, required=True,
                    help="start count start count ... for every OTHER drawn component")
    ap.add_argument("--label", default="the part")
    args = ap.parse_args()

    part = bonesOf(args.mod, *args.part)
    others = {}
    it = iter(args.others)
    for start in it:
        count = next(it)
        for b, w in bonesOf(args.mod, start, count).items():
            others[b] = others.get(b, 0) + w

    total = sum(part.values()) or 1
    shared = {b: w for b, w in part.items() if b in others}
    sharedWeight = sum(shared.values())

    print(f"{args.label}: {len(part)} source bone(s), {len(shared)} also used by another component")
    print(f"  {100.0 * sharedWeight / total:.1f}% of the part's weight sits on shared bones")
    for b, w in sorted(shared.items(), key=lambda kv: -kv[1])[:10]:
        print(f"    source {b:3d}  {100.0 * w / total:5.1f}% of the part, "
              f"and {others[b]} weight elsewhere")

    if shared:
        print("\n  UNSAFE: an anchor chain over these pins them in the OTHER component too.")
        print("  AnchorChains' key space is the source skeleton globally, not per component.")
        return 1

    print("\n  safe: every bone of the part is the part's own")
    return 0


if __name__ == "__main__":
    sys.exit(main())

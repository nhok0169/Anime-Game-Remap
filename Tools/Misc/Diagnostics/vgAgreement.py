"""Where do a pair's TWO vertex group rows disagree about which bone is which part?

The two directions are separate rows and neither is the inverse of the other (invariant 2): several
source groups may share one target, so no inverse exists. But they are descriptions of the SAME two
models, so wherever both have an opinion they should name the same correspondence -- and when one
direction is confirmed in game and the other is an unreviewed finder proposal, the confirmed one is
the best oracle the unreviewed one has.

For every row ``s -> t`` of the row under review, this asks what the OTHER direction says about
``t``:

* ``t -> s``            the two agree outright
* ``t -> s'``           they disagree, and ``s'`` is reported with how far it sits from ``s``
* ``t`` has no row      the other direction never maps anything to it -- no opinion

A disagreement is not automatically a fault: a target several sources share can only send back one
of them. What it is, is the shortlist worth reading by hand, ordered by how far apart the two
answers are -- a few units is two bones of one part, and tens of units is two different parts.

It found the opposite of a fault on ChisaParfait -> Chisa (2026-09-28): ten of the skin's
component-3 bones map into Chisa's component 5, which the component grid flags as body-onto-prop,
and the forward row maps Chisa's 409-418 onto exactly those ten. The two characters file one prop
under different components; the crossing is correct and the grid cannot know that.

Usage::

    py -3 Tools/Misc/Diagnostics/vgAgreement.py <row under review> <the other direction's row>
        [--src <src identity mod>] [--tgt <tgt identity mod>]

Each row file is ``<source> <target>`` pairs, one per line. With the identity mods it also prints
how far apart the two answers sit, which is what separates "two bones of one part" from "two parts".
"""
import argparse
import os
import struct
import sys


def readRow(path):
    out = {}
    for line in open(path):
        parts = line.split()
        if len(parts) == 2 and parts[0].lstrip("-").isdigit():
            out.setdefault(int(parts[0]), int(parts[1]))
    return out


def centroids(mod, influences=8):
    if mod is None:
        return {}

    meshes = os.path.join(mod, "Meshes")
    blend = open(os.path.join(meshes, "Blend.buf"), "rb").read()
    pos = open(os.path.join(meshes, "Position.buf"), "rb").read()
    stride = influences * 2
    n = len(blend) // stride
    acc = {}
    for v in range(n):
        if 12 * v + 12 > len(pos):
            break
        x, y, z = struct.unpack_from("<fff", pos, 12 * v)
        for b in range(influences):
            w = blend[v * stride + influences + b]
            if not w:
                continue
            bone = blend[v * stride + b]
            a = acc.setdefault(bone, [0.0, 0.0, 0.0, 0])
            a[0] += x * w
            a[1] += y * w
            a[2] += z * w
            a[3] += w
    return {b: (a[0] / a[3], a[1] / a[3], a[2] / a[3]) for b, a in acc.items() if a[3]}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("row")
    ap.add_argument("other")
    ap.add_argument("--src", default=None, help="the identity mod of the row's SOURCE")
    ap.add_argument("--tgt", default=None, help="...and of its target (unused, kept for symmetry)")
    ap.add_argument("--top", type=int, default=20)
    args = ap.parse_args()

    row = readRow(args.row)
    other = readRow(args.other)
    sc = centroids(args.src)

    agree = 0
    noOpinion = 0
    disagree = []
    for s, t in sorted(row.items()):
        back = other.get(t)
        if back is None:
            noOpinion += 1
        elif back == s:
            agree += 1
        else:
            d = None
            if s in sc and back in sc:
                d = sum((a - b) ** 2 for a, b in zip(sc[s], sc[back])) ** 0.5
            disagree.append((d if d is not None else -1.0, s, t, back))

    total = len(row)
    print(f"{total} rows under review")
    print(f"  {agree:4d} agree outright  ({100.0 * agree / total:.0f}%)")
    print(f"  {noOpinion:4d} the other direction maps nothing to that target -- no opinion")
    print(f"  {len(disagree):4d} disagree\n")

    known = [d for d in disagree if d[0] >= 0]
    known.sort(reverse=True)
    print(f"the {min(args.top, len(known))} disagreements whose two answers sit FURTHEST apart:")
    for d, s, t, back in known[:args.top]:
        print(f"    src {s:3d} -> tgt {t:3d}, but the other direction sends tgt {t} to src {back} "
              f"-- {d:.1f} away from {s}")

    unknown = len(disagree) - len(known)
    if unknown:
        print(f"\n  ({unknown} more disagree where one of the two source bones carries no geometry,")
        print("   so there is no distance to report and nothing rides the difference)")
    return 0


if __name__ == "__main__":
    sys.exit(main())

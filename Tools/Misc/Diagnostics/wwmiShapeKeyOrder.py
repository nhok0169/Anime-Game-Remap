"""Which of a WuWa source character's shape keys each of the target's shape-key SLOTS is.

    py -3 wwmiShapeKeyOrder.py <Source> <Target> [--version 3_7] [--cpp NAME]

Reads both characters' download folders (Data/Mod Downloads/WuWa/<Name>/<version>): ShapeKeyOffset.buf (the first entry
of each of 128 slots), ShapeKeyVertexId.buf (uint32 per entry), ShapeKeyVertexOffset.buf (6 x float16 per entry, the
first three the position offset) and Position.buf. The game drives a character's keys by SLOT, and two skins of one
character may number the same face keys differently, so a mod's keys moved across unchanged put every expression on
another key (Lynae -> LynaePeppermint: 2 of 105 at the same index, and a blink opened the mouth).

Each key is described by the rest positions of the vertices it moves (quantized to 0.05) and its offset there; two keys
score Jaccard(positions) x cosine(offsets on the shared positions), and a one-to-one assignment (scipy's
linear_sum_assignment) keeps pairs scoring >= 0.5. A target slot with no pair keeps its own index when the source's slot
of that number is unpaired too (both skins' trailing body keys, whose meshes differ), else takes nothing (-1).

Prints the pairs, the weak ones, and with --cpp a C++ initializer for WWMIFixerConfig::shapeKeyOrder.
"""
import argparse, os
import numpy as np
from scipy.optimize import linear_sum_assignment

Repo = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", ".."))
Q = 0.05


def load(name, version):
    p = os.path.join(Repo, "Data", "Mod Downloads", "WuWa", name, version, name)
    firsts = np.fromfile(p + "ShapeKeyOffset.buf", "<u4")
    ids = np.fromfile(p + "ShapeKeyVertexId.buf", "<u4")
    offs = np.fromfile(p + "ShapeKeyVertexOffset.buf", "<f2").reshape(len(ids), -1)[:, :3].astype(np.float64)
    pos = np.fromfile(p + "Position.buf", "<f4").reshape(-1, 3)
    keys = []
    for k in range(len(firsts)):
        a = int(firsts[k]); b = int(firsts[k + 1]) if k + 1 < len(firsts) else len(ids)
        if b <= a:
            keys.append(None)
            continue
        q = np.round(pos[ids[a:b]] / Q).astype(np.int64)
        keys.append({tuple(x): d for x, d in zip(q, offs[a:b])})
    while keys and keys[-1] is None:
        keys.pop()
    return keys


def score(a, b):
    common = a.keys() & b.keys()
    if not common:
        return 0.0
    jac = len(common) / len(a.keys() | b.keys())
    x = np.array([a[c] for c in common]); y = np.array([b[c] for c in common])
    cos = float((x * y).sum() / (np.linalg.norm(x) * np.linalg.norm(y) + 1e-12))
    return jac * max(cos, 0.0)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("source"); ap.add_argument("target")
    ap.add_argument("--version", default = "3_7")
    ap.add_argument("--targetVersion")
    ap.add_argument("--cpp", help = "print a C++ function of this name returning the order")
    args = ap.parse_args()
    src = load(args.source, args.version)
    dst = load(args.target, args.targetVersion or args.version)
    M = np.zeros((len(dst), len(src)))
    for j, d in enumerate(dst):
        if d is None:
            continue
        for i, s in enumerate(src):
            if s is not None:
                M[j, i] = score(d, s)

    rows, cols = linear_sum_assignment(-M)
    order = [-1] * len(dst)
    for j, i in zip(rows, cols):
        if M[j, i] >= 0.5:
            order[j] = int(i)

    paired = set(o for o in order if o >= 0)
    for j in range(len(dst)):
        if order[j] < 0 and j < len(src) and j not in paired:
            order[j] = j
            paired.add(j)

    same = sum(1 for j, i in enumerate(order) if i == j)
    print(f"{args.target} slots {len(dst)}, {args.source} keys {len(src)}; paired by content "
          f"{sum(1 for j, i in zip(rows, cols) if M[j, i] >= 0.5)}, at the same index {same}")
    print("weak pairs (< 0.8):", [(j, order[j], round(float(M[j, order[j]]), 3)) for j in range(len(dst))
                                  if order[j] >= 0 and 0.5 <= M[j, order[j]] < 0.8])
    print("kept by index:", [j for j in range(len(dst)) if order[j] == j and M[j, j] < 0.5])
    print("target slots taking nothing:", [j for j in range(len(dst)) if order[j] < 0])
    print("source keys no slot takes:", sorted(set(range(len(src))) - paired))
    if args.cpp:
        body = ", ".join(str(o) for o in order)
        print(f"\n    const std::vector<long long>& {args.cpp}() {{\n        static const std::vector<long long> table = {{{body}}};\n        return table;\n    }}")


if __name__ == "__main__":
    main()

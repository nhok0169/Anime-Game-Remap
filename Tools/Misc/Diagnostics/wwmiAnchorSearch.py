#
# ===== wwmiAnchorSearch =====
#
# Which single target bone a rigid part -- a fox mask, hairpins, a charm, a chain the target has no
# counterpart for -- should be anchored to, and what the remap is doing with it today.
#
# `--anchor` exists because such a part wants ONE rigid anchor rather than the finder's per-bone
# nearest (the Yelan lesson). Choosing WHICH bone is where two rounds went on Chisa, and both wrong
# instruments looked reasonable:
#
#   * A `vs-cb4` entry's translation column is NOT the bone's position. It is a skinning matrix --
#     the bone's world transform times its inverse bind pose -- so its translation is where the
#     ORIGIN would land. Read that way it named "the bone nearest head height on the mid-line", and
#     skinning the prop with that bone put the prop 90 units away, at her hip.
#   * An axis-aligned bounding box is not rotation invariant. Scoring a candidate by whether the
#     part kept its extents calls a bone that merely TURNED the part "stretched" -- it threw away
#     267 of 272 bones, the right answer among them.
#
# So this does what the GPU does: applies each candidate bone's matrix to the part's rest vertices,
# keeps the candidates whose RMS radius from the centroid is unchanged (a rigid transform preserves
# it, whatever the rotation), and ranks what is left by where the centroid LANDS -- against the
# part's own rest position, which is where its author modelled it.
#
#   python wwmiAnchorSearch.py <mod folder> <target FrameAnalysis folder> --draw <start> <count>
#                              [--blend <RemapBlend.buf>] [--slot-draw <n>] [--top <n>]
#
# `--draw` is the part's draw, as `start count` off its `drawindexed = <count>, <start>, 0` line.
# `--slot-draw` is the draw in the dump whose `vs-cb4` to read (default: the largest one, which is a
# body slot and carries the whole skeleton).
#
# Two mechanics the answer has to be written through, both of which produce a silent no-op-shaped
# error when missed:
#
#   * `AnchorChains`' key is a SOURCE bone and the chain takes whatever IT maps to. Writing the
#     TARGET bone id there anchors the chain somewhere unrelated, and the run still reports
#     `N rows, M differ from the library row`.
#   * Anchor EVERY bone the part uses. Chisa's props put 27% of their weight on one bone and 73% on
#     ten others; anchoring the ten left the prop torn between two places.
#
# See Creating Remaps' "PICK A RIGID ANCHOR BY SKINNING THE PART, NOT OFF A BONE'S POSITION".
#

import argparse
import glob
import os
import sys

import numpy as np


def meshFolder(mod):
    """The folder holding Position.buf, wherever it sits under the mod"""
    found = sorted(glob.glob(os.path.join(mod, "**", "Position.buf"), recursive = True))
    if (not found):
        sys.exit(f"no Position.buf anywhere under {mod}")
    return os.path.dirname(found[0])


def matrices(folder, slotDraw):
    """The target's skinning matrices, out of the vs-cb4 bound at a draw that carries the skeleton"""
    found = sorted(glob.glob(os.path.join(folder, "*-vs-cb4=*.buf")),
                   key = lambda p: os.path.getsize(p))
    if (not found):
        sys.exit(f"no vs-cb4 dump in {folder}")
    pick = found[-1] if (slotDraw is None) else next(
        (p for p in found if (os.path.basename(p).startswith(f"{slotDraw:06d}-"))), found[-1])
    raw = np.fromfile(pick, dtype = np.float32)
    mats = raw[:(len(raw) // 12) * 12].reshape(-1, 3, 4).astype(np.float64)
    ok = np.isfinite(mats).all(axis = (1, 2)) & (np.abs(mats) < 1e4).all(axis = (1, 2))
    return os.path.basename(pick), mats, ok


def main():
    parser = argparse.ArgumentParser(description = "the target bone a rigid part should be anchored to")
    parser.add_argument("mod", help = "the mod folder (its Position.buf / Index.buf are read)")
    parser.add_argument("dump", help = "the TARGET's FrameAnalysis-<Skin>-<date> folder")
    parser.add_argument("--draw", nargs = 2, type = int, required = True, metavar = ("START", "COUNT"),
                        help = "the part's draw, as `start count` off its drawindexed line")
    parser.add_argument("--blend", default = None,
                        help = "the fix's output blend buffer (default: the one *RemapBlend.buf in the mesh folder)")
    parser.add_argument("--slot-draw", type = int, default = None,
                        help = "the dump draw whose vs-cb4 to read (default: the largest)")
    parser.add_argument("--top", type = int, default = 8, help = "how many candidates to print")
    args = parser.parse_args()

    mesh = meshFolder(args.mod)
    pos = np.fromfile(os.path.join(mesh, "Position.buf"), dtype = np.float32).reshape(-1, 3).astype(np.float64)
    idx = np.fromfile(os.path.join(mesh, "Index.buf"), dtype = np.uint32).astype(np.int64)
    blend = args.blend
    if (blend is None):
        found = sorted(glob.glob(os.path.join(mesh, "*RemapBlend.buf")))
        if (not found):
            sys.exit(f"no *RemapBlend.buf in {mesh} -- run the fix first, or pass --blend")
        blend = found[0]
    out = np.fromfile(blend, dtype = np.uint8).reshape(len(pos), 16)
    ids, wts = out[:, :8].astype(np.int64), out[:, 8:].astype(np.float64) / 255.0

    name, mats, ok = matrices(args.dump, args.slot_draw)
    start, count = args.draw
    part = np.unique(idx[start:start + count])
    rest = pos[part]
    restRadius = float(np.linalg.norm(rest - rest.mean(axis = 0), axis = 1).mean())
    print(f"the part: {len(part)} vertices of the draw at {start}, RMS radius {restRadius:.2f}, "
          f"modelled at {rest.mean(axis = 0).round(1)}")
    print(f"the target's skeleton: {int(ok.sum())} usable matrices of {len(mats)}, out of {name}\n")

    # where it lands TODAY, under whatever the fix last wrote
    p = np.concatenate([pos[part], np.ones((len(part), 1))], axis = 1)
    acc, total = np.zeros((len(part), 3)), np.zeros(len(part))
    for s in range(8):
        b, w = ids[part, s], wts[part, s]
        live = (w > 0) & (b < len(mats)) & ok[np.clip(b, 0, len(mats) - 1)]
        if (live.any()):
            acc[live] += w[live, None] * np.einsum("vij,vj->vi", mats[b[live]], p[live])
            total[live] += w[live]
    landed = acc[total > 0] / total[total > 0, None]
    here = float(np.linalg.norm(landed - landed.mean(axis = 0), axis = 1).mean())
    print(f"TODAY it lands at {landed.mean(axis = 0).round(1)}, RMS radius {here:.2f} "
          f"({'rigid' if (abs(here / restRadius - 1) < 0.05) else f'STRETCHED x{here / restRadius:.2f}'}), "
          f"{np.linalg.norm(landed.mean(axis = 0) - rest.mean(axis = 0)):.1f} from where it is modelled")

    used = sorted({int(b) for s in range(8) for b in ids[part, s][wts[part, s] > 0]})
    print(f"  over {len(used)} target bone(s): {used if (len(used) < 16) else str(used[:16]) + ' ...'}")

    # ONE FRAME CANNOT TELL TWO BONES APART THAT COINCIDE IN THIS POSE, and a prop has to MOVE with
    #   the part it belongs to, not merely start beside it. So each candidate also carries the share
    #   of the surrounding geometry's weight it holds: the bone that region already moves with is
    #   the one the prop should ride, even when another sits a centimetre closer in this one frame.
    #   The part's OWN vertices are excluded, or the measure is circular: they carry whatever the
    #   last anchor put on them, and the tool would recommend the anchor already in place.
    near = np.linalg.norm(pos - rest.mean(axis = 0), axis = 1) <= max(3.0 * restRadius, 1e-6)
    near[part] = False
    neighbourhood = {}
    for s in range(8):
        for b, w in zip(ids[near, s], wts[near, s]):
            if (w > 0):
                neighbourhood[int(b)] = neighbourhood.get(int(b), 0.0) + float(w)
    whole = sum(neighbourhood.values()) or 1.0

    pp = np.concatenate([rest, np.ones((len(rest), 1))], axis = 1)
    scores = []
    for b in np.flatnonzero(ok):
        cloud = np.einsum("ij,vj->vi", mats[b], pp)
        centre = cloud.mean(axis = 0)
        radius = float(np.linalg.norm(cloud - centre, axis = 1).mean())
        if (abs(radius / restRadius - 1) < 0.05):
            scores.append((float(np.linalg.norm(centre - rest.mean(axis = 0))), int(b), centre,
                           neighbourhood.get(int(b), 0.0) / whole * 100))
    scores.sort()
    # Every candidate this close is "in place" to within the part's own size; ranking THOSE by what
    #   the region moves with is the real choice, so the window is a scale rather than a top-N.
    close = [s for s in scores if (s[0] <= max(restRadius, scores[0][0] * 1.5))] if (scores) else []
    print(f"\n{len(scores)} bones carry it rigidly, {len(close)} of them leave it where it is modelled:")
    print(f"   {'bone':>10} {'away':>7} {'centre':>24}   what the surrounding geometry already rides")
    for away, b, centre, share in sorted(close, key = lambda s: -s[3])[:args.top]:
        print(f"   target {b:4d} {away:6.1f}  {str(centre.round(1)):>24}   {share:5.1f}% of the weight around it")
    if (close):
        best = max(close, key = lambda s: s[3])
        print(f"\nAnchor every bone of the part to target {best[1]}: of the candidates that carry it"
              f"\n  rigidly and in place, it is the one the geometry around it already moves with"
              f"\n  ({best[3]:.1f}% of that weight). Remember `AnchorChains`' key is the SOURCE bone"
              f"\n  that maps there, not {best[1]} itself.")


if (__name__ == "__main__"):
    main()

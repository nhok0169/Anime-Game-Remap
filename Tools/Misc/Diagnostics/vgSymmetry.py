#
# ===== vgSymmetry =====
#
# Is a vertex group remap LEFT/RIGHT SYMMETRIC? A body is, so a remap of one should be, and where it
# is not the part does not sit wrong -- it WOBBLES, because its two halves ride bones that move
# independently. That is invisible to every distance check: the bones involved land within a few
# units of where they belong, which is why the usual "unmapped group" and "how far did it move"
# tests pass on a model that jiggles like jello in game.
#
# Two defects, both found this way on Chisa -> ChisaParfait (2026-09-22):
#
#   * a CENTRE source bone sent OFF the mid-line. Chisa's necktie is a three-link chain hanging down
#     the centre of her chest; ChisaParfait has no centre chest chain, so the finder matched each
#     link to its nearest bone -- her BREAST PAIR -- putting two links on the left one and the third
#     on the right. The tie then swung with the difference between them.
#   * a MIRROR PAIR whose two targets are not each other's mirror. Chisa's shoulder bones are exact
#     mirror pairs and the table sent every LEFT one to a single target while the RIGHT ones
#     scattered, so the jacket's two halves moved independently.
#
# Both are the Yelan lesson arriving from a new direction: a part the target has no counterpart for
# wants ONE rigid anchor, not the finder's per-bone nearest. The finder optimises each bone alone,
# so nothing in it keeps a pair together.
#
# Report the skew GRADED, in units, never as a yes/no: || reflect(target of b) - target of b's twin
# ||. The binary "is it the exact twin" form calls a pair broken when its two targets are 1.9 apart,
# which is noise, and so reported 95% of a part that renders fine. Zero is perfect, a bone is about
# two units wide, and ten units is two halves in different places.
#
#   py -3 vgSymmetry.py <Source> <Target> --identity <src mod> <tgt mod> [--mod <a fixed mod>]
#                       [--draw <start> <count> <name>]... [--z <min> <max>] [--propose]
#
# The identity mods are the two characters' own models as mods (Tools/Misc/Prototypes/
# wwmiIdentityMod.py); a bone's position is taken as the centroid of the vertices weighted to it,
# which is why they are needed -- a `vs-cb4` entry's translation column is a skinning matrix, NOT a
# pose, and reading a bone's position out of one names the wrong bone (see Creating Remaps).
#
# --propose suggests a repair per broken pair: keep whichever side is better placed for its own
# source and mirror it onto the other. READ THEM, do not apply them wholesale -- it prints the
# placement error before and after, and on several pairs symmetrising makes the placement WORSE,
# which means the pair needs a hand-picked target rather than a mirrored one.
#

import argparse
import collections
import glob
import os
import re
import sys

import numpy as np

Here = os.path.dirname(os.path.abspath(__file__))
Repo = os.path.normpath(os.path.join(Here, "..", "..", ".."))
sys.path.insert(0, os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py"))
import FixRaidenBoss2 as FRB                                   # noqa: E402

MidLine = 1.2        # |x| under this counts as on the body's mid-line
PairTol = 1.5        # how close a reflection has to be for two bones to be a mirror pair
Skewed = 6.0         # a pair this far from symmetric is reported as a real one


def mesh(folder):
    """(meshFolder, positions, merged vertex groups, weights) of a WWMI mod folder"""
    hits = sorted(glob.glob(os.path.join(folder, "**", "Position.buf"), recursive = True))
    if (not hits):
        raise SystemExit(f"no Position.buf under {folder}")
    d = os.path.dirname(hits[0])
    n = os.path.getsize(os.path.join(d, "Position.buf")) // 12
    pos = np.fromfile(os.path.join(d, "Position.buf"), dtype = np.float32).reshape(n, 3)
    blend = np.fromfile(os.path.join(d, "Blend.buf"), dtype = np.uint8).reshape(n, 16)
    # past 256 merged bones the true 16-bit ids live in BlendRemapVertexVG; under it Blend.buf's own
    #   uint8 indices are already merged ids. Reading Blend.buf for a character that HAS the remap
    #   measures component-local indices and means nothing.
    vgPath = os.path.join(d, "BlendRemapVertexVG.buf")
    vg = (np.fromfile(vgPath, dtype = np.uint16).reshape(n, 8) if (os.path.exists(vgPath))
          else blend[:, :8].astype(np.uint16))
    return d, pos, vg, blend[:, 8:].astype(float)


def centroids(folder):
    """{bone: the weighted centroid of the vertices it drives}"""
    _, pos, vg, w = mesh(folder)
    tot = collections.defaultdict(float)
    acc = collections.defaultdict(lambda: np.zeros(3))
    for slot in range(vg.shape[1]):
        used = w[:, slot] > 0
        for bone, weight, p in zip(vg[used, slot], w[used, slot], pos[used]):
            tot[int(bone)] += weight
            acc[int(bone)] += p * weight
    return {b: acc[b] / tot[b] for b in tot}


def windows(folder):
    """{bone: the component index whose vg window contributes it}, from the mod's own .ini"""
    ini = sorted(glob.glob(os.path.join(folder, "**", "*.ini"), recursive = True))[0]
    cur, secs, out = None, {}, {}
    for line in open(ini, encoding = "utf-8", errors = "replace").read().splitlines():
        m = re.match(r"^\s*\[(.+)\]\s*$", line)
        if (m):
            cur = m.group(1)
            secs[cur] = []
        elif (cur):
            secs[cur].append(line)
    for name, body in secs.items():
        if (not re.match(r"TextureOverrideComponent\d+$", name)):
            continue
        text = "\n".join(body)
        off = re.search(r"vg_offset\s*=\s*(\d+)", text)
        cnt = re.search(r"vg_count\s*=\s*(\d+)", text)
        if (off and cnt):
            for b in range(int(off.group(1)), int(off.group(1)) + int(cnt.group(1))):
                out[b] = int(re.search(r"(\d+)$", name).group(1))
    return out


def mirrors(cents):
    """{bone: its reflection in x}, reciprocal pairs only"""
    out = {}
    for b, p in cents.items():
        if (abs(p[0]) < MidLine):
            continue
        reflected = np.array([-p[0], p[1], p[2]])
        best = sorted((float(np.linalg.norm(reflected - q)), o) for o, q in cents.items() if (o != b))
        if (best and best[0][0] < PairTol):
            out[b] = best[0][1]
    return {b: o for b, o in out.items() if (out.get(o) == b)}


def main():
    parser = argparse.ArgumentParser(description = "is a vertex group remap left/right symmetric")
    parser.add_argument("source", help = "the source mod type's name, e.g. Chisa")
    parser.add_argument("target", help = "the target mod type's name, e.g. ChisaParfait")
    parser.add_argument("--identity", nargs = 2, required = True, metavar = ("SRCMOD", "TGTMOD"),
                        help = "the two characters' identity mod folders")
    parser.add_argument("--mod", help = "a mod to weight the report by (otherwise every bone counts equally)")
    parser.add_argument("--draw", nargs = 3, action = "append", default = [], metavar = ("START", "COUNT", "NAME"),
                        help = "a draw range of --mod to report on its own; repeatable")
    parser.add_argument("--z", nargs = 2, type = float, metavar = ("MIN", "MAX"),
                        help = "restrict each --draw to this height band")
    parser.add_argument("--propose", action = "store_true", help = "suggest a repair per broken pair")
    parser.add_argument("--hair", type = int, default = None, metavar = "N",
                        help = "the component index that is PHYSICS-simulated hair on the target "
                               "(1 for Chisa / ChisaParfait); bones of other components sent there are reported")
    args = parser.parse_args()

    sourceType = getattr(FRB.WWMIBuilder, args.source[0].lower() + args.source[1:])()
    remap = {int(k): int(v) for k, v in dict(sourceType.getVGRemap(args.target).remap).items()}
    src, dst = centroids(args.identity[0]), centroids(args.identity[1])
    sMir, dMir = mirrors(src), mirrors(dst)
    print(f"{args.source}: {len(src)} weighted bones, {len(sMir) // 2} mirror pairs, "
          f"{sum(1 for b in src if abs(src[b][0]) < MidLine)} on the mid-line")
    print(f"{args.target}: {len(dst)} weighted bones, {len(dMir) // 2} mirror pairs, "
          f"{sum(1 for b in dst if abs(dst[b][0]) < MidLine)} on the mid-line")
    print(f"remap: {len(remap)} rows")

    def skew(bone):
        twin = sMir.get(bone)
        t, u = remap.get(bone), remap.get(twin)
        if (twin is None or t not in dst or u not in dst):
            return None
        return float(np.linalg.norm(np.array([-dst[t][0], dst[t][1], dst[t][2]]) - dst[u]))

    # THE ONE TO READ FIRST: a bone sent to a component that is a different KIND of thing.
    #   A WWMI character's components are draw slots of one merged skeleton, and they are not
    #   interchangeable: the HAIR is one of them, and hair bones are physics-simulated. A jacket
    #   shoulder skinned to one swings with the hair (a wobble) and takes its settled rest offset
    #   (a lean), while sitting within a few units of where it belongs -- so neither distance nor
    #   symmetry sees it. Chisa's jacket shoulders rode ChisaParfait's hair for two rounds.
    #   Cross-component edges are NOT faults in general: two characters split the TORSO at
    #   different heights, and 29% of Chisa's body crosses that way and renders correctly. What is
    #   a fault is an edge between components of different kinds, which only the caller can name.
    sComp, dComp = windows(args.identity[0]), windows(args.identity[1])
    if (args.hair is not None):
        toHair = [b for b in sorted(src)
                  if (remap.get(b) in dst and sComp.get(b) != args.hair and dComp.get(remap[b]) == args.hair)]
        print(f"\n=== {len(toHair)} bones of another component sent to component {args.hair} "
              f"(the one named as PHYSICS) ===")
        for b in toHair:
            t = remap[b]
            alts = sorted((float(np.linalg.norm(dst[o] - src[b])), o) for o in dst
                          if (dComp.get(o) not in (None, args.hair)))
            near = ", ".join(f"{o} ({d:.1f})" for d, o in alts[:3])
            print(f"  src {b:3d} (comp {sComp.get(b)}, {src[b][0]:6.1f},{src[b][1]:6.1f},{src[b][2]:6.1f})"
                  f" -> tgt {t:3d} ({dst[t][0]:6.1f},{dst[t][1]:6.1f},{dst[t][2]:6.1f})"
                  f"   nearest non-{args.hair}: {near}")
    print("\n=== every source component -> target component edge (for spotting which are KINDS) ===")
    grid = collections.Counter((sComp.get(b), dComp.get(remap[b])) for b in src if (remap.get(b) in dst))
    for (s, t), n in sorted(grid.items(), key = lambda kv: -kv[1]):
        print(f"  component {s} -> component {t}: {n:4d} bones" + ("" if (s == t) else "   (crosses)"))

    offMid = [(b, remap[b]) for b in sorted(src)
              if (remap.get(b) in dst and abs(src[b][0]) < MidLine and abs(dst[remap[b]][0]) >= MidLine)]
    print(f"\n=== {len(offMid)} CENTRE source bones sent off the mid-line ===")
    for b, t in offMid:
        print(f"  src {b:3d} ({src[b][0]:6.1f},{src[b][1]:6.1f},{src[b][2]:6.1f}) -> "
              f"tgt {t:3d} ({dst[t][0]:6.1f},{dst[t][1]:6.1f},{dst[t][2]:6.1f})")

    pairs, seen = [], set()
    for b in sorted(src):
        o, s = sMir.get(b), skew(b)
        if (o is None or b in seen or s is None):
            continue
        seen |= {b, o}
        if (s > Skewed):
            pairs.append((s, b, o))
    pairs.sort(reverse = True)
    print(f"\n=== {len(pairs)} mirror pairs skewed by more than {Skewed} units ===")
    for s, b, o in pairs:
        t, u = remap[b], remap[o]
        print(f"  skew {s:5.1f}   src {b:3d}/{o:3d} (|x| {abs(src[b][0]):5.1f}, {src[b][1]:6.1f},"
              f" {src[b][2]:6.1f}) -> tgt {t:3d} ({dst[t][0]:6.1f},{dst[t][1]:6.1f},{dst[t][2]:6.1f})"
              f" / {u:3d} ({dst[u][0]:6.1f},{dst[u][1]:6.1f},{dst[u][2]:6.1f})")

    if (args.propose):
        print("\n=== proposals -- READ THEM; a worse placement means the pair needs a hand-picked target ===")
        for s, b, o in pairs:
            options = []
            for keep, kb, ob in ((remap[b], b, o), (remap[o], o, b)):
                other = dMir.get(keep)
                if (other is not None):
                    options.append((np.linalg.norm(src[kb] - dst[keep]) + np.linalg.norm(src[ob] - dst[other]),
                                    kb, keep, ob, other))
            if (not options):
                print(f"  src {b}/{o}: neither target has a mirror twin -- hand-pick a pair")
                continue
            options.sort()
            err, kb, keep, ob, other = options[0]
            was = np.linalg.norm(src[b] - dst[remap[b]]) + np.linalg.norm(src[o] - dst[remap[o]])
            flag = "" if (err <= was + 0.5) else "   <-- WORSE placement; hand-pick instead"
            print(f"  {{{kb}, {keep}}}, {{{ob}, {other}}}   (was {b}->{remap[b]}, {o}->{remap[o]};"
                  f" placement error {was:.1f} -> {err:.1f}){flag}")

    if (not args.mod):
        return
    d, pos, vg, w = mesh(args.mod)
    ib = np.fromfile(os.path.join(d, "Index.buf"), dtype = np.uint32)
    draws = [(int(a), int(b), n) for a, b, n in args.draw] or [(0, len(ib), "the whole mod")]
    print(f"\n=== {os.path.basename(args.mod.rstrip(os.sep))}"
          + (f", z {args.z[0]}..{args.z[1]}" if (args.z) else "") + " ===")
    print(f"{'part':24s} {'vtx':>8s}  {'mean skew':>9s}  {'weight past ' + str(Skewed):>16s}  {'off mid-line':>12s}")
    for start, count, name in draws:
        v = np.unique(ib[start:start + count])
        if (args.z):
            v = v[(pos[v][:, 2] >= args.z[0]) & (pos[v][:, 2] <= args.z[1])]
        if (not len(v)):
            print(f"{name:24s}   (no vertices)")
            continue
        tally = collections.Counter()
        for slot in range(vg.shape[1]):
            used = w[v, slot] > 0
            for bone, weight in zip(vg[v, slot][used], w[v, slot][used]):
                tally[int(bone)] += float(weight)
        total = sum(tally.values()) or 1
        num = sum(ww * skew(b) for b, ww in tally.items() if (skew(b) is not None))
        den = sum(ww for b, ww in tally.items() if (skew(b) is not None)) or 1
        big = sum(ww for b, ww in tally.items() if ((skew(b) or 0) > Skewed)) / total * 100
        mid = sum(ww for b, ww in tally.items()
                  if (b in src and remap.get(b) in dst and abs(src[b][0]) < MidLine
                      and abs(dst[remap[b]][0]) >= MidLine)) / total * 100
        print(f"{name:24s} {len(v):8d}  {num / den:9.2f}  {big:15.1f}%  {mid:11.1f}%")


if (__name__ == "__main__"):
    main()

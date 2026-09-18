# A mirror PAIR of source bones collapsing onto ONE off-midline target bone is a one-sided warp:
# whichever side of the pair is on the far side of that target gets dragged across the body.
# Many-to-one is normal (the source has more bones than the target); many-to-one across the MIDLINE
# is not.
import os, sys
import numpy as np
APISrc = r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss\Anime Game Remap (for all users)\api\src\py"
sys.path.insert(0, APISrc); os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))
import FixRaidenBoss2 as FRB

T = r"E:\Computer\Games\Wuthering Waves Mods\Importer\GIMI\YelanTranquilIdentity\YelanTranquil"
Y = r"E:\Computer\Games\Wuthering Waves Mods\Importer\GIMI\YelanIdentity\Yelan"
FRB.CppGlobalModTypes.registerAll()
vg = {m.name: m for m in FRB.CppGlobalModTypes.all()}["YelanTranquil"].vgRemaps


def model(folder, prefix, comp = ""):
    pos = np.fromfile(os.path.join(folder, f"{prefix}{comp}Position.buf"), dtype = np.uint8).reshape(-1, 40)
    xyz = pos[:, 0:12].copy().view(np.float32).reshape(-1, 3)
    bl = np.fromfile(os.path.join(folder, f"{prefix}{comp}Blend.buf"), dtype = np.uint8).reshape(-1, 32)
    w = bl[:, 0:16].copy().view(np.float32).reshape(-1, 4)
    i = bl[:, 16:32].copy().view(np.int32).reshape(-1, 4)
    out = {}
    for g in np.unique(i[w > 0]):
        m = (i == g) & (w > 0)
        rows, slots = np.where(m)
        weights = w[rows, slots]
        out[int(g)] = ((xyz[rows] * weights[:, None]).sum(axis = 0) / weights.sum(), int(m.sum()))
    return out


def mirrors(c, tol):
    ids = sorted(c)
    pts = np.array([c[g][0] for g in ids])
    best = {}
    for k, g in enumerate(ids):
        d = np.linalg.norm(pts - pts[k] * np.array([-1.0, 1.0, 1.0]), axis = 1)
        j = int(np.argmin(d))
        best[g] = (ids[j], float(d[j]))
    return {g: m for g, (m, d) in best.items() if (best.get(m, (None,))[0] == g and d <= tol and g != m)}


yc = model(Y, "Yelan")
yMirror = mirrors(yc, 0.10)       # Yelan's own pairs are not exactly symmetric; allow some slack

# the prototype's corrections, so this can be run against the shipped rows AND against what the fix
# actually uses: pass --fixed to apply them
Overrides = {"Body": {9: 108}}
applyFixes = ("--fixed" in sys.argv)

for comp in ("Body", "Bang", "Eye"):
    tc = model(T, "YelanTranquil", comp)
    row = dict(vg.get(["YelanTranquil", comp, "Yelan", ""], ["1.0", "5.7"]).remap)
    if (applyFixes):
        row.update(Overrides.get(comp, {}))
    tMirror = mirrors(tc, 0.02)
    print(f"=== {comp}: {len(tMirror) // 2} mirror pairs" + (" (with the prototype's overrides)" if applyFixes else " (shipped rows)") + " ===")
    found = False
    for g, m in sorted(tMirror.items()):
        if (g >= m):
            continue
        a, b = row.get(g), row.get(m)
        if (a != b or a is None):
            continue
        c = yc.get(a)
        if (c is None or abs(c[0][0]) < 0.03):
            continue          # a genuinely midline target is fine for both sides
        partner = yMirror.get(a)
        gc, mc = tc[g][0], tc[m][0]
        # the pair member on the SAME side as the target keeps it; the other one wants the mirror
        keeps, moves = (g, m) if (np.sign(gc[0]) == np.sign(c[0][0])) else (m, g)
        pc = yc.get(partner)
        print(f"  COLLAPSED ACROSS THE MIDLINE: {g} and {m} both -> {a}")
        print(f"     Tranquil {g:>3} ({gc[0]:+.3f},{gc[1]:.3f},{gc[2]:+.3f})  {tc[g][1]} vertices")
        print(f"     Tranquil {m:>3} ({mc[0]:+.3f},{mc[1]:.3f},{mc[2]:+.3f})  {tc[m][1]} vertices")
        print(f"     Yelan    {a:>3} ({c[0][0]:+.3f},{c[0][1]:.3f},{c[0][2]:+.3f})  -- off the midline")
        if (partner is not None):
            print(f"     Yelan's mirror of {a} is {partner} ({pc[0][0]:+.3f},{pc[0][1]:.3f},{pc[0][2]:+.3f})")
            print(f"     => keep {keeps} -> {a}, and move {moves} -> {partner}")
        else:
            print(f"     Yelan {a} has no mirror partner within tolerance -- needs a hand decision")
        found = True
    if (not found):
        print("  none")

#
# ===== lumineHeavenSynth =====
#
#   py -3 lumineHeavenSynth.py [out folder] [--m3 <pristine LumineHeaven3>]   (default: ./synth)
#
# Synthetic LumineHeaven mods for the reverse direction's structural axes -- the skin has THREE real mods, all of one
# shape (one .ini, no toggles) -- built from unfixed copies of the identity mod and LumineHeaven3
# (yaoyaoBambooSynth.py's shape):
#   SynthMerged    a genshin_merge_mods.py master of two variants: the identity, and LumineHeaven3's mesh
#   SynthRecolour  a texture-only recolour: `this =` on the skin's head / body diffuse hashes, its hair tinted pink
#   SynthNoEye     the identity with its whole Eye component removed (the merge downloads it)
#   Synth16        the identity with every index buffer 16-bit (R16_UINT)
import os, re, shutil, struct, subprocess, sys
GIMI = r"E:\Computer\Games\Wuthering Waves Mods\Importer\GIMI"
args = [a for a in sys.argv[1:]]
m3Src = None
if "--m3" in args:
    k = args.index("--m3"); m3Src = args[k + 1]; del args[k:k + 2]
Out = os.path.abspath(args[0]) if args else os.path.join(os.getcwd(), "synth")
Repo = os.environ.get("AG_REMAP_REPO") or r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss"
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
sys.path.insert(0, APISrc); os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))
import FixRaidenBoss2 as FRB

def find(name):
    for base in (GIMI, os.path.join(GIMI, "Mods")):
        if os.path.isdir(os.path.join(base, name)):
            return os.path.join(base, name)
    raise SystemExit("no " + name)

def unfixed(src, dst, ignore = None):
    """Copies a mod that has never been fixed -- refuses one that has, rather than guess where its fix begins."""
    shutil.copytree(src, dst, ignore = ignore)
    for d, _, fs in os.walk(dst):
        for f in fs:
            if f.lower().endswith(".ini") and b"remapped by" in open(os.path.join(d, f), "rb").read():
                raise SystemExit(f"{os.path.join(d, f)} has been fixed already: pass an unfixed copy")

def readIni(p):
    return open(p, "rb").read().decode("utf-8").replace("\r\n", "\n")

def writeIni(p, t):
    open(p, "wb").write(t.replace("\n", "\r\n").encode("utf-8"))

def dropSections(t, pattern):
    return "".join(s for s in re.split(r"(?m)^(?=\[)", t) if not re.match(pattern, s))

Merge = os.path.join(find("Neuvillette5"), "genshin_merge_mods.py")
identity, m3 = find("LumineHeavenIdentity"), m3Src or find("LumineHeaven3")
if os.path.exists(Out):
    raise SystemExit(f"{Out} exists: pick a new folder")
os.makedirs(Out)

# ---- SynthMerged ----
merged = os.path.join(Out, "LumineHeavenSynthMerged")
os.makedirs(merged)
unfixed(identity, os.path.join(merged, "A"))
unfixed(m3, os.path.join(merged, "B"))
tb = readIni(os.path.join(merged, "B", "LumineSkin.ini"))
r = subprocess.run([sys.executable, Merge, "-r", ".", "-k", "j"], input = "\n", capture_output = True, text = True, cwd = merged)
print("merge:", r.returncode, r.stdout[-200:].strip().replace("\n", " | "), r.stderr[-300:])

# genshin_merge_mods.py keys a section's lines by their NAME, so of several `drawindexed` lines per slot only the last
# survives. A correct master draws them all: put B's back into its branch.
def sectionDraws(t, name):
    body = re.search(r"(?ms)^\[" + re.escape(name) + r"\]\n(.*?)(?=^\[|\Z)", t).group(1)
    return [l.strip() for l in body.split("\n") if l.strip().startswith("drawindexed")]
mp = os.path.join(merged, "merged.ini")
tm = readIni(mp)
for aSlot, bSlot in (("Head", "Head"), ("Body", "Body"), ("EyeA", "EyesA"), ("BangA", "BangsA")):
    draws = sectionDraws(tb, "TextureOverrideLumineSkin" + bSlot)
    m = re.search(r"(?ms)^\[CommandListLumineHeaven" + aSlot + r"\]\n.*?^else if \$swapvar == 1\n(.*?)^endif", tm)
    if m is None:
        print(f"  B {bSlot}: no swapvar branch found in CommandListLumineHeaven{aSlot} -- left as merged")
        continue
    kept = [l for l in m.group(1).split("\n") if l and not l.strip().startswith("drawindexed")]
    tm = tm[:m.start(1)] + "\n".join(kept + ["\t" + d for d in draws]) + "\n" + tm[m.end(1):]
    print(f"  B {bSlot}: {len(draws)} drawindexed restored")
writeIni(mp, tm)

# ---- SynthRecolour ----
recolour = os.path.join(Out, "LumineHeavenSynthRecolour")
os.makedirs(recolour)
for f in ("LumineHeavenHeadDiffuse.dds", "LumineHeavenBodyDiffuse.dds"):
    shutil.copy(os.path.join(identity, f), recolour)
writeIni(os.path.join(recolour, "tex.ini"), "\n".join([
    "; a texture-only recolour of the skin, by hash (synthetic)",
    "",
    "[TextureOverrideLumineHeavenHeadDiffuse]", "hash = 0c6f8065", "this = ResourceLumineHeavenHeadDiffuse", "",
    "[TextureOverrideLumineHeavenBodyDiffuse]", "hash = f07f0f1e", "this = ResourceLumineHeavenBodyDiffuse", "",
    "[ResourceLumineHeavenHeadDiffuse]", "filename = LumineHeavenHeadDiffuse.dds", "",
    "[ResourceLumineHeavenBodyDiffuse]", "filename = LumineHeavenBodyDiffuse.dds", ""]))
tex = FRB.CppTextureFile(os.path.join(recolour, "LumineHeavenHeadDiffuse.dds"))
tex.open()
px = bytearray(tex.getPixels())
for n in range(0, len(px), 4):
    px[n] = min(255, px[n] + 90); px[n + 1] = px[n + 1] * 3 // 5
tex.setPixels(bytes(px), tex.width, tex.height)
tex.save(compress = False)

# ---- SynthNoEye ----
noEye = os.path.join(Out, "LumineHeavenSynthNoEye")
unfixed(identity, noEye)
for f in os.listdir(noEye):
    if f.startswith("LumineHeavenEye"):
        os.remove(os.path.join(noEye, f))
p = os.path.join(noEye, "LumineHeaven.ini")
writeIni(p, dropSections(readIni(p), r"\[(TextureOverride|Resource)LumineHeavenEye"))

# ---- Synth16 ----
s16 = os.path.join(Out, "LumineHeavenSynth16")
unfixed(identity, s16)
for f in os.listdir(s16):
    if f.endswith(".ib"):
        q = os.path.join(s16, f); d = open(q, "rb").read()
        idx = struct.unpack(f"<{len(d)//4}I", d); assert max(idx) < 65536
        open(q, "wb").write(struct.pack(f"<{len(idx)}H", *idx))
p = os.path.join(s16, "LumineHeaven.ini")
t = readIni(p)
n = t.count("DXGI_FORMAT_R32_UINT")
writeIni(p, t.replace("DXGI_FORMAT_R32_UINT", "DXGI_FORMAT_R16_UINT"))
print("16-bit formats rewritten:", n)
for d in sorted(os.listdir(Out)):
    print(d, sorted(x for x in os.listdir(os.path.join(Out, d)) if x.endswith(".ini") or os.path.isdir(os.path.join(Out, d, x))))

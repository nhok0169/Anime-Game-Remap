#
# ===== yaoyaoBambooSynth =====
#
#   py -3 yaoyaoBambooSynth.py [out folder] [--m1 <pristine YaoyaoBamboo1>]   (default: ./synth)
#
# Synthetic YaoyaoBamboo mods for the reverse direction's structural axes -- the skin has ONE real mod -- built from
# unfixed copies of the identity mod and YaoyaoBamboo1 (neuvilletteMelusentSynth.py's shape):
#   SynthMerged    a genshin_merge_mods.py master of two variants: the identity, and YaoyaoBamboo1's mesh
#   SynthRecolour  a texture-only recolour: `this =` on the skin's head / body diffuse hashes, its hair tinted pink
#   SynthNoEye     the identity with its whole Eye component removed (the merge downloads it)
#   Synth16        the identity with every index buffer 16-bit (R16_UINT)
import os, re, shutil, struct, subprocess, sys
GIMI = r"E:\Computer\Games\Wuthering Waves Mods\Importer\GIMI"
args = [a for a in sys.argv[1:]]
m1Src = None
if "--m1" in args:
    k = args.index("--m1"); m1Src = args[k + 1]; del args[k:k + 2]
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
identity, m1 = find("YaoyaoBambooIdentity"), m1Src or find("YaoyaoBamboo1")
shutil.rmtree(Out, ignore_errors = True); os.makedirs(Out)

# ---- SynthMerged ----
merged = os.path.join(Out, "YaoyaoBambooSynthMerged")
os.makedirs(merged)
unfixed(identity, os.path.join(merged, "A"))
# YaoyaoBamboo1's weapon and glider sub-mods are their own .ini files, not variants
unfixed(m1, os.path.join(merged, "B"), ignore = shutil.ignore_patterns("Sages_Umbrella", "UmbrellaGlider"))
# genshin_merge_mods.py reads a variant's own [KeySwap*] (`type = cycle`) as a RESOURCE and crashes, and mangles its
# inner if / else if chains (it keys a section's lines by their text), so the variant's own toggles are resolved to
# one state first: $Hair = 0, $Ears = 0
def resolveToggles(t, values):
    out, stack = [], []          # stack of [branch active, some branch taken]
    for line in t.split("\n"):
        s = line.strip()
        m = re.match(r"(if|else if)\s+\$(\w+)\s*==\s*(\d+)$", s)
        if m and m.group(2) in values:
            hit = values[m.group(2)] == int(m.group(3))
            if m.group(1) == "if":
                stack.append([hit, hit])
            else:
                stack[-1] = [hit and not stack[-1][1], stack[-1][1] or hit]
            continue
        if stack and s == "else":
            stack[-1] = [not stack[-1][1], True]; continue
        if stack and s == "endif":
            stack.pop(); continue
        if all(a for a, _ in stack):
            out.append(line.strip() if stack else line)
    assert not stack, "unbalanced if"
    return "\n".join(out)
q = os.path.join(merged, "B", "YaoYaoRainlit.ini")
tq = dropSections(readIni(q), r"\[KeySwap")
tq = resolveToggles(tq, {"Hair": 0, "Ears": 0})
assert "$Hair" not in tq.split("[Present]")[1] and "$Ears" not in tq.split("[Present]")[1]
writeIni(q, tq)
r = subprocess.run([sys.executable, Merge, "-r", ".", "-k", "j"], input = "\n", capture_output = True, text = True, cwd = merged)
print("merge:", r.returncode, r.stdout[-200:].strip().replace("\n", " | "), r.stderr[-300:])

# ...and it keys a section's lines by their NAME, so of B's several `drawindexed` lines per slot only the last survives
# (its Head drew 2040 of its 32022 indices). A correct master draws them all: put B's back into its branch.
def sectionDraws(t, name):
    body = re.search(r"(?ms)^\[" + re.escape(name) + r"\]\n(.*?)(?=^\[|\Z)", t).group(1)
    return [l.strip() for l in body.split("\n") if l.strip().startswith("drawindexed")]
tb = tq      # B's .ini as it went in (the merge renames the file DISABLED...)
mp = os.path.join(merged, "merged.ini")
tm = readIni(mp)
for aSlot, bSlot in (("Head", "Head"), ("Body", "Body"), ("EyeA", "EyesA"), ("BangA", "BangsA")):
    draws = sectionDraws(tb, "TextureOverrideYaoYaoRainlit" + bSlot)
    m = re.search(r"(?ms)^\[CommandListYaoyaoBamboo" + aSlot + r"\]\n.*?^else if \$swapvar == 1\n(.*?)^endif", tm)
    branch = m.group(1)
    kept = [l for l in branch.split("\n") if l and not l.strip().startswith("drawindexed")]
    new = "\n".join(kept + ["\t" + d for d in draws]) + "\n"
    tm = tm[:m.start(1)] + new + tm[m.end(1):]
    print(f"  B {bSlot}: {len(draws)} drawindexed restored")
writeIni(mp, tm)

# ---- SynthRecolour ----
recolour = os.path.join(Out, "YaoyaoBambooSynthRecolour")
os.makedirs(recolour)
for f in ("YaoyaoBambooHeadDiffuse.dds", "YaoyaoBambooBodyDiffuse.dds"):
    shutil.copy(os.path.join(identity, f), recolour)
writeIni(os.path.join(recolour, "tex.ini"), "\n".join([
    "; a texture-only recolour of the skin, by hash (synthetic)",
    "",
    "[TextureOverrideYaoyaoBambooHeadDiffuse]", "hash = 3133e7a5", "this = ResourceYaoyaoBambooHeadDiffuse", "",
    "[TextureOverrideYaoyaoBambooBodyDiffuse]", "hash = 3b95c36c", "this = ResourceYaoyaoBambooBodyDiffuse", "",
    "[ResourceYaoyaoBambooHeadDiffuse]", "filename = YaoyaoBambooHeadDiffuse.dds", "",
    "[ResourceYaoyaoBambooBodyDiffuse]", "filename = YaoyaoBambooBodyDiffuse.dds", ""]))
tex = FRB.CppTextureFile(os.path.join(recolour, "YaoyaoBambooHeadDiffuse.dds"))
tex.open()
px = bytearray(tex.getPixels())
for n in range(0, len(px), 4):
    px[n] = min(255, px[n] + 90); px[n + 1] = px[n + 1] * 3 // 5
tex.setPixels(bytes(px), tex.width, tex.height)
tex.save(compress = False)

# ---- SynthNoEye ----
noEye = os.path.join(Out, "YaoyaoBambooSynthNoEye")
unfixed(identity, noEye)
for f in os.listdir(noEye):
    if f.startswith("YaoyaoBambooEye"):
        os.remove(os.path.join(noEye, f))
p = os.path.join(noEye, "YaoyaoBamboo.ini")
writeIni(p, dropSections(readIni(p), r"\[(TextureOverride|Resource)YaoyaoBambooEye"))

# ---- Synth16 ----
s16 = os.path.join(Out, "YaoyaoBambooSynth16")
unfixed(identity, s16)
for f in os.listdir(s16):
    if f.endswith(".ib"):
        q = os.path.join(s16, f); d = open(q, "rb").read()
        idx = struct.unpack(f"<{len(d)//4}I", d); assert max(idx) < 65536
        open(q, "wb").write(struct.pack(f"<{len(idx)}H", *idx))
p = os.path.join(s16, "YaoyaoBamboo.ini")
t = readIni(p)
n = t.count("DXGI_FORMAT_R32_UINT")
writeIni(p, t.replace("DXGI_FORMAT_R32_UINT", "DXGI_FORMAT_R16_UINT"))
print("16-bit formats rewritten:", n)
for d in sorted(os.listdir(Out)):
    print(d, sorted(x for x in os.listdir(os.path.join(Out, d)) if x.endswith(".ini") or os.path.isdir(os.path.join(Out, d, x))))

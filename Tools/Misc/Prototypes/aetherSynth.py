#
# ===== aetherSynth =====
#
#   py -3 aetherSynth.py <unfixed AetherIdentity> [out folder]   (default: ./synth)
#
# Synthetic Aether mods for the forward direction's structural axes the ten real mods do not cover -- built from an
# unfixed copy of the identity mod (lumineHeavenSynth.py's shape). The real ones already hold three genshin_merge_mods.py
# masters (Aether1 / 3 / 5), two mods on his 4.0 hashes (Aether9 / 10), toggled drawindexed ranges (Aether6) and a
# sibling icon .ini (Aether7).
#   SynthRecolour    a texture-only recolour: `this =` on his head / body diffuse hashes, the diffuses channel-rotated
#                    (RGB -> GBR) so a recolour that lands is unmistakable in game
#   SynthBodyOnly    the identity with its whole Head object removed (the game draws its own head)
#   Synth16          the identity with every index buffer 16-bit (R16_UINT)
#   에테르Synth        the identity under a non-Latin folder name
#
import os
import re
import shutil
import sys

import numpy as np

Src = os.path.abspath(sys.argv[1])
Out = os.path.abspath(sys.argv[2]) if len(sys.argv) > 2 else os.path.join(os.getcwd(), "synth")
Repo = os.environ.get("AG_REMAP_REPO") or r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss"
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
sys.path.insert(0, APISrc)
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))
import FixRaidenBoss2 as FRB     # noqa: E402


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


os.makedirs(Out, exist_ok = True)
for name in ("SynthRecolour", "SynthBodyOnly", "Synth16", "에테르Synth"):
    shutil.rmtree(os.path.join(Out, name), ignore_errors = True)

# ---- SynthRecolour ------------------------------------------------------------------------------
recolour = os.path.join(Out, "SynthRecolour")
os.makedirs(recolour)
for obj, hash in (("Head", "cab5aa1f"), ("Body", "1f7b5c39")):
    dst = os.path.join(recolour, f"AetherRecolour{obj}Diffuse.dds")
    shutil.copyfile(os.path.join(Src, f"Aether{obj}Diffuse.dds"), dst)
    tex = FRB.CppTextureFile(dst)
    tex.open()
    px = np.frombuffer(tex.getPixels(), dtype = np.uint8).copy().reshape(-1, 4)
    px[:, :3] = px[:, [1, 2, 0]]
    tex.setPixels(px.tobytes(), tex.width, tex.height)
    tex.save(compress = False)
writeIni(os.path.join(recolour, "AetherRecolour.ini"), """[TextureOverrideAetherRecolourHeadDiffuse]
hash = cab5aa1f
this = ResourceAetherRecolourHeadDiffuse

[TextureOverrideAetherRecolourBodyDiffuse]
hash = 1f7b5c39
this = ResourceAetherRecolourBodyDiffuse

[ResourceAetherRecolourHeadDiffuse]
filename = AetherRecolourHeadDiffuse.dds

[ResourceAetherRecolourBodyDiffuse]
filename = AetherRecolourBodyDiffuse.dds
""")

# ---- SynthBodyOnly ------------------------------------------------------------------------------
bodyOnly = os.path.join(Out, "SynthBodyOnly")
unfixed(Src, bodyOnly, ignore = shutil.ignore_patterns("AetherHead.ib", "AetherHeadDiffuse.dds", "AetherHeadLightMap.dds"))
ini = os.path.join(bodyOnly, "Aether.ini")
writeIni(ini, dropSections(readIni(ini), r"\[(TextureOverrideAetherHead|ResourceAetherHead)"))

# ---- Synth16 ------------------------------------------------------------------------------------
s16 = os.path.join(Out, "Synth16")
unfixed(Src, s16)
for f in os.listdir(s16):
    if f.endswith(".ib"):
        p = os.path.join(s16, f)
        ib = np.frombuffer(open(p, "rb").read(), dtype = np.uint32)
        assert ib.max() < 65536
        open(p, "wb").write(ib.astype(np.uint16).tobytes())
ini = os.path.join(s16, "Aether.ini")
t = readIni(ini)
n = len(re.findall(r"(?m)^format = DXGI_FORMAT_R32_UINT$", t))
assert n == 2, n
writeIni(ini, t.replace("format = DXGI_FORMAT_R32_UINT", "format = DXGI_FORMAT_R16_UINT"))

# ---- a non-Latin folder name --------------------------------------------------------------------
unfixed(Src, os.path.join(Out, "에테르Synth"))

print("written to", Out, sorted(os.listdir(Out)))

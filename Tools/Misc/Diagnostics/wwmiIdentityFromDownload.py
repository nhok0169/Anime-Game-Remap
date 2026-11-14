# Builds a WWMI identity mod from a Data/Mod Downloads/WuWa/<Name>/<ver> folder (whose buffers ARE the
# identity mod's, byte for byte), reusing wwmiIdentityMod.py's own .ini writer and blend-remap logic.
#   py -3 wwmiIdentityFromDownload.py <download folder> <Name> <mod folder>
import json, os, re, shutil, sys
import numpy as np

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
sys.path.insert(0, os.path.join(REPO, "Tools", "Misc", "Prototypes"))
import wwmiIdentityMod as W

src, name, mod = sys.argv[1], sys.argv[2], sys.argv[3]
meta = json.load(open(os.path.join(src, f"{name}Metadata.json"), encoding="utf-8"))

class C:
    pass

comps = []
for i, e in enumerate(meta["components"]):
    c = C(); c.index = i; c.entry = e
    c.vertexOffset = int(e["vertex_offset"]); c.indexOffset = int(e["index_offset"]); c.vertexCount = int(e["vertex_count"])
    c.indices = np.zeros(int(e["index_count"]), dtype=np.uint32)
    comps.append(c)

srcFiles = {"Index": "Index", "Position": "Position", "Blend": "Blend", "Vector": "Vector", "Color": "Color", "TexCoord": "Texcoord",
            "ShapeKeyOffset": "ShapeKeyOffset", "ShapeKeyVertexId": "ShapeKeyVertexId", "ShapeKeyVertexOffset": "ShapeKeyVertexOffset",
            "BlendRemapVertexVG": "BlendRemapVertexVG", "BlendRemapForward": "BlendRemapForward", "BlendRemapReverse": "BlendRemapReverse"}
meshes = os.path.join(mod, "Meshes"); os.makedirs(meshes, exist_ok=True)
copied = 0
for buf, s in srcFiles.items():
    p = os.path.join(src, f"{name}{s}.buf")
    if os.path.isfile(p):
        shutil.copyfile(p, os.path.join(meshes, W.BufferFiles[buf])); copied += 1
    elif buf in ("Vector",):
        sys.exit(f"MISSING {p}")

vertexCount = int(meta["vertex_count"]); indexCount = int(meta["index_count"])
index = np.fromfile(os.path.join(src, f"{name}Index.buf"), dtype="<u4")
assert index.size == indexCount, (index.size, indexCount)
blendStride = sum(int(s["stride"]) for s in meta["export_format"]["Blend"]["semantics"])
wpv = blendStride // 2
blend = np.fromfile(os.path.join(src, f"{name}Blend.buf"), dtype=np.uint8).reshape(vertexCount, blendStride)
weights = blend[:, wpv:]
vgp = os.path.join(src, f"{name}BlendRemapVertexVG.buf")
remaps = []
if os.path.isfile(vgp):
    ids = np.fromfile(vgp, dtype="<u2").reshape(vertexCount, wpv).astype(np.int64)
    remaps, fwd, rev = W.buildBlendRemaps(comps, ids, weights, index)
    assert fwd.tobytes() == open(os.path.join(src, f"{name}BlendRemapForward.buf"), "rb").read(), "forward differs"
    assert rev.tobytes() == open(os.path.join(src, f"{name}BlendRemapReverse.buf"), "rb").read(), "reverse differs"
shapeKeyCount = os.path.getsize(os.path.join(src, f"{name}ShapeKeyVertexId.buf")) // 4

texDir = os.path.join(mod, "Textures"); os.makedirs(texDir, exist_ok=True)
textures = []
pat = re.compile(rf"^{re.escape(name)}Texture(?P<hash>[0-9a-fA-F]{{8}})\.dds$")
for f in sorted(os.listdir(src)):
    m = pat.match(f)
    if not m:
        continue
    out = f"Components-0 t={m.group('hash').lower()}.dds"
    shutil.copyfile(os.path.join(src, f), os.path.join(texDir, out))
    textures.append((out, m.group("hash").lower()))

with open(os.path.join(mod, "mod.ini"), "w", encoding="utf-8", newline="\r\n") as f:
    f.write(W.iniText(name, "Anime Game Remap", meta, comps, vertexCount, indexCount, shapeKeyCount, textures,
                      blendStride=blendStride, remaps=remaps, weightsPerVertex=wpv))
print(f"{name}: {len(comps)} components, {copied} buffers, {len(textures)} textures, remaps {remaps}, blend stride {blendStride}")

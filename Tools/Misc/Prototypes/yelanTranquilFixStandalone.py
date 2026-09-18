"""
Prototype of the Yelan -> YelanTranquil remap, as one script over any Yelan mod folder.

It is the chain that made the china-dress test mod work in game on 2026-09-12 (see
AI Agent Help/VGRemaps/CLAUDE.md, "Recipe: a mod onto a skin of several components"), lifted off
that one mod: everything about the mod is read from its own .ini (file names, which .ib draws which
object, which texture hangs off which register), everything about the two skins is a constant below.

    py -3 yelanTranquilFix.py <mod folder>                 writes <mod folder>/YelanTranquil.ini + YelanTranquil/
    py -3 yelanTranquilFix.py <mod folder> --dryRun        parse and report only
    py -3 yelanTranquilFix.py <mod folder> --noMatchHair   keep the body-painted hair's colour (default: match it to the head's)
    py -3 yelanTranquilFix.py <mod folder> --hairMask 0.25 keep a faint hair highlight in Tranquil's own light blue (default 0: none)
    py -3 yelanTranquilFix.py <mod folder> --noCompress    write 32bpp textures (fast) instead of re-encoding to BC7

What it does, per the recipe:
  1. splits the mod per target component (fill cut; Bang negative-index with the reverse-sheet augmentation, trimmed)
  2. draws Head/Body/Dress/Extra through Tranquil's slot C (the no-normal-map shader, like Yelan's), NNFix
  3. zeroes the second UV and normalises the vertex colour (G, B -> 128) on every vertex
  4. lightmaps: skin 115-127 -> 255; band 0 where the diffuse is navy (hair, by hue) -> 121; the rest of band 0 -> 177;
     the hair-highlight mask (lightmap B) scaled by --hairMask on the hair
  5. body-object diffuse: hair pixels alpha -> 255 (the head texture already is); optionally colour-matched to the head's hair
  6. the face diffuse re-pointed at Tranquil's face texture hash on ps-t1

The generated .ini sits beside the mod's own and draws the same mod on YelanTranquil; the mod's own
.ini keeps drawing it on Yelan. Nothing of the mod is modified.
"""

import argparse
import os
import re
import sys
from typing import Dict, List, Optional, Tuple

ToolSrc = r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss\Tools\VGRemapFinder\src"
APISrc = r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss\Anime Game Remap (for all users)\api\src\py"
for p in (ToolSrc, APISrc):
    if (p not in sys.path):
        sys.path.insert(0, p)

import numpy as np                                                                   # noqa: E402
from VGRemapFinder.ComponentSplit import (ModBuffers, IniLayout, augmentFromReverse, graphCutSplit, iniText,   # noqa: E402
                                          negativeIndexSplit, remapsFromDraft, verifySplit, writeComponent)
from VGRemapFinder.TextureBands import (BandRule, applyBandTable, hairMask, readTexture,                       # noqa: E402
                                        scaleChannel, setChannel, uvCoverageMask, writeTexture)

# ------------------------------------------------------------------------------------------------
# the two skins

Yelan = {"position": "c58c76f9", "blend": "f6e01e3c", "texcoord": "428b836c", "ib": {"82e14ea2", "ba35247d"},
         "face": "d3c0b54a",
         "objects": {0: "Head", 20913: "Body", 51759: "Dress", 54042: "Extra"}}        # match_first_index -> object

Tranquil = {"Body": {"name": "Body", "position": "02c325ef", "blend": "244a4b2f", "texcoord": "c772811d", "ib": "611d6168", "draw": "3a7b10bb",
                     "objects": ["A", "B", "C"], "objectIndexes": [0, 53631, 67374]},
            "Bang": {"name": "Bang", "position": "c0dc5c2f", "blend": "5d532cca", "texcoord": "d5db917d", "ib": "648d61dd", "draw": "11b90d23",
                     "objects": ["A"], "objectIndexes": [0]},
            "Eye": {"name": "Eye", "position": "6bc61bb3", "blend": "10056b37", "texcoord": "e23e5a53", "ib": "54bc082e", "draw": "61b441bd",
                    "objects": ["A"], "objectIndexes": [0]}}
TranquilFaceDiffuse = "e8ad6095"
Strategies = {"Body": "graphcut", "Eye": "graphcut", "Bang": "negative"}

ORFix = "CommandList\\global\\ORFix\\ORFix"
NNFix = "CommandList\\global\\ORFix\\NNFix"
# slot C = Tranquil's no-normal-map shader, the same family as Yelan's body: diffuse, lightmap, NNFix
Layouts = {"Body": IniLayout({"A": [("ps-t0", "Diffuse"), ("ps-t1", "LightMap")], "B": [("ps-t0", "Diffuse"), ("ps-t1", "LightMap")],
                              "C": [("ps-t0", "Diffuse"), ("ps-t1", "LightMap")]}, NNFix),
           "Bang": IniLayout([("ps-t1", "Diffuse"), ("ps-t2", "LightMap")], ORFix),
           "Eye": IniLayout([("ps-t0", "Diffuse"), ("ps-t1", "LightMap")], NNFix)}
ObjectSlots = {"Head": "C", "Body": "C", "Dress": "C", "Extra": "C"}

Draft = r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss\Data\RemapDrafts\YelanRemapDraft.xlsx"
DraftSheet = "V5.7 Yelan to Tranquil (tool)"          # the same rows as the library's Yelan -> YelanTranquil table

# the band table (see TextureBands): skin, hair by hue, then every other band-0 material
BandTable = [BandRule(115, 127, 255, name = "skin 115-127 -> 255"),
             BandRule(0, 0, 121, where = hairMask, name = "hair (navy) band 0 -> 121"),
             BandRule(0, 0, 177, name = "other band 0 -> 177")]


# ------------------------------------------------------------------------------------------------
# reading the mod's own .ini

class ModIni():
    """
    What a Yelan mod's ``.ini`` says about its files: which buffers, which ``.ib`` and which
    textures draw which object
    """

    def __init__(self, path: str):
        self.path = path
        self.folder = os.path.dirname(os.path.abspath(path))
        self.sections: Dict[str, List[Tuple[str, str]]] = {}
        self.conditional: List[str] = []
        current = None
        with open(path, "r", encoding = "utf-8", errors = "replace") as f:
            for raw in f:
                line = raw.split(";", 1)[0].strip()
                if (not line):
                    continue
                if (line.startswith("[") and line.endswith("]")):
                    current = line[1:-1].strip()
                    self.sections[current] = []
                    continue
                if (current is None):
                    continue
                if (re.match(r"(?i)^(if|elif|else|endif)\b", line)):
                    if (current not in self.conditional):
                        self.conditional.append(current)
                    continue
                if ("=" in line):
                    key, value = line.split("=", 1)
                    self.sections[current].append((key.strip().lower(), value.strip()))

    def get(self, section: str, key: str) -> Optional[str]:
        for k, v in self.sections.get(section, []):
            if (k == key.lower()):
                return v
        return None

    def resourceFile(self, resource: Optional[str]) -> Optional[str]:
        if (not resource or resource.lower() == "null"):
            return None
        name = self.get(resource, "filename")
        if (name is None):
            return None
        return os.path.normpath(os.path.join(self.folder, name.replace("\\", os.sep)))

    def overrides(self, hashValue: str) -> List[str]:
        """the TextureOverride sections on a hash"""
        return [s for s in self.sections if (s.lower().startswith("textureoverride") and (self.get(s, "hash") or "").lower() == hashValue)]

    def isYelan(self) -> bool:
        return bool(self.overrides(Yelan["position"])) and any(self.overrides(h) for h in Yelan["ib"])


def readMod(ini: ModIni):
    """
    The mod's files, from its ``.ini``: buffers, ``.ib`` per object in draw order, textures per
    object by register, the face diffuse
    """

    def firstFile(hashValue: str, key: str) -> Optional[str]:
        for s in ini.overrides(hashValue):
            f = ini.resourceFile(ini.get(s, key))
            if (f):
                return f
        return None

    position = firstFile(Yelan["position"], "vb0")
    blend = firstFile(Yelan["blend"], "vb1")
    texcoord = firstFile(Yelan["texcoord"], "vb1")
    ibHash = next((h for h in Yelan["ib"] if ini.overrides(h)), None)
    objects: Dict[str, Dict[str, Optional[str]]] = {}
    for s in ini.overrides(ibHash) if ibHash else []:
        first = ini.get(s, "match_first_index")
        if (first is None):
            continue
        name = Yelan["objects"].get(int(first))
        if (name is None):
            print(f"  ! {s}: match_first_index {first} is not one of Yelan's objects, skipped")
            continue
        objects[name] = {"section": s, "ib": ini.resourceFile(ini.get(s, "ib")),
                         "Diffuse": ini.resourceFile(ini.get(s, "ps-t0")), "LightMap": ini.resourceFile(ini.get(s, "ps-t1"))}
    objects = {name: objects[name] for _, name in sorted(Yelan["objects"].items()) if (name in objects)}
    face = firstFile(Yelan["face"], "ps-t0")
    return position, blend, texcoord, objects, face


# ------------------------------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description = __doc__, formatter_class = argparse.RawDescriptionHelpFormatter)
    parser.add_argument("mod", help = "the Yelan mod folder (its .ini files are found inside, not recursively)")
    parser.add_argument("--ini", default = None, help = "the mod's .ini to read (default: the first one in the folder that draws Yelan)")
    parser.add_argument("--out", default = "YelanTranquil", help = "the subfolder for the generated buffers and textures (default: %(default)s)")
    parser.add_argument("--iniName", default = "YelanTranquil.ini", help = "the generated .ini's name (default: %(default)s)")
    parser.add_argument("--hairMask", type = float, default = 0.0, help = "factor on the hair-highlight mask, lightmap B on the hair (default: %(default)s)")
    parser.add_argument("--noMatchHair", action = "store_true", help = "do not colour-match the Body object's hair to the Head object's")
    parser.add_argument("--noCompress", action = "store_true", help = "write 32bpp textures instead of re-encoding to the source's BCn")
    parser.add_argument("--dryRun", action = "store_true", help = "parse and report, write nothing")
    args = parser.parse_args()

    modFolder = os.path.abspath(args.mod)
    candidates = [args.ini] if (args.ini) else [os.path.join(modFolder, f) for f in sorted(os.listdir(modFolder))
                                                 if (f.lower().endswith(".ini") and not f.upper().startswith("DISABLED") and f != args.iniName)]
    ini = None
    for c in candidates:
        m = ModIni(c)
        if (m.isYelan()):
            ini = m
            break
    if (ini is None):
        raise SystemExit(f"no .ini in '{modFolder}' draws Yelan (position hash {Yelan['position']} + an IB override)")
    print(f"mod .ini: {ini.path}")
    if (ini.conditional):
        print(f"  ! these sections contain if/else (a $swapvar mod?): {', '.join(ini.conditional)} -- the FIRST value of each key is used")

    position, blend, texcoord, objects, face = readMod(ini)
    for label, f in (("position", position), ("blend", blend), ("texcoord", texcoord)):
        print(f"  {label}: {os.path.relpath(f, modFolder) if f else '-- MISSING'}")
        if (not f):
            raise SystemExit(f"the .ini names no {label} buffer for Yelan")
    for name, o in objects.items():
        print(f"  {name}: ib {os.path.relpath(o['ib'], modFolder) if o['ib'] else 'null'}; diffuse {os.path.relpath(o['Diffuse'], modFolder) if o['Diffuse'] else '-'}; lightmap {os.path.relpath(o['LightMap'], modFolder) if o['LightMap'] else '-'}")
    print(f"  face diffuse: {os.path.relpath(face, modFolder) if face else '-'}")
    if (not objects):
        raise SystemExit("the .ini has no object sections on Yelan's IB hash")

    # 1. the buffers
    mod = ModBuffers.fromFiles(position, blend, texcoord, {name: o["ib"] for name, o in objects.items()}, folder = modFolder, prefix = "yelan")
    print(f"buffers: {mod.vertexCount} vertices, strides {mod.positionStride}/{ModBuffers.BlendStride}/{mod.texcoordStride}; triangles " +
          ", ".join(f"{n} {len(ib)}" for n, ib in mod.ibs.items()))
    if (mod.texcoordStride != 20):
        print(f"  ! texcoord stride {mod.texcoordStride}, expected 20 (COLOR + 2 UV sets): the vertex-colour / second-UV edit assumes 20")

    # 3. the per-vertex data Tranquil's slot-C shader reads and Yelan's does not
    tc = mod.texcoord
    colourBefore = np.unique(tc[:, :4], axis = 0)
    tc[:, 1] = 128
    tc[:, 2] = 128
    if (mod.texcoordStride == 20):
        tc[:, 12:20] = 0
    print(f"texcoord: vertex colour G/B -> 128 (was {colourBefore[:3].tolist()}{'...' if len(colourBefore) > 3 else ''}), second UV zeroed")

    # the remap, per component, with the Bang augmented from the reverse sheet
    remaps = remapsFromDraft(Draft, "Yelan", "YelanTranquil", DraftSheet)
    components = [c for c in Tranquil if (c in remaps)]
    secondary = {c: {s: b for s, (b, _) in v.items()} for c, v in augmentFromReverse(remaps, Draft, "Yelan", "YelanTranquil", [c for c in components if (Strategies[c] == "negative")]).items()}
    groupCount = int(mod.blendIndices[mod.blendWeights > 0].max()) + 1
    unmapped = [g for g in range(groupCount) if not any(g in remaps[c] for c in components)]
    print(f"remap: {', '.join(f'{c} {len(remaps[c])}' for c in components)} rows; source groups used 0..{groupCount - 1}; unmapped: {unmapped or 'none'}")
    if (unmapped):
        print("  ! unmapped source groups become negative bone indices (a kink) -- this mod uses groups the remap does not cover")

    if (args.dryRun):
        print("dry run: nothing written")
        return

    # 1. the split
    outRoot = os.path.join(modFolder, args.out)
    os.makedirs(outRoot, exist_ok = True)
    negativeComponents = [c for c in components if (Strategies[c] == "negative")]
    cutComponents = [c for c in components if (Strategies[c] == "graphcut")]
    negativeResults = negativeIndexSplit(mod, remaps, secondary, trim = True)
    exclude = {}
    for objectName, ib in mod.ibs.items():
        mask = np.zeros(len(ib), dtype = bool)
        for c in negativeComponents:
            if (len(ib)):
                mask |= negativeResults[c].live[ib].all(axis = 1)
        exclude[objectName] = mask
    cutResults = graphCutSplit(mod, remaps, "fill", excludeTriangles = exclude, fillComponents = cutComponents)
    results = {c: (negativeResults if (Strategies[c] == "negative") else cutResults)[c] for c in components}

    # the edited full texcoord, for the negative-index component that draws the whole mod
    texcoordPath = os.path.join(outRoot, "yelanTexcoord.buf")
    np.ascontiguousarray(mod.texcoord).tofile(texcoordPath)
    modGeometry = dict(mod.paths)
    modGeometry["texcoord"] = texcoordPath

    files = {}
    componentGeometry = {}
    for c, result in results.items():
        negative = Strategies[c] == "negative"
        files[c] = writeComponent(os.path.join(outRoot, c), f"yelan{c}", result, writeGeometry = not negative)
        if (negative):
            componentGeometry[c] = modGeometry
        print(f"{c} ({'negative index' if negative else 'fill cut'}): " + ", ".join(f"{k} = {v}" for k, v in result.stats.items() if not isinstance(v, dict)))
    failures = []
    for word in ("negative", "graphcut"):
        subset = {c: r for c, r in results.items() if (Strategies[c] == word)}
        if (subset):
            failures += verifySplit(subset, mod, word)
    for objectName, ib in mod.ibs.items():
        if (not len(ib)):
            continue
        drawnBy = np.zeros(len(ib), dtype = int)
        for c in components:
            r = results[c]
            if (Strategies[c] == "negative"):
                drawnBy += r.live[ib].all(axis = 1)
            else:
                keys = {tuple(t) for t in r.vertices[r.ibs[objectName]].tolist()} if (len(r.ibs[objectName])) else set()
                drawnBy += np.array([tuple(t) in keys for t in ib.tolist()], dtype = int) if keys else 0
        print(f"coverage {objectName}: {len(ib)} triangles, drawn by nobody {int((drawnBy == 0).sum())}, by more than one {int((drawnBy > 1).sum())}")
        if ((drawnBy == 0).any() or (drawnBy > 1).any()):
            failures.append(f"{objectName}: coverage")
    for f in failures:
        print(f"  FAILED {f}")

    # 4 + 5. the textures
    import FixRaidenBoss2 as FRB
    textureOverrides: Dict[Tuple[str, str], str] = {}
    uv = mod.texcoord[:, 4:12].copy().view("<f4") if (mod.texcoordStride == 20) else None
    def covered(name: str, shape) -> Optional[np.ndarray]:
        """the texels this object's triangles sample -- a mod may keep the source skin's whole texture on one half"""
        return uvCoverageMask(uv, mod.ibs[name], shape[1], shape[0]) if (uv is not None and len(mod.ibs.get(name, ()))) else None
    headHair = None
    if ("Head" in objects and objects["Head"]["Diffuse"] and len(mod.ibs.get("Head", ()))):
        _, headPixels = readTexture(FRB, objects["Head"]["Diffuse"])
        cov = covered("Head", headPixels.shape)
        headHair = (headPixels, hairMask(headPixels) & (cov if cov is not None else True))
    for name, o in objects.items():
        if (not len(mod.ibs.get(name, ()))):
            continue                                                   # an object with no triangles has nothing to draw
        diffusePixels = None
        if (o["Diffuse"]):
            dtex, diffusePixels = readTexture(FRB, o["Diffuse"])
            original = diffusePixels.copy()                                # the masks are decided on the mod's own art, before any edit
            cov = covered(name, diffusePixels.shape)
            hair = hairMask(original)                                      # every hair texel: an unsampled one costs nothing, a missed one seams
            n = setChannel(diffusePixels, 3, 255, hair)
            note = f"alpha 255 on {n} hair px"
            if (name != "Head" and headHair is not None and not args.noMatchHair and n):
                stats = hair & (cov if cov is not None else True)          # the statistics from the hair this object actually draws
                src = diffusePixels[..., :3][stats].astype(np.float64); ref = headHair[0][..., :3][headHair[1]].astype(np.float64)
                if (len(src) and len(ref)):
                    allHair = diffusePixels[..., :3][hair].astype(np.float64)
                    matched = (allHair - src.mean(0)) * (ref.std(0) / np.maximum(src.std(0), 1e-6)) + ref.mean(0)
                    diffusePixels[..., :3][hair] = np.clip(np.round(matched), 0, 255).astype(np.uint8)
                    note += f"; hair colour {src.mean(0).round(0).tolist()} -> {ref.mean(0).round(0).tolist()} (the head's; statistics over {int(stats.sum())} sampled px)"
            dest = os.path.join(outRoot, f"yelan{name}Diffuse.dds")
            writeTexture(dtex, diffusePixels, dest, compress = not args.noCompress)
            textureOverrides[(name, "Diffuse")] = dest
            print(f"{name} diffuse: {note} -> {os.path.relpath(dest, modFolder)}")
        if (o["LightMap"]):
            ltex, lightPixels = readTexture(FRB, o["LightMap"])
            ref = original if (diffusePixels is not None) else None
            if (ref is not None and ref.shape[:2] != lightPixels.shape[:2]):
                from PIL import Image
                ref = np.array(Image.fromarray(ref).resize((lightPixels.shape[1], lightPixels.shape[0]), Image.NEAREST))
            counts = applyBandTable(lightPixels, BandTable, ref)
            nB = scaleChannel(lightPixels, 2, args.hairMask, hairMask(ref)) if (ref is not None) else 0
            dest = os.path.join(outRoot, f"yelan{name}LightMap.dds")
            writeTexture(ltex, lightPixels, dest, compress = not args.noCompress)
            textureOverrides[(name, "LightMap")] = dest
            print(f"{name} lightmap: " + ", ".join(f"{k}: {v}" for k, v in counts.items()) + f"; highlight mask x{args.hairMask} on {nB} hair px -> {os.path.relpath(dest, modFolder)}")
    if (face):
        textureOverrides[("FaceHead", "Diffuse")] = face

    # 2 + 6. the .ini
    text = iniText("Yelan", "YelanTranquil", results, Tranquil, Layouts, files, modFolder, mod.positionStride, mod.texcoordStride, ModBuffers.BlendStride,
                   componentGeometry = componentGeometry, componentStrategies = {c: Strategies[c] for c in components},
                   strategy = "fill / negative-index (Yelan -> YelanTranquil prototype)", faceDiffuseHash = TranquilFaceDiffuse, faceRegister = "ps-t1",
                   textureOverrides = textureOverrides, objectSlots = ObjectSlots)
    iniPath = os.path.join(modFolder, args.iniName)
    with open(iniPath, "w", encoding = "utf-8", newline = "\r\n") as f:
        f.write(text)
    print(f"ini: {iniPath}")
    print("result: " + ("OK" if (not failures) else f"{len(failures)} problem(s) above"))


if (__name__ == "__main__"):
    main()

#
# ===== yelanTranquilFix (prototype v2) =====
#
# Yelan -> YelanTranquil, driven by the API's own parser and fixer.
#
#   py -3 yelanTranquilFix.py <mod folder>                 fix every Yelan .ini under the folder
#   py -3 yelanTranquilFix.py <mod folder> --components Body,Bang     only some target components
#   py -3 yelanTranquilFix.py <mod folder> --hairMask 0.25 keep a faint hair highlight (lightmap B) in Tranquil's own light blue
#   py -3 yelanTranquilFix.py <mod folder> --noMatchHair   leave the body-painted hair its own colour
#   py -3 yelanTranquilFix.py <mod folder> --keepBackups   keep the .ini backups the API makes
#
# What the API does: classify the mod's sections by hash (the shared GIMICharParser, on hash / index
# rows registered at runtime for Yelan and three pseudo targets YelanTranquilBody / Bang / Eye), copy
# each drawn object's graph onto the target component's draw slot (GraphGroupRemap -- a second object
# on the same slot lands in a second .ini file, the API's merge shape), rewrite the hash
# (RegAssetRemap), the match_first_index (RegNewVals, windowed by GIMIObjPartFilter), the register
# layout (RegRemap), the draw call (RegFillMissing / RegRemove) and the external fix call
# (RegDelimitedAdd), collect the vb0 / vb1 / ib / ps-t* registers into new resources (ResRegCollect)
# and write the .ini and the files.
#
# What this script supplies: the BYTES of each resource, through the resources' fixFunc --
# Tools/VGRemapFinder's ComponentSplit decides which triangles each component draws and what its
# blend looks like (Body / Eye: graph cut in fill mode; Bang: negative index, trimmed), and
# TextureBands rewrites the lightmap bands and the hair alpha / colour. Textures go through the API's
# TexEditor on the Pillow engine (32-bit uncompressed output).
#
# How the target's own body parts are hidden: the remapped ("", "ib") section keeps `handling = skip`
# and LOSES its `drawindexed = auto` (moveDrawIndexed), so nothing the mod does not draw itself is
# redrawn -- Tranquil's slots A and B never appear, without a hide section per slot.
#

import argparse
import os
import re
import sys
from typing import Dict, List, Optional, Tuple

import numpy as np

Repo = os.environ.get("AG_REMAP_REPO", r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss")
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
ToolSrc = os.path.join(Repo, "Tools", "VGRemapFinder", "src")
sys.path.insert(0, APISrc)
sys.path.insert(0, ToolSrc)
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))

import FixRaidenBoss2 as FRB                                                                                   # noqa: E402
from PIL import Image                                                                                          # noqa: E402
from VGRemapFinder.ComponentSplit import (ModBuffers, augmentFromReverse, graphCutSplit, negativeIndexSplit,   # noqa: E402
                                          remapsFromDraft, verifySplit)
from VGRemapFinder.TextureBands import BandRule, applyBandTable, hairMask, scaleChannel, setChannel, uvCoverageMask   # noqa: E402


# ============================================================================== the pair

V = "6.1"
NNFix = "CommandList\\global\\ORFix\\NNFix"
ORFix = "CommandList\\global\\ORFix\\ORFix"

Yelan = {"draw_vb": "589fed34", "position_vb": "c58c76f9", "blend_vb": "f6e01e3c", "texcoord_vb": "428b836c",
         "ib": "82e14ea2", "ibOld": "ba35247d", "tex_face_diffuse": "d3c0b54a",
         "objects": {0: "head", 20913: "body", 51759: "dress", 54042: "extra"}}
Tranquil = {"Body": {"draw_vb": "3a7b10bb", "position_vb": "02c325ef", "blend_vb": "244a4b2f", "texcoord_vb": "c772811d", "ib": "611d6168",
                     "slots": {"A": 0, "B": 53631, "C": 67374}},
            "Bang": {"draw_vb": "11b90d23", "position_vb": "c0dc5c2f", "blend_vb": "5d532cca", "texcoord_vb": "d5db917d", "ib": "648d61dd",
                     "slots": {"A": 0}},
            "Eye": {"draw_vb": "61b441bd", "position_vb": "6bc61bb3", "blend_vb": "10056b37", "texcoord_vb": "e23e5a53", "ib": "54bc082e",
                    "slots": {"A": 0}}}
TranquilFaceDiffuse = "e8ad6095"

# Per target component: which strategy splits the mod's triangles for it, which of the target's draw
# slots the mod is drawn through (the one whose pixel-shader family matches Yelan's: slot C for the
# Body, the no-normal-map shader), how the diffuse / lightmap registers sit there, and which external
# fix is re-issued. Body's slot C and the Eye read ps-t0 / ps-t1 with NNFix; the Bang reads ps-t1 /
# ps-t2 under ORFix.
Plan = {"Body": {"strategy": "graphcut", "slot": "C", "regs": {}, "fix": NNFix, "face": True},
        "Bang": {"strategy": "negative", "slot": "A", "regs": {"ps-t0": "ps-t1", "ps-t1": "ps-t2"}, "fix": ORFix, "face": False},
        "Eye": {"strategy": "graphcut", "slot": "A", "regs": {}, "fix": NNFix, "face": False}}

Draft = os.path.join(Repo, "Data", "RemapDrafts", "YelanRemapDraft.xlsx")
DraftSheet = "V5.7 Yelan to Tranquil (tool)"          # the same rows as the library's Yelan -> YelanTranquil table

# Lightmap alpha is a material band. Yelan: 115-127 skin, 0 hair + cloth. Tranquil (slot C): 255
# skin, 121 hair (dithered 115-128), 177 silk. Hair is told from cloth by hue on the diffuse.
BandTable = [BandRule(115, 127, 255, name = "skin 115-127 -> 255"),
             BandRule(0, 0, 121, where = hairMask, name = "hair (navy) band 0 -> 121"),
             BandRule(0, 0, 177, name = "other band 0 -> 177")]

targetName = lambda component: "YelanTranquil" + component
keepName = lambda name: name


# ============================================================================== the tables

def registerYelan(components: List[str]) -> FRB.ModType:
    """Yelan and one pseudo target per component, as a runtime ModType on the shipped GI builders"""
    FRB.CppGlobalModTypes.registerAll()
    shared = FRB.CppGlobalModTypes.all()[0].vgRemaps
    GI, YID = int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Yelan)

    hashRows = [([V, "Yelan", key], Yelan[key]) for key in ("draw_vb", "position_vb", "blend_vb", "texcoord_vb", "ib", "tex_face_diffuse")]
    indexRows = [([V, "Yelan", "", obj], str(index)) for index, obj in Yelan["objects"].items()]
    vg = shared.clone()
    for component in components:
        target = targetName(component)
        hashRows += [([V, target, key], Tranquil[component][key]) for key in ("draw_vb", "position_vb", "blend_vb", "texcoord_vb", "ib")]
        hashRows.append(([V, target, "tex_face_diffuse"], TranquilFaceDiffuse))
        row = shared.get(["Yelan", "", "YelanTranquil", component], ["1.0", "5.7"], errorOnNotFound = False)
        if (row is None):
            raise SystemExit(f"the API's shared vertex group table has no Yelan -> YelanTranquil {component} row")
        vg.addRows([(["1.0", "Yelan", "", V, target, ""], dict(row.remap))])

    targets = [targetName(c) for c in components]
    hashes = FRB.Hashes({"Yelan": targets}); hashes.addRepoRows(hashRows)
    indices = FRB.Indices({"Yelan": targets}); indices.addRepoRows(indexRows)
    modType = FRB.ModType(GI, YID, "Yelan", [], hashes, indices, None, vg)
    template = next(m for m in FRB.CppGlobalModTypes.all() if m.name == "Keqing")   # any shipped GI type: its builders are the table builders, which consult the overrides
    modType.iniParseBuilder = template.iniParseBuilder
    modType.iniFixBuilder = template.iniFixBuilder
    modType.iniRemoveBuilder = template.iniRemoveBuilder
    FRB.ModTypeIdTools.registerModType(modType)
    FRB.CppGlobalModTypes.registerMissing()
    return modType


# ============================================================================== the parser

def makeParser(modType: FRB.ModType):
    """
    The same parser makeGIMICharParser builds, assembled from Python so the fixer can reach its
    IniFile: a GIMIParser whose sections are sorted by the API's hash / index classifier -- the
    drawn objects by the shared ib hash plus their match_first_index, everything else by a hash of
    its own
    """
    drawnObjs = [("", name) for name in Yelan["objects"].values()]
    hashOnly = {"ib": ("", "ib"), "blend_vb": ("", "blend"), "position_vb": ("", "position"), "texcoord_vb": ("", "texcoord"),
                "draw_vb": ("", "other"), "tex_face_diffuse": ("", "face")}
    modObjs = drawnObjs + [obj for obj in hashOnly.values() if (obj not in drawnObjs)]

    def factory(iniFile, modTypeId):
        classifier = FRB.GIMISectionClassifier(dict(hashOnly), modType.hashes, {"ib": {obj: obj for obj in drawnObjs}}, modType.indices, None)
        # Reverse lookups filtered to Yelan's own rows: the pseudo targets also have an index "0"
        # (their slot A), and without the filter the head's match_first_index resolves to one of
        # theirs and the head falls through to the shared ("", "ib") graph.
        classifier.hashNonVersionVals = {"name": "Yelan"}
        classifier.indexNonVersionVals = {"name": "Yelan"}
        parser = FRB.GIMIParser(iniFile, modObjs = modObjs, objTargetFuncs = [classifier], modTypeId = modTypeId)
        parser.trackKeys = True
        parser.keysToTrack = {"hash", "match_first_index"}
        _alive.append(classifier)
        return parser
    return factory


# ============================================================================== the split

class ModSplit():
    """
    One mod's buffers, split per target component, plus its textures -- computed once per .ini,
    shared by the three component fixers
    """

    def __init__(self, ini, args, components: List[str]):
        self.folder = os.path.dirname(os.path.abspath(ini.file))
        self.args = args
        self.position, self.blend, self.texcoord, self.objects, self.face = self._readFiles(ini)
        rel = lambda p: os.path.relpath(p, self.folder) if p else "-"
        print(f"  files: position {rel(self.position)}, blend {rel(self.blend)}, texcoord {rel(self.texcoord)}, face diffuse {rel(self.face)}")
        for name, o in self.objects.items():
            print(f"    {name}: ib {rel(o['ib'])}; diffuse {rel(o['Diffuse'])}; lightmap {rel(o['LightMap'])}")
        for label, f in (("position", self.position), ("blend", self.blend), ("texcoord", self.texcoord)):
            if (not f):
                raise ValueError(f"the .ini names no {label} buffer for Yelan")
        if (not self.objects):
            raise ValueError("the .ini has no object sections on Yelan's IB hash")

        # 1. the buffers, and the per-vertex data Tranquil's slot-C shader reads and Yelan's does not
        self.mod = ModBuffers.fromFiles(self.position, self.blend, self.texcoord, {n: o["ib"] for n, o in self.objects.items()}, folder = self.folder, prefix = "yelan")
        print(f"  buffers: {self.mod.vertexCount} vertices, strides {self.mod.positionStride}/{ModBuffers.BlendStride}/{self.mod.texcoordStride}; triangles "
              + ", ".join(f"{n} {len(ib)}" for n, ib in self.mod.ibs.items()))
        tc = self.mod.texcoord
        tc[:, 1] = 128
        tc[:, 2] = 128
        if (self.mod.texcoordStride == 20):
            tc[:, 12:20] = 0
        else:
            print(f"  ! texcoord stride {self.mod.texcoordStride}, expected 20 (COLOR + 2 UV sets): only the vertex colour was normalised")

        # 2. the remap, per component, with the negative-index components augmented from the reverse sheet
        remaps = remapsFromDraft(Draft, "Yelan", "YelanTranquil", DraftSheet)
        self.components = [c for c in components if (c in remaps)]
        negative = [c for c in self.components if (Plan[c]["strategy"] == "negative")]
        cut = [c for c in self.components if (Plan[c]["strategy"] == "graphcut")]
        secondary = {c: {s: b for s, (b, _) in v.items()} for c, v in augmentFromReverse(remaps, Draft, "Yelan", "YelanTranquil", negative).items()}
        groupCount = int(self.mod.blendIndices[self.mod.blendWeights > 0].max()) + 1
        unmapped = [g for g in range(groupCount) if not any(g in remaps[c] for c in self.components)]
        print(f"  remap: {', '.join(f'{c} {len(remaps[c])}' for c in self.components)} rows; source groups 0..{groupCount - 1}; unmapped: {unmapped or 'none'}")

        # 3. the split: negative-index components draw every fully-live triangle, the cut components share the rest
        negativeResults = negativeIndexSplit(self.mod, remaps, secondary, trim = True)
        exclude = {}
        for name, ib in self.mod.ibs.items():
            mask = np.zeros(len(ib), dtype = bool)
            for c in negative:
                if (len(ib)):
                    mask |= negativeResults[c].live[ib].all(axis = 1)
            exclude[name] = mask
        cutResults = graphCutSplit(self.mod, remaps, "fill", excludeTriangles = exclude, fillComponents = cut)
        self.results = {c: (negativeResults if (c in negative) else cutResults)[c] for c in self.components}
        for c, r in self.results.items():
            print(f"  {c} ({Plan[c]['strategy']}): " + ", ".join(f"{k} = {v}" for k, v in r.stats.items() if not isinstance(v, dict)))
        failures = []
        for word in ("negative", "graphcut"):
            subset = {c: r for c, r in self.results.items() if (Plan[c]["strategy"] == word)}
            if (subset):
                failures += verifySplit(subset, self.mod, word)
        for name, ib in self.mod.ibs.items():
            if (not len(ib)):
                continue
            drawnBy = np.zeros(len(ib), dtype = int)
            for c, r in self.results.items():
                if (Plan[c]["strategy"] == "negative"):
                    drawnBy += r.live[ib].all(axis = 1)
                elif (len(r.ibs.get(name, ()))):
                    keys = {tuple(t) for t in r.vertices[r.ibs[name]].tolist()}
                    drawnBy += np.array([tuple(t) in keys for t in ib.tolist()], dtype = int)
            print(f"  coverage {name}: {len(ib)} triangles, drawn by nobody {int((drawnBy == 0).sum())}, by more than one {int((drawnBy > 1).sum())}")
            if ((drawnBy == 0).any() or (drawnBy > 1).any()):
                failures.append(f"{name}: coverage")
        for f in failures:
            print(f"  FAILED {f}")

        self._originals: Dict[str, np.ndarray] = {}
        self._edited: Dict[Tuple[str, str], np.ndarray] = {}

    # ---- reading the mod's files out of the API's parsed sections ----

    def _readFiles(self, ini):
        """The buffers, the .ib and textures per object, the face diffuse -- by hash, from IniFile's own sections"""
        templates = ini.getIfTemplates()

        def parts(template):
            return [p for p in template.parts if (hasattr(p, "getVals"))]

        def first(template, key) -> Optional[str]:
            for p in parts(template):
                vals = p.getVals(key)
                if (vals):
                    return vals[0].strip()
            return None

        def fileOf(resource: Optional[str]) -> Optional[str]:
            if (not resource or resource.lower() == "null" or resource not in templates):
                return None
            f = first(templates[resource], "filename")
            return os.path.normpath(os.path.join(self.folder, f.replace("\\", os.sep))) if f else None

        position = blend = texcoord = face = None
        objects: Dict[str, Dict[str, Optional[str]]] = {}
        for template in templates.values():
            h = (first(template, "hash") or "").lower()
            if (h == Yelan["position_vb"]):
                position = position or fileOf(first(template, "vb0"))
            elif (h == Yelan["blend_vb"]):
                blend = blend or fileOf(first(template, "vb1"))
            elif (h == Yelan["texcoord_vb"]):
                texcoord = texcoord or fileOf(first(template, "vb1"))
            elif (h == Yelan["tex_face_diffuse"]):
                face = face or fileOf(first(template, "ps-t0"))
            elif (h in (Yelan["ib"], Yelan["ibOld"])):
                index = first(template, "match_first_index")
                if (index is None):
                    continue
                name = Yelan["objects"].get(int(index))
                if (name is None):
                    print(f"  ! {template.name}: match_first_index {index} is not one of Yelan's objects, skipped")
                    continue
                objects[name] = {"ib": fileOf(first(template, "ib")), "Diffuse": fileOf(first(template, "ps-t0")), "LightMap": fileOf(first(template, "ps-t1"))}
        objects = {name: objects[name] for _, name in sorted(Yelan["objects"].items()) if (name in objects)}
        return position, blend, texcoord, objects, face

    # ---- what each component draws ----

    def drawn(self, component: str) -> List[str]:
        """The mod objects with at least one triangle in this component, in draw order"""
        r = self.results[component]
        return [name for name in self.objects if (len(r.ibs.get(name, ())))]

    def objectOf(self, path: str, kind: str) -> Optional[str]:
        """Which mod object a source file belongs to, by the path the .ini named"""
        for name, o in self.objects.items():
            if (o[kind] and os.path.normcase(os.path.abspath(o[kind])) == os.path.normcase(os.path.abspath(path))):
                return name
        return None

    def blendBytes(self, component: str) -> bytes:
        r = self.results[component]
        return ModBuffers.encodeBlend(r.blendWeights, r.blendIndices)

    def positionBytes(self, component: str) -> bytes:
        return np.ascontiguousarray(self.results[component].position).tobytes()

    def texcoordBytes(self, component: str) -> bytes:
        rows = self.mod.texcoord if (Plan[component]["strategy"] == "negative") else self.results[component].texcoord
        return np.ascontiguousarray(rows).tobytes()

    def ibBytes(self, component: str, name: str) -> bytes:
        return np.ascontiguousarray(self.results[component].ibs[name], dtype = "<u4").tobytes()

    # ---- the textures ----

    def original(self, path: str) -> np.ndarray:
        if (path not in self._originals):
            self._originals[path] = np.array(Image.open(path).convert("RGBA"))
        return self._originals[path]

    def _covered(self, name: str, shape) -> Optional[np.ndarray]:
        """the texels this object's triangles sample -- a mod may keep the source skin's whole texture on one half"""
        if (self.mod.texcoordStride != 20 or not len(self.mod.ibs.get(name, ()))):
            return None
        uv = self.mod.texcoord[:, 4:12].copy().view("<f4")
        return uvCoverageMask(uv, self.mod.ibs[name], shape[1], shape[0])

    def edited(self, name: str, kind: str) -> np.ndarray:
        """The object's diffuse or lightmap after the edit, computed once"""
        key = (name, kind)
        if (key in self._edited):
            return self._edited[key]
        o = self.objects[name]
        if (kind == "Diffuse"):
            original = self.original(o["Diffuse"])
            pixels = original.copy()
            hair = hairMask(original)                                          # every hair texel: an unsampled one costs nothing, a missed one seams
            n = setChannel(pixels, 3, 255, hair)
            note = f"alpha 255 on {n} hair px"
            head = self.objects.get("head")
            if (name != "head" and head and head["Diffuse"] and not self.args.noMatchHair and n and len(self.mod.ibs.get("head", ()))):
                headPixels = self.original(head["Diffuse"])
                headCov = self._covered("head", headPixels.shape)
                refMask = hairMask(headPixels) & (headCov if (headCov is not None) else True)
                cov = self._covered(name, original.shape)
                stats = hair & (cov if (cov is not None) else True)             # the statistics from the hair this object actually draws
                src = pixels[..., :3][stats].astype(np.float64)
                ref = headPixels[..., :3][refMask].astype(np.float64)
                if (len(src) and len(ref)):
                    allHair = pixels[..., :3][hair].astype(np.float64)
                    matched = (allHair - src.mean(0)) * (ref.std(0) / np.maximum(src.std(0), 1e-6)) + ref.mean(0)
                    pixels[..., :3][hair] = np.clip(np.round(matched), 0, 255).astype(np.uint8)
                    note += f"; hair colour {src.mean(0).round(0).tolist()} -> {ref.mean(0).round(0).tolist()} (the head's)"
            print(f"  {name} diffuse: {note}")
        else:
            pixels = self.original(o["LightMap"]).copy()
            ref = self.original(o["Diffuse"]) if (o["Diffuse"]) else None
            if (ref is not None and ref.shape[:2] != pixels.shape[:2]):
                ref = np.array(Image.fromarray(ref).resize((pixels.shape[1], pixels.shape[0]), Image.NEAREST))
            counts = applyBandTable(pixels, BandTable, ref)
            nB = scaleChannel(pixels, 2, self.args.hairMask, hairMask(ref)) if (ref is not None) else 0
            print(f"  {name} lightmap: " + ", ".join(f"{k}: {v}" for k, v in counts.items()) + f"; highlight mask x{self.args.hairMask} on {nB} hair px")
        self._edited[key] = pixels
        return pixels


# ============================================================================== the fixers

_splits: Dict[str, ModSplit] = {}
_written = set()                   # texture files written this run
_alive: List[object] = []          # every edit handed to the API, kept alive for the run


def splitFor(ini, args, components: List[str]) -> ModSplit:
    key = os.path.normcase(os.path.abspath(ini.file))
    if (key not in _splits):
        print(f"{os.path.basename(ini.file)}:")
        _splits[key] = ModSplit(ini, args, components)
    return _splits[key]


class BufferReplace(FRB.RemapBlendReplace):
    """
    RemapBlendReplace for any buffer this script supplies the bytes of: a blend, a position, a
    texcoord or an index buffer. resType picks the stats bucket ("blend" / "position" / "texcoord" /
    "buf" -- the API's own default, "resourceRemapBlend", is counted nowhere); the built resource is
    kept referenced from Python as well.
    """

    def buildResModel(self, *args, **kwargs):
        resource = super().buildResModel(*args, **kwargs)
        if (resource is not None):
            _alive.append(resource)
        return resource


def writeBytesFunc(getBytes):
    """A resource fixFunc: write the bytes 'getBytes(resource)' gives to the resource's fixed path"""
    def fix(resource) -> bool:
        data = getBytes(resource)
        if (data is None):
            return False
        with open(resource.fixedPath, "wb") as f:
            f.write(data)
        return True
    return fix


def makeFixer(component: str, args, components: List[str]):
    plan = Plan[component]
    slot, slotObj = plan["slot"], ("", plan["slot"])
    naming = FRB.CppIniNamingTools

    def factory(parser, toModName: str, modTypeId: int):
        ini = parser._iniFile
        modType = FRB.ModTypeIdTools.getModType(modTypeId)
        split = splitFor(ini, args, components)
        if (component not in split.results):
            raise ValueError(f"no remap rows for {component}")
        drawn = split.drawn(component)
        cut = plan["strategy"] == "graphcut"
        result = split.results[component]
        groups = max(len(drawn), 1)
        print(f"  {toModName}: draws {', '.join(drawn) or 'nothing'} through slot {slot}" + (f" ({groups} .ini groups)" if (groups > 1) else ""))

        # ---- 1. copy each drawn object's graph onto the component's draw slot ----
        #
        # Every drawn object goes to the SAME slot; the second claimant lands in group 1, the third in
        # group 2 (the API's merge shape: one extra .ini file per extra group, and the game overlaps
        # them). Every other graph is copied unrenamed into every group, exactly as the shipped
        # template does, so each file is complete on its own.
        remap = {}
        for name in Yelan["objects"].values():
            remap[(0, "", name)] = [(0, "", slot)] if (name in drawn) else []
        for kind in ("ib", "blend", "position", "texcoord", "other", "face"):
            wanted = drawn and (kind != "face" or plan["face"])
            remap[(0, "", kind)] = [(0, "", kind, keepName) for _ in range(groups)] if wanted else []
        edits: List[object] = [FRB.GraphGroupRemap(remap = remap)]

        # ---- 2. the resources: the API collects the register, this script supplies the bytes ----
        blendHash = Yelan["blend_vb"]

        def blendParts(iterData):
            colouring = iterData.colouring
            if (colouring is not None and any(val.lower() == blendHash for _, val in colouring.getIndVals("hash"))):
                return FRB.Ranges.createFull()
            return FRB.Ranges.createEmpty()

        for g in range(groups):
            if (not drawn):
                break
            edits.append(FRB.ResRegCollect({(g, "", "blend"): "vb1"},
                                           {"blend": BufferReplace((g, "", "blendRemap"), resType = "blend", fixFunc = writeBytesFunc(lambda r: split.blendBytes(component)))},
                                           partPredicates = {(g, "", "blend"): blendParts},
                                           trackKeys = {(g, "", "blend"): True}, keysToTrack = {(g, "", "blend"): {"hash"}}))
            if (cut):
                edits.append(FRB.ResRegCollect({(g, "", "position"): "vb0"},
                                               {"position": BufferReplace((g, "", "positionRemap"), resType = "position", fixFunc = writeBytesFunc(lambda r: split.positionBytes(component)),
                                                                              resSubType = "Position")}))
            edits.append(FRB.ResRegCollect({(g, "", "texcoord"): "vb1"},
                                           {"texcoord": BufferReplace((g, "", "texcoordRemap"), resType = "texcoord", fixFunc = writeBytesFunc(lambda r: split.texcoordBytes(component)),
                                                                              resSubType = "Texcoord")}))

            def ibBytes(resource):
                name = split.objectOf(resource.srcPath, "ib")
                if (name is None):
                    print(f"  ! {os.path.basename(resource.srcPath)} is not the .ib of any of Yelan's objects")
                    return None
                return split.ibBytes(component, name)
            edits.append(FRB.ResRegCollect({(g, "", slot): "ib"},
                                           {"ib": BufferReplace((g, "", slot + "RemapIb"), resType = "buf", fixFunc = writeBytesFunc(ibBytes), resSubType = "Ib")}))

            for reg, kind in (("ps-t0", "Diffuse"), ("ps-t1", "LightMap")):
                def texFix(resource, kind = kind) -> bool:
                    name = split.objectOf(resource.srcPath, kind)
                    if (name is None):
                        print(f"  ! {os.path.basename(resource.srcPath)} is not the {kind} of any of Yelan's objects")
                        return False
                    if (resource.fixedPath in _written):
                        return True                                    # the same edit of the same texture, for another component
                    _written.add(resource.fixedPath)
                    editor = FRB.TexEditor([lambda texFile: setattr(texFile, "img", Image.fromarray(split.edited(name, kind), "RGBA"))],
                                           engine = FRB.TexEngine.Pillow, compress = False)
                    editor.fix(FRB.TextureFile(resource.srcPath, engine = FRB.TexEngine.Pillow), resource.fixedPath)
                    return True
                placeholder = FRB.TexEditor([], engine = FRB.TexEngine.Pillow, compress = False)
                edits.append(FRB.ResRegCollect({(g, "", slot): reg},
                                               {"tex" + kind: FRB.TexReplace((g, "", slot + "RemapTex" + kind), placeholder, fixFunc = texFix, resSubType = kind)}))

        # ---- 3. the index, windowed to the copied object's own KVPs (head and body share the ib hash) ----
        objFilter = FRB.GIMIObjPartFilter(modType.hashes, modType.indices, {"ib"}, None)
        _alive.append(objFilter)          # filter() hands out callables that point back at it; let it outlive this factory
        indexEdits, indexFilters, indexKeys, indexTrack = [], [], [], []
        for g, name in enumerate(drawn):
            indexEdits.append({slotObj: [FRB.RegNewVals({"match_first_index": str(Tranquil[component]["slots"][slot])})]})
            indexFilters.append({slotObj: [objFilter.filter(("", name))]})
            indexKeys.append({slotObj: objFilter.keysToTrack()})
            indexTrack.append({slotObj: True})
        if (drawn):
            edits.append(FRB.GraphGroupEdit(indexEdits, trackKeys = indexTrack, keysToTrack = indexKeys, keyFilters = indexFilters))

        # ---- 4. everything else, per group ----
        rename = FRB.GraphRename(lambda name: naming.getRemapFixName(name, toModName))
        hashRemap = FRB.RegAssetRemap({"hash": (modType.hashes, "HashNotFound")}, toModName, "Yelan", ini.fromVersion, ini.toVersion)
        dropFixCalls = FRB.RegRemove({"run": lambda _ind, val: val in (NNFix, ORFix)})
        fillDraw = FRB.RegFillMissing("drawindexed", "auto")                  # the draw call moves onto each object...
        removeDraw = FRB.RegRemove({"drawindexed": None})                      # ...and off the shared ib section, which now only skips
        addFix = FRB.RegDelimitedAdd([("run", plan["fix"])], {"drawindexed": []}, pathEndOnlyWhenUndelimited = True)
        regs = FRB.RegRemap({src: [dst] for src, dst in plan["regs"].items()})
        perGroup = []
        for g in range(groups):
            group = {slotObj: [dropFixCalls] + ([regs] if plan["regs"] else []) + [fillDraw, addFix, hashRemap],
                     ("", "ib"): [FRB.GraphRename(lambda name: naming.getRemapIbName(name, toModName)), hashRemap, removeDraw],
                     ("", "blend"): [FRB.GraphRename(lambda name: naming.getRemapBlendName(name, toModName)), hashRemap]
                                    + ([FRB.RegNewVals({"draw": f"{result.vertexCount},0"})] if cut else []),
                     ("", "position"): [rename, hashRemap],
                     ("", "texcoord"): [rename, hashRemap],
                     ("", "other"): [rename, hashRemap]}
            if (plan["face"]):
                group[("", "face")] = [rename, hashRemap, FRB.RegRemap({"ps-t0": ["ps-t1"], "ps-t1": ["ps-t0"]})]
            perGroup.append(group)
        if (drawn):
            edits.append(FRB.GraphGroupEdit(perGroup))

        _alive.extend(edits)
        fixer = FRB.GIMIFixer(parser, graphGroupEdits = edits, modsToFix = [toModName])
        _alive.append(fixer)
        return fixer
    return factory


# ============================================================================== run

def main():
    parser = argparse.ArgumentParser(description = "Yelan -> YelanTranquil, through the API's parser and fixer")
    parser.add_argument("mod", help = "the mod folder (every Yelan .ini under it is fixed)")
    parser.add_argument("--components", default = "Body,Bang,Eye", help = "target components to produce (default: %(default)s)")
    parser.add_argument("--hairMask", type = float, default = 0.0, help = "factor on the hair-highlight mask, lightmap B on the hair (default: %(default)s)")
    parser.add_argument("--noMatchHair", action = "store_true", help = "do not move the body-painted hair's colour onto the head-painted hair's")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "print every resource file as it is written")
    parser.add_argument("--loop", action = "store_true", help = "drive parse / fix / resources per .ini from this script instead of RemapService (an API built before 2026-09-12 needs this)")
    args = parser.parse_args()
    components = [c.strip() for c in args.components.split(",") if c.strip()]
    unknown = [c for c in components if (c not in Plan)]
    if (unknown):
        raise SystemExit(f"unknown component(s) {unknown}; choose from {list(Plan)}")

    modType = registerYelan(components)
    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser("Yelan", makeParser(modType))
    for component in components:
        FRB.CppStrategyOverrides.setFixer("Yelan", targetName(component), makeFixer(component, args, components))

    try:
        if (args.loop):
            fixFolder(os.path.abspath(args.mod), args, components)
        else:
            runService(os.path.abspath(args.mod), args)
    finally:
        FRB.CppStrategyOverrides.clear()


def runService(folder: str, args) -> None:
    """The whole run through RemapService: folder walk, undo of a previous fix, backups, resources, summary"""
    service = FRB.RemapService(path = folder, keepBackups = args.keepBackups, forcedModTypeIds = {int(FRB.ModTypeId.Yelan)},
                               logger = FRB.Logger() if args.verbose else None)
    service.fix()
    stats = service.stats
    print(f"\n.ini fixed: {len(stats.ini.fixed)}, skipped: {len(stats.ini.skipped)}")
    for path, error in stats.ini.skipped.items():
        print(f"  SKIPPED {os.path.relpath(path, folder)}: {error}")
    for label in ("blend", "position", "texcoord", "buf", "texEdit"):
        s = getattr(stats, label)
        print(f"{label}: fixed {len(s.fixed)}, skipped {len(s.skipped)}")
        for path in sorted(s.fixed):
            print(f"  {os.path.relpath(path, folder)}")
        for path, error in s.skipped.items():
            print(f"  SKIPPED {os.path.relpath(path, folder)}: {error}")
    if (not stats.blend.fixed and not stats.ini.skipped):
        print("  ! no buffer was written: an API built before 2026-09-12 cannot call a Python fixFunc from RemapService's resource loop"
              " (RemapBlendResource is not copyable) -- rerun with --loop, or rebuild the API")


def fixFolder(folder: str, args, components: List[str]) -> None:
    """
    Every .ini under the folder, through the API: undo a previous fix, parse, fix, then fix the
    resources the fix collected -- the same sequence RemapService runs, minus the service.

    The --loop fallback for an API built before 2026-09-12, whose C++ resource loop could not hand a
    (non-copyable) RemapBlendResource to a Python fixFunc; resources fixed from Python resolve to the
    wrappers getResources() hands out and work on any build.
    """
    YID = int(FRB.ModTypeId.Yelan)
    fixed, skipped, resources = [], {}, {"fixed": [], "skipped": []}
    for root, _, files in os.walk(folder):
        for name in sorted(files):
            if (not name.lower().endswith(".ini") or name.upper().startswith("DISABLED")):
                continue
            if (re.search(r"RemapFix\d+\.ini$", name, re.IGNORECASE)):
                os.remove(os.path.join(root, name))        # a merge's extra file from a previous run; the fix writes it again
                continue
            path = os.path.join(root, name)
            try:
                ini = FRB.IniFile(file = path, forcedFromModTypeIds = {YID})
                ini.removeFix(False, True, False, args.keepBackups)
                ini.clearModels()
                ini.parse()
                ini.fix(keepBackup = args.keepBackups)
            except Exception as e:
                skipped[path] = e
                print(f"  SKIPPED {os.path.relpath(path, folder)}: {type(e).__name__}: {e}")
                continue
            fixed.append(path)
            for resource in ini.getResources():
                if (not hasattr(resource, "fix")):
                    continue
                try:
                    ok = resource.fix()
                except Exception as e:
                    ok = False
                    print(f"  ! {os.path.basename(resource.fixedPath)}: {type(e).__name__}: {e}")
                resources["fixed" if ok else "skipped"].append(resource.fixedPath)
                if (args.verbose):
                    print(f"  {'fixed' if ok else 'FAILED'} {os.path.relpath(resource.fixedPath, folder)}")

    print(f"\n.ini fixed: {len(fixed)}, skipped: {len(skipped)}")
    print(f"resources written: {len(set(resources['fixed']))}, failed: {len(resources['skipped'])}")
    for path in sorted(set(resources["fixed"])):
        print(f"  {os.path.relpath(path, folder)}")
    for path in resources["skipped"]:
        print(f"  FAILED {os.path.relpath(path, folder)}")


if (__name__ == "__main__"):
    main()

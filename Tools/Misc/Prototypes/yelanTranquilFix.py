#
# ===== yelanTranquilFix (prototype v3) =====
#
# Yelan -> YelanTranquil, driven by the API's own parser, fixer and resource groups.
#
#   py -3 yelanTranquilFix.py <mod folder>                         fix every Yelan .ini under the folder
#   py -3 yelanTranquilFix.py <mod folder> --components Body,Bang  only some target components
#   py -3 yelanTranquilFix.py <mod folder> --keepBackups           keep the .ini backups the API makes
#   py -3 yelanTranquilFix.py <mod folder> --verbose               attach the API's logger
#   py -3 yelanTranquilFix.py <mod folder> --loop                  drive parse / fix / resources per .ini from here
#                                                                  instead of RemapService (an API built before 2026-09-12)
#
# Linux / WSL (the only side with a core built after 2026-09-12 until the Windows .pyd is rebuilt):
#   python yelanTranquilFix.py /mnt/e/.../yelanMod                 from a shell that has the API's venv active
#   python yelanTranquilFix.py "E:\...\yelanMod"                    a Windows path is translated to /mnt/<drive>/...
#   py -3 yelanTranquilFix.py <mod folder> --wsl                   from Windows: relaunch this same command under WSL
#                                                                  (distro AG_REMAP_WSL_DISTRO, default Ubuntu-22.04;
#                                                                  venv AG_REMAP_WSL_VENV, default ~/agremap-venv)
# The repo is found at AG_REMAP_REPO, else at its Windows path, translated to /mnt/... on Linux.
# Point it at a folder that holds ONE mod: the service walks every subfolder, and each Yelan .ini
# found there is fixed too, with its buffers written next to the files it references.
#
# What the API does: classify the mod's sections by hash (a GIMIParser on the API's hash classifier,
# over hash / index rows registered at runtime for Yelan and three pseudo targets YelanTranquilBody /
# Bang / Eye), copy each drawn object's graph onto the target component's draw slot (GraphGroupRemap --
# a second object on the same slot lands in a second .ini file, the API's merge shape), rewrite the
# hash (RegAssetRemap), the match_first_index (RegNewVals, windowed by GIMIObjPartFilter), the
# register layout (RegRemap), the draw call (RegFillMissing / RegRemove) and the external fix call
# (RegDelimitedAdd), edit or create the textures (ResRegCollect + TexReplace / TexCreate), and
# collect the mod's BUFFERS AS ONE RESOURCE GROUP per .ini group (ResGroupCollect + BufReplace):
# the Blend.buf decides which vertices a component keeps, the .ib decides which triangles, and the
# Position.buf / Texcoord.buf have to follow the same vertex set -- so they are fixed together, by
# the API's VGSplitGroupResource (issue #190). This script only says WHICH component draws through
# WHICH slot with WHICH textures; nothing of the geometry work lives here any more.
#
# The texture recipe is the user's Copy28: the slots that read a normal map (Tranquil's Body slot C
# and her Bang, under ORFix) get a flat one created, the head diffuse at alpha 1 and every object's
# lightmap moved band to band (Yelan's fur 255 onto Tranquil's fur 0, Yelan's skin 115-127 onto
# Tranquil's skin 255); the Eye keeps the head's original textures under NNFix.
#
# How the target's own body parts are hidden: the remapped ("", "ib") section keeps `handling = skip`
# and LOSES its `drawindexed = auto` (moveDrawIndexed), so nothing the mod does not draw itself is
# redrawn -- Tranquil's slots A and B never appear, without a hide section per slot.
#

import argparse
import os
import re
import sys
from typing import Dict, List, Optional

import numpy as np

OnWindows = (sys.platform == "win32")


def winToPosix(path: str) -> str:
    """`E:\\a\\b` -> `/mnt/e/a/b` on Linux (WSL's default drive mounts); any other path is returned as is"""
    if (OnWindows or (len(path) < 2) or (path[1] != ":") or (not path[0].isalpha())):
        return path
    return "/mnt/" + path[0].lower() + path[2:].replace("\\", "/")


Repo = os.environ.get("AG_REMAP_REPO") or winToPosix(r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss")
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
if (not os.path.isdir(APISrc)):
    raise SystemExit(f"the API's package folder is not at {APISrc}; set AG_REMAP_REPO to the repo's path on this OS")
sys.path.insert(0, APISrc)
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))

import FixRaidenBoss2 as FRB     # noqa: E402
from PIL import Image            # noqa: E402

for needed in ("VGComponentSplit", "VGSplitGroupResource", "BufReplace"):
    if (not hasattr(FRB, needed)):
        raise SystemExit(f"this API build has no {needed}: it needs the core built after 2026-09-12 (the component split)")


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
# slots the mod is drawn through, and the slot's register layout. "normalMap" is the layout Tranquil's
# main-pass shader reads (ps-t0 normal map, ps-t1 diffuse, ps-t2 lightmap, re-slotted by ORFix); the
# Eye reads ps-t0 diffuse / ps-t1 lightmap under NNFix.
Plan = {"Body": {"strategy": "graphcut", "slot": "C", "normalMap": True, "fix": ORFix, "face": True},
        "Bang": {"strategy": "negative", "slot": "A", "normalMap": True, "fix": ORFix, "face": False},
        "Eye": {"strategy": "graphcut", "slot": "A", "normalMap": False, "fix": NNFix, "face": False}}

# The lightmap alpha is a material band, and the legend differs per skin. Read off the two skins' own
# textures at their vertices (the identity mod, Mods/Yelan4, is Yelan's):
#   Yelan:     0 = hair and dark cloth, 64-89 metal, 115-127 skin, 165-189 ornaments, 255 = white FUR
#              (the shawl, the trims, the jacket lining)
#   Tranquil:  0 = white fur, 64-89 silver, 115-128 hair, 165-189 silk and the lace cape, 255 = skin
# So two bands move and the rest already line up: Yelan's skin onto Tranquil's, and Yelan's fur onto
# Tranquil's fur -- which matters twice: her own fur shawl would otherwise render as SKIN, and a mod author
# who leaves the alpha opaque has painted every cloth as fur (the Fontaine mod's grey stockings came out
# beige until the move). The table is applied to every object's lightmap, the head's included.
#
# But a mod author need not follow Yelan's legend at all: a port keeps its SOURCE character's bands (the
# Clorinde port has its hair on 115-127 and its skin on 50-99), and lifting its hair onto Tranquil's skin
# ramp put speckles all over it. So the skin lift is conditional on the DIFFUSE under the pixel looking
# like skin (warm, R > G > B, bright enough) -- dark-blue hair on the skin band stays where it is, which on
# Tranquil is her hair band. The fur move stays unconditional: a mod's 255 is never Tranquil-legend skin.
SkinBand = (115, 127, 255)
FurBand = (255, 0)


def skinColoured(rgb) -> "np.ndarray":
    """Where a diffuse (H x W x 3, uint8) is skin-coloured: warm and bright, red over green over blue"""
    r, g, b = (rgb[..., i].astype(np.int16) for i in range(3))
    return (r >= 96) & (r >= g) & (g >= b) & ((r - b) >= 16) & ((r - b) <= 140)
# The flat normal map the reference draws with is 127 / 127 / 255 under a BC7_UNORM_SRGB header; a created
# texture is written untagged, so it carries what that samples as: round(255 * (127 / 255) ** 2.2) = 55
FlatNormal = (55, 55, 255, 255)

targetName = lambda component: "YelanTranquil" + component
keepName = lambda name: name


# ============================================================================== the tables

def registerYelan(components: List[str]) -> FRB.ModType:
    """Yelan and one pseudo target per component, as a runtime ModType on the shipped GI builders"""
    FRB.CppGlobalModTypes.registerAll()
    shared = FRB.CppGlobalModTypes.all()[0].vgRemaps
    GI, YID = int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Yelan)

    hashRows = [([V, "Yelan", key], Yelan[key]) for key in ("draw_vb", "position_vb", "blend_vb", "texcoord_vb", "ib", "tex_face_diffuse")]
    # No index rows for the pseudo targets: nothing looks them up (RegNewVals writes the index from
    # the table above), and a reverse lookup of "0" that could land on a target row blinds the
    # classifier and GIMIObjPartFilter's window
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


def componentSpecs(components: List[str]) -> List[FRB.VGComponentSpec]:
    """
    Every component of the target, from the API's shared table: the forward row is the component's
    bones, and for a negative-index component the REVERSE row turned around is its secondary bones
    (the Bang's bone 0 is Yelan's head 64: a hair vertex weighted head + bang keeps its head weight)
    """
    shared = FRB.CppGlobalModTypes.all()[0].vgRemaps
    specs = []
    for component in components:
        forward = dict(shared.get(["Yelan", "", "YelanTranquil", component], ["1.0", "5.7"], errorOnNotFound = False).remap)
        secondary = {}
        if (Plan[component]["strategy"] == "negative"):
            reverse = shared.get(["YelanTranquil", component, "Yelan", ""], ["1.0", "5.7"], errorOnNotFound = False)
            if (reverse is not None):
                for bone, source in reverse.remap.items():
                    if (source not in forward and source not in secondary):
                        secondary[source] = bone
        specs.append(FRB.VGComponentSpec(component, forward, secondary = secondary, negativeIndex = Plan[component]["strategy"] == "negative"))
    return specs


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
        # Reverse lookups filtered to Yelan's own rows: the pseudo targets share hash types with her
        classifier.hashNonVersionVals = {"name": "Yelan"}
        classifier.indexNonVersionVals = {"name": "Yelan"}
        parser = FRB.GIMIParser(iniFile, modObjs = modObjs, objTargetFuncs = [classifier], modTypeId = modTypeId)
        parser.trackKeys = True
        parser.keysToTrack = {"hash", "match_first_index"}
        _alive.append(classifier)
        return parser
    return factory


# ============================================================================== the mod

class ModFiles():
    """
    One mod's files, read out of the API's parsed sections, and what each component of the target
    would draw of it -- computed once per .ini, shared by the component fixers. The fixer factories
    run BEFORE the parser parses, so the files are found by hash over IniFile.getIfTemplates() rather
    than through the parser's graphs
    """

    def __init__(self, ini, components: List[str]):
        self.folder = os.path.dirname(os.path.abspath(ini.file))
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

        # The split, once, to know what each component draws (the fixer needs it before any resource
        # exists); the grouped resource redoes it in C++ when it writes the files
        blendRaw = np.frombuffer(open(self.blend, "rb").read(), dtype = np.uint8)
        if (len(blendRaw) % 32):
            raise ValueError(f"'{self.blend}' is not a whole number of 32-byte lines")
        rows = blendRaw.reshape(-1, 32)
        weights = rows[:, :16].copy().view("<f4").reshape(-1, 4)
        indices = rows[:, 16:].copy().view("<i4").reshape(-1, 4)
        self.vertexCount = len(rows)
        self.ibNames = [name for name, o in self.objects.items() if (o["ib"])]
        self.ibPaths = [self.objects[name]["ib"] for name in self.ibNames]
        ibs = [np.frombuffer(open(p, "rb").read(), dtype = "<u4").reshape(-1, 3).tolist() for p in self.ibPaths]
        self.texcoordStride = os.path.getsize(self.texcoord) // self.vertexCount if (self.vertexCount) else 0
        self.positionStride = os.path.getsize(self.position) // self.vertexCount if (self.vertexCount) else 0
        print(f"  buffers: {self.vertexCount} vertices, texcoord stride {self.texcoordStride}; triangles " + ", ".join(f"{n} {len(ib)}" for n, ib in zip(self.ibNames, ibs)))

        self.components = components
        self.specs = componentSpecs(components)
        split = FRB.VGComponentSplit(weights.tolist(), indices.tolist(), ibs, self.specs)
        self.results: Dict[str, FRB.VGComponentBuffers] = {c: split.split(c) for c in components}
        for c, r in self.results.items():
            s = r.stats
            kept = ", ".join(f"{n} {k}" for n, k in zip(self.ibNames, s.trianglesKept))
            print(f"  {c} ({Plan[c]['strategy']}): vertices {s.keptVertices} of {s.vertexCount}; triangles {kept}"
                  + (f"; sentinels {s.sentinels}" if (Plan[c]["strategy"] == "negative") else f"; renormalised {s.renormalised}, neighbour-skinned {s.neighbourSkinned}"))
        for i, name in enumerate(self.ibNames):
            drawnBy = np.zeros(len(ibs[i]), dtype = int)
            for c, r in self.results.items():
                keys = {tuple(t) for t in (np.array(r.vertices)[np.array(r.ibs[i], dtype = np.int64)].tolist() if (len(r.ibs[i]) and Plan[c]["strategy"] == "graphcut") else r.ibs[i])}
                drawnBy += np.array([tuple(t) in keys for t in ibs[i]], dtype = int) if keys else 0
            print(f"  coverage {name}: {len(ibs[i])} triangles, drawn by nobody {int((drawnBy == 0).sum())}, by more than one {int((drawnBy > 1).sum())}")

        self._sizes: Dict[str, tuple] = {}

    def _readFiles(self, ini):
        templates = ini.getIfTemplates()

        def first(template, key) -> Optional[str]:
            for p in template.parts:
                if (hasattr(p, "getVals")):
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

    def drawn(self, component: str) -> List[str]:
        """The mod objects with at least one triangle in this component, in draw order"""
        kept = self.results[component].stats.trianglesKept
        return [name for name, n in zip(self.ibNames, kept) if (n > 0)]

    def keptVertices(self, component: str) -> int:
        return self.results[component].stats.keptVertices

    def objectOf(self, path: str, kind: str) -> Optional[str]:
        for name, o in self.objects.items():
            if (o[kind] and os.path.normcase(os.path.abspath(o[kind])) == os.path.normcase(os.path.abspath(path))):
                return name
        return None

    def textureSize(self, name: str) -> tuple:
        """(width, height) of the object's diffuse, for the flat normal map created beside it"""
        if (name not in self._sizes):
            path = self.objects[name]["Diffuse"]
            with Image.open(path) as image:
                self._sizes[name] = image.size
        return self._sizes[name]

    def texcoordLineEdit(self):
        """
        The per-vertex data Tranquil's shader reads and Yelan's does not: vertex colour G = B = 128
        (the mod carried 188) and a zeroed second UV set (Tranquil's opaque vertices carry none)
        """
        stride = self.texcoordStride
        def edit(line: bytes) -> bytes:
            out = bytearray(line)
            out[1] = 128
            out[2] = 128
            if (stride == 20):
                out[12:20] = bytes(8)
            return bytes(out)
        return edit


# ============================================================================== the textures

def alphaOne(texFile) -> None:
    """The head diffuse at alpha 1 -- Tranquil's shader darkens by diffuse alpha, Yelan's ignores it"""
    texFile.img.putalpha(1)


def liftBands(diffusePath: Optional[str]):
    """
    The lightmap filter for one object: its bands from Yelan's legend onto Tranquil's -- fur 255 -> 0,
    then skin 115-127 -> 255 where the object's diffuse agrees that the pixel is skin
    """
    def lift(texFile) -> None:
        pixels = np.array(texFile.img)
        alpha = pixels[..., 3]
        fur = (alpha == FurBand[0])
        skin = (alpha >= SkinBand[0]) & (alpha <= SkinBand[1])
        if (diffusePath and os.path.isfile(diffusePath)):
            diffuse = FRB.TextureFile(diffusePath, readPillowImg = True)
            diffuse.open()
            if (diffuse.hasImage):
                img = diffuse.img.convert("RGB")
                if (img.size != texFile.img.size):
                    img = img.resize(texFile.img.size, Image.BILINEAR)
                skin &= skinColoured(np.asarray(img))
        alpha[fur] = FurBand[1]
        alpha[skin] = SkinBand[2]
        texFile.img = Image.fromarray(pixels, "RGBA")
    return lift


# ============================================================================== the fixers

_files: Dict[str, ModFiles] = {}
_alive: List[object] = []          # every edit handed to the API, kept alive for the run
_written = set()                   # texture files written this run (one edit of one texture serves every component)


def filesFor(ini, components: List[str]) -> ModFiles:
    key = os.path.normcase(os.path.abspath(ini.file))
    if (key not in _files):
        print(f"{os.path.basename(ini.file)}:")
        _files[key] = ModFiles(ini, components)
    return _files[key]


def texReplace(files: ModFiles, resModObj, kind: str, filterFunc) -> FRB.TexReplace:
    """A texture edit through the API's Pillow-engine TexEditor, written once per source texture"""
    def fix(resource) -> bool:
        if (resource.fixedPath in _written):
            return True
        _written.add(resource.fixedPath)
        # The Compressonator engine, not Pillow: it reads the DX10 header, and a BC7_UNORM_SRGB source
        # (the head diffuse) gets the 1/2.2 pre-correction baked in on the way out, since the file is
        # written back untagged. Pillow writes a legacy header with the values untouched, and the
        # shader then samples them without the sRGB decode: a visibly brighter texture in game.
        # mipmaps: every texture the game ships carries its chain, and one written without it is
        # sampled from its top level at every distance -- speckles over the hair (2026-09-12)
        editor = FRB.TexEditor([filterFunc], readPillowImg = True, compress = True, mipmaps = True)
        editor.fix(FRB.TextureFile(resource.srcPath, readPillowImg = True), resource.fixedPath)
        return True
    placeholder = FRB.TexEditor([], compress = True)
    return FRB.TexReplace(resModObj, placeholder, fixFunc = fix, resSubType = kind)


def makeFixer(component: str, components: List[str]):
    plan = Plan[component]
    slot, slotObj = plan["slot"], ("", plan["slot"])
    naming = FRB.CppIniNamingTools

    def factory(parser, toModName: str, modTypeId: int):
        ini = parser._iniFile
        modType = FRB.ModTypeIdTools.getModType(modTypeId)
        files = filesFor(ini, components)
        drawn = files.drawn(component)
        cut = plan["strategy"] == "graphcut"
        groups = max(len(drawn), 1)
        print(f"  {toModName}: draws {', '.join(drawn) or 'nothing'} through slot {slot}" + (f" ({groups} .ini groups)" if (groups > 1) else ""))

        # ---- 1. copy each drawn object's graph onto the component's draw slot ----
        #
        # Every drawn object goes to the SAME slot; the second claimant lands in group 1, which is a
        # second .ini file (the API's merge shape). Every other graph is copied unrenamed into every
        # group, exactly as the shipped template does, so each file is complete on its own.
        remap = {}
        for name in Yelan["objects"].values():
            remap[(0, "", name)] = [(0, "", slot)] if (name in drawn) else []
        for kind in ("ib", "blend", "position", "texcoord", "other", "face"):
            wanted = drawn and (kind != "face" or plan["face"])
            remap[(0, "", kind)] = [(0, "", kind, keepName) for _ in range(groups)] if wanted else []
        edits: List[object] = [FRB.GraphGroupRemap(remap = remap)]

        for g, name in enumerate(drawn):
            # ---- 2. the textures: edit, then the flat normal map where the slot reads one ----
            if (plan["normalMap"]):
                if (name == "head"):
                    edits.append(FRB.ResRegCollect({(g, "", slot): "ps-t0"}, {"diffuse": texReplace(files, (g, "", slot + "RemapTexDiffuse"), "Diffuse", alphaOne)}))
                edits.append(FRB.ResRegCollect({(g, "", slot): "ps-t1"}, {"lightMap": texReplace(files, (g, "", slot + "RemapTexLightMap"), "LightMap", liftBands(files.objects[name]["Diffuse"]))}))
                # ps-t0 is duplicated onto a scratch register that the created normal map then
                # replaces; the main edit renames it back to ps-t0 once the shift is done
                edits.append(FRB.GraphGroupEdit([{slotObj: [FRB.RegRemap({"ps-t0": ["ps-t1", "ps-tNormal"], "ps-t1": ["ps-t2"]})]} if (i == g) else {} for i in range(groups)]))
                creator = FRB.TexCreator(1024, 1024, FRB.CppColour(*FlatNormal), compress = True, mipmaps = True)   # flat: any size serves every object
                edits.append(FRB.ResRegCollect({(g, "", slot): "ps-tNormal"}, {"normalMap": FRB.TexCreate((g, "", slot + "RemapNormal"), "NormalMap", creator)}))

        # ---- 3. the buffers, as ONE resource group: split together by VGSplitGroupResource ----
        #
        # The collect splices the collected register into an `if 1 ... endif` block, which splits the
        # section into parts; the draw call below is filled with RegFillMissingMode.BottomCover (a
        # fresh LAST part) so it still lands after the ib and the textures whatever the order here.
        #
        # The blend, the texcoord and this object's ib, plus the position for a cut component (a
        # negative-index component draws every vertex, so it keeps the mod's own Position.buf).
        # Every drawn object's ib is handed to the group, whether or not this group holds it: the
        # vertex set is the union over all of them.
        for g, name in enumerate(drawn):
            kinds = {"blend": ((g, "", "blend"), "vb1"), "texcoord": ((g, "", "texcoord"), "vb1"), "ib": ((g, "", slot), "ib")}
            if (cut):
                kinds["position"] = ((g, "", "position"), "vb0")
            srcRegs, resEdits = {}, {}
            for kind, (srcGraph, reg) in kinds.items():
                resObj = (g, "", f"{slot}Remap{kind.capitalize()}")
                srcRegs[resObj] = {srcGraph: reg}
                resEdits[resObj] = {component: FRB.BufReplace(resObj, kind, resSubType = name if (kind == "ib") else None)}
            builder = FRB.IniGroupedResBuilder(FRB.VGSplitGroupResource, args = [f"Yelan{toModName}Buffers"],
                                               kwargs = {"component": component, "specs": files.specs, "ibPaths": files.ibPaths,
                                                         "texcoordLineEdit": files.texcoordLineEdit()})
            edits.append(FRB.ResGroupCollect([component], srcRegs, resEdits, {component: builder}, id = g))

        # ---- 4. the index, windowed to the copied object's own KVPs (head and body share the ib hash) ----
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

        # ---- 5. everything else, per group ----
        rename = FRB.GraphRename(lambda n: naming.getRemapFixName(n, toModName))
        hashRemap = FRB.RegAssetRemap({"hash": (modType.hashes, "HashNotFound")}, toModName, "Yelan", ini.fromVersion, ini.toVersion)
        dropFixCalls = FRB.RegRemove({"run": lambda _ind, val: val in (NNFix, ORFix)})
        fillDraw = FRB.RegFillMissing("drawindexed", "auto", fillMode = FRB.RegFillMissingMode.BottomCover)   # the draw call moves onto each object, at its END...
        removeDraw = FRB.RegRemove({"drawindexed": None})                      # ...and off the shared ib section, which now only skips
        addFix = FRB.RegDelimitedAdd([("run", plan["fix"])], {"drawindexed": []}, pathEndOnlyWhenUndelimited = True)
        normalBack = FRB.RegRemap({"ps-tNormal": ["ps-t0"]})
        perGroup = []
        for g in range(groups):
            group = {slotObj: [dropFixCalls] + ([normalBack] if plan["normalMap"] else []) + [fillDraw, addFix, hashRemap],
                     ("", "ib"): [FRB.GraphRename(lambda n: naming.getRemapIbName(n, toModName)), hashRemap, removeDraw],
                     ("", "blend"): [FRB.GraphRename(lambda n: naming.getRemapBlendName(n, toModName)), hashRemap]
                                    + ([FRB.RegNewVals({"draw": f"{files.keptVertices(component)},0"})] if cut else []),
                     ("", "position"): [rename, hashRemap],
                     ("", "texcoord"): [rename, hashRemap],
                     ("", "other"): [rename, hashRemap,
                                     FRB.RegNewVals({"override_byte_stride": str(files.positionStride), "override_vertex_count": str(files.keptVertices(component))},
                                                    addNewKVPs = True)]}
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
    parser = argparse.ArgumentParser(description = "Yelan -> YelanTranquil, through the API's parser, fixer and resource groups")
    parser.add_argument("mod", help = "the mod folder (every Yelan .ini under it is fixed)")
    parser.add_argument("--components", default = "Body,Bang,Eye", help = "target components to produce (default: %(default)s)")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--loop", action = "store_true", help = "drive parse / fix / resources per .ini from this script instead of RemapService")
    parser.add_argument("--wsl", action = "store_true", help = "from Windows: run this same command under WSL, where the core is built (AG_REMAP_WSL_DISTRO / AG_REMAP_WSL_VENV)")
    args = parser.parse_args()
    if (args.wsl):
        raise SystemExit(relaunchUnderWsl(args))
    args.mod = winToPosix(args.mod)
    components = [c.strip() for c in args.components.split(",") if c.strip()]
    unknown = [c for c in components if (c not in Plan)]
    if (unknown):
        raise SystemExit(f"unknown component(s) {unknown}; choose from {list(Plan)}")

    modType = registerYelan(components)
    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser("Yelan", makeParser(modType))
    for component in components:
        FRB.CppStrategyOverrides.setFixer("Yelan", targetName(component), makeFixer(component, components))

    try:
        if (args.loop):
            fixFolder(os.path.abspath(args.mod), args)
        else:
            runService(os.path.abspath(args.mod), args)
    finally:
        FRB.CppStrategyOverrides.clear()


def relaunchUnderWsl(args) -> int:
    """
    Run this script again inside WSL with the same arguments (minus --wsl): the venv's python, the
    script and the mod folder addressed through /mnt/<drive>/..., the repo through AG_REMAP_REPO
    """
    import shlex
    import subprocess

    if (not OnWindows):
        raise SystemExit("--wsl is for a Windows shell; this is already Linux")
    distro = os.environ.get("AG_REMAP_WSL_DISTRO", "Ubuntu-22.04")
    venv = os.environ.get("AG_REMAP_WSL_VENV", "~/agremap-venv")
    toPosix = lambda p: "/mnt/" + p[0].lower() + p[2:].replace("\\", "/")
    script = toPosix(os.path.abspath(__file__))
    mod = toPosix(os.path.abspath(args.mod))
    repo = toPosix(os.path.abspath(Repo))
    flags = [f"--components={args.components}"] + [f"--{name}" for name in ("keepBackups", "verbose", "loop") if getattr(args, name)]
    command = f"source {venv}/bin/activate && AG_REMAP_REPO={shlex.quote(repo)} python {shlex.quote(script)} {shlex.quote(mod)} {' '.join(flags)}"
    print(f"wsl -d {distro}: {command}")
    return subprocess.call(["wsl", "-d", distro, "--", "bash", "-lc", command])


def runService(folder: str, args) -> None:
    """The whole run through RemapService: folder walk, undo of a previous fix, backups, resources, summary"""
    service = FRB.RemapService(path = folder, keepBackups = args.keepBackups, forcedModTypeIds = {int(FRB.ModTypeId.Yelan)},
                               logger = FRB.Logger() if args.verbose else None)
    service.fix()
    stats = service.stats
    print(f"\n.ini fixed: {len(stats.ini.fixed)}, skipped: {len(stats.ini.skipped)}")
    for path, error in stats.ini.skipped.items():
        print(f"  SKIPPED {os.path.relpath(path, folder)}: {error}")
    for label in ("blend", "position", "texcoord", "buf", "texEdit", "texAdd"):
        s = getattr(stats, label)
        print(f"{label}: fixed {len(s.fixed)}, skipped {len(s.skipped)}")
        for path in sorted(s.fixed):
            print(f"  {os.path.relpath(path, folder)}")
        for path, error in s.skipped.items():
            print(f"  SKIPPED {os.path.relpath(path, folder)}: {error}")


def fixFolder(folder: str, args) -> None:
    """
    Every .ini under the folder, through the API: undo a previous fix, parse, fix, then fix the
    resources the fix collected -- the same sequence RemapService runs, minus the service (for an API
    whose service loop predates the 2026-09-12 fixFunc fix)
    """
    YID = int(FRB.ModTypeId.Yelan)
    fixed, skipped, written, failed = [], {}, [], []
    for root, _, names in os.walk(folder):
        for name in sorted(names):
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
            for resource in list(ini.getResources()) + list(ini.getGroupedResources()):
                try:
                    ok = resource.fix()
                except Exception as e:
                    ok = False
                    print(f"  ! {getattr(resource, 'name', getattr(resource, 'fixedPath', '?'))}: {type(e).__name__}: {e}")
                members = resource.memberResources() if hasattr(resource, "memberResources") else [resource]
                for member in members:
                    (written if ok else failed).append(getattr(member, "fixedPath", member.srcPath))
    print(f"\n.ini fixed: {len(fixed)}, skipped: {len(skipped)}")
    print(f"resources written: {len(set(written))}, failed: {len(failed)}")
    for path in sorted(set(written)):
        print(f"  {os.path.relpath(path, folder)}")
    for path in failed:
        print(f"  FAILED {os.path.relpath(path, folder)}")


if (__name__ == "__main__"):
    main()

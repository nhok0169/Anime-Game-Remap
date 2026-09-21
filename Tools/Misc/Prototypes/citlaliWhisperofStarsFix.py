#
# ===== citlaliWhisperofStarsFix (prototype v1) =====
#
# Citlali -> CitlaliWhisperofStars, driven by the API's own parser, fixer and resource groups. The
# machinery is bennettAdventureFix.py's (itself yelanTranquilFix.py's); the tables and the three
# differences below are this pair's. Anything learnt here should go back into those.
#
#   py -3 citlaliWhisperofStarsFix.py <mod folder>                            fix every Citlali .ini under the folder
#   py -3 citlaliWhisperofStarsFix.py <mod folder> --components Body,Eyes     only some target components
#   py -3 citlaliWhisperofStarsFix.py <mod folder> --keepBackups              keep the .ini backups the API makes
#   py -3 citlaliWhisperofStarsFix.py <mod folder> --verbose                  attach the API's logger
#
# ---- What the 6.7 frame analyses settled (2026-09-21) ----
#
# Read off FrameAnalysis-Citlali-2026-09-21-101712 and FrameAnalysis-CitlaliWhisperofWinds-2026-09-21-102307
# with Tools/Misc/Diagnostics/giDrawTable.py:
#
#   * the skin draws SIX slots through three skinned components -- Body A (first index 0), B (60888),
#     C (111096), D (122916), Bangs A (0), Eyes A (0). Body A, B, C, the Bangs and the Eyes are all
#     on the NORMAL-MAP shader 2c157719180b096c (bound LND); Body D is on a plain one (diffuse, light
#     map) and is a small skin-only piece nothing is remapped onto;
#   * the Bangs and the Eyes read Body slot A's textures (same hashes);
#   * the skin's Face / Mouth / Eyebrows meshes are unskinned and SHARED with base Citlali by hash --
#     the game draws them for both, so this fix never touches them; only the face DIFFUSE differs
#     (9fb78572 -> 5783625d), and both characters bind it at ps-t1.
#
# ---- Three things that differ from Bennett ----
#
#   1. CITLALI'S OWN SECTIONS ARE ALREADY THE NORMAL-MAP LAYOUT (ps-t0 normal map, ps-t1 diffuse,
#      ps-t2 light map): her Head and Body draw through the same shader family as the skin's slots.
#      So there is normally NO register shift and NO created normal map -- the two structural edits
#      Bennett could not do without. The layout is read per OBJECT off the mod's own section (a
#      third ps-t bound => normal-map layout), and a plain-layout mod still gets Bennett's shift.
#   2. HER HAIR HAS BONES OF ITS OWN, so the skin's Bangs component is a real target (the forward
#      table has a Citlali -> Bangs row), where Bennett's Bang was left to the skin.
#   3. THE FACE SWAP IS GUARDED: the two-way ps-t0 <-> ps-t1 swap is applied only when the mod binds
#      its face diffuse at ps-t0 (a pre-6.x mod). A mod already on ps-t1 -- every identity mod, and
#      any mod made from a 6.x dump -- is left alone, which is what the Bennett prototype deferred.
#
# ---- Which slot, and the one band move (measured 2026-09-21) ----
#
# Light map ALPHA per object, over the texels each object's UVs cover, with the diffuse under it:
#
#   Citlali head   0 (49%, pale lavender)   255 (50%, her navy HAIR)
#   Citlali body   0 (40%, purple cloth)   78-80 (30%, navy cloth)   255 (29%, 96% skin-coloured = SKIN)
#   skin Body A    0 (6%)  78 (23%, pale cloth)  128 (3%)  177 (38%, 92% skin-coloured = SKIN)  255 (30%)
#   skin Bangs A   255 (100%, its hair -- same texture set as Body A, so on A 255 = HAIR)
#   skin Body B/C  0 and 78 cloth, a little 128 skin, NO 255 -- the outfit
#
# So slot A: the only Body slot with the skin's skin ramp at any size, and its hair ramp is at the same
# 255 as Citlali's hair. Her hair needs nothing. Her body's SKIN at 255 would land on the skin's HAIR
# ramp, so SkinBand moves it 255 -> 177 -- on her BODY only and only where the diffuse is
# skin-coloured, because 255 on her HEAD is her hair (the Bennett lesson: a band number means a
# different material on each object).
#
# Vertex colour: the skin carries G = B = 128 on every vertex; Citlali 128 on 84% of G and 51 on 16%.
# The G/B -> 128 normalisation is kept (it is what the skin's own model carries) -- suspect it first
# if outlines look wrong.
#

import argparse
import os
import sys
from typing import Dict, List, Optional, Set

import numpy as np

Repo = os.environ.get("AG_REMAP_REPO") or r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss"
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
if (not os.path.isdir(APISrc)):
    raise SystemExit(f"the API's package folder is not at {APISrc}; set AG_REMAP_REPO to the repo's path")
sys.path.insert(0, APISrc)
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))

import FixRaidenBoss2 as FRB     # noqa: E402

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from remapPrototypeTools import activeInis, pickVariant     # noqa: E402
from PIL import Image            # noqa: E402

for needed in ("VGComponentSplit", "VGSplitGroupResource", "BufReplace"):
    if (not hasattr(FRB, needed)):
        raise SystemExit(f"this API build has no {needed}: it needs the core built after 2026-09-12 (the component split)")


# ============================================================================== the pair

V = "6.7"
NNFix = "CommandList\\global\\ORFix\\NNFix"
ORFix = "CommandList\\global\\ORFix\\ORFix"

SrcName = "Citlali"
SkinName = "CitlaliWhisperofStars"

# Citlali's hash.json has not changed since it was added at 5.3, and a 6.7 frame dump binds every one
# of these unchanged -- one version, so no history to match.
Citlali = {"draw_vb": "eec92f64", "position_vb": "362dc30c", "blend_vb": "3e939d2e", "texcoord_vb": "3421fed9",
           "ib": "f81f893c", "tex_face_diffuse": "9fb78572",
           "objects": {0: "head", 27393: "body"}}

Skin = {"Body": {"draw_vb": "2da4e7cf", "position_vb": "43a45ed1", "blend_vb": "209b5952", "texcoord_vb": "0e50caed", "ib": "f117984b",
                 "slots": {"A": 0, "B": 60888, "C": 111096, "D": 122916}},
        "Bangs": {"draw_vb": "59d2fb7c", "position_vb": "902a9ef2", "blend_vb": "5cebe332", "texcoord_vb": "502e7a13", "ib": "d44b2c85",
                  "slots": {"A": 0}},
        "Eyes": {"draw_vb": "4e827f53", "position_vb": "b95d494c", "blend_vb": "89585621", "texcoord_vb": "b8b31d33", "ib": "f4cca9ef",
                 "slots": {"A": 0}}}
SkinFaceDiffuse = "5783625d"

# Each target component's Texcoord stride, off the skin's dump. Citlali's is 20 (a second UV set,
# all zero), so her lines pass through for the Body and are narrowed for the Bangs and the Eyes.
TexcoordStride = {"Body": 20, "Bangs": 12, "Eyes": 12}

# Every ps-t register the TARGET's own slot binds, read off CitlaliWhisperofStarsIdentity. A register
# beyond this list is left to the GAME by the skin's own mod, so a remapped section drops it.
SlotRegisters = {"Body": ("ps-t0", "ps-t1", "ps-t2"),
                 "Bangs": ("ps-t0", "ps-t1", "ps-t2"),
                 "Eyes": ("ps-t0", "ps-t1", "ps-t2")}

# Per target component: the split strategy, the draw slot the mod is drawn through, whether that slot
# reads a normal map (all three do), its fix call, and whether it carries the face section
Plan = {"Body": {"strategy": "graphcut", "slot": "A", "normalMap": True, "fix": ORFix, "face": True},
        "Bangs": {"strategy": "graphcut", "slot": "A", "normalMap": True, "fix": ORFix, "face": False},
        "Eyes": {"strategy": "graphcut", "slot": "A", "normalMap": True, "fix": ORFix, "face": False}}

# (from, to, condition, object): see the header
SkinBand = (255, 177, "skin", "body")
BandMoves = [SkinBand]

SkipTextures = False       # --noTextures:  no band move; the mod's own light map is bound untouched


def skinColoured(rgb) -> "np.ndarray":
    """Where a diffuse (H x W x 3, uint8) is skin-coloured: warm and bright, red over green over blue"""
    r, g, b = (rgb[..., i].astype(np.int16) for i in range(3))
    return (r >= 96) & (r >= g) & (g >= b) & ((r - b) >= 16) & ((r - b) <= 140)


Conditions = {"skin": skinColoured, "": lambda rgb: np.ones(rgb.shape[:2], dtype = bool)}


def movesFor(objName: str):
    """The band moves that apply to one object -- empty means its light map is not touched at all"""
    return [b for b in BandMoves if (b[3] is None or b[3] == objName)]


# The flat normal map for a PLAIN-layout mod (see difference 1): 127 / 127 / 255 sampled through the
# reference's BC7_UNORM_SRGB header, written untagged -- round(255 * (127 / 255) ** 2.2) = 55
FlatNormal = (55, 55, 255, 255)

targetName = lambda component: SkinName + component
keepName = lambda name: name


# ============================================================================== the tables

def registerCitlali(components: List[str]) -> FRB.ModType:
    """Citlali and one pseudo target per component, as a runtime ModType on the shipped GI builders"""
    FRB.CppGlobalModTypes.registerAll()
    shared = FRB.CppGlobalModTypes.all()[0].vgRemaps
    GI, CID = int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Citlali)

    hashRows = [([V, SrcName, key], Citlali[key]) for key in ("draw_vb", "position_vb", "blend_vb", "texcoord_vb", "ib", "tex_face_diffuse")]
    # No index rows for the pseudo targets: nothing looks them up (RegNewVals writes the index), and a
    # reverse lookup of "0" that could land on a target row blinds the classifier and the part filter
    indexRows = [([V, SrcName, "", obj], str(index)) for index, obj in Citlali["objects"].items()]
    vg = shared.clone()
    for component in components:
        target = targetName(component)
        hashRows += [([V, target, key], Skin[component][key]) for key in ("draw_vb", "position_vb", "blend_vb", "texcoord_vb", "ib")]
        hashRows.append(([V, target, "tex_face_diffuse"], SkinFaceDiffuse))
        row = shared.get([SrcName, "", SkinName, component], ["1.0", "6.7"], errorOnNotFound = False)
        if (row is None):
            raise SystemExit(f"the vertex group table has no {SrcName} -> {SkinName} {component} row")
        vg.addRows([(["1.0", SrcName, "", V, target, ""], dict(row.remap))])

    targets = [targetName(c) for c in components]
    hashes = FRB.Hashes({SrcName: targets}); hashes.addRepoRows(hashRows)
    indices = FRB.Indices({SrcName: targets}); indices.addRepoRows(indexRows)
    modType = FRB.ModType(GI, CID, SrcName, [], hashes, indices, None, vg)
    template = next(m for m in FRB.CppGlobalModTypes.all() if m.name == "Keqing")   # any shipped GI type: its builders are the table builders, which consult the overrides
    modType.iniParseBuilder = template.iniParseBuilder
    modType.iniFixBuilder = template.iniFixBuilder
    modType.iniRemoveBuilder = template.iniRemoveBuilder
    FRB.ModTypeIdTools.registerModType(modType)
    FRB.CppGlobalModTypes.registerMissing()
    return modType


def componentSpecs(components: List[str]) -> List[FRB.VGComponentSpec]:
    """Every component of the target, from the API's shared table (forward row = the component's bones)"""
    shared = FRB.CppGlobalModTypes.all()[0].vgRemaps
    specs = []
    for component in components:
        forward = dict(shared.get([SrcName, "", SkinName, component], ["1.0", "6.7"], errorOnNotFound = False).remap)
        secondary = {}
        if (Plan[component]["strategy"] == "negative"):
            reverse = shared.get([SkinName, component, SrcName, ""], ["1.0", "6.7"], errorOnNotFound = False)
            if (reverse is not None):
                for bone, source in reverse.remap.items():
                    if (source not in forward and source not in secondary):
                        secondary[source] = bone
        specs.append(FRB.VGComponentSpec(component, forward, secondary = secondary, negativeIndex = Plan[component]["strategy"] == "negative"))
    return specs


# ============================================================================== the parser

def makeParser(modType: FRB.ModType):
    """A GIMIParser sorted by the API's hash / index classifier, as makeGIMICharParser builds one"""
    drawnObjs = [("", name) for name in Citlali["objects"].values()]
    hashOnly = {"ib": ("", "ib"), "blend_vb": ("", "blend"), "position_vb": ("", "position"), "texcoord_vb": ("", "texcoord"),
                "draw_vb": ("", "other"), "tex_face_diffuse": ("", "face")}
    modObjs = drawnObjs + [obj for obj in hashOnly.values() if (obj not in drawnObjs)]

    def factory(iniFile, modTypeId):
        classifier = FRB.GIMISectionClassifier(dict(hashOnly), modType.hashes, {"ib": {obj: obj for obj in drawnObjs}}, modType.indices, None)
        classifier.hashNonVersionVals = {"name": SrcName}
        classifier.indexNonVersionVals = {"name": SrcName}
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
    would draw of it -- computed once per .ini, shared by the component fixers
    """

    def __init__(self, ini, components: List[str]):
        self.folder = os.path.dirname(os.path.abspath(ini.file))
        self.position, self.blend, self.texcoord, self.objects, self.face, self.faceRegs = self._readFiles(ini)
        rel = lambda p: os.path.relpath(p, self.folder) if p else "-"
        print(f"  files: position {rel(self.position)}, blend {rel(self.blend)}, texcoord {rel(self.texcoord)}, "
              f"face diffuse {rel(self.face)} on {'/'.join(sorted(self.faceRegs)) or '-'}")
        for name, o in self.objects.items():
            print(f"    {name} ({o['layout']} layout): ib {rel(o['ib'])}; normal map {rel(o['NormalMap'])}; diffuse {rel(o['Diffuse'])}; lightmap {rel(o['LightMap'])}")
        for label, f in (("position", self.position), ("blend", self.blend), ("texcoord", self.texcoord)):
            if (not f):
                raise ValueError(f"the .ini names no {label} buffer for Citlali")
        if (not self.objects):
            raise ValueError("the .ini has no object sections on Citlali's IB hash")

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

        graphCache: Dict[str, object] = {}

        def viaRun(sectionName: str, key: str) -> Optional[str]:
            """A key's value on a section, following `run =` into command lists (a merged mod's shape)"""
            direct = first(templates[sectionName], key)
            if (direct is not None):
                return direct
            if (sectionName not in graphCache):
                try:
                    graphCache[sectionName] = FRB.IniSectionGraph(dict(templates), [sectionName])
                except Exception as error:
                    print(f"  ! could not build the call graph for [{sectionName}]: {error}")
                    graphCache[sectionName] = None
            graph = graphCache[sectionName]
            if (graph is None):
                return None
            found = []
            for name, section in graph.sections.items():
                if (name == sectionName):
                    continue
                value = first(section, key)
                if (value and value not in found):
                    found.append(value)
            if (not found):
                return None
            if (len(found) > 1):
                print(f"  [{sectionName}] {key}: {len(found)} variants behind `run =`, using {found[0]} (also {', '.join(found[1:])})")
            return found[0]

        def fileOf(resource: Optional[str]) -> Optional[str]:
            if (not resource or resource.lower() == "null" or resource not in templates):
                return None
            f = first(templates[resource], "filename")
            return os.path.normpath(os.path.join(self.folder, f.replace("\\", os.sep))) if f else None

        position = blend = texcoord = face = None
        faceRegs: Set[str] = set()
        objects: Dict[str, Dict[str, Optional[str]]] = {}
        for sectionName, template in templates.items():
            h = (first(template, "hash") or "").lower()
            if (h == Citlali["position_vb"]):
                position = position or fileOf(viaRun(sectionName, "vb0"))
            elif (h == Citlali["blend_vb"]):
                blend = blend or fileOf(viaRun(sectionName, "vb1"))
            elif (h == Citlali["texcoord_vb"]):
                texcoord = texcoord or fileOf(viaRun(sectionName, "vb1"))
            elif (h == Citlali["tex_face_diffuse"]):
                # which register the MOD binds its face diffuse on decides the face swap (difference 3)
                for reg in ("ps-t0", "ps-t1"):
                    f = fileOf(viaRun(sectionName, reg))
                    if (f):
                        faceRegs.add(reg)
                        face = face or f
            elif (h == Citlali["ib"]):
                index = first(template, "match_first_index")
                if (index is None):
                    continue
                name = Citlali["objects"].get(int(index))
                if (name is None):
                    print(f"  ! {template.name}: match_first_index {index} is not one of Citlali's objects, skipped")
                    continue
                t0, t1, t2 = (fileOf(viaRun(sectionName, r)) for r in ("ps-t0", "ps-t1", "ps-t2"))
                if (t2):     # the normal-map layout: normal map, diffuse, light map
                    objects[name] = {"ib": fileOf(viaRun(sectionName, "ib")), "layout": "normal", "NormalMap": t0, "Diffuse": t1, "LightMap": t2}
                else:        # the plain layout: diffuse, light map
                    objects[name] = {"ib": fileOf(viaRun(sectionName, "ib")), "layout": "plain", "NormalMap": None, "Diffuse": t0, "LightMap": t1}
        objects = {name: objects[name] for _, name in sorted(Citlali["objects"].items()) if (name in objects)}
        return position, blend, texcoord, objects, face, faceRegs

    def drawn(self, component: str) -> List[str]:
        """The mod objects with at least one triangle in this component, in draw order"""
        kept = self.results[component].stats.trianglesKept
        return [name for name, n in zip(self.ibNames, kept) if (n > 0)]

    def keptVertices(self, component: str) -> int:
        return self.results[component].stats.keptVertices

    def texcoordLineEdit(self, targetStride: int):
        """
        The per-vertex data the TARGET component's shader reads. G / B -> 128 is the skin's own
        vertex colour (see the header); TEXCOORD1 is zeroed for a 20-byte target. The stride itself is
        retargeted after the service run (retargetTexcoords), because the buffer writer sizes its
        output from the SOURCE stride
        """
        def edit(line: bytes) -> bytes:
            out = bytearray(line)
            out[1] = 128
            out[2] = 128
            if (targetStride == 20 and len(out) >= 20):
                out[12:20] = bytes(8)
            return bytes(out)
        return edit


# ============================================================================== the textures

def liftBands(diffusePath: Optional[str], objName: str):
    """Moves each of BandMoves on this object where the object's DIFFUSE agrees the pixel is that material"""
    def lift(texFile) -> None:
        pixels = np.array(texFile.img)
        alpha = pixels[..., 3]
        original = alpha.copy()          # every decision from the ORIGINAL alpha: a legend is a permutation
        moved = 0
        for fromVal, toVal, condition, onlyObj in movesFor(objName):
            mask = (original == fromVal)
            if (diffusePath and os.path.isfile(diffusePath)):
                diffuse = FRB.TextureFile(diffusePath, readPillowImg = True)
                diffuse.open()
                if (diffuse.hasImage):
                    img = diffuse.img.convert("RGB")
                    if (img.size != texFile.img.size):
                        img = img.resize(texFile.img.size, Image.BILINEAR)
                    mask &= Conditions[condition](np.asarray(img))
            else:
                print(f"      ! {objName}: no diffuse to condition the band move on -- nothing moved")
                mask[:] = False
            alpha[mask] = toVal
            moved += int(mask.sum())
        print(f"      band move on {objName}: {moved} px")
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


def texReplace(resModObj, kind: str, filterFunc, compress: bool = True) -> FRB.TexReplace:
    """A texture edit through the API's TexEditor, written once per source texture"""
    def fix(resource) -> bool:
        if (resource.fixedPath in _written):
            return True
        _written.add(resource.fixedPath)
        # compress = False for a light map: its ALPHA is a band selector, and BC7's lossy alpha moves
        # values off their band. mipmaps: a texture written without its chain speckles at distance.
        editor = FRB.TexEditor([filterFunc], readPillowImg = True, compress = compress, mipmaps = True)
        editor.fix(FRB.TextureFile(resource.srcPath, readPillowImg = True), resource.fixedPath)
        return True
    placeholder = FRB.TexEditor([], compress = compress)
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
        remap = {}
        for name in Citlali["objects"].values():
            remap[(0, "", name)] = [(0, "", slot)] if (name in drawn) else []
        for kind in ("ib", "blend", "position", "texcoord", "other", "face"):
            wanted = drawn and (kind != "face" or plan["face"])
            remap[(0, "", kind)] = [(0, "", kind, keepName) for _ in range(groups)] if wanted else []
        edits: List[object] = [FRB.GraphGroupRemap(remap = remap)]

        # ---- 2. the textures ----
        shifted = set()                    # groups whose registers were shifted (a plain-layout object)
        for g, name in enumerate(drawn):
            obj = files.objects[name]
            lightReg = "ps-t2" if (obj["layout"] == "normal") else "ps-t1"
            if (movesFor(name) and not SkipTextures):
                edits.append(FRB.ResRegCollect({(g, "", slot): lightReg},
                                               {"lightMap": texReplace((g, "", slot + "RemapTexLightMap"), "LightMap",
                                                                       liftBands(obj["Diffuse"], name), compress = False)}))
            else:
                print(f"  {name}: {'--noTextures, ' if SkipTextures else 'no band move, '}light map left untouched")
            if (plan["normalMap"] and obj["layout"] == "plain"):
                # Bennett's structural edit, only for a mod drawn by the plain shader: shift the
                # registers into the three-register layout and supply a flat normal map
                print(f"  {name}: plain layout -- shifting registers and creating a flat normal map")
                edits.append(FRB.GraphGroupEdit([{slotObj: [FRB.RegRemap({"ps-t0": ["ps-t1", "ps-tNormal"], "ps-t1": ["ps-t2"]})]} if (i == g) else {} for i in range(groups)]))
                creator = FRB.TexCreator(1024, 1024, FRB.CppColour(*FlatNormal), compress = True, mipmaps = True)
                edits.append(FRB.ResRegCollect({(g, "", slot): "ps-tNormal"}, {"normalMap": FRB.TexCreate((g, "", slot + "RemapNormal"), "NormalMap", creator)}))
                shifted.add(g)

        # ---- 3. the buffers, as ONE resource group split together by VGSplitGroupResource ----
        for g, name in enumerate(drawn):
            kinds = {"blend": ((g, "", "blend"), "vb1"), "texcoord": ((g, "", "texcoord"), "vb1"), "ib": ((g, "", slot), "ib")}
            if (cut):
                kinds["position"] = ((g, "", "position"), "vb0")
            srcRegs, resEdits = {}, {}
            for kind, (srcGraph, reg) in kinds.items():
                resObj = (g, "", f"{slot}Remap{kind.capitalize()}")
                srcRegs[resObj] = {srcGraph: reg}
                resEdits[resObj] = {component: FRB.BufReplace(resObj, kind, resSubType = name if (kind == "ib") else None)}
            builder = FRB.IniGroupedResBuilder(FRB.VGSplitGroupResource, args = [f"{SrcName}{toModName}Buffers"],
                                               kwargs = {"component": component, "specs": files.specs, "ibPaths": files.ibPaths,
                                                         "texcoordLineEdit": files.texcoordLineEdit(TexcoordStride[component])})
            edits.append(FRB.ResGroupCollect([component], srcRegs, resEdits, {component: builder}, id = g))

        # ---- 4. the index, windowed to the copied object's own KVPs (head and body share the ib hash) ----
        objFilter = FRB.GIMIObjPartFilter(modType.hashes, modType.indices, {"ib"}, None)
        _alive.append(objFilter)
        indexEdits, indexFilters, indexKeys, indexTrack = [], [], [], []
        for g, name in enumerate(drawn):
            indexEdits.append({slotObj: [FRB.RegNewVals({"match_first_index": str(Skin[component]["slots"][slot])})]})
            indexFilters.append({slotObj: [objFilter.filter(("", name))]})
            indexKeys.append({slotObj: objFilter.keysToTrack()})
            indexTrack.append({slotObj: True})
        if (drawn):
            edits.append(FRB.GraphGroupEdit(indexEdits, trackKeys = indexTrack, keysToTrack = indexKeys, keyFilters = indexFilters))

        # ---- 5. everything else, per group ----
        rename = FRB.GraphRename(lambda n: naming.getRemapFixName(n, toModName))
        hashRemap = FRB.RegAssetRemap({"hash": (modType.hashes, "HashNotFound")}, toModName, SrcName, ini.fromVersion, ini.toVersion)
        dropFixCalls = FRB.RegRemove({"run": lambda _ind, val: val in (NNFix, ORFix)})
        fillDraw = FRB.RegFillMissing("drawindexed", "auto", fillMode = FRB.RegFillMissingMode.BottomCover)
        removeDraw = FRB.RegRemove({"drawindexed": None})
        addFix = FRB.RegDelimitedAdd([("run", plan["fix"])], {"drawindexed": []}, pathEndOnlyWhenUndelimited = True,
                                    mode = FRB.RegDelimitedAddMode.PerPath)   # ONE call per path: ORFix / NNFix re-slot the ps-t registers, so two undo each other
        normalBack = FRB.RegRemap({"ps-tNormal": ["ps-t0"]})
        # difference 3: swap the face registers only for a mod binding its face diffuse at ps-t0
        swapFace = "ps-t0" in files.faceRegs
        if (plan["face"] and files.face):
            print(f"  face: diffuse bound on {'/'.join(sorted(files.faceRegs))} -- " + ("swapping ps-t0 <-> ps-t1" if swapFace else "already ps-t1, no swap"))
        perGroup = []
        for g in range(groups):
            # the draw count is measured off the split index buffer, never inherited from the mod
            setDraw = []
            if (g < len(drawn)):
                i = files.ibNames.index(drawn[g])
                count = len(files.results[component].ibs[i]) * 3
                setDraw = [FRB.RegNewVals({"drawindexed": f"{count}, 0, 0"})]
            group = {slotObj: [dropFixCalls] + ([normalBack] if (g in shifted) else []) + [fillDraw] + setDraw + [addFix, hashRemap],
                     ("", "ib"): [FRB.GraphRename(lambda n: naming.getRemapIbName(n, toModName)), hashRemap, removeDraw],
                     ("", "blend"): [FRB.GraphRename(lambda n: naming.getRemapBlendName(n, toModName)), hashRemap]
                                    + ([FRB.RegNewVals({"draw": f"{files.keptVertices(component)},0"})] if cut else []),
                     ("", "position"): [rename, hashRemap],
                     ("", "texcoord"): [rename, hashRemap],
                     ("", "other"): [rename, hashRemap,
                                     FRB.RegNewVals({"override_byte_stride": str(files.positionStride), "override_vertex_count": str(files.keptVertices(component))},
                                                    addNewKVPs = True)]}
            if (plan["face"]):
                group[("", "face")] = [rename, hashRemap] + ([FRB.RegRemap({"ps-t0": ["ps-t1"], "ps-t1": ["ps-t0"]})] if swapFace else [])
            perGroup.append(group)
        if (drawn):
            edits.append(FRB.GraphGroupEdit(perGroup))

        _alive.extend(edits)
        fixer = FRB.GIMIFixer(parser, graphGroupEdits = edits, modsToFix = [toModName])
        _alive.append(fixer)
        return fixer
    return factory


# ============================================================================== after the service

def componentOf(name: str) -> Optional[str]:
    return next((c for c in Skin if f"{SkinName}{c}" in name), None)


def retargetTexcoords(folder: str) -> None:
    """Bring every written Texcoord buffer to its TARGET component's stride, in either direction (see bennettAdventureFix.py)"""
    import re

    for iniPath in activeInis(folder):
        with open(iniPath, "rb") as f:
            iniRaw = f.read()
        crlf = b"\r\n" in iniRaw
        iniText = iniRaw.decode("utf-8", "replace").replace("\r\n", "\n")
        changed = False
        for block in re.finditer(r"\[Resource[^\]]*\]\n(?:[^\[]*\n)?", iniText):
            body = block.group(0)
            nameMatch = re.search(r"filename\s*=\s*(\S+Texcoord\S*\.buf)", body)
            strideMatch = re.search(r"stride\s*=\s*(\d+)", body)
            if (not nameMatch or not strideMatch):
                continue
            fileName = nameMatch.group(1)
            component = componentOf(fileName)
            if (component is None):
                continue
            want, have = TexcoordStride[component], int(strideMatch.group(1))
            if (have == want):
                continue
            bufPath = os.path.join(os.path.dirname(iniPath), fileName)
            if (not os.path.isfile(bufPath)):
                print(f"  ! {fileName} is named by the .ini but not on disk")
                continue
            data = np.fromfile(bufPath, dtype = np.uint8)
            if (len(data) % have):
                print(f"  ! {fileName} is not a whole number of {have}-byte lines, not retargeted")
                continue
            lines = data.reshape(-1, have)
            if (want > have):
                out = np.zeros((len(lines), want), dtype = np.uint8)
                out[:, :have] = lines
            else:
                dropped = lines[:, want:]
                if (dropped.any()):
                    print(f"  ! {fileName}: narrowing {have} -> {want} discards non-zero TEXCOORD1 on {int((dropped != 0).any(axis = 1).sum())} of {len(lines)} vertices")
                out = lines[:, :want].copy()
            out.tofile(bufPath)
            iniText = iniText.replace(body, body.replace(f"stride = {have}", f"stride = {want}"), 1)
            changed = True
            print(f"  {'widened' if (want > have) else 'narrowed'} {fileName}: stride {have} -> {want} over {len(lines)} vertices ({component})")
        if (changed):
            out = iniText.replace("\n", "\r\n") if crlf else iniText
            with open(iniPath, "wb") as f:
                f.write(out.encode("utf-8"))


def trimSlotRegisters(folder: str) -> None:
    """Drop every ps-t binding a remapped section carries that the target's slot does not use, and any register bound twice"""
    import re

    for iniPath in activeInis(folder):
        with open(iniPath, "rb") as f:
            iniRaw = f.read()
        crlf = b"\r\n" in iniRaw
        lines = iniRaw.decode("utf-8", "replace").replace("\r\n", "\n").split("\n")
        out, section, allowed, seen, dropped = [], None, None, set(), []
        for line in lines:
            stripped = line.strip()
            if (stripped.startswith("[") and stripped.endswith("]")):
                section = stripped[1:-1]
                component = componentOf(section)
                allowed = SlotRegisters[component] if (component is not None and "Remap" in section) else None
                seen = set()
                out.append(line)
                continue
            reg = re.match(r"\s*(ps-t\d+)\s*=", line)
            if (allowed is not None and reg):
                name = reg.group(1)
                if (name not in allowed):
                    dropped.append(f"{section}: {stripped}  (the slot does not bind {name})")
                    continue
                if (name in seen):
                    dropped.append(f"{section}: {stripped}  ({name} already bound above)")
                    continue
                seen.add(name)
            out.append(line)
        if (not dropped):
            continue
        fixed = "\n".join(out)
        with open(iniPath, "wb") as f:
            f.write((fixed.replace("\n", "\r\n") if crlf else fixed).encode("utf-8"))
        print(f"  trimmed {len(dropped)} register binding(s) in {os.path.relpath(iniPath, folder)}:")
        for d in dropped:
            print(f"      {d}")


def drawnComponents(folder: str) -> Set[str]:
    """Which target components the written .ini files actually DRAW (read off the output, across every .ini)"""
    import re

    drawn = set()
    for iniPath in activeInis(folder):
        with open(iniPath, "rb") as f:
            iniText = f.read().decode("utf-8", "replace").replace("\r\n", "\n")
        marks = [(m.group(1), m.start()) for m in re.finditer(r"^\[([^\]]+)\]", iniText, re.M)]
        for i, (name, start) in enumerate(marks):
            end = marks[i + 1][1] if (i + 1 < len(marks)) else len(iniText)
            component = componentOf(name)
            if (component is None or "RemapFix" not in name):
                continue
            if (re.search(r"^\s*drawindexed\s*=", iniText[start:end], re.M)):
                drawn.add(component)
    return drawn


def hideUndrawnComponents(folder: str, components: List[str]) -> None:
    """Suppress the skin's own draw of every component the output does NOT draw (it would sit on top of the mod)"""
    import re

    drawn = drawnComponents(folder)
    if (not drawn):
        print("  ! nothing was drawn for ANY component -- not hiding anything, the fix did not land")
        return
    for component in components:
        if (component not in drawn):
            print(f"  ! {component} was asked for but nothing landed on it -- the skin's own {component} is hidden instead")
    missing = [c for c in Skin if c not in drawn]
    if (not missing):
        return
    for iniPath in activeInis(folder):
        with open(iniPath, "rb") as f:
            iniRaw = f.read()
        iniText = iniRaw.decode("utf-8", "replace").replace("\r\n", "\n")
        if ("RemapFix]" not in iniText and "RemapFix\n" not in iniText):
            continue
        if (re.search(r"RemapFix\d+\.ini$", os.path.basename(iniPath))):
            continue
        add = []
        for component in missing:
            name = f"TextureOverride{SkinName}{component}IBHide"
            if (name in iniText):
                continue
            add.append(f"\n[{name}]\nhash = {Skin[component]['ib']}\nhandling = skip\n")
            print(f"  hiding the skin's own {component} draw (ib {Skin[component]['ib']})")
        if (not add):
            continue
        iniText += "\n; The skin's own draws for components nothing was remapped onto.\n" + "".join(add)
        out = iniText.replace("\n", "\r\n") if (b"\r\n" in iniRaw) else iniText
        with open(iniPath, "wb") as f:
            f.write(out.encode("utf-8"))


# ============================================================================== run

def main():
    parser = argparse.ArgumentParser(description = "Citlali -> CitlaliWhisperofStars, through the API's parser, fixer and resource groups")
    parser.add_argument("mod", help = "the mod folder (every Citlali .ini under it is fixed)")
    parser.add_argument("--components", default = "Body,Bangs,Eyes", help = "target components to produce (default: %(default)s)")
    parser.add_argument("--variant", default = None, help = "a merged mod: disable its master .ini and enable this variant instead")
    parser.add_argument("--noTextures", action = "store_true", help = "DIAGNOSTIC: no band move; the mod's own light map is bound untouched")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    args = parser.parse_args()
    global SkipTextures
    SkipTextures = args.noTextures
    components = [c.strip() for c in args.components.split(",") if c.strip()]
    unknown = [c for c in components if (c not in Plan)]
    if (unknown):
        raise SystemExit(f"unknown component(s) {unknown}; choose from {list(Plan)}")

    modType = registerCitlali(components)
    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser(SrcName, makeParser(modType))
    for component in components:
        FRB.CppStrategyOverrides.setFixer(SrcName, targetName(component), makeFixer(component, components))

    folder = os.path.abspath(args.mod)
    try:
        service = FRB.RemapService(path = folder, keepBackups = args.keepBackups, forcedModTypeIds = {int(FRB.ModTypeId.Citlali)},
                                   logger = FRB.Logger() if args.verbose else None)
        pickVariant(folder, args.variant)
        service.fix()
        retargetTexcoords(folder)
        trimSlotRegisters(folder)
        hideUndrawnComponents(folder, components)
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
    finally:
        FRB.CppStrategyOverrides.clear()


if (__name__ == "__main__"):
    main()

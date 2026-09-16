#
# ===== bennettAdventureFix (prototype v1) =====
#
# Bennett -> BennettAdventure, driven by the API's own parser, fixer and resource groups. A copy of
# yelanTranquilFix.py with this pair's tables; the machinery below "the tables" is that script's,
# unchanged, and anything learnt here should go back into both.
#
#   py -3 bennettAdventureFix.py <mod folder>                         fix every Bennett .ini under the folder
#   py -3 bennettAdventureFix.py <mod folder> --components Body,Bang  only some target components
#   py -3 bennettAdventureFix.py <mod folder> --keepBackups           keep the .ini backups the API makes
#   py -3 bennettAdventureFix.py <mod folder> --verbose               attach the API's logger
#   py -3 bennettAdventureFix.py <mod folder> --wsl                   from Windows: relaunch under WSL
#
# ---- What the frame analyses settled, and what is still a guess ----
#
# Read off FrameAnalysis-BennettAdventure-2026-09-14-044616 and FrameAnalysis-Bennett-2026-09-14-044301,
# so these are measurements rather than the Yelan analogy they started as:
#
#   * the skin draws FOUR slots -- Body A (first index 0) and B (44334), Bang A (0), Eye A (0);
#   * Body A, Body B and the Bang are all on the NORMAL-MAP shader 2c157719180b096c (bound LND:
#     lightmap ps-t0, normal map ps-t1, diffuse ps-t2), so all three want the ORFix three-register
#     layout. The Eye is on 95aa6cdb84eb7b99 with no normal map -- the plain NNFix layout;
#   * the Bang and the Eye have NO textures of their own: both read Body slot A's set (draws 49 and
#     48 bind 04cd73c6 / c3e39ad5 / bfa7fe04, which are A's lightmap / normal map / diffuse);
#   * BOTH characters bind the face diffuse at ps-t1, with the face LIGHT MAP d4841e1a at ps-t0
#     (main face draws 42/43/46/47/52/53 on the skin, 37/38/39 on Bennett) -- the GI 6.x face
#     register swap, and the reason both identity mods want --faceRegister ps-t1.
#
# **BENNETT HAS NO PLAIN BODY SLOT TO LAND ON, which Yelan did.** Tranquil's slot C was the
# no-normal-map variant matching Yelan's own shader family, and picking it is what stopped a hidden
# throat strip rendering as bright static. BennettAdventure has no such slot: A and B are both on the
# normal-map shader, so Bennett's model MUST be drawn through one, and the flat normal map this
# script creates is structural rather than cosmetic.
#
# ---- Which slot, and why no band lift: the measurement (2026-09-14) ----
#
# A GIMI lightmap's ALPHA is a material band selecting a shading ramp, and the bands here are
# discrete: 0, 78, 126-128, 176-178, 255. Histogrammed per band with the mean DIFFUSE under it:
#
#   Bennett body    0 (77%, dark cloth)  126-128 (white trim)  255 (12.5%, mean RGB 232/204/181,
#                                                              99.5% skin-coloured) = SKIN
#   Adventure A     0 (35%)  78  126-128  177-178 (30%, pale)  255 (7.3%, mean RGB 233/203/180,
#                                                              100% skin-coloured) = SKIN
#   Adventure B     0 (48%, orange 188/135/79)  78 (26%, blue 35/76/98)  126-128  176-178
#                   -- NO 255 BAND AT ALL, and nothing skin-coloured in any band
#
# So slot A is not the analogy to Yelan's, it is the measured answer twice over: it is the only Body
# slot with a skin band, and its skin band is at the SAME alpha as Bennett's with a diffuse mean
# matching to within 1/255 per channel. Drawing him through B would shade his face and arms with the
# outfit's ramps, which is the mistake Yelan's slot-B run made visible.
#
# The two SKIN bands already coincide at 255, so his body needed nothing -- and v1 concluded from
# that alone that nothing needed lifting at all. **That was wrong, and the hair showed it in game
# (2026-09-15): patches of different white where the base model is uniform.**
#
# The mistake was measuring whole textures. A whole-texture histogram is dominated by unused UV space
# and by whichever material covers most of the sheet, so it answered for his BODY and said nothing
# about his HEAD. Measured over the pixels each material actually uses:
#
#   Bennett head   alpha 0   = his white HAIR    (98.8% of the head's pale/neutral pixels)
#                  alpha 255 = his brass GOGGLES (98.3% of its warm pixels, diffuse 147/144/91)
#   Adventure A    176-178 (32%, diffuse 213-226 pale) = HER HAIR;  0 = a mid-dark material
#
# So his hair, all of it on band 0, was being shaded by her band-0 ramp, and his goggles on 255 by
# her SKIN ramp. HairBand moves 0 -> 177.
#
# **The move is conditional on the DIFFUSE, and has to be**: the same band numbers mean different
# materials on his own two objects -- 0 is hair on his head and dark cloth on his body, 255 is
# goggles on his head and skin on his body. An unconditional move would put his body's cloth on her
# hair ramp, which is the shape of the bug that put static on Yelan's neck.
#
# And the move is keyed by OBJECT as well: "pale and neutral" finds his hair on the head texture
# but also catches 62908 pixels of pale CLOTH on the body one, which has no business on her hair
# ramp. Measured, not guessed -- the first run with the condition alone reported exactly that.
#
# The goggles are left on 255 deliberately: none of her bands is a brass/metal one, so there is
# nowhere measured to send them, and a guess is worse than the current wrongness. They stay
# skin-shaded pending an in-game look.
#
# ---- The Bang draws NOTHING, and that is the answer rather than a gap (2026-09-14) ----
#
# Yelan's Bang was a negative-index component: it drew the whole mod trimmed to its own bones, with
# those bones read off the REVERSE row (VGComponentSpec.secondary), because her bang bones had no
# forward row of their own. The split honours a secondary bone only on a vertex that ALSO carries a
# forward bone -- without that guard the whole face is drawn twice.
#
# Bennett cannot use that, and the reason is in his blend buffer: he has no hair bone. Group 0 is his
# head, and it carries 3225 vertices -- his hair AND his face. Groups 1 and 2 are his two eye bones
# (101 vertices each, weight exactly 1.0), and those are the whole of the rest of his head. All nine
# of the skin's Bang bones map back to exactly those three groups.
#
# So there is no partition that gives the Bang his hair without also giving it his face, his forward
# Bang row is empty, and an empty forward set means the guard above can never fire: the Bang would
# draw nothing whatever we did. The right answer is therefore to NOT FIX the Bang component at all --
# left unfixed its ib section keeps its `drawindexed = auto` and BennettAdventure keeps her own
# bangs, which is what you want when the source has no hair of its own to put there. Bennett's hair
# still arrives, drawn by the Body component along with the rest of his head.
#
# ---- Known wrong on the identity mod only: the face ----
#
# The face edit is a two-way ps-t0 <-> ps-t1 swap, which is what every shipped GI character does
# and is right for any mod authored before GI 6.x. The identity mod is built from a 6.x dump and
# already binds ps-t1, so the swap moves it the wrong way. The maintainer's call (2026-09-14) is
# to keep the identity mod at ps-t1 and add a heuristic guard to the swap later -- so until then,
# the identity mod's remapped face is knowingly wrong and everything else about it is not.
#
# **The texture recipe is the flat normal map plus ONE band move** (see above). Yelan's alpha-1 head
# diffuse and her vertex-colour edit are still not applied: each was derived from her own pair in
# game and neither is known to hold here. Add them one at a time, each with a measurement behind it --
# that is how the hair band was found, and how v1's "nothing to lift" was found to be wrong.
#

import argparse
import os
import re
import shutil
import sys
from typing import Dict, List, Optional, Set

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

# GIMI file conventions both prototypes need; see that module's docstring
from remapPrototypeTools import activeInis, pickVariant     # noqa: E402
from PIL import Image            # noqa: E402

for needed in ("VGComponentSplit", "VGSplitGroupResource", "BufReplace"):
    if (not hasattr(FRB, needed)):
        raise SystemExit(f"this API build has no {needed}: it needs the core built after 2026-09-12 (the component split)")


# ============================================================================== the pair

V = "6.1"
NNFix = "CommandList\\global\\ORFix\\NNFix"
ORFix = "CommandList\\global\\ORFix\\ORFix"

# Every version of Bennett's hashes the library knows, straight out of data/HashData.cpp, which
# follows his hash.json's history in the assets repo. A mod carries whichever version its author
# dumped, so all of them have to be matchable -- a 4.0-era mod says position 993d1661 where today's
# model says 6cff51b4, and knowing only the latter means not finding its position buffer at all.
#
# The FIRST value of each list is the current one, which is what gets written out.
BennettHistory = {
    "draw_vb":     [("4.1", "02cf3aa5"), ("4.0", "8b2a1582")],
    "position_vb": [("4.4", "6cff51b4"), ("4.0", "993d1661")],
    "ib":          [("4.3", "cdc66323"), ("4.0", "f51209fc")],
    "blend_vb":    [("4.0", "d4acf3f7")],          # never moved
    "texcoord_vb": [("4.0", "acde80a4")],          # never moved
    "tex_face_diffuse": [("4.0", "50f7dc9a")],     # never moved
}


def knownHashes(key: str) -> set:
    """every value Bennett has ever had for one hash type"""
    return {h for _, h in BennettHistory[key]}


# Bennett's hashes are the CURRENT model's: his hash.json moved three times inside the library's
# window (draw_vb at 4.1, ib at 4.3, position_vb at 4.4), and these are the post-4.4 values, which
# are what a mod built on today's model carries. ibOld is his pre-4.3 ib, for a mod that predates it.
Bennett = {"draw_vb": "02cf3aa5", "position_vb": "6cff51b4", "blend_vb": "d4acf3f7", "texcoord_vb": "acde80a4",
           "ib": "cdc66323", "ibOld": "f51209fc", "tex_face_diffuse": "50f7dc9a",
           "objects": {0: "head", 9879: "body"}}

Adventure = {"Body": {"draw_vb": "bc87167b", "position_vb": "14efbc45", "blend_vb": "09b92379", "texcoord_vb": "51dd19aa", "ib": "022a9ccd",
                      "slots": {"A": 0, "B": 44334}},
             "Bang": {"draw_vb": "2f953b46", "position_vb": "a8a0adb9", "blend_vb": "d2bb6147", "texcoord_vb": "5a79eaa8", "ib": "43ad99d1",
                      "slots": {"A": 0}},
             "Eye": {"draw_vb": "feb0e532", "position_vb": "f5dd3d9e", "blend_vb": "89827a3f", "texcoord_vb": "941adcbf", "ib": "91b4d5dd",
                     "slots": {"A": 0}}}
# Each target component's Texcoord stride, measured off her own dump: her Body carries a second
# UV set (TEXCOORD1 at offset 12) and her Bang and Eye do not. Bennett's is 12 throughout, so his
# lines are padded for the Body and pass through for the others.
TexcoordStride = {"Body": 20, "Bang": 12, "Eye": 12}

# Every ps-t register the TARGET's own slot binds, read off BennettAdventureIdentity -- her model as
# a mod, and the one arrangement of hers confirmed correct in game. A register BEYOND this list is
# not a harmless extra: the slot means something different to her shader than it does to Bennett's,
# and her own mod deliberately leaves it to the GAME (see CreatingRemaps, "A TextureOverride binds
# registers only for the draw its hash matches").
SlotRegisters = {"Body": ("ps-t0", "ps-t1", "ps-t2"),
                 "Bang": ("ps-t0", "ps-t1", "ps-t2"),
                 "Eye": ("ps-t0", "ps-t1")}

# Diagnostic switches, set from main(). Both make the output deliberately incomplete -- they
# exist to bisect a symptom, not to ship.
SkipTextures = False       # --noTextures:  no band move; the mod's own light map is bound
SkipNormalMap = False      # --noNormalMap: no created normal map and no register shift either

AdventureFaceDiffuse = "2b1b2edf"

# Per target component: which strategy splits the mod's triangles for it, which of the target's draw
# slots the mod is drawn through, and the slot's register layout -- every field measured off the
# frame analysis (see the header), the Body's choice of slot A included -- it is the only Body slot
# with a skin band, and its skin band sits at the same alpha as Bennett's.
#
# The Bang is negative-index for the reason Tranquil's was, only more so: the vertex-group table has
# NO forward Bennett -> Bang row at all (all nine of its groups correspond to his three head bones,
# so the correspondence exists only in reverse). Its bones come entirely from the reverse row via
# VGComponentSpec.secondary, and without that it would draw nothing.
Plan = {"Body": {"strategy": "graphcut", "slot": "A", "normalMap": True, "fix": ORFix, "face": True},
        "Bang": {"strategy": "negative", "slot": "A", "normalMap": True, "fix": ORFix, "face": False},
        "Eye": {"strategy": "graphcut", "slot": "A", "normalMap": False, "fix": NNFix, "face": False}}

# NO BAND LIFT YET. On Yelan this is where her legend was mapped onto Tranquil's, band by band, and
# the lift is applied to every object's lightmap. Bennett's and BennettAdventure's legends have not
# been measured against each other in game, so nothing is moved: a wrong lift is not a no-op, it
# puts alpha 0 on a band that meant something (Yelan's neck static). Measure first, then fill in.
# ---- the light map band moves (2026-09-15, measured; see the header) ----
#
# (from, to, condition): the alpha band to move, where to, and which diffuse the pixel must have for
# the move to apply. The condition is what keeps this off his BODY, where the same band numbers mean
# different materials.
HairBand = (0, 177, "pale", "head")   # his white hair -> her pale/hair ramp (176-178), HEAD only
# his brass goggles sit on 255, which is her SKIN ramp. Left alone deliberately: none of her bands is
# a brass/metal one, so there is nowhere measured to send them. Revisit with an in-game look.
GogglesBand = None


def skinColoured(rgb) -> "np.ndarray":
    """Where a diffuse (H x W x 3, uint8) is skin-coloured: warm and bright, red over green over blue"""
    r, g, b = (rgb[..., i].astype(np.int16) for i in range(3))
    return (r >= 96) & (r >= g) & (g >= b) & ((r - b) >= 16) & ((r - b) <= 140)
def paleNeutral(rgb) -> "np.ndarray":
    """Where a diffuse is his silver HAIR: bright and close to neutral, which his dark cloth is not"""
    channels = rgb.astype(np.int16)
    return (channels.max(axis = -1) >= 150) & ((channels.max(axis = -1) - channels.min(axis = -1)) <= 40)


def movesFor(objName: str):
    """The band moves that apply to one object -- empty means its light map is not touched at all"""
    return [b for b in (HairBand, GogglesBand) if (b is not None and (b[3] is None or b[3] == objName))]


Conditions = {"pale": paleNeutral, "skin": skinColoured, "": lambda rgb: np.ones(rgb.shape[:2], dtype = bool)}


# The flat normal map the reference draws with is 127 / 127 / 255 under a BC7_UNORM_SRGB header; a created
# texture is written untagged, so it carries what that samples as: round(255 * (127 / 255) ** 2.2) = 55
FlatNormal = (55, 55, 255, 255)

targetName = lambda component: "BennettAdventure" + component
keepName = lambda name: name


# ============================================================================== the tables

def registerBennett(components: List[str]) -> FRB.ModType:
    """Bennett and one pseudo target per component, as a runtime ModType on the shipped GI builders"""
    FRB.CppGlobalModTypes.registerAll()
    shared = FRB.CppGlobalModTypes.all()[0].vgRemaps
    GI, YID = int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Bennett)

    # every version of every type, so a mod written against an older model still resolves. The
    # rows are filed at the version each value belongs to, which is what HashData.cpp does.
    hashRows = [([version, "Bennett", key], value)
                for key in ("draw_vb", "position_vb", "blend_vb", "texcoord_vb", "ib", "tex_face_diffuse")
                for version, value in BennettHistory[key]]
    # No index rows for the pseudo targets: nothing looks them up (RegNewVals writes the index from
    # the table above), and a reverse lookup of "0" that could land on a target row blinds the
    # classifier and GIMIObjPartFilter's window
    indexRows = [([V, "Bennett", "", obj], str(index)) for index, obj in Bennett["objects"].items()]
    vg = shared.clone()
    for component in components:
        target = targetName(component)
        hashRows += [([V, target, key], Adventure[component][key]) for key in ("draw_vb", "position_vb", "blend_vb", "texcoord_vb", "ib")]
        hashRows.append(([V, target, "tex_face_diffuse"], AdventureFaceDiffuse))
        row = shared.get(["Bennett", "", "BennettAdventure", component], ["1.0", "5.7"], errorOnNotFound = False)
        if (row is None):
            raise SystemExit(
                f"the vertex group table has no Bennett -> BennettAdventure {component} row, so nothing of\n"
                f"his model belongs to that component. For the Bang that is the MEASURED answer rather than a\n"
                f"gap (see the header): drop it from --components and the skin keeps its own bangs.")
        vg.addRows([(["1.0", "Bennett", "", V, target, ""], dict(row.remap))])

    targets = [targetName(c) for c in components]
    hashes = FRB.Hashes({"Bennett": targets}); hashes.addRepoRows(hashRows)
    indices = FRB.Indices({"Bennett": targets}); indices.addRepoRows(indexRows)
    modType = FRB.ModType(GI, YID, "Bennett", [], hashes, indices, None, vg)
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
    (every one of the Bang's nine bones is one of Bennett's three head bones, and the forward table
    has no Bennett -> Bang row at all, so WITHOUT this the Bang draws nothing)
    """
    shared = FRB.CppGlobalModTypes.all()[0].vgRemaps
    specs = []
    for component in components:
        forward = dict(shared.get(["Bennett", "", "BennettAdventure", component], ["1.0", "5.7"], errorOnNotFound = False).remap)
        secondary = {}
        if (Plan[component]["strategy"] == "negative"):
            reverse = shared.get(["BennettAdventure", component, "Bennett", ""], ["1.0", "5.7"], errorOnNotFound = False)
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
    drawnObjs = [("", name) for name in Bennett["objects"].values()]
    hashOnly = {"ib": ("", "ib"), "blend_vb": ("", "blend"), "position_vb": ("", "position"), "texcoord_vb": ("", "texcoord"),
                "draw_vb": ("", "other"), "tex_face_diffuse": ("", "face")}
    modObjs = drawnObjs + [obj for obj in hashOnly.values() if (obj not in drawnObjs)]

    def factory(iniFile, modTypeId):
        classifier = FRB.GIMISectionClassifier(dict(hashOnly), modType.hashes, {"ib": {obj: obj for obj in drawnObjs}}, modType.indices, None)
        # Reverse lookups filtered to Bennett's own rows: the pseudo targets share hash types with him
        classifier.hashNonVersionVals = {"name": "Bennett"}
        classifier.indexNonVersionVals = {"name": "Bennett"}
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
                raise ValueError(f"the .ini names no {label} buffer for Bennett")
        if (not self.objects):
            raise ValueError("the .ini has no object sections on Bennett's IB hash")

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

        graphCache: Dict[str, object] = {}

        def viaRun(sectionName: str, key: str) -> Optional[str]:
            """
            A key's value on a section, following `run =` into command lists when it is not there
            directly -- the shape the GIMI mod merger writes.

            Uses IniSectionGraph, which builds the call graph from the root and walks every `run =`
            transitively (cycles included). A $swapvar branch offers one value per variant; the first
            is taken and the rest reported, so a merged mod is remapped from its first variant.
            """
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
                value = first(section, key)          # `first` knows the parts live under the template
                if (value and value not in found):
                    found.append(value)
            if (not found):
                return None
            if (len(found) > 1):
                print(f"  [{sectionName}] {key}: {len(found)} variants behind `run =`, using {found[0]}"
                      f" (also {', '.join(found[1:])})")
            else:
                print(f"  [{sectionName}] {key} resolved through `run =` -> {found[0]}")
            return found[0]

        def fileOf(resource: Optional[str]) -> Optional[str]:
            if (not resource or resource.lower() == "null" or resource not in templates):
                return None
            f = first(templates[resource], "filename")
            return os.path.normpath(os.path.join(self.folder, f.replace("\\", os.sep))) if f else None

        position = blend = texcoord = face = None
        objects: Dict[str, Dict[str, Optional[str]]] = {}
        for sectionName, template in templates.items():
            h = (first(template, "hash") or "").lower()
            if (h in knownHashes("position_vb")):
                position = position or fileOf(viaRun(sectionName, "vb0"))
            elif (h in knownHashes("blend_vb")):
                blend = blend or fileOf(viaRun(sectionName, "vb1"))
            elif (h in knownHashes("texcoord_vb")):
                texcoord = texcoord or fileOf(viaRun(sectionName, "vb1"))
            elif (h in knownHashes("tex_face_diffuse")):
                # BOTH registers: a mod authored pre-6.x binds its face diffuse at ps-t0, one built
                # from a 6.x dump (an identity mod, say) at ps-t1. Reading only ps-t0 silently
                # reported "face diffuse -" on the latter.
                face = face or fileOf(viaRun(sectionName, "ps-t0")) or fileOf(viaRun(sectionName, "ps-t1"))
            elif (h in (Bennett["ib"], Bennett["ibOld"])):
                index = first(template, "match_first_index")
                if (index is None):
                    continue
                name = Bennett["objects"].get(int(index))
                if (name is None):
                    print(f"  ! {template.name}: match_first_index {index} is not one of Bennett's objects, skipped")
                    continue
                objects[name] = {"ib": fileOf(viaRun(sectionName, "ib")), "Diffuse": fileOf(viaRun(sectionName, "ps-t0")), "LightMap": fileOf(viaRun(sectionName, "ps-t1"))}
        objects = {name: objects[name] for _, name in sorted(Bennett["objects"].items()) if (name in objects)}
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

    def texcoordLineEdit(self, targetStride: int):
        """
        The per-vertex data the TARGET component's shader reads and the source's does not.

        THE STRIDE IS THE IMPORTANT PART, AND IT IS A PROPERTY OF THE MOD RATHER THAN OF THE
        CHARACTER. VANILLA Bennett's Texcoord is 12 bytes a vertex (COLOR 4 + TEXCOORD 8);
        BennettAdventure's Body is 20, because she carries a second UV set at offset 12 and vanilla
        he does not. Handing her Body a 12-byte buffer makes every read at offset 12 fall into the
        NEXT vertex's COLOR.

        But a real MOD may already be 20 -- its author carries a second UV set -- and her Bang and
        Eye are 12, so that same mod has to be NARROWED onto them instead. Measure the source
        stride; never infer it from the character. retargetTexcoords() does the conversion, in
        whichever direction the pair needs.

        The vertex colour normalisation is kept from the Yelan pair, where that mod carried 188.
        Measured on Bennett: he already has G = 128 in 98% of vertices and B = 128 in 99%, so it is
        very nearly a no-op here rather than a correction.
        """
        def edit(line: bytes) -> bytes:
            out = bytearray(line)
            out[1] = 128
            out[2] = 128
            # NOT widened here. A longer returned line is not honoured by the buffer writer -- it
            # sizes its output from the SOURCE stride -- and returning one made it write nothing at
            # all, silently: the identity mod went from three texcoord buffers to one. Widening is
            # done by retargetTexcoords() after the service run instead.
            if (targetStride == 20 and len(out) >= 20):
                out[12:20] = bytes(8)
            return bytes(out)
        return edit


# ============================================================================== the textures

def alphaOne(texFile) -> None:
    """UNUSED IN v1. On the Yelan pair the target's shader darkens by diffuse alpha and the source's
    ignores it, so her head diffuse was forced to alpha 1. Not measured on Bennett."""
    texFile.img.putalpha(1)


def liftBands(diffusePath: Optional[str], objName: str):
    """
    Moves HairBand where the object's DIFFUSE agrees the pixel is that material.

    Conditional because the band numbers mean different things on his own two objects -- 0 is hair on
    his head and dark cloth on his body -- so an unconditional move would reshade the body.
    """
    def lift(texFile) -> None:
        pixels = np.array(texFile.img)
        alpha = pixels[..., 3]
        moved = 0
        for band in (HairBand, GogglesBand):
            if (band is None):
                continue
            fromVal, toVal, condition, onlyObj = band
            if (onlyObj is not None and objName != onlyObj):
                continue                      # the legend is per OBJECT: band 0 is hair on his head
                                              # and dark cloth on his body
            mask = (alpha == fromVal)
            if (diffusePath and os.path.isfile(diffusePath)):
                diffuse = FRB.TextureFile(diffusePath, readPillowImg = True)
                diffuse.open()
                if (diffuse.hasImage):
                    img = diffuse.img.convert("RGB")
                    if (img.size != texFile.img.size):
                        img = img.resize(texFile.img.size, Image.BILINEAR)
                    mask &= Conditions[condition](np.asarray(img))
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


def texReplace(files: ModFiles, resModObj, kind: str, filterFunc, compress: bool = True) -> FRB.TexReplace:
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
        # compress = False for a light map: its ALPHA is a material band selector, and BC7's lossy
        # alpha moves values off their band and so onto a different shading ramp.
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
        # what the section will actually be written as, after the diagnostic switches
        normalMap = plan["normalMap"] and not SkipNormalMap
        cut = plan["strategy"] == "graphcut"
        groups = max(len(drawn), 1)
        print(f"  {toModName}: draws {', '.join(drawn) or 'nothing'} through slot {slot}" + (f" ({groups} .ini groups)" if (groups > 1) else ""))

        # ---- 1. copy each drawn object's graph onto the component's draw slot ----
        #
        # Every drawn object goes to the SAME slot; the second claimant lands in group 1, which is a
        # second .ini file (the API's merge shape). Every other graph is copied unrenamed into every
        # group, exactly as the shipped template does, so each file is complete on its own.
        remap = {}
        for name in Bennett["objects"].values():
            remap[(0, "", name)] = [(0, "", slot)] if (name in drawn) else []
        for kind in ("ib", "blend", "position", "texcoord", "other", "face"):
            wanted = drawn and (kind != "face" or plan["face"])
            remap[(0, "", kind)] = [(0, "", kind, keepName) for _ in range(groups)] if wanted else []
        edits: List[object] = [FRB.GraphGroupRemap(remap = remap)]

        for g, name in enumerate(drawn):
            # ---- 2. the textures: edit, then the flat normal map where the slot reads one ----
            if (normalMap):
                # the light map, band-moved (see liftBands). Collected at ps-t1, which is where the
                # source's PLAIN layout holds it -- before the shift below moves it to ps-t2.
                if (movesFor(name) and not SkipTextures):
                    edits.append(FRB.ResRegCollect({(g, "", slot): "ps-t1"},
                                                   {"lightMap": texReplace(files, (g, "", slot + "RemapTexLightMap"),
                                                                           "LightMap", liftBands(files.objects[name]["Diffuse"], name),
                                                                           compress = False)}))
                elif (SkipTextures):
                    print(f"  {name}: --noTextures, light map left untouched")
                else:
                    print(f"  {name}: no band move, light map left untouched")
                # The diffuse is NOT edited. Yelan's recipe edits the head diffuse to
                # alpha 1 and lifts every lightmap band to band here; both were derived from HER
                # pair in game and neither is known to hold for Bennett. The mod's own textures pass
                # through untouched until a measurement says otherwise -- see the file header.
                #
                # What remains is structural, not cosmetic: Bennett is drawn by the plain two-register
                # shader (ps-t0 diffuse, ps-t1 lightmap) and every Body / Bang slot of the skin is on
                # the three-register normal-map one, so the registers have to be SHIFTED and a normal
                # map supplied. ps-t0 is duplicated onto a scratch register that the created normal
                # map then replaces; the main edit renames it back to ps-t0 once the shift is done
                edits.append(FRB.GraphGroupEdit([{slotObj: [FRB.RegRemap({"ps-t0": ["ps-t1", "ps-tNormal"], "ps-t1": ["ps-t2"]})]} if (i == g) else {} for i in range(groups)]))
                creator = FRB.TexCreator(1024, 1024, FRB.CppColour(*FlatNormal), compress = True, mipmaps = True)   # flat: any size serves every object
                edits.append(FRB.ResRegCollect({(g, "", slot): "ps-tNormal"}, {"normalMap": FRB.TexCreate((g, "", slot + "RemapNormal"), "NormalMap", creator)}))
            elif (plan["normalMap"]):
                print(f"  {slot}: --noNormalMap, keeping the mod's plain layout and NNFix")

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
            builder = FRB.IniGroupedResBuilder(FRB.VGSplitGroupResource, args = [f"Bennett{toModName}Buffers"],
                                               kwargs = {"component": component, "specs": files.specs, "ibPaths": files.ibPaths,
                                                         "texcoordLineEdit": files.texcoordLineEdit(TexcoordStride[component])})
            edits.append(FRB.ResGroupCollect([component], srcRegs, resEdits, {component: builder}, id = g))

        # ---- 4. the index, windowed to the copied object's own KVPs (head and body share the ib hash) ----
        objFilter = FRB.GIMIObjPartFilter(modType.hashes, modType.indices, {"ib"}, None)
        _alive.append(objFilter)          # filter() hands out callables that point back at it; let it outlive this factory
        indexEdits, indexFilters, indexKeys, indexTrack = [], [], [], []
        for g, name in enumerate(drawn):
            indexEdits.append({slotObj: [FRB.RegNewVals({"match_first_index": str(Adventure[component]["slots"][slot])})]})
            indexFilters.append({slotObj: [objFilter.filter(("", name))]})
            indexKeys.append({slotObj: objFilter.keysToTrack()})
            indexTrack.append({slotObj: True})
        if (drawn):
            edits.append(FRB.GraphGroupEdit(indexEdits, trackKeys = indexTrack, keysToTrack = indexKeys, keyFilters = indexFilters))

        # ---- 5. everything else, per group ----
        rename = FRB.GraphRename(lambda n: naming.getRemapFixName(n, toModName))
        hashRemap = FRB.RegAssetRemap({"hash": (modType.hashes, "HashNotFound")}, toModName, "Bennett", ini.fromVersion, ini.toVersion)
        dropFixCalls = FRB.RegRemove({"run": lambda _ind, val: val in (NNFix, ORFix)})
        fillDraw = FRB.RegFillMissing("drawindexed", "auto", fillMode = FRB.RegFillMissingMode.BottomCover)   # the draw call moves onto each object, at its END...
        removeDraw = FRB.RegRemove({"drawindexed": None})                      # ...and off the shared ib section, which now only skips
        addFix = FRB.RegDelimitedAdd([("run", plan["fix"] if normalMap else NNFix)], {"drawindexed": []}, pathEndOnlyWhenUndelimited = True,
                                    mode = FRB.RegDelimitedAddMode.PerPath)   # ONE call per path: NNFix/ORFix re-slot the ps-t registers, so two undo each other
        normalBack = FRB.RegRemap({"ps-tNormal": ["ps-t0"]})
        perGroup = []
        for g in range(groups):
            # THE DRAW COUNT IS MEASURED OFF THE SPLIT INDEX BUFFER, NEVER INHERITED FROM THE MOD.
            #
            # A mod that draws for itself carries its own `drawindexed = <count>, 0, 0`, and that
            # count is its WHOLE object -- but after the split this component's .ib holds only the
            # triangles the split kept for it. RegFillMissing never fires on such a section (the
            # register is not missing), so the mod's stale count survives and the draw reads past the
            # end of the buffer.
            #
            # Measured on a real Bennett mod (2026-09-15): its head section draws 9879 indices, the
            # Eye component's split .ib holds 828, and the remapped Eye draw still asked for 9879 --
            # 9051 indices of whatever followed the buffer, which rendered as mangled eyes. The head
            # drawn through the Body component had the same fault (9879 asked, 9051 present). The
            # identity mod never showed it because it draws through `auto` and so has no count to go
            # stale: a mod that draws for itself is the case that matters, and it is the common one.
            #
            # Each split .ib is self-contained and renumbered, so the offset is always 0.
            setDraw = []
            if (g < len(drawn)):
                i = files.ibNames.index(drawn[g])
                count = len(files.results[component].ibs[i]) * 3
                setDraw = [FRB.RegNewVals({"drawindexed": f"{count}, 0, 0"})]
            group = {slotObj: [dropFixCalls] + ([normalBack] if normalMap else []) + [fillDraw] + setDraw + [addFix, hashRemap],
                     ("", "ib"): [FRB.GraphRename(lambda n: naming.getRemapIbName(n, toModName)), hashRemap, removeDraw],
                     ("", "blend"): [FRB.GraphRename(lambda n: naming.getRemapBlendName(n, toModName)), hashRemap]
                                    + ([FRB.RegNewVals({"draw": f"{files.keptVertices(component)},0"})] if cut else []),
                     ("", "position"): [rename, hashRemap],
                     ("", "texcoord"): [rename, hashRemap],
                     ("", "other"): [rename, hashRemap,
                                     FRB.RegNewVals({"override_byte_stride": str(files.positionStride), "override_vertex_count": str(files.keptVertices(component))},
                                                    addNewKVPs = True)]}
            if (plan["face"]):
                # The two-way face swap every shipped GI character carries: GI 6.x swapped which
                # register the shader reads the face diffuse and the face light map out of, so a mod
                # binding its diffuse at ps-t0 has to be moved to ps-t1.
                #
                # It is a SWAP, so it also moves an already-correct ps-t1 back to ps-t0. That is right
                # for the mods this runs on -- essentially all of them predate 6.x -- and wrong for a
                # mod built from a 6.x dump, which is what the identity mod is. Measured on both
                # characters 2026-09-14: the game binds the face diffuse at ps-t1 and the face light
                # map (d4841e1a, shared by the two skins) at ps-t0.
                #
                # THE IDENTITY MOD STAYS AT ps-t1 (maintainer, 2026-09-14) and this swap stays
                # unguarded for now; a HEURISTIC GUARD is deferred work. So when testing the identity
                # mod in game, its remapped FACE is the one part that is knowingly wrong -- judge the
                # face on a real mod instead, and do not "correct" the swap on the strength of it.
                #
                # What the guard has to decide is which register the MOD binds its face diffuse on,
                # and normalise to ps-t1 rather than swapping blind:
                #   * the section's own registers are readable here -- ModFiles already looks for the
                #     face file on ps-t0 and then ps-t1 (see _readFiles), so the binding register is
                #     known at the point the edit is built;
                #   * a mod that binds ps-t0 needs the move, a mod already on ps-t1 needs nothing,
                #     and a mod binding BOTH is a real case (some carry a light map too) -- that one
                #     wants the swap, since its ps-t0 really is a diffuse in the old convention;
                #   * it belongs in the shipped GIMICharFixerConfig (swapFaceRegs) as well, not just
                #     here, because every one of the 44 compiled characters carries the same swap.
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

def retargetTexcoords(folder: str) -> None:
    """
    Bring every written Texcoord buffer to its TARGET component's stride, in either direction.

    Runs after the service, because the buffer writer sizes its output from the source stride (see
    this file's note on texcoordLineEdit).

    BOTH DIRECTIONS ARE REAL, AND WHICH ONE A MOD NEEDS CANNOT BE ASSUMED FROM THE CHARACTER.
    Vanilla Bennett's Texcoord is 12 bytes a vertex and BennettAdventure's Body is 20, so a
    vanilla-shaped mod is WIDENED onto her Body. But a mod whose author carries a second UV set is
    already 20, and her Bang and Eye are 12 -- that mod has to be NARROWED onto them.

    This guard used to be ``have >= want``, which skipped every narrowing silently. Measured on a
    real mod (2026-09-15): its Texcoord is stride 20, her Eye slot reads 12, and the eye draw was
    handed the 20-byte buffer -- so every vertex's UV after the first was read 8 bytes late and the
    eyes rendered as blank white. Nothing in the run said anything; the buffer was written, named
    and bound.

    Narrowing only ever drops TEXCOORD1, the second UV set. It is REPORTED when it is not zero
    rather than discarded quietly.
    """
    import re

    for iniPath in activeInis(folder):
        with open(iniPath, "rb") as f:
            iniRaw = f.read()
        crlf = b"\r\n" in iniRaw
        iniText = iniRaw.decode("utf-8", "replace").replace("\r\n", "\n")
        changed = False

        # [ResourceXxx] ... stride = N ... filename = Y.buf, for the remapped texcoord buffers only
        for block in re.finditer(r"\[Resource[^\]]*\]\n(?:[^\[]*\n)?", iniText):
            body = block.group(0)
            nameMatch = re.search(r"filename\s*=\s*(\S+Texcoord\S*\.buf)", body)
            strideMatch = re.search(r"stride\s*=\s*(\d+)", body)
            if (not nameMatch or not strideMatch):
                continue
            fileName = nameMatch.group(1)
            # the writer names a remapped buffer <mod><target><kind>..., so the component is in it
            component = next((c for c in TexcoordStride if f"BennettAdventure{c}" in fileName), None)
            if (component is None):
                continue
            want = TexcoordStride[component]
            have = int(strideMatch.group(1))
            if (have == want):
                continue

            bufPath = os.path.join(os.path.dirname(iniPath), fileName)
            if (not os.path.isfile(bufPath)):
                # A SAFETY NET, not a known bug. In normal runs every named buffer is written, and
                # both .ini groups of one component get byte-identical Texcoord content (it is ONE
                # split output), so a missing one can be filled from its sibling.
                #
                # It exists because a change to texcoordLineEdit once made the writer produce nothing
                # at all, silently -- the identity mod went from three texcoord buffers to one while
                # every log line still said success. If this fires, the cause is upstream and worth
                # finding rather than living with.
                sibling = next((os.path.join(os.path.dirname(bufPath), n)
                                for n in sorted(os.listdir(os.path.dirname(bufPath)))
                                if (n != fileName and f"BennettAdventure{component}" in n
                                    and "Texcoord" in n and n.endswith(".buf")
                                    and os.path.isfile(os.path.join(os.path.dirname(bufPath), n)))), None)
                if (sibling is None):
                    print(f"  ! {fileName} is named by the .ini but not on disk, and has no sibling to copy")
                    continue
                shutil.copyfile(sibling, bufPath)
                print(f"  ! {fileName} was named but never written (writer dedup) -- filled from {os.path.basename(sibling)}")

            data = np.fromfile(bufPath, dtype = np.uint8)
            if (len(data) % have):
                print(f"  ! {fileName} is not a whole number of {have}-byte lines, not retargeted")
                continue
            lines = data.reshape(-1, have)

            if (want > have):
                out = np.zeros((len(lines), want), dtype = np.uint8)
                out[:, :have] = lines                   # the new bytes are TEXCOORD1, zeroed
            else:
                # narrowing drops TEXCOORD1. Say so if it held anything -- a silent discard here is
                # how the opposite mistake stayed invisible for four in-game rounds.
                dropped = lines[:, want:]
                if (dropped.any()):
                    rows = int((dropped != 0).any(axis = 1).sum())
                    print(f"  ! {fileName}: narrowing {have} -> {want} discards non-zero TEXCOORD1 "
                          f"on {rows} of {len(lines)} vertices")
                out = lines[:, :want].copy()

            out.tofile(bufPath)

            iniText = iniText.replace(body, body.replace(f"stride = {have}", f"stride = {want}"), 1)
            changed = True
            verb = "widened" if (want > have) else "narrowed"
            print(f"  {verb} {fileName}: stride {have} -> {want} over {len(lines)} vertices ({component})")

        if (changed):
            out = iniText.replace("\n", "\r\n") if crlf else iniText
            with open(iniPath, "wb") as f:
                f.write(out.encode("utf-8"))


def trimSlotRegisters(folder: str) -> None:
    """
    Drop every ps-t binding a remapped section carries that the target's slot does not use.

    The remapped sections inherit their ps-t lines from the MOD's section, and Bennett's body binds
    four (diffuse, light map, metal map, shadow ramp). Her Eye slot binds two and her Body three, so
    the surplus lands in registers her shaders read as something else entirely -- and which her own
    mod leaves unbound on purpose, so the GAME's textures serve them.

    A register bound TWICE in one section is dropped to its first binding for the same reason: the
    later silently discards the earlier, and on the Body that meant Bennett's metal map overwriting
    the remapped light map -- the band move included -- in the slot her shader reads the light map
    from.

    Registers are matched by NAME, not by position, and a scratch name (ps-tNormal) is never touched:
    only a literal ps-t<number> is considered.
    """
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
                component = next((c for c in SlotRegisters if f"BennettAdventure{c}" in section), None)
                allowed = SlotRegisters[component] if (component is not None and "Remap" in section) else None
                seen = set()
                out.append(line)
                continue

            reg = re.match(r"\s*(ps-t\d+)\s*=", line)
            if (allowed is not None and reg):
                name = reg.group(1)
                if (name not in allowed):
                    dropped.append(f"{section}: {stripped}  (her slot does not bind {name})")
                    continue
                if (name in seen):
                    dropped.append(f"{section}: {stripped}  ({name} already bound above; the later one wins)")
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
    """
    Which target components the written .ini files actually DRAW.

    Read off the output rather than inferred from the request, because those differ: a component is
    drawn only if some `...BennettAdventure<C>...RemapFix` section carries a `drawindexed`. Collected
    across every .ini of the mod, never per file -- GIMI merges them all, so a hide written into one
    file would suppress a draw issued from another.
    """
    import re

    drawn = set()
    for iniPath in activeInis(folder):
        with open(iniPath, "rb") as f:
            iniText = f.read().decode("utf-8", "replace").replace("\r\n", "\n")

        marks = [(m.group(1), m.start()) for m in re.finditer(r"^\[([^\]]+)\]", iniText, re.M)]
        for i, (name, start) in enumerate(marks):
            end = marks[i + 1][1] if (i + 1 < len(marks)) else len(iniText)
            component = next((c for c in Adventure if f"BennettAdventure{c}" in name), None)
            if (component is None or "RemapFix" not in name):
                continue
            if (re.search(r"^\s*drawindexed\s*=", iniText[start:end], re.M)):
                drawn.add(component)
    return drawn


def hideUndrawnComponents(folder: str, components: List[str], keepBangs: bool) -> None:
    """
    Suppress the draw of every target component the fix did NOT remap onto.

    A component nothing was remapped onto still draws the SKIN's own geometry, on top of whatever the
    mod put there. For Bennett that is her Bang -- her front fringe over his hair, which is what made
    the hair look like two different whites.

    `handling = skip` with no drawindexed is what suppresses a draw; it is the same shape the fix
    leaves on a component it does remap, minus the re-issued draw.

    WHAT COUNTS AS "REMAPPED ONTO" IS WHAT THE OUTPUT DRAWS, NOT WHAT WAS ASKED FOR. This used to
    read the --components list, which is a request rather than a result. The split selects by VERTEX
    GROUP, and a mod is free not to use the bones a component's row names, so a component can be
    requested and still come out empty. Measured on a HuoHuo-over-Bennett mod (2026-09-15): it
    weights nothing to Bennett's groups 1 or 2, so the Eye row selected no vertices, the Eye cut was
    empty, and her own eyes drew on top of HuoHuo's -- a second pair inside the first. Her Bang was
    hidden correctly in the same run, purely because the Bang is excluded by the CLI, which is what
    made the omission look like a working feature.
    """
    import re

    drawn = drawnComponents(folder)
    if (not drawn):
        print("  ! nothing was drawn for ANY component -- not hiding anything, the fix did not land")
        return

    missing = [c for c in Adventure if c not in drawn]
    if (keepBangs):
        missing = [c for c in missing if c != "Bang"]

    for component in components:
        if (component not in drawn):
            print(f"  ! {component} was asked for but nothing landed on it -- this mod uses none of "
                  f"the vertex groups its row names, so the skin's own {component} is hidden instead")
    if (not missing):
        return

    for iniPath in activeInis(folder):
        with open(iniPath, "rb") as f:
            iniRaw = f.read()
        iniText = iniRaw.decode("utf-8", "replace").replace("\r\n", "\n")
        if ("RemapFix]" not in iniText and "RemapFix\n" not in iniText):
            continue                                   # not one of ours
        if (re.search(r"RemapFix\d+\.ini$", os.path.basename(iniPath))):
            continue                                   # the merge's extra files: one hide is enough

        add = []
        for component in missing:
            name = f"TextureOverrideBennettAdventure{component}IBHide"
            if (name in iniText):
                continue
            add.append(f"\n[{name}]\nhash = {Adventure[component]['ib']}\nhandling = skip\n")
            print(f"  hiding the skin's own {component} draw (ib {Adventure[component]['ib']})")
        if (not add):
            continue

        iniText += ("\n; The skin's own draws for components nothing was remapped onto. Left drawing,\n"
                    "; they sit on top of the mod -- her bangs over his hair, as two different whites.\n"
                    + "".join(add))
        out = iniText.replace("\n", "\r\n") if (b"\r\n" in iniRaw) else iniText
        with open(iniPath, "wb") as f:
            f.write(out.encode("utf-8"))


def normaliseIndexBuffers(folder: str, enabled: bool) -> None:
    """
    Rewrite every 16-bit index buffer a mod declares as a 32-bit one, before anything else runs.

    A GIMI mod's .ib is usually R32_UINT and both this script and the API's buffer writer assume it.
    A mod that declares `format = DXGI_FORMAT_R16_UINT` otherwise fails with numpy's "buffer size
    must be a multiple of element size" -- and only when the byte count happens not to divide by
    four, so a 16-bit buffer that does divide by four would be read as half as many WRONG indices
    and never complain.

    Normalising here rather than further in keeps one story: the .ini names the widened file, the
    split is given the same path, and the generated resource inherits R32_UINT. The mod's own .ib is
    left on disk and the .ini is backed up beside it.
    """
    import glob
    import re

    for iniPath in activeInis(folder):
        with open(iniPath, "rb") as f:
            iniRaw = f.read()
        iniText = iniRaw.decode("utf-8", "replace").replace("\r\n", "\n")

        if (not enabled):
            # REPORT ONLY. Rewriting the mod's .ini is destructive and is not something a diagnostic
            # run should do without being asked; the .ini is skipped further on for want of a
            # readable index buffer, which is the same outcome as before this existed.
            if ("R16_UINT" in iniText):
                print(f"  ! {os.path.basename(iniPath)} declares 16-bit index buffers, which the split cannot read."
                      "\n    Nothing was changed. Re-run with --normaliseIndices to widen them to 32-bit"
                      "\n    (that REWRITES this .ini and writes .r32.ib files beside the originals).")
            continue

        blocks = list(re.finditer(r"\[Resource[^\]]*\]\n(?:[^\[]*\n)?", iniText))
        changed = False

        for block in blocks:
            body = block.group(0)
            if ("R16_UINT" not in body):
                continue
            nameMatch = re.search(r"filename\s*=\s*(\S+)", body)
            if (not nameMatch):
                continue
            src = os.path.join(os.path.dirname(iniPath), nameMatch.group(1).replace("\\", os.sep))
            if (not os.path.isfile(src)):
                print(f"  ! {nameMatch.group(1)} is declared R16 but not on disk")
                continue

            wide = os.path.splitext(src)[0] + ".r32.ib"
            data = np.fromfile(src, dtype = "<u2").astype("<u4")
            data.tofile(wide)
            # keep the declared path's DIRECTORY -- a merged mod names buffers in variant
            # subfolders (".\BennettMirrorred\BennettHead.ib") and only the file name changes
            declared = nameMatch.group(1)
            cut = max(declared.rfind(chr(92)), declared.rfind("/"))
            newDeclared = declared[:cut + 1] + os.path.basename(wide)
            newBody = (body.replace(declared, newDeclared)
                           .replace("DXGI_FORMAT_R16_UINT", "DXGI_FORMAT_R32_UINT"))
            iniText = iniText.replace(body, newBody, 1)
            changed = True
            print(f"  {os.path.basename(src)}: R16 -> R32 ({len(data)} indices) as {os.path.basename(wide)}")

        if (changed):
            backup = iniPath + ".preR32.bak"
            if (not os.path.exists(backup)):
                with open(backup, "wb") as f:
                    f.write(iniRaw)
            out = iniText.replace("\n", "\r\n") if (b"\r\n" in iniRaw) else iniText
            with open(iniPath, "wb") as f:
                f.write(out.encode("utf-8"))


def main():
    parser = argparse.ArgumentParser(description = "Bennett -> BennettAdventure, through the API's parser, fixer and resource groups")
    parser.add_argument("mod", help = "the mod folder (every Bennett .ini under it is fixed)")
    parser.add_argument("--components", default = "Body,Eye", help = "target components to produce (default: %(default)s -- the Bang is left alone on purpose, see the header)")
    parser.add_argument("--variant", default = None, help = "a merged mod: disable its master .ini and enable this variant instead, refreshing that variant's stale hashes from the master (the prototype cannot fix a merged master)")
    parser.add_argument("--normaliseIndices", action = "store_true", help = "DESTRUCTIVE: rewrite the mod's .ini to widen 16-bit index buffers to 32-bit (backs each .ini up first)")
    parser.add_argument("--keepSkinBangs", action = "store_true", help = "leave BennettAdventure's own bangs drawing over the mod's hair (they overlap; this was the old behaviour)")
    parser.add_argument("--noTextures", action = "store_true", help = "DIAGNOSTIC: no band move; the mod's own light map is bound untouched")
    parser.add_argument("--noNormalMap", action = "store_true", help = "DIAGNOSTIC: also drop the created normal map and the register shift, keeping the mod's plain layout")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--loop", action = "store_true", help = "drive parse / fix / resources per .ini from this script instead of RemapService")
    parser.add_argument("--wsl", action = "store_true", help = "from Windows: run this same command under WSL, where the core is built (AG_REMAP_WSL_DISTRO / AG_REMAP_WSL_VENV)")
    args = parser.parse_args()
    if (args.wsl):
        raise SystemExit(relaunchUnderWsl(args))
    args.mod = winToPosix(args.mod)
    global SkipTextures, SkipNormalMap
    SkipTextures = args.noTextures or args.noNormalMap    # no normal map implies no band move either
    SkipNormalMap = args.noNormalMap
    if (SkipTextures or SkipNormalMap):
        print("  DIAGNOSTIC RUN: "
              + ("no texture edits" if SkipTextures else "")
              + (", no created normal map and no register shift" if SkipNormalMap else "")
              + " -- this output is deliberately incomplete")
    components = [c.strip() for c in args.components.split(",") if c.strip()]
    unknown = [c for c in components if (c not in Plan)]
    if (unknown):
        raise SystemExit(f"unknown component(s) {unknown}; choose from {list(Plan)}")

    modType = registerBennett(components)
    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser("Bennett", makeParser(modType))
    for component in components:
        FRB.CppStrategyOverrides.setFixer("Bennett", targetName(component), makeFixer(component, components))

    try:
        if (args.loop):
            fixFolder(os.path.abspath(args.mod), args)
        else:
            runService(os.path.abspath(args.mod), args, components)
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
    flags = [f"--components={args.components}"] + [f"--{name}" for name in ("keepBackups", "verbose", "loop", "noTextures", "noNormalMap", "keepSkinBangs", "normaliseIndices") if getattr(args, name)]
    flags += ([f"--variant={args.variant}"] if args.variant else [])
    command = f"source {venv}/bin/activate && AG_REMAP_REPO={shlex.quote(repo)} python {shlex.quote(script)} {shlex.quote(mod)} {' '.join(flags)}"
    print(f"wsl -d {distro}: {command}")
    return subprocess.call(["wsl", "-d", distro, "--", "bash", "-lc", command])


def runService(folder: str, args, components: List[str]) -> None:
    """The whole run through RemapService: folder walk, undo of a previous fix, backups, resources, summary"""
    service = FRB.RemapService(path = folder, keepBackups = args.keepBackups, forcedModTypeIds = {int(FRB.ModTypeId.Bennett)},
                               logger = FRB.Logger() if args.verbose else None)
    pickVariant(folder, args.variant)
    normaliseIndexBuffers(folder, args.normaliseIndices)
    service.fix()
    retargetTexcoords(folder)
    trimSlotRegisters(folder)
    hideUndrawnComponents(folder, components, args.keepSkinBangs)
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
    YID = int(FRB.ModTypeId.Bennett)
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

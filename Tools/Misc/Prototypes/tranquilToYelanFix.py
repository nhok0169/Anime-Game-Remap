#
# ===== tranquilToYelanFix (prototype v2) =====
#
# YelanTranquil -> Yelan: a multi-component SOURCE onto a classic single-component target, driven by
# the API's own parser, fixer and resources. The inverse of yelanTranquilFix.py.
#
#   py -3 tranquilToYelanFix.py <mod folder>                     fix every YelanTranquil .ini under the folder
#   py -3 tranquilToYelanFix.py <mod folder> --components Body   only some source components
#   py -3 tranquilToYelanFix.py <mod folder> --keepBackups       keep the .ini backups the API makes
#   py -3 tranquilToYelanFix.py <mod folder> --verbose           attach the API's logger
#   py -3 tranquilToYelanFix.py <mod folder> --noTextures        geometry only, light maps untouched
#   py -3 tranquilToYelanFix.py <mod folder> --compressTextures  BC7 the edited light maps (smaller, bands blur)
#
# ---- The buffers are MERGED, and v1's three-.ini shape was wrong (2026-09-13) ----
#
# v1 gave each source component its own .ini file, each binding Yelan's single set of buffer hashes
# to that component's own buffers, on the strength of the API's own merge shape -- YelanRemapFix1.ini
# says n sets of sections on one hash inside a single .ini "will trigger a warning on GIMI ... and
# only 1 of the mod objects will be displayed", so the fix uses "GIMI's overlapping mod bug/feature
# from loading multiple mods of the same character" instead.
#
# In game that produced a model stretched into spikes, and enabling any ONE of the three .ini files
# rendered that component correctly. The missing half of the trick is in the same comment: "since the
# mod used in all the .ini files are exactly the same, the user would not see the overlap". The
# overlap works for DUPLICATE content. Here each file bound DIFFERENT bytes to `hash = c58c76f9`, so
# one binding won globally and the other two components' index buffers addressed its vertices.
#
# So the buffers are concatenated instead, which is what a one-component target actually wants:
#
#   * Position / Blend / Texcoord become one buffer each, the components laid end to end in
#     ComponentOrder (Body first, so its index buffers need no offset at all).
#   * Each component's Blend is remapped through ITS OWN reverse vertex-group row before the
#     concatenation -- that is the whole reason the components cannot share one remap.
#   * The Texcoord stride is levelled up to the widest component's: Tranquil's Eye has no TEXCOORD1
#     (stride 12) where her Body and Bang do (20), and a concatenated buffer has one stride. The
#     short lines are zero-padded at the END, which is exactly where TEXCOORD1 sits.
#   * Every object's .ib is offset by its component's first vertex, and objects landing on the SAME
#     Yelan slot have their index buffers concatenated into one -- Tranquil's Bang and Eye both go
#     to Yelan's head, and after the register shift they bind identical textures, so one draw serves
#     both. That is what removes the last reason to write a second .ini file.
#
# One .ini file out, four object sections, one set of buffers. Nothing overlaps.
#
# The blend remap is done here in numpy rather than by RemapBlendReplace, because the components have
# to be remapped separately and then joined. It is checked byte for byte against the API's own
# RemapBlendReplace output (see merge.py): the one rule that is not obvious is that the API remaps a
# bone slot only where it carries WEIGHT -- a zero-weight slot keeps its literal index -- so
# [0,0,0,0] with weights [1,0,0,0] becomes [64,0,0,0] and not [64,64,64,64].
#
# ---- What lands where (measured, 2026-09-13) ----
#
# Every draw was isolated out of the two frame analyses (each draw's render target minus the one
# before it), so these are what the slots ARE, not what their names suggest:
#
#   Tranquil Body A  17877 tris  the body: skin, legs, arms, back hair, shoes   -> Yelan body   (20913)
#   Tranquil Body B   4581 tris  jewellery and ornaments                        -> Yelan extra  (54042)
#   Tranquil Body C   6780 tris  the outfit: bodice, sash, skirt panels, gloves -> Yelan dress  (51759)
#   Tranquil Bang     2564 tris  the front fringe                               -> Yelan head   (0)
#   Tranquil Eye       156 tris  the eyes                                       -> Yelan head   (0)
#
# Yelan's own four draws, for comparison: body = the whole skin/bodysuit, head = the bob and fringe,
# extra = the shoulder ornament spikes, dress = a one-sided hip drape.
#
# ---- The register layout ----
#
# All four of Yelan's draws are the no-normal-map shader family (vs 95aa6cdb84eb7b99 and
# d4c01363144d79d6, both falling to ORFix's LDX branch), so every remapped object is the two-register
# NNFix layout: ps-t0 diffuse, ps-t1 light map. Tranquil's slots A and B and her Bang read a normal
# map on ps-t0, so for those the normal map is dropped and the rest shifted down (ps-t1 -> ps-t0,
# ps-t2 -> ps-t1) -- the GanyuTwilight -> Ganyu shape. Her slot C and her Eye are already the plain
# layout and are left alone.
#
# The face diffuse needs no register change: both skins bind it at ps-t1 on the main pass (dump draws
# 39-41 for Yelan, 42-48 for Tranquil), only the hash moves.
#
# ---- The light map bands ----
#
# Measured rather than inherited: see the Bands table below for both skins' legends as read off their
# own identity mods, the three moves, why each is conditional on the diffuse, and why band 0 is the
# one asymmetry with the forward direction. The edited light maps are written UNCOMPRESSED by
# default -- the alpha being edited is a band selector and BC7's alpha is lossy enough to shift 12.8%
# of this particular edit off its target value (`--compressTextures` to compare).
#

import argparse
import os
import sys
from typing import Dict, List, Optional, Tuple

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


# ============================================================================== the pair

V = "6.1"
NNFix = "CommandList\\global\\ORFix\\NNFix"
ORFix = "CommandList\\global\\ORFix\\ORFix"

Source, Target = "YelanTranquil", "Yelan"

Kinds = ("position", "blend", "texcoord", "other", "ib")
HashKey = {"position": "position_vb", "blend": "blend_vb", "texcoord": "texcoord_vb", "other": "draw_vb", "ib": "ib"}
BlendStride = 32

Yelan = {"draw_vb": "d17ac213", "position_vb": "c58c76f9", "blend_vb": "f6e01e3c", "texcoord_vb": "428b836c",
         "ib": "82e14ea2", "tex_face_diffuse": "d3c0b54a",
         "objects": {"head": 0, "body": 20913, "dress": 51759, "extra": 54042}}
Tranquil = {"Body": {"draw_vb": "3a7b10bb", "position_vb": "02c325ef", "blend_vb": "244a4b2f", "texcoord_vb": "c772811d", "ib": "611d6168"},
            "Bang": {"draw_vb": "11b90d23", "position_vb": "c0dc5c2f", "blend_vb": "5d532cca", "texcoord_vb": "d5db917d", "ib": "648d61dd"},
            "Eye":  {"draw_vb": "61b441bd", "position_vb": "6bc61bb3", "blend_vb": "10056b37", "texcoord_vb": "e23e5a53", "ib": "54bc082e"}}
TranquilFaceDiffuse = "e8ad6095"

# The order the components are laid into the merged buffers. Body first on purpose: its objects then
# start at vertex 0 and their index buffers pass through untouched.
ComponentOrder = ("Body", "Bang", "Eye")

# Where an object with NO textures of its own gets them (2026-09-14).
#
# Tranquil's Bang and Eye have no textures in hash.json and the game draws them with the Body slot
# A set (dump draws 50-52 and 49 bind fe0fd573 / 183ca818 / f5cc10b7, which are Body A's). A mod may
# therefore leave its Bang and Eye sections with an `ib` and nothing else -- the recolor at
# Mods/YelanTranquil2 does -- and then the remapped head draw binds no texture at all.
#
# That is worse than it sounds, and not just "the head is untextured": ORFix / NNFix re-slot whatever
# is bound whether or not the section bound it, so a fix call over a section that brought nothing
# scrambles the registers the game had set. So an object with no textures borrows its donor's (added
# as ps-t0 diffuse / ps-t1 light map, which is already Yelan's layout, so no shift is needed), and if
# there is no donor either it gets NO `run =` line.
TextureDonor = {"Bang": ("Body", "A"), "Eye": ("Body", "A")}

# Each source draw slot: its match_first_index on Tranquil, and the Yelan object it lands on. Two
# slots may name the same Yelan object -- their index buffers are then concatenated into one draw.
Plan = {"Body": {"slots": {"A": {"index": 0,     "to": "body"},
                           "B": {"index": 53631, "to": "extra"},
                           "C": {"index": 67374, "to": "dress"}}},
        "Bang": {"slots": {"A": {"index": 0,     "to": "head"}}},
        "Eye":  {"slots": {"A": {"index": 0,     "to": "head"}}}}

# Corrections to the shipped reverse vertex-group rows, which are finder-made and half-checked.
#
# Body 9 -> 85 warped the knee on ONE side in game (2026-09-13). 8 and 9 are a mirror pair of knee
# helper bones (44 vertices each, X -0.052 / +0.052, Y 0.552) and BOTH were sent to Yelan 85, which
# sits at X -0.100 -- so the positive-side knee was driven by the negative-side thigh and its
# vertices were dragged across the body. Yelan's thigh pair is 85 / 108, and the bones around it
# already respect the sides: Tranquil 34 (X -0.091) -> 85 and Tranquil 58 (X +0.065) -> 108. So 9
# belongs on 108.
#
# The shape to look for, and the reason it is worth its own check: many-to-one is NORMAL here (the
# source has more bones than the target), but a mirror PAIR collapsing onto one OFF-MIDLINE bone
# never is -- whichever side is on the far side of that bone gets pulled across. It is the only
# such case in all three components (Tools/Misc/Diagnostics/mirrorRemapCheck.py).
#
# This is a prototype-side override: `data/VGRemapData.cpp`'s row needs the same correction when
# this is transcribed into C++.
VGRemapFixes: Dict[str, Dict[int, int]] = {"Body": {9: 108}}

_alive: List[object] = []          # objects the API holds by reference and Python would otherwise collect
_written: Dict[str, tuple] = {}    # output texture path -> what produced it, so a shared source is written once
keepName = lambda name: name


# ============================================================================== the lightmap bands
#
# A GIMI lightmap's ALPHA is a material band: each value selects a shading ramp, and the legend is
# per skin, so a band that means one thing on the source means something else on the target. Both
# legends below were re-measured off the two identity mods (alpha histogram of each lightmap with
# the mean diffuse colour under each band), not taken on trust:
#
#   YelanTranquil Body A   0 white fur (diffuse 187,204,221) | 64-89 silver (lightmap R 240)
#                          115-128 hair (55,65,105) | 165-189 silk and lace (82,75,87) | 255 SKIN (243,216,197)
#   YelanTranquil Body B/C 0 white trim | 64-89 silver | 115-128 dark navy CLOTH (45,64,88)
#                          165-189 teal silk (117,168,198) | no 255 band at all
#   Yelan Head/Extra       0 hair (57,65,95) | 115-128 skin (169,155,133) | 255 FUR (190,201,215), 65% of it
#   Yelan Body/Dress       0 hair and dark cloth (55,56,91) | 64-89 metal | 115-128 SKIN (244,224,206)
#                          165-189 ornaments | 255 fur (201,205,219)
#
# So three bands move and the rest already line up (silver/metal at 64-89, the dark 165-189). The
# three are applied SIMULTANEOUSLY -- every mask is taken from the original alpha before anything is
# written -- because they are a permutation: 0 -> 255 -> 121 -> ... would otherwise chase itself.
#
# Each move is conditioned on the DIFFUSE under the pixel, for the reason the forward direction
# found the hard way: a mod need not follow its character's legend at all, because a PORT keeps its
# SOURCE character's bands. An unconditional move puts a ported mod's hair on Yelan's skin ramp.
#
# One asymmetry from the forward direction worth knowing: there the fur move was unconditional,
# because Yelan's 255 is never Tranquil-legend skin. Here it cannot be, because the band in question
# is 0 -- the DEFAULT a lazy or ported mod leaves everything on -- and Yelan's own band 0 is her
# hair-and-all-cloth band, i.e. the safe no-op. So band 0 moves to her fur only where the diffuse
# actually looks like white fur; everything else stays put.
Bands: Tuple[Tuple[int, int, int, str], ...] = (
    (255, 255, 121, "skin"),        # Tranquil skin -> Yelan's skin band (115-127), mid value
    (0,     0, 255, "whiteFur"),    # Tranquil white fur -> Yelan's fur band
    (115, 128,   0, "notSkin"),     # Tranquil hair / dark cloth -> Yelan's hair-and-cloth band
)


def skinColoured(rgb: "np.ndarray") -> "np.ndarray":
    """Where a diffuse (H x W x 3) is skin: warm and bright, red over green over blue"""
    r, g, b = (rgb[..., i].astype(np.int16) for i in range(3))
    return (r >= 96) & (r >= g) & (g >= b) & ((r - b) >= 16) & ((r - b) <= 140)


def whiteFurColoured(rgb: "np.ndarray") -> "np.ndarray":
    """Where a diffuse is Tranquil's white fur: bright and close to neutral (hers reads faintly blue)"""
    channels = rgb.astype(np.int16)
    return (channels.max(axis = -1) >= 140) & ((channels.max(axis = -1) - channels.min(axis = -1)) <= 70)


Conditions = {"skin": skinColoured,
              "notSkin": lambda rgb: ~skinColoured(rgb),
              "whiteFur": whiteFurColoured,
              "": None}


# ============================================================================== the textures

# keyed by the SOURCE light map, not by the object: the API names a collected resource after the
# source texture, so every object sharing one light map shares one edited output and one edit. A
# per-object counter could only ever read zero for the second and third of them.
bandReports: Dict[str, dict] = {}
bandUsers: Dict[str, List[str]] = {}


def remapBands(diffusePath: Optional[str], report: Optional[dict] = None):
    """The lightmap filter for one object: its bands from Tranquil's legend onto Yelan's"""
    def edit(texFile) -> None:
        pixels = np.array(texFile.img.convert("RGBA"))
        alpha = pixels[..., 3]
        diffuse = None
        if (diffusePath and os.path.isfile(diffusePath)):
            src = FRB.TextureFile(diffusePath, readPillowImg = True)
            src.open()
            if (src.hasImage):
                img = src.img.convert("RGB")
                if (img.size != texFile.img.size):
                    img = img.resize(texFile.img.size, Image.BILINEAR)
                diffuse = np.asarray(img)
        masks = []
        for lo, hi, to, condition in Bands:
            mask = (alpha >= lo) & (alpha <= hi)
            test = Conditions[condition]
            if (test is not None and diffuse is not None):
                mask &= test(diffuse)
            masks.append((mask, to, f"{lo}-{hi} -> {to}"))
        for mask, to, label in masks:
            if (report is not None):
                report[label] = report.get(label, 0) + int(mask.sum())
            alpha[mask] = to
        texFile.img = Image.fromarray(pixels, "RGBA")
    return edit


def texReplace(resModObj, kind: str, filterFunc, compress: bool = False) -> FRB.TexReplace:
    """A texture edit through the API's Pillow-engine TexEditor, written once per output file"""
    def fix(resource) -> bool:
        if (resource.fixedPath in _written):
            return True
        _written[resource.fixedPath] = (kind,)
        # mipmaps: a texture written without its chain is sampled from the top level at every
        # distance, which speckles (2026-09-12).
        #
        # compress defaults OFF here, unlike everywhere else in this repo, because the thing being
        # edited IS the alpha and the alpha IS a band selector. BC7's alpha is lossy, and this edit
        # is a three-way permutation (0 -> 255, 115-128 -> 0, 255 -> 121), so a 4x4 block can hold
        # three widely separated values and the encoder splits the difference: measured on Body A,
        # 12.8% of the texture came back one or two off its target (10.8% of it at 254 instead of
        # 255) against 0.7% written uncompressed. The forward direction does not hit this because
        # its edit moves almost everything to one value. The cost is 5.3 MB a texture against 1.3.
        editor = FRB.TexEditor([filterFunc], readPillowImg = True, compress = compress, mipmaps = True)
        editor.fix(FRB.TextureFile(resource.srcPath, readPillowImg = True), resource.fixedPath)
        return True
    placeholder = FRB.TexEditor([], compress = True)
    return FRB.TexReplace(resModObj, placeholder, fixFunc = fix, resSubType = kind)


# ============================================================================== the buffers

class BufferReplace(FRB.RemapBlendReplace):
    """
    RemapBlendReplace for a buffer this script supplies the bytes of. resType picks the stats bucket
    ("blend" / "position" / "texcoord" / "buf"); the API's own default, "resourceRemapBlend", is
    counted nowhere. The built resource is kept referenced from Python as well.
    """

    def buildResModel(self, *args, **kwargs):
        resource = super().buildResModel(*args, **kwargs)
        if (resource is not None):
            _alive.append(resource)
        return resource


def writeBytesFunc(getBytes):
    """A resource fixFunc: write the bytes 'getBytes()' gives to the resource's fixed path"""
    def fix(resource) -> bool:
        data = getBytes()
        if (data is None):
            return False
        with open(resource.fixedPath, "wb") as f:
            f.write(data)
        return True
    return fix


def remapBlend(raw: bytes, row: dict, label: str) -> bytes:
    """
    One component's Blend.buf through its reverse vertex-group row.

    Byte for byte what the API's own RemapBlendReplace produces (checked on all three components),
    including the rule that is easy to miss: a bone slot is remapped only where it carries WEIGHT,
    so [0,0,0,0] with weights [1,0,0,0] becomes [64,0,0,0] rather than [64,64,64,64].

    An unmapped source group becomes the API's -index-1 sentinel, which is a bone, not nothing: the
    model kinks and nothing is logged. Every such group is reported here instead.
    """
    rows = np.frombuffer(raw, dtype = np.uint8).reshape(-1, BlendStride).copy()
    idx = rows[:, 16:].view("<i4").reshape(-1, 4)
    weights = rows[:, :16].view("<f4").reshape(-1, 4)
    out = idx.copy()
    live = (weights > 0)
    missing = []
    for old in np.unique(idx[live]):
        new = row.get(int(old))
        if (new is None):
            missing.append(int(old))
            new = -int(old) - 1
        out[live & (idx == old)] = new
    if (missing):
        print(f"    WARNING: {label} has no remap for weighted vertex groups {sorted(missing)};"
              f" they become NEGATIVE bone indices and the model will kink there")
    rows[:, 16:] = out.view(np.uint8).reshape(-1, BlendStride // 2)
    return rows.tobytes()


class MergedBuffers():
    """
    The three components laid end to end: one Position, one Blend, one Texcoord, and one index
    buffer per Yelan object.
    """

    def __init__(self, files: "ModFiles", components: List[str], vgRows: Dict[str, dict]):
        self.order = [c for c in ComponentOrder if c in components]
        self.offsets: Dict[str, int] = {}
        self.total = 0
        for component in self.order:
            self.offsets[component] = self.total
            self.total += files.vertices[component]

        # the Texcoord stride is levelled up: Tranquil's Eye carries no TEXCOORD1 where the others
        # do, and one buffer has one stride. The pad goes at the END, which is where TEXCOORD1 sits.
        self.texcoordStride = max(files.texcoordStride[c] for c in self.order)
        self.positionStride = max(files.positionStride[c] for c in self.order)

        positions, blends, texcoords = [], [], []
        for component in self.order:
            n = files.vertices[component]
            positions.append(self._lines(files.position[component], n, self.positionStride, f"{component} position"))
            blends.append(np.frombuffer(remapBlend(open(files.blend[component], "rb").read(),
                                                   vgRows[component], f"{component} blend"), dtype = np.uint8).reshape(n, BlendStride))
            texcoords.append(self._lines(files.texcoord[component], n, self.texcoordStride, f"{component} texcoord"))
        self.position = np.concatenate(positions).tobytes()
        self.blend = np.concatenate(blends).tobytes()
        self.texcoord = np.concatenate(texcoords).tobytes()

        # every object's .ib offset by its component's first vertex, and objects landing on the same
        # Yelan slot concatenated into one draw
        self.members: Dict[str, List[tuple]] = {}
        for component in self.order:
            for slot, spec in Plan[component]["slots"].items():
                self.members.setdefault(spec["to"], []).append((component, slot))
        self.ibs: Dict[str, bytes] = {}
        self.ibChanged: Dict[str, bool] = {}
        for obj, members in self.members.items():
            parts = []
            for component, slot in members:
                raw = np.fromfile(files.objects[(component, slot)]["ib"], dtype = "<u4")
                parts.append(raw + self.offsets[component])
            merged = np.concatenate(parts).astype("<u4")
            self.ibs[obj] = merged.tobytes()
            single = (len(members) == 1 and self.offsets[members[0][0]] == 0)
            self.ibChanged[obj] = not single

        print(f"    merged: {self.total} vertices (" + ", ".join(f"{c} {files.vertices[c]} at {self.offsets[c]}" for c in self.order)
              + f"), texcoord stride {self.texcoordStride}")
        for obj, members in self.members.items():
            names = " + ".join(f"{c}{s}" for c, s in members)
            print(f"      {obj}: {len(self.ibs[obj]) // 12} triangles from {names}"
                  + ("" if self.ibChanged[obj] else " (index buffer unchanged)"))

    @staticmethod
    def _lines(path: Optional[str], n: int, stride: int, label: str) -> "np.ndarray":
        if (not path):
            raise ValueError(f"the .ini names no file for the {label}")
        raw = np.fromfile(path, dtype = np.uint8)
        if (len(raw) % n):
            raise ValueError(f"'{os.path.basename(path)}' is not a whole number of lines for {n} vertices")
        own = len(raw) // n
        lines = raw.reshape(n, own)
        if (own == stride):
            return lines
        if (own > stride):
            raise ValueError(f"'{os.path.basename(path)}' has stride {own}, wider than the merged {stride}")
        padded = np.zeros((n, stride), dtype = np.uint8)
        padded[:, :own] = lines
        print(f"    {label}: stride {own} padded to {stride} (the missing TEXCOORD1 zeroed)")
        return padded


# ============================================================================== the tables

def registerTranquil(components: List[str]) -> Tuple[FRB.ModType, Dict[str, dict]]:
    """
    YelanTranquil -> Yelan as a runtime ModType, and the reverse vertex-group row per component.

    The hash TYPE keys carry the component (`Body;ib`, `Bang;position`, ...) -- the classifier's own
    convention for a component column, and the thing that lets one table hold three sets of buffers
    whose types would otherwise collide. Yelan is registered under the SAME keys with her single set
    of values, three times over, so RegAssetRemap's reverse-then-forward lookup collapses the three
    components onto her one model without being told which component it is looking at.
    """
    FRB.CppGlobalModTypes.registerAll()
    shipped = {m.name: m for m in FRB.CppGlobalModTypes.all()}
    GI, TID = int(FRB.GameTypeId.GI), int(FRB.ModTypeId.YelanTranquil)

    hashRows, indexRows = [], []
    for component in components:
        for kind in Kinds:
            hashRows.append(([V, Source, f"{component};{kind}"], Tranquil[component][HashKey[kind]]))
            hashRows.append(([V, Target, f"{component};{kind}"], Yelan[HashKey[kind]]))
        # the source's own draw slots, so the classifier can tell its objects apart and
        # GIMIObjPartFilter can window each one; NO target index rows -- the Yelan index is written
        # from a literal by RegNewVals, and a target row of "0" would blind the reverse lookup
        for slot, spec in Plan[component]["slots"].items():
            indexRows.append(([V, Source, component, slot], str(spec["index"])))
    hashRows.append(([V, Source, "face"], TranquilFaceDiffuse))
    hashRows.append(([V, Target, "face"], Yelan["tex_face_diffuse"]))

    hashes = FRB.Hashes({Source: [Target]}); hashes.addRepoRows(hashRows)
    indices = FRB.Indices({Source: [Target]}); indices.addRepoRows(indexRows)

    vg = shipped[Source].vgRemaps
    rows = {}
    for component in components:
        row = vg.get([Source, component, Target, ""], ["1.0", "5.7"], errorOnNotFound = False)
        if (row is None):
            raise SystemExit(f"the API's vertex group table has no {Source} {component} -> {Target} row")
        rows[component] = dict(row.remap)
        for bone, target in VGRemapFixes.get(component, {}).items():
            was = rows[component].get(bone)
            if (was == target):
                print(f"  note: the shipped {component} row already sends {bone} to {target}; the override is now redundant")
            else:
                print(f"  vertex group override: {component} {bone} -> {target} (the shipped row says {was})")
            rows[component][bone] = target

    keywords = list(shipped[Source].keywords) if hasattr(shipped[Source], "keywords") else []
    modType = FRB.ModType(GI, TID, Source, keywords, hashes, indices, None, vg)
    template = shipped["Keqing"]        # any shipped GI type: its builders are the table builders, which consult the overrides
    modType.iniParseBuilder = template.iniParseBuilder
    modType.iniFixBuilder = template.iniFixBuilder
    modType.iniRemoveBuilder = template.iniRemoveBuilder
    FRB.ModTypeIdTools.registerModType(modType)
    FRB.CppGlobalModTypes.registerMissing()
    return modType, rows


# ============================================================================== the parser

def makeParser(modType: FRB.ModType, components: List[str]):
    """
    A GIMIParser whose sections are sorted by the API's hash / index classifier. Each component's
    buffers are a mod object of their own -- ("Body", "position"), ("Bang", "position"), ... -- which
    is what makes the three sets addressable separately; the drawn objects are told apart by their
    component's ib hash first and their match_first_index second.
    """
    hashOnly, indexKeyToModObj, modObjs = {}, {}, []
    for component in components:
        for kind in Kinds:
            key = f"{component};{kind}"
            if (kind == "ib"):
                indexKeyToModObj[key] = {(component, slot): (component, slot) for slot in Plan[component]["slots"]}
                modObjs += [(component, slot) for slot in Plan[component]["slots"]]
            hashOnly[key] = (component, kind)
            modObjs.append((component, kind))
    hashOnly["face"] = ("", "face")
    modObjs.append(("", "face"))

    # A draw slot's match_first_index is only unique WITHIN its component: Body A, Bang A and Eye A
    # are all index 0. The classifier reverse-looks-up the index on its own, before it knows which
    # component's ib hash the section carried, and "the first remaining candidate wins" -- so every
    # index-0 section resolved to (Body, A), which is not in the Bang's or the Eye's index map, and
    # both fell back to their component's shared ib graph. The symptom is a drawn object that keeps
    # the source's registers and never gets a drawindexed: it was never a drawn object.
    #
    # So the slot is resolved per component here instead, which is the only place the ib hash and the
    # index are both known.
    indexToSlot = {c: {str(spec["index"]): slot for slot, spec in Plan[c]["slots"].items()} for c in components}

    def firstVal(part, key) -> Optional[str]:
        vals = part.getVals(key) if hasattr(part, "getVals") else None
        return vals[0].strip() if vals else None

    def factory(iniFile, modTypeId):
        classifier = FRB.GIMISectionClassifier(dict(hashOnly), modType.hashes, dict(indexKeyToModObj), modType.indices, None)
        # Yelan shares every hash TYPE key with the source (that is how the remap works), so a
        # reverse lookup has to be pinned to the source's own rows or a section classifies as the
        # target's and the fix runs backwards
        classifier.hashNonVersionVals = {"name": Source}
        classifier.indexNonVersionVals = {"name": Source}

        def classify(parser, sectionName, section, disjoint, part, kvps):
            objs = classifier(parser, sectionName, section, disjoint, part, kvps)
            index = firstVal(part, "match_first_index")
            if (index is None):
                return objs
            out = []
            for obj in objs:
                slot = indexToSlot.get(obj[0], {}).get(index) if (isinstance(obj, tuple) and len(obj) == 2 and obj[1] == "ib") else None
                out.append((obj[0], slot) if slot else obj)
            return out

        parser = FRB.GIMIParser(iniFile, modObjs = modObjs, objTargetFuncs = [classify], modTypeId = modTypeId)
        parser.trackKeys = True
        parser.keysToTrack = {"hash", "match_first_index"}
        _alive.extend((classifier, classify))
        return parser
    return factory


# ============================================================================== the mod's files

class ModFiles():
    """
    The mod's buffers and textures per component, found by hash over IniFile.getIfTemplates() -- the
    fixer factory runs BEFORE the parser parses, so the parser's graphs are not available yet.
    """

    def __init__(self, ini, components: List[str], vgRows: Dict[str, dict]):
        self.folder = os.path.dirname(os.path.abspath(ini.file))
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

        byHash = {}
        for template in templates.values():
            h = (first(template, "hash") or "").lower()
            if (h):
                byHash.setdefault(h, []).append(template)

        # Each drawn object's index buffer and textures and, from the section itself, whether it
        # reads a NORMAL MAP: Tranquil's normal-map layout is ps-t0 normal / ps-t1 diffuse / ps-t2
        # light map and her plain one is ps-t0 diffuse / ps-t1 light map, so the presence of a ps-t2
        # says which, and which register the light map is in. Read off the mod rather than assumed.
        self.objects: Dict[tuple, dict] = {}
        for component in components:
            for slot, spec in Plan[component]["slots"].items():
                section = next((t for t in byHash.get(Tranquil[component]["ib"], [])
                                if (first(t, "match_first_index") == str(spec["index"]))), None)
                if (section is None):
                    raise ValueError(f"the .ini has no section on the {component} ib hash with match_first_index {spec['index']}")
                normalMap = bool(first(section, "ps-t2"))
                diffuseRes = first(section, "ps-t1" if normalMap else "ps-t0")
                lightMapRes = first(section, "ps-t2" if normalMap else "ps-t1")
                self.objects[(component, slot)] = {
                    "normalMap": normalMap,
                    "lightMapReg": "ps-t2" if normalMap else "ps-t1",
                    "ib": fileOf(first(section, "ib")),
                    "diffuse": fileOf(diffuseRes), "diffuseRes": diffuseRes,
                    "lightMap": fileOf(lightMapRes), "lightMapRes": lightMapRes,
                    "borrowed": None}

        # an object that binds nothing borrows its donor's textures (see TextureDonor)
        for (component, slot), info in self.objects.items():
            if (info["diffuse"] or info["lightMap"]):
                continue
            donor = TextureDonor.get(component)
            src = self.objects.get(donor) if donor else None
            if (src is None or not (src["diffuseRes"] or src["lightMapRes"])):
                print(f"    NOTE: {component}{slot} binds no textures and has no donor to borrow from;"
                      f" it will be drawn with no texture registers and NO fix call")
                continue
            info.update({"normalMap": False, "lightMapReg": "ps-t1", "borrowed": donor,
                         "diffuse": src["diffuse"], "diffuseRes": src["diffuseRes"],
                         "lightMap": src["lightMap"], "lightMapRes": src["lightMapRes"]})

        self.blend, self.position, self.texcoord = {}, {}, {}
        self.vertices, self.positionStride, self.texcoordStride = {}, {}, {}
        rel = lambda p: os.path.relpath(p, self.folder) if p else "-"
        for component in components:
            self.blend[component] = next((fileOf(first(t, "vb1")) for t in byHash.get(Tranquil[component]["blend_vb"], [])), None)
            self.position[component] = next((fileOf(first(t, "vb0")) for t in byHash.get(Tranquil[component]["position_vb"], [])), None)
            self.texcoord[component] = next((fileOf(first(t, "vb1")) for t in byHash.get(Tranquil[component]["texcoord_vb"], [])), None)
            if (not self.blend[component]):
                raise ValueError(f"the .ini names no Blend.buf for the {component} component (hash {Tranquil[component]['blend_vb']})")
            self.vertices[component] = os.path.getsize(self.blend[component]) // BlendStride
            n = self.vertices[component]
            self.positionStride[component] = (os.path.getsize(self.position[component]) // n) if (self.position[component] and n) else 40
            self.texcoordStride[component] = (os.path.getsize(self.texcoord[component]) // n) if (self.texcoord[component] and n) else 20
            print(f"    {component}: {n} vertices; blend {rel(self.blend[component])}, position {rel(self.position[component])}"
                  f", texcoord {rel(self.texcoord[component])} (strides {self.positionStride[component]}/{self.texcoordStride[component]})")
            for slot in Plan[component]["slots"]:
                o = self.objects[(component, slot)]
                borrowed = f"  <- borrowed from {o['borrowed'][0]}{o['borrowed'][1]}" if o["borrowed"] else ""
                print(f"      {slot}: {'normal-map' if o['normalMap'] else 'plain'} layout;"
                      f" ib {rel(o['ib'])}, diffuse {rel(o['diffuse'])}, light map {rel(o['lightMap'])} on {o['lightMapReg']}{borrowed}")

        self.merged = MergedBuffers(self, components, vgRows)


_files: Dict[str, ModFiles] = {}


def filesFor(ini, components: List[str], vgRows: Dict[str, dict]) -> ModFiles:
    key = os.path.normcase(os.path.abspath(ini.file))
    if (key not in _files):
        print(f"  {os.path.basename(key)}:")
        _files[key] = ModFiles(ini, components, vgRows)
    return _files[key]


# ============================================================================== the fixer

def makeFixer(components: List[str], vgRows: Dict[str, dict], skipTextures: bool = False, compress: bool = False):
    naming = FRB.CppIniNamingTools

    def factory(parser, toModName: str, modTypeId: int):
        ini = parser._iniFile
        modType = FRB.ModTypeIdTools.getModType(modTypeId)
        files = filesFor(ini, components, vgRows)
        merged = files.merged
        targetVertices = int(FRB.CppGlobalModTypes.all()[0].vertexCounts.get([Target], V))
        skeleton = merged.order[0]        # the component whose buffer sections carry the merged files

        # one representative source slot per Yelan object; the others are dropped, since the
        # representative's draw covers their triangles through the concatenated index buffer
        rep = {obj: members[0] for obj, members in merged.members.items()}
        for obj, members in merged.members.items():
            binds = {(files.objects[m]["diffuse"], files.objects[m]["lightMap"]) for m in members}
            if (len(binds) > 1):
                print(f"  WARNING: {obj} is drawn from {members} but they bind DIFFERENT textures {binds};"
                      f" one draw cannot serve both -- only {rep[obj]}'s textures will be used")

        # ---- 1. the graphs onto Yelan's, all in ONE .ini file ----
        remap = {}
        for component in components:
            for kind in Kinds:
                remap[(0, component, kind)] = [(0, "", kind, keepName)] if (component == skeleton) else []
            for slot in Plan[component]["slots"]:
                obj = Plan[component]["slots"][slot]["to"]
                remap[(0, component, slot)] = [(0, "", obj)] if (rep[obj] == (component, slot)) else []
        remap[(0, "", "face")] = [(0, "", "face", keepName)]
        edits: List[object] = [FRB.GraphGroupRemap(remap = remap)]

        # ---- 2. the match_first_index, windowed to the copied object's own KVPs ----
        objFilter = FRB.GIMIObjPartFilter(modType.hashes, modType.indices, {f"{c};ib" for c in components}, None)
        _alive.append(objFilter)
        indexEdits, indexFilters, indexKeys, indexTrack = {}, {}, {}, {}
        for obj, (component, slot) in rep.items():
            key = ("", obj)
            indexEdits[key] = [FRB.RegNewVals({"match_first_index": str(Yelan["objects"][obj])})]
            indexFilters[key] = [objFilter.filter((component, slot))]
            indexKeys[key] = objFilter.keysToTrack()
            indexTrack[key] = True
        edits.append(FRB.GraphGroupEdit([indexEdits], trackKeys = [indexTrack], keysToTrack = [indexKeys], keyFilters = [indexFilters]))

        # ---- 3. the merged buffers ----
        for kind, reg, resType in (("position", "vb0", "position"), ("blend", "vb1", "blend"), ("texcoord", "vb1", "texcoord")):
            data = getattr(merged, kind)
            edit = BufferReplace((0, "", kind + "Merged"), resType = resType, resSubType = kind.capitalize(),
                                 fixFunc = writeBytesFunc(lambda data = data: data))
            edits.append(FRB.ResRegCollect({(0, "", kind): reg}, {kind: edit}))
            _alive.append(edit)
        for obj in merged.members:
            if (not merged.ibChanged[obj]):
                continue      # its component starts at vertex 0 and nothing else draws into it
            edit = BufferReplace((0, "", obj + "MergedIb"), resType = "buf", resSubType = "Ib",
                                 fixFunc = writeBytesFunc(lambda data = merged.ibs[obj]: data))
            edits.append(FRB.ResRegCollect({(0, "", obj): "ib"}, {"ib": edit}))
            _alive.append(edit)

        # ---- 4. the light map bands, at the register the SOURCE holds them in ----
        #
        # Before the shift below, not after: the collect names a register of the section as it
        # stands, which is Tranquil's layout (ps-t2 where the object reads a normal map, ps-t1
        # where it does not). The shift in step 5 then moves the edited result down to ps-t1.
        # an object that brought no textures gets its donor's registers ADDED first, so that the
        # collect below has something to collect and the draw binds something at all
        adds = {}
        for obj, (component, slot) in rep.items():
            info = files.objects[(component, slot)]
            if (info["borrowed"] and info["diffuseRes"] and info["lightMapRes"]):
                adds[("", obj)] = [FRB.RegNewVals({"ps-t0": info["diffuseRes"], "ps-t1": info["lightMapRes"]}, addNewKVPs = True)]
                print(f"  {obj}: no textures of its own, borrowing {info['borrowed'][0]}{info['borrowed'][1]}'s"
                      f" ({info['diffuseRes']} / {info['lightMapRes']})")
        if (adds):
            edits.append(FRB.GraphGroupEdit([adds]))

        if (not skipTextures):
            # Two objects sharing one source light map share one edited FILE, so an edit that
            # depends on something else (here: the diffuse under the pixel, which decides every
            # band move) must agree between them or one object silently gets the other's texture.
            # Checked here rather than left to the writer: the API dedups before the write is
            # reached, so a runtime guard there never fires.
            diffuseOf: Dict[str, set] = {}
            for component, slot in rep.values():
                info = files.objects[(component, slot)]
                if (info["lightMap"]):
                    diffuseOf.setdefault(info["lightMap"], set()).add(info["diffuse"])
            for lightMap, diffuses in diffuseOf.items():
                if (len(diffuses) > 1):
                    print(f"  WARNING: {os.path.basename(lightMap)} is shared by objects with DIFFERENT diffuses "
                          f"({sorted(os.path.basename(d or '-') for d in diffuses)}); they need one band edit but"
                          f" will get one file -- give them separate copies of the light map")

            for obj, (component, slot) in rep.items():
                info = files.objects[(component, slot)]
                key = info["lightMap"] or f"<none for {component}{slot}>"
                bandUsers.setdefault(key, []).append(f"{component}{slot} -> {obj}")
                report = bandReports.setdefault(key, {})
                edit = texReplace((0, "", obj + "RemapTexLightMap"), "LightMap",
                                  remapBands(info["diffuse"], report), compress = compress)
                edits.append(FRB.ResRegCollect({(0, "", obj): info["lightMapReg"]}, {"lightMap": edit}))
                _alive.append(edit)

        # ---- 5. everything else ----
        rename = FRB.GraphRename(lambda n: naming.getRemapFixName(n, toModName))
        hashRemap = FRB.RegAssetRemap({"hash": (modType.hashes, "HashNotFound")}, toModName, Source, ini.fromVersion, ini.toVersion)
        dropFixCalls = FRB.RegRemove({"run": lambda _ind, val: val in (NNFix, ORFix)})
        dropNormalMap = FRB.RegRemove({"ps-t0": None})
        shiftDown = FRB.RegRemap({"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"]})
        fillDraw = FRB.RegFillMissing("drawindexed", "auto", fillMode = FRB.RegFillMissingMode.BottomCover)
        removeDraw = FRB.RegRemove({"drawindexed": None})
        addFix = FRB.RegDelimitedAdd([("run", NNFix)], {"drawindexed": []}, pathEndOnlyWhenUndelimited = True,
                                    mode = FRB.RegDelimitedAddMode.PerPath)   # ONE call per path: NNFix/ORFix re-slot the ps-t registers, so two undo each other

        group: Dict[tuple, list] = {}
        for obj, (component, slot) in rep.items():
            info = files.objects[(component, slot)]
            shift = [dropNormalMap, shiftDown] if info["normalMap"] else []
            # no textures and no donor: no fix call either, or NNFix re-slots registers this section
            # never bound and scrambles what the game had set
            fixCall = [addFix] if (info["diffuse"] or info["lightMap"]) else []
            if (not fixCall):
                print(f"  {obj}: drawn with no texture registers, so no {NNFix.rsplit(chr(92), 1)[-1]} call")
            group[("", obj)] = [dropFixCalls] + shift + [fillDraw] + fixCall + [hashRemap]
        group[("", "ib")] = [FRB.GraphRename(lambda n: naming.getRemapIbName(n, toModName)), hashRemap, removeDraw]
        group[("", "blend")] = [FRB.GraphRename(lambda n: naming.getRemapBlendName(n, toModName)), hashRemap,
                                FRB.RegNewVals({"draw": f"{merged.total},0"})]
        group[("", "position")] = [rename, hashRemap]
        group[("", "texcoord")] = [rename, hashRemap]
        # Yelan is 16062 vertices and the merged model is larger: without the raise the draw reads
        # past the end of a buffer sized for her own model
        raise_ = []
        if (merged.total > targetVertices):
            raise_ = [FRB.RegNewVals({"override_byte_stride": str(merged.positionStride),
                                      "override_vertex_count": str(merged.total)}, addNewKVPs = True)]
        group[("", "other")] = [rename, hashRemap] + raise_
        # The face diffuse is bound at ps-t1 by BOTH skins' main pass (dump draws 39-41 for Yelan,
        # 42-48 for Tranquil) -- GI 6.x swapped it with the light map. But a MOD may still write the
        # pre-6.x ps-t0, and this recolor does, so it is normalised rather than passed through.
        # Reading "no register change needed" off an identity mod I had written myself was an
        # over-fit; ps-t0 there would have clobbered the face LIGHT MAP.
        group[("", "face")] = [rename, hashRemap, FRB.RegRemap({"ps-t0": ["ps-t1"]})]
        edits.append(FRB.GraphGroupEdit([group]))

        _alive.extend(edits)
        fixer = FRB.GIMIFixer(parser, graphGroupEdits = edits, modsToFix = [toModName])
        _alive.append(fixer)
        return fixer
    return factory


# ============================================================================== run

def runService(modFolder: str, args):
    service = FRB.RemapService(path = modFolder, keepBackups = args.keepBackups,
                               forcedModTypeIds = {int(FRB.ModTypeId.YelanTranquil)},
                               logger = FRB.Logger() if args.verbose else None)
    service.fix()
    stats = service.stats
    print(f"\n.ini fixed: {len(stats.ini.fixed)}, skipped: {len(stats.ini.skipped)}")
    for path, error in stats.ini.skipped.items():
        print(f"  SKIPPED {os.path.relpath(path, modFolder)}: {error}")
    for label in ("blend", "position", "texcoord", "buf", "texEdit", "texAdd"):
        s = getattr(stats, label)
        print(f"{label}: fixed {len(s.fixed)}, skipped {len(s.skipped)}")
        for path in sorted(s.fixed):
            print(f"  {os.path.relpath(path, modFolder)}")
        for path, error in s.skipped.items():
            print(f"  SKIPPED {os.path.relpath(path, modFolder)}: {error}")
    # what each band move actually touched. A move that reports 0 pixels is a move that did nothing:
    # either the mod does not use that band, or its diffuse condition rejected every pixel
    if (bandReports):
        print("\nlight map bands moved (pixels), per source light map:")
        for src in sorted(bandReports):
            counts = bandReports[src]
            print(f"  {os.path.basename(src)}  [{', '.join(bandUsers.get(src, []))}]")
            print("     " + (", ".join(f"{label} {n}" for label, n in sorted(counts.items())) or "NOTHING MOVED"))


def main():
    parser = argparse.ArgumentParser(description = "YelanTranquil -> Yelan, through the API's parser and fixer")
    parser.add_argument("mod", help = "the mod folder (every YelanTranquil .ini under it is fixed)")
    parser.add_argument("--components", default = ",".join(ComponentOrder), help = "source components to remap (default: %(default)s)")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--noTextures", action = "store_true", help = "leave the light map bands alone (geometry only)")
    parser.add_argument("--compressTextures", action = "store_true",
                        help = "write the edited light maps BC7-compressed (smaller, but the band values blur)")
    args = parser.parse_args()
    args.mod = winToPosix(args.mod)
    components = [c.strip() for c in args.components.split(",") if c.strip()]
    unknown = [c for c in components if (c not in Plan)]
    if (unknown):
        raise SystemExit(f"unknown component(s) {unknown}; choose from {list(Plan)}")

    modType, vgRows = registerTranquil(components)
    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser(Source, makeParser(modType, components))
    FRB.CppStrategyOverrides.setFixer(Source, Target, makeFixer(components, vgRows, skipTextures = args.noTextures,
                                                                compress = args.compressTextures))
    try:
        runService(os.path.abspath(args.mod), args)
    finally:
        FRB.CppStrategyOverrides.clear()


if (__name__ == "__main__"):
    main()

#
# ===== adventureToBennettFix (prototype v1) =====
#
# BennettAdventure -> Bennett: a multi-component SOURCE onto a classic single-component target, the
# inverse of bennettAdventureFix.py. A copy of tranquilToYelanFix.py with this pair's tables; the
# machinery from "the lightmap bands" down is that script's, including its buffer MERGE -- the
# design note at the top of it is the one to read for why the components are concatenated into one
# buffer set and one .ini file rather than given one .ini each.
#
#   py -3 adventureToBennettFix.py <mod folder>                     fix every BennettAdventure .ini under the folder
#   py -3 adventureToBennettFix.py <mod folder> --components Body   only some source components
#   py -3 adventureToBennettFix.py <mod folder> --keepBackups       keep the .ini backups the API makes
#   py -3 adventureToBennettFix.py <mod folder> --verbose           attach the API's logger
#   py -3 adventureToBennettFix.py <mod folder> --noTextures        geometry only, light maps untouched
#
# ---- What lands where (measured 2026-09-14, not read off the slot names) ----
#
# Every draw isolated out of the two frame analyses taken today -- each draw's render target minus
# the one before it -- because the names say nothing (YelanTranquil's "Body B" turned out to be her
# jewellery). What the four source slots actually draw:
#
#   Adventure Body A  14778 tris  torso, arms, hands, jacket, back hair  -> Bennett body  (9879)
#   Adventure Body B  13073 tris  legs, trousers, boots                  -> Bennett body  (9879)
#   Adventure Bang     3128 tris  the front fringe                       -> Bennett head  (0)
#   Adventure Eye       276 tris  the eyes                               -> Bennett head  (0)
#
# Bennett's own two draws, for comparison: head = the fringe and his goggles, body = everything else
# including his face and cape.
#
# So BOTH pairs collapse: A and B onto his body, the Bang and the Eye onto his head. Yelan had only
# the one pair (Bang + Eye -> head); here the machinery's "two slots may name the same target object,
# their index buffers are concatenated into one draw" runs twice.
#
# ---- The register layout ----
#
# Bennett's own draws are all the no-normal-map shader 95aa6cdb84eb7b99 (draws 40 and 41), so every
# remapped object is the two-register NNFix layout: ps-t0 diffuse, ps-t1 light map. The source's
# Body A, Body B and Bang read a normal map on ps-t0, so for those the normal map is DROPPED and the
# rest shifted down (ps-t1 -> ps-t0, ps-t2 -> ps-t1) -- the GanyuTwilight -> Ganyu shape. Her Eye is
# already the plain layout (95aa6cdb84eb7b99, the same shader as Bennett's own draws) and is left
# alone.
#
# The face diffuse needs no register change, only the hash: BOTH characters bind it at ps-t1 with
# the face light map d4841e1a at ps-t0 (main draws 42/43/46/47/52/53 on the skin, 37/38/39 on
# Bennett). Same as the Yelan pair.
#
# ---- The light map bands: nothing moves, and that is measured ----
#
# A GIMI light map's alpha is a material band selecting a shading ramp, and the legend is per skin,
# so the Yelan pair needed three bands moved. This pair needs none. Histogrammed off both sides'
# own textures (alpha counts with the mean diffuse under each band):
#
#   Adventure Body A   0 (35%) | 78 | 126-128 | 177-178 (30%, pale 213/208/196) | 255 SKIN (233/203/180, 100% skin-coloured)
#   Adventure Body B   0 (48%, orange 188/135/79) | 78 (26%, blue 35/76/98) | 126-128 | 176-178 | NO 255 band
#   Bennett body       0 (77%, dark cloth) | 126-128 (white trim, 242/241/236) | 255 SKIN (232/204/181, 99.5%)
#   Bennett head       0 (85%, pale 215/209/197) | 255 (13.5%, dark 102/105/71)
#
# The one band that can be identified on both sides -- skin -- is already at 255 on both, with the
# diffuse means agreeing to within 1/255 per channel. Every other band of the source has no
# identifiable counterpart in Bennett's legend, and moving one on a guess is not a no-op: it puts a
# value on a ramp that means something else, which is how the Yelan run put static on her neck. So
# Bands is EMPTY until something in game says otherwise, and the mod's light maps pass through.
#
# ---- The per-member blocks, and A BUG IN THE SHARED MACHINERY THEY EXPOSED (2026-09-15) ----
#
# `body` is BodyA + BodyB and `head` is BangA + EyeA, so both of Bennett's objects are merges. The
# `body` section carries BodyA's bindings + `drawindexed = auto`, then BodyB's bindings +
# `drawindexed = 39219, 44334, 0`, every line at the section's own depth -- checked by counting
# if/endif rather than by eye, since stripping whitespace throws away the nesting that is the whole
# question. `head` gets no appended draw, correctly: its members agree on their textures (both borrow
# Body A's) and the mod draws through `auto`, which is the one case that covers a merge on its own.
#
# **`match_first_index` DOES NOT LAND THROUGH THE WINDOWED PASS ON A MULTI-MEMBER OBJECT.** Measured:
# `body` (2 members, target index 9879) came out 0, while the same edit run with `--components Body`
# alone wrote 9879. Neither a pass per object nor a GIMIObjPartFilter per object nor narrowing the
# filter to the component's own ib hash type changed it. It is written in the object's own group pass
# instead, where every other per-object edit already lands.
#
# tranquilToYelanFix.py has the same defect and CANNOT SHOW IT: its only multi-member object is
# `head`, whose target index is 0, so a write that never happened is indistinguishable from one that
# did. Verified against a real run of it -- her body/extra/dress come out 20913/54042/51759 through
# single-member objects, and her head reads 0 either way. Check that before transcribing either
# direction into C++, and prefer the group-pass write there too.
#
# The member's light map binds the mod's OWN resource rather than an edited copy; the reason, and
# the guard for when a band move makes that wrong, is at the emission itself.
#
# ---- The vertex group rows ----
#
# Shipped in data/VGRemapData.cpp as of this session: BennettAdventure Body (106 groups), Bang (9)
# and Eye (2) onto Bennett. Proposed by Tools/VGRemapFinder from the geometry and NOT yet confirmed
# in game -- Bennett has no hand-made draft to score against.
#
# The mirror-pair check that found YelanTranquil's warped knee was run on these and they PASS: of 59
# mirror pairs in the Body, exactly one collapses onto a single target bone (69 and 70 -> Bennett 4)
# and that bone is on the midline (X +0.000), so neither side is dragged across. VGRemapFixes is
# therefore empty, where the Yelan prototype needed an override.
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

Source, Target = "BennettAdventure", "Bennett"

Kinds = ("position", "blend", "texcoord", "other", "ib")
HashKey = {"position": "position_vb", "blend": "blend_vb", "texcoord": "texcoord_vb", "other": "draw_vb", "ib": "ib"}
BlendStride = 32

# Bennett's CURRENT model: his hash.json moved three times inside the library's window (draw_vb at
# 4.1, ib at 4.3, position_vb at 4.4) and these are the post-4.4 values.
Bennett = {"draw_vb": "02cf3aa5", "position_vb": "6cff51b4", "blend_vb": "d4acf3f7", "texcoord_vb": "acde80a4",
           "ib": "cdc66323", "tex_face_diffuse": "50f7dc9a",
           "objects": {"head": 0, "body": 9879}}
Adventure = {"Body": {"draw_vb": "bc87167b", "position_vb": "14efbc45", "blend_vb": "09b92379", "texcoord_vb": "51dd19aa", "ib": "022a9ccd"},
             "Bang": {"draw_vb": "2f953b46", "position_vb": "a8a0adb9", "blend_vb": "d2bb6147", "texcoord_vb": "5a79eaa8", "ib": "43ad99d1"},
             "Eye":  {"draw_vb": "feb0e532", "position_vb": "f5dd3d9e", "blend_vb": "89827a3f", "texcoord_vb": "941adcbf", "ib": "91b4d5dd"}}
AdventureFaceDiffuse = "2b1b2edf"

# The order the components are laid into the merged buffers. Body first on purpose: its objects then
# start at vertex 0 and their index buffers pass through untouched.
ComponentOrder = ("Body", "Bang", "Eye")

# Where an object with NO textures of its own gets them. The skin's Bang and Eye have empty texture
# lists in hash.json, and the frame analysis says which set the game draws them with: draws 49 and 48
# both bind Body slot A's 04cd73c6 / c3e39ad5 / bfa7fe04. A mod may leave those sections with an `ib`
# and nothing else, and then the remapped head draw binds no texture at all -- which is worse than an
# untextured head, because ORFix / NNFix re-slot whatever is bound whether the section bound it or
# not, so a fix call over a section that brought nothing scrambles the registers the game had set.
TextureDonor = {"Bang": ("Body", "A"), "Eye": ("Body", "A")}

# Each source draw slot: its match_first_index on the skin, and the Bennett object it lands on. Two
# slots naming the same Bennett object have their index buffers concatenated into one draw -- which
# happens TWICE here, unlike the Yelan pair. See the header for what each slot actually draws.
Plan = {"Body": {"slots": {"A": {"index": 0,     "to": "body"},
                           "B": {"index": 44334, "to": "body"}}},
        "Bang": {"slots": {"A": {"index": 0,     "to": "head"}}},
        "Eye":  {"slots": {"A": {"index": 0,     "to": "head"}}}}

# No corrections needed to the shipped reverse rows: the mirror-pair check passes (see the header).
# Yelan needed one here, for a knee that warped in game.
VGRemapFixes: Dict[str, Dict[int, int]] = {}

_alive: List[object] = []          # objects the API holds by reference and Python would otherwise collect
_written: Dict[str, tuple] = {}    # output texture path -> what produced it, so a shared source is written once
keepName = lambda name: name


# ============================================================================== the lightmap bands
#
# EMPTY ON PURPOSE -- see the header. The only band identifiable on both sides is skin, and it is
# already at 255 on both, so there is nothing to move. Add a row here only with a measurement or an
# in-game observation behind it; a wrong move is not a no-op.
Bands: Tuple[Tuple[int, int, int, str], ...] = ()

def skinColoured(rgb: "np.ndarray") -> "np.ndarray":
    """Where a diffuse (H x W x 3) is skin: warm and bright, red over green over blue"""
    r, g, b = (rgb[..., i].astype(np.int16) for i in range(3))
    return (r >= 96) & (r >= g) & (g >= b) & ((r - b) >= 16) & ((r - b) <= 140)


def whiteFurColoured(rgb: "np.ndarray") -> "np.ndarray":
    """Where a diffuse is the source's white fur (UNUSED: Bands is empty for this pair)"""
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
    """The lightmap filter for one object: its bands from the source's legend onto the target's (UNUSED: Bands is empty for this pair)"""
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
    buffer per Bennett object.
    """

    def __init__(self, files: "ModFiles", components: List[str], vgRows: Dict[str, dict]):
        self.order = [c for c in ComponentOrder if c in components]
        self.offsets: Dict[str, int] = {}
        self.total = 0
        for component in self.order:
            self.offsets[component] = self.total
            self.total += files.vertices[component]

        # the Texcoord stride is levelled up: the source's Eye and Bang carry no TEXCOORD1 where the Body does
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
        # Bennett slot concatenated into one draw
        self.members: Dict[str, List[tuple]] = {}
        for component in self.order:
            for slot, spec in Plan[component]["slots"].items():
                self.members.setdefault(spec["to"], []).append((component, slot))
        self.ibs: Dict[str, bytes] = {}
        self.ibChanged: Dict[str, bool] = {}
        # each member's (component, slot, index offset, index count) inside the merged buffer: what
        # the appended per-member draws below are built from. Measured off the files, never guessed --
        # a draw from a guessed count addresses whatever happens to sit at that offset.
        self.ranges: Dict[str, List[tuple]] = {}
        for obj, members in self.members.items():
            parts = []
            running = 0
            for component, slot in members:
                raw = np.fromfile(files.objects[(component, slot)]["ib"], dtype = "<u4")
                parts.append(raw + self.offsets[component])
                self.ranges.setdefault(obj, []).append((component, slot, running, len(raw)))
                running += len(raw)
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

def registerAdventure(components: List[str]) -> Tuple[FRB.ModType, Dict[str, dict]]:
    """
    BennettAdventure -> Bennett as a runtime ModType, and the reverse vertex-group row per component.

    The hash TYPE keys carry the component (`Body;ib`, `Bang;position`, ...) -- the classifier's own
    convention for a component column, and the thing that lets one table hold three sets of buffers
    whose types would otherwise collide. Bennett is registered under the SAME keys with his single set
    of values, three times over, so RegAssetRemap's reverse-then-forward lookup collapses the three
    components onto her one model without being told which component it is looking at.
    """
    FRB.CppGlobalModTypes.registerAll()
    shipped = {m.name: m for m in FRB.CppGlobalModTypes.all()}
    GI, TID = int(FRB.GameTypeId.GI), int(FRB.ModTypeId.BennettAdventure)

    hashRows, indexRows = [], []
    for component in components:
        for kind in Kinds:
            hashRows.append(([V, Source, f"{component};{kind}"], Adventure[component][HashKey[kind]]))
            hashRows.append(([V, Target, f"{component};{kind}"], Bennett[HashKey[kind]]))
        # the source's own draw slots, so the classifier can tell its objects apart and
        # GIMIObjPartFilter can window each one; NO target index rows -- the Bennett index is written
        # from a literal by RegNewVals, and a target row of "0" would blind the reverse lookup
        for slot, spec in Plan[component]["slots"].items():
            indexRows.append(([V, Source, component, slot], str(spec["index"])))
    hashRows.append(([V, Source, "face"], AdventureFaceDiffuse))
    hashRows.append(([V, Target, "face"], Bennett["tex_face_diffuse"]))

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
        # Bennett shares every hash TYPE key with the source (that is how the remap works), so a
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
        # reads a NORMAL MAP: the source's normal-map layout is ps-t0 normal / ps-t1 diffuse / ps-t2
        # light map and her plain one is ps-t0 diffuse / ps-t1 light map, so the presence of a ps-t2
        # says which, and which register the light map is in. Read off the mod rather than assumed.
        self.objects: Dict[tuple, dict] = {}
        for component in components:
            for slot, spec in Plan[component]["slots"].items():
                section = next((t for t in byHash.get(Adventure[component]["ib"], [])
                                if (first(t, "match_first_index") == str(spec["index"]))), None)
                if (section is None):
                    raise ValueError(f"the .ini has no section on the {component} ib hash with match_first_index {spec['index']}")
                normalMap = bool(first(section, "ps-t2"))
                diffuseRes = first(section, "ps-t1" if normalMap else "ps-t0")
                lightMapRes = first(section, "ps-t2" if normalMap else "ps-t1")
                self.objects[(component, slot)] = {
                    "normalMap": normalMap,
                    "draws": bool(first(section, "drawindexed")),
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
            self.blend[component] = next((fileOf(first(t, "vb1")) for t in byHash.get(Adventure[component]["blend_vb"], [])), None)
            self.position[component] = next((fileOf(first(t, "vb0")) for t in byHash.get(Adventure[component]["position_vb"], [])), None)
            self.texcoord[component] = next((fileOf(first(t, "vb1")) for t in byHash.get(Adventure[component]["texcoord_vb"], [])), None)
            if (not self.blend[component]):
                raise ValueError(f"the .ini names no Blend.buf for the {component} component (hash {Adventure[component]['blend_vb']})")
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

        # one representative source slot per Bennett object; the others are dropped, since the
        # representative's draw covers their triangles through the concatenated index buffer
        rep = {obj: members[0] for obj, members in merged.members.items()}
        for obj, members in merged.members.items():
            binds = {(files.objects[m]["diffuse"], files.objects[m]["lightMap"]) for m in members}
            if (len(binds) > 1):
                print(f"  {obj}: members {members} bind different textures -- each gets its own"
                      f" bindings and its own draw, since one draw binds one set (representative {rep[obj]})")

        # ---- 1. the graphs onto Bennett's, all in ONE .ini file ----
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
        # ONE GraphGroupEdit PER OBJECT, not one carrying every object.
        #
        # A draw slot's match_first_index is unique only WITHIN its component, and BennettAdventure's
        # Body A, Bang A and Eye A are ALL 0. Put every object's edit in one pass and the windows
        # cannot tell those sections apart: each edit reaches both, and the last one written wins.
        # Measured -- `body` came out 0 (the `head` edit's value) with all three components, and the
        # correct 9879 with `--components Body` alone, which is the same edit in a pass of its own.
        #
        # The Yelan pair has the identical collision (her Body A, Bang A and Eye A are also 0), so
        # tranquilToYelanFix.py very likely carries this too -- worth checking her generated
        # match_first_index before transcribing either direction into C++.
        # ...and A FILTER OF ITS OWN per object. One shared GIMIObjPartFilter handed out windows by
        # calling .filter(modObj) on the same instance; with three slots that all carry
        # match_first_index 0, every window ended up the last-configured one and `body` took `head`'s
        # value. A fresh instance per object is what keeps the windows independent.
        for obj, (component, slot) in rep.items():
            key = ("", obj)
            # ONLY THIS COMPONENT'S ib hash type. Handing every filter all three made the window
            # ambiguous: match_first_index is unique only within a component and Body A / Bang A /
            # Eye A are all 0, so a window that may match any component's ib could not pick out the
            # body's section. Each component's ib hash IS unique (Body 022a9ccd, Bang 43ad99d1, Eye
            # 91b4d5dd), so hash-type + index together identify the slot exactly.
            objFilter = FRB.GIMIObjPartFilter(modType.hashes, modType.indices, {f"{component};ib"}, None)
            _alive.append(objFilter)
            edits.append(FRB.GraphGroupEdit(
                [{key: [FRB.RegNewVals({"match_first_index": str(Bennett["objects"][obj])})]}],
                trackKeys = [{key: True}],
                keysToTrack = [{key: objFilter.keysToTrack()}],
                keyFilters = [{key: [objFilter.filter((component, slot))]}]))

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
        # stands, which is the source's layout (ps-t2 where the object reads a normal map, ps-t1
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
        # ---- the per-member blocks, for a target object several components merge onto ----
        #
        # See this file's header and GIMIMergeFixer.cpp: the representative's draw covers only the
        # FIRST member, and one section binds one set of registers. Bennett needs this twice over --
        # his body is BodyA + BodyB (different textures) and his head is BangA + EyeA (same textures,
        # but still two ranges).
        memberBlocks = {}
        for obj, ranges in merged.ranges.items():
            if (len(ranges) < 2):
                continue
            repComponent, repSlot = rep[obj]
            repInfo = files.objects[(repComponent, repSlot)]
            differ = len({(files.objects[m]["diffuseRes"], files.objects[m]["lightMapRes"])
                          for m in merged.members[obj]}) > 1

            # `drawindexed = auto` covers the whole merged buffer on its own ONLY when the mod drew
            # nothing of its own and every member agrees on its textures.
            if (not repInfo.get("draws") and not differ):
                continue

            block = []
            for component, slot, offset, count in ranges[1:]:
                info = files.objects[(component, slot)]
                own = ((info["diffuseRes"] or info["lightMapRes"])
                       and (info["diffuseRes"] != repInfo["diffuseRes"]
                            or info["lightMapRes"] != repInfo["lightMapRes"]))
                if (own):
                    if (info["diffuseRes"]):
                        block.append(("ps-t0", info["diffuseRes"]))
                    if (info["lightMapRes"]):
                        # The member's OWN light map resource, as the mod already defines it.
                        #
                        # The representative's is bound to an EDITED copy, whose resource the API
                        # names itself (getRemapTexResourceName -> ...BennettLightMapRemapTex). A
                        # member cannot go through the same path: ResRegCollect collects one named
                        # register out of the section, and the member's registers are appended by
                        # RegBottomAdd afterwards, so there is nothing for a collect to name. Writing
                        # the edited name by hand is what the first version did, and it produced
                        # `bodyRemapTexLightMap` -- a reference no resource section defines.
                        #
                        # Binding the original is CORRECT while Bands is empty, because the
                        # representative's "edit" is then a pass-through copy and the two agree. It
                        # stops being correct the moment a band moves, so that case raises rather
                        # than silently shipping one edited light map and one unedited one.
                        if (Bands and not skipTextures):
                            raise SystemExit(
                                f"{obj}: member {component}{slot} needs its own EDITED light map (Bands is not empty),"
                                " and the prototype cannot name one -- give the member its own texture edit"
                                " (see GIMIMergeFixer.cpp's buildMemberTexEdits) before enabling a band move")
                        block.append(("ps-t1", info["lightMapRes"]))
                    # its own fix call: rebinding ps-t0/ps-t1 starts a new binding epoch and NNFix
                    # re-slots whatever is bound when it runs, so the PerPath call ahead of the
                    # section's first draw does not serve this one
                    block.append(("run", NNFix))
                block.append(("drawindexed", f"{count}, {offset}, 0"))
            if (block):
                memberBlocks[("", obj)] = [FRB.RegBottomAdd(block)]
                drawnBy = ", ".join(f"{c}{s}" for c, s, _, _ in ranges)
                print(f"  {obj}: {len(ranges)} members ({drawnBy}), "
                      + ("textures differ -- " if differ else "")
                      + f"appending {sum(1 for k, _ in block if k == 'drawindexed')} draw(s)")

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
            # The per-member blocks go AFTER the section's own fix call and before the hash remap:
            # each carries its own `run =` where it rebinds registers, so the PerPath call above has
            # to be placed already. RegBottomAdd puts them at the section's own depth, outside every
            # `if` block -- RegSurroundedAdd(latest) would land inside the last one.
            # match_first_index is written HERE, in the object's own group pass, not through the
            # windowed pass above. On a target object that several components MERGE onto, the
            # windowed write does not land -- measured: `body` (2 members, target index 9879) stayed
            # 0, while the same edit with `--components Body` alone wrote 9879 correctly.
            #
            # tranquilToYelanFix.py has the same defect and cannot show it: its only multi-member
            # object is `head`, whose target index IS 0, so a write that never happened is
            # indistinguishable from one that did. Check Yelan's `head` before trusting hers.
            setIndex = FRB.RegNewVals({"match_first_index": str(Bennett["objects"][obj])})
            group[("", obj)] = ([dropFixCalls] + shift + [fillDraw] + fixCall
                                + memberBlocks.get(("", obj), []) + [setIndex, hashRemap])
        group[("", "ib")] = [FRB.GraphRename(lambda n: naming.getRemapIbName(n, toModName)), hashRemap, removeDraw]
        group[("", "blend")] = [FRB.GraphRename(lambda n: naming.getRemapBlendName(n, toModName)), hashRemap,
                                FRB.RegNewVals({"draw": f"{merged.total},0"})]
        group[("", "position")] = [rename, hashRemap]
        group[("", "texcoord")] = [rename, hashRemap]
        # Bennett is 16433 vertices and the merged model is larger: without the raise the draw reads
        # past the end of a buffer sized for her own model
        raise_ = []
        if (merged.total > targetVertices):
            raise_ = [FRB.RegNewVals({"override_byte_stride": str(merged.positionStride),
                                      "override_vertex_count": str(merged.total)}, addNewKVPs = True)]
        group[("", "other")] = [rename, hashRemap] + raise_
        # The face diffuse is bound at ps-t1 by BOTH skins' main pass (dump draws 37-39 for Bennett,
        # 42-53 for BennettAdventure) -- GI 6.x swapped it with the light map. But a MOD may still write the
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
                               forcedModTypeIds = {int(FRB.ModTypeId.BennettAdventure)},
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
    parser = argparse.ArgumentParser(description = "BennettAdventure -> Bennett, through the API's parser and fixer")
    parser.add_argument("mod", help = "the mod folder (every BennettAdventure .ini under it is fixed)")
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

    modType, vgRows = registerAdventure(components)
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

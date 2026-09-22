#
# ===== exorcistToSanhuaFix =====
#
# PROTOTYPE of the REVERSE WuWa remap: a WWMI mod made for SanhuaExorcist (WWMI-Assets' SanhuaSkin1),
# fixed IN PLACE so it also renders on Sanhua. The mirror of sanhuaExorcistFix.py and the same shape:
# a parser and a fixer built from Python, registered through CppStrategyOverrides ahead of the
# library's stub rows (SanhuaExorcist's parse row and the SanhuaExorcist -> Sanhua fix row are both
# wwmiStub), driven by the API's own RemapService over the mod folder. The fix is appended to the
# mod's own .ini as a `; --------------- SanhuaExorcist Remap ---------------` block; the remapped
# blend, the zero shape-key stream and the invented mask are written beside the mod's files; a
# second run undoes the first before fixing again; --undo removes it.
#
#   python exorcistToSanhuaFix.py <mod folder> [--keepBackups] [--hideOrig] [--undo] [--verbose]
#                                 [--vgRemap <json: {source group: target group}>]
#                                 [--shapeKeys hide|leave|retarget] [--hideTextureOverrides] [--noSplit] [--plan default|bangsSlot]
#
# Read sanhuaExorcistFix.py's header first: everything it says about what a WWMI mod is and how the
# API fixes one holds here unchanged (the slot sections retargeted by RegNewVals / RegAssetRemap, the
# blend through the library's VGRemaps row over the 8-byte layout, textures bound by REGISTER on the
# target's draws through a per-component command list gated on [ShaderOverride] filter_index tags,
# the zero shape-key stream at vb6, one remapped section per target draw per file, the shape-key
# sections hidden by default, the target slots nothing is drawn through skipped with their bones
# still merged, a texture's role by hash > pixel identity > Component<N>_<Type> name, a planned role
# the mod has no file for bound to the SOURCE's own texture). This header only says what is
# different in this direction, all of it read off the two frame dumps of 2026-09-19
# (FrameAnalysis-Sanhua-2026-09-19-005853 / FrameAnalysis-SanhuaExorcist-2026-09-19-005636, through
# Tools/Misc/Diagnostics/wwmiDrawTable.py, and each dump texture correlated against the download
# folders' asset textures).
#
# ---- The two skins' slots, and the plan (2026-09-19) ----
#
#   Exorcist (source, 6 components)                   Sanhua (target, 7 slots)
#     0 bangs      69e3d321 + 8fbb5532                  0 bangs      a512f04f + f6bc3927 (+ 94d9d5e9, globals only)
#                  t0 f8d5c991 diffuse, t1 63c807fe                  t0 diffuse, t1 the shared default mask, t5 ramp
#                  shared mask, t5 4478285f ramp
#     1 hair       69e3d321: t0 11171f1c, t1 9febd992   1 hair       69e3d321: t0 diffuse, t1 'normal', t5 ramp
#     2 face       374a4f8f: t0 46177147 face mask      2 face       374a4f8f: t0 face mask, t1 face diffuse
#                  (both skins), t1 c98e83cd diffuse
#     3 torso      3093e3c7: t0 72739d6e normal,        3 arm skin   7a0ab7c3: t0 normal, t1 diffuse, NO mask
#       + arms     t1 e0c15187 mask, t2 52f35e6d        4 bodice     96356f03: t0 normal, t1 mask, t2 diffuse
#       + ribbons  diffuse                              5 skirt      96356f03: the same
#     4 bun +      5cc08ed6: t0 221b8ad6 normal,
#       trousers   t1 a global (no mask), t2 8e5306a9
#     5 eyes       056f9f3c: t1 c88cc1fc eye mask, t2   6 eyes       056f9f3c: the same layout
#                  1dcc0f1d iris (both skins), t5 ramp
#
#  So hair / face / eyes go one to one, and the BANGS go through Sanhua's HAIR slot: Exorcist draws her
#  bangs with the hair shader, Sanhua with two bangs shaders of her own that turned a dark-recoloured
#  fringe brown in game (see Plans). Exorcist's torso -- coat, arms and ribbons in one component -- goes
#  through Sanhua's BODICE slot, the body family with a mask slot (t0 normal, t1 mask, t2 diffuse,
#  the torso's own layout). Her hair bun + trousers go through Sanhua's SKIRT slot, the same body
#  family, with an INVENTED mask: Exorcist's slot 4 shader reads no mask at all, and Sanhua's body
#  shader needs one. Nothing goes through Sanhua's ARM SKIN slot (its shader has no mask and would
#  shade the whole coat as skin), so that slot and Sanhua's bangs slot are skipped, their bones still
#  merged. The bangs and the hair share the hair slot's draw, so the bangs' section stays in the
#  mod's .ini and the hair's goes into <stem>RemapFix1.ini (the forward direction's per-window split).
#
#  vb6: on Sanhua the dump binds the shape-key offset stream on the ARM SKIN, FACE and EYE draws
#  (7a0ab7c3 / 374a4f8f / 056f9f3c and their secondary passes), not on the bodice / skirt / hair /
#  bangs ones. The face and eyes are drawn here, so the zero stream is still bound on every remapped
#  section, exactly as forward.
#
# ---- The material masks (measured 2026-09-19, full resolution) ----
#
#  Exorcist's torso mask e0c15187: (0,51,0) 56%, (203,51,0) 21%, (255,77,0) 21% -- the last is SKIN,
#  on her bare arms. Sanhua's body masks (bodice 89ba19a1, skirt f0713dc7) hold only (0,0,0) and
#  (203,0,0): her skin is its own component on its own shader, so her BODY shader (96356f03) is never
#  handed the skin code by the game. Sanhua's FACE mask uses (255,77,0) / (0,51,0) like Exorcist's, and
#  in the forward direction Sanhua's G = 0 masks drew right on Exorcist's torso shader. OPEN, and the
#  first thing to look at in game: whether 96356f03 shades (255,77,0) as skin. If the arms come out
#  shaded like cloth, the torso mask is the thing to edit (or the arms need their own route).
#  The invented mask for the bun + trousers is ClothMask = (0,0,0): Sanhua's plain-cloth code, 96% of
#  her bodice mask and 75% of her skirt's.
#
# ---- The [ShaderOverride] filter_index values are SHARED with the forward direction ----
#
#  3dmigoto keys a ShaderOverride by shader hash, across every loaded .ini, so a mod fixed forward
#  and a mod fixed in reverse, both installed, declare overrides on the SAME hair / face / eye
#  shaders. If their filter_index values differ, whichever file loads last wins and the other mod's
#  `if ps == <filter>` never matches -- its textures silently not bound. So every shader both
#  directions tag keeps the forward's value (69e3d321 = 3381.91, 374a4f8f = 3381.93, 056f9f3c =
#  3381.96), and the shaders only this direction tags take values the forward never uses
#  (3381.81 up). The compiled template derives its values from filterBase / filterStep in slot order,
#  which cannot express this: a port needs an explicit shader -> filter table (or both configs
#  sharing one).
#
# ---- A MOD FROM BEFORE WWMI'S MERGED SKELETON (the ALPHA-2 shape, 2026-09-19) ----
#
#  WWMI grew the merged skeleton in its beta. An older mod ("WWMI ALPHA-2 INI",
#  `required_wwmi_version = 0.70`; the installed core still runs one) has no `$\WWMIv1\vg_offset` in
#  its sections, no `CommandListMergeSkeleton`, and no merged-skeleton resources -- because back then a
#  draw could only address the bones the GAME hands it for that component. So its Blend.buf holds each
#  component's OWN bone indices, not the merged skeleton's: SanhuaExorcist3's component 3 uses 0..104
#  where the identity mod's uses 22..126. Remapped as if they were merged, every bone of the body goes
#  somewhere else -- reported in game as "Sanhua's body became a noodle mess"
#  (`AI Agent Help/CreatingRemaps/Images/SanhuaExorcist/2_5/SanhuaNoddleMess.png`).
#
#  `ModFiles.legacy` detects the shape (no `vg_offset` in any slot section) and the fix then does two
#  things. The blend is read PER COMPONENT and lifted through the source's `vg_map` (the download
#  folder's Metadata.json; the library has vg_offset / vg_count but not the map) before the library's
#  row is applied -- `remapLegacyBlend`. And the fix SUPPLIES the merged skeleton the mod lacks: the
#  four resources, a `[TextureOverrideMarkBoneDataCB]` on the bone-data constant buffer, and one merge
#  command list per target slot that every remapped and hidden section runs. Keeping the mod's old
#  local-space shape instead is not an option: through Sanhua the mod's torso lands on bones 1..203,
#  far outside any single slot's window, and a local index can only name bones inside its own.
#
# Not done, deliberately, until the identity mod has been seen in game: any edit of Exorcist's torso
# mask, the vertex-group anchoring of chains Sanhua has no counterpart for (the forward's --anchor),
# and downloads of a whole missing component.
#

import argparse
import json
import os
import re
import shutil
import struct
import sys
from typing import Dict, List, Optional, Tuple

OnWindows = (sys.platform == "win32")


def winToPosix(path: str) -> str:
    if (OnWindows or (len(path) < 2) or (path[1] != ":") or (not path[0].isalpha())):
        return path
    return "/mnt/" + path[0].lower() + path[2:].replace("\\", "/")


Repo = os.environ.get("AG_REMAP_REPO") or winToPosix(r"C:\Users\AlexX\Documents\Games\Mods\Repos\Anime-Game-Remap")
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
if (not os.path.isdir(APISrc)):
    raise SystemExit(f"the API's package folder is not at {APISrc}; set AG_REMAP_REPO to the repo's path on this OS")
sys.path.insert(0, APISrc)
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))

import FixRaidenBoss2 as FRB     # noqa: E402

SourceName, TargetName = "SanhuaExorcist", "Sanhua"
WuWaVersion = "2.5"               # the version the library files both characters under (a reverse lookup of `0` needs it)
SlotPrefix = "component"          # the 'type' the library files a WWMI draw slot under: component0, component1, ...
ShapeKeyType = "shapekeys"        # the 'type' of the ShapeKeyChecksums row

ShapeKeyZero = "ShapeKeyZero"               # the zero shape-key offset stream bound at vb6 (sanhuaExorcistFix.py's header, point 10)
ShapeKeyStride = 24                         # bytes a vertex in that stream, off the frame dump's vb6 layout
MergedSkeletonSlots = 768                   # the merged skeleton buffer's float4 slots, as WWMI Tools' own template declares it
BoneDataFilter = "3381.7777"                # WWMI's marker on the game's bone-data constant buffer ([TextureOverrideMarkBoneDataCB])
WWMI = "$\\WWMIv1\\"                        # the namespaced variable prefix every WWMI .ini uses
SkeletonMerger = "CustomShader\\WWMIv1\\SkeletonMerger"    # the core's compute shader that merges one window's bones
NL = "\n"

# The textures the fix INVENTS, by role: Sanhua's body shader needs a material mask where Exorcist's
#   slot 4 shader (hair bun, trousers) reads none; (0, 0, 0) is Sanhua's plain-cloth code (see the header)
CreatedTextures: Dict[str, Tuple[int, int, int, int]] = {"ClothMask": (0, 0, 0, 255)}

# The target's pixel shaders that bind CHARACTER textures, per draw slot, off
#   FrameAnalysis-Sanhua-2026-09-19-005853 (wwmiDrawTable.py). Slot 0 (the bangs) is drawn on two
#   bangs passes; its third (94d9d5e9, the eye region) binds only globals. Slots 4 and 5 share a shader.
SlotPasses = {0: ["a512f04f32f6aa26", "f6bc3927337f5b8c"], 1: ["69e3d3219c979981"], 2: ["374a4f8fc9a5ea6a"],
              3: ["7a0ab7c3ffbea13c"], 4: ["96356f03a963d1d2"], 5: ["96356f03a963d1d2"], 6: ["056f9f3c356ff96e"]}

# One filter_index per distinct shader, SHARED with sanhuaExorcistFix.py for the shaders both directions
#   tag (see the header): the forward's values first, then this direction's own from 3381.81 up
ForwardFilters = {"69e3d3219c979981": "3381.91", "8fbb55320274f090": "3381.92", "374a4f8fc9a5ea6a": "3381.93",
                  "3093e3c72c791456": "3381.94", "5cc08ed68341dd38": "3381.95", "056f9f3c356ff96e": "3381.96"}
OwnFilterBase = 3381.81


def passFilters() -> Dict[str, str]:
    out: Dict[str, str] = {}
    own = 0
    for ps in dict.fromkeys(ps for passes in SlotPasses.values() for ps in passes):
        if (ps in ForwardFilters):
            out[ps] = ForwardFilters[ps]
        else:
            out[ps] = f"{OwnFilterBase + 0.01 * own:.2f}"
            own += 1
    if (any(v in ForwardFilters.values() for k, v in out.items() if k not in ForwardFilters)):
        raise SystemExit("a filter_index of this direction collides with one of the forward direction's")
    return out


PassFilters = passFilters()

# SanhuaExorcist's textures by the hash WWMI Tools filed them under (WWMI-Assets/PlayerCharacterData/
#   SanhuaSkin1), with the role each plays in its component's MAIN pass -- every one read off the
#   2026-09-19 dump by correlating the texture the pass binds against the asset files (1.00 on each but
#   the bangs' diffuse, 0.91 through the streamed mip's junk under the alpha, identical by eye).
#   1dcc0f1d (iris), c88cc1fc (eye mask) and 46177147 (face mask) are the same textures on both skins.
Roles = {
    "f8d5c991": "bangsDiffuse", "63c807fe": "bangsMask", "4478285f": "t5Ramp",
    "11171f1c": "hairDiffuse", "9febd992": "hairNormal",
    "46177147": "faceMask", "c98e83cd": "faceDiffuse",
    "72739d6e": "torsoNormal", "e0c15187": "torsoMask", "52f35e6d": "torsoDiffuse",
    "221b8ad6": "lowerNormal", "8e5306a9": "lowerDiffuse",
    "c88cc1fc": "eyeMask", "1dcc0f1d": "irisDiffuse",
    # the same textures under the hashes of the 2.2 / 2.4 game versions (Data/Mod Downloads/WuWa/SanhuaExorcist/
    #   SanhuaExorcistHashLineage.json): every file WWMI-Assets' SanhuaSkin1 folder ever held, extracted from its
    #   git history and correlated 1.00 with the current texture -- the pairs git's own rename detection makes
    #   in the 2.4 / 2.6 diffs. A mod exported before 2.6 (sanhua_qiming) carries these and REPAINTS most of
    #   them, so pixels cannot place its files and only the hash can
    "31a5f36a": "bangsDiffuse", "d153e37f": "bangsMask", "f22348f0": "t5Ramp",
    "9522bbc7": "hairDiffuse", "a0cf932f": "hairNormal", "464256d1": "faceDiffuse",
    "d5a089c8": "torsoNormal", "6a10c291": "torsoNormal", "168462a9": "torsoDiffuse",
    "d96aa9b8": "lowerNormal", "fd078186": "lowerDiffuse", "3cd03f60": "irisDiffuse",
    # the LIVE hashes of the game's own textures at LOD bias Ultra High (the 2026-09-20 max-LOD
    #   frame dump, each measured 1.00 against the asset file): what a mod exported from a dump today
    #   carries, where the asset repo's hashes are what WWMI Tools' own exports carry
    "af3ba241": "bangsDiffuse", "0442ed26": "bangsMask", "38074c14": "t5Ramp", "10980f87": "hairDiffuse",
    "77a480f9": "hairNormal", "bf16c0c7": "faceMask", "280300b0": "faceDiffuse", "773ed913": "torsoNormal",
    "bc4f430b": "torsoMask", "2dad4dfe": "torsoDiffuse", "83b34054": "lowerNormal", "dfd02e32": "lowerDiffuse",
    "5764478b": "irisDiffuse", "d0524bfb": "eyeMask",
}

# source component -> (target slot, {ps register: role, or a CreatedTextures role})
Plan = {
    0: (1, {"ps-t0": "bangsDiffuse", "ps-t1": "bangsMask", "ps-t5": "t5Ramp"}),   # through Sanhua's HAIR slot and shader: see Plans
    1: (1, {"ps-t0": "hairDiffuse", "ps-t1": "hairNormal", "ps-t5": "t5Ramp"}),
    2: (2, {"ps-t0": "faceMask", "ps-t1": "faceDiffuse"}),
    5: (6, {"ps-t1": "eyeMask", "ps-t2": "irisDiffuse", "ps-t5": "t5Ramp"}),   # ps-t0 is a global the game keeps (matches no asset texture)
    3: (4, {"ps-t0": "torsoNormal", "ps-t1": "torsoMask", "ps-t2": "torsoDiffuse"}),
    4: (5, {"ps-t0": "lowerNormal", "ps-t1": "ClothMask", "ps-t2": "lowerDiffuse"}),
}
# THE BANGS GO THROUGH SANHUA'S HAIR SLOT (the default since 2026-09-19; `--plan bangsSlot` is the first routing): Exorcist's bangs drawn through Sanhua's HAIR slot, with the hair shader (69e3d321), as
#   Exorcist draws her own bangs -- and Sanhua's bangs slot skipped. Sanhua draws her bangs with two shaders of
#   their own (a512f04f / f6bc3927), and on the sanhua_qiming recolour (dark-painted hair, 2026-09-19) those
#   turned the fringe BROWN while the same textures through the hair shader stayed black: her bangs shader
#   gets the hair's material block shifted 9 registers, with a warm row (0.68, 0.3, 0.2) the light-grey
#   vanilla hair of both skins would hide. Two sources on the hair slot's draw, so the bangs land in a second
#   .ini (the one-section-per-draw split). Confirmed in game on that mod: the whole head one colour. Sanhua's
#   see-through pass (f6bc3927) is not run for them.
Plans = {
    "default": Plan,
    "bangsSlot": {**Plan, 0: (0, Plan[0][1])},     # the first routing: Sanhua's own bangs slot and shaders (the brown fringe)
}
Labels = {0: "bangs", 1: "hair", 2: "face", 3: "torso, arms, ribbons", 4: "hair bun, trousers", 5: "eyes"}
TargetLabels = {0: "bangs", 1: "hair", 2: "face", 3: "arm skin", 4: "bodice, hat, ribbons, boots", 5: "skirt", 6: "eyes"}

# The WWMI command lists every component section runs, by the value of its `run =` line. The texture
#   command list is added right after the shared-resource override and before the first draw.
OverrideSharedResources = "CommandListOverrideSharedResources"

_alive: List[object] = []     # Python-built edits, classifiers and resources the C++ side holds only by reference


# ---------------------------------------------------------------------------------------------
# the library

def characterFromLibrary(modType):
    """Everything the fix needs to know about one WWMI character, read out of its ModType's tables
    (the same shape WWMI-Assets' Metadata.json has). A missing row is an error: a zero here would
    look like a real value."""
    name = modType.name

    def hashOf(kind):
        value = modType.hashes.get([name, kind], None, False)
        if (value is None):
            raise SystemExit(f"the library has no '{kind}' hash for {name}")
        return value

    components = []
    while (True):
        slot = f"{SlotPrefix}{len(components)}"
        first = modType.indices.get([name, "", slot], None, False)
        if (first is None):
            break
        count, vgOffset, vgCount = modType.getIndexCount(slot), modType.getVGOffset(slot), modType.getVGCount(slot)
        if (count is None or vgOffset is None or vgCount is None):
            raise SystemExit(f"the library has {name}'s {slot} match_first_index but not all of its match_index_count / vg_offset / vg_count")
        components.append({"index_offset": int(first), "index_count": int(count), "vg_offset": int(vgOffset), "vg_count": int(vgCount)})
    if (not components):
        raise SystemExit(f"the library has no draw slots (Indices rows typed {SlotPrefix}N) for {name}")

    checksum = modType.getShapeKeyChecksum(ShapeKeyType)
    vertexCount = modType.getVertexCount()
    if (checksum is None or vertexCount is None):
        raise SystemExit(f"the library has no shape-key checksum or vertex count for {name}")

    return {"name": name, "vb0_hash": hashOf("vb0"), "cb4_hash": hashOf("cb4"), "vertex_count": int(vertexCount),
            "index_count": components[-1]["index_offset"] + components[-1]["index_count"], "components": components,
            "shapekeys": {"offsets_hash": hashOf("shapekey_offsets"), "scale_hash": hashOf("shapekey_scale"), "checksum": int(checksum)}}


def wwmiBlendElements():
    """The 8-byte WWMI blend line: four R8 bone indices then four R8 weights (Metadata.json's
    export_format 'Blend'; the API's default BlendFile layout is GIMI's 32-byte one)"""
    uint8 = FRB.BufUnSignedInt(name = "UnsignedInt8", size = 1)
    return [FRB.BufElementType(FRB.BufElementNames.BlendIndices.value, "R8G8B8A8_UINT", [uint8] * 4),
            FRB.BufElementType(FRB.BufElementNames.BlendWeight.value, "R8G8B8A8_UINT", [uint8] * 4)]


def writeSolidDds(path, colour, size = 16):
    """An uncompressed R8G8B8A8_UNORM DDS of one colour"""
    w = h = size
    header = bytearray(b"DDS ")
    flags = 0x1 | 0x2 | 0x4 | 0x1000 | 0x8      # caps, height, width, pixelformat, pitch
    header += struct.pack("<IIIIIII", 124, flags, h, w, w * 4, 0, 1)
    header += b"\0" * 44                          # reserved
    header += struct.pack("<II4sIIIII", 32, 0x4, b"DX10", 0, 0, 0, 0, 0)   # pixel format: fourcc DX10
    header += struct.pack("<IIIII", 0x1000, 0, 0, 0, 0)                   # caps
    header += struct.pack("<IIIII", 28, 3, 0, 1, 0)                        # DX10: R8G8B8A8_UNORM, texture2d, 1 array
    body = bytes(colour) * (w * h)
    with open(path, "wb") as f:
        f.write(bytes(header) + body)


# ---------------------------------------------------------------------------------------------
# the parser

BoneDataObjs = {"cb4": ("", "boneData")}
ShapeKeyObjs = {"shapekey_offsets": ("", "shapekeyOffsets"), "shapekey_scale": ("", "shapekeyScale")}


def slotObjs(sourceType) -> List[Tuple[str, str]]:
    return [("", f"{SlotPrefix}{i}") for i in range(len(characterFromLibrary(sourceType)["components"]))]


def hashOnlyObjs(shapeKeys: bool) -> Dict[str, Tuple[str, str]]:
    """The objects identified by a hash alone: the bone-data override always, the shape-key overrides
    only when the run retargets them"""
    return {**BoneDataObjs, **(ShapeKeyObjs if shapeKeys else {})}


def makeParser(sourceType, shapeKeys: bool = False):
    """
    A GIMIParser whose sections are sorted by the API's hash / index classifier -- the draw slots by the
    vb0 hash plus their match_first_index, the bone-data and shape-key overrides by a hash of their own.
    The texture overrides carry hashes the library does not know and stay unclassified: they keep
    serving the mod on the source and never fire while the target is drawn.
    """
    slots = slotObjs(sourceType)

    def factory(iniFile, modTypeId):
        # The version is passed explicitly: a reverse lookup with no version resolves `0` (component 0's
        # index) through GI's 6.1 bucket, which holds no WuWa row
        version = iniFile.fromVersion if (iniFile.fromVersion is not None) else WuWaVersion
        hashOnly = hashOnlyObjs(shapeKeys)
        classifier = FRB.GIMISectionClassifier(dict(hashOnly), sourceType.hashes, {"vb0": {obj: obj for obj in slots}}, sourceType.indices, version)
        classifier.hashNonVersionVals = {"name": SourceName}
        classifier.indexNonVersionVals = {"name": SourceName}
        parser = FRB.GIMIParser(iniFile, modObjs = slots + list(hashOnly.values()), objTargetFuncs = [classifier], modTypeId = modTypeId)
        parser.trackKeys = True
        parser.keysToTrack = {"hash", "match_first_index", "match_index_count"}
        _alive.append(classifier)
        return parser
    return factory


# ---------------------------------------------------------------------------------------------
# the mod's own files, read off the .ini the API parsed

SectionPattern = re.compile(r"^\[(?P<name>[^\]]+)\]\s*$")


def iniSections(text: str) -> Dict[str, List[str]]:
    """{section name: [lines]} of an .ini's text, comments included"""
    sections: Dict[str, List[str]] = {}
    current = None
    for line in text.splitlines():
        match = SectionPattern.match(line)
        if (match):
            current = match.group("name")
            sections[current] = []
        elif (current is not None):
            sections[current].append(line)
    return sections


def keyValue(line):
    stripped = line.strip()
    if (not stripped or stripped.startswith(";") or "=" not in stripped):
        return None, None
    key, _, value = stripped.partition("=")
    return key.strip(), value.strip()


# ---------------------------------------------------------------------------------------------
# the mod's textures, by role

ServiceRoot: Optional[str] = None       # the folder the run was pointed at: a mod's textures may be declared in any .ini of it
AssetsFolder = os.path.join(Repo, "Data", "Mod Downloads", "WuWa", SourceName, WuWaVersion.replace(".", "_"))

# A planned role the mod has NO file for is bound to the SOURCE's own game texture (the mod's UVs are the
# source's). Role -> the source's hash; the prototype copies it out of AssetsFolder under the name the
# API would download it as. Roles whose hash BOTH skins bind (eyeMask c88cc1fc, faceMask 46177147,
# irisDiffuse 1dcc0f1d) need none: the target's texture IS the source's.
FallbackTextures: Dict[str, str] = {
    "bangsDiffuse": "f8d5c991", "bangsMask": "63c807fe", "t5Ramp": "4478285f",
    "hairDiffuse": "11171f1c", "hairNormal": "9febd992", "faceDiffuse": "c98e83cd",
    "torsoNormal": "72739d6e", "torsoMask": "e0c15187", "torsoDiffuse": "52f35e6d",
    "lowerNormal": "221b8ad6", "lowerDiffuse": "8e5306a9",
}
IdentityMin, IdentityGap = 0.97, 0.90   # a file IS a game texture when its colour correlates >= IdentityMin with one asset and < IdentityGap with every other
ComponentTagPattern = re.compile(r"^components?[\s_-]*([\d-]+)\s+t=[0-9a-f]{8}\.dds$", re.IGNORECASE)   # WWMI Tools' export name
ComponentFilePattern = re.compile(r"component[\s_-]*(\d+)[\s_-]+([a-z]+)\.dds$", re.IGNORECASE)
TypeOfSuffix = {"diffuse": "diffuse", "albedo": "diffuse", "base": "diffuse", "color": "diffuse", "colour": "diffuse", "d": "diffuse",
                "lm": "mask", "lightmap": "mask", "mask": "mask", "m": "mask", "nm": "normal", "normal": "normal", "normalmap": "normal", "n": "normal"}
TypeRoles = {0: {"diffuse": "bangsDiffuse", "mask": "bangsMask"},
             1: {"diffuse": "hairDiffuse", "mask": "hairNormal", "normal": "hairNormal"},     # 9febd992 reads as a mask by its pixels, like Sanhua's cef6494f
             2: {"diffuse": "faceDiffuse", "mask": "faceMask"},
             3: {"diffuse": "torsoDiffuse", "mask": "torsoMask", "normal": "torsoNormal"},
             4: {"diffuse": "lowerDiffuse", "normal": "lowerNormal"},
             5: {"diffuse": "irisDiffuse", "mask": "eyeMask"}}


class TextureIndex():
    """Every .dds under the run's root with the role it plays, and every .ini's resource sections -> files"""

    def __init__(self, root: str):
        self.root = root
        self.resourcesByIni: Dict[str, Dict[str, str]] = {}      # .ini abs path -> resource section -> file abs path
        self.roleOf: Dict[str, List[Tuple[str, str]]] = {}       # file abs path -> [(role, how it was decided)]: EVERY role its hashes name
        self.unresolved: List[str] = []
        self.real: Dict[str, str] = {}                           # matching key (case-folded abs path) -> the file's real spelling
        hashesOfFile: Dict[str, List[str]] = {}
        ddsFiles: List[str] = []
        remapFix = FRB.IniKeywords.RemapFix.value.lower()
        for folder, dirs, names in os.walk(root):
            dirs[:] = sorted(d for d in dirs if (not d.upper().startswith("DISABLED")))
            for name in sorted(names):
                path = os.path.normcase(os.path.abspath(os.path.join(folder, name)))
                low = name.lower()
                if (low.endswith(".dds") and FRB.IniKeywords.RemapTex.value.lower() not in low and FRB.IniKeywords.RemapDL.value.lower() not in low):
                    ddsFiles.append(path)
                    self.real[path] = os.path.abspath(os.path.join(folder, name))
                elif (low.endswith(".ini") and not name.upper().startswith("DISABLED") and remapFix not in low):
                    with open(path, "r", encoding = "utf-8", errors = "replace") as f:
                        sections = iniSections(f.read())
                    resources: Dict[str, str] = {}
                    for sec, lines in sections.items():
                        if (sec.startswith("Resource") and remapFix not in sec.lower()):
                            fileName = next((v for k, v in map(keyValue, lines) if k == "filename"), "")
                            if (fileName.lower().endswith(".dds")):
                                resources[sec] = os.path.normcase(os.path.abspath(os.path.join(folder, fileName.replace("\\", "/"))))
                    self.resourcesByIni[path] = resources
                    for sec, lines in sections.items():
                        if (sec.startswith("TextureOverrideTexture")):
                            kvps = [keyValue(line) for line in lines]
                            h = next((v.lower() for k, v in kvps if k == "hash"), None)
                            res = next((v for k, v in kvps if k == "this"), None)
                            if (h and res in resources):
                                hashesOfFile.setdefault(resources[res], []).append(h)
        counts = {"hash": 0, "pixels": 0, "name": 0}
        pending: List[str] = []
        for f in ddsFiles:
            match = re.search(r"t=([0-9a-fA-F]{8})\.dds$", f)
            hashes = hashesOfFile.get(f, []) + ([match.group(1).lower()] if (match) else [])
            # a file plays EVERY role its hashes name (one atlas serving two components)
            roles: List[Tuple[str, str]] = []
            for h in hashes:
                if (h in Roles and Roles[h] not in [r for r, _ in roles]):
                    roles.append((Roles[h], f"hash {h}"))
            if (roles):
                self.roleOf[f] = roles; counts["hash"] += 1
            else:
                pending.append(f)
        for f, h, score in self._identify(pending):
            self.roleOf[f] = [(Roles[h], f"the game's own {h} by its pixels ({score:.2f})")]; counts["pixels"] += 1
        for f in pending:
            if (f in self.roleOf):
                continue
            match = ComponentFilePattern.search(os.path.basename(f))
            role = TypeRoles.get(int(match.group(1)), {}).get(TypeOfSuffix.get(match.group(2).lower(), "")) if (match) else None
            if (role):
                self.roleOf[f] = [(role, "its name")]; counts["name"] += 1
            else:
                self.unresolved.append(f)
        print(f"  textures under {os.path.basename(root)}: {len(ddsFiles)} files -- {counts['hash']} placed by hash, {counts['pixels']} by pixel identity with a game texture, "
              f"{counts['name']} by their Component<N>_<Type> name, {len(self.unresolved)} with no role")

    def _identify(self, files: List[str]):
        """(file, asset hash, colour correlation) for every file that is one of the game's own textures under another name"""
        if (not files or not os.path.isdir(AssetsFolder)):
            return
        sys.path.insert(0, os.path.join(Repo, "Tools", "Misc", "Diagnostics"))
        import wwmiTextureFix as texFix
        cache: Dict[str, object] = {}
        assets = {h: texFix.decode(p, cache) for h, p in texFix.currentTextures(AssetsFolder).items() if h in Roles}
        for f in files:
            x = texFix.decode(f, cache)
            if (x is None):
                continue
            scores = sorted(((texFix.corr(x[..., :3], y[..., :3]), h) for h, y in assets.items() if y is not None), reverse = True)
            if (scores and scores[0][0] >= IdentityMin and (len(scores) == 1 or scores[1][0] < IdentityGap)):
                yield f, scores[0][1], scores[0][0]


_textureIndex: Optional[TextureIndex] = None


def textureIndex() -> TextureIndex:
    global _textureIndex
    if (_textureIndex is None or _textureIndex.root != ServiceRoot):
        _textureIndex = TextureIndex(ServiceRoot)
    return _textureIndex


class ModFiles():
    """
    One mod's textures by role and its draws per source component, read off the .ini file the API
    parsed and the run's texture index: a role is bound through the .ini's own resource section when it
    has one for that file, else through a resource the fix declares
    """
    def __init__(self, ini, parser, source):
        self.sections = iniSections(ini.fileTxt)
        self.iniFolder = ini.folder
        self.present: List[int] = []         # source components the mod has a section for
        self.draws: Dict[int, int] = {}      # source component -> its number of drawindexed / custom-shader draws
        self.drawRanges: Dict[int, List[Tuple[int, int]]] = {}   # source component -> its (index count, first index) draws
        targets = parser._sectionTargets
        for i in range(len(source["components"])):
            names = targets.get(("", f"{SlotPrefix}{i}")) or []
            if (not names):
                continue
            self.present.append(i)
            self.draws[i] = sum(1 for name in names for line in self.sections.get(name, [])
                                if (keyValue(line)[0] == "drawindexed" or (keyValue(line)[0] == "run" and keyValue(line)[1].startswith("CustomShader"))))
            self.drawRanges[i] = [tuple(int(n) for n in v.split(",")[:2]) for name in names for k, v in map(keyValue, self.sections.get(name, []))
                                  if (k == "drawindexed" and len(v.split(",")) >= 2 and v.split(",")[0].strip().isdigit())]

        # WWMI's ALPHA-era shape (`required_wwmi_version` 0.7, "WWMI ALPHA-2 INI"): the mod's sections carry
        # no `$\WWMIv1\vg_offset` and the mod has no merged skeleton at all, because back then each draw
        # could only use the bones the GAME hands it for that component -- so the mod's blend indices are
        # that component's LOCAL ones, not the merged skeleton's. See makeFixer's legacy branch
        self.legacy = not any("vg_offset" in line for name in (n for i in self.present for n in (targets.get(("", f"{SlotPrefix}{i}")) or []))
                              for line in self.sections.get(name, []))
        self.indexFile = next((v for k, v in map(keyValue, self.sections.get("ResourceIndexBuffer", [])) if k == "filename"), "Meshes/Index.buf")

        index = textureIndex()
        iniPath = os.path.normcase(os.path.abspath(ini.file))
        iniFolder = os.path.dirname(iniPath)
        resourceOfFile: Dict[str, str] = {}
        for res, f in index.resourcesByIni.get(iniPath, {}).items():
            resourceOfFile.setdefault(f, res)
        real = lambda f: index.real.get(f, f)       # noqa: E731 -- the file's real spelling, for what gets written
        self.textureFolder = next((os.path.dirname(os.path.relpath(real(f), iniFolder)).replace("\\", "/") for f in resourceOfFile), "") or "Textures"
        self.resourceOfRole: Dict[str, str] = {}
        self.declared: Dict[str, str] = {}          # declared resource name -> the file's path relative to this .ini, for a file no resource of this .ini names
        self.unknownTextures: List[str] = [os.path.relpath(f, index.root) for f in index.unresolved]
        byRole: Dict[str, List[Tuple[str, str]]] = {}      # role -> [(file, how)], every role of every file
        for f, roles in index.roleOf.items():
            for role, how in roles:
                byRole.setdefault(role, []).append((f, how))

        self._byRole = byRole
        self._resourceOfFile = resourceOfFile
        self._real = real
        self._iniFolder = iniFolder
        self._root = index.root
        self._chosen: Dict[Tuple[str, int], str] = {}       # (role, source component) -> the file bound
        self._declaredName: Dict[str, str] = {}             # file -> the RemapRef resource the fix declares for it

    def _rank(self, f: str, component: int):
        """How well file 'f' serves source 'component': first the SPECIFICITY of its WWMI-Tools
        `Components-<a>-<b>... t=<hash>` tag for that component (a tag naming only this component, then a
        tag listing it among others, then none), then whether this .ini already has a resource for it, then
        its distance from the .ini. A hash override binds ONE file for a hash wherever it is drawn, so an
        exporter that writes the same hash per component-set leaves the per-component art unbound in the
        mod's own .ini -- sanhua_qiming's `Components-0 t=d153e37f.dds` (the bangs' own mask, in the mod's
        atlas layout) against its bound `Components-0-1-2-3-4 t=d153e37f.dds` (the game's shared mask, in
        the GAME's layout, which put wrong-coloured patches on the front of the hair on Sanhua, 2026-09-19).
        A register binding is per component and can honour the specific one"""
        match = ComponentTagPattern.match(os.path.basename(f))
        tag = [int(c) for c in match.group(1).split("-") if c] if (match) else []
        specificity = 0 if (tag == [component]) else (1 if (component in tag) else 2)
        rel = os.path.relpath(f, self._iniFolder).replace("\\", "/")
        return (specificity, 0 if (f in self._resourceOfFile) else 1, rel.count("../"), len(rel))

    def hasRole(self, role: str) -> bool:
        return bool(self._byRole.get(role)) or (role in self.resourceOfRole)

    def resourceOf(self, role: str, component: int) -> Optional[str]:
        """The resource this .ini binds for 'role' on source 'component' (None: the mod has no file for it)"""
        cands = self._byRole.get(role)
        if (not cands):
            return self.resourceOfRole.get(role)       # a fallback download, or nothing
        ranked = sorted(cands, key = lambda c: self._rank(c[0], component))
        best = ranked[0][0]
        key = (role, component)
        if (key not in self._chosen):
            self._chosen[key] = best
            if (len(ranked) > 1 and self._rank(ranked[1][0], component)[:3] == self._rank(best, component)[:3]):
                print(f"    WARNING: {os.path.relpath(ranked[1][0], self._root)} also has the role {role} ({ranked[1][1]}), "
                      f"already taken by {os.path.relpath(best, self._root)} ({ranked[0][1]}); the first one is bound")
        if (best in self._resourceOfFile):
            return self._resourceOfFile[best]
        if (best not in self._declaredName):
            names = set(self._declaredName.values())
            name = f"Resource{role[0].upper()}{role[1:]}{TargetName}{FRB.IniKeywords.RemapRef.value}"
            n = 1
            while (name in names):
                n += 1
                name = f"Resource{role[0].upper()}{role[1:]}{n}{TargetName}{FRB.IniKeywords.RemapRef.value}"
            self._declaredName[best] = name
            self.declared[name] = os.path.relpath(self._real(best), self._iniFolder).replace("/", "\\")
        return self._declaredName[best]

    def fallbacks(self, roles: List[str]) -> List[str]:
        """The resource sections for the planned 'roles' this .ini has no file for: the source's own game
        texture, copied out of AssetsFolder into the texture folder as <Source><Role>RemapDL.dds (the name the
        API downloads it under); binds them"""
        out: List[str] = []
        for role in roles:
            if (self.hasRole(role) or role in CreatedTextures or role not in FallbackTextures):
                continue
            src = os.path.join(AssetsFolder, f"{SourceName}Texture{FallbackTextures[role]}.dds")
            if (not os.path.isfile(src)):
                print(f"    WARNING: no {os.path.basename(src)} under {AssetsFolder} for the {role} the mod lacks")
                continue
            fileName = f"{SourceName}{role[0].upper()}{role[1:]}{FRB.IniKeywords.RemapDL.value}.dds"
            rel = f"{self.textureFolder}/{fileName}"
            os.makedirs(os.path.join(self.iniFolder, self.textureFolder), exist_ok = True)
            shutil.copyfile(src, os.path.join(self.iniFolder, self.textureFolder, fileName))
            name = f"Resource{SourceName}{role[0].upper()}{role[1:]}{FRB.IniKeywords.RemapDL.value}"
            self.resourceOfRole[role] = name
            out.append("\n".join([f"[{name}]", f"filename = {rel}", ""]))
        return out

    def declare(self) -> List[str]:
        """The resource sections for the files resourceOf chose that none of this .ini's own resources name.
        RemapRef, not RemapFix: the API's undo deletes every file a RemapFix section in the block names"""
        return ["\n".join([f"[{name}]", f"filename = {rel}", ""]) for name, rel in self.declared.items()]


# ---------------------------------------------------------------------------------------------
# the fixer

class WWMIBlendReplace(FRB.RemapBlendReplace):
    """RemapBlendReplace whose bytes this script supplies: the API's BlendFile over the 8-byte WWMI
    layout, through the library's VGRemaps row. The built resource is kept referenced from Python."""

    def buildResModel(self, *args, **kwargs):
        resource = super().buildResModel(*args, **kwargs)
        if (resource is not None):
            _alive.append(resource)
        return resource


def sourceVgMaps() -> List[Dict[int, int]]:
    """Each SOURCE component's `vg_map`: its own bone indices -> the merged skeleton's, out of the
    download folder's Metadata.json (the library carries vg_offset / vg_count but not the map)"""
    path = os.path.join(AssetsFolder, f"{SourceName}Metadata.json")
    if (not os.path.isfile(path)):
        raise ValueError(f"a legacy (alpha-era) mod needs {SourceName}'s vg_map, and {path} is not there")
    with open(path, "r", encoding = "utf-8") as f:
        meta = json.load(f)
    maps = [{int(k): int(v) for k, v in (c.get("vg_map") or {}).items()} for c in meta["components"]]
    if (not all(maps)):
        raise ValueError(f"{path} has a component with no vg_map")
    return maps


def remapLegacyBlend(vgRemap, files: "ModFiles", iniFolder: str):
    """
    A RemapBlendResource fixFunc for an ALPHA-era mod, whose blend indices are each component's OWN
    (see ModFiles.legacy): a vertex's index is read as local to the component that DRAWS it, lifted into
    the source's merged skeleton through that component's `vg_map`, and only then sent through the
    library's row. Every vertex of such a mod would otherwise be re-indexed as if it were already
    merged, which is what turned a body into a noodle mess in game (SanhuaExorcist3, 2026-09-19).
    """
    def fix(resource) -> bool:
        maps = sourceVgMaps()
        with open(os.path.join(iniFolder, files.indexFile.replace("\\", "/")), "rb") as f:
            indices = f.read()
        with open(resource.srcPath, "rb") as f:
            blend = bytearray(f.read())
        vertexCount = len(blend) // 8
        componentOf = bytearray(b"\xff") * vertexCount
        for component, ranges in files.drawRanges.items():
            for count, start in ranges:
                for k in range(start, start + count):
                    v = int.from_bytes(indices[k * 4:k * 4 + 4], "little")
                    if (v < vertexCount):
                        componentOf[v] = component
        remap = dict(vgRemap.remap) if (hasattr(vgRemap, "remap")) else dict(vgRemap)
        remap = {int(k): int(v) for k, v in remap.items()}
        unmapped, noComponent = set(), 0
        for v in range(vertexCount):
            component = componentOf[v]
            if (component == 0xFF):
                noComponent += 1
                continue
            vgMap = maps[component]
            for b in range(4):
                at = v * 8 + b
                if (blend[v * 8 + 4 + b] == 0):
                    continue                       # a weight-zero slot: the library leaves those alone too
                merged = vgMap.get(blend[at])
                target = remap.get(merged) if (merged is not None) else None
                if (target is None):
                    unmapped.add((component, blend[at]))
                    continue
                blend[at] = target
        with open(resource.fixedPath, "wb") as f:
            f.write(bytes(blend))
        print(f"    the mod is ALPHA-era: {vertexCount} vertices' blend read as PER-COMPONENT indices, lifted through {SourceName}'s vg_map, then remapped"
              + (f" ({len(unmapped)} (component, bone) pairs had no row, left as they were)" if (unmapped) else "")
              + (f"; {noComponent} vertices are drawn by nothing" if (noComponent) else ""))
        return True
    return fix


def remapWWMIBlend(vgRemap, forced: bool):
    """A RemapBlendResource fixFunc: Blend.buf -> the resource's fixed path, indices through 'vgRemap'
    (the library's row the resource carries, unless 'forced' says the script's table wins)"""
    def fix(resource) -> bool:
        remap = vgRemap if (forced or getattr(resource, "vgRemap", None) is None) else resource.vgRemap
        FRB.BlendFile(resource.srcPath, wwmiBlendElements()).remap(remap, fixedBlendFile = resource.fixedPath)
        return True
    return fix


def effectiveRemap(sourceType, target, remapOverride: Optional[Dict[int, int]]):
    """The vertex group remap this run writes the blend with: the library's row, or --vgRemap's table;
    checked to cover every group the library knows and to point inside the target's merged skeleton"""
    library = sourceType.getVGRemap(TargetName)
    if (library is None):
        raise SystemExit(f"the library has no vertex group remap {SourceName} -> {TargetName}")
    base = {int(k): int(v) for k, v in dict(library.remap).items()}
    remap = dict(remapOverride) if (remapOverride is not None) else base
    missing = sorted(set(base) - set(remap))
    if (missing):
        raise SystemExit(f"--vgRemap has no row for source groups the library's row maps: {missing}")
    targetSlots = int(target["components"][-1]["vg_offset"]) + int(target["components"][-1]["vg_count"])
    if (max(remap.values()) >= targetSlots):
        raise SystemExit(f"the remap points outside {TargetName}'s {targetSlots} merged slots")
    changed = sum(1 for g in base if remap.get(g) != base[g])
    forced = (remapOverride is not None)
    print(f"  vertex group remap: {'--vgRemap' if forced else 'the library row'} ({len(remap)} rows, {changed} differ from the library row)")
    return FRB.VGRemap(remap), forced


def makeFixer(sourceType, targetType, remapOverride: Optional[Dict[int, int]] = None, shapeKeys: bool = False, planName: str = "default"):
    plan = Plans[planName]
    source, target = characterFromLibrary(sourceType), characterFromLibrary(targetType)
    vgRemap, forcedRemap = effectiveRemap(sourceType, target, remapOverride)
    naming = FRB.CppIniNamingTools

    def factory(parser, toModName: str, modTypeId: int):
        ini = parser._iniFile
        modType = FRB.ModTypeIdTools.getModType(modTypeId)
        files = ModFiles(ini, parser, source)
        if (not files.present):
            raise ValueError(f"no [TextureOverrideComponent*] section on {SourceName}'s hash {source['vb0_hash']}")
        print(f"  {os.path.relpath(ini.file, ini.folder) if ini.folder else ini.file}: {SourceName} components {files.present} -> {toModName}")

        def fixName(name: str) -> str:
            return naming.getRemapFixName(name, toModName)

        createdResource = {role: fixName(f"Resource{role}") for role in CreatedTextures}     # a 3dmigoto resource section's name starts with Resource

        def resourceOf(role: str, component: int) -> Optional[str]:
            return createdResource[role] if (role in CreatedTextures) else files.resourceOf(role, component)

        fallbacks = files.fallbacks([role for i in files.present if (i in plan) for role in plan[i][1].values()])
        for i in files.present:                   # choose every component's files, so the ones to declare are known
            for role in (plan[i][1].values() if (i in plan) else []):
                resourceOf(role, i)
        appended: List[str] = files.declare() + fallbacks     # the textures this .ini has no resource of its own for, then the downloads
        # ---- the zero shape-key offset stream, unless WWMI's own pipeline is retargeted to fill vb6 ----
        zeroResource = None
        if (not shapeKeys):
            constants = {k: v for k, v in map(keyValue, files.sections.get("Constants", [])) if k}
            vertexCount = int(constants.get("global $mesh_vertex_count", 0))
            if (vertexCount <= 0):
                raise ValueError("no `global $mesh_vertex_count` in [Constants], so the zero shape-key stream cannot be sized")
            blendFile = next((v for k, v in map(keyValue, files.sections.get("ResourceBlendBuffer", [])) if k == "filename"), "Meshes/Blend.buf")
            zeroFile = os.path.join(os.path.dirname(blendFile.replace("\\", "/")), f"{toModName}{FRB.IniKeywords.Remap.value}{ShapeKeyZero}.buf").replace("\\", "/")
            os.makedirs(os.path.dirname(os.path.join(ini.folder, zeroFile)) or ini.folder, exist_ok = True)
            with open(os.path.join(ini.folder, zeroFile), "wb") as f:
                f.write(bytes(vertexCount * ShapeKeyStride))
            zeroResource = fixName(f"Resource{ShapeKeyZero}")
            appended.append("\n".join([f"[{zeroResource}]", "type = Buffer", "format = DXGI_FORMAT_R32G32B32_FLOAT", f"stride = {ShapeKeyStride}", f"filename = {zeroFile}", ""]))
            print(f"    vb6 (the game's shape-key offsets, applied by vertex id): every remapped draw binds {vertexCount} zero offsets instead")

        # ---- the edits shared by every object: the target's hashes, the target's checksum, the names ----
        hashRemap = FRB.RegAssetRemap({"hash": (modType.hashes, FRB.IniKeywords.HashNotFound.value),
                                       "$\\WWMIv1\\shapekey_checksum": (modType.shapeKeyChecksums, "ChecksumNotFound")},
                                      toModName, SourceName, ini.fromVersion, ini.toVersion)
        rename = FRB.GraphRename(fixName)
        perObj: Dict[Tuple[str, str], List[object]] = {obj: [hashRemap, rename] for obj in hashOnlyObjs(shapeKeys).values()}
        print(f"  shape keys: {'retargeted to ' + toModName if shapeKeys else 'not retargeted (the mod' + chr(39) + 's own sections are hidden or left alone per --shapeKeys)'}")

        # ---- an ALPHA-era mod has no merged skeleton, so the fix supplies one (ModFiles.legacy) ----
        # WWMI grew the merged skeleton in its beta: before it, a draw could only address the bones the
        # game hands it for that component, so such a mod's sections carry no vg_offset / vg_count and no
        # merge at all. Retargeted onto a skin whose slots hold different bones, that cannot work -- so the
        # blend is lifted into merged space (remapLegacyBlend) and these sections do what a beta mod's own
        # text does: mark the game's bone-data constant buffer, merge each window into a skeleton buffer of
        # the fix's own, and bind it in place of the game's.
        mergeList: Dict[int, str] = {}
        if (files.legacy):
            merged, mergedRW = fixName("ResourceMergedSkeleton"), fixName("ResourceMergedSkeletonRW")
            extra, extraRW = fixName("ResourceExtraMergedSkeleton"), fixName("ResourceExtraMergedSkeletonRW")
            for name in (merged, extra):
                appended.append(NL.join([f"[{name}]", ""]))
            for name in (mergedRW, extraRW):
                appended.append(NL.join([f"[{name}]", "type = RWBuffer", "format = R32G32B32A32_FLOAT", f"array = {MergedSkeletonSlots}", ""]))
            if (not any(name.startswith("TextureOverrideMarkBoneDataCB") for name in files.sections)):
                appended.append(NL.join([f"[{fixName('TextureOverrideMarkBoneDataCB')}]", f"hash = {target['cb4_hash']}",
                                         "match_priority = 0", f"filter_index = {BoneDataFilter}", ""]))
            for slot, c in enumerate(target["components"]):
                name = fixName(f"CommandListMergeSlot{slot}")
                mergeList[slot] = name
                lines = [f"[{name}]"]
                for cb, rw, ro in (("vs-cb4", mergedRW, merged), ("vs-cb3", extraRW, extra)):
                    # only when the bone data itself is bound: another pass of the same draw has something
                    # else in that slot, and merging THAT would fill the skeleton with junk
                    lines += [f"if {cb} == {BoneDataFilter}",
                              f"    {WWMI}vg_offset = {c['vg_offset']}", f"    {WWMI}vg_count = {c['vg_count']}",
                              f"    {WWMI}custom_mesh_scale = 1.00",
                              f"    cs-cb8 = ref {cb}", f"    cs-u6 = {rw}", f"    run = {SkeletonMerger}",
                              f"    {ro} = copy {rw}", f"    {cb} = {ro}", "endif"]
                appended.append(NL.join(lines + [""]))
            print(f"    the mod is ALPHA-era (no merged skeleton of its own): the fix adds one, "
                  f"{len(target['components'])} slot merge lists on the bone data {target['cb4_hash']}")

        # ---- the draw slots: retargeted, and handed their texture command list ----
        dropped = [i for i in files.present if (i not in plan)]
        if (dropped):
            print(f"    not drawn: components {dropped} ({', '.join(Labels.get(i, str(i)) for i in dropped)})")
        for i in files.present:
            if (i not in plan):
                perObj[("", f"{SlotPrefix}{i}")] = []
                continue
            slot, regs = plan[i]
            c = target["components"][slot]
            edits: List[object] = []
            bindings = [f"    {reg} = {resourceOf(role, i)}" for reg, role in regs.items() if (resourceOf(role, i) is not None)]
            additions: List[Tuple[str, str]] = [("run", mergeList[slot])] if (slot in mergeList) else []
            additions += [("vb6", zeroResource)] if (zeroResource) else []
            if (bindings):
                cmdList = fixName(f"CommandList{SourceName}{SlotPrefix.capitalize()}{i}Textures")
                condition = " || ".join(f"ps == {PassFilters[ps]}" for ps in SlotPasses[slot])
                appended.append("\n".join([f"[{cmdList}]", f"if {condition}"] + bindings + ["endif", ""]))
                additions.append(("run", cmdList))
            if (additions):
                # what every remapped section of this component must end up with, for the check that runs
                # over the written file (ensureAdditions): a graph edit that finds no position adds nothing
                # and says nothing, and this component then draws with no vb6 zero stream and no textures
                RequiredAdditions[f"TextureOverride{SlotPrefix}{i}{toModName}{FRB.IniKeywords.RemapFix.value}".lower()] = list(additions)
                # right after the shared-resource override, the EARLIEST spot after it; no after-register
                # (behind a `$draw_x` toggle no draw is certain -- sanhuaExorcistFix.py's header, point 14)
                # the anchor is matched under BOTH names: GraphGroupRemap renames as it copies, so a section
                # in a later group (the copies a collision makes) meets the renamed value here
                anchors = {OverrideSharedResources, fixName(OverrideSharedResources)}
                edits.append(FRB.RegSurroundedAdd(additions,
                                                  beforeRegs = {"run": lambda v: v in anchors},
                                                  latest = False))
            retarget = {"match_first_index": str(c["index_offset"]), "match_index_count": str(c["index_count"]),
                        f"{WWMI}vg_offset": str(c["vg_offset"]), f"{WWMI}vg_count": str(c["vg_count"])}
            # and what the written file must then say, for ensureValues: a RegNewVals that does not reach a
            # line leaves the SOURCE's number there, which for vg_offset / vg_count merges the target's bone
            # data into the wrong window of the merged skeleton -- the component drawn on wrong bones
            RequiredValues[f"TextureOverride{SlotPrefix}{i}{toModName}{FRB.IniKeywords.RemapFix.value}".lower()] = dict(retarget)
            edits.append(FRB.RegNewVals(retarget))
            edits += [hashRemap, rename]
            perObj[("", f"{SlotPrefix}{i}")] = edits

        graphEdits: List[object] = [FRB.GraphGroupEdit([perObj])]
        if (dropped):
            graphEdits.append(FRB.GraphRemove([(0, "", f"{SlotPrefix}{i}") for i in dropped]))

        # ---- the blend: the register bound in the shared override, collected into a RemapBlend ----
        graphEdits.append(FRB.ResRegCollect({(0, "", f"{SlotPrefix}{i}"): "vb4" for i in files.present if (i in plan)},
                                            {"blend": WWMIBlendReplace((0, "", "blend"), resType = "blend",
                                                                        fixFunc = (remapLegacyBlend(vgRemap, files, ini.folder) if (files.legacy)
                                                                                   else remapWWMIBlend(vgRemap, forcedRemap)))}))

        # ---- the target slots nothing is drawn through: skipped, and their bones still merged ----
        drawnSlots = {plan[i][0] for i in files.present if (i in plan)}
        for slot, c in enumerate(target["components"]):
            if (slot in drawnSlots):
                continue
            name = f"TextureOverride{toModName}{SlotPrefix.capitalize()}{slot}{FRB.IniKeywords.Remap.value}Hide"
            appended.append("\n".join([
                f"; nothing of the mod is drawn through {toModName}'s {TargetLabels.get(slot, slot)} slot: the skin's own geometry is skipped and its bones still merged",
                f"[{name}]", f"hash = {target['vb0_hash']}", f"match_first_index = {c['index_offset']}", f"match_index_count = {c['index_count']}",
                "$object_detected = 1", "if $mod_enabled"]
                + ([f"    run = {mergeList[slot]}", "    handling = skip"] if (slot in mergeList) else
                   [f"    local $state_id_{slot}", f"    if $state_id_{slot} != $state_id", f"        $state_id_{slot} = $state_id",
                    f"        $\\WWMIv1\\vg_offset = {c['vg_offset']}", f"        $\\WWMIv1\\vg_count = {c['vg_count']}",
                    f"        run = {fixName('CommandListMergeSkeleton')}", "    endif",
                    "    if ResourceMergedSkeleton !== null", "        handling = skip", "    endif"])
                + ["endif", ""]))

        # ---- the shader tags the texture command lists ask about, and the invented textures ----
        for i, (ps, filterIndex) in enumerate(PassFilters.items()):
            appended.append("\n".join([f"[{fixName(f'ShaderOverridePass{i}')}]", f"hash = {ps}", f"filter_index = {filterIndex}", ""]))
        usedCreated = [role for role in CreatedTextures if any(r == role for i in files.present if (i in plan) for r in plan[i][1].values())]
        for role in usedCreated:
            texFile = os.path.join(files.textureFolder, f"{role}{toModName}{FRB.IniKeywords.RemapTex.value}.dds").replace("\\", "/")
            os.makedirs(os.path.join(ini.folder, files.textureFolder), exist_ok = True)
            writeSolidDds(os.path.join(ini.folder, texFile), CreatedTextures[role])
            appended.append("\n".join([f"[{createdResource[role]}]", f"filename = {texFile}", ""]))

        _alive.extend(graphEdits)
        fixer = FRB.GIMIFixer(parser, graphGroupEdits = graphEdits, modsToFix = [toModName])
        fixer.appendedSections = "\n".join(appended)
        _alive.append(fixer)

        # ---- report: the per-slot table ----
        for i in files.present:
            if (i not in plan):
                continue
            slot, regs = plan[i]
            bound = ", ".join(f"{reg}={resourceOf(role, i) or 'GAME (mod has none)'}" for reg, role in regs.items())
            print(f"    slot {slot} {TargetLabels[slot]:<28} <- {i} {Labels.get(i, str(i)):<22} {files.draws[i]} draws  {bound}")
        for slot in range(len(target["components"])):
            if (slot not in drawnSlots):
                print(f"    slot {slot} {TargetLabels[slot]:<28} <- (nothing; the skin's own geometry skipped)")
        if (files.unknownTextures):
            print(f"    textures with no role (left to the mod's own hash overrides): {len(files.unknownTextures)}")
            for u in files.unknownTextures:
                print(f"      {u}")
        return fixer
    return factory


# ---------------------------------------------------------------------------------------------
# run

ShapeKeySections = ["TextureOverrideShapeKeyOffsets", "TextureOverrideShapeKeyScale", "CommandListSetupShapeKeys", "CommandListLoadShapeKeys",
                    "TextureOverrideShapeKeyLoaderCallback", "CommandListMultiplyShapeKeys", "TextureOverrideShapeKeyMultiplierCallback"]
HideMarker = FRB.IniKeywords.HideOriginalComment.value


def hideOriginalSections(iniPath: str, matches) -> int:
    """Comments out, with the API's HideOrig marker (so an undo restores them), every section of the mod's
    OWN text -- everything before the fix block -- whose name 'matches' accepts. Returns the count."""
    with open(iniPath, "r", encoding = "utf-8", newline = "") as f:
        text = f.read()
    ending = "\r\n" if ("\r\n" in text) else "\n"
    lines = text.split(ending)
    header = next((k for k, line in enumerate(lines) if line.startswith("; ---") and "Remap" in line), len(lines))
    hidden, inside = 0, False
    for k in range(header):
        line = lines[k]
        match = SectionPattern.match(line)
        if (match):
            inside = matches(match.group("name")) and not line.startswith(HideMarker)
            hidden += int(inside)
        if (inside and line.strip() and not line.startswith(HideMarker)):
            lines[k] = HideMarker + line
        if (not line.strip()):
            inside = False
    with open(iniPath, "w", encoding = "utf-8", newline = "") as f:
        f.write(ending.join(lines))
    return hidden


RequiredAdditions: Dict[str, List[Tuple[str, str]]] = {}     # remapped section name (lowercased) -> the lines it must carry
RequiredValues: Dict[str, Dict[str, str]] = {}               # remapped section name (lowercased) -> the values it must hold


def ensureValues(iniPath: str) -> List[str]:
    """
    Checks the WRITTEN file: every remapped slot section must carry the TARGET's window and vertex-group
    window, and rewrites any the fixer's `RegNewVals` did not reach.

    The same malformed section that defeats the added lines (see ensureAdditions) defeats the rewritten
    ones: SanhuaExorcist4's torso kept `vg_offset` / `vg_count` 22 / 105, the SOURCE's, so its section
    merged the target's slot-4 bone data into merged slots 22..126 instead of 51..122 and every torso
    vertex read a bone that was never written -- the body a mess in game, while `match_first_index`
    (a line at the section's top level, which the edit did reach) was right (2026-09-19).
    """
    with open(iniPath, "r", encoding = "utf-8", newline = "") as f:
        text = f.read()
    ending = "\r\n" if ("\r\n" in text) else "\n"
    lines = text.split(ending)
    section: Optional[Dict[str, str]] = None
    name = ""
    fixed: List[str] = []
    for k, line in enumerate(lines):
        match = SectionPattern.match(line)
        if (match):
            name = match.group("name")
            section = RequiredValues.get(name.lower())
            continue
        if (section is None):
            continue
        key, value = keyValue(line)
        want = section.get(key) if (key is not None) else None
        if (want is not None and value != want):
            lines[k] = line.replace(f"= {value}", f"= {want}") if (f"= {value}" in line) else f"{line[:len(line) - len(line.lstrip())]}{key} = {want}"
            fixed.append(f"{name}: {key} {value} -> {want}")
    if (fixed):
        with open(iniPath, "w", encoding = "utf-8", newline = "") as f:
            f.write(ending.join(lines))
    return fixed


def ensureAdditions(iniPath: str) -> List[str]:
    """
    Checks the WRITTEN file: every remapped slot section must carry the lines the fixer meant to add
    after the shared-resource override (the merge list, the `vb6` zero stream, the texture command
    list), and adds the ones a graph edit did not place. Returns what it repaired.

    `RegSurroundedAdd` adds nothing at all when it can find no position, and says nothing about it.
    SanhuaExorcist4's author ends the torso's section with `run = CustomShader1` and puts the closing
    `endif`s inside THAT section, so the section's `if` blocks do not balance -- and for that one
    section the edit silently added nothing, leaving the torso drawn with the game's shape-key offsets
    (the body a mess in game) and none of the mod's textures, while every other component was fine
    (2026-09-19). Balancing the section by hand makes the edit work, so this is about the shape of the
    mod's text, not about the mod's content.
    """
    with open(iniPath, "r", encoding = "utf-8", newline = "") as f:
        text = f.read()
    ending = "\r\n" if ("\r\n" in text) else "\n"
    lines = text.split(ending)
    # the file's sections as [first line, last line exclusive)
    bounds: List[Tuple[str, int, int]] = []
    for k, line in enumerate(lines):
        match = SectionPattern.match(line)
        if (match):
            if (bounds):
                bounds[-1] = (bounds[-1][0], bounds[-1][1], k)
            bounds.append((match.group("name").lower(), k, len(lines)))
    inserts: Dict[int, List[str]] = {}
    repaired: List[str] = []
    for name, start, end in bounds:
        wanted = RequiredAdditions.get(name)
        if (not wanted):
            continue
        body = [keyValue(line) for line in lines[start:end]]
        missing = [(k, v) for k, v in wanted if (k, v) not in body]
        if (not missing):
            continue
        anchor = next((k for k in range(start, end)
                       if (keyValue(lines[k])[0] == "run" and OverrideSharedResources.lower() in (keyValue(lines[k])[1] or "").lower())), None)
        if (anchor is None):
            print(f"    WARNING: {name} has no `run = ...{OverrideSharedResources}` line to add {[k for k, _ in missing]} after; left alone")
            continue
        indent = lines[anchor][:len(lines[anchor]) - len(lines[anchor].lstrip())] or "\t\t"
        inserts[anchor] = [f"{indent}{k} = {v}" for k, v in missing]
        repaired += [f"{name}: {k} = {v}" for k, v in missing]
    if (not repaired):
        return []
    out: List[str] = []
    for k, line in enumerate(lines):
        out.append(line)
        out += inserts.get(k, [])
    with open(iniPath, "w", encoding = "utf-8", newline = "") as f:
        f.write(ending.join(out))
    return repaired


def ensureBlendBinding(iniPath: str, blendReg: str = "vb4") -> List[str]:
    """
    Checks the WRITTEN file: inside the fix block every copy of the shared-resource override must bind
    the REMAPPED blend, and rewrites the ones that do not.

    The same malformed section that defeats the additions (see ensureAdditions) also made the fixer
    write a SECOND copy of `CommandListOverrideSharedResources` under the same name -- and only the
    first had its `vb4` collected into the RemapBlend, so the second still bound the mod's own blend,
    in the SOURCE's bone space (SanhuaExorcist4, 2026-09-19). Two sections of one name is 3dmigoto's
    choice to resolve; making every copy bind the same buffer takes the choice out of it.
    """
    with open(iniPath, "r", encoding = "utf-8", newline = "") as f:
        text = f.read()
    ending = "\r\n" if ("\r\n" in text) else "\n"
    lines = text.split(ending)
    start = next((k for k, line in enumerate(lines) if (line.startswith("; ---") and FRB.IniKeywords.Remap.value in line)), None)
    if (start is None):
        return []
    remapped = next((v for k, v in map(keyValue, lines[start:]) if (k == blendReg and v and FRB.IniKeywords.Remap.value in v)), None)
    if (remapped is None):
        return []
    fixed: List[str] = []
    for k in range(start, len(lines)):
        key, value = keyValue(lines[k])
        if (key == blendReg and value != remapped):
            lines[k] = lines[k].replace(value, remapped)
            fixed.append(f"{value} -> {remapped}")
    if (fixed):
        with open(iniPath, "w", encoding = "utf-8", newline = "") as f:
            f.write(ending.join(lines))
    return fixed


def duplicateSections(iniPath: str) -> List[str]:
    """The section names the fix block declares more than once -- always worth saying out loud"""
    with open(iniPath, "r", encoding = "utf-8", newline = "") as f:
        lines = f.read().splitlines()
    start = next((k for k, line in enumerate(lines) if (line.startswith("; ---") and FRB.IniKeywords.Remap.value in line)), len(lines))
    names = [m.group("name") for m in map(SectionPattern.match, lines[start:]) if m]
    return sorted({name for name in names if names.count(name) > 1})


SlotSectionPattern = re.compile(rf"^TextureOverride{SlotPrefix}\d+{TargetName}{FRB.IniKeywords.RemapFix.value}$", re.IGNORECASE)
DrawKeys = {"drawindexed", "vb6"}


def isDrawLine(line: str) -> bool:
    """A line of a remapped slot section that DRAWS or serves a draw: the draw itself, the zero stream,
    the texture command list, a custom-shader draw"""
    key, value = keyValue(line)
    return key in DrawKeys or (key == "run" and (value.startswith(f"CommandList{SourceName}") or value.startswith("CustomShader")))


def skipOnlyBlock(name: str, block: List[str], indent: str = "\t") -> List[str]:
    """
    A remapped slot section cut down to what a file that does NOT draw that slot still owes: merging
    the slot's bones, and skipping the skin's own geometry. REBUILT from the section's own numbers
    rather than filtered line by line.

    Filtering was wrong in a way that only a mod with an odd shape shows. A skip-only copy kept the
    mod's `run = CommandListOverrideSharedResources` -- which binds the mod's `ib` and `vb0`-`vb4` --
    and relied on the section's own `run = CommandListCleanupSharedResources` to put the game's `vb0`
    back. SanhuaExorcist4 keeps that cleanup call inside the `[CustomShader1]` its section calls, and
    the filter drops a `run = CustomShader...` as a draw line: so the copy bound the mod's buffers and
    never restored them, and every later draw of the frame read the MOD's vertex buffer. The body was
    a mess until the extra file was disabled (2026-09-20). A section that draws nothing has no reason
    to bind anything, so the rebuilt block binds nothing at all.
    """
    def value(key: str) -> Optional[str]:
        return next((v for k, v in map(keyValue, block) if k == key), None)

    merge = next((v for k, v in map(keyValue, block) if (k == "run" and v and "Merge" in v)), None)
    state = next((line.strip().split()[-1] for line in block if line.strip().startswith("local $state_id")), None)
    out = [f"[{name}]"]
    for key in ("hash", "match_first_index", "match_index_count"):
        if (value(key) is not None):
            out.append(f"{key} = {value(key)}")
    if (value("$object_detected") is not None):
        out.append("$object_detected = 1")
    out.append("if $mod_enabled")
    offset, count = value(f"{WWMI}vg_offset"), value(f"{WWMI}vg_count")
    if (merge and offset is not None and state):        # the mod's own merge, guarded once a frame
        out += [f"{indent}local {state}", f"{indent}if {state} != $state_id", f"{indent * 2}{state} = $state_id",
                f"{indent * 2}{WWMI}vg_offset = {offset}", f"{indent * 2}{WWMI}vg_count = {count}",
                f"{indent * 2}run = {merge}", f"{indent}endif",
                f"{indent}if ResourceMergedSkeleton !== null", f"{indent * 2}handling = skip", f"{indent}endif"]
    elif (merge):                                        # the fix's own per-slot merge list (a legacy mod)
        out += [f"{indent}run = {merge}", f"{indent}handling = skip"]
    else:
        out += [f"{indent}handling = skip"]
    return out + ["endif", ""]


def splitSlotFiles(iniPath: str) -> List[str]:
    """One remapped section per target window per file (sanhuaExorcistFix.py's header, point 11). With
    this direction's plan no two sources share a target slot, so this only fires for a mod whose .ini
    matches one component in several sections. Returns the extra files written."""
    with open(iniPath, "r", encoding = "utf-8", newline = "") as f:
        text = f.read()
    ending = "\r\n" if ("\r\n" in text) else "\n"
    lines = text.split(ending)
    header = next((k for k, line in enumerate(lines) if line.startswith("; ---") and "Remap" in line), None)
    if (header is None):
        return []
    sections = []
    k = header
    while (k < len(lines)):
        match = SectionPattern.match(lines[k])
        if (match):
            end = k + 1
            while (end < len(lines) and not SectionPattern.match(lines[end]) and not lines[end].startswith("; ----")):
                end += 1
            sections.append((match.group("name"), k, end))
            k = end
        else:
            k += 1
    windows: Dict[str, List[Tuple[str, int, int]]] = {}
    for name, start, end in sections:
        if (SlotSectionPattern.match(name)):
            first = next((v for key, v in map(keyValue, lines[start:end]) if key == "match_first_index"), None)
            windows.setdefault(first, []).append((name, start, end))
    extras = [(name, start, end) for group in windows.values() for name, start, end in group[1:]]
    if (not extras):
        return []

    def skipOnly(name: str, block: List[str]) -> List[str]:
        return skipOnlyBlock(name, block)

    stem, ext = os.path.splitext(iniPath)
    written = []
    for n, (own, ownStart, ownEnd) in enumerate(extras, 1):
        out = []
        inside = False
        for line in lines[:header]:
            match = SectionPattern.match(line)
            if (match):
                inside = match.group("name").startswith("TextureOverride") and not line.startswith(HideMarker)
            if (not line.strip()):
                inside = False
            out.append((HideMarker + line) if (inside and line.strip() and not line.startswith(HideMarker)) else line)
        k = header
        while (k < len(lines)):
            sec = next(((name, s, e) for name, s, e in sections if s == k), None)
            if (sec is None):
                out.append(lines[k]); k += 1; continue
            name, s, e = sec
            block = lines[s:e]
            if (SlotSectionPattern.match(name) and name != own):
                block = skipOnly(name, block)
            out.extend(block)
            k = e
        extra = f"{stem}{FRB.IniKeywords.RemapFix.value}{n}{ext}"
        with open(extra, "w", encoding = "utf-8", newline = "") as f:
            f.write(ending.join(out))
        written.append(extra)
    drop = {start for _, start, _ in extras}
    kept = []
    k = 0
    while (k < len(lines)):
        sec = next(((name, s, e) for name, s, e in sections if s == k and s in drop), None)
        if (sec is None):
            kept.append(lines[k]); k += 1
        else:
            k = sec[2]
    with open(iniPath, "w", encoding = "utf-8", newline = "") as f:
        f.write(ending.join(kept))
    return written


def removeSplitFiles(folder: str) -> int:
    """Deletes the extra .ini files a previous split wrote (the API's undo does not know them)"""
    removed = 0
    for root, _, names in os.walk(folder):
        for name in names:
            if (re.search(rf"(?:{TargetName})?{FRB.IniKeywords.RemapFix.value}\d+\.ini$", name, re.IGNORECASE)):
                os.remove(os.path.join(root, name)); removed += 1
    return removed


def runService(folder: str, args) -> None:
    """The whole run through RemapService: folder walk, undo of a previous fix, backups, resources, summary"""
    removed = removeSplitFiles(folder)
    if (removed):
        print(f"  removed {removed} extra .ini file(s) of a previous split")
    service = FRB.RemapService(path = folder, keepBackups = args.keepBackups, hideOrig = args.hideOrig, undoOnly = args.undo,
                               logger = FRB.Logger() if args.verbose else None)
    service.fix()
    stats = service.stats
    if (not args.undo):
        for path in stats.ini.fixed:
            retargeted = ensureValues(path)
            if (retargeted):
                print(f"  {os.path.relpath(path, folder)}: {len(retargeted)} value(s) the graph edit could not rewrite, corrected to {TargetName}'s: " + ", ".join(retargeted))
            rebound = ensureBlendBinding(path)
            if (rebound):
                print(f"  {os.path.relpath(path, folder)}: {len(rebound)} copy of the shared-resource override bound the mod's own blend, rebound to the remapped one ({', '.join(rebound)})")
            duplicates = duplicateSections(path)
            if (duplicates):
                print(f"  {os.path.relpath(path, folder)}: WARNING: the fix block declares {len(duplicates)} section name(s) more than once: {', '.join(duplicates)}")
            repaired = ensureAdditions(path)
            if (repaired):
                print(f"  {os.path.relpath(path, folder)}: {len(repaired)} line(s) the graph edit could not place, added after the shared-resource override "
                      f"(a section whose `if` blocks do not balance -- see ensureAdditions): " + ", ".join(repaired))
            if (args.shapeKeys == "hide"):
                n = hideOriginalSections(path, lambda name: name in ShapeKeySections)
                print(f"  {os.path.relpath(path, folder)}: {n} shape-key sections of the mod's own commented out (--shapeKeys leave keeps them)")
            if (args.hideTextureOverrides):
                n = hideOriginalSections(path, lambda name: name.startswith("TextureOverrideTexture"))
                print(f"  {os.path.relpath(path, folder)}: {n} [TextureOverrideTexture] sections commented out")
            if (not args.noSplit):
                extras = splitSlotFiles(path)
                if (extras):
                    print(f"  {os.path.relpath(path, folder)}: {len(extras)} further section(s) on an already-taken draw moved into their own file(s): "
                          + ", ".join(os.path.basename(e) for e in extras))
    print(f"\n.ini fixed: {len(stats.ini.fixed)}, skipped: {len(stats.ini.skipped)}")
    for path in sorted(stats.ini.fixed):
        print(f"  {os.path.relpath(path, folder)}")
    for path, error in stats.ini.skipped.items():
        print(f"  SKIPPED {os.path.relpath(path, folder)}: {error}")
    for label in ("blend", "position", "texcoord", "buf", "texEdit", "texAdd"):
        s = getattr(stats, label)
        if (not s.fixed and not s.skipped):
            continue
        print(f"{label}: fixed {len(s.fixed)}, skipped {len(s.skipped)}")
        for path in sorted(s.fixed):
            print(f"  {os.path.relpath(path, folder)}")
        for path, error in s.skipped.items():
            print(f"  SKIPPED {os.path.relpath(path, folder)}: {error}")


def main():
    parser = argparse.ArgumentParser(description = f"{SourceName} -> {TargetName}, in place, through the API's parser, fixer and RemapService")
    parser.add_argument("mod", help = f"the mod folder (every {SourceName} .ini under it is fixed in place; DISABLED ones are skipped)")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--hideOrig", action = "store_true", help = f"comment out the mod's own {SourceName} sections, so the mod renders on {TargetName} only")
    parser.add_argument("--undo", action = "store_true", help = "remove a previous fix instead of fixing")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--shapeKeys", choices = ["hide", "leave", "retarget"], default = "hide",
                        help = f"the mod's shape-key sections: 'hide' (default) comments them out, 'leave' keeps them on {SourceName}'s own hashes, 'retarget' copies them onto {TargetName}'s buffer")
    parser.add_argument("--hideTextureOverrides", action = "store_true",
                        help = "comment the mod's own [TextureOverrideTexture] sections out too")
    parser.add_argument("--noSplit", action = "store_true",
                        help = f"keep every remapped section in mod.ini (default: one section per target draw per file, the rest in <stem>{TargetName}RemapFix<n>.ini)")
    parser.add_argument("--plan", choices = list(Plans), default = "default",
                        help = f"which source component goes through which target slot: 'default', Exorcist's bangs through {TargetName}'s hair slot and shader, as {SourceName} draws them, {TargetName}'s bangs slot skipped; or 'bangsSlot' (the bangs through {TargetName}'s own bangs slot and shaders, which turns a dark fringe brown)")
    parser.add_argument("--vgRemap", default = None, metavar = "JSON",
                        help = "a {source group: target group} table to write the blend with instead of the library's VGRemaps row")
    args = parser.parse_args()
    remapOverride = None
    if (args.vgRemap):
        with open(winToPosix(args.vgRemap), "r", encoding = "utf-8") as f:
            remapOverride = {int(k): int(v) for k, v in json.load(f).items()}
    folder = os.path.abspath(winToPosix(args.mod))
    if (not os.path.isdir(folder)):
        raise SystemExit(f"not a folder: {folder}")
    global ServiceRoot
    ServiceRoot = folder

    sourceType, targetType = FRB.WWMIBuilder.sanhuaExorcist(), FRB.WWMIBuilder.sanhua()
    FRB.CppStrategyOverrides.clear()
    retarget = (args.shapeKeys == "retarget")
    FRB.CppStrategyOverrides.setParser(SourceName, makeParser(sourceType, retarget))
    FRB.CppStrategyOverrides.setFixer(SourceName, TargetName, makeFixer(sourceType, targetType, remapOverride, retarget, args.plan))
    try:
        runService(folder, args)
    finally:
        FRB.CppStrategyOverrides.clear()


if (__name__ == "__main__"):
    main()

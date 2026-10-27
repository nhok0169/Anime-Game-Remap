#
# ===== miyabiBlossomFix =====
#
# PROTOTYPE of the Miyabi -> MiyabiBlossom fix (ZZZ 3.2) -- pipeline step 5, the first ZZZ remap. Built
# from the library's own parser and edits through CppStrategyOverrides, the hand-built route
# (overrideScript2.py / yelanTranquilFixPerBuffer.py), and run in place through RemapService the way the
# compiled fix will be: the fix block goes into the mod's own .ini, the undo and the backups are the API's.
#
#   py -3 miyabiBlossomFix.py <mod folder> [<mod folder> ...]          fix in place
#   py -3 miyabiBlossomFix.py <mod folder> --copy <scratch folder>      fix a copy, leave the mod alone
#   py -3 miyabiBlossomFix.py <mod folder> --undo                       remove the fix
#
# THE SHAPE. A ZZMI character is GIMI-shaped, but every component skins from its OWN bone palette, and
# both Miyabi and the skin are several components. Data/RemapDrafts/MiyabiRemapDraft.xlsx routes each
# source component WHOLE to one target component, so each pair below is an ordinary one-mesh remap:
#
#     Miyabi Body (her outfit) -> skin Dress     Miyabi Legs (bare limbs) -> skin Body
#     Hair -> Hair, HairShadow -> HairShadow     Face: the SAME mesh and hashes on both -- left alone
#
# Per pair: the component's sections are copied (GraphGroupRemap keeps only that component's graphs in
# that pair's fixer), renamed (GraphRename), re-pointed at the target's hashes (RegAssetRemap, filtered
# to the source component's rows) and objects (RegNewVals on match_first_index), its Blend.buf rebuilt
# through the vertex group row (RemapBlendReplace), and -- Body -> Dress only -- its Texcoord.buf cut
# from 24 to 20 bytes a vertex. ZZZ has no NNFix / ORFix.
#
# TEXTURES. Hair and HairShadow draw the SAME textures on both outfits; Body -> Dress and Legs -> Body do
# not, and most real mods recolour by texture HASH (`[TextureOverride...] hash = <Miyabi's diffuse> /
# this = ...`), which never fires on the skin. So, for those two pairs:
#   - every such texture-hash section is copied onto the TARGET's texture hash of the same role (toggles
#     and all: it is the same section with another hash), like the mesh sections;
#   - a role the mod does not override by hash gets the SOURCE's own game texture on the target's hash
#     (the mod's UVs are the source's), copied from the download folder after the fix (GAP 6);
#   - and every light map (bound in the section, by hash, or the fallback) gets the BAND EDIT below.
# The normal map (ebac056e) is one texture for every object of both outfits, so it needs nothing.
#
# THE BAND EDIT. A light map's RED channel is a material band (0 / 77 / 128 / 179 / 255), and the band
# picks a pair of shading tints out of the draw's ps-cb3 (rows 39-43 lit, 44-48 shade, band order). That
# table is per OUTFIT: Miyabi's Body bands are cool blues, the skin's Dress bands warm, and band 0 is the
# skin's SKIN tone -- so her navy hakama (band 0) rendered salmon. Each source band is moved to the target
# band whose tint pair is nearest (Euclidean over lit + shade RGB, from the two 3.2 frame dumps; keep a band
# when it ties). Texture-only, so approximate: the other per-outfit cb3 rows (rim, specular) stay the skin's.
#
# GAPS -- what the library does not do yet, and where this script fills in (each marked GAP below):
#   1. no template is multi-component -> multi-component (each pair is a one-mesh remap here)
#   2. the blend edit looks its row up under the fixer's TARGET name, a component id here
#      (MiyabiBlossomDress) while the rows are keyed by skin + component (MiyabiBlossom, Dress) -- the
#      GI component template passes its targetSkin explicitly; this script files copies of the rows
#      under the component ids, in its own process only
#   3. the slot indices (Hair B 15132 -> 14916) live in a private Indices table: filing a "0" in the
#      shared IndexData would capture other characters' reverse lookups (Creating Remaps, Yelan)
#   4. no texcoord stride edit outside the GI templates' configs: a fixFunc writes the 20-byte file
#   5. GIMIParser re-points every GIMISectionClassifier at the classified mod type's tables, so a classifier
#      with a table of its own (the slot indices) is handed over wrapped in a plain function
#   6. no fallback texture for a role a recolour does not cover outside the GI templates: the sections go in
#      through GIMIFixer.appendedSections and the files are copied from the download folder after the run
#   7. MaterialBandRemapFilter edits the ALPHA channel (GI's band); ZZZ's band is RED, so the edit is a
#      TexEditor filter of this script's own
#   8. ResRegCollect does not strip a binding's `ref ` prefix (IniNamingTools::removeRefPrefix), which every
#      ZZMI `Resource\ZZMI\<Role> = ref ...` line carries
#   9. TexReplace names an edited file after its source file only, so one file edited two ways collides
#  10. a light map bound through a resource VARIABLE in a command list every component shares (Miyabi9)
#      gets no band edit: the port needs a per-component copy of the list, or a band edit of the variable
#  11. a buffer an ib section BORROWS from another component (a Body texcoord in the Legs section) is not
#      converted by the component that owns it; the texcoord is cut here by its declared stride
#  12. the library's names append the tag, which reorders sections of one hash and first index; 3DMigoto runs
#      them in name order
#  13. a VertexLimitRaise copy keeps the source's override_byte_stride
#  14. a copy keeps the source object's match_index_count
#  15. the sword's textures differ per outfit (its ibs do not); its recolours are carried by text (runOne)
#  16. a mesh drawn from two components' sections shows the later-drawn slot, and the two characters order
#      their slots differently; the Body -> Dress copy drops an ib the Legs sections also draw
#  17. the diffuse is scaled per light map band for the tint difference (THE TINT COMPENSATION); the light map
#      is found by a scan of the .ini, as no edit pairs one texture with another
#




import argparse
import os
import re
import shutil
import stat
import struct
import sys

import numpy as np
from PIL import Image

API = os.environ.get("AG_REMAP_REPO", os.path.join(
    "E:", os.sep, "Computer", "Games", "Genshin", "Repos", "Repos", "Fix-Raiden-Boss",
    "Anime Game Remap (for all users)", "api", "src", "py"))
sys.path.insert(0, os.path.abspath(API))
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(os.path.abspath(API), "FixRaidenBoss2"))

import FixRaidenBoss2 as FRB  # noqa: E402

SOURCE, TARGET = "Miyabi", "MiyabiBlossom"
V = "3.2"

# source component -> target component, and each component's objects (A, B, ... in first-index order)
Routes = {"Body": "Dress", "Legs": "Body", "Hair": "Hair", "HairShadow": "HairShadow"}
SourceSlots = {"Body": {"a": 0}, "Legs": {"a": 0}, "Hair": {"a": 0, "b": 15132}, "HairShadow": {"a": 0}}
TargetSlots = {"Dress": {"a": 0}, "Body": {"a": 0}, "Hair": {"a": 0, "b": 14916}, "HairShadow": {"a": 0}}
TexcoordStride = {("Miyabi", "Body"): 24, ("Miyabi", "Legs"): 20, ("Miyabi", "Hair"): 20, ("Miyabi", "HairShadow"): 24,
                  ("MiyabiBlossom", "Dress"): 20, ("MiyabiBlossom", "Body"): 20, ("MiyabiBlossom", "Hair"): 20, ("MiyabiBlossom", "HairShadow"): 24}

# the graphs a component's sections are sorted into, by the hash TYPE that names them
HashKinds = {"ib": "ib", "blend_vb": "blend", "texcoord_vb": "texcoord", "draw_vb": "vertexlimit", "position_vb": "position"}

# the pairs whose two components draw DIFFERENT textures, and the roles carried (see TEXTURES above)
TexPairs = ("Body", "Legs")
TexRoles = {"diffuse": "Diffuse", "lightmap": "LightMap", "materialmap": "MaterialMap"}

# THE BAND EDIT: source band -> target band, per source component (bands not listed keep their value)
BandLevels = (0, 77, 128, 179, 255)
BandMoves = {"Body": {0: 179, 77: 179, 128: 0, 255: 179},       # onto the skin's Dress table
             # Legs -> Body: NONE. Moving its bands by the same tint rule ({77, 128, 255} -> 179) turned Miyabi8's
             # black-and-red dress (which that mod packs into the legs / hair atlas) grey, and the mod's own light map
             # renders it as on Miyabi -- measured in game, 2026-10-27
             "Legs": {}}
DownloadFolder = os.path.join(os.path.dirname(os.path.abspath(API)), "..", "..", "..", "Data", "Mod Downloads", "ZZZ", SOURCE, "3_2")


def compObjs(comp):
    """Every graph a source component's sections are sorted into"""
    objs = list(SourceSlots[comp]) + list(HashKinds.values())
    if (comp in TexPairs):
        objs += [f"tex_{role}" for role in TexRoles]
    return objs

_alive = []      # objects whose callables the API keeps pointers into (see Creating Remaps' prototype notes)
naming = FRB.IniNamingTools
keepName = lambda name: name


def srcName(comp): return SOURCE + comp


def OBJ(comp, obj):
    """The mod object for one of a component's graphs. The ZZZ component goes into the OBJECT name and the
    mod object's own component stays "": a non-empty component means "the same object drawn more than
    once" to the graph groups, which then fold every object of that component into its ib graph"""
    return ("", f"{comp}_{obj}")
def dstName(comp): return TARGET + comp


# ============================================================================== the tables

def modTypeOf(name):
    FRB.CppGlobalModTypes.registerMissing()
    for modType in FRB.CppGlobalModTypes.all():
        if (modType.name == name):
            return modType
    raise SystemExit(f"no mod type {name}")


def slotIndices():
    """GAP 3: the slot indices, in a table of this script's own"""
    indices = FRB.Indices({srcName(s): [dstName(t)] for s, t in Routes.items()})
    rows = [([V, srcName(c), "", OBJ(c, o)[1]], str(i)) for c, objs in SourceSlots.items() for o, i in objs.items()]
    rows += [([V, dstName(c), "", OBJ(c, o)[1]], str(i)) for c, objs in TargetSlots.items() for o, i in objs.items()]
    indices.addRepoRows(rows)
    return indices


def fileRowsUnderComponentIds(modType):
    """GAP 2: the shipped rows (Miyabi, S) -> (MiyabiBlossom, T), filed again under the target COMPONENT
    id the blend edit asks with -- in this process only"""
    vg = modType.vgRemaps
    rows = []
    for s, t in Routes.items():
        row = modType.getVGRemap(TARGET, fromComp = s, toComp = t)
        if (row is None):
            raise SystemExit(f"no vertex group row Miyabi {s} -> MiyabiBlossom {t}")
        remap = dict(row.remap)
        remap.update(RowOverrides.get((s, t), {}))
        rows.append((["1.0", SOURCE, s, V, dstName(t), t], remap))
    vg.addRows(rows)


# Corrections to the shipped rows, measured in game -- each still to be carried into the RemapDraft and
# VGRemapData.cpp at the port. Miyabi Legs 44 is an upper-arm helper her own mesh barely uses (18 vertices) and
# the finder sent it to the skin's 1 with 40 / 42 / 43; a mod that hangs its shoulder pieces on it (Miyabi3)
# then had them slide to the elbow in the idle, and only the skin's SHOULDER (56) keeps them where they sit on
# Miyabi (the main upper arm, 0, droops the same way 1 does)
RowOverrides = {("Legs", "Body"): {44: 56}}


# ============================================================================== the parser

def makeParser(modType, indices):
    """One GIMISectionClassifier per source component, each filtered to that component's own hash rows --
    every component carries the same hash TYPES, so a type alone cannot say which one a section is"""
    modObjs = []
    classifiers = []
    for comp in Routes:
        hashOnly = {kind: OBJ(comp, obj) for kind, obj in HashKinds.items()}
        if (comp in TexPairs):
            # a texture-hash section (`this = ...`) of the component's own textures. Legs shares its textures
            # with Hair, and the name filter keeps that section in each component's graph
            hashOnly.update({f"tex_{o}_{role}": OBJ(comp, f"tex_{role}") for o in SourceSlots[comp] for role in TexRoles})
        byIndex = {"ib": {OBJ(comp, o): OBJ(comp, o) for o in SourceSlots[comp]}}
        modObjs += [OBJ(comp, obj) for obj in compObjs(comp)]
        classifiers.append((hashOnly, byIndex, srcName(comp)))

    def factory(iniFile, modTypeId):
        funcs = []
        for hashOnly, byIndex, name in classifiers:
            classifier = FRB.GIMISectionClassifier(dict(hashOnly), modType.hashes, byIndex, indices, None)
            classifier.hashNonVersionVals = {"name": name}
            classifier.indexNonVersionVals = {"name": name}
            _alive.append(classifier)
            # GAP 5: handed over as a PLAIN function. GIMIParser re-points every GIMISectionClassifier it
            # is given at the classified mod type's own hashes and indices (the pure-Python original's
            # behaviour), which would throw the slot-index table above away -- A and B then fall through
            # to the hash-only ib entry and every object is fixed as the shared IB section
            funcs.append(lambda parser, name, section, disjoint, part, colouring, c = classifier: c(parser, name, section, disjoint, part, colouring))
        parser = FRB.GIMIParser(iniFile, modObjs = list(modObjs), objTargetFuncs = funcs, modTypeId = modTypeId)
        parser.disjointModObjs = False
        parser.trackKeys = True
        parser.keysToTrack = {"hash", "match_first_index"}
        return parser
    return factory


# ============================================================================== the fixer

def texcoordCut(fromStride: int, toStride: int):
    """GAP 4: a Texcoord.buf rewritten at the target component's stride. ZZZ's 20-byte texcoord is
    COLOR, TEXCOORD0 (half2), TEXCOORD1 (float2), TEXCOORD2 (half2); the 24-byte one appends a TEXCOORD3
    (half2) that equals TEXCOORD0 on 93% of Miyabi's Body -- so narrowing drops it and widening copies
    TEXCOORD0 into it"""
    def fix(resource) -> bool:
        data = open(resource.srcPath, "rb").read()
        if (len(data) % fromStride):
            print(f"  ! {os.path.basename(resource.srcPath)}: {len(data)} bytes is not a multiple of {fromStride}; left as it is")
            return False
        out = bytearray()
        for i in range(0, len(data), fromStride):
            line = data[i:i + fromStride]
            out += line[:toStride] if (toStride <= fromStride) else line + line[4:8] * ((toStride - fromStride) // 4)
        with open(resource.fixedPath, "wb") as f:
            f.write(out)
        return True
    return fix


ALL = None   # a draw of the whole ib (`drawindexed = auto`)


def drawRange(value: str):
    """(start, end) of a `drawindexed = count, start, base` value, or ALL for `auto`"""
    parts = [p.strip() for p in value.split(",")]
    if (parts[0].lower() == "auto"):
        return ALL
    try:
        return (int(parts[1]), int(parts[1]) + int(parts[0]))
    except (IndexError, ValueError):
        return ALL


def rangesDrawnBy(iniPath: str, ibHash: str) -> dict:
    """GAP 16: {ib resource: [(start, end) or ALL]} that the mod's own sections on 'ibHash' draw, following each
    section's `ib =` lines in order"""
    text = open(iniPath, encoding = "utf-8", errors = "replace").read().split("; --------------- Miyabi Remap")[0]
    out = {}
    sections = []
    for m in re.finditer(r"^\[([^\]]+)\][ \t]*\r?\n(.*?)(?=^\[|\Z)", text, re.M | re.S):
        h = re.search(r"^\s*hash\s*=\s*([0-9a-fA-F]{8})", m.group(2), re.M)
        if (h and h.group(1).lower() == ibHash.lower()):
            sections.append(m)
    # a section with no `ib =` of its own (Miyabi6's `handling = skip` + `drawindexed = auto` IB section) draws
    # whatever ib the hash's other sections bound: credited to that ib when there is exactly one
    bound = {removeRef(v) for m in sections for v in re.findall(r"^\s*ib\s*=\s*(.+?)\s*$", m.group(2), re.M)} - {"null"}
    inherited = next(iter(bound)) if (len(bound) == 1) else None
    for m in sections:
        cur = inherited
        for line in m.group(2).splitlines():
            ib = re.match(r"\s*ib\s*=\s*(.+?)\s*$", line)
            draw = re.match(r"\s*drawindexed\s*=\s*(.+?)\s*$", line)
            if (ib):
                cur = removeRef(ib.group(1))
            elif (draw and cur and cur != "null"):
                out.setdefault(cur, []).append(drawRange(draw.group(1)))
    return out


def overlaps(r, ranges) -> bool:
    return any(o is ALL or r is ALL or (r[0] < o[1] and o[0] < r[1]) for o in ranges)


_targetCounts = {}


def targetIndexCount(tcomp: str, obj: str) -> int:
    """GAP 14: the target object's index count, from the target's download folder's hash.json"""
    if (not _targetCounts):
        path = os.path.join(os.path.dirname(DownloadFolder), "..", TARGET, "3_2", TARGET + "Hash.json")
        import json
        for e in json.load(open(path, encoding = "utf-8")):
            for i, count in enumerate(e["object_counts"]):
                _targetCounts[(e["component_name"], "abcdefgh"[i])] = count
    return _targetCounts[(tcomp, obj)]


def orderedName(name: str, fixed: str) -> str:
    """GAP 12: 'fixed' (the library's <name><tag>) with the tag moved in front of the original name's own part,
    so the copies of one hash sort among themselves as the originals do"""
    prefix = "TextureOverride"
    if (not fixed.startswith(name) or not name.startswith(prefix)):
        return fixed
    return prefix + fixed[len(name):] + name[len(prefix):]


def declaredStrides(iniPath: str) -> dict:
    """GAP 11: {resource section: (file, declared stride)} of the .ini, to tell a borrowed texcoord by its stride"""
    text = open(iniPath, encoding = "utf-8", errors = "replace").read()
    out = {}
    for m in re.finditer(r"^\[(Resource[^\]]*)\]\s*\n(.*?)(?=^\[|\Z)", text, re.M | re.S):
        stride = re.search(r"^\s*stride\s*=\s*(\d+)", m.group(2), re.M)
        file = re.search(r"^\s*filename\s*=\s*(.+?)\s*$", m.group(2), re.M)
        if (stride and file):
            out[m.group(1)] = (os.path.basename(file.group(1).replace("\\", "/")).lower(), int(stride.group(1)))
    return out


def texcoordCutDeclared(declared: dict, toStride: int):
    """texcoordCut from whatever stride the resource section of the file declares"""
    byFile = {file: stride for file, stride in declared.values()}

    def fix(resource) -> bool:
        stride = byFile.get(os.path.basename(resource.srcPath).lower())
        if (stride is None):
            print(f"  ! {os.path.basename(resource.srcPath)}: no declared stride; left as it is")
            return False
        return texcoordCut(stride, toStride)(resource)
    return fix


def bandEdit(comp: str):
    """GAP 7: THE BAND EDIT on a light map's red channel -- each pixel to its nearest band level, then that band
    moved per BandMoves. Every decision from the ORIGINAL red: the moves may form a permutation"""
    levels = np.array(BandLevels, np.int16)
    to = np.array([BandMoves[comp].get(int(v), int(v)) for v in BandLevels], np.uint8)

    def edit(texFile) -> None:
        pixels = np.array(texFile.img.convert("RGBA"))
        red = pixels[..., 0].astype(np.int16)
        dist = np.abs(red[..., None] - levels)
        nearest = dist.argmin(axis = -1)
        # only a texel ON a band moves (within a few levels, for compression); a mod's in-between reds -- a gradient,
        # anti-aliasing, a level of its own -- keep their value rather than snapping to a band they are not
        onBand = dist.min(axis = -1) <= BandTolerance
        moving = onBand & (to[nearest].astype(np.int16) != levels[nearest])
        pixels[..., 0] = np.where(moving, to[nearest], pixels[..., 0])
        texFile.img = Image.fromarray(pixels, "RGBA")
    return edit


BandTolerance = 8


def removeRef(value: str) -> str:
    """IniNamingTools::removeRefPrefix, which is not bound"""
    value = value.strip()
    return value[4:].strip() if (value[:4].lower() == "ref ") else value


# THE TINT COMPENSATION. A band move picks the NEAREST of the skin's five tint pairs, which for dark cloth is a
# lighter, more neutral pair (Miyabi6's teal dress came out pale, Miyabi8's black dress grey). So each diffuse
# texel is also scaled, in linear light, by its band's Miyabi tint pair over the skin tint pair the band now lands
# on. ps-cb3 rows 39-43 (lit) and 44-48 (shade) of each outfit's draw, from the two 3.2 frame dumps.
# MEASURED WORSE, so OFF (2026-10-27): on the identity mod it turned the white shirt and lapels lavender, and the
# grey / pale dark cloth it was written for was the LEGS band move's doing (now removed), not a tint ceiling.
TintCompensation = False
Tints = {
    ("Miyabi", "Body"): [(0.598, 0.643, 0.787), (0.737, 0.807, 0.848), (0.791, 0.619, 0.521), (0.590, 0.546, 0.787), (0.738, 0.830, 0.888), (0.485, 0.481, 0.692), (0.492, 0.549, 0.748), (0.604, 0.375, 0.238), (0.451, 0.420, 0.604), (0.329, 0.433, 0.604)],
    ("Miyabi", "Legs"): [(1.000, 0.911, 0.886), (0.701, 0.664, 0.890), (0.704, 0.765, 0.803), (0.791, 0.514, 0.509), (0.789, 0.657, 0.735), (0.880, 0.783, 0.783), (0.456, 0.423, 0.694), (0.408, 0.454, 0.619), (0.459, 0.257, 0.263), (0.606, 0.415, 0.491)],
    ("MiyabiBlossom", "Dress"): [(0.856, 0.725, 0.725), (0.890, 0.854, 0.792), (0.890, 0.872, 0.792), (0.791, 0.707, 0.705), (1.000, 0.911, 0.886), (0.725, 0.521, 0.521), (0.692, 0.667, 0.617), (0.692, 0.665, 0.617), (0.604, 0.604, 0.604), (0.880, 0.783, 0.783)],
    ("MiyabiBlossom", "Body"): [(1.000, 0.913, 0.888), (0.385, 0.368, 0.487), (0.890, 0.872, 0.792), (0.856, 0.725, 0.725), (1.000, 0.911, 0.886), (0.880, 0.784, 0.784), (0.134, 0.125, 0.197), (0.692, 0.665, 0.617), (0.725, 0.521, 0.521), (0.880, 0.783, 0.783)]}


def tintRatios(comp: str):
    """Per source band: Miyabi's (lit + shade) over the skin's (lit + shade) on the band it moves to"""
    src, dst = Tints[(SOURCE, comp)], Tints[(TARGET, Routes[comp])]
    out = []
    for k, level in enumerate(BandLevels):
        j = BandLevels.index(BandMoves[comp].get(level, level))
        s = np.add(src[k], src[k + 5]); d = np.add(dst[j], dst[j + 5])
        out.append(np.clip(s / np.maximum(d, 1e-3), 0.4, 2.5))
    return np.array(out, np.float32)


def tintCompensation(comp: str, lightMapOf):
    """GAP 17: scale a diffuse per band. 'lightMapOf()' names the light map whose red channel gives each texel's band"""
    ratios = tintRatios(comp)
    levels = np.array(BandLevels, np.int16)

    def edit(texFile) -> None:
        path = lightMapOf()
        if (not path or not os.path.isfile(path)):
            print(f"      ! {comp}: no light map to read the bands from; diffuse left as it is")
            return
        lm = FRB.TextureFile(path, readPillowImg = True)
        lm.open()
        img = texFile.img.convert("RGBA")
        red = np.asarray(lm.img.convert("RGBA").resize(img.size, Image.NEAREST))[..., 0].astype(np.int16)
        band = np.abs(red[..., None] - levels).argmin(axis = -1)
        px = np.asarray(img).astype(np.float32) / 255.0
        rgb = px[..., :3]
        lin = np.where(rgb <= 0.04045, rgb / 12.92, ((rgb + 0.055) / 1.055) ** 2.4) * ratios[band]
        lin = np.clip(lin, 0.0, 1.0)
        px[..., :3] = np.where(lin <= 0.0031308, lin * 12.92, 1.055 * lin ** (1 / 2.4) - 0.055)
        texFile.img = Image.fromarray((px * 255.0 + 0.5).astype(np.uint8), "RGBA")
    return edit


def resourceFiles(iniPath: str) -> dict:
    """{resource section: file path} of the .ini"""
    text = open(iniPath, encoding = "utf-8", errors = "replace").read()
    out = {}
    for m in re.finditer(r"^\[(Resource[^\]]*)\]\s*\n(.*?)(?=^\[|\Z)", text, re.M | re.S):
        file = re.search(r"^\s*filename\s*=\s*(.+?)\s*$", m.group(2), re.M)
        if (file):
            out[m.group(1)] = os.path.join(os.path.dirname(iniPath), file.group(1).replace("\\", os.sep))
    return out


def lightMapFor(iniPath: str, comp: str):
    """GAP 17: the light map the mod binds for 'comp' (by hash, then in its ib sections), else the game's"""
    def find():
        text = open(iniPath, encoding = "utf-8", errors = "replace").read().split("; --------------- Miyabi Remap")[0]
        files = resourceFiles(iniPath)
        lmHash = texHash(SOURCE, comp, "lightmap").lower()
        ibHash = modTypeOf(SOURCE).hashes.get([srcName(comp), "ib"], V).lower()
        candidates = []
        for m in re.finditer(r"^\[([^\]]+)\][ \t]*\r?\n(.*?)(?=^\[|\Z)", text, re.M | re.S):
            h = re.search(r"^\s*hash\s*=\s*([0-9a-fA-F]{8})", m.group(2), re.M)
            if (not h):
                continue
            if (h.group(1).lower() == lmHash):
                candidates += re.findall(r"^\s*this\s*=\s*(\S+)", m.group(2), re.M)
            elif (h.group(1).lower() == ibHash):
                candidates += re.findall(r"^\s*(?:Resource\\ZZMI\\LightMap|ps-t5)\s*=\s*(?:ref\s+)?(\S+)", m.group(2), re.M | re.I)
        for name in candidates:
            if (name in files and os.path.isfile(files[name])):
                return files[name]
        return os.path.join(DownloadFolder, f"{srcName(comp)}ALightMap.dds")
    return find


_written = set()


def taggedFile(path: str, tag: str) -> str:
    """GAP 9: TexReplace names the edited file after the SOURCE file alone (<file>RemapTex.dds), so one light map
    two components share (Miyabi1's single atlas) would get both components' band edits written to ONE file.
    The target component goes into the name, here and in the resource section (see makeFixer)"""
    head, sep, tail = path.rpartition("RemapTex")
    return f"{head}{tag}{sep}{tail}" if sep else path


def texReplace(resModObj, kind: str, filterFunc, tag: str) -> "FRB.TexReplace":
    """A texture edit through the API's TexEditor, written once per output file -- uncompressed, because BC7
    would move red values off their band"""
    def fix(resource) -> bool:
        dest = taggedFile(resource.fixedPath, tag)
        if (dest in _written):
            return True
        _written.add(dest)
        editor = FRB.TexEditor([filterFunc], readPillowImg = True, compress = False, mipmaps = True)
        editor.fix(FRB.TextureFile(resource.srcPath, readPillowImg = True), dest)
        return True
    edit = FRB.TexReplace(resModObj, FRB.TexEditor([], compress = False), fixFunc = fix, resSubType = kind)
    _alive.append(edit)
    return edit


def texHash(name: str, comp: str, role: str) -> str:
    return modTypeOf(name).hashes.get([name + comp, f"tex_a_{role}"], V)


def recolouredHashes(iniPath: str) -> set:
    """The texture hashes some `this =` section of the mod's .ini files overrides (the folder of 'iniPath' and
    below, as readAllInis reads them; DISABLED files skipped)"""
    found = set()
    for root, _, files in os.walk(os.path.dirname(iniPath)):
        for f in files:
            if (not f.lower().endswith(".ini") or f.upper().startswith("DISABLED")):
                continue
            text = open(os.path.join(root, f), encoding = "utf-8", errors = "replace").read()
            for m in re.finditer(r"^\[([^\]]+)\][^\[]*?^\s*hash\s*=\s*([0-9a-fA-F]{8})(.*?)(?=^\[|\Z)", text, re.M | re.S):
                if ("Remap" not in m.group(1) and re.search(r"^\s*this\s*=", m.group(3), re.M)):
                    found.add(m.group(2).lower())
    return found


_fallbacks = []      # (source download, file to write, band edit or None), written after the run (GAP 6)


def fallbackSections(ini, comp: str, toName: str) -> str:
    """GAP 6: a role the mod recolours by no hash gets the source's game texture on the target's hash"""
    recoloured = recolouredHashes(ini.file)
    text = []
    for role, Role in TexRoles.items():
        srcHash = texHash(SOURCE, comp, role)
        name = fallbackName(ini.file, toName, Role)
        file = os.path.join(os.path.dirname(ini.file), name + ".dds")
        moves = bool(BandMoves.get(comp))
        edit = (bandEdit(comp) if (role == "lightmap") else tintCompensation(comp, lightMapFor(ini.file, comp)) if (role == "diffuse" and TintCompensation) else None) if moves else None
        _fallbacks.append((os.path.join(DownloadFolder, f"{srcName(comp)}A{Role}.dds"), file, edit))
        text.append(f"[Resource{name}]\nfilename = {name}.dds\n")
        if (srcHash.lower() in recoloured):
            # the mod recolours it, maybe only under a toggle: its carried copy gets this as its DEFAULT
            # (defaultRecolours), so a toggle that is off does not leave the skin's own texture at Miyabi's UVs
            _defaults[(os.path.normcase(ini.file), texHash(TARGET, Routes[comp], role).lower())] = f"Resource{name}"
            continue
        text.append(f"[TextureOverride{name}]\nhash = {texHash(TARGET, Routes[comp], role)}\nthis = Resource{name}\n")
        print(f"  {toName}: the mod recolours no {Role} -- the game's {srcName(comp)} {Role} goes on the skin's")
    return "\n".join(text)


_defaults = {}   # (ini, target texture hash) -> the fallback resource the carried recolour copies default to


def fallbackName(iniPath: str, toName: str, Role: str) -> str:
    """One name per .ini: two .ini files in one folder writing the same file would have an undo of one delete the
    other's texture"""
    stem = re.sub(r"\W", "", os.path.splitext(os.path.basename(iniPath))[0])
    return f"{toName}RemapDL{Role}{stem}"


def defaultRecolours(folder):
    """GAP 6b: each carried recolour copy (a fix-block section on a target texture hash, with `this =`) first binds
    the source's game texture, so the mod's own `this` lines -- under a toggle or not -- override a default"""
    for ini in _inis(folder):
        key = os.path.normcase(ini)
        wanted = {h: res for (path, h), res in _defaults.items() if path == key}
        if (not wanted):
            continue
        raw = open(ini, "rb").read().decode("utf-8", errors = "surrogateescape")
        nl = "\r\n" if ("\r\n" in raw) else "\n"
        i = raw.find("; --------------- Miyabi Remap")
        if (i < 0):
            continue
        def inject(m):
            h = re.search(r"^(\s*hash\s*=\s*([0-9a-fA-F]{8})[ \t]*\r?\n)", m.group(2), re.M)
            if (not h or h.group(2).lower() not in wanted or "RemapFix" not in m.group(1) or not re.search(r"^\s*this\s*=", m.group(2), re.M)):
                return m.group(0)
            body = m.group(2)
            return f"[{m.group(1)}]{nl}" + body[:h.end()] + f"this = {wanted[h.group(2).lower()]}{nl}" + body[h.end():]
        block = SectionRe.sub(inject, raw[i:])
        open(ini, "wb").write((raw[:i] + block).encode("utf-8", errors = "surrogateescape"))


def writeFallbacks():
    for src, dest, edit in _fallbacks:
        if (not os.path.isfile(src)):
            print(f"  ! no download {src}")
            continue
        if (edit is None):
            shutil.copyfile(src, dest)
        else:
            FRB.TexEditor([edit], readPillowImg = True, compress = False, mipmaps = True).fix(FRB.TextureFile(src, readPillowImg = True), dest)
    _fallbacks.clear()


def makeFixer(comp, modType, indices):
    tcomp = Routes[comp]
    toName = dstName(tcomp)

    def factory(parser, toModName, modTypeId):
        ini = parser._iniFile
        objs = list(SourceSlots[comp])

        # ---- 1. this pair's graphs only: every other component's are some other fixer's ----
        remap = {}
        for other in Routes:
            for obj in compObjs(other):
                remap[(0, *OBJ(other, obj))] = [(0, *OBJ(other, obj), keepName)] if (other == comp) else []
        edits = [FRB.GraphGroupRemap(remap = remap)]

        # ---- 2. the blend: rebuilt through the vertex group row (GAP 2 filed it under toModName) ----
        # wherever the component's sections bind its blend: a mod may run its pre-skin pass (`vb2 = ...` +
        # `draw = <n>,0`) from the BLEND section or from the POSITION section (Miyabi1 / 4 / 8 / 11 do the
        # latter for their legs), and a blend left unremapped there skins with Miyabi's bone indices on the
        # skin's palette, which shatters the model
        edits.append(FRB.ResRegCollect({(0, *OBJ(comp, kind)): "vb2" for kind in ("blend", "position")},
                                       {"blend": FRB.RemapBlendReplace((0, *OBJ(comp, "RemapBlend")), resType = "blend",
                                                                       fromComp = comp, toComp = tcomp)}))

        # ---- 3. the texcoord, where the two strides differ ----
        fromStride, toStride = TexcoordStride[(SOURCE, comp)], TexcoordStride[(TARGET, tcomp)]
        if (fromStride != toStride):
            edits.append(FRB.ResRegCollect({(0, *OBJ(comp, "texcoord")): "vb1"},
                                           {"texcoord": FRB.RemapBlendReplace((0, *OBJ(comp, "RemapTexcoord")), resType = "texcoord",
                                                                              fixFunc = texcoordCut(fromStride, toStride))}))
            # ...and the resource section the collect wrote still DECLARES the source's stride: the bytes and
            # the declared stride are two edits (Creating Remaps' "WIDENING A BUFFER IS TWO EDITS")
            edits.append(FRB.GraphGroupEdit([{OBJ(comp, "RemapTexcoord"): [FRB.RegNewVals({"stride": str(toStride)})]}]))

        # ---- 3a. a texcoord an ib section BORROWS from another component (GAP 11): Miyabi1 / 4 / 5 / 8 draw
        # their Body mesh from the LEGS section (`ib = <Body ib>`, `vb0 = <pre-skinned Body>`, `vb1 = <Body
        # texcoord>`), so the Legs -> Body copy binds a 24-byte Body texcoord on a slot that reads 20 ----
        declared = declaredStrides(ini.file)
        borrowed = lambda reg, name, data: declared.get(removeRef(name), (None, toStride))[1] != toStride
        # ...and the same for a texcoord the BLEND or POSITION section binds for the draw passes (Miyabi3:
        # `if DRAW_TYPE == 2 || DRAW_TYPE == 4  vb1 = ResourceBodyTexcoord`)
        holders = [OBJ(comp, obj) for obj in objs] + [OBJ(comp, "blend"), OBJ(comp, "position")]
        edits.append(FRB.ResRegCollect({(0, *h): "vb1" for h in holders},
                                       {"texcoord": FRB.RemapBlendReplace((0, *OBJ(comp, "RemapBorrowedTexcoord")), resType = "texcoord",
                                                                          fixFunc = texcoordCutDeclared(declared, toStride))},
                                       resPredicates = {(0, *h): borrowed for h in holders}))
        edits.append(FRB.GraphGroupEdit([{OBJ(comp, "RemapBorrowedTexcoord"): [FRB.RegNewVals({"stride": str(toStride)})]}]))

        # ---- 3b. the light maps, wherever the mod binds them: THE BAND EDIT ----
        if (BandMoves.get(comp)):
            lightMapRefs = [(OBJ(comp, "tex_lightmap"), "this", "RemapTexLightMap")]
            for obj in objs:
                lightMapRefs += [(OBJ(comp, obj), "Resource\\ZZMI\\LightMap", f"{obj}RemapTexLightMap"),
                                 (OBJ(comp, obj), "ps-t5", f"{obj}RemapTexLightMapT5")]
            # GAP 8: ResRegCollect looks a register's value up as a section name as it stands, and ZZMI writes
            # `Resource\ZZMI\LightMap = ref ResourceX` -- so the `ref` comes off for the collect and goes back on
            # after it (without it 3DMigoto COPIES the resource rather than referencing it)
            zzmiLightMap = "Resource\\ZZMI\\LightMap"
            stripRef = FRB.RegNewVals({zzmiLightMap: FRB.FromOldVal(lambda old, modType: removeRef(old))})
            addRef = FRB.RegNewVals({zzmiLightMap: FRB.FromOldVal(lambda old, modType: old if (removeRef(old).lower() == "null") else "ref " + removeRef(old))})
            edits.append(FRB.GraphGroupEdit([{OBJ(comp, obj): [stripRef] for obj in objs}]))
            # GAP 10: a `ps-t5 = ref <resource VARIABLE>` (Miyabi9's shared CommandListRunSlotFix, one list for every
            # component) names no file, and editing it per component means rewriting the mod's own list -- left
            # unedited, so such a mod keeps the skin's band tints
            direct = lambda reg, name, data: not name.strip().lower().startswith("ref ")
            for graph, reg, resName in lightMapRefs:
                edits.append(FRB.ResRegCollect({(0, *graph): reg},
                                               {"lightMap": texReplace((0, *OBJ(comp, resName)), "LightMap", bandEdit(comp), tcomp)},
                                               resPredicates = {(0, *graph): direct}))
            edits.append(FRB.GraphGroupEdit([{OBJ(comp, obj): [addRef] for obj in objs}]))
            tagName = FRB.RegNewVals({"filename": FRB.FromOldVal(lambda old, modType: taggedFile(old, tcomp))})
            edits.append(FRB.GraphGroupEdit([{OBJ(comp, resName): [tagName] for _, _, resName in lightMapRefs}]))

        # ---- 3c. THE TINT COMPENSATION on every diffuse, the same three ways (OFF: see TintCompensation) ----
        if (BandMoves.get(comp) and TintCompensation):
            zzmiDiffuse = "Resource\\ZZMI\\Diffuse"
            stripRefD = FRB.RegNewVals({zzmiDiffuse: FRB.FromOldVal(lambda old, modType: removeRef(old))})
            addRefD = FRB.RegNewVals({zzmiDiffuse: FRB.FromOldVal(lambda old, modType: "ref " + removeRef(old))})
            diffuseRefs = [(OBJ(comp, "tex_diffuse"), "this", "RemapTexDiffuse")]
            for obj in objs:
                diffuseRefs += [(OBJ(comp, obj), zzmiDiffuse, f"{obj}RemapTexDiffuse"), (OBJ(comp, obj), "ps-t3", f"{obj}RemapTexDiffuseT3")]
            compensate = tintCompensation(comp, lightMapFor(ini.file, comp))
            edits.append(FRB.GraphGroupEdit([{OBJ(comp, obj): [stripRefD] for obj in objs}]))
            for graph, reg, resName in diffuseRefs:
                edits.append(FRB.ResRegCollect({(0, *graph): reg},
                                               {"diffuse": texReplace((0, *OBJ(comp, resName)), "Diffuse", compensate, tcomp)},
                                               resPredicates = {(0, *graph): direct}))
            edits.append(FRB.GraphGroupEdit([{OBJ(comp, obj): [addRefD] for obj in objs}]))
            edits.append(FRB.GraphGroupEdit([{OBJ(comp, resName): [tagName] for _, _, resName in diffuseRefs}]))

        # ---- 4. names, hashes and slot indices ----
        # GAP 12: the names keep the ORIGINALS' order. 3DMigoto runs the sections of one hash and first index in
        # name order, and a mod may split one draw over two (Miyabi8's LegsA draws the legs, LegsA2 rebinds vb0 /
        # vb1 to the Body and draws it). Appending the tag sorts "LegsA<tag>" AFTER "LegsA2<tag>" ('2' < 'M'), so
        # LegsA then drew the legs' indices over the Body's vertices: shards. The tag goes after "TextureOverride"
        rename = FRB.GraphRename(lambda n: orderedName(n, naming.getRemapFixName(n, toModName)))
        hashRemap = FRB.RegAssetRemap({"hash": (modType.hashes, "HashNotFound")}, toModName, srcName(comp), ini.fromVersion, ini.toVersion)
        # GAP 13: a VertexLimitRaise's override_byte_stride is the SOURCE component's draw stride (Position 40 +
        # Texcoord + Blend 32: Miyabi's Body 96), and the target's is its own (the skin's Dress 92)
        drawStride = FRB.RegNewVals({"override_byte_stride": str(40 + toStride + 32)})
        group = {OBJ(comp, "ib"): [FRB.GraphRename(lambda n: orderedName(n, naming.getRemapIbName(n, toModName))), hashRemap],
                 OBJ(comp, "blend"): [FRB.GraphRename(lambda n: orderedName(n, naming.getRemapBlendName(n, toModName))), hashRemap],
                 OBJ(comp, "texcoord"): [rename, hashRemap],
                 OBJ(comp, "vertexlimit"): [rename, hashRemap, drawStride],
                 OBJ(comp, "position"): [rename, hashRemap]}
        # GAP 14: a section that also matches on match_index_count (Miyabi3) names the SOURCE object's count, and the
        # target object's differs (Miyabi's Body 26925 -> the skin's Dress 24567): unchanged, the copy matches nothing
        # GAP 16: a mesh two components' sections BOTH draw (Miyabi5 / 6 draw the Body's ib from the Body section
        # and again from LegsA2, over the pre-skinned Body) shows whichever slot the game draws LATER, at equal
        # depth. Miyabi draws Body (frame dump draw 47) before Legs (48), so the Legs-slot copy is what shows on
        # her; the skin draws Body (45) before Dress (46), which would show the Body-slot copy instead, with the
        # game's Body texture at the mod's UVs. So the Body -> Dress copy drops a draw whose index RANGE a Legs section
        # also draws from the same ib (Miyabi5 splits the Body ib between the two slots with no overlap and keeps
        # all of its draws; Miyabi6 draws it whole from both)
        later = rangesDrawnBy(ini.file, modType.hashes.get([srcName("Legs"), "ib"], V)) if (comp == "Body") else {}
        mine = rangesDrawnBy(ini.file, modType.hashes.get([srcName("Body"), "ib"], V)) if (comp == "Body") else {}
        shared = [r for ib in mine if ib in later for r in later[ib]]
        noDouble = FRB.RegNewVals({"drawindexed": FRB.FromOldVal(
            lambda old, modType: "0, 0, 0" if overlaps(drawRange(old), shared) else old)})
        for obj in objs:
            group[OBJ(comp, obj)] = [rename, hashRemap, FRB.RegNewVals({"match_first_index": str(TargetSlots[tcomp][obj]),
                                                                       "match_index_count": str(targetIndexCount(tcomp, obj))})]
            if (shared):
                group[OBJ(comp, obj)].append(noDouble)
        if (shared):
            group[OBJ(comp, "ib")].append(noDouble)
        if (comp in TexPairs):
            for role in TexRoles:
                group[OBJ(comp, f"tex_{role}")] = [rename, hashRemap]
        edits.append(FRB.GraphGroupEdit([group]))

        _alive.extend(edits)
        fixer = FRB.GIMIFixer(parser, graphGroupEdits = edits, modsToFix = [toModName])
        # a section of the mod's own on the component's ib (`hash = ...` line, not a substring: a help overlay or a
        # commented line naming the hash is no reason to add fallbacks)
        ownText = open(ini.file, encoding = "utf-8", errors = "replace").read().split("; --------------- Miyabi Remap")[0]
        drawsComp = re.search(r"^[ \t]*hash[ \t]*=[ \t]*" + modType.hashes.get([srcName(comp), "ib"], V) + r"\b", ownText, re.M | re.I)
        if (comp in TexPairs and drawsComp):
            fixer.appendedSections = fallbackSections(ini, comp, toModName)
        _alive.append(fixer)
        return fixer
    return factory


# ============================================================================== run

def onReadOnly(func, path, _excInfo):
    os.chmod(path, stat.S_IWRITE)
    func(path)


def install():
    modType = modTypeOf(SOURCE)
    indices = slotIndices()
    fileRowsUnderComponentIds(modType)
    _alive.extend([modType, indices])
    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser(SOURCE, makeParser(modType, indices))
    for comp, tcomp in Routes.items():
        FRB.CppStrategyOverrides.setFixer(SOURCE, dstName(tcomp), makeFixer(comp, modType, indices))


# GAP 15: the sword's index buffers are shared by both outfits, its TEXTURES are not, so a weapon mod's
# texture-hash recolour never fires on the skin (whose sword then draws the skin's glowing textures at the
# mod's UVs). A weapon-only `.ini` holds no section the parser classifies as Miyabi, so no fixer runs on it:
# the copies are written here, between markers of this script's own, and removed before every fix and undo.
# The port wants weapon rows in HashData and a weapon pseudo-component instead.
WeaponTextures = {"718d7915": "e1215530", "b73ed7e7": "9d2adcc5", "e1603ca5": "4659445f"}   # Miyabi -> skin
WeaponBegin, WeaponEnd = "; ----- MiyabiBlossomWeaponRemap: begin -----", "; ----- MiyabiBlossomWeaponRemap: end -----"


def _inis(folder):
    for root, _, files in os.walk(folder):
        for f in files:
            if (f.lower().endswith(".ini") and not f.upper().startswith("DISABLED")):
                yield os.path.join(root, f)


def removeWeaponCopies(folder):
    for ini in _inis(folder):
        raw = open(ini, "rb").read().decode("utf-8", errors = "surrogateescape")
        i = raw.find(WeaponBegin)
        if (i < 0):
            continue
        j = raw.find(WeaponEnd, i)
        j = len(raw) if (j < 0) else j + len(WeaponEnd)
        open(ini, "wb").write((raw[:i].rstrip("\r\n") + raw[j:]).encode("utf-8", errors = "surrogateescape"))


def addWeaponCopies(folder):
    for ini in _inis(folder):
        raw = open(ini, "rb").read().decode("utf-8", errors = "surrogateescape")
        nl = "\r\n" if ("\r\n" in raw) else "\n"
        own = raw.split("; --------------- Miyabi Remap")[0]
        copies = []
        for m in re.finditer(r"^\[(TextureOverride[^\]]*)\][ \t]*\r?\n(.*?)(?=^\[|\Z)", own, re.M | re.S):
            body = m.group(2)
            h = re.search(r"^(\s*hash\s*=\s*)([0-9a-fA-F]{8})", body, re.M)
            if (not h or h.group(2).lower() not in WeaponTextures or not re.search(r"^\s*this\s*=", body, re.M)):
                continue
            body = body[:h.start(2)] + WeaponTextures[h.group(2).lower()] + body[h.end(2):]
            name = "TextureOverrideMiyabiBlossomWeaponRemapFix" + m.group(1)[len("TextureOverride"):]
            copies.append(f"[{name}]{nl}{body.rstrip()}{nl}")
        if (copies):
            block = nl.join([nl + WeaponBegin] + copies + [WeaponEnd + nl])
            open(ini, "wb").write((raw.rstrip("\r\n") + nl + block).encode("utf-8", errors = "surrogateescape"))
            print(f"  {os.path.relpath(ini, folder)}: {len(copies)} sword texture override(s) carried to the skin's sword")


# GAP 18: A BODY MESH DRAWN FROM THE LEGS SLOT GOES BACK TO THE DRESS SLOT. Miyabi1 / 4 / 5 / 6 / 8 pre-skin their
# Body mesh on the Body pass (`ResourceBodyVB = copy so0`) and draw it from a LEGS section (`ib = <Body ib>`,
# `vb0 = ResourceBodyVB`). On Miyabi her Body and Legs palettes put the same bone in the same place, so that works;
# on the skin the DRESS palette (where the Body copy pre-skins) sits ~3.5-4 cm off the BODY palette (which the Legs
# copy draws in), so the Body mesh drew that far off the body under it (Miyabi6's hip hole). So those draws are
# moved, with their if-structure and texture lines, into the Dress copy, which reads its own pre-skin; the Legs
# copy's are zeroed. The Dress slot binds other texture hashes, so the mod's Legs texture overrides are bound there
# explicitly (`this` -> the role's register). A text pass over the fix block, after the API's fix; the port wants a
# graph edit that moves draws between two fixers' copies.
LegsTexRegs = {"012e84e9": "ps-t3", "ebac056e": "ps-t4", "a6ea6d83": "ps-t5", "d5462e37": "ps-t6"}   # Miyabi Legs
SectionRe = re.compile(r"^\[([^\]]+)\][ \t]*\r?\n(.*?)(?=^\[|^; \*\*\*|\Z)", re.M | re.S)


def moveCrossSlotDraws(folder):
    for ini in _inis(folder):
        raw = open(ini, "rb").read().decode("utf-8", errors = "surrogateescape")
        nl = "\r\n" if ("\r\n" in raw) else "\n"
        i = raw.find("; --------------- Miyabi Remap")
        if (i < 0 or "; ***** MiyabiBlossomBody *****" not in raw or "; ***** MiyabiBlossomDress *****" not in raw):
            continue
        own, block = raw[:i], raw[i:]
        # the buffers a Body section pre-skins into, and the Legs ib(s)
        bodyIb, legsIb = "981c1a1e", "d8003df3"
        written, legsIbs, texLines = set(), set(), []
        for m in SectionRe.finditer(own):
            h = re.search(r"^\s*hash\s*=\s*([0-9a-fA-F]{8})", m.group(2), re.M)
            if (not h):
                continue
            h = h.group(1).lower()
            written |= set(re.findall(r"^\s*(Resource\w+)\s*=\s*(?:copy\s+)?so0\s*$", m.group(2), re.M))
            if (h == legsIb):
                legsIbs |= {removeRef(v) for v in re.findall(r"^\s*ib\s*=\s*(.+?)\s*$", m.group(2), re.M)}
            if (h in LegsTexRegs and re.search(r"^\s*this\s*=", m.group(2), re.M)):
                body = re.sub(r"^(\s*)this(\s*=)", rf"\g<1>{LegsTexRegs[h]}\2", m.group(2).rstrip(), flags = re.M)
                # only the structure and the bindings: a `hash =` copied in would re-key the Dress section
                texLines += [l for l in body.splitlines() if re.match(r"\s*(if|else|elif|endif)\b|\s*ps-t\d+\s*=", l)]
        if (not written):
            continue
        # the Legs copy sections: zero their Body draws, and collect them (with structure and texture lines)
        a = block.index("; ***** MiyabiBlossomBody *****")
        # only the Legs copies: up to the next component's heading (the Hair's copies follow, and their `ib` and
        # `ps-t` lines had been swept into the Dress section with the moved draws)
        nextHead = block.find("; ***** MiyabiBlossom", a + 10)
        z = nextHead if nextHead > 0 else len(block)
        legsPart, rest = block[a:z], block[z:]
        moved, sectionTex = [], []
        def zero(m):
            body = m.group(2)
            if (not re.search(r"^\s*match_first_index\s*=", body, re.M)):
                return m.group(0)
            ib = vb0 = None
            out, keep = [], []
            for line in body.split(nl):
                s = line.strip()
                mi = re.match(r"ib\s*=\s*(.+)$", s)
                mv = re.match(r"vb0\s*=\s*(.+)$", s)
                if (mi):
                    ib = removeRef(mi.group(1))
                if (mv):
                    vb0 = removeRef(mv.group(1))
                isDraw = re.match(r"drawindexed\s*=", s)
                cross = bool(isDraw and vb0 in written and ib)   # drawn over the Body's pre-skinned vertices
                tex = re.match(r"(Resource\\ZZMI\\\w+\s*=|run\s*=\s*CommandList\\ZZMI\\SetTextures|ps-t\d+\s*=)", s, re.I)
                # structure, ib, texture lines, the vertex buffers that name the Body's pre-skin, and the cross
                # draws, in place (a texture toggle keeps its `if`)
                pre = re.match(r"vb[03]\s*=\s*(.+)$", s)
                if (re.match(r"(if|else|elif|endif)\b", s) or mi or cross or tex or (pre and removeRef(pre.group(1)) in written)):
                    keep.append(line)
                out.append(line.replace(s, "drawindexed = 0, 0, 0") if cross else line)
            if (any(re.match(r"\s*(drawindexed|Resource\\ZZMI|run\s*=|ps-t)", l, re.I) for l in keep)):
                moved.append((any(re.match(r"\s*drawindexed", l) for l in keep), nl.join(keep)))
            return f"[{m.group(1)}]{nl}" + nl.join(out)
        legsPart = SectionRe.sub(zero, legsPart)
        if (not any(draws for draws, _ in moved)):
            continue
        moved = [text for _, text in moved]
        block = block[:a] + legsPart + rest
        # into the Dress copy's object section, at its end
        d = block.index("; ***** MiyabiBlossomDress *****")
        e = block.index("; ***** MiyabiBlossomBody *****")
        dressPart = block[d:e]
        target = next((m for m in SectionRe.finditer(dressPart) if re.search(r"^\s*match_first_index\s*=", m.group(2), re.M)), None)
        if (target is None):
            print(f"  ! {os.path.basename(ini)}: no Dress object section to move the Body draws into")
            continue
        insert = nl.join(["; the Body draws this mod makes from its Legs slot (GAP 18)"] + texLines + moved) + nl
        end = target.end()
        dressPart = dressPart[:end].rstrip() + nl + insert + nl + dressPart[end:].lstrip("\r\n")
        block = block[:d] + dressPart + block[e:]
        open(ini, "wb").write((own + block).encode("utf-8", errors = "surrogateescape"))
        print(f"  {os.path.relpath(ini, folder)}: {len(moved)} Legs-slot Body draw section(s) moved to the Dress slot")


def runOne(folder, undo = False):
    ids = {int(getattr(FRB.ModTypeId, SOURCE))}
    removeWeaponCopies(folder)
    service = FRB.RemapService(path = folder, keepBackups = True, undoOnly = undo, readAllInis = True,
                               fromModTypeIds = ids, logger = FRB.Logger())
    service.fix()
    writeFallbacks()
    if (not undo):
        addWeaponCopies(folder)
        moveCrossSlotDraws(folder)
        defaultRecolours(folder)
    stats = service.stats
    print(f"{folder}: ini fixed {len(stats.ini.fixed)}, skipped {len(stats.ini.skipped)}")
    for path, err in stats.ini.skipped.items():
        print(f"  SKIPPED {os.path.relpath(path, folder)}: {err}")
    return stats


def main():
    ap = argparse.ArgumentParser(description = "PROTOTYPE: Miyabi -> MiyabiBlossom (ZZZ)")
    ap.add_argument("folders", nargs = "+")
    ap.add_argument("--copy", help = "fix copies of the folders under this scratch folder instead")
    ap.add_argument("--undo", action = "store_true")
    args = ap.parse_args()

    install()
    for folder in args.folders:
        target = folder
        if (args.copy):
            target = os.path.join(args.copy, os.path.basename(os.path.normpath(folder)))
            if (os.path.isdir(target)):
                shutil.rmtree(target, onerror = onReadOnly)
            shutil.copytree(folder, target)
        runOne(target, undo = args.undo)
    FRB.CppStrategyOverrides.clear()


if (__name__ == "__main__"):
    main()

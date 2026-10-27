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
             "Legs": {77: 179, 128: 179, 255: 179}}             # onto the skin's Body table
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
        rows.append((["1.0", SOURCE, s, V, dstName(t), t], dict(row.remap)))
    vg.addRows(rows)


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
        nearest = np.abs(red[..., None] - levels).argmin(axis = -1)
        pixels[..., 0] = to[nearest]
        texFile.img = Image.fromarray(pixels, "RGBA")
    return edit


def removeRef(value: str) -> str:
    """IniNamingTools::removeRefPrefix, which is not bound"""
    value = value.strip()
    return value[4:].strip() if (value[:4].lower() == "ref ") else value


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
        if (srcHash.lower() in recoloured):
            continue
        name = f"{toName}RemapDL{Role}"
        file = os.path.join(os.path.dirname(ini.file), name + ".dds")
        _fallbacks.append((os.path.join(DownloadFolder, f"{srcName(comp)}A{Role}.dds"), file, bandEdit(comp) if (role == "lightmap") else None))
        text.append(f"[TextureOverride{name}]\nhash = {texHash(TARGET, Routes[comp], role)}\nthis = Resource{name}\n\n"
                    f"[Resource{name}]\nfilename = {name}.dds\n")
        print(f"  {toName}: the mod recolours no {Role} -- the game's {srcName(comp)} {Role} goes on the skin's")
    return "\n".join(text)


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
        if (comp in TexPairs):
            lightMapRefs = [(OBJ(comp, "tex_lightmap"), "this", "RemapTexLightMap")]
            for obj in objs:
                lightMapRefs += [(OBJ(comp, obj), "Resource\\ZZMI\\LightMap", f"{obj}RemapTexLightMap"),
                                 (OBJ(comp, obj), "ps-t5", f"{obj}RemapTexLightMapT5")]
            # GAP 8: ResRegCollect looks a register's value up as a section name as it stands, and ZZMI writes
            # `Resource\ZZMI\LightMap = ref ResourceX` -- so the `ref` comes off for the collect and goes back on
            # after it (without it 3DMigoto COPIES the resource rather than referencing it)
            zzmiLightMap = "Resource\\ZZMI\\LightMap"
            stripRef = FRB.RegNewVals({zzmiLightMap: FRB.FromOldVal(lambda old, modType: removeRef(old))})
            addRef = FRB.RegNewVals({zzmiLightMap: FRB.FromOldVal(lambda old, modType: "ref " + removeRef(old))})
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
        for obj in objs:
            group[OBJ(comp, obj)] = [rename, hashRemap, FRB.RegNewVals({"match_first_index": str(TargetSlots[tcomp][obj]),
                                                                       "match_index_count": str(targetIndexCount(tcomp, obj))})]
        if (comp in TexPairs):
            for role in TexRoles:
                group[OBJ(comp, f"tex_{role}")] = [rename, hashRemap]
        edits.append(FRB.GraphGroupEdit([group]))

        _alive.extend(edits)
        fixer = FRB.GIMIFixer(parser, graphGroupEdits = edits, modsToFix = [toModName])
        if (comp in TexPairs and modType.hashes.get([srcName(comp), "ib"], V).lower() in open(ini.file, encoding = "utf-8", errors = "replace").read().lower()):
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


def runOne(folder, undo = False):
    ids = {int(getattr(FRB.ModTypeId, SOURCE))}
    service = FRB.RemapService(path = folder, keepBackups = True, undoOnly = undo, readAllInis = True,
                               fromModTypeIds = ids, logger = FRB.Logger())
    service.fix()
    writeFallbacks()
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

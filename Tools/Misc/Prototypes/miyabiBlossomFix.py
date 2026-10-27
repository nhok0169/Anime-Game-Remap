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
# from 24 to 20 bytes a vertex. Textures need nothing: a ZZMI mod binds them by ROLE through
# CommandList\ZZMI\SetTextures and both outfits draw with the same shaders. ZZZ has no NNFix / ORFix.
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
#

import argparse
import os
import shutil
import stat
import struct
import sys

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
        byIndex = {"ib": {OBJ(comp, o): OBJ(comp, o) for o in SourceSlots[comp]}}
        modObjs += [OBJ(comp, o) for o in SourceSlots[comp]] + [OBJ(comp, obj) for obj in HashKinds.values()]
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


def makeFixer(comp, modType, indices):
    tcomp = Routes[comp]
    toName = dstName(tcomp)

    def factory(parser, toModName, modTypeId):
        ini = parser._iniFile
        objs = list(SourceSlots[comp])

        # ---- 1. this pair's graphs only: every other component's are some other fixer's ----
        remap = {}
        for other in Routes:
            for obj in list(SourceSlots[other]) + list(HashKinds.values()):
                remap[(0, *OBJ(other, obj))] = [(0, *OBJ(other, obj), keepName)] if (other == comp) else []
        edits = [FRB.GraphGroupRemap(remap = remap)]

        # ---- 2. the blend: rebuilt through the vertex group row (GAP 2 filed it under toModName) ----
        edits.append(FRB.ResRegCollect({(0, *OBJ(comp, "blend")): "vb2"},
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

        # ---- 4. names, hashes and slot indices ----
        rename = FRB.GraphRename(lambda n: naming.getRemapFixName(n, toModName))
        hashRemap = FRB.RegAssetRemap({"hash": (modType.hashes, "HashNotFound")}, toModName, srcName(comp), ini.fromVersion, ini.toVersion)
        group = {OBJ(comp, "ib"): [FRB.GraphRename(lambda n: naming.getRemapIbName(n, toModName)), hashRemap],
                 OBJ(comp, "blend"): [FRB.GraphRename(lambda n: naming.getRemapBlendName(n, toModName)), hashRemap],
                 OBJ(comp, "texcoord"): [rename, hashRemap],
                 OBJ(comp, "vertexlimit"): [rename, hashRemap],
                 OBJ(comp, "position"): [rename, hashRemap]}
        for obj in objs:
            group[OBJ(comp, obj)] = [rename, hashRemap, FRB.RegNewVals({"match_first_index": str(TargetSlots[tcomp][obj])})]
        edits.append(FRB.GraphGroupEdit([group]))

        _alive.extend(edits)
        fixer = FRB.GIMIFixer(parser, graphGroupEdits = edits, modsToFix = [toModName])
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

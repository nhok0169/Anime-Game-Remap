#
# ===== neuvilletteFromMelusentFix (prototype) =====
#
# NeuvilletteMelusent ("Melusent Gift", 6.3) -> Neuvillette, as NOTHING BUT A CONFIG: a GIMIComponentParserConfig
# and a GIMIMergeFixerConfig handed to the bound makeGIMIComponentParser / makeGIMIMergeFixer -- the same factories
# the compiled rows will call -- registered on CppStrategyOverrides and run by RemapService. The reverse of
# neuvilletteMelusentFix.py; the port is a transcription of the two configs.
#
#   py -3 neuvilletteFromMelusentFix.py <mod folder>                  fix every NeuvilletteMelusent .ini under the folder
#   py -3 neuvilletteFromMelusentFix.py <mod folder> --download disabled --verbose
#
# ---- The skin's slots (off FrameAnalysis-NeuvilletteMelusent-2026-09-24-204316, giDrawTable.py) ----
#
#   slot           first    draw                               textures it reads
#   main Head      0        vs 63e32ce4 / ps 883013ba, LND     its own (hair, face skin, lapels)
#   main Body      46620    vs 63e32ce4 / ps 5f3b4260, LND     its own (waistcoat, trousers)
#   main Dress     71025    vs 4c036e7e / ps 26dbacaa, LND     the Head's set
#   Coat A         0        vs 63e32ce4 / ps 5f3b4260, LND     the Body's set
#   Bang A         0        vs 63e32ce4 / ps 883013ba, LND     the Head's set
#   Eye A          0        plain vs 95aa6cdb, LD              the Head's diffuse / light map
#
# The main mesh's component name is EMPTY (its files are NeuvilletteMelusentHead.ib,
# NeuvilletteMelusentPosition.buf), so a slot borrowing its textures names its donor ";Head".
#
# ---- Where each slot lands ----
#
# Neuvillette is one mesh of three objects: head (plain shader, hair and face), body (normal-map shader),
# dress (his cravat and ruffles, plain). Each slot goes to the object whose TEXTURES it draws with, so a merged
# object keeps one texture set as far as possible: the Head set (main Head, main Dress, Bang, Eye) onto his
# head, the Body set (main Body, Coat) onto his body. His dress receives nothing; the whole-ib skip keeps his
# own cravat hidden.
#
# ---- The target's layout ----
#
# ONE layout for all three objects, the normal-map one under ORFix, although his head and dress draw on a
# plain shader: his own mods say ORFix serves that shader from the normal-map layout too (Neuvillette8 writes
# its head that way and renders right on his card), so GIMIMergeFixerConfig's single targetLayout is enough.
#
#
# ---- Library gaps found, all filled in SHARED code (see Creating Remaps' "NEUVILLETTE <-> NEUVILLETTEMELUSENT") ----
#
#   * GIMIMergeFixerConfig::Component::modTypeName -- the unnamed main mesh's hashes are NeuvilletteMelusentMain's.
#   * GIMIMergeFixerConfig::texcoordStride -- every skin component carries 12 bytes, Neuvillette reads 20.
#   * A 16-bit mod's merged `drawindexed` counts were halved (bytes / 4).
#   * A recolour in a SIBLING .ini (NeuvilletteMelusent1's tex.ini) is carried onto the mesh file's slots through
#     RemapRef resources, and the recolour file defers to the sibling that draws the mesh.
#
# Tested on the identity, NeuvilletteMelusent1 and four synthetic mods (neuvilletteMelusentSynth.py).

import argparse
import os
import sys

Repo = os.environ.get("AG_REMAP_REPO") or r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss"
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
if (not os.path.isdir(APISrc)):
    raise SystemExit(f"the API's package folder is not at {APISrc}; set AG_REMAP_REPO to the repo's path")
sys.path.insert(0, APISrc)
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))

import FixRaidenBoss2 as FRB     # noqa: E402

SrcName = "NeuvilletteMelusent"
DstName = "Neuvillette"
Prefix = "NeuvilletteMelusent"

# (component, slot, first index, layout, target object, the game model's index count, donor)
Slots = [("", "Head", "0", "normal", "head", 46620, ""),
         ("", "Body", "46620", "normal", "body", 24405, ""),
         ("", "Dress", "71025", "normal", "head", 3567, ";Head"),
         ("Coat", "A", "0", "normal", "body", 26532, ";Body"),
         ("Bang", "A", "0", "normal", "head", 9336, ";Head"),
         ("Eye", "A", "0", "plain", "head", 528, ";Head")]

# component -> (the game model's vertex count, texcoord stride), in MERGE order: the biggest first
Components = {"": (22503, 12), "Coat": (8670, 12), "Bang": (2644, 12), "Eye": (168, 12)}
ModTypeNames = {"": "NeuvilletteMelusentMain", "Coat": "NeuvilletteMelusentCoat",
                "Bang": "NeuvilletteMelusentBang", "Eye": "NeuvilletteMelusentEye"}


def parserConfig() -> "FRB.GIMIComponentParserConfig":
    config = FRB.GIMIComponentParserConfig()
    config.modTypeId = FRB.ModTypeId.NeuvilletteMelusent
    config.downloadCharFolder = "NeuvilletteMelusent"
    config.downloadVersionFolder = "6_3"
    config.downloadPrefix = Prefix

    components = []
    for name, (vertexCount, texcoordStride) in Components.items():
        component = FRB.GIMIComponentParserConfig.Component()
        component.name = name
        component.modTypeName = ModTypeNames[name]
        component.texcoordStride = texcoordStride
        component.vertexCount = vertexCount

        slots = []
        for comp, slotName, index, layout, _, _, donor in Slots:
            if (comp != name):
                continue
            slot = FRB.GIMIComponentParserConfig.Slot()
            slot.name = slotName
            slot.index = index
            if (layout == "normal"):
                slot.normalMapReg, slot.diffuseReg, slot.lightMapReg = "ps-t0", "ps-t1", "ps-t2"
            else:
                slot.normalMapReg, slot.diffuseReg, slot.lightMapReg = "", "ps-t0", "ps-t1"
            slot.noTextures = bool(donor)
            slot.textureDonor = donor
            # The target reads normal maps (see the header), so a borrowing slot needs the donor's too.
            slot.donorNormalMap = bool(donor)
            slots.append(slot)
        component.slots = slots
        components.append(component)

    config.components = components
    return config


def fixerConfig() -> "FRB.GIMIMergeFixerConfig":
    config = FRB.GIMIMergeFixerConfig()

    components = []
    for name, (vertexCount, _) in Components.items():
        component = FRB.GIMIMergeFixerConfig.Component()
        component.name = name
        # The main mesh is the UNNAMED component: its hashes are filed under NeuvilletteMelusentMain, not
        # under the skin's own name followed by "" (GIMIMergeFixerConfig.Component.modTypeName).
        component.modTypeName = ModTypeNames[name]
        component.vertexCount = vertexCount

        slots = []
        for comp, slotName, index, layout, to, indexCount, donor in Slots:
            if (comp != name):
                continue
            slot = FRB.GIMIMergeFixerConfig.Slot()
            slot.name = slotName
            slot.index = index
            slot.to = to
            slot.normalMap = (layout == "normal")
            slot.borrowFrom = donor
            slot.indexCount = indexCount
            slot.outline = True
            slots.append(slot)
        component.slots = slots
        components.append(component)

    # the skin's eyes are his 1.24 cm lower: moved back up unless the mod brings its own face
    for component in components:
        if component.name == "Eye":
            component.positionOffset = [0.0, 0.01237, 0.00021]
            component.offsetOnlyWithGameFace = True
    config.components = components
    config.targetObjs = ["head", "body", "dress"]
    config.targetLayout = FRB.GIMIMergeFixerConfig.TargetLayout.NormalMap
    config.texRegsByName = True
    config.downloadPrefix = Prefix

    # Neuvillette's Texcoord is 20 bytes a vertex (a second UV set); every one of the skin's components carries
    # 12. The merge pads each line at its end up to his width, and the copied section declares it.
    config.texcoordStride = 20

    # Neuvillette binds his face diffuse at ps-t1, the GI 6.x layout.
    config.faceReg = "ps-t1"
    config.lightMapEdit = None
    config.compressTextures = False

    # a mod hiding the skin's face meshes by hash hides his too; his unreached dress withdraws a TexFx request
    config.sideMeshes = ["ib_face", "ib_headupper"]
    config.texFxGuardUnreached = True
    # (copyPreamble is set on the compiled row only: a merge writes one .ini group, and the preamble heads copies)
    return config


# ---- Vertex-group moves tried in game before they go into VGRemapData.cpp ----
#
# The skin wears its coat as a MANTLE: its sleeves (Coat 32 -> 34 -> 36 left, 33 -> 35 -> 37 right, the bell cuff last)
# hang EMPTY from the shoulders, behind the arms (z -0.18..-0.30 against the arm's -0.03), on a chain of their own.
# Neuvillette has nothing like it, and every split of that chain showed in game (the identity mod, 2026-09-25):
#   * the draft (34 / 36 onto his cuff group 58, 32 onto his shoulder cloth 57): the cuff jutted out sideways at
#     elbow height, pale lining to the camera, when he raised the goblet;
#   * onto his elbow (84) or forearm (85): the sleeve followed the forearm while its top followed 57 -- stretched;
#   * whole onto 57: it swung forward to horizontal when he drank (57 moves with his arm);
#   * whole onto his clavicle (70 / 89): right when he raised an arm, but rigid at the bind pose's angle, so at
#     idle both cuffs flared out from his sides.
# SleevesOnUpperArm hangs the whole chain from his UPPER ARM (64 / 67): along the arm at idle, lifted with it, the
# forearm bending inside -- a loosely worn coat. The same audit found the draft sending the mantle's RIGHT shoulder
# (Coat 14 / 31) to 65 where its left twin (13 / 30) goes to 64, whose mirror is 67.
SleevesOnUpperArm = {32: 64, 34: 64, 36: 64, 33: 67, 35: 67, 37: 67, 14: 67, 31: 67}
# The main mesh's own cuff band (53 / 54, on the forearm under the mantle) goes to his FOREARM (85 / 104, the bone his
# hand chain shares vertices with) rather than to 58 / 61, his separate cuff piece, which shares none.
MainCuffsOnForearm = {53: 85, 54: 104}


def applyVgMoves(moves: dict, fromComp: str):
    """Moves source groups of the skin's component `fromComp` onto other Neuvillette groups, by REPLACING that
    component's reverse row through VGRemaps.addRows (same key, same versions)."""
    modType = FRB.GIBuilder.neuvilletteMelusent()
    remap = modType.getVGRemap(DstName, fromComp = fromComp, toComp = "")
    row = {} if remap is None else dict(remap.remap)
    row.update(moves)
    modType.vgRemaps.addRows([(["1.0", SrcName, fromComp, "6.3", DstName, ""], row)])


def main():
    parser = argparse.ArgumentParser(description = "NeuvilletteMelusent -> Neuvillette, as a merge-template config")
    parser.add_argument("mod", help = "the mod folder (every NeuvilletteMelusent .ini under it is fixed)")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--download", default = None,
                        help = "the API's downloadMode, eg. `disabled` -- omit for the API's own default")
    parser.add_argument("--draftCuffs", action = "store_true", help = "use the draft's sleeve rows instead of SleevesOnUpperArm / MainCuffsOnForearm (the A/B)")
    args = parser.parse_args()

    # Until the rows are transcribed into VGRemapData.cpp -- after which this is a no-op.
    if not args.draftCuffs:
        applyVgMoves(SleevesOnUpperArm, "Coat")
        applyVgMoves(MainCuffsOnForearm, "")

    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser(SrcName, FRB.makeGIMIComponentParser(parserConfig()))
    FRB.CppStrategyOverrides.setFixer(SrcName, DstName, FRB.makeGIMIMergeFixer(fixerConfig()))

    folder = os.path.abspath(args.mod)
    if (not os.path.isdir(folder)):
        raise SystemExit(f"no such mod folder: {folder}")
    try:
        service = FRB.RemapService(path = folder, keepBackups = args.keepBackups,
                                   **({"downloadMode": args.download} if args.download else {}),
                                   logger = FRB.Logger() if args.verbose else None)
        service.fix()

        stats = service.stats
        print(f"\n.ini fixed: {len(stats.ini.fixed)}, skipped: {len(stats.ini.skipped)}")
        for path, error in stats.ini.skipped.items():
            print(f"  SKIPPED {os.path.relpath(path, folder)}: {error}")
        for label in ("blend", "position", "texcoord", "buf", "texEdit", "texAdd", "download"):
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

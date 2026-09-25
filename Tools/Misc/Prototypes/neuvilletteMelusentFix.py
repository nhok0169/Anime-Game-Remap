#
# ===== neuvilletteMelusentFix (prototype) =====
#
# Neuvillette -> NeuvilletteMelusent ("Melusent Gift", 6.3), as NOTHING BUT A CONFIG: a
# GIMICharParserConfig and a GIMIComponentFixerConfig handed to the bound makeGIMICharParser /
# makeGIMIComponentFixer -- the same factories the compiled rows will call -- registered on
# CppStrategyOverrides and run by RemapService. So the port is a transcription of the two configs.
#
#   py -3 neuvilletteMelusentFix.py <mod folder>                  fix every Neuvillette .ini under the folder
#   py -3 neuvilletteMelusentFix.py <mod folder> --download disabled --verbose
#
# Every value below was read off the two frame dumps of the outfit shop's previews
# (FrameAnalysis-Neuvillette-2026-09-24-205101, FrameAnalysis-NeuvilletteMelusent-2026-09-24-204316)
# with Tools/Misc/Diagnostics/giDrawTable.py, and off the two download folders.
#
# ---- What the dumps say (2026-09-24) ----
#
#   * Neuvillette is one mesh: Head (first index 0), Body (33879), Dress (79377). His Head draws on the
#     PLAIN shader pair vs 95aa6cdb / ps 20872172 (light map, diffuse at ps-t0/1 in the dump -- ps-t0/1 =
#     diffuse / light map in a mod, under NNFix); his Body on the normal-map pair vs 2c157719 /
#     ps 6546504e (LND in the dump); his Dress on the plain vs d4c01363 with the Body's textures. His
#     face diffuse 81e80510 is bound at ps-t1 (GI 6.x), on unskinned face meshes both characters draw.
#   * The skin is four skinned components -- an UNNAMED main mesh, Coat, Bang, Eye:
#       main Head   0      vs 63e32ce4 / ps 883013ba, LND   its own set (hair, face skin, lapels)
#       main Body   46620  vs 63e32ce4 / ps 5f3b4260, LND   its own set (waistcoat, trousers)
#       main Dress  71025  vs 4c036e7e / ps 26dbacaa, LND   the Head's set
#       Coat A      0      vs 63e32ce4 / ps 5f3b4260, LND   the Body's set
#       Bang A      0      vs 63e32ce4 / ps 883013ba, LND   the Head's set
#       Eye A       0      vs 95aa6cdb (plain), LD         the Head's diffuse / light map
#     and its face diffuse is its OWN (6dab6f0e), bound at ps-t1 on the same unskinned face meshes.
#
# ---- Which slot ----
#
# Each of his objects goes through the slot that shades the same KIND of part: his head (hair, face
# skin) through the main Head slot (the skin's hair shader) and his body through the main Body slot.
# His DRESS is not a dress: it is his white cravat, lace and cuff ruffles (849 vertices round the collar
# and the wrists), and through the skin's Dress slot (ps 26dbacaa) they came out with DARK BLOTCHES
# (Neuvillette6 and the identity, 2026-09-24). Tried in game, one change at a time: the created normal
# map at 128 instead of 55 (no change), his body light map's 126-128 band moved to 255 (no change),
# the dress through the Head slot (pinkish-red shadows -- the head atlas's skin band), through the Body
# slot (clean, a faint cyan cast in the shadows: his white cloth band 126-128 is the skin body atlas's
# CYAN cloth band), and Body slot + the dress band 126-128 moved to 255 (neutral grey but harder,
# darker shadows). Body slot, no band move, is the default -- the cast is the maintainer's call. Whatever of him lands on the Coat / Bang / Eye bones is drawn by that
# component's one slot. The main mesh's component name is the EMPTY string (its files are
# NeuvilletteMelusentHead.ib, NeuvilletteMelusentPosition.buf -- how the skin's own mods name them);
# only its fix-target id, NeuvilletteMelusentMain, carries a name.
#
# ---- Band legends (light map alpha, over each whole atlas) ----
#
#   Neuvillette head   255 white HAIR, 128-129 blue hair, 76-77 gold, 0 dark
#   Neuvillette body   255 SKIN, 126-128 silver cloth, 78 gold, 0 dark-blue cloth
#   skin head atlas    255 white HAIR, 126-127 SKIN, 178 gold, 78 light blue, 0 dark blue
#   skin body atlas    255 white cloth, 176-178 gold, 126-128 cyan, 78 dark grey, 0 blue cloth
#
# They differ on skin (his 255 against the skin's 126-127) and on gold (76-78 against 176-178). No band
# move until the game says one is needed (Charlotte's rule).
#
# ---- What the game has said so far ----
#
#   * WHITE SPARKLES over his boots, trousers and gear (identity mod, 2026-09-24): the template's
#     invented flat normal map carried B = 255, and this skin's shader reads normal-map B as a glitter
#     mask (every real 6.x normal map here has B ~0). Not the slot (swapping his head and body slots moved
#     nothing), not the light map's B or alpha (each zeroed / moved in turn, no change). Fixed by
#     flatNormal's B = 0 -- see fixerConfig().
#
# ---- Library gaps found, all filled in SHARED code (see Creating Remaps' "NEUVILLETTE <-> NEUVILLETTEMELUSENT") ----
#
#   * ObjDownloadRegs::coverRegs -- his mods write the head in two layouts, and a one-register download fired on
#     the other one's mods.
#   * GIMIComponentFixerConfig::texRegsByName, believed only without an own fix call and with one role per name.
#   * Hide-by-result + Component::slotIndices; GIMIFixer renders appendedSections with no groups (an owner that
#     draws nothing lost the hides).
#   * A merged master: `draw` per .ini GROUP, and a group's raw vb0 / vb1 removed where its object keeps nothing
#     (RegBranchAdd::Branch::removals); ResEditConfig::filePerSection; ResGroupCollect skips `null`.
#   * A copy declares the downloads it binds (GIMIFixer::groupToStr).
#   * RemapService::_removeRemapCopies deletes what an undo of a copy removed.
#

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

SrcName = "Neuvillette"
Skin = "NeuvilletteMelusent"

# The skin's main-mesh slots: name -> match_first_index (the GAME model's, off the frame dump)
MainSlots = {"Head": "0", "Body": "46620", "Dress": "71025"}


def parserConfig() -> "FRB.GIMICharParserConfig":
    config = FRB.GIMICharParserConfig()
    config.modTypeId = int(FRB.ModTypeId.Neuvillette)
    config.downloadCharFolder = "Neuvillette"
    config.downloadVersionFolder = "4_0"
    config.downloadPrefix = "Neuvillette"
    config.drawnObjs = ["head", "body", "dress"]
    config.texcoordStride = 20       # NeuvilletteTexcoord.buf: 495240 / 24762
    # His head and dress are the plain layout (diffuse ps-t0, light map ps-t1); his body the normal-map
    # layout (normal map ps-t0, diffuse ps-t1, light map ps-t2).
    #
    # His MODS do not agree, though: Neuvillette8 writes his head on the normal-map layout (diffuse ps-t1,
    # light map ps-t2, ORFix) where Neuvillette6 writes it plain. A download keyed on ps-t0 then fired on
    # Neuvillette8's head and put his head DIFFUSE in the normal-map slot (his hair ribbon drew flat green
    # and dark red, 2026-09-24). coverRegs: a section binding any texture register brings its own set.
    cover = ["ps-t0", "ps-t1", "ps-t2"]
    config.objDownloadRegs = [FRB.GIMICharParserConfig.ObjDownloadRegs("head", "ps-t0", "ps-t1", "", cover),
                              FRB.GIMICharParserConfig.ObjDownloadRegs("body", "ps-t1", "ps-t2", "ps-t0", cover),
                              FRB.GIMICharParserConfig.ObjDownloadRegs("dress", "ps-t0", "ps-t1", "", cover)]
    # The face meshes are shared and a face section exists only because a mod overrides the face: the
    # download could only ever fire wrongly (Citlali's reasoning, GIMICharParserConfig::faceDownload).
    config.faceDownload = False
    return config


def fixerConfig(dressSlot: str = "Body") -> "FRB.GIMIComponentFixerConfig":
    config = FRB.GIMIComponentFixerConfig()
    config.targetSkin = Skin
    config.drawnObjs = ["head", "body", "dress"]

    # Per object: a section binding ps-t2 is on the normal-map layout (his body), otherwise plain (his
    # head, his dress) and shifted up onto the skin's normal-map slots with a flat normal map.
    config.sourceLayout = FRB.GIMIComponentFixerConfig.SourceLayout.Detect
    # ...and each binding read by its resource NAME first: Neuvillette8 writes his dress in the GAME's
    # register order (ps-t0 light map, ps-t1 diffuse, no fix call), which read positionally drew his hair
    # ribbon's tails with the light map as their colour -- flat green (2026-09-24).
    config.texRegsByName = True
    # Both characters read the face diffuse at ps-t1 (GI 6.x): swap only a mod still on ps-t0.
    config.faceSwapOnlyFromDiffuseReg = True

    main = FRB.GIMIComponentFixerConfig.Component()
    main.name = ""
    main.modTypeName = Skin + "Main"
    main.slot = "Head"
    main.slotIndex = MainSlots["Head"]
    main.objSlotIndices = [("body", MainSlots["Body"]), ("dress", MainSlots[dressSlot])]
    main.slotIndices = list(MainSlots.values())
    main.negativeIndex = False
    main.normalMap = True
    main.face = True
    main.texcoordStride = 12         # NeuvilletteMelusentTexcoord.buf: 270036 / 22503
    main.slotRegisters = ["ps-t0", "ps-t1", "ps-t2"]

    coat = FRB.GIMIComponentFixerConfig.Component()
    coat.name = "Coat"
    coat.modTypeName = Skin + "Coat"
    coat.slot = "A"
    coat.slotIndex = "0"
    coat.slotIndices = ["0"]
    coat.negativeIndex = False
    coat.normalMap = True
    coat.face = False
    coat.texcoordStride = 12
    coat.slotRegisters = ["ps-t0", "ps-t1", "ps-t2"]

    bang = FRB.GIMIComponentFixerConfig.Component()
    bang.name = "Bang"
    bang.modTypeName = Skin + "Bang"
    bang.slot = "A"
    bang.slotIndex = "0"
    bang.slotIndices = ["0"]
    bang.negativeIndex = False
    bang.normalMap = True
    bang.face = False
    bang.texcoordStride = 12
    bang.slotRegisters = ["ps-t0", "ps-t1", "ps-t2"]

    # His eyes (vertex groups 13 / 14) are in his HEAD object. The skin draws its Eye on the PLAIN shader
    # 95aa6cdb with a diffuse / light map at ps-t0 / ps-t1 under NNFix.
    eye = FRB.GIMIComponentFixerConfig.Component()
    eye.name = "Eye"
    eye.modTypeName = Skin + "Eye"
    eye.slot = "A"
    eye.slotIndex = "0"
    eye.slotIndices = ["0"]
    eye.negativeIndex = False
    eye.normalMap = False
    eye.face = False
    eye.texcoordStride = 12
    eye.slotRegisters = ["ps-t0", "ps-t1"]
    # His eye mesh IS the skin's, vertex for vertex, 1.24 cm higher: the eyes sit in the GAME's face mesh,
    # and at his height the irises were behind the skin's upper lids (white eyes, no pupils). Measured as
    # the skin's EyePosition.buf minus this fix's Eye for his identity mod (residual under 0.3 mm).
    eye.positionOffset = [0.0, -0.01237, -0.00021]
    # ...but only into the GAME's face: Neuvillette2 hides it and draws its own inside his head mesh, which
    # reaches the skin unshifted -- shifted, its eyes sat below that face and looked down.
    eye.offsetOnlyWithGameFace = True

    # The Coat draws nothing: his whole outfit is on the main mesh -- see TailsInMain below.

    config.components = [main, coat, bang, eye]
    # Every one of the skin's components receives a forward vertex-group row, so none is hidden by
    # request -- but a mod can still put NOTHING on a component's bones (Neuvillette8, a summer outfit,
    # has no coat), and the template hides such a component by the result. The TexFx guards come from
    # slotIndices the same way: whichever slot this mod draws nothing through (the Dress slot, now his
    # dress goes through the Body slot; every slot of an empty component) is guarded.
    config.hiddenComponents = []
    config.unremappedSlots = []

    # The flat normal map invented for a plain-layout object (his head, his dress) has BLUE = 0, where the
    # template's default is 255. Every GI 6.x normal map in play -- his body's, both of the skin's -- is
    # R, G ~128, B ~0, A 255, and this skin's shaders read B as a GLITTER mask: with the default, his
    # boots, trousers and gear came out covered in white sparkles (in game, 2026-09-24), gone the moment
    # the created map's B was zeroed and nothing else changed. R / G stay the template's 55 (the sRGB
    # pre-correction every earlier component config was confirmed with).
    config.flatNormal = FRB.CppColour(55, 55, 0, 255)

    config.lightMapEdit = None
    config.compressTextures = False  # a band selector is exact; BC7 would move it
    return config


# ---- Vertex-group moves tried in game before they go into VGRemapData.cpp ----
#
# His long coat tails (groups 32-47, left 32-39 / right 40-47, waist to hem) were split across TWO of the
# skin's components by the draft -- the upper groups onto the main mesh's back pieces (85-88), the lower
# ones onto the Coat's tail ends (Coat:24/25). A vertex weighted to both is drawn by both components
# through different bones, and on Neuvillette3's long coat the hem came out SAWTOOTHED where the two
# copies part (2026-09-24). --tailsInCoat keeps the whole chain in the Coat, down its back panel by
# height: Coat:7 -> Coat:9 -> Coat:24 on the left, Coat:8 -> Coat:10 -> Coat:25 on the right. The hem
# came out smooth, matching the mod's own preview.
TailsInCoat = {32: 7, 35: 7, 36: 9, 33: 9, 37: 24, 34: 24, 38: 24, 39: 24,
               40: 8, 43: 8, 44: 10, 41: 10, 45: 25, 42: 25, 46: 25, 47: 25}

# ONE OUTFIT, ONE COMPONENT: his coat groups (0, 1 and the tail chains 32-47) go to the MAIN mesh, and the Coat
# row is EMPTY, so the Coat draws nothing and the template hides it. The main mesh and the Coat share no bone
# (checked in the skin's frame dump), and a mod's outfit is one connected garment -- welded pieces and shell/lining
# layers link every coat, cape and tail to the body -- so ANY cut between the two components tears somewhere:
# Neuvillette3, 4 and 5's capes ripped open in game (2026-09-25) under every split tried (tails whole in the Coat;
# a Coat claim share; an overlap band; whole connected pieces, which then pulled a coat's lining through its shell).
# With the whole outfit on the main mesh there is no seam at all. The cost is the swing: the main mesh has no long
# coat bones, so his upper links take its back-skirt bones (85-88, 73) and his lower links its knees (23 / 43) and
# shins (6 / 26) -- a hem moves with the legs like a long skirt. The maintainer grades geometry faults above texture
# ones, and a hole is the worst geometry fault; 0 / 1 (back pieces) ride the spine rather than the nearest bone (60,
# behind the upper back), which may be a loose piece.
TailsInMain = {0: 0, 1: 0, 32: 85, 33: 23, 34: 23, 35: 73, 36: 87, 37: 23, 38: 6, 39: 6, 40: 86, 41: 43, 42: 43, 43: 86, 44: 88, 45: 43, 46: 26, 47: 26}


def applyVgMoves(moves: dict, toComp: str):
    """Moves source groups onto target component `toComp`: removed from whichever forward row holds them,
    added to that component's row. Rows are REPLACED through VGRemaps.addRows (same key, same versions)."""
    modType = FRB.GIBuilder.neuvillette()
    rows = {}
    for comp in ("", "Coat", "Bang", "Eye"):
        remap = modType.getVGRemap(Skin, fromComp = "", toComp = comp)
        rows[comp] = {} if remap is None else dict(remap.remap)
    for group, target in moves.items():
        for comp in rows:
            rows[comp].pop(group, None)
        rows[toComp][group] = target
    assert sorted(g for r in rows.values() for g in r) == list(range(116)), "every source group maps exactly once"
    table = modType.vgRemaps
    for comp, remap in rows.items():
        table.addRows([(["1.0", SrcName, "", "6.3", Skin, comp], remap)])


def main():
    parser = argparse.ArgumentParser(description = "Neuvillette -> NeuvilletteMelusent, as a component-template config")
    parser.add_argument("mod", help = "the mod folder (every Neuvillette .ini under it is fixed)")
    parser.add_argument("--dressSlot", default = "Body", choices = sorted(MainSlots),
                        help = "the main-mesh slot his DRESS (cravat, lace, cuff ruffles) is drawn through (default: Body -- see the header)")
    parser.add_argument("--draftTails", action = "store_true", help = "use the draft's split coat-tail rows instead of TailsInMain (the A/B)")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--download", default = None,
                        help = "the API's downloadMode, eg. `disabled` -- omit for the API's own default")
    args = parser.parse_args()

    # Confirmed in game on Neuvillette3, 4 and 5 (2026-09-25): applied unless --draftTails, until the rows are
    # transcribed into VGRemapData.cpp -- after which this is a no-op (the rows already say so).
    if not args.draftTails:
        applyVgMoves(TailsInMain, "")
    config = fixerConfig(args.dressSlot)
    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser(SrcName, FRB.makeGIMICharParser(parserConfig()))
    for component in config.components:
        FRB.CppStrategyOverrides.setFixer(SrcName, component.modTypeName, FRB.makeGIMIComponentFixer(config, component.name))

    folder = os.path.abspath(args.mod)
    if (not os.path.isdir(folder)):
        raise SystemExit(f"no such mod folder: {folder}")
    try:
        # The classifier decides which .ini files are Neuvillette's, as it will for the compiled fix.
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

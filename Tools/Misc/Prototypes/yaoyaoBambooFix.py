#
# ===== yaoyaoBambooFix (prototype) =====
#
# Yaoyao -> YaoyaoBamboo ("Rainlit Bamboo Reverie", 6.3), as NOTHING BUT A CONFIG: a GIMICharParserConfig and a
# GIMIComponentFixerConfig handed to the bound makeGIMICharParser / makeGIMIComponentFixer -- the same factories the
# compiled rows will call -- registered on CppStrategyOverrides and run by RemapService. So the port is a
# transcription of the two configs. The Neuvillette -> NeuvilletteMelusent prototype is the template: the skin is the
# same shape (an unnamed main mesh, a Bang, an Eye), with no Coat and no Dress.
#
#   py -3 yaoyaoBambooFix.py <mod folder>                  fix every Yaoyao .ini under the folder
#   py -3 yaoyaoBambooFix.py <mod folder> --download disabled --verbose
#
# Every value below was read off the two frame dumps of the outfit shop's previews
# (FrameAnalysis-Yaoyao-2026-09-27-093623, FrameAnalysis-YaoyaoBamboo-2026-09-27-093332) with
# Tools/Misc/Diagnostics/giDrawTable.py, and off the two download folders.
#
# ---- What the dumps say (2026-09-27) ----
#
#   * Yaoyao is one mesh: Head (first index 0), Body (21678), both on the PLAIN shader pair vs ca90e47a /
#     ps cfb7d837 (diffuse, light map at ps-t0 / ps-t1; in the 6.x G-buffer pass vs 422d7a0f they are light map,
#     diffuse -- LDX). She has NO normal map: the 0fa35364 her asset hash.json calls one is a global texture bound at
#     ps-t6 on every draw of the frame, the face's too. Her face diffuse c70ae897 is bound at ps-t1 (GI 6.x), on the
#     unskinned face meshes (9b6aa8c5 / 7ee0568d / 4d0c13fe) both characters draw.
#   * The skin is three skinned components -- an UNNAMED main mesh, a Bang, an Eye:
#       main Head   0      vs 2c157719 / ps 92544cbc, LND   its own set (hair, face skin, the crate, the umbrella)
#       main Body   43092  vs 2c157719 / ps 6546504e, LND   its own set (coat, shorts, legs)
#       Bang A      0      vs 2c157719 / ps 92544cbc, LND   the Head's set
#       Eye  A      0      vs 95aa6cdb (plain), LD         the Head's diffuse / light map (plus a flat 128 x 128 at t2)
#     and its face diffuse is c70ae897 too -- the SAME texture as hers, on the same shared face meshes, so a mod's
#     face section needs no hash remap at all.
#
# ---- Which slot ----
#
# Her head (hair, face skin, the basket's rabbit) through the main Head slot, her body through the main Body slot:
# the same kind of part on each side. Whatever of her lands on the Bang / Eye bones is drawn by that component's one
# slot. The main mesh's component name is the EMPTY string (its files are YaoyaoBambooHead.ib,
# YaoyaoBambooPosition.buf); only its fix-target id, YaoyaoBambooMain, carries a name.
#
# ---- Band legends (light map alpha, over each whole atlas; mean diffuse under the band) ----
#
#   Yaoyao head    255 hair AND face skin (96%), 254 a little
#   Yaoyao body    255 SKIN (249,228,210), 126-128 brown cloth, 0 cream / white cloth
#   skin head      126-128 hair (brown), 255 pale cloth / puffballs, 176-177 gold, 78 dark teal, 0 green cloth
#   skin body      255 SKIN (246,216,196), 126-128 gold ribbon, 76-78 cyan coat, 0 dark cloth
#
# They agree on skin (255) and differ on hair (her 255 against the skin's 126-128) and on 126-128 (her brown cloth,
# the skin's gold ribbon). No band move until the game says one is needed (Charlotte's rule).
#
# ---- What the game has said so far (her identity mod and ten real mods, 2026-09-27) ----
#
#   * HER HAIR AND BELLS CAME OUT PALE, and on a blonde mod (Yaoyao5) grey-green. Two causes, both on her head, each
#     settled by one hand edit of a fixed .ini before it went in:
#       - the light map's ALPHA BAND: her whole head atlas is 255, which the skin's head reads as its pale green-white
#         puffball flowers; its hair is on 126-128. Moved to 127 the blonde hair came back blonde, and her own brown
#         hair matched her own outfit's (fringe h31 s0.49 v0.71 against h34 s0.46 v0.73). UNGATED -- a skin-colour gate
#         lets blonde hair through, which is why a gated first try measured "no change";
#       - the diffuse ALPHA: the skin's head diffuse is alpha 0 under its hair and hers is 255, and the skin's head
#         shader reads it (Yelan's lesson 7). With the band moved and the alpha left, the bells stayed pale. Alpha 1.
#     The light map's R (hers ~178 over the hair, the skin's ~12) and the normal map's B (the mods' ~0, the skin's
#     ~255) were each tried and changed nothing.
#   * YAOYAO2 DREW ALL THREE OF ITS HAIRSTYLES AT ONCE (the skin's-looking braids, the hair clips gone): the template's
#     unconditional drawindexed = auto after a $Hair if / else if chain with no else -- fillDrawOnlyWhenUndrawn.
#   * YAOYAO3 AND YAOYAO10 CAME OUT VIVID GREEN: a third texture called a normal map at ps-t2 beside their own NNFix
#     read the section as the normal-map layout, and the light map became the diffuse -- layoutFromOwnFixCall.
#   * Mods whose own outfit is broken by a stale 4.0 hash (Yaoyao4, 7, 9: ib 54c0b1e8) render right on the skin.
#   * Yaoyao5's long hair shows small dark shards on the skin, and they are NOT the basket: they are the OUTLINE pass. Her
#     long hair is built in close layers, and the skin's outline shell sits further out than hers, so the inner layers'
#     shell pokes through the outer layer (2026-09-27). Found by elimination in game: the shards stayed with the whole
#     back hair rigid on the head bone (--vgMove 0:20,1:20,2:20,3:20; her rest pose renders clean), with a flat light
#     map, diffuse alpha 0, the vertex colour put back, and with the head painted in flat ID colours; they belong to the
#     head's draw (purpleSlot.py), and went with `if vs != 037730.0` round its drawindexed -- which also drops every
#     mod's hair outline. Vertex colour B moves them (255 worse, 0 fewer). KEPT, by the maintainer's choice: one mod of
#     ten shows it, and skipping the outline changes all of them.
#
# ---- Library gaps found, all filled in SHARED code, each default off so no earlier character moves ----
#
#   * RegFillMissing::onlyWhenAbsent / GIMIComponentFixerConfig::fillDrawOnlyWhenUndrawn (Yaoyao2). Asked PER ROOT and
#     of the parts the edit's filter accepts (the audit: asked of the whole graph, a second root that draws hid the
#     first's object, and a `run =` list another object shares counted its draw).
#   * GIMIComponentFixerConfig::layoutFromOwnFixCall (Yaoyao3, Yaoyao10) -- and such a section's OWN ps-t2 is dropped
#     before the shift puts the light map there: a mod writing `ps-t2 = ...NormalMap` ahead of its ps-t1 lost its light
#     map to the fake normal map (proved on a reordered Yaoyao3, the audit's scenario).
#   * A bound C++ texture filter (CppTransparencyAdjustFilter and four more) raised on a CppTextureFile made inside
#     C++ -- its binding reached for the pure-Python TextureFile's `img` (PyTexFilterCommon.cpp). Fixed; this prototype
#     still sets the head alpha over the pixel bytes, which is exactly what the compiled TexEditor::setTransparency does.
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

SrcName = "Yaoyao"
Skin = "YaoyaoBamboo"

# The skin's main-mesh slots: name -> match_first_index (the GAME model's, off the frame dump)
MainSlots = {"Head": "0", "Body": "43092"}


def parserConfig() -> "FRB.GIMICharParserConfig":
    config = FRB.GIMICharParserConfig()
    config.modTypeId = int(FRB.ModTypeId.Yaoyao)
    config.downloadCharFolder = "Yaoyao"
    config.downloadVersionFolder = "4_0"
    config.downloadPrefix = "Yaoyao"
    config.drawnObjs = ["head", "body"]
    config.texcoordStride = 12       # YaoyaoTexcoord.buf: 201468 / 16789
    # Both her objects are the plain layout (diffuse ps-t0, light map ps-t1). A mod that writes one on the normal-map
    # layout brings its own set: a section binding ANY of the three registers downloads nothing (Neuvillette's point 1).
    cover = ["ps-t0", "ps-t1", "ps-t2"]
    config.objDownloadRegs = [FRB.GIMICharParserConfig.ObjDownloadRegs("head", "ps-t0", "ps-t1", "", cover),
                              FRB.GIMICharParserConfig.ObjDownloadRegs("body", "ps-t0", "ps-t1", "", cover)]
    # The face meshes are shared and a face section exists only because a mod overrides the face: the download could
    # only ever fire wrongly (Citlali's reasoning, GIMICharParserConfig::faceDownload).
    config.faceDownload = False
    return config


def fixerConfig(headHairBand = 127, headAlphaOne = True, headLightR = None, headBandGate = False) -> "FRB.GIMIComponentFixerConfig":
    config = FRB.GIMIComponentFixerConfig()
    config.targetSkin = Skin
    config.drawnObjs = ["head", "body"]

    # Per object: a section binding ps-t2 is on the normal-map layout, otherwise plain and shifted up onto the skin's
    # normal-map slots with a flat normal map.
    config.sourceLayout = FRB.GIMIComponentFixerConfig.SourceLayout.Detect
    # ...but a section rendering through its OWN NNFix is plain whatever it binds at ps-t2: Yaoyao3 and Yaoyao10 bind a
    # third texture they call a normal map beside NNFix, and read by ps-t2 their light maps became the skin's diffuse
    # (the whole outfit vivid green, 2026-09-27).
    config.layoutFromOwnFixCall = True
    # ...and each binding read by its resource NAME first (believed only as texRegsByName's rule allows).
    config.texRegsByName = True
    # Both characters read the face diffuse at ps-t1 (GI 6.x): swap only a mod still on ps-t0.
    config.faceSwapOnlyFromDiffuseReg = True

    main = FRB.GIMIComponentFixerConfig.Component()
    main.name = ""
    main.modTypeName = Skin + "Main"
    main.slot = "Head"
    main.slotIndex = MainSlots["Head"]
    main.objSlotIndices = [("body", MainSlots["Body"])]
    main.slotIndices = list(MainSlots.values())
    main.negativeIndex = False
    main.normalMap = True
    main.face = True
    main.texcoordStride = 12         # YaoyaoBambooTexcoord.buf: 334032 / 27836
    main.slotRegisters = ["ps-t0", "ps-t1", "ps-t2"]

    bang = FRB.GIMIComponentFixerConfig.Component()
    bang.name = "Bang"
    bang.modTypeName = Skin + "Bang"
    bang.slot = "A"
    bang.slotIndex = "0"
    bang.slotIndices = ["0"]
    bang.negativeIndex = False
    bang.normalMap = True
    bang.face = False
    bang.texcoordStride = 12         # YaoyaoBambooBangTexcoord.buf: 33948 / 2829
    bang.slotRegisters = ["ps-t0", "ps-t1", "ps-t2"]

    # Her eyes (vertex groups 4 / 5) are in her HEAD object. The skin draws its Eye on the PLAIN shader 95aa6cdb with
    # a diffuse / light map at ps-t0 / ps-t1 under NNFix.
    eye = FRB.GIMIComponentFixerConfig.Component()
    eye.name = "Eye"
    eye.modTypeName = Skin + "Eye"
    eye.slot = "A"
    eye.slotIndex = "0"
    eye.slotIndices = ["0"]
    eye.negativeIndex = False
    eye.normalMap = False
    eye.face = False
    eye.texcoordStride = 12          # YaoyaoBambooEyeTexcoord.buf: 2856 / 238
    eye.slotRegisters = ["ps-t0", "ps-t1"]
    # Neuvillette's eyes sat 1.24 cm off the skin's; hers are measured by --measureEye (see main()) before any
    # offset goes in. offsetOnlyWithGameFace so a mod that hides the game's face and brings its own is never moved.
    eye.positionOffset = [0.0, 0.0, 0.0]
    eye.offsetOnlyWithGameFace = True

    config.components = [main, bang, eye]
    config.hiddenComponents = []
    # Her face meshes are drawn by the skin under the SAME hashes, so a mod hiding one by hash hides it on the skin
    # too: no side meshes to translate.
    config.sideMeshes = []
    config.unremappedSlots = []

    config.lightMapEdit = None
    if (headHairBand is not None):
        # Her head atlas is ALL band 255 (hair, bells and the little skin it holds), where the skin's head shader reads
        # 255 as its pale cloth / puffballs and keeps its hair on 126-128: move what is not skin onto the hair band.
        Band = FRB.CppMaterialBandRemapFilter.Band
        bands = [Band(254, 255, headHairBand, FRB.CppMaterialBandRemapFilter.skinColoured, True) if headBandGate
                 else Band(254, 255, headHairBand)]
        config.lightMapEdit = lambda diffusePath: FRB.CppMaterialBandRemapFilter(bands, diffusePath)
        config.lightMapObjs = ["head"]
    if (headLightR is not None):
        # EXPERIMENT: her head light map carries R ~178 over the hair, the skin's ~12.
        def setR(texFile):
            import numpy as np
            px = np.frombuffer(texFile.getPixels(), dtype = np.uint8).copy().reshape(-1, 4)
            px[:, 0] = headLightR
            texFile.setPixels(px.tobytes(), texFile.width, texFile.height)
        config.lightMapEdit = lambda diffusePath: setR
        config.lightMapObjs = ["head"]
    # A mod toggling variants of one object on an if / else if chain with no else (Yaoyao2's $Hair) drew EVERY
    # variant at once under the template's unconditional drawindexed = auto.
    config.fillDrawOnlyWhenUndrawn = True
    if (headAlphaOne):
        # The skin's head diffuse is alpha 0 under its hair and hers is 255 everywhere; the skin's head shader reads
        # diffuse alpha (Yelan's lesson 7). Her whole head diffuse is 255, so -254 lands every pixel on 1, Yelan's value.
        # GAP: a bound C++ filter (CppTransparencyAdjustFilter) raises here -- its binding reaches for the pure-Python
        # TextureFile's `img`, which a CppTextureFile made inside the fixer does not have. Until that is fixed, the
        # alpha is set over the pixel bytes directly.
        def alphaOne(texFile):
            import numpy as np
            px = np.frombuffer(texFile.getPixels(), dtype = np.uint8).copy().reshape(-1, 4)
            px[:, 3] = 1
            texFile.setPixels(px.tobytes(), texFile.width, texFile.height)
        config.diffuseEdits = [("head", alphaOne)]
    config.compressTextures = False  # a band selector is exact; BC7 would move it
    return config


# Her back assembly -- the basket (0-3), its rim knobs (19 / 24), the rabbit (20-23) and the Vision hanging from it
# (54-58) -- in the skin's MAIN mesh row, for an A/B of which skin bone it rides.
BackGroups = [0, 1, 2, 3, 19, 20, 21, 22, 23, 24, 54, 55, 56, 57, 58]


def applyVgMoves(moves: dict, toComp: str = ""):
    """Moves source groups onto target component `toComp`: removed from whichever forward row holds them, added to
    that component's row. Rows are REPLACED through VGRemaps.addRows (same key, same versions)."""
    modType = FRB.GIBuilder.yaoyao()
    rows = {}
    for comp in ("", "Bang", "Eye"):
        remap = modType.getVGRemap(Skin, fromComp = "", toComp = comp)
        rows[comp] = {} if remap is None else dict(remap.remap)
    for group, target in moves.items():
        for comp in rows:
            rows[comp].pop(group, None)
        rows[toComp][group] = target
    assert sorted(g for r in rows.values() for g in r) == list(range(112)), "every source group maps exactly once"
    table = modType.vgRemaps
    for comp, remap in rows.items():
        table.addRows([(["1.0", SrcName, "", "6.3", Skin, comp], remap)])


def main():
    parser = argparse.ArgumentParser(description = "Yaoyao -> YaoyaoBamboo, as a component-template config")
    parser.add_argument("mod", help = "the mod folder (every Yaoyao .ini under it is fixed)")
    parser.add_argument("--headHairBand", type = int, default = 127, help = "the band the head light map's 254-255 moves onto (default 127, the skin's hair band; -1 for none)")
    parser.add_argument("--headBandGate", action = "store_true", help = "move only where the diffuse is NOT skin-coloured (an experiment: blonde hair passes a skin test)")
    parser.add_argument("--noHeadAlpha", action = "store_true", help = "leave the head diffuse's alpha alone (the A/B for the edit below)")
    parser.add_argument("--headLightR", type = int, default = None, help = "set the head light map's R to this (an experiment)")
    parser.add_argument("--backOn", type = int, default = None, help = "put her whole back assembly on this skin main-mesh bone (an A/B; default: the VGRemapData rows)")
    parser.add_argument("--vgMove", default = None,
                        help = "source:target main-mesh group moves for an A/B in game, eg. `1:20,2:20` (default: the rows)")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--download", default = None,
                        help = "the API's downloadMode, eg. `disabled` -- omit for the API's own default")
    args = parser.parse_args()

    if (args.backOn is not None):
        applyVgMoves({g: args.backOn for g in BackGroups})
    if (args.vgMove):
        applyVgMoves({int(a): int(b) for a, b in (m.split(":") for m in args.vgMove.split(","))})
    config = fixerConfig(None if args.headHairBand < 0 else args.headHairBand, not args.noHeadAlpha, args.headLightR, args.headBandGate)
    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser(SrcName, FRB.makeGIMICharParser(parserConfig()))
    for component in config.components:
        FRB.CppStrategyOverrides.setFixer(SrcName, component.modTypeName, FRB.makeGIMIComponentFixer(config, component.name))

    folder = os.path.abspath(args.mod)
    if (not os.path.isdir(folder)):
        raise SystemExit(f"no such mod folder: {folder}")
    try:
        # The classifier decides which .ini files are Yaoyao's, as it will for the compiled fix.
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

#
# ===== lumineHeavenFix (prototype) =====
#
# Lumine -> LumineHeaven ("As Heaven and Earth Are Made Anew", 6.3), as NOTHING BUT A CONFIG: a GIMICharParserConfig
# and a GIMIComponentFixerConfig handed to the bound makeGIMICharParser / makeGIMIComponentFixer -- the same factories
# the compiled rows will call -- registered on CppStrategyOverrides and run by RemapService. So the port is a
# transcription of the two configs. The Yaoyao -> YaoyaoBamboo prototype is the template (the skin is the same shape:
# an unnamed main mesh, a Bang, an Eye), with Neuvillette's face handling (the skin has its OWN face meshes and face
# diffuse).
#
#   py -3 lumineHeavenFix.py <mod folder>                  fix every Lumine .ini under the folder
#   py -3 lumineHeavenFix.py <mod folder> --download disabled --verbose
#
# Every value below was read off the two frame dumps (FrameAnalysis-Lumine-2026-09-29-185919, her default outfit in
# the overworld; FrameAnalysis-LumineHeaven-2026-09-29-190651, the skin's preview in the character menu's Dressing
# Room -- the skin is NOT in the outfit shop) with Tools/Misc/Diagnostics/giDrawTable.py, and off the two download
# folders.
#
# ---- What the dumps say (2026-09-29) ----
#
#   * Lumine is one mesh: Head (first index 0), Body (6915), Dress (40413), all PLAIN (6.x G-buffer order: light map
#     ps-t0, diffuse ps-t1 -- LDX); the Dress on its own shader pair (vs d4c01363 / ps 93dcb43f) with the Body's
#     textures. No normal map. Her face diffuse cdabcf6f is bound at ps-t1 on her unskinned face mesh (3049e662).
#   * The skin is three skinned components -- an UNNAMED main mesh, a Bang, an Eye:
#       main Head   0      vs 2c157719 / ps 92544cbc, LND   its own set (hair, the neck scarf, sleeves, the bow)
#       main Body   57141  vs 2c157719 / ps 6546504e, LND   its own set (dress, skirt, legs)
#       Bang A      0      vs 2c157719 / ps 92544cbc, LND   the Head's set
#       Eye  A      0      vs 95aa6cdb / ps 02928611, LD    its OWN small iris atlas (47b6d152 / bde4c76a, a flat
#                                                          white 128 x 128 at t2)
#     and its face is its OWN: diffuse 1e92b57a on face meshes of its own (15825079 / 82d9b411) -- so, as on
#     Neuvillette, a mod's face section is remapped by hash (tex_face_diffuse) and its face / head-upper hides are
#     translated (sideMeshes).
#   * Her eyes sit exactly where the skin's Eye component is (vertex-group centroids within 0.15 mm): no offset.
#
# ---- Which slot ----
#
# Her head (hair, the face skin it holds, her flower) through the main Head slot; her body AND her dress through the
# main Body slot -- the skin has no dress slot, and both are cloth and skin of the body. Whatever of her lands on the
# Bang / Eye bones (her two front bangs, the flower; her eyes) is drawn by that component's one slot.
#
# ---- Band legends (light map alpha over each atlas; mean diffuse under the band) ----
#
#   Lumine head    255 hair (98%; the whole atlas), 254 a little
#   Lumine body    0 white / grey cloth (81%), 255 SKIN (231,213,193), 123-128 gold / brown trims
#   skin head      126-128 HAIR (blonde), 0 the scarf / sleeve cloth, 176-178 gold, 78 dark, 255 a little skin
#   skin body      0 cloth, 78 dark cloth, 255 SKIN (234,207,185), 176-178 gold
#
# They agree on skin (255 on both bodies) and differ on hair: her 255 against the skin's 126-128. And her head diffuse
# is alpha 255 where the skin's is 0 (Yelan's lesson 7). The Yaoyao situation on paper -- but the game disagreed on
# half of it:
#
# ---- What the game has said so far (her identity mod, 2026-09-29, the Dressing Room preview, one variable each) ----
#
#   * HEAD DIFFUSE ALPHA: REQUIRED. Left at her 255, the skin's head shader drew her hair as a glowing ORANGE -- every
#     variant without the edit, whatever the band. Set to 1 (Yaoyao's value) the hair is her own colour.
#   * HEAD LIGHT MAP BAND: NOT WANTED. Moved 254-255 -> 127 (the skin's hair band, Yaoyao's fix) the hair came out a
#     saturated gold next to her own outfit's pale cream; left on 255 it matched. Off by default; --headHairBand 127
#     is the A/B.
#   * THE FACE: the skin draws its OWN face meshes (15825079 / 82d9b411; hers are 3049e662 / 92af2d49), so her face
#     diffuse carried onto the skin's face hash lands on a different mesh. The two atlases share a layout but not the
#     eyes -- hers paints open eye-whites and lash lines for her mesh, the skin's closed lid-lines for its -- and in
#     game her face on the skin lost its lashes and washed out, while the skin's own face with her eyes (the Eye
#     component) looked like her. So NO component carries the face (Component::face = false): on the skin the
#     skin's face is drawn. What that costs: a mod that repaints her face (Lumine1's pale make-up, Lumine5's) keeps
#     the skin's face on the skin; eight of her ten mods ship the vanilla face or none. --carryFace is the A/B.
#
# ---- Library gaps found ----
#
#   * A skin with its OWN face mesh has no way to take a mod's face paint: GIMIComponentFixerConfig carries the face
#     diffuse whole or not at all, and an atlas cannot be moved between two meshes by a hash remap.
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

SrcName = "Lumine"
Skin = "LumineHeaven"

# The skin's main-mesh slots: name -> match_first_index (the GAME model's, off the frame dump)
MainSlots = {"Head": "0", "Body": "57141"}


def parserConfig() -> "FRB.GIMICharParserConfig":
    config = FRB.GIMICharParserConfig()
    config.modTypeId = int(FRB.ModTypeId.Lumine)
    config.downloadCharFolder = "Lumine"
    config.downloadVersionFolder = "4_0"
    config.downloadPrefix = "Lumine"
    config.drawnObjs = ["head", "body", "dress"]
    config.texcoordStride = 20       # LumineTexcoord.buf: 240440 / 12022
    # All three of her objects are the plain layout (diffuse ps-t0, light map ps-t1). A mod that writes one on the
    # normal-map layout brings its own set: a section binding ANY of the three registers downloads nothing
    # (Neuvillette's point 1).
    cover = ["ps-t0", "ps-t1", "ps-t2"]
    config.objDownloadRegs = [FRB.GIMICharParserConfig.ObjDownloadRegs(obj, "ps-t0", "ps-t1", "", cover)
                              for obj in ("head", "body", "dress")]
    # A face section exists only because a mod overrides the face: the download could only ever fire wrongly
    # (Citlali's reasoning, GIMICharParserConfig::faceDownload).
    config.faceDownload = False
    return config


def fixerConfig(headHairBand = None, headAlphaOne = True, innerOutline = False, carryFace = False,
                mirrored = ("dress",), mirrorBackedReach = 0.01) -> "FRB.GIMIComponentFixerConfig":
    config = FRB.GIMIComponentFixerConfig()
    config.targetSkin = Skin
    config.drawnObjs = ["head", "body", "dress"]

    # PLAIN, always: she has no normal map on any object, and every one of her ten mods binds diffuse / light map at
    # ps-t0 / ps-t1 -- while two of them (Lumine4, Lumine6's variants) are the older GIMI shape with a MetalMap /
    # ShadowRamp at ps-t2 / ps-t3 and no fix call. Detect read that ps-t2 as a normal-map layout and put the kimono's
    # light map in the diffuse slot: Lumine4 came out vivid green (in game, 2026-09-29). Bennett's config is Plain for
    # the same reason (SourceLayout's own warning).
    config.sourceLayout = FRB.GIMIComponentFixerConfig.SourceLayout.Plain
    # ...and each binding read by its resource NAME first (believed only as texRegsByName's rule allows).
    config.texRegsByName = True
    # Both characters read the face diffuse at ps-t1 (GI 6.x): swap only a mod still on ps-t0.
    config.faceSwapOnlyFromDiffuseReg = True

    main = FRB.GIMIComponentFixerConfig.Component()
    main.name = ""
    main.modTypeName = Skin + "Main"
    main.slot = "Head"
    main.slotIndex = MainSlots["Head"]
    main.objSlotIndices = [("body", MainSlots["Body"]), ("dress", MainSlots["Body"])]
    main.slotIndices = list(MainSlots.values())
    main.negativeIndex = False
    main.normalMap = True
    main.face = carryFace            # the skin's face is drawn: see the header
    main.texcoordStride = 12         # LumineHeavenTexcoord.buf: 374148 / 31179
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
    bang.texcoordStride = 12         # LumineHeavenBangTexcoord.buf: 32880 / 2740
    bang.slotRegisters = ["ps-t0", "ps-t1", "ps-t2"]

    # Her eyes (vertex groups 5 / 6) are in her HEAD object and UV into her head atlas. The skin draws its Eye on the
    # PLAIN shader 95aa6cdb with a diffuse / light map at ps-t0 / ps-t1 under NNFix: the mod's head set goes there.
    eye = FRB.GIMIComponentFixerConfig.Component()
    eye.name = "Eye"
    eye.modTypeName = Skin + "Eye"
    eye.slot = "A"
    eye.slotIndex = "0"
    eye.slotIndices = ["0"]
    eye.negativeIndex = False
    eye.normalMap = False
    eye.face = False
    eye.texcoordStride = 12          # LumineHeavenEyeTexcoord.buf: 2952 / 246
    eye.slotRegisters = ["ps-t0", "ps-t1"]
    eye.positionOffset = [0.0, 0.0, 0.0]   # measured: her eyes and the skin's coincide within 0.15 mm
    eye.offsetOnlyWithGameFace = True

    # Her DRESS object is single-layer cloth -- her own long back drape, and every mod's coat, cape or skirt tails built
    # on it -- and the skin's Body shader lights its back faces as rim light: Lumine2's coat lining came out bright blue,
    # the drapes of Lumine4 / 7 and her own identity dark, where on her own outfit they are pale (in game, 2026-09-29).
    # A mirrored inner layer (Neuvillette's mirroredObjs) gives the inside a front face: Lumine2's lining came back pale.
    main.mirroredObjs = list(mirrored)
    # ...but not where the mod models its own lining: a twin moved inward from a coat lands in front of the lining a
    # few millimetres behind, as flat grey polygons over Lumine2's coat flaps (in game, 2026-09-29).
    main.mirrorBackedReach = mirrorBackedReach

    if (innerOutline):
        main.innerOutlineObjs = ["head"]
        bang.innerOutlineObjs = ["head"]

    config.components = [main, bang, eye]
    config.hiddenComponents = []
    # The skin draws its OWN face and head-upper meshes: a mod hiding hers by hash (a mask, a custom face) has to hide
    # the skin's too.
    config.sideMeshes = ["ib_face", "ib_headupper"]
    config.unremappedSlots = []

    config.lightMapEdit = None
    if (headHairBand is not None):
        # EXPERIMENT (not wanted, see the header): her head atlas is ALL band 255, the skin's hair is on 126-128.
        Band = FRB.CppMaterialBandRemapFilter.Band
        bands = [Band(254, 255, headHairBand)]
        config.lightMapEdit = lambda diffusePath: FRB.CppMaterialBandRemapFilter(bands, diffusePath)
        config.lightMapObjs = ["head"]
    # A mod toggling variants of one object on an if / else if chain with no else drew EVERY variant at once under the
    # template's unconditional drawindexed = auto (Yaoyao2).
    config.fillDrawOnlyWhenUndrawn = True
    if (headAlphaOne):
        # The skin's head diffuse is alpha 0 and hers is 255 everywhere; the skin's head shader reads diffuse alpha.
        def alphaOne(texFile):
            import numpy as np
            px = np.frombuffer(texFile.getPixels(), dtype = np.uint8).copy().reshape(-1, 4)
            px[:, 3] = 1
            texFile.setPixels(px.tobytes(), texFile.width, texFile.height)
        config.diffuseEdits = [("head", alphaOne)]
    config.compressTextures = False  # a band selector is exact; BC7 would move it
    return config


def main():
    parser = argparse.ArgumentParser(description = "Lumine -> LumineHeaven, as a component-template config")
    parser.add_argument("mod", help = "the mod folder (every Lumine .ini under it is fixed)")
    parser.add_argument("--headHairBand", type = int, default = -1, help = "move the head light map's 254-255 onto this band (an A/B: 127 is the skin's hair band; default -1, none)")
    parser.add_argument("--mirror", default = "dress", help = "comma-separated source objects given a mirrored inner layer on the main mesh (default `dress`; `none` for the A/B)")
    parser.add_argument("--mirrorBackedReach", type = float, default = 0.01, help = "a mirrored triangle with a layer facing the other way this close behind it gets no twin (default 0.01; 0 mirrors every triangle, the A/B)")
    parser.add_argument("--carryFace", action = "store_true", help = "an A/B: carry the mod's face diffuse onto the skin's face hash (it lands on a different mesh)")
    parser.add_argument("--noHeadAlpha", action = "store_true", help = "leave the head diffuse's alpha alone (the A/B for the edit)")
    parser.add_argument("--innerOutline", action = "store_true", help = "drop the outline of the hair's inner layers (Yaoyao's layered-hair fix)")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--download", default = None,
                        help = "the API's downloadMode, eg. `disabled` -- omit for the API's own default")
    args = parser.parse_args()

    config = fixerConfig(None if args.headHairBand < 0 else args.headHairBand, not args.noHeadAlpha, args.innerOutline, args.carryFace,
                         [o for o in args.mirror.split(",") if o and o != "none"], args.mirrorBackedReach)
    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser(SrcName, FRB.makeGIMICharParser(parserConfig()))
    for component in config.components:
        FRB.CppStrategyOverrides.setFixer(SrcName, component.modTypeName, FRB.makeGIMIComponentFixer(config, component.name))

    folder = os.path.abspath(args.mod)
    if (not os.path.isdir(folder)):
        raise SystemExit(f"no such mod folder: {folder}")
    try:
        # The classifier decides which .ini files are Lumine's, as it will for the compiled fix.
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

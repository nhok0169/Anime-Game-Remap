#
# ===== aetherHeavenFix (prototype) =====
#
# Aether -> AetherHeaven ("As Heaven and Earth Are Made Anew", 6.3), as NOTHING BUT A CONFIG: a GIMICharParserConfig
# and a GIMIComponentFixerConfig handed to the bound makeGIMICharParser / makeGIMIComponentFixer -- the same factories
# the compiled rows will call -- registered on CppStrategyOverrides and run by RemapService. So the port is a
# transcription of the two configs. The Lumine -> LumineHeaven prototype (lumineHeavenFix.py) is the template: the
# two Travelers' skins are one outfit set, the same shape (an unnamed main mesh, a Bang, an Eye), and Aether is Lumine
# with one object fewer (no Dress).
#
#   py -3 aetherHeavenFix.py <mod folder>                  fix every Aether .ini under the folder
#   py -3 aetherHeavenFix.py <mod folder> --download disabled --verbose
#
# Every value below was read off the two frame dumps (FrameAnalysis-Aether-2026-10-24-164115, his default outfit in
# the character menu's Dressing Room; FrameAnalysis-AetherHeaven-2026-10-24-164142, the skin's preview there -- the
# skin is not in the outfit shop) and off the two download folders.
#
# ---- What the dumps say (2026-10-24) ----
#
#   * Aether is one mesh: Head (first index 0), Body (8874), both PLAIN (6.x G-buffer order: light map ps-t0, diffuse
#     ps-t1, the Body's metal map at ps-t2 -- LDX), vs 95aa6cdb. No normal map. His face diffuse 50c169e8 is bound at
#     ps-t1 on his unskinned face mesh (3e0837e2).
#   * The skin is three skinned components -- an UNNAMED main mesh, a Bang, an Eye:
#       main Head   0      vs 2c157719 / ps 92544cbc, LND   its own set (the braid, the coat's upper half, the legs)
#       main Body   31407  vs 2c157719 / ps 6546504e, LND   its own set (the coat, the pauldron, the long drape)
#       Bang A      0      vs 2c157719 / ps 92544cbc, LND   the Head's set
#       Eye  A      0      vs 95aa6cdb / ps aa207905, LD    its OWN small iris atlas (47b6d152 light map / a35f0697)
#     and its face is its OWN: diffuse b921d262 on face meshes of its own (013d13f1 / 3d359360; his are 3e0837e2 /
#     3ff23b6d), so, as on LumineHeaven, no component carries a mod's face, and a mod's face / head-upper hides are
#     translated (sideMeshes).
#   * His eyes sit where the skin's Eye component is (vertex-group centroids within 0.7 mm), and the two models' soles
#     agree within 0.3 mm (-0.0065 against -0.0068): no position offset either way.
#
# ---- Which slot ----
#
# His head (hair, the face skin it holds) through the main Head slot; his body (with its cape and scarf) through the
# main Body slot. Whatever of him lands on the Bang / Eye bones (his side locks and ahoge; his eyes) is drawn by that
# component's one slot.
#
# ---- Band legends (light map alpha over each atlas; mean diffuse under the band) ----
#
#   Aether head    255 hair (96%; the whole atlas), 254 a little -- and diffuse alpha 255 EVERYWHERE
#   Aether body    0 dark cloth (48%), 126-128 gold / pale cloth (38%), 255 SKIN (245,221,197); alpha 255 on 1.7%
#   skin head      0 HAIR (blonde, 59%), 76-78 dark, 128-129 grey cloth, 255 a little
#   skin body      0 cloth, 76 dark, 178-181 gold, 128 blue, 255 SKIN (241,206,180)
#
# Both bodies mark skin with 255. The heads differ (his hair 255, the skin's 0), the head diffuse alpha is Lumine's
# situation exactly (255 against the skin's 0: Lumine's hair glowed orange until it was set to 1), and his body's
# alpha-255 pixels are glow, which the skin's body shader tints its own warm gold (Lumine10). So the Lumine edits are
# the starting point -- head alpha to 1, cool glows cleared -- and the head band move Lumine REJECTED in game is off
# here too (--headHairBand is the A/B). All three are checked again in game, not inherited.
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

# the share of each back-cape link on the skin's back coat chain -- see fixerConfig; -1 for the plain row
CapeChainShare = 0.5
# EXPERIMENT: the lowest cape links' own share (49 / 28 / 55 / 27); -1 the same as CapeChainShare
CapeTipShare = -1.0

# a light map texel counts as COOL when its diffuse is this much bluer than red -- see fixerConfig's coolBands
CoolHueMargin = 4

SrcName = "Aether"
Skin = "AetherHeaven"

# The skin's main-mesh slots: name -> match_first_index (the GAME model's, off the frame dump)
MainSlots = {"Head": "0", "Body": "31407"}


def parserConfig() -> "FRB.GIMICharParserConfig":
    config = FRB.GIMICharParserConfig()
    config.modTypeId = int(FRB.ModTypeId.Aether)
    config.downloadCharFolder = "Aether"
    config.downloadVersionFolder = "4_0"
    config.downloadPrefix = "Aether"
    config.drawnObjs = ["head", "body"]
    config.texcoordStride = 12       # AetherTexcoord.buf: 154752 / 12896
    # Both of his objects are the plain layout (diffuse ps-t0, light map ps-t1 in a mod). A mod that writes one on the
    # normal-map layout brings its own set: a section binding ANY of the three registers downloads nothing.
    cover = ["ps-t0", "ps-t1", "ps-t2"]
    config.objDownloadRegs = [FRB.GIMICharParserConfig.ObjDownloadRegs(obj, "ps-t0", "ps-t1", "", cover)
                              for obj in ("head", "body")]
    # A face section exists only because a mod overrides the face: the download could only ever fire wrongly.
    config.faceDownload = False
    return config


InnerOutlineAxis = True
CoolGlowGainFull, CoolGlowGainTop, CoolGlowDarkMargin = 64, 192, 12     # LumineFixer.cpp's, overridden by main()


def fixerConfig(headHairBand = None, headAlphaOne = True, innerOutline = ("body",), carryFace = False,
                mirrored = (), mirrorBackedReach = 0.01, coolGlow = 60, coolGlowGain = 2.2, bangSeam = True, betweenBands = True, coolBands = True) -> "FRB.GIMIComponentFixerConfig":
    config = FRB.GIMIComponentFixerConfig()
    config.targetSkin = Skin
    config.drawnObjs = ["head", "body"]

    # PLAIN, always: he has no normal map on any object, and an older GIMI mod's MetalMap / ShadowRamp at ps-t2 / ps-t3
    # would read as a normal-map layout to Detect (Lumine point 3: her kimono went vivid green).
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
    main.objSlotIndices = [("body", MainSlots["Body"])]
    main.slotIndices = list(MainSlots.values())
    main.negativeIndex = False
    main.normalMap = True
    main.face = carryFace            # the skin's face is drawn: see the header
    main.texcoordStride = 12         # AetherHeavenTexcoord.buf: 317052 / 26421
    main.slotRegisters = ["ps-t0", "ps-t1", "ps-t2"]
    # His back cape, link for link, SHARED between the skin's back coat chain (CapeChainShare) and its upper torso (50)
    # (2026-10-25, three in-game rounds). On the chain alone his cape's two panels closed over the split between them:
    # its top links meet at the skin's centre seam. On the torso alone they kept the gap they are MODELLED with, which
    # is far wider than his own card shows once his cape chains have pulled it in. 50 / 50 gives his slit, a cloak's
    # (Aether2, Aether3) included: 70 / 30 overlapped the panels below the waist (epoch 5), 60 / 40 left a cloak's slit a
    # hairline (epoch 6), 40 / 60 opened the cape wider than his card.
    capeShare = CapeChainShare
    if (capeShare >= 0):
        chain = {42: 74, 44: 74, 43: 75, 50: 75, 45: 76, 46: 76, 51: 77, 52: 77,
                 47: 78, 48: 78, 53: 79, 54: 79, 49: 80, 28: 80, 55: 81, 27: 81}
        tips = {49, 28, 55, 27}
        share = lambda g: CapeTipShare if (g in tips and CapeTipShare >= 0) else capeShare
        main.splitGroups = {g: [(b, share(g)), (50, 1.0 - share(g))] for g, b in chain.items()}

    bang = FRB.GIMIComponentFixerConfig.Component()
    bang.name = "Bang"
    bang.modTypeName = Skin + "Bang"
    bang.slot = "A"
    bang.slotIndex = "0"
    bang.slotIndices = ["0"]
    bang.negativeIndex = False
    bang.normalMap = True
    bang.face = False
    bang.texcoordStride = 12         # AetherHeavenBangTexcoord.buf: 38316 / 3193
    bang.slotRegisters = ["ps-t0", "ps-t1", "ps-t2"]
    if (bangSeam):
        # His side locks and ahoge go to the Bang's lock bones and his head stays on the main mesh's head, so the scalp
        # where they blend into his head is a HARD switch between two components: 28 vertices sit at one point in both
        # buffers, bound to main 4 on one side and Bang 13 / 15 / 8 on the other (measured on his identity mod). That
        # opens as a rip when the locks swing (Citlali's temple, CitlaliFixer.cpp). The Bang draws a ring of the main
        # mesh's triangles too, and those take his head's weight on the Bang's own head-parented bulk (Bang:0, which
        # the reverse row sends back to his head) instead of renormalising onto the lock chain.
        bang.overlapRings = 1
        bang.standIns = {2: 0}

    # His eyes (vertex groups 0 / 1) are in his HEAD object and UV into his head atlas. The skin draws its Eye on the
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
    eye.texcoordStride = 12          # AetherHeavenEyeTexcoord.buf: 4800 / 400
    eye.slotRegisters = ["ps-t0", "ps-t1"]
    eye.positionOffset = [0.0, 0.0, 0.0]   # measured: his eyes and the skin's coincide within 0.7 mm
    eye.offsetOnlyWithGameFace = True

    # His cape and scarf are single-layer cloth IN HIS BODY OBJECT, so a mirrored inner layer (Lumine's fix for her
    # dress) would twin the whole body; off until the game shows the skin's Body shader lighting their back faces.
    main.mirroredObjs = list(mirrored)
    main.mirrorBackedReach = mirrorBackedReach
    main.mirrorBackUV = True

    if (innerOutline):
        main.innerOutlineObjs = list(innerOutline)
        main.innerOutlineFacingAxis = InnerOutlineAxis
        bang.innerOutlineObjs = [o for o in innerOutline if o == "head"]

    config.components = [main, bang, eye]
    config.hiddenComponents = []
    # The skin draws its OWN face and head-upper meshes: a mod hiding his by hash (a mask, a custom face) has to hide
    # the skin's too.
    config.sideMeshes = ["ib_face", "ib_headupper"]
    config.unremappedSlots = []

    config.lightMapEdit = None
    if (betweenBands):
        # A band BETWEEN his pale cloth (126-128) and the skin's gold (178-181): his shader shades 150-175 like his pale
        # cloth and the skin's shades it from a WARM ramp row. Aether8 paints its white trousers on 163: on the skin
        # their shaded side came out peach where his own card shades them lavender, and with 160-170 moved to 126
        # (only that) they matched his card. Neither bands 0 / 78 / 128 / 255, the light map's R, nor the invented
        # normal map was it (each tested alone on the same mod, 2026-10-24). His vanilla atlases hold 0.01% there.
        Band = FRB.CppMaterialBandRemapFilter.Band
        bands = [Band(150, 175, 126)]
        if (coolBands):
            # A COOL texel on his pale cloth, between or hair band onto the skin's band 0 (2026-10-25). His shader keeps
            # a surface's own hue in shade; the skin's multiplies it by a tinted ramp row -- warm on 126 and 255, cool
            # on 0. So a warm surface (his cream cape, blonde hair) shades right where it is, and a cool one went peach:
            # Aether10's blue-grey mane on 255, a lilac cape on 126 (SynthRecolour). On 0 both matched his card, and
            # moving the warm ones too turned his cream cape lavender (Aether7) and flattened his blonde hair. Cool is
            # bluer than red by a margin, so a neutral grey or white stays put. First match wins: the gated bands lead.
            cool = lambda red, green, blue: blue >= red + CoolHueMargin
            bands = [Band(118, 135, 0, cool), Band(150, 175, 0, cool), Band(240, 255, 0, cool)] + bands
        config.lightMapEdit = lambda diffusePath: FRB.CppMaterialBandRemapFilter(bands, diffusePath)
        config.lightMapObjs = ["head", "body"]
    if (headHairBand is not None):
        # EXPERIMENT (Lumine rejected it in game): his head atlas is all band 254-255, the skin's hair is on band 0.
        Band = FRB.CppMaterialBandRemapFilter.Band
        bands = [Band(254, 255, headHairBand)]
        config.lightMapEdit = lambda diffusePath: FRB.CppMaterialBandRemapFilter(bands, diffusePath)
        config.lightMapObjs = ["head"]
    # A mod toggling variants of one object on an if / else if chain with no else drew EVERY variant at once under the
    # template's unconditional drawindexed = auto (Yaoyao2).
    config.fillDrawOnlyWhenUndrawn = True
    config.diffuseEdits = []
    if (headAlphaOne):
        # The skin's head diffuse is alpha 0 and his is 255 everywhere; the skin's head shader reads diffuse alpha.
        def alphaOne(texFile):
            import numpy as np
            px = np.frombuffer(texFile.getPixels(), dtype = np.uint8).copy().reshape(-1, 4)
            px[:, 3] = 1
            texFile.setPixels(px.tobytes(), texFile.width, texFile.height)
        config.diffuseEdits = [("head", alphaOne)]
    if (coolGlow >= 0):
        # A GLOW the mod paints COOL loses its alpha, its brightness baked into its colour (LumineFixer.cpp's
        # clearCoolGlow, the same constants): the skin's body shader multiplies every glow by its own warm gold.
        def clearCoolGlow(texFile):
            import numpy as np
            px = np.frombuffer(texFile.getPixels(), dtype = np.uint8).copy().reshape(-1, 4)
            rgb = px[:, :3].astype(np.int32)
            peak = rgb.max(axis = 1)
            darkBlue = ((peak > 10) & (rgb[:, 2] >= rgb[:, 0] + CoolGlowDarkMargin) & (rgb[:, 2] >= rgb[:, 1] + CoolGlowDarkMargin)
                        if CoolGlowDarkMargin >= 0 else np.zeros(len(px), bool))
            sel = (px[:, 3] > 200) & (((peak > coolGlow) & (rgb[:, 2] > rgb[:, 0])) | darkBlue)
            px[sel, 3] = 0
            if (coolGlowGain != 1.0):
                peak = rgb[sel].max(axis = 1).astype(np.float64)
                gain = 1 + (coolGlowGain - 1) * np.clip((CoolGlowGainTop - peak) / (CoolGlowGainTop - CoolGlowGainFull), 0, 1)
                px[sel, :3] = np.clip(rgb[sel] * gain[:, None] + 0.5, 0, 255).astype(np.uint8)
            texFile.setPixels(px.tobytes(), texFile.width, texFile.height)
        config.diffuseEdits = list(config.diffuseEdits) + [("body", clearCoolGlow)]
    config.compressTextures = False  # a band selector is exact; BC7 would move it
    return config


def main():
    parser = argparse.ArgumentParser(description = "Aether -> AetherHeaven, as a component-template config")
    parser.add_argument("mod", help = "the mod folder (every Aether .ini under it is fixed)")
    parser.add_argument("--headHairBand", type = int, default = -1, help = "move the head light map's 254-255 onto this band (an A/B: 0 is the skin's hair band; default -1, none)")
    parser.add_argument("--mirror", default = "none", help = "comma-separated source objects given a mirrored inner layer on the main mesh (default `none`; `body` for the A/B)")
    parser.add_argument("--mirrorBackedReach", type = float, default = 0.01, help = "a mirrored triangle with a layer facing the other way this close behind it gets no twin (default 0.01)")
    parser.add_argument("--coolGlow", type = int, default = 60, help = "body diffuse pixels with alpha > 200, brighter than this and bluer than red lose their alpha (the skin glows them gold); -1 off, the A/B")
    parser.add_argument("--coolGlowGain", type = float, default = 2.2, help = "the RGB of a pixel --coolGlow clears is multiplied by this, tapered; 1 off, the A/B")
    parser.add_argument("--coolGlowDarkMargin", type = int, default = 12, help = "a dark alpha-255 texel this much bluer than red and green is cleared too; -1 off")
    parser.add_argument("--coolGlowGainFull", type = int, default = 64, help = "texels this dark (max RGB) take the whole --coolGlowGain")
    parser.add_argument("--coolGlowGainTop", type = int, default = 192, help = "texels this bright (max RGB) take none of it")
    parser.add_argument("--carryFace", action = "store_true", help = "an A/B: carry the mod's face diffuse onto the skin's face hash (it lands on a different mesh)")
    parser.add_argument("--noHeadAlpha", action = "store_true", help = "leave the head diffuse's alpha alone (the A/B for the edit)")
    parser.add_argument("--noInnerOutlineAxis", action = "store_true", help = "an A/B: a triangle facing the vertical axis is NOT taken for an inner layer")
    parser.add_argument("--innerOutline", default = "body", help = "comma-separated source objects whose INNER layers lose their outline (Yaoyao's fix; empty for none)")
    parser.add_argument("--noBetweenBands", action = "store_true", help = "an A/B: leave light map bands 150-175 where they are")
    parser.add_argument("--coolHueMargin", type = int, default = 4, help = "a texel is cool when blue >= red + this (default 4; negative takes in neutral greys / whites)")
    parser.add_argument("--capeChainShare", type = float, default = 0.5, help = "each back-cape link's share on the skin's back coat chain, the rest on its torso (default 0.5; -1 the plain vertex group row)")
    parser.add_argument("--capeTipShare", type = float, default = -1.0, help = "an A/B: the lowest cape links' share on the coat chain (default -1, as --capeChainShare)")
    parser.add_argument("--noCoolBands", action = "store_true", help = "an A/B: leave cool texels on bands 118-135 / 150-175 / 240-255 (no move onto the skin's 0)")
    parser.add_argument("--noBangSeam", action = "store_true", help = "an A/B: no overlap ring / stand-in where his side locks meet his head")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--download", default = None, help = "the API's downloadMode, eg. `disabled` -- omit for the API's own default")
    args = parser.parse_args()
    global CoolHueMargin, CapeChainShare, CapeTipShare
    CapeTipShare = args.capeTipShare
    CoolHueMargin = args.coolHueMargin
    CapeChainShare = args.capeChainShare
    global InnerOutlineAxis, CoolGlowGainFull, CoolGlowGainTop, CoolGlowDarkMargin
    CoolGlowGainFull, CoolGlowGainTop, CoolGlowDarkMargin = args.coolGlowGainFull, args.coolGlowGainTop, args.coolGlowDarkMargin
    InnerOutlineAxis = not args.noInnerOutlineAxis

    config = fixerConfig(None if args.headHairBand < 0 else args.headHairBand, not args.noHeadAlpha,
                         [o for o in args.innerOutline.split(",") if o], args.carryFace,
                         [o for o in args.mirror.split(",") if o and o != "none"], args.mirrorBackedReach,
                         args.coolGlow, args.coolGlowGain, not args.noBangSeam, not args.noBetweenBands, not args.noCoolBands)
    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser(SrcName, FRB.makeGIMICharParser(parserConfig()))
    for component in config.components:
        FRB.CppStrategyOverrides.setFixer(SrcName, component.modTypeName, FRB.makeGIMIComponentFixer(config, component.name))

    folder = os.path.abspath(args.mod)
    if (not os.path.isdir(folder)):
        raise SystemExit(f"no such mod folder: {folder}")
    try:
        # The classifier decides which .ini files are Aether's, as it will for the compiled fix.
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

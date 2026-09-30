#
# ===== lumineFromHeavenFix (prototype) =====
#
# LumineHeaven ("As Heaven and Earth Are Made Anew", 6.3) -> Lumine, as NOTHING BUT A CONFIG: a
# GIMIComponentParserConfig and a GIMIMergeFixerConfig handed to the bound makeGIMIComponentParser /
# makeGIMIMergeFixer -- the same factories the compiled rows will call -- registered on CppStrategyOverrides and run by
# RemapService. The reverse of lumineHeavenFix.py, and the same shape as yaoyaoFromBambooFix.py (a skin of an unnamed
# main mesh, a Bang and an Eye merged onto a one-mesh character); the port is a transcription of the two configs.
#
#   py -3 lumineFromHeavenFix.py <mod folder>                  fix every LumineHeaven .ini under the folder
#   py -3 lumineFromHeavenFix.py <mod folder> --download disabled --verbose
#
# ---- The skin's slots (off FrameAnalysis-LumineHeaven-2026-09-29-190651, giDrawTable.py) ----
#
#   slot        first   draw                              textures it reads
#   main Head   0       vs 2c157719 / ps 92544cbc, LND    its own (hair, the neck scarf, sleeves, the bow)
#   main Body   57141   vs 2c157719 / ps 6546504e, LND    its own (dress, skirt, legs)
#   Bang A      0       vs 2c157719 / ps 92544cbc, LND    the Head's set
#   Eye A       0       plain vs 95aa6cdb, LD             its OWN iris atlas (EyeADiffuse / EyeALightMap)
#
# Counts off the download folder (Data/Mod Downloads/GI/LumineHeaven/6_3): main 31179 vertices (Head 57141 indices,
# Body 47298), Bang 2740 (9096), Eye 246 (720); every Texcoord 12 bytes, where Lumine's is 20.
#
# ---- Where each slot lands ----
#
# Lumine is one mesh of three objects, head / body / dress, all on the PLAIN shader (diffuse, light map; no normal
# map). The Head set (main Head, Bang, Eye) onto her head, the Body set onto her body; her dress receives nothing, and
# the whole-ib skip keeps her own dress hidden (Neuvillette's dress, the same way).
#
# ---- The target's layout ----
#
# PLAIN under NNFix (TargetLayout.Plain, the Yaoyao layout): she reads no normal map, so the skin's is dropped and the
# diffuse / light map shifted down.
#
# ---- The face ----
#
# As in the forward direction, the two skins draw DIFFERENT face meshes (the skin 15825079 / 82d9b411, Lumine
# 3049e662 / 92af2d49) and their face atlases do not correspond (lashes and eyes): the face graph is left alone
# (faceReg empty) and on Lumine her own face is drawn. A mod hiding the skin's face meshes by hash hides hers too
# (sideMeshes).
#
# ---- The diffuse alphas and light map bands, before the game has spoken ----
#
#   skin head  alpha ~0,  hair on band 126-128, scarf / sleeve cloth on 0     Lumine head  alpha 255, all band 255
#   skin body  alpha ~0,  skin on 255, cloth on 0 / 78, gold 176-178          Lumine body  alpha ~0, skin 255, cloth 0
#
# Whether her head shader needs the skin's hair moved onto her 255, or its alpha raised, is for the game to say.
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

SrcName = "LumineHeaven"
DstName = "Lumine"
Prefix = "LumineHeaven"

# (component, slot, first index, layout, target object, the game model's index count, donor)
Slots = [("", "Head", "0", "normal", "head", 57141, ""),
         ("", "Body", "57141", "normal", "body", 47298, ""),
         ("Bang", "A", "0", "normal", "head", 9096, ";Head"),
         ("Eye", "A", "0", "plain", "head", 720, "")]

# component -> (the game model's vertex count, texcoord stride), in MERGE order: the biggest first
Components = {"": (31179, 12), "Bang": (2740, 12), "Eye": (246, 12)}
ModTypeNames = {"": "LumineHeavenMain", "Bang": "LumineHeavenBang", "Eye": "LumineHeavenEye"}


def parserConfig() -> "FRB.GIMIComponentParserConfig":
    config = FRB.GIMIComponentParserConfig()
    config.modTypeId = FRB.ModTypeId.LumineHeaven
    config.downloadCharFolder = "LumineHeaven"
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
            # Lumine reads no normal map, so a borrowing slot needs only the donor's diffuse and light map.
            slot.donorNormalMap = False
            slots.append(slot)
        component.slots = slots
        components.append(component)

    config.components = components

    # A slot written in the GAME's register order: LumineHeaven1's Eye binds only `ps-t1 = ...Diffuse`, where the skin's
    # 6.x eye shader reads the diffuse, and a download decided per register put the game's eye diffuse at ps-t0 beside it
    # -- read by position on Lumine, the game's iris drew and the mod's became the light map (dark eyes, 2026-09-29).
    # LIBRARY GAP, filled: GIMIComponentParserConfig::downloadsByName.
    config.downloadsByName = True
    return config


def alphaSet(value: int):
    def edit(tex: "FRB.CppTextureFile"):
        px = bytearray(tex.getPixels())
        px[3::4] = bytes([value]) * (len(px) // 4)
        tex.setPixels(bytes(px), tex.width, tex.height)
    return edit


def fixerConfig(headAlpha = None, headHairBand = None) -> "FRB.GIMIMergeFixerConfig":
    config = FRB.GIMIMergeFixerConfig()

    components = []
    for name, (vertexCount, _) in Components.items():
        component = FRB.GIMIMergeFixerConfig.Component()
        component.name = name
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

    config.components = components
    config.targetObjs = ["head", "body", "dress"]
    config.targetLayout = FRB.GIMIMergeFixerConfig.TargetLayout.Plain
    config.texRegsByName = True
    config.downloadPrefix = Prefix

    # Lumine's Texcoord is 20 bytes a vertex (a second UV set); every one of the skin's components carries 12. The
    # merge pads each line at its end up to her width, and the copied section declares it.
    config.texcoordStride = 20

    # The face graph is left alone: see the header.
    config.faceReg = ""

    # a mod hiding the skin's face meshes by hash hides hers too; her unreached dress withdraws a TexFx request
    config.sideMeshes = ["ib_face", "ib_headupper"]
    config.texFxGuardUnreached = True

    config.lightMapEdit = None
    if (headHairBand is not None):
        # EXPERIMENT: the skin's head puts hair on 126-128; hers reads hair on 255.
        Band = FRB.CppMaterialBandRemapFilter.Band
        bands = [Band(126, 128, headHairBand)]
        # (the merge's light map edit runs on every drawn object; the skin's body has 126-128 on 0.7%)
        config.lightMapEdit = lambda diffusePath: FRB.CppMaterialBandRemapFilter(bands, diffusePath)
    if (headAlpha is not None):
        # EXPERIMENT: the skin's head diffuse is alpha ~0, hers 255.
        config.diffuseEdits = [("head", alphaSet(headAlpha))]
    config.compressTextures = False
    return config


def main():
    parser = argparse.ArgumentParser(description = "LumineHeaven -> Lumine, as a merge-template config")
    parser.add_argument("mod", help = "the mod folder (every LumineHeaven .ini under it is fixed)")
    parser.add_argument("--headAlpha", type = int, default = None, help = "an A/B: set the head diffuse's alpha to this")
    parser.add_argument("--headHairBand", type = int, default = None, help = "an A/B: move the head light map's 126-128 onto this band")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--download", default = None,
                        help = "the API's downloadMode, eg. `disabled` -- omit for the API's own default")
    args = parser.parse_args()

    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser(SrcName, FRB.makeGIMIComponentParser(parserConfig()))
    FRB.CppStrategyOverrides.setFixer(SrcName, DstName, FRB.makeGIMIMergeFixer(fixerConfig(args.headAlpha, args.headHairBand)))

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

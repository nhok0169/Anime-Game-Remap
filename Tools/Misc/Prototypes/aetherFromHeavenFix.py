#
# ===== aetherFromHeavenFix (prototype) =====
#
# AetherHeaven ("As Heaven and Earth Are Made Anew", 6.3) -> Aether, as NOTHING BUT A CONFIG: a
# GIMIComponentParserConfig and a GIMIMergeFixerConfig handed to the bound makeGIMIComponentParser /
# makeGIMIMergeFixer -- the same factories the compiled rows will call -- registered on CppStrategyOverrides and run by
# RemapService. The reverse of aetherHeavenFix.py, and lumineFromHeavenFix.py with one target object fewer (Aether has
# no dress); the port is a transcription of the two configs.
#
#   py -3 aetherFromHeavenFix.py <mod folder>                  fix every AetherHeaven .ini under the folder
#   py -3 aetherFromHeavenFix.py <mod folder> --download disabled --verbose
#
# ---- The skin's slots (off FrameAnalysis-AetherHeaven-2026-10-24-164142) ----
#
#   slot        first   draw                              textures it reads
#   main Head   0       vs 2c157719 / ps 92544cbc, LND    its own (the braid, the coat's upper half, the legs)
#   main Body   31407   vs 2c157719 / ps 6546504e, LND    its own (the coat, the pauldron, the long drape)
#   Bang A      0       vs 2c157719 / ps 92544cbc, LND    the Head's set
#   Eye A       0       plain vs 95aa6cdb, LD             its OWN iris atlas (EyeADiffuse / EyeALightMap)
#
# Counts off the download folder (Data/Mod Downloads/GI/AetherHeaven/6_3): main 26421 vertices (Head 31407 indices,
# Body 61062), Bang 3193 (12087), Eye 400 (1176); every Texcoord 12 bytes, and Aether's is 12 too (no padding).
#
# ---- Where each slot lands ----
#
# Aether is one mesh of two objects, head / body, both on the PLAIN shader. The Bang and the Eye onto his head, the
# main Head and the Body onto his BODY, and the main Head's HAIR split off onto his head -- LumineHeaven's lesson
# (2026-09-30): his head draw shades everything as hair and his body draw everything as cloth, and the skin's Head slot
# holds both. Over the skin's own Head slot the hair is band 0 (3961 of 10469 triangles, all but 4 on the braid / head
# bones, diffuse blonde), and the cloth and legs sit on 71-90, 91-140, 141-200 and 201-255 -- so 0-40 takes the hair
# with a margin and touches no cloth.
#
# ---- The target's layout ----
#
# PLAIN (TargetLayout.Plain): he reads no normal map, so the skin's is dropped and the diffuse / light map shifted down.
#
# ---- The face ----
#
# The two skins draw DIFFERENT face meshes (the skin 013d13f1 / 3d359360, Aether 3e0837e2 / 3ff23b6d): the face graph is
# left alone (faceReg empty) and on Aether his own face is drawn. A mod hiding the skin's face meshes by hash hides his
# too (sideMeshes).
#
# ---- The diffuse alphas and light map bands, before the game has spoken ----
#
#   skin head  alpha ~0,  hair on band 0, cloth on 76-78 / 128, legs 255     Aether head  alpha 255, all band 255
#   skin body  alpha ~0,  skin 255, cloth 0 / 76, blue 128, gold 176-183      Aether body  alpha ~0, skin 255, cloth 0 / 126
#
# LumineHeaven -> Lumine needed no texture edit (her head shader rendered the skin's hair band as hair); whether his
# needs the skin's 0 moved onto his 255, or the skin's gold moved off 176, is for the game to say -- --headHairBand and
# --bodyBand are the A/Bs.
#

import argparse
import os
import sys

Repo = os.environ.get("AG_REMAP_REPO") or os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", ".."))
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
if (not os.path.isdir(APISrc)):
    raise SystemExit(f"the API's package folder is not at {APISrc}; set AG_REMAP_REPO to the repo's path")
sys.path.insert(0, APISrc)
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))

import FixRaidenBoss2 as FRB     # noqa: E402

SrcName = "AetherHeaven"
DstName = "Aether"
Prefix = "AetherHeaven"

# (component, slot, first index, layout, target object, the game model's index count, donor)
Slots = [("", "Head", "0", "normal", "body", 31407, ""),
         ("", "Body", "31407", "normal", "body", 61062, ""),
         ("Bang", "A", "0", "normal", "head", 12087, ";Head"),
         ("Eye", "A", "0", "plain", "head", 1176, "")]

# component -> (the game model's vertex count, texcoord stride), in MERGE order: the biggest first
Components = {"": (26421, 12), "Bang": (3193, 12), "Eye": (400, 12)}
ModTypeNames = {"": "AetherHeavenMain", "Bang": "AetherHeavenBang", "Eye": "AetherHeavenEye"}

# (name, split from, bands, to): the hair of the main Head slot onto his head -- see the header
# how far the skin's left back drape is pulled in at its hem -- see fixerConfig; 0 off
DrapePull = 0.13

# how far the coat's side panels are pulled in at the hem -- see fixerConfig; 0 off
SidePull = 0.0

# the height the drape's pull fades to nothing at -- see fixerConfig
DrapeFadeTop = 1.15

# how far the coat's front panels are pulled in at the hem -- see fixerConfig; 0 off
FrontPull = 0.0

# how far every coat panel (and the drape's lowest links) is pulled in towards his axis at the hem -- see fixerConfig
CoatRadial = 0.07
CoatRadialExtra = [103, 104]

SplitSlots = [("Head#hair", "Head", [(0, 40)], "head")]


def parserConfig() -> "FRB.GIMIComponentParserConfig":
    config = FRB.GIMIComponentParserConfig()
    config.modTypeId = FRB.ModTypeId.AetherHeaven
    config.downloadCharFolder = "AetherHeaven"
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
            # Aether reads no normal map, so a borrowing slot needs only the donor's diffuse and light map.
            slot.donorNormalMap = False
            slots.append(slot)
        component.slots = slots
        components.append(component)

    config.components = components

    # A slot written in the GAME's register order (an Eye binding only `ps-t1 = ...Diffuse`) is read by its names --
    # LumineHeaven's dark eyes, GIMIComponentParserConfig::downloadsByName.
    config.downloadsByName = True
    return config


def alphaSet(value: int):
    def edit(tex: "FRB.CppTextureFile"):
        px = bytearray(tex.getPixels())
        px[3::4] = bytes([value]) * (len(px) // 4)
        tex.setPixels(bytes(px), tex.width, tex.height)
    return edit


def fixerConfig(headAlpha = None, headHairBand = None, bodyBand = None, splitHair = True) -> "FRB.GIMIMergeFixerConfig":
    config = FRB.GIMIMergeFixerConfig()

    components = []
    for name, (vertexCount, _) in Components.items():
        component = FRB.GIMIMergeFixerConfig.Component()
        component.name = name
        component.modTypeName = ModTypeNames[name]
        component.vertexCount = vertexCount
        if (name == "" and DrapePull > 0):
            # The skin's one-sided LEFT BACK DRAPE (99 -> 104, shoulder to shin, with the circle emblem) is MODELLED
            # splayed out to his left, and the skin's simulated bones swing it to the middle of its back. On his left
            # inner cape chain it hung off to his left; on his torso further still (reverse epochs 1-3, 2026-10-25). So it
            # is pulled in, more the lower it hangs: full at its hem (y 0.4), nothing at its root (y 1.15). 7 cm, with the
            # side panels pulled in too (below), put its emblem and pendant where the original's hang (2026-10-25).
            push = FRB.VGPushAway()
            push.groups = [99, 100, 101, 102, 103, 104]
            push.side = -1
            push.from_ = [-3.0, 0.0, -0.15]
            push.distance = DrapePull
            push.fade = [0.4, DrapeFadeTop]
            component.pushAway = [push]
        if (name == "" and SidePull > 0):
            # ...and the coat's side panels (66 / 68 / 70 / 72 on his left, 67 / 69 / 71 / 73 on his right) 8 cm in at the hem:
            # on his outer cape chains they flared wider than on the skin (reverse epochs 1-4); 4 cm still flared.
            pushes = list(component.pushAway)
            panels = [([66, 68, 70, 72], -1, SidePull), ([67, 69, 71, 73], 1, SidePull)]
            if (FrontPull > 0):
                # ...and its front panels (58 / 60 / 62 / 64 his left, 59 / 61 / 63 / 65 his right) 8 cm, on his pelvis and
                # thighs, which part in his idle: the coat's front flared wide where the skin's hangs straight (epoch 5)
                panels += [([58, 60, 62, 64], -1, FrontPull), ([59, 61, 63, 65], 1, FrontPull)]
            for groups, side, distance in panels:
                sidePush = FRB.VGPushAway()
                sidePush.groups = groups
                sidePush.side = side
                sidePush.from_ = [3.0 * side, 0.0, 0.0]
                sidePush.distance = distance
                sidePush.fade = [0.4, 1.0]
                pushes.append(sidePush)
            component.pushAway = pushes
        if (name == "" and CoatRadial > 0):
            # Every coat panel (front 58-65, side 66-73, back 74-81, and the drape's lowest links 103 / 104) pulled in
            # towards his body's axis, more the lower it hangs. The skin's coat is MODELLED wider than its own simulated
            # bones let it hang, and on his bones it keeps that width: its hem flared all round (reverse epochs 1-6). Which
            # bones carried the flare was found by throwing each candidate set aside in game (2026-10-25); the flare sat on
            # several at once, so pulling one set at a time (side panels, front panels) changed nothing visible. 7 cm.
            radial = FRB.VGPushAway()
            radial.groups = list(range(58, 82)) + (CoatRadialExtra or [])
            radial.side = 0
            radial.from_ = [0.0, 0.0, 0.0]
            radial.distance = -CoatRadial
            radial.fade = [0.4, 1.0]
            component.pushAway = list(component.pushAway) + [radial]

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
        if (name == "" and splitHair):
            for splitName, splitFrom, bands, to in SplitSlots:
                slot = FRB.GIMIMergeFixerConfig.Slot()
                slot.name = splitName
                slot.index = next(s[2] for s in Slots if s[0] == name and s[1] == splitFrom)
                slot.to = to
                slot.normalMap = True
                slot.splitFrom = splitFrom
                slot.splitBands = bands
                slot.outline = True
                slots.append(slot)
        component.slots = slots
        components.append(component)

    config.components = components
    config.targetObjs = ["head", "body"]
    config.targetLayout = FRB.GIMIMergeFixerConfig.TargetLayout.Plain
    config.texRegsByName = True
    config.downloadPrefix = Prefix

    # both Texcoords are 12 bytes a vertex
    config.texcoordStride = 12

    # The face graph is left alone: see the header.
    config.faceReg = ""

    # a mod hiding the skin's face meshes by hash hides his too
    config.sideMeshes = ["ib_face", "ib_headupper"]
    config.texFxGuardUnreached = True

    config.lightMapEdit = None
    Band = FRB.CppMaterialBandRemapFilter.Band
    bands = []
    if (headHairBand is not None):
        # EXPERIMENT: the skin's hair is on band 0; his head reads hair on 255.
        bands.append(Band(0, 40, headHairBand))
    if (bodyBand is not None):
        # EXPERIMENT: the skin's gold is on 176-183; his gold / pale cloth is on 126.
        bands.append(Band(170, 190, bodyBand))
    if (bands):
        config.lightMapEdit = lambda diffusePath: FRB.CppMaterialBandRemapFilter(bands, diffusePath)
    if (headAlpha is not None):
        # EXPERIMENT: the skin's head diffuse is alpha ~0, his 255.
        config.diffuseEdits = [("head", alphaSet(headAlpha))]
    config.compressTextures = False
    return config


def main():
    parser = argparse.ArgumentParser(description = "AetherHeaven -> Aether, as a merge-template config")
    parser.add_argument("mod", help = "the mod folder (every AetherHeaven .ini under it is fixed)")
    parser.add_argument("--headAlpha", type = int, default = None, help = "an A/B: set the head diffuse's alpha to this")
    parser.add_argument("--headHairBand", type = int, default = None, help = "an A/B: move the light maps' 0-40 onto this band")
    parser.add_argument("--bodyBand", type = int, default = None, help = "an A/B: move the light maps' 170-190 onto this band")
    parser.add_argument("--headTo", default = "body", help = "the object the main Head slot lands on (default `body`; `head` for the A/B)")
    parser.add_argument("--noSplitHair", action = "store_true", help = "an A/B: keep the main Head slot whole (on --headTo)")
    parser.add_argument("--drapePull", type = float, default = 0.13, help = "how far the skin's left back drape is pulled in at its hem (default 0.07, with --sidePull 0.08; 0 off)")
    parser.add_argument("--sidePull", type = float, default = 0.0, help = "an A/B: pull the coat's side panels in sideways at the hem by this much (default 0; --coatRadial replaced it)")
    parser.add_argument("--drapeFadeTop", type = float, default = 1.15, help = "the height the drape's pull fades to nothing at (default 1.15)")
    parser.add_argument("--frontPull", type = float, default = 0.0, help = "an A/B: pull the coat's front panels in sideways at the hem by this much (default 0; --coatRadial replaced it)")
    parser.add_argument("--coatRadial", type = float, default = 0.07, help = "how far every coat panel is pulled in towards his axis at the hem (default 0.07; 0 off)")
    parser.add_argument("--coatRadialExtra", default = "103,104", help = "more source groups for --coatRadial, comma-separated (default the drape's lowest links, 103,104)")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--download", default = None,
                        help = "the API's downloadMode, eg. `disabled` -- omit for the API's own default")
    args = parser.parse_args()
    Slots[0] = Slots[0][:4] + (args.headTo,) + Slots[0][5:]
    global DrapePull, SidePull, DrapeFadeTop, FrontPull, CoatRadial, CoatRadialExtra
    CoatRadial = args.coatRadial
    CoatRadialExtra = [int(g) for g in args.coatRadialExtra.split(",") if g]
    FrontPull = args.frontPull
    DrapeFadeTop = args.drapeFadeTop
    DrapePull = args.drapePull
    SidePull = args.sidePull

    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser(SrcName, FRB.makeGIMIComponentParser(parserConfig()))
    FRB.CppStrategyOverrides.setFixer(SrcName, DstName, FRB.makeGIMIMergeFixer(
        fixerConfig(args.headAlpha, args.headHairBand, args.bodyBand, not args.noSplitHair)))

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

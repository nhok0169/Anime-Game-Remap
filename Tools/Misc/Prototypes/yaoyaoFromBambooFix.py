#
# ===== yaoyaoFromBambooFix (prototype) =====
#
# YaoyaoBamboo ("Rainlit Bamboo Reverie", 6.3) -> Yaoyao, as NOTHING BUT A CONFIG: a GIMIComponentParserConfig and a
# GIMIMergeFixerConfig handed to the bound makeGIMIComponentParser / makeGIMIMergeFixer -- the same factories the
# compiled rows will call -- registered on CppStrategyOverrides and run by RemapService. The reverse of
# yaoyaoBambooFix.py, and the same shape as neuvilletteFromMelusentFix.py (a skin of an unnamed main mesh, a Bang and
# an Eye merged onto a one-mesh character); the port is a transcription of the two configs.
#
#   py -3 yaoyaoFromBambooFix.py <mod folder>                  fix every YaoyaoBamboo .ini under the folder
#   py -3 yaoyaoFromBambooFix.py <mod folder> --download disabled --verbose
#
# ---- The skin's slots (off FrameAnalysis-YaoyaoBamboo-2026-09-27-093332, giDrawTable.py) ----
#
#   slot        first   draw                              textures it reads
#   main Head   0       vs 2c157719 / ps 92544cbc, LND    its own (hair, face skin, the crate, the umbrella)
#   main Body   43092   vs 2c157719 / ps 6546504e, LND    its own (coat, shorts, legs)
#   Bang A      0       vs 2c157719 / ps 92544cbc, LND    the Head's set
#   Eye A       0       plain vs 95aa6cdb, LD             the Head's diffuse / light map
#
# Counts off the download folder (Data/Mod Downloads/GI/YaoyaoBamboo/6_3): main 27836 vertices (Head 43092 indices,
# Body 61554), Bang 2829 (10713), Eye 238 (696); every Texcoord 12 bytes, as Yaoyao's is.
#
# ---- Where each slot lands ----
#
# Yaoyao is one mesh of two objects, head and body, both on the PLAIN shader (diffuse ps-t0, light map ps-t1, no
# normal map -- see yaoyaoBambooFix.py's "What the dumps say"). The Head set (main Head, Bang, Eye) onto her head, the
# Body set onto her body: each slot goes to the object whose textures it draws with.
#
# ---- The target's layout ----
#
# PLAIN under NNFix (TargetLayout.Plain, the Yelan / Bennett layout): she reads no normal map, so the skin's is
# dropped and the diffuse / light map shifted down.
#
# ---- Library gaps found, in the SHARED merge template ----
#
# Both on the synthetic merged master (yaoyaoBambooSynth.py: the identity beside YaoyaoBamboo1, which is on GIMI's
# newer SetTextures API), and both general to every merged master:
#   * an APPENDED member's bindings were read once, off whichever branch binds each register first -- branch 0's eyes
#     drew on its head LIGHT MAP beside branch 1's. Now resolved per branch (GIMIMergeFixer's rolesInBranch).
#   * a target object only ONE slot lands on (her body) got no draw in a branch that left it to the game's whole-ib
#     draw, because the mod draws it in another branch -- the identity variant had no body. Now drawn per branch.
#
# The diffuse alphas are SWAPPED between the two characters (hers: head 255, body 0; the skin's: head ~0, body 255),
# and the skin's head light map puts hair on 126-128 where hers reads hair on 255 -- whether the reverse needs either
# moved is for the game to say (the forward needed both on the head).

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

SrcName = "YaoyaoBamboo"
DstName = "Yaoyao"
Prefix = "YaoyaoBamboo"

# (component, slot, first index, layout, target object, the game model's index count, donor)
Slots = [("", "Head", "0", "normal", "head", 43092, ""),
         ("", "Body", "43092", "normal", "body", 61554, ""),
         ("Bang", "A", "0", "normal", "head", 10713, ";Head"),
         ("Eye", "A", "0", "plain", "head", 696, ";Head")]

# component -> (the game model's vertex count, texcoord stride), in MERGE order: the biggest first
Components = {"": (27836, 12), "Bang": (2829, 12), "Eye": (238, 12)}
ModTypeNames = {"": "YaoyaoBambooMain", "Bang": "YaoyaoBambooBang", "Eye": "YaoyaoBambooEye"}


def parserConfig() -> "FRB.GIMIComponentParserConfig":
    config = FRB.GIMIComponentParserConfig()
    config.modTypeId = FRB.ModTypeId.YaoyaoBamboo
    config.downloadCharFolder = "YaoyaoBamboo"
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
            # Yaoyao reads no normal map, so a borrowing slot needs only the donor's diffuse and light map.
            slot.donorNormalMap = False
            slots.append(slot)
        component.slots = slots
        components.append(component)

    config.components = components
    return config


def alphaZero(tex: "FRB.CppTextureFile"):
    px = bytearray(tex.getPixels())
    px[3::4] = bytes(len(px) // 4)
    tex.setPixels(bytes(px), tex.width, tex.height)


def fixerConfig(bodyAlphaZero: bool = True) -> "FRB.GIMIMergeFixerConfig":
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
    config.targetObjs = ["head", "body"]
    config.targetLayout = FRB.GIMIMergeFixerConfig.TargetLayout.Plain
    config.texRegsByName = True
    config.downloadPrefix = Prefix
    config.texcoordStride = 12

    # Both characters bind the face diffuse at ps-t1 (GI 6.x), on the SAME shared face meshes and the SAME hash
    # (c70ae897): the mod's own face section already fires on her, so it is copied only when its diffuse has to move
    # -- a copy is a second override on one hash, "Possible Mod Conflict" on every reload (2026-09-27).
    config.faceReg = "ps-t1"
    config.faceOnlyWhenMoved = True

    # Her BODY shader reads the diffuse alpha as a glow: hers is ~0, the skin's 255 all over, and the skin's identity
    # mod came out lit up white from the collar down on her (2026-09-27). Alpha 0 -- settled by one hand edit first.
    # Her head's alpha (hers 255, the skin's ~0) changed nothing measurable (fringe 171.3/136.6/91.0 against
    # 171.5/136.8/90.6), so it is left.
    if bodyAlphaZero:
        config.diffuseEdits = [("body", alphaZero)]

    # No band move: the skin's hair (126-128 on its head light map) renders as hair on her head shader -- the same
    # fringe colour as on the skin's own card within 2%.
    config.lightMapEdit = None
    config.compressTextures = False
    config.texFxGuardUnreached = True
    return config


def main():
    parser = argparse.ArgumentParser(description = "YaoyaoBamboo -> Yaoyao, as a merge-template config")
    parser.add_argument("mod", help = "the mod folder (every YaoyaoBamboo .ini under it is fixed)")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--download", default = None,
                        help = "the API's downloadMode, eg. `disabled` -- omit for the API's own default")
    args = parser.parse_args()

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

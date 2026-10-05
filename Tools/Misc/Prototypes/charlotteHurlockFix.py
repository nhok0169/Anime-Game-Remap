#
# ===== charlotteHurlockFix (prototype) =====
#
# Charlotte -> CharlotteHurlock ("Hurlock Variations", 6.7), as NOTHING BUT A CONFIG: a
# GIMICharParserConfig and a GIMIComponentFixerConfig handed to the bound makeGIMICharParser /
# makeGIMIComponentFixer -- the same factories the compiled rows will call -- registered on
# CppStrategyOverrides and run by RemapService. So the port is a transcription of the two configs.
#
#   py -3 charlotteHurlockFix.py <mod folder>                  fix every Charlotte .ini under the folder
#   py -3 charlotteHurlockFix.py <mod folder> --slot A         draw her head AND body through Body slot A (the old default)
#   py -3 charlotteHurlockFix.py <mod folder> --download disabled --verbose
#
# Every value below was read off the two 6.7 frame dumps (FrameAnalysis-Charlotte-2026-09-23-200020,
# FrameAnalysis-CharlotteHurlock-2026-09-23-195553) with Tools/Misc/Diagnostics/giDrawTable.py and off
# the two identity mods (CharlotteIdentity, CharlotteHurlockIdentity, byte-identical to those dumps).
#
# ---- What the dumps say (2026-09-23) ----
#
#   * Charlotte is one mesh, Head (first index 0) + Body (23271), BOTH drawn by the normal-map shader
#     pair vs 2c157719180b096c / ps 6546504e7a226d3a, bound LND (light map, normal map, diffuse at
#     ps-t0/1/2 in the dump; ps-t0/1/2 = normal map / diffuse / light map in a mod, under ORFix).
#     Her face diffuse (58d9859b) is bound at ps-t1 (GI 6.x) -- and it is the SKIN's face diffuse too.
#   * The skin is four skinned components: Body A (0) / B (53529) / C (99756) / D (103914) / E (104172),
#     Bangs A, Eyes A, Camera A. Body A and the Bangs draw with ps 92544cbc55d1d94b and Body A's
#     textures; Body B, the Camera and C draw with Body B's (B and the Camera on ps 6546504e, C on the
#     plain d4c01363144d79d6); D reads A's; E is a small piece drawn only in two special passes; the
#     Eyes are on the PLAIN shader 95aa6cdb84eb7b99 with Body B's diffuse / light map (LDX).
#
# ---- Which slot: the band legends (light map alpha, over each whole atlas) ----
#
#   Charlotte head   126-128 pink HAIR, 177-178 gold, 255 her dark-red HAT, 0 dark
#   Charlotte body   126-128 red CLOTH, 177-178 gold, 255 SKIN, 0 grey
#   skin Body A      128 pink HAIR, 76-78 beige cloth, 179 white cloth, 255 SKIN, 0 dark
#   skin Body B      128 teal CLOTH, 76-78 beige cloth, 178-181 dark cloth, 255 SKIN, 0 grey
#
# Her head fits slot A band for band (hair on hair), her body fits slot B band for band -- and since
# 2026-09-24 each goes through its OWN slot (`--slot split`, the default; Component.objSlotIndices).
# The slot decides the PIXEL SHADER, which is what settled it: slot A draws on 92544cbc (the skin's
# hair / skin shader), slot B on 6546504e -- Charlotte's own. Her body through slot A looked right on
# her identity mod and put black shards over Charlotte8's dark cardigan; through slot B they are gone.
# `--slot A` / `--slot B` draw both objects through one slot, the earlier choices. No band table.
#
# ---- The camera is NOT hers (the maintainer's call, 2026-09-23) ----
#
# Both characters carry a camera as a mesh of its own: the skin's Camera component, and on base
# Charlotte a separate ib (deb75778, 1435 vertices, textured from her head atlas) that her hash.json
# does not list -- the same size as the skin's but a different mesh. It is an external ACCESSORY she
# uses, not part of the character, so it is not remapped: the skin's Camera component is hidden, and a
# mod's own CharlotteCamera sections (Charlotte3 removes it, Charlotte7 retextures it) stay on base
# Charlotte's camera and do nothing on the skin.
#
# ---- Library gaps this found (each filled in the library, not here) ----
#
#   * GIMIComponentFixerConfig / makeGIMIComponentFixer had no binding (bound 2026-09-23), and
#     GIMICharParserConfig was missing objDownloadRegs, faceDownload and three more (bound too).
#   * A PLAIN slot reached by a NORMAL-MAP-layout object (Charlotte onto the skin's Eyes) left the
#     normal map where the plain shader reads the diffuse. The template now drops it and shifts the
#     diffuse / light map down (GIMIComponentFixer's buildPlainSlotShift) -- the merge template's rule.
#   * STILL A GAP: a band table per OBJECT. Her head's 255 (hat) and her body's 126-128 (cloth) would
#     each want a move on slot A, and the moves are opposite in meaning on the other object (255 is
#     her body's skin, 126-128 her head's hair). lightMapObjs picks the objects ONE edit applies to,
#     so the prototype makes no band move until the game says one is needed.
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

SrcName = "Charlotte"
Skin = "CharlotteHurlock"

# The skin's Body slots: name -> match_first_index (the GAME model's, off hash.json)
BodySlots = {"A": "0", "B": "53529", "C": "99756", "D": "103914", "E": "104172"}


def parserConfig() -> "FRB.GIMICharParserConfig":
    config = FRB.GIMICharParserConfig()
    config.modTypeId = int(FRB.ModTypeId.Charlotte)
    config.downloadCharFolder = "Charlotte"
    config.downloadVersionFolder = "4_0"
    config.downloadPrefix = "Charlotte"
    config.drawnObjs = ["head", "body"]
    config.texcoordStride = 12       # CharlottePosition.buf / Texcoord.buf: 213480 / 17790
    # Both objects on the normal-map layout: diffuse ps-t1, light map ps-t2, normal map ps-t0.
    config.objDownloadRegs = [FRB.GIMICharParserConfig.ObjDownloadRegs("head", "ps-t1", "ps-t2", "ps-t0"),
                              FRB.GIMICharParserConfig.ObjDownloadRegs("body", "ps-t1", "ps-t2", "ps-t0")]
    # Her face and the skin's are the same mesh and the same diffuse (58d9859b), both read at ps-t1:
    # a face section only exists because a mod overrides the face, so the ps-t0 download could only
    # ever fire wrongly (Citlali's reasoning, GIMICharParserConfig::faceDownload).
    config.faceDownload = False
    return config


def fixerConfig(slotMode: str) -> "FRB.GIMIComponentFixerConfig":
    # "split": head through slot A, body through slot B -- see the header.
    bodySlot = "A" if slotMode == "split" else slotMode
    usedSlots = {"A", "B"} if slotMode == "split" else {slotMode}
    config = FRB.GIMIComponentFixerConfig()
    config.targetSkin = Skin
    config.drawnObjs = ["head", "body"]

    # Her own sections are the normal-map layout the skin's Body slots read: pass through; a mod still
    # on the plain layout (no ps-t2) gets the shift up.
    config.sourceLayout = FRB.GIMIComponentFixerConfig.SourceLayout.Detect
    # Both characters read the face diffuse at ps-t1 (GI 6.x): swap only a mod still on ps-t0.
    config.faceSwapOnlyFromDiffuseReg = True

    body = FRB.GIMIComponentFixerConfig.Component()
    body.name = "Body"
    body.modTypeName = Skin + "Body"
    body.slot = bodySlot
    body.slotIndex = BodySlots[bodySlot]
    if slotMode == "split":
        body.objSlotIndices = [("body", BodySlots["B"])]
    body.negativeIndex = False
    body.normalMap = True
    body.face = True
    body.texcoordStride = 20         # the skin's Body Texcoord is 20 (a second UV set)
    body.slotRegisters = ["ps-t0", "ps-t1", "ps-t2"]

    # Her eyes (vertex groups 13 / 14) are in her HEAD object. The skin draws its Eyes on a PLAIN
    # shader with Body B's diffuse / light map at ps-t0 / ps-t1 under NNFix.
    eyes = FRB.GIMIComponentFixerConfig.Component()
    eyes.name = "Eyes"
    eyes.modTypeName = Skin + "Eyes"
    eyes.slot = "A"
    eyes.slotIndex = "0"
    eyes.negativeIndex = False
    eyes.normalMap = False
    eyes.face = False
    eyes.texcoordStride = 12
    eyes.slotRegisters = ["ps-t0", "ps-t1"]

    config.components = [body, eyes]

    # No forward vertex-group row reaches the skin's Bangs (Charlotte's fringe is on her head bone) or
    # its Camera (she has none), so their own geometry would still draw over hers.
    config.hiddenComponents = [Skin + "Bangs", Skin + "Camera"]

    # Every Body slot but the one drawn through, so a TexFx request is not served on one of them.
    config.unremappedSlots = [(Skin + "Body", [index for name, index in BodySlots.items() if name not in usedSlots])]

    config.lightMapEdit = None
    config.compressTextures = False  # a band selector is exact; BC7 would move it
    return config


def main():
    parser = argparse.ArgumentParser(description = "Charlotte -> CharlotteHurlock, as a component-template config")
    parser.add_argument("mod", help = "the mod folder (every Charlotte .ini under it is fixed)")
    parser.add_argument("--slot", default = "split", choices = ["split"] + sorted(BodySlots),
                        help = "split (default): head through Body slot A, body through slot B; "
                               "or one slot for both")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--download", default = None,
                        help = "the API's downloadMode, eg. `disabled` -- omit for the API's own default")
    args = parser.parse_args()

    config = fixerConfig(args.slot)
    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser(SrcName, FRB.makeGIMICharParser(parserConfig()))
    for component in config.components:
        FRB.CppStrategyOverrides.setFixer(SrcName, component.modTypeName, FRB.makeGIMIComponentFixer(config, component.name))

    folder = os.path.abspath(args.mod)
    if (not os.path.isdir(folder)):
        raise SystemExit(f"no such mod folder: {folder}")
    try:
        # The classifier decides which .ini files are Charlotte's, as it will for the compiled fix.
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

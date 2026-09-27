#
# ===== charlotteFromHurlockFix (prototype) =====
#
# CharlotteHurlock -> Charlotte: a skin of FOUR components (Body, Bangs, Eyes, Camera) back onto a
# character of one mesh, as NOTHING BUT A CONFIG -- a GIMIComponentParserConfig handed to
# makeGIMIComponentParser and a GIMIMergeFixerConfig handed to makeGIMIMergeFixer (the merge template
# YelanTranquil, BennettAdventure and CitlaliWhisperofStars are compiled from), registered on
# CppStrategyOverrides and run by RemapService. The port is a transcription of the two configs below
# into IniParseData/CharlotteHurlock/ and IniFixData/CharlotteHurlock/.
#
#   py -3 charlotteFromHurlockFix.py <mod folder>                     fix every CharlotteHurlock .ini under it
#   py -3 charlotteFromHurlockFix.py <mod folder> --download disabled no downloads (a deterministic A/B)
#   py -3 charlotteFromHurlockFix.py <mod folder> --verbose           attach the API's logger
#
# ---- What the 6.7 frame dumps say (2026-09-23) ----
#
# FrameAnalysis-CharlotteHurlock-2026-09-23-195553 through Tools/Misc/Diagnostics/giDrawTable.py:
#
#   slot            first   draw (G-buffer)                        textures it reads
#   Body A          0       vs 2c157719 / ps 92544cbc, LND        its own (hair, skin, some cloth)
#   Body B          53529   vs 2c157719 / ps 6546504e, LND        its own (the outfit)
#   Body C          99756   vs d4c01363 (plain), LD               Body B's diffuse / light map
#   Body D          103914  vs b466a89c, LND                      Body A's set
#   Body E          104172  only a special pass (vs c4a3e42f)     -- a lens; NOT merged, see below
#   Bangs A         0       vs 2c157719 / ps 92544cbc, LND        Body A's set
#   Eyes A          0       vs 95aa6cdb (plain), LD               Body B's diffuse / light map
#   Camera A        0       vs 2c157719 / ps 6546504e, LND        Body B's set -- NOT merged, see below
#
# Charlotte reads normal map / diffuse / light map at ps-t0/1/2 under ORFix on BOTH her objects, on the
# same shader pair (2c157719 / 6546504e): the target's layout is NormalMap, and a plain slot (C, the
# Eyes) is shifted UP by the template.
#
# ---- Where the slots land ----
#
# Charlotte's head and body draw on the SAME shader, so the shader-family rule that sent Citlali's
# fringe to her body does not choose between them; every slot goes to her BODY, as Citlali's did, and
# her head receives nothing (the whole-ib skip keeps her own head hidden). Her light map legend and the
# skin's put hair, skin and cloth on the same bands (see charlotteHurlockFix.py), so there is no band
# move until the game says otherwise.
#
# ---- What is deliberately NOT merged ----
#
#   * The Camera component: an ACCESSORY, not part of the character (the maintainer, 2026-09-23). Base
#     Charlotte's own camera is a separate mesh the game draws on her anyway; a mod's Camera sections
#     are left alone and match nothing on Charlotte.
#   * Body E (504 indices): drawn only in the skin's special pass on shader c4a3e42f -- a lens. Merged
#     into Charlotte's body it would be drawn OPAQUE in her G-buffer pass.
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

SrcName = "CharlotteHurlock"
DstName = "Charlotte"
Prefix = "CharlotteHurlock"

# (component, slot, match_first_index, layout, lands on, the GAME's index count, borrows from)
# Index counts off hash.json's object_index_counts; read only for a slot the mod does not carry.
Slots = [("Body", "A", "0", "normal", "body", 53529, ""),
         ("Body", "B", "53529", "normal", "body", 46227, ""),
         ("Body", "C", "99756", "plain", "body", 4158, "Body;B"),
         ("Body", "D", "103914", "normal", "body", 258, "Body;A"),
         ("Bangs", "A", "0", "normal", "body", 7104, "Body;A"),
         ("Eyes", "A", "0", "plain", "body", 468, "Body;B")]

# The GAME model's vertex count and texcoord stride per component (download Blend.buf bytes / 32,
# Texcoord.buf bytes / vertices).
Components = {"Body": (30214, 20), "Bangs": (1840, 12), "Eyes": (120, 12)}


def parserConfig() -> "FRB.GIMIComponentParserConfig":
    config = FRB.GIMIComponentParserConfig()
    config.modTypeId = FRB.ModTypeId.CharlotteHurlock
    config.downloadCharFolder = "CharlotteHurlock"
    config.downloadVersionFolder = "6_7"
    config.downloadPrefix = Prefix

    components = []
    for name, (vertexCount, texcoordStride) in Components.items():
        component = FRB.GIMIComponentParserConfig.Component()
        component.name = name
        component.modTypeName = SrcName + name
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
            # Charlotte reads normal maps, so a borrowing slot needs the donor's too.
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
            # Every one of the skin's slots outlines with Charlotte's own outline shader (vs 67fd126e in
            # both dumps), so none is kept out of her outline pass.
            slot.outline = True
            slots.append(slot)
        component.slots = slots
        components.append(component)

    config.components = components
    config.targetObjs = ["head", "body"]
    config.targetLayout = FRB.GIMIMergeFixerConfig.TargetLayout.NormalMap

    # Every carried binding onto the register its resource NAME says: the skin's mods bind in ORFix's
    # order (CharlotteHurlock4) or through GIMI's SetTextures API (CharlotteHurlock1, normalized to the
    # same), and a name decides the role either way.
    config.texRegsByName = True
    config.downloadPrefix = Prefix

    # Charlotte binds her face diffuse (58d9859b -- the skin's too) at ps-t1, the GI 6.x layout.
    config.faceReg = "ps-t1"
    config.lightMapEdit = None
    config.compressTextures = False
    return config


def main():
    parser = argparse.ArgumentParser(description = "CharlotteHurlock -> Charlotte, as a merge-template config")
    parser.add_argument("mod", help = "the mod folder (every CharlotteHurlock .ini under it is fixed)")
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

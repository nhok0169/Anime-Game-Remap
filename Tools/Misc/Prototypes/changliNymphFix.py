#
# ===== changliNymphFix (prototype v1) =====
#
# Changli -> ChangliNymph (Wuthering Waves): a character onto her own Laurel Nymph skin, both past 256
# merged bones. Built FROM the library -- the whole fix is a WWMIParserConfig handed to makeWWMIParser and
# a WWMIFixerConfig handed to makeWWMIFixer, the factories every WuWa pair is compiled from, registered
# through CppStrategyOverrides and run by RemapService. So the port is a transcription of the two configs
# below into IniParseData/Changli/ and IniFixData/Changli/.
#
#   py -3 changliNymphFix.py <mod folder>                     fix every Changli .ini under the folder
#   py -3 changliNymphFix.py <mod folder> --download disabled no downloads (for a deterministic A/B)
#   py -3 changliNymphFix.py <mod folder> --verbose           attach the API's logger
#   py -3 changliNymphFix.py <mod folder> --undo              undo a previous fix
#
# ---- The pairing, measured (2026-10-20) ----
#
#   Geometry (per-component overlap of the two download folders' Position.buf, 2 cm) and shader family,
#   off character-menu dumps at 3.7.1 (FrameAnalysis-Changli-2026-10-20-111723, -ChangliNymph-...-111534):
#     0 bangs, 1 hair, 2 face, 6 eyes are the SAME mesh on both (overlap 1.00) and drawn by the same
#       shaders with the same register layout -> the same slot
#     3 her upper body (torso, arms, hands; 9d4c7d96) -> the skin's body slot 4 (14fee50a): 58% of it lies
#       on that slot's surface, 33% under the skin's sheer dress (slot 3, d5572b1c, a shader she has no part for)
#     4 her legs and skirt (fbe2f2f0) -> slot 4 too, 78%; the second claimant lands in a RemapFix1.ini
#     5 her coat, shoulder drapes and side sashes -> slot 5, which the skin draws with the SAME shader
#       (7f639abb); the geometry lies over the skin's shawl (slot 7) and slot 5
#     7 a 25-vertex chest piece she draws with an emissive shader (bb1c5ba6) -> slot 4, where it lies
#   The skin's sheer dress (3) and shawl (7) take nothing and are skipped by the template.
#
# ---- What the library had to gain first (2026-10-20) ----
#
#   * WWMIFixerConfig::targetInfluences. Every earlier pair joined two characters of ONE blend width;
#     she is 4 influences a vertex and her skin 8, and the skin's draws read the blend through their own
#     input layout, so her unwidened blend drew the whole model as giant polygons. Proven by a hand edit
#     (the fix's blend and BlendRemapVertexVG padded to 8, strides 16, the remapper's count 8) before
#     the library changed, then by the same mod through the new field.
#
# GAPS this prototype knows about (each is a note for the port, step 7, or for the in-game audit):
#   * Slot 4's ps-t3 is the skin's UV-mapped glow texture (742c5c7b, shared with her) and her upper / lower
#     body have no counterpart: bound to an invented flat "no glow" texture until the game says otherwise.
#   * Her chest piece (7) loses its emissive shader on the skin's lit body shader.
#   * Shape keys: 72 of her 86 face keys are the skin's slots 14-85 by content; the skin's slots 0-13 (two
#     sets of seven face keys) pair with nothing of hers, and get no offsets (her 72-85 are dropped).
#   * Only her CURRENT skeleton (1.4 on) is read: a 1.1 / 1.2 mod (Changli2, 3, 7, 8, 12, 14) carries a
#     renumbered one (the community wwmi_fix_23's ChangLi / Changli_1.4 remaps), not handled yet.
import argparse
import json
import os
import sys

Here = os.path.dirname(os.path.abspath(__file__))
Repo = os.environ.get("AG_REMAP_REPO") or os.path.normpath(os.path.join(Here, "..", "..", ".."))
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
if (not os.path.isdir(APISrc)):
    raise SystemExit(f"the API's package folder is not at {APISrc}; set AG_REMAP_REPO to the repo's path")
sys.path.insert(0, APISrc)
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))
sys.path.insert(0, os.path.join(Repo, "Tools", "Misc", "Diagnostics"))

import FixRaidenBoss2 as FRB     # noqa: E402

SrcName = "Changli"
DstName = "ChangliNymph"
Downloads = os.path.join(Repo, "Data", "Mod Downloads", "WuWa", "Changli", "3_7")
LiveHashes = os.path.join(Repo, "Data", "Mod Downloads", "WuWa", "Changli", "ChangliLiveHashes.json")
B = FRB.WWMIFixerConfig.Binding


# ---- her textures by the hash the download folder files them under (WWMI-Assets 2.7, which most mods
# carry) -- their LIVE twins (what the game binds at full texture quality, ChangliLiveHashes.json) are
# added below, and the textures only the 3.7 dump holds are filed by their live hash ----
AssetRoles = {
    "25e6decf": "bangsMap", "89da6405": "bangsDiffuse", "7eb470fc": "bangsStrand", "fcec9218": "bangsOutline",
    "0484ab73": "hairMap", "9a3f146d": "hairDiffuse", "fde34d42": "hairStrand",
    "05187cd9": "faceMask", "a97f587b": "faceDiffuse",
    "69994531": "upperNormal", "3ff7cad2": "upperMask", "8407b021": "upperDiffuse", "5ed7e436": "upperDetail",
    "ee867ac2": "lowerNormal", "4da65048": "lowerMask", "3e6d3c8e": "lowerDiffuse",
    "59b52b44": "coatNormal", "258c919f": "coatDiffuse",
    "21f813ba": "glow",
    "1bc98900": "eyeDiffuse", "8383cbbf": "eyeMask",
}
DumpOnlyRoles = {"2ce995ee": "upperRamp", "5a9f3650": "coatOutline", "48e4d268": "hairOutline",
                 "dd9d28ce": "eyeHighlight", "6bf7a371": "eyeIris"}


def liveRoles():
    with open(LiveHashes, encoding = "utf-8") as f:
        live = {k: v for k, v in json.load(f).items() if k != "_"}
    return {live[h]: role for h, role in AssetRoles.items() if h in live}


def allRoles():
    return {**AssetRoles, **liveRoles(), **DumpOnlyRoles}


# ---- which role each of HER registers binds, per component (her own draws, wwmiPassLayout.py) ----
RegisterRoles = {
    0: {"ps-t0": "bangsMap", "ps-t1": "bangsDiffuse", "ps-t2": "bangsStrand"},
    1: {"ps-t0": "hairMap", "ps-t1": "hairDiffuse", "ps-t2": "hairStrand"},
    2: {"ps-t0": "faceMask", "ps-t1": "faceDiffuse"},
    3: {"ps-t0": "upperNormal", "ps-t1": "upperMask", "ps-t2": "upperDiffuse", "ps-t3": "upperDetail", "ps-t4": "upperRamp"},
    4: {"ps-t0": "lowerNormal", "ps-t1": "lowerMask", "ps-t2": "lowerDiffuse"},
    5: {"ps-t0": "coatNormal", "ps-t1": "coatDiffuse"},
    6: {"ps-t0": "eyeIris", "ps-t1": "eyeMask", "ps-t2": "eyeDiffuse"},
    7: {"ps-t0": "glow"},
}

# Textures the two skins share under one hash: the game already binds them on the target
NoFallback = {"hairMap", "eyeDiffuse", "eyeMask", "eyeIris", "glow"}


def thumbprints():
    """Her game textures' 16 x 16 thumbprints, by hash, for a mod file no hash names"""
    import wwmiTextureThumbs
    result = {}
    for h in {**AssetRoles, **DumpOnlyRoles}:
        path = os.path.join(Downloads, f"ChangliTexture{h}.dds")
        if (os.path.isfile(path)):
            thumb = wwmiTextureThumbs.thumbprint(path)
            if (thumb is not None):
                result[h] = thumb
    return result


def textureFacts() -> "FRB.WWMITextureFacts":
    facts = FRB.WWMITextureFacts()
    facts.roles = allRoles()
    facts.registerRoles = RegisterRoles
    facts.textureThumbprints = thumbprints()
    facts.downloadGameFolder = "WuWa"
    facts.downloadCharFolder = "Changli"
    facts.downloadVersionFolder = "3_7"
    facts.downloadPrefix = "Changli"
    files = {**AssetRoles, **DumpOnlyRoles}
    facts.fallbackTextures = {role: h for h, role in files.items() if role not in NoFallback}
    return facts


def parserConfig(facts) -> "FRB.WWMIParserConfig":
    config = FRB.WWMIParserConfig()
    config.version = "1.1"             # her oldest mesh: a mod of any of her three generations classifies
    config.textures = facts
    return config


def fixerConfig(facts, hideShapeKeys: bool = False, noOrder: bool = False, noDispatch: bool = False) -> "FRB.WWMIFixerConfig":
    config = FRB.WWMIFixerConfig()
    config.targetId = FRB.ModTypeId.ChangliNymph
    config.version = "2.4"
    config.sourceVersion = "1.4"
    config.sourceVersionByVb0 = {"77975500": "1.1", "5f8aac45": "1.2", "d14bed8b": "1.4"}

    # ---- the passes the TARGET draws each slot on (her Laurel Nymph dump, wwmiPassLayout.py) ----
    config.slotPasses = [
        ["880e1a81d913db05", "83782bd563d3a6c9"],   # 0 bangs (the second inherits the first's set)
        ["518dadece9a19b08"],                       # 1 hair
        ["ed1c0f8b2ba08ac4"],                       # 2 face
        ["d5572b1cbbba07a4"],                       # 3 sheer dress (nothing of hers lands there)
        ["14fee50a0ce4ec91"],                       # 4 body
        ["7f639abb370f996d"],                       # 5 skirt and veils
        ["5616e4de7d4e70ef"],                       # 6 eyes
        ["43b4972776737988"],                       # 7 shawl (nothing of hers lands there)
    ]

    # ---- every pass gated through its VERTEX shader, so RabbitFX's pixel-shader tags survive ----
    config.passVertexShaders = {
        "880e1a81d913db05": ["a695e6d5f9d489cf"],
        "83782bd563d3a6c9": ["a695e6d5f9d489cf"],
        "518dadece9a19b08": ["e97b4254e5ed8812"],
        "ed1c0f8b2ba08ac4": ["7c0b4db32cee62d3"],
        "d5572b1cbbba07a4": ["cb05fd1251af9a73"],
        "14fee50a0ce4ec91": ["e00893f9d07dc9b0"],
        "7f639abb370f996d": ["a434a95920c66e51"],
        "5616e4de7d4e70ef": ["ceff9a81afb1de70"],
        "43b4972776737988": ["a434a95920c66e51"],
        "f8c96a270bf847dd": ["6a6650a9db8983ce", "e4a3da6d1d1068b9"],
        "705575a2e1b4a4b9": ["7148f95feba974ac"],
        "e04f4df80ee6b0ab": ["5fd6e5bb6ff81c53"],
    }

    # ---- and every tag NAMED. A shader holds ONE filter_index across every loaded .ini, so the shaders
    # other pairs already tag keep their values (Sanhua's hair .91, Chisa's face .76 and outlines .73 /
    # .731, Lynae's eye shaders .625 / .718); the rest take 3381.51 up, a range no other pair uses ----
    config.filterIndices = {
        "a695e6d5f9d489cf": "3381.91",
        "7c0b4db32cee62d3": "3381.76",
        "6a6650a9db8983ce": "3381.73",
        "e4a3da6d1d1068b9": "3381.731",
        "ceff9a81afb1de70": "3381.625",
        "5fd6e5bb6ff81c53": "3381.718",
        "e00893f9d07dc9b0": "3381.51",
        "a434a95920c66e51": "3381.511",
        "e97b4254e5ed8812": "3381.512",
        "7148f95feba974ac": "3381.513",
        "cb05fd1251af9a73": "3381.517",
    }

    # ---- source component -> target slot and the registers it binds there ----
    SC = FRB.WWMIFixerConfig.SourceComponent
    config.plan = {
        0: SC(0, [B("ps-t0", "bangsMap"), B("ps-t1", "bangsDiffuse"), B("ps-t2", "bangsStrand")]),
        1: SC(1, [B("ps-t0", "hairMap"), B("ps-t1", "hairDiffuse"), B("ps-t2", "hairStrand")]),
        2: SC(2, [B("ps-t0", "faceMask"), B("ps-t1", "faceDiffuse")]),
        # her body shaders read normal / mask / diffuse at t0 / t1 / t2 as the skin's body shader does;
        # the skin's t3 is its glow map (see GAPS)
        3: SC(4, [B("ps-t0", "upperNormal"), B("ps-t1", "upperMask"), B("ps-t2", "upperDiffuse"), B("ps-t3", "noGlow")]),
        4: SC(4, [B("ps-t0", "lowerNormal"), B("ps-t1", "lowerMask"), B("ps-t2", "lowerDiffuse"), B("ps-t3", "noGlow")]),
        # the same shader on both: normal at t0, diffuse at t1
        5: SC(5, [B("ps-t0", "coatNormal"), B("ps-t1", "coatDiffuse")]),
        6: SC(6, [B("ps-t0", "eyeIris"), B("ps-t1", "eyeMask"), B("ps-t2", "eyeDiffuse")]),
        # her emissive chest piece: its one texture is the glow map the skin's body reads at t3
        7: SC(4, [B("ps-t2", "glow"), B("ps-t3", "glow")]),
    }

    # ---- a slot's OTHER passes: the outlines take an outline / diffuse map at ps-t0, the eye's second
    # pass sets ps-t1 only ----
    config.extraPassRegs = {
        0: {"f8c96a270bf847dd": [B("ps-t0", "bangsOutline")]},
        1: {"f8c96a270bf847dd": [B("ps-t0", "hairOutline")]},
        4: {"f8c96a270bf847dd": [B("ps-t0", "upperDiffuse", 3), B("ps-t0", "lowerDiffuse", 4), B("ps-t0", "glow", 7)]},
        5: {"705575a2e1b4a4b9": [B("ps-t0", "coatOutline")]},
        6: {"e04f4df80ee6b0ab": [B("ps-t1", "eyeHighlight")]},
    }

    noGlow = FRB.WWMIFixerConfig.CreatedTexture()
    noGlow.role = "noGlow"
    noGlow.colour = FRB.CppColour(249, 12, 4, 0)
    config.createdTextures = [noGlow]

    config.sourceTextures = facts
    config.flatFallsBackToSource = {"upperMask", "lowerMask", "faceMask"}

    # ---- the three lines that undo the remap on a source past 256 bones, and RabbitFX's ----
    R = FRB.WWMIFixerConfig.RegRemoval
    config.removedRegs = [
        R("ResourceBlendBufferOverride", "ref"),
        R("ResourceMergedSkeletonOverride", "ref"),
        R("ResourceExtraMergedSkeletonOverride", "ref"),
        R("Resource\\RabbitFX\\Diffuse"), R("Resource\\RabbitFX\\Lightmap"), R("Resource\\RabbitFX\\Normalmap"),
        R("Resource\\RabbitFX\\Materialmap"), R("Resource\\RabbitFX\\Cutoutmap"), R("Resource\\RabbitFX\\Specialmap"),
        R("run", "commandlist\\rabbitfx\\settextures"),
    ]

    # ---- the shape keys are RETARGETED (the Chisa decision), in the skin's slot order
    # (Tools/Misc/Diagnostics/wwmiShapeKeyOrder.py Changli ChangliNymph), with its dispatch height ----
    # DIAGNOSTIC --hideShapeKeys: the template's default, the mod's shape-key overrides commented out
    config.hiddenObjs = ["shapekeyOffsets", "shapekeyScale"] if hideShapeKeys else []
    config.zeroShapeKeyStream = False
    if (not noDispatch):
        config.shapeKeyDispatchSize = "1072"
    if (not noOrder):
        config.shapeKeyOrder = [-1] * 14 + list(range(72))
    config.cleanTexcoords = True
    config.carryByRegisterRole = True
    # the skin's slots reach bone 263 (264 x 3 float4 = 792), past the 768 default
    config.mergedSkeletonSlots = 1536
    # her upper and lower body both land on slot 4, so a copy .ini exists on a target past 256 bones
    config.copiesShareSkeleton = True
    # her blend is FOUR influences a vertex (R8G8B8A8 ids + weights) and the skin's draws read EIGHT
    # (R8 x 8 each): unwidened, every vertex was read across its neighbour's bytes and the whole model
    # drew as giant polygons in game (2026-10-20, the library gap this pair found:
    # WWMIFixerConfig::targetInfluences)
    config.targetInfluences = 8

    config.sourceLabels = {0: "bangs", 1: "hair", 2: "face", 3: "upper body", 4: "legs and skirt",
                           5: "coat and sashes", 6: "eyes", 7: "chest piece"}
    config.targetLabels = {0: "bangs", 1: "hair", 2: "face", 3: "sheer dress", 4: "body",
                           5: "skirt and veils", 6: "eyes", 7: "shawl"}
    return config


def main():
    parser = argparse.ArgumentParser(description = "Changli -> ChangliNymph, as a WWMI-template config")
    parser.add_argument("mod", help = "the mod folder (every Changli .ini under it is fixed)")
    parser.add_argument("--undo", action = "store_true", help = "undo a previous fix instead")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--hideShapeKeys", action = "store_true", help = "DIAGNOSTIC: hide the mod's shape-key overrides instead of retargeting them")
    parser.add_argument("--noShapeKeyOrder", action = "store_true", help = "DIAGNOSTIC: keep the mod's shape-key slot order")
    parser.add_argument("--noShapeKeyDispatch", action = "store_true", help = "DIAGNOSTIC: keep the mod's shape-key dispatch height")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--download", default = None,
                        help = "the API's downloadMode, eg. `disabled` -- omit for the API's own default")
    args = parser.parse_args()

    facts = textureFacts()
    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser(SrcName, FRB.makeWWMIParser(parserConfig(facts)))
    FRB.CppStrategyOverrides.setFixer(SrcName, DstName, FRB.makeWWMIFixer(fixerConfig(facts, args.hideShapeKeys, args.noShapeKeyOrder, args.noShapeKeyDispatch)))

    folder = os.path.abspath(args.mod)
    try:
        kwargs = {"path": folder, "keepBackups": args.keepBackups, "logger": FRB.Logger() if args.verbose else None}
        if (args.download):
            kwargs["downloadMode"] = args.download
        if (args.undo):
            kwargs["undoOnly"] = True
        service = FRB.RemapService(**kwargs)
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

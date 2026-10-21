#
# ===== nymphChangliFix (prototype v1) =====
#
# ChangliNymph -> Changli (Wuthering Waves): the Laurel Nymph skin's mods onto Changli herself, both past
# 256 merged bones. Built FROM the library, as changliNymphFix.py (the forward direction) is: the whole fix
# is a WWMIParserConfig handed to makeWWMIParser and a WWMIFixerConfig handed to makeWWMIFixer, registered
# through CppStrategyOverrides and run by RemapService, so the port is a transcription of the two configs.
#
#   py -3 nymphChangliFix.py <mod folder>                     fix every ChangliNymph .ini under the folder
#   py -3 nymphChangliFix.py <mod folder> --download disabled no downloads (for a deterministic A/B)
#   py -3 nymphChangliFix.py <mod folder> --verbose           attach the API's logger
#   py -3 nymphChangliFix.py <mod folder> --undo              undo a previous fix
#
# ---- The pairing, measured (2026-10-20) ----
#
#   Geometry (per-component overlap of the two download folders' Position.buf, 2 cm) and the register
#   layouts each pass SETS (wwmiPassLayout.py on the character-menu dumps at 3.7.1,
#   FrameAnalysis-ChangliNymph-2026-10-20-111534 and FrameAnalysis-Changli-2026-10-20-111723):
#     0 bangs, 1 hair, 2 face, 6 eyes: the same mesh (overlap 1.00) and the same shaders -> the same slot
#     4 her body (14fee50a: normal / mask / diffuse / glow at t0-t3) lies 48% on Changli's upper body and
#       41% on her legs; it goes to the LEGS slot (fbe2f2f0), whose shader reads normal / mask / diffuse at
#       t0-t2 exactly as hers does. Changli's upper-body shader (9d4c7d96) also reads a detail map and a ramp
#       she has no counterpart for
#     3 her sheer dress (d5572b1c: normal, diffuse) lies 86% on Changli's upper body -> slot 3, drawn
#       OPAQUE there: Changli has no sheer shader, and a remap adds none it can do without
#     5 her skirt and veils (7f639abb) -> Changli's coat slot 5, the SAME shader (normal, diffuse)
#     7 her shawl (43b49727: normal, diffuse) lies 88% on Changli's coat -> slot 5 as well, a second
#       section in the mod's own file
#   Changli's chest tattoo (slot 7) takes nothing and is hidden.
#   Her outline passes bind each part's own DIFFUSE at ps-t0 through the shaders Changli's outlines use
#   (f8c96a27, 705575a2), where Changli binds dedicated outline maps: the reverse binds the diffuse, as
#   the skin does.
#
# ---- What the library had to gain first (2026-10-20) ----
#
#   * WWMIFixerConfig::targetInfluences NARROWS too: her blend is 8 influences a vertex and Changli's draws
#     read 4. Each vertex keeps its 4 heaviest slots, their weights scaled to the same total, and the fix's
#     BlendRemapVertexVG keeps the same slots.
#
# GAPS this prototype knows about:
#   * Her body's glow map (ps-t3, 742c5c7b, the texture Changli's tattoo draws with) has no register on
#     Changli's legs shader: a mod's glow there is lost.
#   * The sheer dress is drawn opaque (see above).
#   * Shape keys: Changli's slots 0-71 take her keys 14-85 by content; Changli's 72-85 pair with nothing of
#     hers, and her keys 0-13 are dropped.
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

SrcName = "ChangliNymph"
DstName = "Changli"
Downloads = os.path.join(Repo, "Data", "Mod Downloads", "WuWa", "ChangliNymph", "3_7")
LiveHashes = os.path.join(Repo, "Data", "Mod Downloads", "WuWa", "ChangliNymph", "ChangliNymphLiveHashes.json")
B = FRB.WWMIFixerConfig.Binding
# the highest diffuse alpha any of Changli's own body / coat diffuses carries (measured on her 3_7 downloads)
ChangliDiffuseAlphaMax = 117
# below this alpha a texel of her see-through parts is three-quarters see-through or more on her own shader: treated as hidden
SeeThroughHidden = 64


# ---- her textures by the hash WWMI-Assets (ChangliSkin1, 2.7) and the download folder file them under;
# their LIVE twins are added below, and the textures only the 3.7 dump holds are filed by their live hash ----
AssetRoles = {
    "edd1df6c": "bangsMap", "45938ff1": "bangsDiffuse", "b7ea5559": "bangsStrand",
    "0484ab73": "hairMap", "e5a02d0a": "hairDiffuse", "0a785985": "hairStrand",
    "85e460ab": "faceMask", "8c983968": "faceDiffuse",
    "46328db0": "sheerNormal", "014b4c3f": "sheerDiffuse",
    # c4daa4c2 is the body mask WWMI-Assets filed at 2.7; the 3.7 game binds b4ef021f, a different texture
    "1b670385": "bodyNormal", "c4daa4c2": "bodyMask", "12638aa6": "bodyDiffuse",
    "21f813ba": "glow",
    "8c630ed4": "skirtNormal", "ccc37f56": "skirtDiffuse",
    "97de9831": "shawlNormal", "f938eae9": "shawlDiffuse",
    "1bc98900": "eyeDiffuse", "8383cbbf": "eyeMask",
}
DumpOnlyRoles = {"b4ef021f": "bodyMask", "6bf7a371": "eyeIris", "a32859fe": "eyeHighlight"}


def liveRoles():
    with open(LiveHashes, encoding = "utf-8") as f:
        live = {k: v for k, v in json.load(f).items() if k != "_"}
    return {live[h]: role for h, role in AssetRoles.items() if h in live}


def allRoles():
    return {**AssetRoles, **liveRoles(), **DumpOnlyRoles}


# ---- which role each of HER registers binds, per component (her own draws, wwmiPassLayout.py), plus
# RabbitFX's resource names (the Lightmap is the mask, as on every pair so far) ----
def rabbit(diffuse, mask = None, normal = None):
    keys = {"Resource\\RabbitFX\\Diffuse": diffuse}
    if (mask):
        keys["Resource\\RabbitFX\\Lightmap"] = mask
    if (normal):
        keys["Resource\\RabbitFX\\Normalmap"] = normal
    return keys

RegisterRoles = {
    0: {"ps-t0": "bangsMap", "ps-t1": "bangsDiffuse", "ps-t2": "bangsStrand", **rabbit("bangsDiffuse")},
    1: {"ps-t0": "hairMap", "ps-t1": "hairDiffuse", "ps-t2": "hairStrand", **rabbit("hairDiffuse")},
    2: {"ps-t0": "faceMask", "ps-t1": "faceDiffuse", **rabbit("faceDiffuse", "faceMask")},
    3: {"ps-t0": "sheerNormal", "ps-t1": "sheerDiffuse", **rabbit("sheerDiffuse", None, "sheerNormal")},
    4: {"ps-t0": "bodyNormal", "ps-t1": "bodyMask", "ps-t2": "bodyDiffuse", "ps-t3": "glow",
        **rabbit("bodyDiffuse", "bodyMask", "bodyNormal")},
    5: {"ps-t0": "skirtNormal", "ps-t1": "skirtDiffuse", **rabbit("skirtDiffuse", None, "skirtNormal")},
    6: {"ps-t0": "eyeIris", "ps-t1": "eyeMask", "ps-t2": "eyeDiffuse", **rabbit("eyeDiffuse", "eyeMask")},
    7: {"ps-t0": "shawlNormal", "ps-t1": "shawlDiffuse", **rabbit("shawlDiffuse", None, "shawlNormal")},
}

# Textures the two skins share under one hash: the game already binds them on the target
NoFallback = {"hairMap", "eyeDiffuse", "eyeMask", "eyeIris", "glow"}


def thumbprints():
    """Her game textures' 16 x 16 thumbprints, by hash, for a mod file no hash names"""
    import wwmiTextureThumbs
    result = {}
    for h in {**AssetRoles, **DumpOnlyRoles}:
        path = os.path.join(Downloads, f"ChangliNymphTexture{h}.dds")
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
    facts.downloadCharFolder = "ChangliNymph"
    facts.downloadVersionFolder = "3_7"
    facts.downloadPrefix = "ChangliNymph"
    # the CURRENT game texture of each role: AssetRoles first, so a role with a dump-only twin (the body
    # mask) falls back to the one the 3.7 game binds
    files = {**AssetRoles, **DumpOnlyRoles}
    facts.fallbackTextures = {role: h for h, role in files.items()
                              if role not in NoFallback and os.path.isfile(os.path.join(Downloads, f"ChangliNymphTexture{h}.dds"))}
    return facts


def currentVgMaps():
    """Each component's local -> merged map out of her Metadata.json, for a mod from before WWMI's merged skeleton"""
    with open(os.path.join(Downloads, "ChangliNymphMetadata.json"), encoding = "utf-8") as f:
        meta = json.load(f)
    maps = {}
    for k, comp in enumerate(meta["components"]):
        vg = {int(a): int(b) for a, b in comp["vg_map"].items()}
        maps[k] = [vg.get(i, 0) for i in range(max(vg) + 1)] if vg else [0]
    return maps


def parserConfig(facts) -> "FRB.WWMIParserConfig":
    config = FRB.WWMIParserConfig()
    config.version = "2.4"
    config.textures = facts
    return config


def fixerConfig(facts) -> "FRB.WWMIFixerConfig":
    config = FRB.WWMIFixerConfig()
    config.targetId = FRB.ModTypeId.Changli
    config.version = "1.4"            # her live vb0 is 1.4's, and her bones are numbered as at 1.4
    config.sourceVersion = "2.4"

    # a mod from before WWMI's merged skeleton (ChangliNymph2 / 4, WWMI BETA-2 exports with no vg_offset)
    # holds each component's LOCAL bone ids: lifted through her Metadata.json's per-component vg_map
    config.sourceVgMaps = currentVgMaps()

    # ---- the passes the TARGET draws each slot on (Changli's dump, wwmiPassLayout.py) ----
    config.slotPasses = [
        ["880e1a81d913db05", "83782bd563d3a6c9"],   # 0 bangs
        ["518dadece9a19b08"],                       # 1 hair
        ["ed1c0f8b2ba08ac4"],                       # 2 face
        ["9d4c7d9618ef7f45"],                       # 3 upper body
        ["fbe2f2f061949559"],                       # 4 legs and skirt
        ["7f639abb370f996d"],                       # 5 coat
        ["5616e4de7d4e70ef"],                       # 6 eyes
        ["bb1c5ba6e1be6395"],                       # 7 chest flame tattoo (nothing of hers lands there)
    ]

    # ---- every pass gated through its VERTEX shader -- both skins' and both dumps', per pixel shader ----
    config.passVertexShaders = {
        "880e1a81d913db05": ["a695e6d5f9d489cf"],
        "83782bd563d3a6c9": ["a695e6d5f9d489cf"],
        "518dadece9a19b08": ["e97b4254e5ed8812"],
        "ed1c0f8b2ba08ac4": ["7c0b4db32cee62d3", "4ce754b5b9574b1a"],
        "9d4c7d9618ef7f45": ["6e5d4bc00bc160ee"],
        "fbe2f2f061949559": ["3b36d7d284d20522"],
        "7f639abb370f996d": ["3ba352c448cd29cc", "a434a95920c66e51"],
        "5616e4de7d4e70ef": ["ceff9a81afb1de70"],
        "bb1c5ba6e1be6395": ["488d2eb256ff8c71"],
        "f8c96a270bf847dd": ["6a6650a9db8983ce", "e4a3da6d1d1068b9"],
        "705575a2e1b4a4b9": ["3e22b125329523fa", "7148f95feba974ac"],
        "e04f4df80ee6b0ab": ["5fd6e5bb6ff81c53", "a54621ce48ed541b"],
    }

    # ---- and every tag NAMED: the values the forward direction and the other pairs already give a
    # shader, and 3381.521 up for the vertex shaders only Changli's own draws use ----
    config.filterIndices = {
        "a695e6d5f9d489cf": "3381.91",
        "7c0b4db32cee62d3": "3381.76",
        "4ce754b5b9574b1a": "3381.769",
        "6a6650a9db8983ce": "3381.73",
        "e4a3da6d1d1068b9": "3381.731",
        "ceff9a81afb1de70": "3381.625",
        "5fd6e5bb6ff81c53": "3381.718",
        "a54621ce48ed541b": "3381.616",
        "a434a95920c66e51": "3381.511",
        "e97b4254e5ed8812": "3381.512",
        "7148f95feba974ac": "3381.513",
        "6e5d4bc00bc160ee": "3381.521",
        "3b36d7d284d20522": "3381.522",
        "3ba352c448cd29cc": "3381.523",
        "488d2eb256ff8c71": "3381.524",
        "3e22b125329523fa": "3381.525",
    }

    # ---- source component -> target slot and the registers it binds there ----
    SC = FRB.WWMIFixerConfig.SourceComponent
    config.plan = {
        0: SC(0, [B("ps-t0", "bangsMap"), B("ps-t1", "bangsDiffuse"), B("ps-t2", "bangsStrand")]),
        1: SC(1, [B("ps-t0", "hairMap"), B("ps-t1", "hairDiffuse"), B("ps-t2", "hairStrand")]),
        2: SC(2, [B("ps-t0", "faceMask"), B("ps-t1", "faceDiffuse")]),
        # her sheer dress through Changli's upper-body shader: normal / mask / diffuse / detail at t0-t3.
        # She has no mask or detail map for it; the masks share one legend (R = 255 bare skin), so a flat
        # cloth mask, and a flat detail at the mean of Changli's own (70, alpha 0). The ramp (t4) is the
        # game's
        3: SC(3, [B("ps-t0", "sheerNormal"), B("ps-t1", "flatClothMask"), B("ps-t2", "sheerDiffuse"), B("ps-t3", "flatDetail")]),
        # her body through Changli's legs shader: the same three inputs at the same registers
        4: SC(4, [B("ps-t0", "bodyNormal"), B("ps-t1", "bodyMask"), B("ps-t2", "bodyDiffuse")]),
        # the same shader on both: normal at t0, diffuse at t1
        5: SC(5, [B("ps-t0", "skirtNormal"), B("ps-t1", "skirtDiffuse")]),
        6: SC(6, [B("ps-t0", "eyeIris"), B("ps-t1", "eyeMask"), B("ps-t2", "eyeDiffuse")]),
        # her shawl through the coat slot's shader: normal at t0, diffuse at t1
        7: SC(5, [B("ps-t0", "shawlNormal"), B("ps-t1", "shawlDiffuse")]),
    }
    plan = config.plan                   # pybind hands back a copy: edit it, then assign it back
    # the shawl shares slot 5 with her skirt: drawn from the mod's OWN file, where the shape-keyed positions
    # are computed (a copy computes none)
    plan[7].sameFileSection = True
    # neither her sheer dress nor her shawl has an outline pass on her own draws
    plan[3].offPasses = ["f8c96a270bf847dd"]
    plan[7].offPasses = ["705575a2e1b4a4b9"]
    config.plan = plan

    # ---- a slot's OTHER passes: her outlines bind the part's own DIFFUSE at ps-t0 (Changli binds
    # dedicated outline maps there); the eye's second pass sets ps-t1 only ----
    config.extraPassRegs = {
        0: {"f8c96a270bf847dd": [B("ps-t0", "bangsDiffuse")]},
        1: {"f8c96a270bf847dd": [B("ps-t0", "hairDiffuse")]},
        4: {"705575a2e1b4a4b9": [B("ps-t0", "bodyDiffuse")]},
        5: {"705575a2e1b4a4b9": [B("ps-t0", "skirtDiffuse", 5)]},
        6: {"e04f4df80ee6b0ab": [B("ps-t1", "eyeHighlight")]},
    }

    def created(role, colour):
        c = FRB.WWMIFixerConfig.CreatedTexture()
        c.role, c.colour = role, colour
        return c
    config.createdTextures = [created("flatClothMask", FRB.CppColour(0, 128, 0, 255)),
                              created("flatDetail", FRB.CppColour(70, 70, 70, 0))]

    config.sourceTextures = facts
    config.flatFallsBackToSource = {"bodyMask", "faceMask"}

    # ---- her see-through parts' diffuse ALPHA is opacity (mean 165 on the sheer dress, 221 on the shawl,
    # up to 255) on their own shaders; on Changli's opaque shaders alpha is a scalar her own diffuses keep
    # at or below 117 (every one: upper, legs, coat), and above it lights the part -- the sheer dress's gold
    # embroidery and bodice drew glowing and orange (in-game check 1). Clamped to her ceiling; RGB untouched,
    # and the gamma stays on because this is a colour texture (ChisaParfait -> Chisa, the same edit) ----
    # ...and a texel her own shader all but cuts away (alpha below SeeThroughHidden: the quads of her fine gold
    # chains are a gold line on grey, the grey invisible) is drawn on Changli's opaque shaders, so the chains drew as dark grey cords:
    # each fully transparent texel takes the colour of the nearest visible one (edge padding), so a chain quad
    # draws gold (in-game audit, reverse epoch 1)
    def alphaClamp(ctx):
        def run(tex):
            import numpy as np
            w, h = tex.width, tex.height
            px = np.frombuffer(bytes(tex.getPixels()), np.uint8).reshape(h, w, 4).copy()
            # one pass at a time, every hidden texel next to a visible one (as of the previous pass) takes
            # the colour of its first visible neighbour in the order left, right, up, down -- the same rule,
            # step for step, as ChangliNymphFixer.cpp's padHidden, so the two write the same bytes
            visible = px[..., 3] >= SeeThroughHidden
            if visible.any():
                rgb = px[..., :3]
                while True:
                    prevVis = visible.copy()
                    prevRgb = rgb.copy()
                    todo = ~prevVis
                    taken = np.zeros_like(todo)
                    for dy, dx in ((0, -1), (0, 1), (-1, 0), (1, 0)):
                        srcVis = np.zeros_like(prevVis)
                        srcRgb = np.zeros_like(prevRgb)
                        ys = slice(max(dy, 0), h + min(dy, 0)); yd = slice(max(-dy, 0), h + min(-dy, 0))
                        xs = slice(max(dx, 0), w + min(dx, 0)); xd = slice(max(-dx, 0), w + min(-dx, 0))
                        srcVis[yd, xd] = prevVis[ys, xs]
                        srcRgb[yd, xd] = prevRgb[ys, xs]
                        m = todo & srcVis & ~taken
                        rgb[m] = srcRgb[m]
                        taken |= m
                    if not taken.any():
                        break
                    visible |= taken
            px[..., 3] = np.minimum(px[..., 3], ChangliDiffuseAlphaMax)
            tex.setPixels(px.tobytes(), w, h)
        return run
    E = FRB.WWMIFixerConfig.TexEdit
    config.texEdits = [E(role, "AlphaClamp", alphaClamp) for role in ("sheerDiffuse", "shawlDiffuse")]

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

    # ---- the shape keys are RETARGETED, in Changli's slot order (wwmiShapeKeyOrder.py ChangliNymph
    # Changli: her slots 0-71 take the skin's keys 14-85 by content, 72-85 nothing), with her dispatch height ----
    config.hiddenObjs = []
    config.zeroShapeKeyStream = False
    config.shapeKeyDispatchSize = "1097"
    config.shapeKeyOrder = list(range(14, 86)) + [-1] * 14
    config.cleanTexcoords = True
    config.carryByRegisterRole = True
    config.readInisBelow = True
    # her slots reach bone 275 (276 x 3 float4 = 828), past the 768 default
    config.mergedSkeletonSlots = 1536
    config.copiesShareSkeleton = True
    # the skin's blend is EIGHT influences a vertex and Changli's draws read FOUR
    config.targetInfluences = 4

    config.sourceLabels = {0: "bangs", 1: "hair", 2: "face", 3: "sheer dress", 4: "body",
                           5: "skirt and veils", 6: "eyes", 7: "shawl"}
    config.targetLabels = {0: "bangs", 1: "hair", 2: "face", 3: "upper body", 4: "legs and skirt",
                           5: "coat and sashes", 6: "eyes", 7: "chest flame tattoo"}
    return config


def main():
    parser = argparse.ArgumentParser(description = "ChangliNymph -> Changli, as a WWMI-template config")
    parser.add_argument("mod", help = "the mod folder (every ChangliNymph .ini under it is fixed)")
    parser.add_argument("--undo", action = "store_true", help = "undo a previous fix instead")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--download", default = None,
                        help = "the API's downloadMode, eg. `disabled` -- omit for the API's own default")
    args = parser.parse_args()

    facts = textureFacts()
    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser(SrcName, FRB.makeWWMIParser(parserConfig(facts)))
    FRB.CppStrategyOverrides.setFixer(SrcName, DstName, FRB.makeWWMIFixer(fixerConfig(facts)))

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

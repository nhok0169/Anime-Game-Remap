#
# ===== lynaePeppermintFix (prototype v1) =====
#
# Lynae -> LynaePeppermint (Wuthering Waves): a character onto her own skin, both past 256 merged
# bones. Built FROM the library -- the whole fix is a WWMIParserConfig handed to makeWWMIParser and a
# WWMIFixerConfig handed to makeWWMIFixer, the factories Sanhua -> SanhuaExorcist and Chisa ->
# ChisaParfait are compiled from, registered through CppStrategyOverrides and run by RemapService.
# So the port is a transcription of the two configs below into IniParseData/Lynae/ and
# IniFixData/Lynae/.
#
#   py -3 lynaePeppermintFix.py <mod folder>                     fix every Lynae .ini under the folder
#   py -3 lynaePeppermintFix.py <mod folder> --download disabled no downloads (for a deterministic A/B)
#   py -3 lynaePeppermintFix.py <mod folder> --verbose           attach the API's logger
#   py -3 lynaePeppermintFix.py <mod folder> --undo              undo a previous fix
#
# ---- What the library had to gain first (2026-10-05) ----
#
#   * Most of WWMIFixerConfig was reachable from C++ only: passVertexShaders, filterIndices,
#     extraPassRegs, removedRegs, texEdits, anchorChains, cleanTexcoords, sourceVersion and the rest.
#     All of them are bound now (py/src/data/PyWWMIBuilders.cpp), with RegRemoval, TexEdit and
#     TexEditContext, so a WuWa prototype is a config like the GI ones.
#   * The two download folders lacked fourteen textures the draws set -- among them the R8_UNORM
#     ps-t2 detail maps, which 3DMigoto's default dump writes as lossy .jpg previews (commit
#     88c93b51, Data/Mod Downloads/WuWa/README.md).
#   * wwmiIdentityMod.py bound the PREVIOUS pose where Lynae's skeleton sits in vs-cb3 alone (her
#     early depth passes), which dropped her ID card, pin and ear cups out of her own identity mod.
#     Fixed; CreatingRemaps' Lynae section has the story. WWMIFixer's CommandListMergeSlot<N> has the
#     same two independent `if`s, which matters only on a target with such passes: LynaePeppermint
#     has none, Lynae has eleven (the reverse direction).
#
# ---- The pairing, measured (2026-10-05) ----
#
#   Geometry (per-component overlap of the two download folders' Position.buf) and shader family:
#   0 bangs, 1 hair, 2 face, 3 upper body, 4 lower body and 7 eyes land on the same slot (the face
#   and eyes are the SAME mesh: overlap 1.00). Lynae's 5 (the jacket hanging off one shoulder) goes
#   onto the skin's 6 (her jacket), and her 6 (props: headphones, chest pin, belt ID card) onto the
#   skin's 5 (a hip pouch), the one slot left -- same register order, and the skin has no
#   special-material shader, so the props' glass / iridescent passes do not carry over.
#
# GAPS this prototype knows about (each is a note for the port, step 7):
#   * The props lose their special-material passes (Lynae draws component 6 on six shaders, four of
#     them a glass / foil layer); the skin's pouch shader is plain cloth.
#   * The hair strand maps (ps-t5) differ between the skins in G and alpha (Lynae A = 0, the skin's
#     A = 255) and the hair shaders differ -- bound raw until the game says otherwise.
#   * TextureFile cannot decode the legacy 8-bit luminance .dds (the detail maps), so a mod's detail
#     map is identified by hash only, never by thumbprint.
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

SrcName = "Lynae"
DstName = "LynaePeppermint"
Downloads = os.path.join(Repo, "Data", "Mod Downloads", "WuWa", "Lynae", "3_7")
B = FRB.WWMIFixerConfig.Binding


# ---- her textures by the hash the game binds them under: the 3.7 dump's, then every older
# generation wwmiHashHistory.py proved from the mods (all but Lynae8/11 carry an older one) ----
CurrentRoles = {
    "997e0a9a": "bangsMask", "7d044e58": "bangsDiffuse", "48947f0c": "bangsStrand",
    "bfa33038": "hairMask", "265d844d": "hairDiffuse", "694d3d7f": "hairRamp", "b2ef1928": "hairStrand",
    "d7c56e9e": "hairShadeRamp",
    "e28662e9": "faceMask", "0f19cf16": "faceDiffuse",
    "739ef6e9": "upperNormal", "d5dfa096": "upperMask", "c5784ddf": "upperDetail", "179ec8b9": "upperDiffuse",
    "0f5d6c08": "lowerNormal", "94d7281e": "lowerMask", "950acc22": "lowerDetail", "1998df83": "lowerDiffuse",
    "d63a624a": "lowerSheer",
    "0eacd18a": "jacketNormal", "d2ef0438": "jacketMask", "6d5f71f9": "jacketDetail", "f94bbcf4": "jacketDiffuse",
    "cecc13eb": "propsNormal", "4f1d5285": "propsMask", "c49bec43": "propsDetail", "c3375aee": "propsDiffuse",
    "6bf7a371": "eyeIris", "a506a70d": "eyeMask", "a004a399": "eyeDiffuse",
}
OlderRoles = {
    "347a41a2": "bangsMask", "8e20c909": "bangsMask", "b0e8646a": "bangsDiffuse",
    "513eace9": "bangsStrand", "1261dcbc": "bangsStrand",
    "f45cea79": "hairMask", "47df9d42": "hairMask", "7065b0be": "hairDiffuse",
    "17372a89": "hairStrand", "b5453395": "hairStrand",
    "2e60ca6c": "faceDiffuse",
    "2fd59f3a": "upperNormal", "38686e88": "upperMask", "60caec0b": "upperDiffuse",
    "ec6b0edc": "lowerNormal", "6c09d0d7": "lowerNormal", "dd5dceaa": "lowerMask", "23d9b128": "lowerDiffuse",
    "7f91ca3e": "lowerSheer",
    "a8e46264": "jacketNormal", "358f8d1e": "jacketNormal", "10c8713e": "jacketMask",
    "879f275e": "jacketDiffuse", "70ee685f": "jacketDiffuse",
    "391a7fde": "propsNormal", "ba2b09a1": "propsNormal", "7aa57b2c": "propsMask", "e45c9797": "propsMask",
    "211e50f4": "propsDiffuse", "37cdf366": "propsDiffuse",
    "8383cbbf": "eyeMask", "cb04a3d7": "eyeDiffuse", "c84b599c": "eyeDiffuse",
}

# ---- which role each of HER registers binds, per component (her own draws, wwmiPassLayout.py) ----
def rabbit(diffuse, mask, normal = None):
    keys = {"Resource\\RabbitFX\\Diffuse": diffuse, "Resource\\RabbitFX\\Lightmap": mask}
    if (normal):
        keys["Resource\\RabbitFX\\Normalmap"] = normal
    return keys

RegisterRoles = {
    0: {"ps-t0": "bangsMask", "ps-t1": "bangsDiffuse", "ps-t4": "hairShadeRamp", "ps-t5": "bangsStrand", **rabbit("bangsDiffuse", "bangsMask")},
    1: {"ps-t0": "hairMask", "ps-t1": "hairDiffuse", "ps-t2": "hairRamp", "ps-t4": "hairShadeRamp", "ps-t5": "hairStrand", **rabbit("hairDiffuse", "hairMask")},
    2: {"ps-t0": "faceMask", "ps-t1": "faceDiffuse", **rabbit("faceDiffuse", "faceMask")},
    3: {"ps-t0": "upperNormal", "ps-t1": "upperMask", "ps-t2": "upperDetail", "ps-t3": "upperDiffuse", **rabbit("upperDiffuse", "upperMask", "upperNormal")},
    4: {"ps-t0": "lowerNormal", "ps-t1": "lowerMask", "ps-t2": "lowerDetail", "ps-t3": "lowerDiffuse", "ps-t8": "lowerSheer", **rabbit("lowerDiffuse", "lowerMask", "lowerNormal")},
    5: {"ps-t0": "jacketNormal", "ps-t1": "jacketMask", "ps-t2": "jacketDetail", "ps-t3": "jacketDiffuse", **rabbit("jacketDiffuse", "jacketMask", "jacketNormal")},
    6: {"ps-t0": "propsNormal", "ps-t1": "propsMask", "ps-t2": "propsDetail", "ps-t3": "propsDiffuse", **rabbit("propsDiffuse", "propsMask", "propsNormal")},
    7: {"ps-t0": "eyeIris", "ps-t1": "eyeMask", "ps-t2": "eyeDiffuse", **rabbit("eyeDiffuse", "eyeMask")},
}

# The eye textures are the SAME hashes on both skins, so the game already binds them on the target;
# every other role falls back to her own game texture when the mod ships no file for it.
NoFallback = {"eyeIris", "eyeMask", "eyeDiffuse"}


def thumbprints():
    """Her game textures' 16 x 16 thumbprints, by hash, for a mod file no hash names"""
    import wwmiTextureThumbs
    result = {}
    for h in CurrentRoles:
        path = os.path.join(Downloads, f"LynaeTexture{h}.dds")
        if (os.path.isfile(path)):
            thumb = wwmiTextureThumbs.thumbprint(path)
            if (thumb is not None):
                result[h] = thumb
    return result


def textureFacts() -> "FRB.WWMITextureFacts":
    facts = FRB.WWMITextureFacts()
    facts.roles = {**OlderRoles, **CurrentRoles}
    facts.registerRoles = RegisterRoles
    facts.textureThumbprints = thumbprints()
    facts.downloadGameFolder = "WuWa"
    facts.downloadCharFolder = "Lynae"
    facts.downloadVersionFolder = "3_7"
    facts.downloadPrefix = "Lynae"
    facts.fallbackTextures = {role: h for h, role in CurrentRoles.items() if role not in NoFallback}
    return facts


def parserConfig(facts) -> "FRB.WWMIParserConfig":
    config = FRB.WWMIParserConfig()
    config.version = "3.6"            # her vb0 most mods carry; the 3.7 row covers the rest
    config.textures = facts
    return config


# ---- Lynae's 3.6 numbering, as 3.7 ids: her 3.7 update inserted bones into her skeleton and renumbered
# these 71 merged ids (every other id is the same in both). Reconstructed from Lynae5 against the 3.7
# identity mod by WWMI's first-slot rule; nine 3.6 merged-skeleton mods agree with it on 99.7-99.98% of
# their vanilla-position vertices ----
Renumbered36To37 = {
        144: 145, 145: 146, 146: 147, 147: 148, 148: 149, 149: 150, 150: 151, 151: 152, 152: 153, 153: 154,
        154: 144, 166: 167, 167: 168, 168: 169, 169: 170, 170: 171, 171: 172, 172: 173, 173: 174, 174: 175,
        175: 176, 176: 177, 177: 178, 178: 179, 179: 180, 180: 181, 181: 182, 182: 184, 183: 185, 184: 186,
        185: 187, 186: 188, 187: 189, 188: 190, 189: 191, 190: 192, 191: 193, 192: 194, 193: 195, 194: 196,
        195: 197, 197: 199, 198: 200, 199: 201, 200: 202, 201: 203, 202: 204, 203: 205, 204: 206, 205: 207,
        206: 208, 207: 209, 208: 210, 209: 211, 210: 212, 211: 213, 212: 214, 213: 215, 214: 216, 215: 217,
        216: 218, 217: 219, 218: 220, 219: 221, 220: 222, 221: 223, 222: 224, 223: 225, 224: 183, 225: 226,
        226: 166
}


def boneCentroids():
    """Her rest-pose bone centroids in the 3.7 numbering, off the download folder's own buffers"""
    import numpy as np
    pos = np.fromfile(os.path.join(Downloads, "LynaePosition.buf"), np.float32).reshape(-1, 3)
    blend = np.fromfile(os.path.join(Downloads, "LynaeBlend.buf"), np.uint8).reshape(len(pos), -1)
    n = blend.shape[1] // 2
    wide = np.fromfile(os.path.join(Downloads, "LynaeBlendRemapVertexVG.buf"), np.uint16).reshape(len(pos), -1)[:, :n]
    heaviest = np.argmax(blend[:, n:], axis = 1)
    bone = wide[np.arange(len(pos)), heaviest].astype(int)
    strong = blend[np.arange(len(pos)), n + heaviest] > 127
    result = {}
    for b in np.unique(bone[strong]):
        sel = strong & (bone == b)
        if (sel.sum() >= 3):
            result[int(b)] = tuple(float(x) for x in pos[sel].mean(axis = 0))
    return result


def fixerConfig(facts, keepRabbitFX: bool = False) -> "FRB.WWMIFixerConfig":
    config = FRB.WWMIFixerConfig()
    config.targetId = FRB.ModTypeId.LynaePeppermint
    config.version = "3.7"
    # Hers, for a mod whose vb0 is not in sourceVersionByVb0. Both skins share cb4 f02baf77, which at 3.6
    # only Lynae has; a 3.7-vb0 mod is looked up at 3.7, where no NotFound has been seen on any of the
    # 13 mods (grep the output when adding mods).
    config.sourceVersion = "3.6"
    # Her 3.7 update moved her vb0 AND renumbered her skeleton, so each mod takes the vertex group
    # row of the version it was exported at, read off its own vb0
    config.sourceVersionByVb0 = {"0c33d628": "3.6", "7e400733": "3.7"}
    # ...but the vb0 is not what the BONES are numbered by: a hash-update tool rewrote Lynae11's vb0 to
    # 3.7's and kept her 3.6 ids. So the blend's row is chosen by the mod's own geometry
    N = FRB.WWMIFixerConfig.SkeletonNumbering
    config.skeletonNumberings = [N("3.6", Renumbered36To37), N("3.7", {})]
    config.referenceBoneCentroids = boneCentroids()

    # ---- the passes the TARGET draws each slot on (her Peppermint dump, wwmiDrawTable.py) ----
    config.slotPasses = [
        ["dfeea2d5f7210740", "8485dc12c5851fa9"],   # 0 bangs (the second pass inherits the first's set)
        ["dfeea2d5f7210740"],                       # 1 hair
        ["640fac991b3599a7"],                       # 2 face
        ["ed4fe222718a4497"],                       # 3 upper body
        ["0a52e215e81518f3"],                       # 4 lower body
        ["12fcca8e31aad1f5", "4470be479e212ca1"],   # 5 hip pouch (both passes set the whole set)
        ["efc2690a4acd3c11", "0b6e3ef7b7c30cd3"],   # 6 jacket (the second inherits the first's)
        ["fa9e4d98ed0a570e"],                       # 7 eyes
    ]

    # ---- every pass gated through its VERTEX shaders, so RabbitFX's pixel-shader tags survive ----
    config.passVertexShaders = {
        "dfeea2d5f7210740": ["c30da0cb86079064"],
        "8485dc12c5851fa9": ["c30da0cb86079064"],
        "640fac991b3599a7": ["1c42858ceb438917"],
        "ed4fe222718a4497": ["1f324d354402418c"],
        "0a52e215e81518f3": ["8af0aa3dcb3903ee"],
        "12fcca8e31aad1f5": ["fd92e896d5503d96"],
        "4470be479e212ca1": ["fd92e896d5503d96"],
        "efc2690a4acd3c11": ["1f324d354402418c"],
        "0b6e3ef7b7c30cd3": ["1f324d354402418c"],
        "fa9e4d98ed0a570e": ["c32a69851757154a"],
        "e04f4df80ee6b0ab": ["a54621ce48ed541b"],
        "bce1512f1c6b82fe": ["a57b6349c93b6107"],
        "f8c96a270bf847dd": ["6a6650a9db8983ce", "e4a3da6d1d1068b9"],
        "0fbe7ebba08cd1b0": ["641c11c9ee112caf", "d87c4657c08f2cdd"],
    }

    # ---- and every tag NAMED. A shader holds ONE filter_index across every loaded .ini, so the
    # three outline vertex shaders Chisa -> ChisaParfait already tags keep its values; the rest take
    # 3381.610 up, a range no other pair uses (Chisa .710-.734 / .76-.768, Sanhua .81-.84 / .91-.96) ----
    config.filterIndices = {
        "6a6650a9db8983ce": "3381.73",
        "e4a3da6d1d1068b9": "3381.731",
        "641c11c9ee112caf": "3381.715",
        "c30da0cb86079064": "3381.61",
        "1c42858ceb438917": "3381.611",
        "1f324d354402418c": "3381.612",
        "8af0aa3dcb3903ee": "3381.613",
        "fd92e896d5503d96": "3381.614",
        "c32a69851757154a": "3381.615",
        "a54621ce48ed541b": "3381.616",
        "a57b6349c93b6107": "3381.617",
        "d87c4657c08f2cdd": "3381.618",
    }

    # ---- source component -> target slot and the registers it binds there ----
    config.plan = {
        # ps-t2 (36c90686) and ps-t3 (4eaa9816) are the same texture on both skins: left to the game
        0: FRB.WWMIFixerConfig.SourceComponent(0, [B("ps-t0", "bangsMask"), B("ps-t1", "bangsDiffuse"),
                                                   B("ps-t4", "hairShadeRamp"), B("ps-t5", "bangsStrand")]),
        1: FRB.WWMIFixerConfig.SourceComponent(1, [B("ps-t0", "hairMask"), B("ps-t1", "hairDiffuse"), B("ps-t2", "hairRamp"),
                                                   B("ps-t4", "hairShadeRamp"), B("ps-t5", "hairStrand")]),
        # The face is the same mesh on both, and the skin's face shader reads its diffuse at ps-t2
        # where hers reads it at ps-t1. The two face MASKS are packed differently (hers R 246 / G 28
        # / B 0, the skin's R 118 / B 101 / A 202), so the skin's own mask stays -- its UVs are hers.
        2: FRB.WWMIFixerConfig.SourceComponent(2, [B("ps-t2", "faceDiffuse")]),
        3: FRB.WWMIFixerConfig.SourceComponent(3, [B("ps-t0", "upperNormal"), B("ps-t1", "upperMask"),
                                                   B("ps-t2", "upperDetail"), B("ps-t3", "upperDiffuse")]),
        4: FRB.WWMIFixerConfig.SourceComponent(4, [B("ps-t0", "lowerNormal"), B("ps-t1", "lowerMask"),
                                                   B("ps-t2", "lowerDetail"), B("ps-t3", "lowerDiffuse")]),
        # The skin's jacket shader reads mask / diffuse / detail at ps-t1 / t2 / t3 -- hers reads the
        # detail at t2 and the diffuse at t3
        5: FRB.WWMIFixerConfig.SourceComponent(6, [B("ps-t0", "jacketNormal"), B("ps-t1", "jacketMask"),
                                                   B("ps-t2", "jacketDiffuse"), B("ps-t3", "jacketDetail")]),
        6: FRB.WWMIFixerConfig.SourceComponent(5, [B("ps-t0", "propsNormal"), B("ps-t1", "propsMask"),
                                                   B("ps-t2", "propsDetail"), B("ps-t3", "propsDiffuse")]),
        7: FRB.WWMIFixerConfig.SourceComponent(7, [B("ps-t0", "eyeIris"), B("ps-t1", "eyeMask"), B("ps-t2", "eyeDiffuse")]),
    }

    # ---- a slot's OTHER passes: the outlines take the slot's diffuse at ps-t0; the eye's second
    # pass sets ps-t0 only ----
    config.extraPassRegs = {
        0: {"bce1512f1c6b82fe": [B("ps-t0", "bangsDiffuse")]},
        1: {"f8c96a270bf847dd": [B("ps-t0", "hairDiffuse")]},
        3: {"f8c96a270bf847dd": [B("ps-t0", "upperDiffuse")]},
        4: {"0fbe7ebba08cd1b0": [B("ps-t0", "lowerDiffuse")]},
        5: {"0fbe7ebba08cd1b0": [B("ps-t0", "propsDiffuse")]},
        6: {"0fbe7ebba08cd1b0": [B("ps-t0", "jacketDiffuse")]},
        7: {"e04f4df80ee6b0ab": [B("ps-t0", "eyeMask")]},
    }

    config.sourceTextures = facts

    # ---- the body's material-code maps (ps-t2) mark skin with material 0 on Lynae and 4 on the skin
    # (low four bits; the high four are flags), so a map carried across unchanged shaded the body's
    # skin as cloth -- whiter, with lavender shadows, under a warm face. 0 and 4 are exchanged, the same
    # rule both ways (core's lynaeSkinCodeSwap) ----
    def _swapCode(v):
        lo, hi = v & 0x0F, v & 0xF0
        return hi | (4 if lo == 0 else 0 if lo == 4 else lo)
    _codeTable = bytes(_swapCode(v) for v in range(256))
    def skinCodeSwap(ctx):
        def run(tex):
            tex.gamma = None                     # these bytes are codes, not colour
            px = bytearray(tex.getPixels())
            red = bytes(px[0::4]).translate(_codeTable)
            px[0::4] = red
            px[1::4] = red
            px[2::4] = red
            tex.setPixels(bytes(px), tex.width, tex.height)
        return run
    config.texEdits = [FRB.WWMIFixerConfig.TexEdit(role, "SkinCode", skinCodeSwap) for role in ("upperDetail", "lowerDetail")]

    # Masks mark regions: a flat one from a mod says "all of this is one material"
    # Her own masks are flat in places on purpose (her hair mask is one value over the whole texture,
    # her lower-body and jacket masks R = 255 throughout), so a flat mask is replaced by HER game
    # texture rather than left to the game: the skin's hair mask is structured and laid out for the
    # skin's UVs, and would land in patches on hers (Chisa's hairMask lesson).
    config.flatFallsBackToSource = {"upperMask", "lowerMask", "jacketMask", "propsMask", "faceMask", "hairMask", "bangsMask"}

    # ---- the three lines that undo the remap on a source past 256 bones, and RabbitFX's ----
    R = FRB.WWMIFixerConfig.RegRemoval
    config.removedRegs = [
        R("ResourceBlendBufferOverride", "ref"),
        R("ResourceMergedSkeletonOverride", "ref"),
        R("ResourceExtraMergedSkeletonOverride", "ref"),
    ]
    # DIAGNOSTIC --keepRabbitFX: leave a mod's RabbitFX lines in its remapped sections. Two mods here
    # (Lynae10's suit, Lynae11's stockings and lining) get their look from RabbitFX on Lynae's shaders,
    # not from their textures; whether RabbitFX patches the skin's shaders too decides if keeping the
    # lines carries that look
    if (not keepRabbitFX):
        config.removedRegs = list(config.removedRegs) + [
            R("Resource\\RabbitFX\\Diffuse"), R("Resource\\RabbitFX\\Lightmap"), R("Resource\\RabbitFX\\Normalmap"),
            R("Resource\\RabbitFX\\Materialmap"), R("Resource\\RabbitFX\\Cutoutmap"), R("Resource\\RabbitFX\\Specialmap"),
            R("run", "commandlist\\rabbitfx\\settextures")]

    # ---- the shape keys are RETARGETED (the Chisa decision): the overrides keep firing and the
    # asset remap moves their hashes and checksum onto the skin's ----
    config.hiddenObjs = []
    config.zeroShapeKeyStream = False
    # ...and a batched export's dispatch height is the skin's (her Metadata.json's dispatch_y), or WWMI
    # loads too few of the mod's offsets on her draws and the body is drawn as spikes (Lynae9)
    config.shapeKeyDispatchSize = "1854"
    config.cleanTexcoords = True
    # a mod's line that puts a texture into another kind of slot on purpose keeps that slot (Lynae3's
    # U toggle binds each part's diffuse into the ramp / detail slot, ps-t2, which both skins read alike)
    config.carryByRegisterRole = True
    # The fix's own merged skeleton is sized for the TARGET: LynaePeppermint's slots reach bone 314
    # (315 x 3 float4 = 945), past the 768 default; 1536 is ChisaParfait -> Chisa's value, and WWMI Tools'
    config.mergedSkeletonSlots = 1536

    # ---- a mod from before WWMI's merged skeleton holds per-component LOCAL bone ids (Lynae5, a
    # `WWMI BETA-8 INI` with no vg_offset). Every such mod of hers is a 3.6 export, so its locals are
    # lifted into her 3.6 numbering (which the 3.6 VGRemapData row then reads) -- NOT through the
    # 3.7 Metadata.json's vg_map: her 3.7 update inserted bones into components 3-6, and the 3.7 map
    # put Lynae5's body on the neighbouring bones (a leg drawn as a long stick in game). Reconstructed
    # from Lynae5 against the 3.7 identity mod; see VGRemapData.cpp's 3.6 row ----
    config.sourceVgMaps = {
        0: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44],
        1: [45, 46, 47, 48, 49, 0, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116],
        2: [0],
        3: [106, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 0, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 72, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228],
        4: [229, 230, 231, 187, 233, 186, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321],
        5: [322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 207, 206, 210],
        6: [247, 337, 323, 330, 360, 328, 336, 338, 339, 322, 325, 324, 326, 359, 363, 361, 327, 332, 329, 331, 333, 334, 335, 364, 358, 340, 214, 219, 190, 186],
        7: [0]
    }

    config.sourceLabels = {0: "bangs", 1: "hair", 2: "face", 3: "upper body", 4: "lower body",
                           5: "jacket", 6: "props (headphones, pin, ID card)", 7: "eyes"}
    config.targetLabels = {0: "bangs", 1: "hair", 2: "face", 3: "upper body", 4: "lower body",
                           5: "hip pouch", 6: "jacket", 7: "eyes"}
    return config


def main():
    parser = argparse.ArgumentParser(description = "Lynae -> LynaePeppermint, as a WWMI-template config")
    parser.add_argument("mod", help = "the mod folder (every Lynae .ini under it is fixed)")
    parser.add_argument("--undo", action = "store_true", help = "undo a previous fix instead")
    parser.add_argument("--keepRabbitFX", action = "store_true", help = "DIAGNOSTIC: keep the mod's RabbitFX lines in the remapped sections")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--download", default = None,
                        help = "the API's downloadMode, eg. `disabled` -- omit for the API's own default")
    args = parser.parse_args()

    facts = textureFacts()
    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser(SrcName, FRB.makeWWMIParser(parserConfig(facts)))
    FRB.CppStrategyOverrides.setFixer(SrcName, DstName, FRB.makeWWMIFixer(fixerConfig(facts, args.keepRabbitFX)))

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

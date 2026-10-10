#
# ===== peppermintLynaeFix (prototype v1) =====
#
# LynaePeppermint -> Lynae (Wuthering Waves): the reverse of lynaePeppermintFix.py, a skin back onto her
# character, both past 256 merged bones. Built FROM the library -- a WWMIParserConfig handed to
# makeWWMIParser and a WWMIFixerConfig handed to makeWWMIFixer, registered through CppStrategyOverrides
# and run by RemapService -- so the port is a transcription into IniParseData/LynaePeppermint/ and
# IniFixData/LynaePeppermint/.
#
#   py -3 peppermintLynaeFix.py <mod folder>                     fix every LynaePeppermint .ini under the folder
#   py -3 peppermintLynaeFix.py <mod folder> --download disabled no downloads (for a deterministic A/B)
#   py -3 peppermintLynaeFix.py <mod folder> --verbose           attach the API's logger
#   py -3 peppermintLynaeFix.py <mod folder> --undo              undo a previous fix
#
# ---- What this direction needs that the forward one did not (2026-10-05) ----
#
#   * The TARGET (Lynae) draws passes with her skeleton in vs-cb3 alone: her early depth passes, 11
#     draws of her dump. The fix's merge list read vs-cb3 as the previous pose on every draw, which is
#     what dropped her ID card, pin and ear cups out of her own identity mod when its builder did the
#     same. WWMIFixerConfig::currentPoseInCb3Only (new, opt-in) is WWMI Tools' own `elif vs-cb3` branch.
#   * Her lower body's outline pass (30ab50e7) binds a 2048 white map at ps-t0 and the diffuse at ps-t1,
#     where every other outline takes the diffuse at ps-t0. A flat white there drew the outline shell of
#     the skin's layered thigh overlay as two black bands down the backs of her thighs; the skin's own
#     outline reads its DIFFUSE at ps-t0, so the diffuse goes at both.
#   * Her props slot (card, pin, headphones) is NOT a cloth slot: it renders through a special-material
#     (glass / foil) layer, and anything drawn there through the ordinary passes barely reaches the
#     G-buffer -- measured in a frame dump, 700 pixels for a whole shirt, and 137 for her OWN props in
#     the vanilla frame. So nothing of the skin goes there: the skin's two cloth components (her coat,
#     5, and her shirt / jacket / shoes, 6) BOTH land on Lynae's jacket slot, the second through a
#     generated copy .ini (a merge), and her props slot is left hidden.
#   * That merge needed two template fixes for a target past 256 bones (2026-10-06): each copy gets
#     merge-only sections for the slots other files draw (its own skeleton was otherwise zero there),
#     and the bone-data marker and shape-key overrides go in the mod's own file only (a copy repeating
#     them drew the whole model as giant polygons). Both are WWMIFixerConfig::copiesShareSkeleton,
#     opt-in so ChisaParfait -> Chisa's in-game-confirmed copies stay as they were.
#
# ---- The pairing, measured (2026-10-05) ----
#
#   0-4 and 7 one to one (the face and eyes are the same mesh); the skin's coat (5) and jacket (6)
#   both onto Lynae's jacket (5).
#
# GAPS this prototype knows about:
#   * The skin's sheer garments (her see-through shirt and coat) render opaque: Lynae's cloth shaders
#     have no such material.
#   * Lynae's sheer lower-body look has no counterpart on the skin, so nothing of the skin's draws there.
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

SrcName = "LynaePeppermint"
DstName = "Lynae"
Downloads = os.path.join(Repo, "Data", "Mod Downloads", "WuWa", "LynaePeppermint", "3_7")
B = FRB.WWMIFixerConfig.Binding


# ---- her textures by the hash the game binds them under (her 3.7 dump), then the older generation
# wwmiHashHistory.py proved from LynaePeppermint2 ----
CurrentRoles = {
    "59324a0f": "bangsMask", "180c8e8a": "bangsDiffuse", "1dbd2313": "bangsStrand",
    "34926f91": "hairMask", "c56aa503": "hairDiffuse", "5a750238": "hairRamp", "a299690c": "hairStrand",
    "4644dbe8": "hairShadeRamp",
    "701b1015": "faceMask", "bf5c5233": "faceMap", "6bc6b4c8": "faceDiffuse",
    "14b3b873": "upperNormal", "4f2871af": "upperMask", "7003edde": "upperDetail", "e546902d": "upperDiffuse",
    "b66b4c2a": "lowerNormal", "fef651e2": "lowerMask", "0fa3ab67": "lowerDetail", "16e48d10": "lowerDiffuse",
    "6f56f1a3": "pouchNormal", "ee1f2bc0": "pouchMask", "15f5a1c4": "pouchDetail", "46bb3c2d": "pouchDiffuse",
    "9617b922": "jacketNormal", "ca537274": "jacketMask", "cb051033": "jacketDiffuse", "a1fa0024": "jacketDetail",
    "6bf7a371": "eyeIris", "a506a70d": "eyeMask", "a004a399": "eyeDiffuse",
}
OlderRoles = {
    "6b7842d9": "bangsDiffuse", "b7820acf": "hairDiffuse",
    "a37e527f": "upperNormal", "68a0988e": "upperMask", "ad3621fc": "upperDiffuse",
    "73975da7": "lowerNormal", "e3466a1e": "lowerMask", "2b8fb2f2": "lowerDiffuse",
    "fa1f94a9": "jacketNormal", "33594042": "jacketMask", "561382b1": "jacketDiffuse",
}


def rabbit(diffuse, mask, normal = None):
    keys = {"Resource\\RabbitFX\\Diffuse": diffuse, "Resource\\RabbitFX\\Lightmap": mask}
    if (normal):
        keys["Resource\\RabbitFX\\Normalmap"] = normal
    return keys


# ---- which role each of HER registers binds, per component (her Peppermint dump, wwmiPassLayout.py) ----
RegisterRoles = {
    0: {"ps-t0": "bangsMask", "ps-t1": "bangsDiffuse", "ps-t4": "hairShadeRamp", "ps-t5": "bangsStrand", **rabbit("bangsDiffuse", "bangsMask")},
    1: {"ps-t0": "hairMask", "ps-t1": "hairDiffuse", "ps-t2": "hairRamp", "ps-t4": "hairShadeRamp", "ps-t5": "hairStrand", **rabbit("hairDiffuse", "hairMask")},
    2: {"ps-t0": "faceMask", "ps-t1": "faceMap", "ps-t2": "faceDiffuse", **rabbit("faceDiffuse", "faceMask")},
    3: {"ps-t0": "upperNormal", "ps-t1": "upperMask", "ps-t2": "upperDetail", "ps-t3": "upperDiffuse", **rabbit("upperDiffuse", "upperMask", "upperNormal")},
    4: {"ps-t0": "lowerNormal", "ps-t1": "lowerMask", "ps-t2": "lowerDetail", "ps-t3": "lowerDiffuse", **rabbit("lowerDiffuse", "lowerMask", "lowerNormal")},
    5: {"ps-t0": "pouchNormal", "ps-t1": "pouchMask", "ps-t2": "pouchDetail", "ps-t3": "pouchDiffuse", **rabbit("pouchDiffuse", "pouchMask", "pouchNormal")},
    # her jacket shader reads mask / diffuse / detail at ps-t1 / t2 / t3
    6: {"ps-t0": "jacketNormal", "ps-t1": "jacketMask", "ps-t2": "jacketDiffuse", "ps-t3": "jacketDetail", **rabbit("jacketDiffuse", "jacketMask", "jacketNormal")},
    7: {"ps-t0": "eyeIris", "ps-t1": "eyeMask", "ps-t2": "eyeDiffuse", **rabbit("eyeDiffuse", "eyeMask")},
}

# The eye textures are the same hashes on both skins, and the face map / mask are not bound on Lynae
NoFallback = {"eyeIris", "eyeMask", "eyeDiffuse", "faceMask", "faceMap"}


def thumbprints():
    import wwmiTextureThumbs
    result = {}
    for h in CurrentRoles:
        path = os.path.join(Downloads, f"LynaePeppermintTexture{h}.dds")
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
    facts.downloadCharFolder = "LynaePeppermint"
    facts.downloadVersionFolder = "3_7"
    facts.downloadPrefix = "LynaePeppermint"
    facts.fallbackTextures = {role: h for h, role in CurrentRoles.items() if role not in NoFallback}
    return facts


def parserConfig(facts) -> "FRB.WWMIParserConfig":
    config = FRB.WWMIParserConfig()
    config.version = "3.7"
    config.textures = facts
    return config


def fixerConfig(facts) -> "FRB.WWMIFixerConfig":
    config = FRB.WWMIFixerConfig()
    config.targetId = FRB.ModTypeId.Lynae
    config.version = "3.7"            # her live vb0 is 3.7's, and her bones are numbered as at 3.7
    config.sourceVersion = "3.7"

    # ---- the TARGET's draws read their skeleton from vs-cb3 alone on her early depth passes ----
    config.currentPoseInCb3Only = True
    # ...and two source components share her jacket slot through a copy .ini, which must share the
    # mod's skeleton state (see the header)
    config.copiesShareSkeleton = True

    # ---- the passes the TARGET (Lynae) draws each slot on (her dumps, wwmiDrawTable.py) ----
    config.slotPasses = [
        ["d4252cf5b68968eb", "5eb19d847b81ed33"],   # 0 bangs (the second pass inherits the first's set)
        ["d4252cf5b68968eb"],                       # 1 hair
        ["97ce9ee79fd51349"],                       # 2 face
        ["a175ff3bd01feb29", "8d8250706d224af7"],   # 3 upper body (the second inherits)
        ["a68c6144f18d6caf"],                       # 4 lower body
        ["404f5464fdffc665"],                       # 5 jacket
        # 6 props: nothing of the skin is drawn there (see the header), so only the hide section uses it
        ["0f6f8facde912ff4", "2d6d59feda76cef9", "208e7eadc60a7abd", "0fc420109de57a0e"],
        ["fa9e4d98ed0a570e"],                       # 7 eyes (and e04f4df8, in extraPassRegs)
    ]

    # ---- every pass gated through its VERTEX shaders ----
    config.passVertexShaders = {
        "d4252cf5b68968eb": ["3e7bb648e306c671"],
        "5eb19d847b81ed33": ["3e7bb648e306c671"],
        "97ce9ee79fd51349": ["7c0b4db32cee62d3"],
        "a175ff3bd01feb29": ["1f324d354402418c"],
        "8d8250706d224af7": ["1f324d354402418c"],
        "a68c6144f18d6caf": ["343a49bd31719ade", "c30da0cb86079064"],
        "404f5464fdffc665": ["6ffcf365937e50bd"],
        "0f6f8facde912ff4": ["d4edc0d609271c1a"],
        "2d6d59feda76cef9": ["3424c4bc44c29aad"],
        "208e7eadc60a7abd": ["759b7b30c86ca081"],
        "0fc420109de57a0e": ["e2cb95b268cbb51e"],
        "bb718d7619295f25": ["99b91a71ed833cb8"],
        "9a01e7bd0aeff915": ["3b8be044e56e8c14"],
        "fa9e4d98ed0a570e": ["c32a69851757154a", "ceff9a81afb1de70"],
        "e04f4df80ee6b0ab": ["5fd6e5bb6ff81c53", "a54621ce48ed541b"],
        "bce1512f1c6b82fe": ["a57b6349c93b6107"],
        "f8c96a270bf847dd": ["6a6650a9db8983ce", "e4a3da6d1d1068b9"],
        "30ab50e715dce218": ["6a6650a9db8983ce", "e4a3da6d1d1068b9"],
    }

    # ---- every tag NAMED, and every one another pair tags keeps THAT value (a shader holds one
    # filter_index across every loaded .ini): Chisa's 3e7bb648 .71, 343a49bd .713, 5fd6e5bb .718,
    # 6a6650a9 .73, e4a3da6d .731, ChisaParfait -> Chisa's 7c0b4db3 .76, and this pair's forward
    # direction's .61x. The shaders only this direction tags take 3381.62x ----
    config.filterIndices = {
        "3e7bb648e306c671": "3381.71",
        "343a49bd31719ade": "3381.713",
        "5fd6e5bb6ff81c53": "3381.718",
        "6a6650a9db8983ce": "3381.73",
        "e4a3da6d1d1068b9": "3381.731",
        "7c0b4db32cee62d3": "3381.76",
        "c30da0cb86079064": "3381.61",
        "1f324d354402418c": "3381.612",
        "c32a69851757154a": "3381.615",
        "a54621ce48ed541b": "3381.616",
        "a57b6349c93b6107": "3381.617",
        "6ffcf365937e50bd": "3381.62",
        "d4edc0d609271c1a": "3381.621",
        "3424c4bc44c29aad": "3381.622",
        "759b7b30c86ca081": "3381.623",
        "e2cb95b268cbb51e": "3381.624",
        "ceff9a81afb1de70": "3381.625",
        "99b91a71ed833cb8": "3381.626",
        "3b8be044e56e8c14": "3381.627",
    }

    # ---- source component -> Lynae's slot and the registers it binds there ----
    config.plan = {
        # ps-t2 (36c90686) and ps-t3 (4eaa9816) are the same texture on both skins: left to the game
        0: FRB.WWMIFixerConfig.SourceComponent(0, [B("ps-t0", "bangsMask"), B("ps-t1", "bangsDiffuse"),
                                                   B("ps-t4", "hairShadeRamp"), B("ps-t5", "bangsStrand")]),
        1: FRB.WWMIFixerConfig.SourceComponent(1, [B("ps-t0", "hairMask"), B("ps-t1", "hairDiffuse"), B("ps-t2", "hairRamp"),
                                                   B("ps-t4", "hairShadeRamp"), B("ps-t5", "hairStrand")]),
        # Lynae's face shader reads the diffuse at ps-t1; her mask (packed differently) stays -- the
        # face is the same mesh, so its UVs are hers
        2: FRB.WWMIFixerConfig.SourceComponent(2, [B("ps-t1", "faceDiffuse")]),
        3: FRB.WWMIFixerConfig.SourceComponent(3, [B("ps-t0", "upperNormal"), B("ps-t1", "upperMask"),
                                                   B("ps-t2", "upperDetail"), B("ps-t3", "upperDiffuse")]),
        4: FRB.WWMIFixerConfig.SourceComponent(4, [B("ps-t0", "lowerNormal"), B("ps-t1", "lowerMask"),
                                                   B("ps-t2", "lowerDetail"), B("ps-t3", "lowerDiffuse")]),
        # the coat AND the jacket onto Lynae's jacket (her shader reads detail at t2, diffuse at t3);
        # the coat, first in order, stays in the mod's own file and the jacket goes to the copy
        5: FRB.WWMIFixerConfig.SourceComponent(5, [B("ps-t0", "pouchNormal"), B("ps-t1", "pouchMask"),
                                                   B("ps-t2", "pouchDetail"), B("ps-t3", "pouchDiffuse")]),
        6: FRB.WWMIFixerConfig.SourceComponent(5, [B("ps-t0", "jacketNormal"), B("ps-t1", "jacketMask"),
                                                   B("ps-t2", "jacketDetail"), B("ps-t3", "jacketDiffuse")]),
        7: FRB.WWMIFixerConfig.SourceComponent(7, [B("ps-t0", "eyeIris"), B("ps-t1", "eyeMask"), B("ps-t2", "eyeDiffuse")]),
    }

    # ---- a slot's OTHER passes ----
    config.extraPassRegs = {
        0: {"bce1512f1c6b82fe": [B("ps-t0", "bangsDiffuse")]},
        1: {"f8c96a270bf847dd": [B("ps-t0", "hairDiffuse")]},
        3: {"f8c96a270bf847dd": [B("ps-t0", "upperDiffuse")]},
        # her lower body's outline reads a mask at ps-t0 and the diffuse at ps-t1. Her own mask there is a
        # flat white ("outline everywhere"); the skin's outline reads its DIFFUSE at ps-t0 instead, and
        # with the white the outline shell of the skin's layered thigh overlay drew as two black bands
        # down the backs of her thighs (2026-10-06). So the skin's diffuse goes at both
        4: {"30ab50e715dce218": [B("ps-t0", "lowerDiffuse"), B("ps-t1", "lowerDiffuse")]},
        # her jacket slot takes TWO sources (the coat, 5, and the jacket, 6), and "the diffuse" is a
        # different file for each: without srcComponent both land in one list and the jacket's wins
        5: {"f8c96a270bf847dd": [B("ps-t0", "pouchDiffuse", 5), B("ps-t0", "jacketDiffuse", 6)]},
        # her second eye pass SETS its own t0, and it is the eye MASK (a506a70d in her dump's draw of
        # it): the iris the main eye pass reads there would be the wrong role (the forward direction's
        # row, the same shader on both skins)
        7: {"e04f4df80ee6b0ab": [B("ps-t0", "eyeMask")]},
    }

    config.sourceTextures = facts

    # ---- her material masks mark the region under a SHEER garment with R = 0 (the bikini under her
    # see-through shirt, a coat's translucent panels); Lynae's masks are R = 255 almost everywhere, and
    # her shaders do not draw an R = 0 texel as cloth -- a skin mod's coat vanished from Lynae. Every
    # R = 0 becomes 255, the cloth both legends agree on; G (how shiny) and the packing stay ----
    def maskFilter(alpha):
      def make(ctx):
        def run(tex):
            tex.gamma = None                     # these bytes are data, not colour
            px = bytearray(tex.getPixels())
            red = px[0::4]
            px[0::4] = bytes(255 if r == 0 else r for r in red)
            # and A: hers is 255 over the translucent regions (40% of her jacket mask), Lynae's is 0
            # almost everywhere
            px[3::4] = bytes([alpha]) * (len(px) // 4)
            tex.setPixels(bytes(px), tex.width, tex.height)
        return run
      return make

    E = FRB.WWMIFixerConfig.TexEdit
    config.texEdits = [E(role, "Repack", maskFilter(0)) for role in ("upperMask", "lowerMask", "jacketMask", "pouchMask")]

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
    config.texEdits = list(config.texEdits) + [E(role, "SkinCode", skinCodeSwap) for role in ("upperDetail", "lowerDetail")]

    # Masks mark regions: a flat one from a mod is replaced by HER game texture
    config.flatFallsBackToSource = {"upperMask", "lowerMask", "jacketMask", "pouchMask", "hairMask", "bangsMask"}

    R = FRB.WWMIFixerConfig.RegRemoval
    config.removedRegs = [
        R("ResourceBlendBufferOverride", "ref"),
        R("ResourceMergedSkeletonOverride", "ref"),
        R("ResourceExtraMergedSkeletonOverride", "ref"),
        R("Resource\\RabbitFX\\Diffuse"), R("Resource\\RabbitFX\\Lightmap"), R("Resource\\RabbitFX\\Normalmap"),
        R("Resource\\RabbitFX\\Materialmap"), R("Resource\\RabbitFX\\Cutoutmap"), R("Resource\\RabbitFX\\Specialmap"),
        R("run", "commandlist\\rabbitfx\\settextures"),
    ]

    # ---- the shape keys are retargeted; a batched export's dispatch height is LYNAE's (her
    # Metadata.json's dispatch_y) ----
    config.hiddenObjs = []
    config.zeroShapeKeyStream = False
    config.shapeKeyDispatchSize = "1631"
    config.cleanTexcoords = True
    # a mod's line that puts a texture into another kind of slot on purpose keeps that slot (Lynae3's
    # U toggle binds each part's diffuse into the ramp / detail slot, ps-t2, which both skins read alike)
    config.carryByRegisterRole = True
    # Lynae's merged skeleton is 401 slots (1203 float4)
    config.mergedSkeletonSlots = 1536

    # ---- a mod from before WWMI's merged skeleton: the skin was never renumbered, so her 3.7 vg_map ----
    with open(os.path.join(Downloads, "LynaePeppermintMetadata.json"), encoding = "utf-8") as f:
        metadata = json.load(f)
    config.sourceVgMaps = {i: [int(c["vg_map"][str(k)]) for k in range(len(c["vg_map"]))]
                           for i, c in enumerate(metadata["components"])}

    config.sourceLabels = {0: "bangs", 1: "hair", 2: "face", 3: "upper body", 4: "lower body",
                           5: "coat", 6: "shirt, jacket and shoes", 7: "eyes"}
    # (copyPreamble, the comment heading the generated copy .ini, is set on the compiled row only: the
    # constant, IniComments::GIMIObjMergerPreamble, is not bound)
    config.targetLabels = {0: "bangs", 1: "hair", 2: "face", 3: "upper body", 4: "lower body",
                           5: "jacket", 6: "props (headphones, pin, ID card)", 7: "eyes"}
    return config


def main():
    parser = argparse.ArgumentParser(description = "LynaePeppermint -> Lynae, as a WWMI-template config")
    parser.add_argument("mod", help = "the mod folder (every LynaePeppermint .ini under it is fixed)")
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

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
#   * A mod that draws a component from its OWN mesh buffers (Changli13: [ResourcePositionBuffer.Component5] /
#     .Component7 and Blend.buf under Meshes/Split and Meshes/Tattoo, each merged into a private skeleton
#     window -- vg_offset 0, so its ids are that component's LOCAL ones): the template remaps one mesh's blend,
#     skips these two ("Blend.buf and Position.buf do not agree") and the remapped sections then bind the
#     RemapBlend files it never wrote. LIBRARY GAP for the port: a blend resource takes the Position.buf its
#     own shared-resource list binds beside it, and a section whose vg_offset is 0 has its ids lifted through
#     that component's vg_map. Component 7 is her chest FLAME TATTOO (the Tattoo folder's name says so).
#   * Old skeletons: 1.1 / 1.2 rows are registered at run time (registerOldRows) and each mod's numbering is
#     read off its geometry. Every old mod in hand weights no coat bone (ids <= 228), so 1.1 and 1.2 read
#     alike for them; Changli12's hands pick 1.4 over 1.2 by only 1.98 / 2.12 -- check its fingers in game.
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
# ...plus RabbitFX's resource names, for a mod that hands its art to RabbitFX by name rather than by
# register (Changli13's Textures/textures.ini binds its bodies only as Resource\RabbitFX\Diffuse): the
# Lightmap is the mask, as on every pair so far
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
    3: {"ps-t0": "upperNormal", "ps-t1": "upperMask", "ps-t2": "upperDiffuse", "ps-t3": "upperDetail", "ps-t4": "upperRamp",
        **rabbit("upperDiffuse", "upperMask", "upperNormal")},
    4: {"ps-t0": "lowerNormal", "ps-t1": "lowerMask", "ps-t2": "lowerDiffuse", **rabbit("lowerDiffuse", "lowerMask", "lowerNormal")},
    5: {"ps-t0": "coatNormal", "ps-t1": "coatDiffuse", **rabbit("coatDiffuse", None, "coatNormal")},
    6: {"ps-t0": "eyeIris", "ps-t1": "eyeMask", "ps-t2": "eyeDiffuse", **rabbit("eyeDiffuse", "eyeMask")},
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


# ---- her skeleton's renumberings. A mod carries the bone ids of the game version its author exported
# at, and her merged skeleton was renumbered twice. Both maps are the community wwmi_fix_23's
# (WWMI_FIX_23.py, the remaps it applies to her mods' Blend.buf): its "fd9483ca" list takes a 1.1 id to
# 1.2 (only these 39 entries differ from identity; ids 238 and 254 both become 252), and its
# "060f5303" map takes a 1.2 / 1.3 merged-skeleton id to 1.4, unchanged since ----
Renumber11To12 = {
    238: 252, 239: 238, 240: 257, 241: 239, 242: 240, 243: 241, 244: 242, 245: 243, 246: 244, 247: 245,
    248: 246, 249: 247, 250: 248, 251: 249, 252: 250, 253: 251, 254: 252, 255: 253, 256: 254, 257: 255,
    258: 256, 259: 258, 260: 259, 261: 260, 262: 261, 263: 262, 264: 263, 265: 264, 266: 265, 267: 266,
    268: 267, 269: 268, 270: 269, 271: 270, 272: 271, 273: 272, 274: 273, 275: 274, 276: 275,
}
Renumber12To14 = {135: 137, 137: 135, 123: 124, 124: 125, 125: 126, 126: 123}
OldSkeletonSlots = 282      # 1.1's: component 5 one bone wider (vg 229/48, 6 at 277, 7 at 278-281)


def toCurrent(version, bone):
    if (version == "1.1"):
        bone = Renumber11To12.get(bone, bone)
    if (version in ("1.1", "1.2")):
        bone = Renumber12To14.get(bone, bone)
    return bone


def registerOldRows():
    """The vertex group rows for a mod in her 1.1 or 1.2 numbering, read off the reviewed 1.4 row through
    each renumbering. GAP for the port: these go into VGRemapData.cpp beside the 1.4 row (Lynae's 3.6 row
    is the precedent)."""
    source = FRB.WWMIBuilder.changli()
    current = source.getVGRemap(DstName, FRB.Version.parse("1.4"), FRB.Version.parse("2.4"))
    row = dict(current.remap)
    for version in ("1.1", "1.2"):
        old = {bone: row.get(toCurrent(version, bone), 0) for bone in range(OldSkeletonSlots)}
        source.vgRemaps.addRows([([version, SrcName, "", "2.4", DstName, ""], old)])


def boneCentroids():
    """Her rest-pose bone centroids in the current numbering, off the download folder's own buffers"""
    import numpy as np
    pos = np.fromfile(os.path.join(Downloads, "ChangliPosition.buf"), np.float32).reshape(-1, 3)
    blend = np.fromfile(os.path.join(Downloads, "ChangliBlend.buf"), np.uint8).reshape(len(pos), -1)
    n = blend.shape[1] // 2
    wide = np.fromfile(os.path.join(Downloads, "ChangliBlendRemapVertexVG.buf"), np.uint16).reshape(len(pos), -1)[:, :n]
    heaviest = np.argmax(blend[:, n:], axis = 1)
    bone = wide[np.arange(len(pos)), heaviest].astype(int)
    strong = blend[np.arange(len(pos)), n + heaviest] > 127
    result = {}
    for b in np.unique(bone[strong]):
        sel = strong & (bone == b)
        if (sel.sum() >= 3):
            result[int(b)] = tuple(float(x) for x in pos[sel].mean(axis = 0))
    return result


def currentVgMaps():
    """Each component's local -> merged map out of her current Metadata.json, for a mod from before WWMI's
    merged skeleton (Changli10). GAP: a pre-merged-skeleton mod of her 1.1 / 1.2 mesh would need that
    version's maps, which no source holds; none is in hand."""
    with open(os.path.join(Downloads, "ChangliMetadata.json"), encoding = "utf-8") as f:
        meta = json.load(f)
    maps = {}
    for k, comp in enumerate(meta["components"]):
        vg = {int(a): int(b) for a, b in comp["vg_map"].items()}
        maps[k] = [vg.get(i, 0) for i in range(max(vg) + 1)] if vg else [0]
    return maps


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
    # ...but the vb0 is not what the BONES are numbered by: the community hash tools rewrite a mod's
    # hashes and leave its ids (Changli2 / 7 carry 1.2's vb0 over 1.1's vg windows). So the blend's row
    # is chosen by the mod's own geometry, Lynae's way
    N = FRB.WWMIFixerConfig.SkeletonNumbering
    config.skeletonNumberings = [
        N("1.1", {b: toCurrent("1.1", b) for b in range(OldSkeletonSlots) if toCurrent("1.1", b) != b}),
        N("1.2", dict(Renumber12To14)),
        N("1.4", {}),
    ]
    config.referenceBoneCentroids = boneCentroids()
    # a mod from before WWMI's merged skeleton (Changli10) holds each component's LOCAL ids
    config.sourceVgMaps = currentVgMaps()

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
        # the face and eye-highlight shaders are shared, and other frames draw them through a second vertex
        # shader (the Sanhua, Chisa and Lynae pairs list both): gated on one, the textures would flicker
        "ed1c0f8b2ba08ac4": ["7c0b4db32cee62d3", "4ce754b5b9574b1a"],
        "d5572b1cbbba07a4": ["cb05fd1251af9a73"],
        "14fee50a0ce4ec91": ["e00893f9d07dc9b0"],
        "7f639abb370f996d": ["a434a95920c66e51"],
        "5616e4de7d4e70ef": ["ceff9a81afb1de70"],
        "43b4972776737988": ["a434a95920c66e51"],
        "f8c96a270bf847dd": ["6a6650a9db8983ce", "e4a3da6d1d1068b9"],
        "705575a2e1b4a4b9": ["7148f95feba974ac"],
        "e04f4df80ee6b0ab": ["5fd6e5bb6ff81c53", "a54621ce48ed541b"],
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
        "4ce754b5b9574b1a": "3381.769",
        "a54621ce48ed541b": "3381.616",
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
        # her emissive chest piece: its one texture is the glow map the skin's body reads at t3. The body
        # shader's normal and mask are UV-mapped, so flat ones stand in at the piece's UVs (the masks share
        # one legend: R = 255 bare skin, so R 0 is cloth)
        7: SC(4, [B("ps-t0", "flatNormal"), B("ps-t1", "flatClothMask"), B("ps-t2", "glow"), B("ps-t3", "glow")]),
    }
    # her legs and chest piece share slot 4 with her upper body: drawn from the mod's OWN file, where the
    # shape-keyed positions are computed (a copy computes none), as Lynae's lower body is
    plan = config.plan                   # pybind hands back a copy: edit it, then assign it back
    plan[4].sameFileSection = True
    plan[7].sameFileSection = True
    # her chest piece has no outline pass on her own draws
    plan[7].offPasses = ["f8c96a270bf847dd"]
    config.plan = plan

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
    def created(role, colour):
        c = FRB.WWMIFixerConfig.CreatedTexture()
        c.role, c.colour = role, colour
        return c
    # the skin's body normal map averages (126, 127, 47, 102): flat x / y, its own B and A kept
    config.createdTextures = [noGlow, created("flatNormal", FRB.CppColour(128, 128, 47, 102)),
                              created("flatClothMask", FRB.CppColour(0, 128, 0, 255))]

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
    # Changli13 draws from mod.ini and binds every texture from Textures/textures.ini
    config.readInisBelow = True
    # the skin's slots reach bone 263 (264 x 3 float4 = 792), past the 768 default
    config.mergedSkeletonSlots = 1536
    # INERT for every mod in hand: her slot-4 sources are drawn from the mod's own file (sameFileSection),
    # so no copy .ini is written; kept so a copy, should one ever be written, shares the skeleton state
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
    registerOldRows()
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

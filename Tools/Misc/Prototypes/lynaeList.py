# LYNAE -> LYNAEPEPPERMINT, AS A LIST OF EDITS
#
# The parser sorts the mod's sections into graphs (a WWMISectionClassifier over Lynae's tables); the fixer is the list
# EDITS: the library's generic edits, with this pair's choices written out as tables.
#
# NO GAME DATA IS WRITTEN HERE: every register, draw pass, hash, index, game shader, shape-key order, skeleton numbering,
# vertex group map, code map field and texture description is read from the API's versioned asset tables (ModType's
# TextureRegisters, DrawPasses, Hashes, Indices, GameShaders, ShapeKeyRemaps, VGRemaps, BoneCentroids,
# ShapeKeyDispatchSizes, MaterialCodes, TextureDescs) at the RUN's versions -- the ones the run
# is asked for (--fromVersion / --version), else her and the skin's newest. So the lists are built per run (lists(FROM,
# TO)), and what this file holds at its top is only the pair's version-free CHOICES: which component goes where, which
# role lands on which, which part her own shader draws.
#
# Beyond Sanhua's list: LynaePeppermint's skeleton passes 256 bones, so the fix merges and remaps the skeleton itself
# (SKELETON) and writes WWMI's blend remap; her parts draw with her OWN pixel shaders (OWN_SHADERS); her lower body goes
# through the skin's upper-body slot as a second section of the mod's own file (SAME_FILE); and the shape keys are put in
# the skin's order.
#
#   py -3 lynaeList.py <mod folder> [--fromVersion X] [--version Y] [--download normal]
import argparse
import itertools
import os
import struct
import sys
import tempfile
import types

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "Anime Game Remap (for all users)", "api"))
import src.py.FixRaidenBoss2 as FRB          # noqa: E402
from wwmiSectionClassifier import WWMISectionClassifier    # noqa: E402

K = FRB.IniKeywords
SOURCE, TARGET = "Lynae", "LynaePeppermint"
S, T = FRB.ModTypes[SOURCE].value, FRB.ModTypes[TARGET].value
Z3 = FRB.Z3Context()
V = FRB.Version.parse
NEWEST = lambda modType: modType.hashes.versionOf(modType.hashes.get([modType.name, "vb0"], None, False), [modType.name, "vb0"])   # her newest model
RENAME = lambda name: FRB.IniNamingTools.getRemapFixName(name, TARGET)
CAPITAL = lambda role: role[:1].upper() + role[1:]
# a mod from before WWMI's merged skeleton (no vg_offset): its blend names each component's OWN bones, lifted into the merged
# skeleton through her older vg_maps (WWMIBlend.LegacyLift)
MERGED = lambda: any(row.key.strip().lower() == K.WWMIVgOffset.lower() for row in data().collected(FRB.IniParseRows.Objs))
CUSTOM_SHADER = "CustomShader"                  # (not yet an IniKeywords constant)


def need(value, what):
    """A table row the list cannot do without: missing, the list says which rather than writing a wrong section"""
    if value is None:
        raise LookupError("no asset table row for " + what)
    return value


# ================================================================================================= the pair's choices
# each source component's target slot; SAME_FILE: drawn as a second section on its slot, in the mod's own file
SLOTS = {"component0": "component0", "component1": "component1", "component2": "component2", "component3": "component3",
         "component4": "component3", "component5": "component6", "component6": "component5", "component7": "component7"}
SAME_FILE = ["component4"]

# each target section's texture list: the draw whose REGISTERS it is bound in -- the target slot's, or (a part her own
# shader draws, OWN_SHADERS) her own component's -- and each role of the mod's under that draw's name for it
MAIN = {"component0": (SOURCE, "component0", {"bangsMask": "bangsMask", "bangsDiffuse": "bangsDiffuse", "hairShadeRamp": "hairShadeRamp",
                                              "bangsStrand": "bangsStrand"}),
        "component1": (SOURCE, "component1", {"hairMask": "hairMask", "hairDiffuse": "hairDiffuse", "hairRamp": "hairRamp",
                                              "hairShadeRamp": "hairShadeRamp", "hairStrand": "hairStrand"}),
        "component2": (TARGET, "component2", {"faceDiffuse": "faceDiffuse"}),
        "component3": (SOURCE, "component3", {"upperNormal": "upperNormal", "upperMask": "upperMask", "upperDetail": "upperDetail",
                                              "upperDiffuse": "upperDiffuse"}),
        "component4": (SOURCE, "component4", {"lowerNormal": "lowerNormal", "lowerMask": "lowerMask", "lowerDetail": "lowerDetail",
                                              "lowerDiffuse": "lowerDiffuse", "filmFoil": "filmFoil", "lowerSheer": "lowerSheer"}),
        "component5": (SOURCE, "component5", {"jacketNormal": "jacketNormal", "jacketMask": "jacketMask", "jacketDetail": "jacketDetail",
                                              "jacketDiffuse": "jacketDiffuse", "filmFoil": "filmFoil"}),
        # her props on the skin's hip pouch, her foil where the pouch reads its own
        "component6": (TARGET, "component5", {"propsNormal": "pouchNormal", "propsMask": "pouchMask", "propsDetail": "pouchDetail",
                                              "propsDiffuse": "pouchDiffuse", "filmFoil": "pouchFoil"}),
        "component7": (TARGET, "component7", {"eyeIris": "eyeIris", "eyeMask": "eyeMask", "eyeDiffuse": "eyeDiffuse"}),
        # the reroutes (REROUTES): her tights as her lower body; her jacket's and props' film by her film shader (her jacket's)
        "component104": (SOURCE, "component4", {"lowerNormal": "lowerNormal", "lowerMask": "lowerMask", "lowerDetail": "lowerDetail",
                                                "lowerDiffuse": "lowerDiffuse", "filmFoil": "filmFoil", "lowerSheer": "lowerSheer"}),
        "component105": (SOURCE, "component5", {"jacketNormal": "jacketNormal", "jacketMask": "jacketMask", "jacketDetail": "jacketDetail",
                                                "jacketDiffuse": "jacketDiffuse", "filmFoil": "filmFoil"}),
        "component106": (SOURCE, "component5", {"propsNormal": "jacketNormal", "propsMask": "jacketMask", "propsDetail": "jacketDetail",
                                                "propsDiffuse": "jacketDiffuse", "filmFoil": "filmFoil"})}

# ...and its list for the slot's OTHER pass (an outline, the eye's second pass), run right before its draw: each role
# under the TARGET slot's name for it
OTHER = {"component0": {"bangsDiffuse": "bangsDiffuse"}, "component1": {"hairDiffuse": "hairDiffuse"},
         "component3": {"upperDiffuse": "upperDiffuse"}, "component4": {"lowerDiffuse": "upperDiffuse"},
         "component5": {"jacketDiffuse": "jacketDiffuse"}, "component6": {"propsDiffuse": "pouchDiffuse"},
         "component7": {"eyeMask": "eyeMask"}, "component105": {"jacketDiffuse": "pouchDiffuse"},
         "component106": {"propsDiffuse": "pouchDiffuse"}}

# a role the mod has no texture for: Lynae's own game texture, downloaded -- but her eyes', which the skin's eye slot
# draws with its own (GAME_LEFT)
GAME_LEFT = ["eyeIris", "eyeMask", "eyeDiffuse"]
DOWNLOAD = lambda role: FRB.IniNamingTools.getRemapDLResourceName(SOURCE + CAPITAL(role))
GAME = lambda role: FRB.IniNamingTools.getRemapDLResourceName(SOURCE + CAPITAL(role) + "Game")   # ...as a toggle's default

# her parts drawn with her OWN game pixel shaders: per target section, the draw's name, her game shader, the component of
# hers whose draw it is, its culling (None: the slot's own state), and the game textures it reads besides its list -- each
# supplied as SHADER_READS says
STASH = lambda reg: K.Resource + RENAME("Stash4" + FRB.IniNamingTools.getRegTag(reg))
OWN_SHADERS = {"component0": ("LynaeBangs", "LynaeBangs", "component0", "none", ["ramp"]),
               "component1": ("LynaeHair", "LynaeHair", "component1", "none", ["ramp"]),
               "component3": ("LynaeUpper", "LynaeUpper", "component3", "back", ["matcap", "ramp"]),
               "component4": ("LynaeLower", "LynaeLower", "component4", "back", ["glow", "flow", "noise", "ramp"]),
               "component5": ("LynaeJacket", "LynaeFilm", "component5", "back", ["ramp"]),
               "component104": ("LynaeLower", "LynaeLower", "component4", "back", ["glow", "flow", "noise", "ramp"]),
               "component105": ("LynaeFilm", "LynaeFilm", "component5", None, ["ramp"]),
               "component106": ("LynaeFilm", "LynaeFilm", "component5", None, ["ramp"])}
# ...her ramp downloaded; the matcap both skins share, where the skin's slot holds it; the game's effect textures (glow, flow,
# noise), stashed off the skin's lower-body draw (STASHED)
SHADER_READS = {"ramp": "download", "matcap": "skin", "glow": "stash", "flow": "stash", "noise": "stash"}

# DRAW REROUTES: the draws of a component whose texels carry a flag of its code map go through another plan, as a second
# section in the mod's own file -- her jacket's and props' FILM through the hip pouch with her film shader, and her lower
# body's TIGHTS (and shorts) as her lower body, but off the slot's outline pass. Per reroute: its source component, its
# target slot, the code map's role, the flag (its bits read from MaterialCodes), the share of a draw's vertices on flagged
# texels that takes it, and whether her own game code map stands in for a mod with no code map of its own (fetched as a
# download is: a run with downloads disabled has none)
REROUTES = {"component104": ("component4", "component3", "lowerDetail", "tights", 0.01, False),
            "component105": ("component5", "component5", "jacketDetail", "film", 0.5, False),
            "component106": ("component6", "component5", "propsDetail", "film", 0.5, True)}
OFF_OTHER = ["component104"]                                            # a reroute kept off the slot's other pass

SRC = lambda c: REROUTES[c][0] if c in REROUTES else c                  # the source component a target section is drawn from
SLOT_OF = {**SLOTS, **{r: v[1] for r, v in REROUTES.items()}}           # every target section's slot
TARGET_OBJ = {c: (TARGET, slot + "_" + c[len("component"):]) if c in SAME_FILE or c in REROUTES else (TARGET, slot) for c, slot in SLOT_OF.items()}
REROUTES_OF = lambda c: [r for r, v in REROUTES.items() if v[0] == c]
REROUTE_RENAME = lambda r: (lambda name: RENAME(name + "Reroute" + r[len("component"):]))

# RABBITFX ON A PART HER OWN SHADER DRAWS: a mod may hand a part's maps to RabbitFX (its SetTextures), which routes them by
# the kind of texture at ps-t0..t3 -- on the skin's slot, other files. So the mod's RabbitFX lines are kept and the routing
# PINNED to hers after the call: per register, the tag RabbitFX gives her texture there (her game texture's, or the mod's
# own file in its place), as RabbitFX's x187..w187 flags -- her game texture's tag read off what her own draw binds there
# (DrawPasses' main.ps-t0..t3) and how the game creates it (TextureDescs, RabbitFX.tagOf). The parts:
RABBITFX = ["component0", "component1", "component3", "component4", "component5"]
RABBITFX_LINES = [reg for reg, _ in [("Resource\\RabbitFX\\Diffuse", ""), ("Resource\\RabbitFX\\Lightmap", ""), ("Resource\\RabbitFX\\Normalmap", ""),
                                     ("Resource\\RabbitFX\\Materialmap", ""), ("Resource\\RabbitFX\\Cutoutmap", ""), ("Resource\\RabbitFX\\Specialmap", "")]]
PIN = lambda c: RENAME(K.CommandList + "RabbitFXPin" + c[len("component"):])

# what the shader draws set, saved and put back around them
BACKUP = lambda reg: K.Resource + RENAME("ShaderBackup" + FRB.IniNamingTools.getRegTag(reg))

# a target slot nothing of the mod lands on is hidden, its bones merged; and when the skin's lower-body slot is hidden while
# the mod draws her lower body, the game textures her lower body's shader reads are stashed off its draw
STASHED = {"component4": (["component4", "component104"], ["glow", "flow", "noise"])}   # hidden slot -> (the sections reading it, roles)
TARGET_LABELS = {"component0": "bangs", "component1": "hair", "component2": "face", "component3": "upper body",
                 "component4": "lower body", "component5": "hip pouch", "component6": "jacket", "component7": "eyes"}


# the buffers the mod's shared list rebinds, parked first and put back by the cleanup list; and the lines a remapped
# section drops (WWMI's blend remap of the SOURCE, its skeleton merge, RabbitFX's)
BYPASSABLE = ["ib"] + ["vb" + str(n) for n in range(1, 10)]          # vb0 the mod parks itself
BYPASSED = lambda: [reg for reg in BYPASSABLE if any(row.section.lower() == SHARED.lower() and row.key.strip().lower() == reg
                                                     for row in data().collected(FRB.IniParseRows.Objs))]
REMOVED = [("ResourceBlendBufferOverride", "ref"), ("ResourceMergedSkeletonOverride", "ref"), ("ResourceExtraMergedSkeletonOverride", "ref"),
           (K.Run, "commandlistmergeskeleton"), ("CheckTextureOverride", "vs-cb3"),
           ("CheckTextureOverride", "vs-cb4"), ("vs-cb3", "resourceextramergedskeleton"), ("vs-cb4", "resourcemergedskeleton"),
           ("vs-cb3", "resourcemergedskeleton")]
SHARED, CLEANUP = "CommandListOverrideSharedResources", "CommandListCleanupSharedResources"   # the mod's lists around a draw


# her props go onto the skin's hip pouch, whose shader draws every material her props' code map uses as its foil: all but
# her film become its plain material -- her code map's fields and the pouch's plain code read from MaterialCodes. A texture
# edit is a filter, or a function of the run's versions that makes one
def plainProps(FROM, TO):
    film = need(S.getMaterialCode("component6", "propsDetail.flags.film", FROM), "Lynae's props film flag")
    material = need(S.getMaterialCode("component6", "propsDetail.material", FROM), "Lynae's props material field")
    plain = need(T.getMaterialCode("component5", "pouchDetail.material.plain", TO), "the pouch's plain material")
    return FRB.ChannelMapFilter.codeMap([code if code & film else (code & ~material) | plain for code in range(256)])


TEX_EDITS = {("component6", "propsDetail"): ("PropsDetailPlain", plainProps)}

# the skeleton the fix merges and remaps: per bone-data buffer (vs-cb4, vs-cb3), its merged and remapped buffers
FILTER = "3381.7777"
SK = lambda name: K.Resource + RENAME(name)
CBS = [(K.WWMIBoneDataCb, "MergedSkeleton", "RemappedSkeleton"), (K.WWMIExtraBoneDataCb, "ExtraMergedSkeleton", "ExtraRemappedSkeleton")]
SHAPE_KEYS = [("ShapeKeyVertexId", "DXGI_FORMAT_R32_UINT", "4"), ("ShapeKeyVertexOffset", "DXGI_FORMAT_R16_FLOAT", "2"),
              ("ShapeKeyOffset", "DXGI_FORMAT_R32G32B32A32_UINT", "16")]

NOT_TOGGLES = [K.WWMIObjectDetected, K.WWMIObjectDetected + "==1"]     # conditions every section of a WWMI mod sits under
SECTION_GUARDS = NOT_TOGGLES + [K.WWMIModEnabled.lower(), "resourcemergedskeleton!==null"]   # ...and a component's draw

# ================================================================================================= the graphs, by id
TEXTURE = FRB.IniGraphModObjKeywords.Texture
ROLE = lambda c, role: (0, "role", c + ":" + role)                     # the mod's resource for a component's role
REF = lambda role: (0, "ref", role)                                     # a resource the fix declares for a linked .ini's file
REGION_ROLES = ["bangsMask", "faceMask", "hairMask", "jacketMask", "lowerMask", "propsMask", "upperMask"]   # masks: a flat one marks nothing
GLOBAL = (0, "global", "")                                              # the whole file, while the parser reads it
data, ini = (lambda: FRB.EditScope.parseData()), (lambda: FRB.EditScope.ini())
OWN = lambda value: ini().resourceNamed(value)                          # the resource a value names ("" for none)
FILE = lambda section: (lambda row: row.value.strip() if row is not None else None)(
    data().firstCollected(FRB.IniParseRows.Declared, section, K.Filename))   # the file a resource section declares
# a resource the mod could use for a role: one whose file the folder holds -- and, for a mask, not a flat one (it marks no regions)
USABLE = lambda value, role: (lambda file: file is not None and os.path.isfile(ABS(file)) and not (
    role in REGION_ROLES and FRB.TexThumbprint.channelIsConstant(ABS(file), 0)))(FILE(OWN(value)))
THIS_ROWS = lambda role: [row for row in data().collected("this") if row.graphId[1:] == (TEXTURE, role) and row.key.strip().lower() == K.This]
SINGLE = lambda role: len(THIS_ROWS(role)) == 1   # one `this` toggles it
TOGGLED = lambda role: any(any(g.replace(" ", "").lower() not in NOT_TOGGLES for g in row.guards) for row in THIS_ROWS(role))         # the mod's override of the role's hash binds under a toggle
ABS = lambda rel: os.path.normpath(os.path.join(ini().folder, rel.replace("\\", os.sep)))   # a path the .ini names, on disk
# a component DRAWS when a line it reaches draws, or runs a list this file does not hold (a library's); one that draws nothing
# and skips the game's draw is UNDRAWN -- unless every component looks so (a mod the reading does not understand)
DRAWS = lambda c: any(tuple(row.graphId[1:]) == ("", c) and (row.key.strip().lower() == K.DrawIndexed or (
    row.key.strip().lower() == K.Run and ini().getSection(row.value.strip()) is None and "\\wwmiv1\\" not in row.value.lower()))
    for row in data().collected(FRB.IniParseRows.Objs))
KEPT = lambda c: DRAWS(c) or not SKIPS(c) or not any(DRAWS(other) for other in SLOTS)   # a component the fix takes
# the draws of a target section, numbered in order of (index count, first index): a reroute's, the ones it takes; a
# component's, the rest
SPLITS = lambda r: data().variables()["splits"].get(r, [])
SKIPS = lambda c: any(tuple(row.graphId[1:]) == ("", c) and row.key.strip().lower() == K.Handling and row.value.strip().lower() == K.Skip
                      for row in data().collected(FRB.IniParseRows.Objs))


# ================================================================================================= the lists, at a run's versions
def lists(FROM, TO):
    """The pair's parser and fixer at a run's versions -- FROM her mods', TO the skin's: every register, pass, hash, shader
    and table row read from the API's asset tables at them"""

    # ---- the registers of each texture list: MAIN's layouts, OTHER's slot's other pass (<role>@other)
    regOf = lambda who, comp, role: need((S.getTextureRegister(comp, role, FROM) if who == SOURCE else T.getTextureRegister(comp, role, TO)),
                                         who + " " + comp + " " + role)
    TEXTURES = {c: {regOf(who, comp, layoutRole): role for role, layoutRole in roles.items()} for c, (who, comp, roles) in MAIN.items()}
    PASS0 = {c: {regOf(TARGET, SLOT_OF[c], role + "@other"): srcRole for srcRole, role in roles.items()} for c, roles in OTHER.items()}

    # ---- the skin's slots and each one's draw passes, by their vertex shaders' tags (DrawPasses): a list binds only on its
    #      slot's passes
    TARGET_SLOTS = list(itertools.takewhile(lambda slot: T.indices.get([TARGET, "", slot], TO, False) is not None,
                                            ("component" + str(n) for n in itertools.count())))
    passNames = lambda slot, kind: list(itertools.takewhile(lambda name: T.getDrawPass(slot, name, TO) is not None,
                                                            [kind] + [kind + str(n) for n in range(2, 10)]))
    shadersOf = lambda slot, kind: [vs for name in passNames(slot, kind) for vs in T.getDrawPass(slot, name, TO).split(",")]
    tagsOf = lambda slot, kind: [tag for name in passNames(slot, kind) for tag in T.getDrawPass(slot, name + ".tags", TO).split(",")]
    PASSES = {vs: tag for kind in ("main", "other") for slot in TARGET_SLOTS for vs, tag in zip(shadersOf(slot, kind), tagsOf(slot, kind))}
    GUARDS = {slot: " || ".join("vs == " + tag for tag in dict.fromkeys(tagsOf(slot, "main"))) for slot in TARGET_SLOTS}
    PASS0_GUARDS = {slot: " || ".join("vs == " + tag for tag in dict.fromkeys(tagsOf(slot, "other"))) for slot in TARGET_SLOTS if tagsOf(slot, "other")}
    OFF_PASSES = {c: " && ".join("vs != " + tag for tag in dict.fromkeys(tagsOf(SLOT_OF[c], "other"))) for c in OFF_OTHER}

    # ---- her game textures (Hashes), each downloaded from the folder of its row's version
    roles = dict.fromkeys([role for _, _, rs in MAIN.values() for role in rs] + [role for rs in OTHER.values() for role in rs]
                          + [read for *_, reads in OWN_SHADERS.values() for read in reads if read == "ramp"])
    DOWNLOADS = {role: textureHash for role in sorted(roles) if role not in GAME_LEFT for textureHash in [S.hashes.get([SOURCE, role], FROM, False)]
                 if textureHash is not None}
    FOLDER = lambda role: FRB.DownloadTools.versionFolder(S.hashes.versionOf(DOWNLOADS[role], [SOURCE, role]))

    # ---- her own shaders (GameShaders): each draw's lines, its code and its constants
    STASH_REGS = {slot: [regOf(TARGET, slot, role) for role in roles] for slot, (_, roles) in STASHED.items()}   # where the skin's slot binds them
    supply = {"download": lambda c, role: DOWNLOAD(role),
              "skin": lambda c, role: K.Ref + " " + regOf(TARGET, SLOT_OF[c], role),
              "stash": lambda c, role: K.Ref + " " + STASH(next(regOf(TARGET, slot, role) for slot, (sections, _) in STASHED.items() if c in sections))}
    SHADERS = {c: (name, [(regOf(SOURCE, comp, read), supply[SHADER_READS[read]](c, read)) for read in shaderReads], cull, game)
               for c, (name, game, comp, cull, shaderReads) in OWN_SHADERS.items()}
    SHADER_CODE = {c: need(S.getGameShader(game, FROM), game) for c, (*_, game) in SHADERS.items()}
    SHADER_CB4 = {c: [int(word, 16) for word in need(S.getGameShader(game + ".ps-cb4", FROM), game + ".ps-cb4").split(",")]
                  for c, (*_, game) in SHADERS.items()}

    # ---- where her own draws bind each role, as her mods write it (her register layout, and RabbitFX's keys)
    SOURCE_REGISTERS = {c: {reg: role for role in dict.fromkeys(role for d, (_, _, rs) in MAIN.items() if SRC(d) == c for role in rs)
                            for reg in (S.getTextureRegister(c, role + "@rabbitfx", FROM), S.getTextureRegister(c, role, FROM)) if reg is not None}
                        for c in SLOTS}

    # ---- her skeleton's numberings: one per model of hers (her vb0's versions), each onto the newest (VGRemaps); a mod's
    #      blend is in one of them, read off its geometry against her bones' rest-pose centroids (BoneCentroids)
    VB0 = {value.lower(): version for version, names in S.hashes.repo.toNestedDict().items() for kind, value in names.get(SOURCE, {}).items()
           if kind == "vb0"}
    VERSIONS = sorted(set(VB0.values()), key = V)
    REFERENCE = VERSIONS[-1]
    # each of her components' vertex groups in her merged skeleton at a version (VGRemaps, component -> ""): a mod from
    # before WWMI's merged skeleton is an export of her first model, so its blend is lifted through her oldest
    VG_MAPS = lambda version: {int(c[len("component"):]): [row.remap[i] for i in range(len(row.remap))]
                               for c in SLOTS for row in [need(S.getVGRemap(SOURCE, V(version), V(version), c, ""), SOURCE + " " + c + " vg_map at " + version)]}
    NUMBERINGS = [FRB.SkeletonNumberingProbe.Numbering(version, {k: v for k, v in need(S.getVGRemap(SOURCE, V(version), V(REFERENCE)),
                                                                                         SOURCE + " " + version + " numbering").remap.items() if k != v})
                  for version in VERSIONS]
    CENTROIDS = S.getBoneCentroids(V(REFERENCE))
    SHAPE_KEY_ORDER = list(need(S.getShapeKeyRemap(TARGET, FROM, TO), "the shape-key order"))
    DISPATCH = need(T.getShapeKeyDispatchSize(TO), "the shape-key dispatch size")

    # ---- the source's numbering -> the vertex group row, and what depends on it
    NUMBERING = lambda: data().variables()["boneNumbering"]                  # the mod's, as the parser read it
    VG_ROW = lambda version: S.getVGRemap(TARGET, V(version), TO)          # the vertex group row from that numbering
    DRAWS_OF = lambda c: SPLITS(c) if c in REROUTES else [v for v in draws(c) if not any(v in SPLITS(r) for r in REROUTES_OF(c))]
    DRAWN = lambda c: KEPT(SRC(c)) and bool(DRAWS_OF(c))                    # a target section with a draw

    # what the shader draws set, saved and put back around them: the constant buffer, and each drawn shader's registers
    SHADER_REGS = lambda: ["ps-cb4"] + sorted({reg for c, (_, lines, _, _) in SHADERS.items() if DRAWN(c) for reg, _ in lines},
                                              key = lambda reg: int(reg[len("ps-t"):]))
    CB4 = lambda c: K.Resource + RENAME(SHADERS[c][0] + "Pscb4")

    BONES = lambda: str(len(set(VG_ROW(NUMBERING()).remap.values())))       # the bones the mod's blend remap names

    # ...or the override that toggles it, copied once per register it is bound at (and once more for a part her own shader
    # draws, whose art stays unedited: "raw")
    TOGGLE = lambda c, reg, role: (0, "toggle", role + ":" + reg + (":raw" if c in SHADERS else ""))
    ROLES = list(dict.fromkeys((SRC(c), role) for c, lines in list(TEXTURES.items()) + list(PASS0.items()) for role in lines.values()))
    LISTS = [(c, "", lines) for c, lines in TEXTURES.items()] + [(c, "Pass0", lines) for c, lines in PASS0.items()]
    LINES = [(c, reg, role) for c, lines in TEXTURES.items() for reg, role in lines.items()]
    ALL_LINES = [(c, kind, reg, role) for c, kind, lines in LISTS for reg, role in lines.items()]
    TOGGLES = list(dict.fromkeys(TOGGLE(c, reg, role) for c, kind, reg, role in ALL_LINES))

    # ================================================================================================= the parser
    # The classifier sorts the sections (WWMISectionClassifier: components by vb0 + match_first_index, the hash-only objects,
    # each role's texture overrides by every hash the role has had); these edits read what the fixer needs
    CLASSIFIER = WWMISectionClassifier(S, FROM, list(SLOTS), [role for _, role in ROLES],
                                       {"cb4": "boneData", "shapekey_offsets": "shapekeyOffsets", "shapekey_scale": "shapekeyScale"}, [SOURCE, TARGET])
    PARSE = [
        # the rows the fixer reads: every declaration and every key of the file, of each component, of each role's overrides
        FRB.GraphCreate(GLOBAL, z3 = Z3, sectionsOf = lambda group: list(ini().getIfTemplates().values()) if group == 0 else []),
        FRB.GraphCollect({GLOBAL: [K.Filename, K.Type, K.Format, K.Stride]}, name = FRB.IniParseRows.Declared),
        FRB.GraphCollect({GLOBAL: lambda key: True}, name = FRB.IniParseRows.Sections),
        FRB.GraphRemove([GLOBAL]),
        FRB.GraphCollect({(0, "", c): lambda key: True for c in SLOTS}, name = FRB.IniParseRows.Objs, withConditions = True),   # (with each row's root section)

        # a section that only WATCHES the source's vb0 is the fix's only in a file that draws none of her components (a help
        # overlay's, a skill effect's): there it is carried onto the target's vb0; beside her components it rides along as written
        FRB.GraphFilter([(0, "", "watch")], lambda owner, part, i: not data().collected(FRB.IniParseRows.Objs), removeEmptied = True),
        FRB.GraphCollect({(0, TEXTURE, role): [K.Hash, K.This] for _, role in ROLES}, name = "this", withConditions = True),

        # which numbering the mod's bone ids are in
        FRB.GraphVarApply([], lambda: boneNumbering(), "boneNumbering"),

        # a component the mod draws nothing of, skipping the game's draw, is none of this fix's: its target slot is hidden
        *[FRB.GraphFilter([(0, "", c)], lambda owner, part, i, c = c: KEPT(c),
                          removeEmptied = True) for c in SLOTS],

        # each component's resource for a role: the mod's override of the role's hash (when it does not toggle it) -- else what
        # the component binds at the role's register (an existing role graph is kept); either only when it is USABLE
        *[FRB.ResRegCollect({(0, TEXTURE, role): K.This}, {"": FRB.ResIdentity(ROLE(c, role), createResModel = False, fileOf = OWN)},
                            resPredicates = {(0, TEXTURE, role): lambda reg, value, d, role = role: OWN(value) != "" and USABLE(value, role) and not TOGGLED(role)})
          for c, role in ROLES],
        *[FRB.ResRegCollect({(0, "", c): reg}, {"": FRB.ResIdentity(ROLE(c, role), createResModel = False, fileOf = OWN)},
                            resPredicates = {(0, "", c): lambda r, value, d, c = c, reg = reg, role = role: OWN(value) != "" and USABLE(value, role)
                                             and not REG_TOGGLES(c, reg)})
          for c, regs in SOURCE_REGISTERS.items() for reg, role in regs.items() if (c, role) in ROLES],

        # the draws each reroute takes
        FRB.GraphVarApply([], lambda: splitDraws(), "splits"),

        # ...or the override that toggles it, as it is: a copy per register it is bound at
        *[FRB.GraphCopy((0, TEXTURE, role), toggle, lambda owner, part, i, role = role: TOGGLED(role) and part.getByInd(i)[0].lower() not in K.MatchKeys,
                        keepEmpty = False)
          for toggle, role in dict.fromkeys((TOGGLE(c, reg, role), role) for c, kind, reg, role in ALL_LINES)],


        # ...or, toggled at the component's own register: each binding under its condition, as a toggle of the fix's
        *[FRB.GraphCreate(TOGGLE(c, reg, role), z3 = Z3, onlyWithout = [TOGGLE(c, reg, role)[1:]], sectionsOf = lambda group, c = c, reg = reg, role = role: (
            lambda rows: [REG_TOGGLE_SECTION(rows, reg)] if rows else None)(
            [row for src, regs in SOURCE_REGISTERS.items() if src == SRC(c) for sreg, r in regs.items() if r == role for row in REG_TOGGLES(src, sreg)]))
          for c, kind, reg, role in ALL_LINES],

        # a role graph left empty is no graph: a fix tells "the mod has none" by its absence
        FRB.GraphFilter([ROLE(c, role) for c, role in ROLES], lambda owner, part, i: True, removeEmptied = True),
        # ...and a toggle copy no drawn section's list binds is none
        *[FRB.GraphFilter([toggle], lambda owner, part, i, toggle = toggle: any(TOGGLE(c, reg, role) == toggle and KEPT(SRC(c)) and (c not in REROUTES or SPLITS(c))
                                                                                for c, _, reg, role in ALL_LINES), removeEmptied = True)
          for toggle in TOGGLES],

        # a role no line of this file binds, but a LINKED .ini binds on the component's draw: its file, declared by the fix
        FRB.GraphVarApply([], lambda: linkedFiles(), "linkedFiles"),
        *[FRB.GraphCreate(graph, z3 = Z3, onlyWithout = [ROLE(c, role)[1:]], sectionsOf = lambda group, c = c, role = role: (
            lambda rel: [FRB.SectionText(Z3, REF_NAME(role)).key(K.Filename, rel).build()] if rel else None)(data().variables()["linkedFiles"].get((c, role))))
          for c, role in ROLES for graph in (REF(role), ROLE(c, role))],   # the declaration first: it is made while the role has none
    ]
    REF_NAME = lambda role: K.Resource + FRB.IniNamingTools.getModSuffixedName(CAPITAL(role), K.RemapRef, TARGET)


    def rabbitFXFlags(c):
        """RabbitFX's x187..w187 for a component: per register ps-t0..t3, the tag of what she has there -- the mod's own file when
        the component binds the register itself (untoggled), else the mod's `this` over her game texture of the role, else her
        game texture's tag"""
        bound = lambda k: need(S.getDrawPass(c, "main.ps-t" + str(k), FROM), SOURCE + " " + c + " main.ps-t" + str(k))
        tags = [FRB.RabbitFX.tagOf(*need(S.getTextureDesc(bound(k), FROM), SOURCE + " texture " + bound(k))) for k in range(4)]
        tagOf = lambda resource: (lambda file: FRB.RabbitFX.textureTag(ABS(file)) if file and os.path.isfile(ABS(file)) else None)(FILE(OWN(resource)))
        for k in range(4):
            reg = "ps-t" + str(k)
            role = SOURCE_REGISTERS.get(c, {}).get(reg)
            if role is None:
                continue
            bound = [row.value for row in data().collected(FRB.IniParseRows.Objs) if tuple(row.graphId[1:]) == ("", c) and FRB.IniNamingTools.sameKey(row.key, reg)]
            # the mod's `this` over her CURRENT game texture of the role (the override whose hash is it)
            sections = {row.section for row in data().collected("this") if row.graphId[1:] == (TEXTURE, role) and row.key.strip().lower() == K.Hash
                        and row.value.strip().lower() == DOWNLOADS.get(role, "").lower()}
            overrides = [row.value for row in THIS_ROWS(role) if row.section in sections]
            tag = tagOf(bound[0]) if bound and not TOGGLED(role) else None
            if tag is None and overrides:
                tag = tagOf(overrides[0])
            if tag is not None:
                tags[k] = tag
        return FRB.RabbitFX.routingFlags(tags)


    def SHADER_DRAW(c, i, value):
        """One draw of a target section through her own shader: a list that runs it on the slot's pass, saving and putting back
        what it sets, and draws as the mod does elsewhere (a reroute kept off a pass: not there); and the CustomShader itself.
        A shader with her own render state (a cull) runs only where the draw has a render target (o3)"""
        name, lines, cull, _ = SHADERS[c]
        custom, regs = DRAW(c, i).replace(K.CommandList, CUSTOM_SHADER, 1), ["ps-cb4"] + [reg for reg, _ in lines]
        offPass = RENAME(K.CommandList + "OffPass" + c[len("component"):] + "Draw" + "_".join(value.replace(" ", "").split(",")[:2]))
        plain = [(K.Run, offPass)] if c in OFF_PASSES else [(K.DrawIndexed, value)]
        text = FRB.SectionText(Z3, DRAW(c, i))
        text = text.open("o3 !== null") if cull else text
        text = (text.open(GUARDS[SLOT_OF[c]]).keys([(BACKUP(reg), K.Ref + " " + reg) for reg in regs]).key(K.Run, custom)
                .keys([(reg, K.Ref + " " + BACKUP(reg)) for reg in regs]).orElse().keys(plain).close())
        text = text.orElse().keys(plain).close() if cull else text
        state = [("blend", "disable"), ("rasterizer_state_merge", "true"), ("cull", cull)] if cull else []
        sections = [text.build(), FRB.SectionText(Z3, custom).keys([("ps", BESIDE_MESH(FRB.IniNamingTools.getRemapElementName("", name, TARGET) + ".hlsl")),
                                                                    ("ps-cb4", CB4(c)), *lines, *state, (K.DrawIndexed, value)]).build()]
        if c in OFF_PASSES:
            sections.append(FRB.SectionText(Z3, offPass).open(OFF_PASSES[c]).key(K.DrawIndexed, value).close().build())
        return sections


    def REG_TOGGLES(c, reg):
        """The rows binding a register in a component's own sections as a TOGGLE: several resources, each under one condition of
        the mod's (none when they are not)"""
        rows = [row for row in data().collected(FRB.IniParseRows.Objs) if tuple(row.graphId[1:]) == ("", c) and FRB.IniNamingTools.sameKey(row.key, reg)
                and OWN(row.value)]
        toggles = [[g for g in row.guards if g.replace(" ", "").lower() not in SECTION_GUARDS] for row in rows]
        flat = rows and all(len(conditions) == 1 for conditions in toggles)
        return rows if flat and len({row.value.strip().lower() for row in rows}) > 1 else []


    def REG_TOGGLE_SECTION(rows, reg):
        """A toggle of the fix's from bindings at a component's register: each under its own conditions, at 'reg'"""
        text = FRB.SectionText(Z3, "CommandListRegisterToggle")
        for row in rows:
            conditions = [g for g in row.guards if g.replace(" ", "").lower() not in SECTION_GUARDS]
            text = text.open(" && ".join(conditions)).key(reg, row.value.strip()).close()
        return text.build()


    def splitDraws():
        """Per reroute: the draws of its source component more than its share of whose vertices land on texels of its code map
        carrying its flags (DrawUVCoverage) -- the code map the mod draws the component with, else her own game map's flags"""
        index, texcoord, position = FILE(K.WWMIIndexBuffer), FILE(K.WWMITexcoordBuffer), FILE(K.WWMIPositionBuffer)
        found, coverage = {}, None
        if not (index and texcoord and position):
            return found
        for r, (c, _, role, flagName, share, gameMap) in REROUTES.items():
            if not KEPT(c) or not draws(c):
                continue
            flags = need(S.getMaterialCode(c, role + ".flags." + flagName, FROM), SOURCE + " " + c + " " + role + " flag " + flagName)
            code = codeMap(c, role)
            code = ABS(code) if code is not None else (gameTexture(role) if gameMap else None)
            if code is not None:
                tex = FRB.CppTextureFile(code)
                tex.open()
                if not tex.hasImage:
                    continue
                codes, w, h = tex.shaderChannel(0), tex.width, tex.height
                test = lambda x, y, codes = codes, w = w, flags = flags: (codes[y * w + x] & flags) != 0
            else:
                continue
            coverage = coverage or FRB.DrawUVCoverage.ofMesh(ABS(index), ABS(texcoord), os.path.getsize(ABS(position)) // FRB.WWMIBlend.PositionStride)
            ranges = [tuple(int(n) for n in v.split(",")[:2]) for v in draws(c)]
            taken = set(map(tuple, coverage.coveredRanges(ranges, w, h, test, share)))
            found[r] = [v for v in draws(c) if tuple(int(n) for n in v.split(",")[:2]) in taken]
        return found


    GAME_FOLDER = os.path.join(tempfile.gettempdir(), "AGRemapGameTextures")    # her game textures a step reads, not the mod's

    def gameTexture(role):
        """Her game texture of a role, fetched as a download is (into GAME_FOLDER, not the mod's folder) -- none when the run's
        downloads are disabled, or the fetch fails"""
        if ini().downloadMode == FRB.DownloadMode.Disabled or role not in DOWNLOADS:
            return None
        try:
            return FRB.FileDownload(FRB.DownloadTools.url("WuWa", SOURCE, FOLDER(role), SOURCE, "Texture" + DOWNLOADS[role], FRB.FileExt.DDS),
                                    FRB.DownloadTools.fixedFileName(SOURCE, CAPITAL(role), FRB.FileExt.DDS)).get(GAME_FOLDER)[0]
        except Exception as error:
            print("could not fetch " + SOURCE + "'s " + role + ": " + str(error))
            return None

    def codeMap(c, role):
        """The file the mod draws a component's code map with: its override of the role's hash, else what the component binds
        at the role's register -- a usable one, of the mod's own (not a previous fix's)"""
        candidates = [row.value for row in THIS_ROWS(role) if not TOGGLED(role)]
        candidates += [row.value for row in data().collected(FRB.IniParseRows.Objs) if tuple(row.graphId[1:]) == ("", c)
                       and any(FRB.IniNamingTools.sameKey(reg, row.key) for reg, r in SOURCE_REGISTERS.get(c, {}).items() if r == role)]
        usable = [value for value in candidates if OWN(value) and USABLE(value, role) and not FRB.IniNamingTools.looksRemapped(OWN(value), [SOURCE, TARGET])]
        return FILE(OWN(usable[0])) if usable else None


    def boneNumbering():
        """The version whose numbering the mod's blend gives its bone ids in: off its geometry (a merged mod's), else by its vb0,
        else her oldest"""
        position, blend = FILE(K.WWMIPositionBuffer), FILE(K.WWMIBlendBuffer)
        vertexVG = FILE(K.Resource + "BlendRemapVertexVGBuffer")
        if position and blend and MERGED():
            found = FRB.SkeletonNumberingProbe.probe(ABS(position), ABS(blend), ABS(vertexVG) if vertexVG else None, NUMBERINGS, CENTROIDS)
            if found is not None:
                return NUMBERINGS[found.best].version
        hashes = [row.value.strip().lower() for row in data().collected(FRB.IniParseRows.Objs) if row.key.strip().lower() == K.Hash]
        return next((VB0[h] for h in hashes if h in VB0), VERSIONS[0])


    LINKED_PARSE = [   # what is read of a linked .ini: its declarations, and every key of each component
        FRB.GraphCreate(GLOBAL, z3 = Z3, sectionsOf = lambda group: list(ini().getIfTemplates().values()) if group == 0 else []),
        FRB.GraphCollect({GLOBAL: [K.Filename]}, name = FRB.IniParseRows.Declared),
        FRB.GraphRemove([GLOBAL]),
        FRB.GraphCollect({(0, "", c): lambda key: True for c in SLOTS}, name = FRB.IniParseRows.Objs)]


    def linkedFiles():
        """Per (component, role): the file a LINKED .ini binds for it, on the component's own draw, at the role's register --
        relative to this .ini's folder"""
        found, here = {}, ini()
        for linked in here.linkedInis():
            sibling = here.siblingOf(linked, {getattr(FRB.ModTypeId, SOURCE)})
            reader = parser(sibling, LINKED_PARSE)
            reader.parse()
            for row in reader.parseData.collected(FRB.IniParseRows.Objs):
                c = row.graphId[2]
                role = next((r for reg, r in SOURCE_REGISTERS.get(c, {}).items() if FRB.IniNamingTools.sameKey(reg, row.key)), None)
                declared = reader.parseData.firstCollected(FRB.IniParseRows.Declared, sibling.resourceNamed(row.value), K.Filename)
                if role is not None and (c, role) in ROLES and declared is not None:
                    found.setdefault((c, role), os.path.relpath(os.path.join(sibling.folder, declared.value.strip()), here.folder))
        return found

    parser = lambda ini_, edits = None: FRB.GIMIParser(ini_, modObjs = CLASSIFIER.objects, objTargetFuncs = [CLASSIFIER], disjointModObjs = False,
                                                       modTypeId = getattr(FRB.ModTypeId, SOURCE), keysToTrack = {K.Hash, K.MatchFirstIndex, K.MatchIndexCount},
                                                       commandEdits = PARSE if edits is None else edits)

    # ================================================================================================= the fix's graphs, by id
    LIST = lambda c, kind = "": (0, "list" + kind, c)                       # a source component's texture list
    LIST_NAME = lambda c, kind = "": RENAME("CommandListLynae" + CAPITAL(c) + "Textures" + kind)
    STAND_IN = lambda c, role: (0, "standIn", c + ":" + role)
    SECTIONS, SKELETON, BLEND_REMAP = (0, "", "fixSections"), (0, "", "skeleton"), (0, "", "blendRemap")
    SHADER = lambda c: (0, "shader", c)
    PIN_GRAPH = lambda c: (0, "rabbitFXPin", c)
    # whether a component's sections run RabbitFX's SetTextures (directly or through a list they call)
    CALLS_RABBITFX = lambda c: any(tuple(row.graphId[1:]) == ("", c) and row.key.strip().lower() == K.Run
                                   and row.value.strip().lower() == K.RabbitFXSetTexturesPath.lower() for row in data().collected(FRB.IniParseRows.Objs))
    RESETS = (0, "", "resetsDetected")              # the mod's [Present] resets $object_detected: the skeleton remap is latched
    SOURCES = [("", c) for c in SLOTS]
    # a component's draws -- the ones in its own sections, not in a list they call, which the mod draws its own way -- numbered
    # in order of (index count, first index)
    ROOTS = lambda c: {row.rootSection for row in data().collected(FRB.IniParseRows.Objs) if tuple(row.graphId[1:]) == ("", c)}
    draws = lambda c: sorted(dict.fromkeys(row.value.strip() for row in data().collected(FRB.IniParseRows.Objs)
                                           if tuple(row.graphId[1:]) == ("", c) and row.key.strip().lower() == K.DrawIndexed
                                           and row.section == row.rootSection),
                             key = lambda value: tuple(int(n) for n in value.split(",")[:2]))
    MERGE = lambda slot: RENAME(K.CommandList + "MergeSlot" + slot[len("component"):])
    DRAW_NAMES = lambda c: {DRAW(c, i) for i in range(len(DRAWS_OF(c)))} if c in SHADERS else set()
    DRAW = lambda c, i: RENAME(K.CommandList + SHADERS[c][0] + c[len("component"):] + "Draw" + str(i))
    MESHES = lambda: os.path.dirname((FILE(K.WWMIBlendBuffer) or "").replace("\\", "/"))   # where the mod's mesh is
    BESIDE_MESH = lambda name: "/".join(part for part in (MESHES(), name) if part)

    # the files the fix makes, which the mod decides the place (beside its mesh) and the contents of: set by the list's step
    # "the fix's files", read when the files are written after the list
    SHAPE_KEY_FILES = FRB.FileAddCreate((0, "", "shapeKeyFiles"), {})
    SHADER_BUFFERS = FRB.FileAddCreate((0, "", "shaderBuffers"), {})
    SHADER_SOURCES = FRB.FileRefAdd({})
    TEXCOORD_FILE = FRB.FileAddCreate((0, "", "texcoordFile"), {})
    TEXCOORD = K.Resource + RENAME("TexcoordNoNaN")       # the cleaned copy of the mod's texcoords
    CLEANED = (0, "", "texcoordCleaned")                   # the copy differs from the mod's: the remapped sections bind it
    BLEND_INPUTS = {}                               # the mod's blend -> its position buffer and its own BlendRemapVertexVG


    def fixFiles():
        """The fix's files for the .ini the list runs on: the shape keys in the target's order, her shaders' code and
        constants, beside the mod's mesh; and what the blend remap reads besides the blend"""
        folder, abs_ = ini().folder, lambda rel: os.path.normpath(os.path.join(ini().folder, rel.replace("\\", os.sep)))
        keys = {name: FILE(K.Resource + name + "Buffer") for name, _, _ in SHAPE_KEYS}
        if all(keys.values()):
            mod = FRB.WWMIShapeKeys.Files(abs_(keys["ShapeKeyOffset"]), abs_(keys["ShapeKeyVertexId"]), abs_(keys["ShapeKeyVertexOffset"]))
            reordered = lambda which: getattr(FRB.WWMIShapeKeys.reordered(mod, SHAPE_KEY_ORDER), which)
            SHAPE_KEY_FILES.files = {K.Resource + RENAME(name + "Buffer"): FRB.FileAddCreate.File(
                keys = [(K.Type, K.Buffer), (K.Format, fmt), (K.Stride, stride)],
                file = BESIDE_MESH(FRB.IniNamingTools.getRemapElementName("", name, TARGET) + ".buf"),
                makeBytes = lambda which = {"ShapeKeyOffset": "offsets", "ShapeKeyVertexId": "vertexIds", "ShapeKeyVertexOffset": "vertexOffsets"}[name]: reordered(which))
                for name, fmt, stride in SHAPE_KEYS}
        SHADER_BUFFERS.files = {CB4(c): FRB.FileAddCreate.File(
            keys = [(K.Type, K.Buffer), (K.Format, "DXGI_FORMAT_R32G32B32A32_FLOAT")],
            file = BESIDE_MESH(FRB.IniNamingTools.getRemapElementName("", SHADERS[c][0] + "Pscb4", TARGET) + ".buf"),
            bytes = struct.pack("<%dI" % len(SHADER_CB4[c]), *SHADER_CB4[c]))
            for c in SHADERS if DRAWN(c)}
        SHADER_SOURCES.files = {BESIDE_MESH(FRB.IniNamingTools.getRemapElementName("", SHADERS[c][0], TARGET) + ".hlsl"): SHADER_CODE[c].encode("utf-8")
                                for c in SHADERS if DRAWN(c)}
        blend, position = FILE(K.WWMIBlendBuffer), FILE(K.WWMIPositionBuffer)
        vertexVG = FILE(K.Resource + "BlendRemapVertexVGBuffer")
        if blend and position:
            index = FILE(K.WWMIIndexBuffer)
            ranges = {int(c[len("component"):]): [(int(v.split(",")[0]), int(v.split(",")[1])) for v in draws(c)] for c in SLOTS}
            legacy = None if MERGED() or not index else FRB.WWMIBlend.LegacyLift(abs_(index), ranges, VG_MAPS(VERSIONS[0]))
            BLEND_INPUTS[abs_(blend)] = (abs_(position), abs_(vertexVG) if vertexVG else "", VG_ROW(NUMBERING()), legacy)
        # the texcoords, CLEANED for the target's passes: a NaN in the second UV (the skin's shader reads it, hers does not)
        # set to 0, and a U past 1 folded into [0, 1) unless its triangle straddles a tile -- written only when that changes
        # something; the mod's own buffer is untouched
        texcoord, index = FILE(K.WWMITexcoordBuffer), FILE(K.WWMIIndexBuffer)
        stride = data().firstCollected(FRB.IniParseRows.Declared, K.WWMITexcoordBuffer, K.Stride)
        stride = int(stride.value) if stride is not None and stride.value.strip().isdigit() else 16
        cleaned = False
        if texcoord and index and os.path.isfile(abs_(texcoord)):
            keep = FRB.TexcoordFile(abs_(texcoord), FRB.TexcoordFile.halfElements(stride)).straddling(FRB.IbFile(abs_(index)).getIndices())
            stats = FRB.TexcoordFile(abs_(texcoord), FRB.TexcoordFile.halfElements(stride)).clean(keep)
            cleaned = stats.cleared > 0 or stats.folded > 0
            fmt = data().firstCollected(FRB.IniParseRows.Declared, K.WWMITexcoordBuffer, K.Format)
            TEXCOORD_FILE.files = {TEXCOORD: FRB.FileAddCreate.File(
                keys = [(K.Type, K.Buffer), (K.Format, fmt.value.strip() if fmt is not None else "DXGI_FORMAT_R16G16_FLOAT"), (K.Stride, str(stride))],
                file = BESIDE_MESH(TARGET + K.Remap + "Texcoord.buf"),
                makeBytes = lambda path = abs_(texcoord): (lambda copy: (copy.clean(keep), copy.read())[1])(
                    FRB.TexcoordFile(path, FRB.TexcoordFile.halfElements(stride))))} if cleaned else {}
        return {"folder": folder, "texcoordCleaned": cleaned}


    def writeBlendRemap(resource):
        """The blend in the blend remap's LOCAL ids, and the remap's three buffers beside it (WWMIBlend.writeRemap)"""
        position, vertexVG, row, legacy = BLEND_INPUTS[os.path.normpath(resource.srcPath)]
        beside = lambda which: os.path.join(os.path.dirname(resource.fixedPath), FRB.IniNamingTools.getRemapElementName("", "BlendRemap" + which, TARGET) + ".buf")
        FRB.WWMIBlend.writeRemap(resource.srcPath, resource.fixedPath, vertexVG, position, row,
                                 FRB.WWMIBlend.RemapFiles(beside("VertexVG"), beside("Forward"), beside("Reverse")), legacy)
        return True


    def HIDE(slot):
        """A target slot nothing of the mod lands on: the skin's own geometry skipped, its bones merged -- by the fix's merge of
        its window, skipped while the mod's skeleton is (a merged mod); else by the slot's own merge list, and skipped (a mod
        from before WWMI's merged skeleton, which declares none) -- and what it stashes, stashed"""
        text = (FRB.SectionText(Z3, K.TextureOverride + TARGET + CAPITAL(slot) + K.Remap + "Hide")
                .prefix("; nothing of the mod is drawn through " + TARGET + "'s " + TARGET_LABELS[slot] + " slot: the skin's own geometry is skipped and its bones still merged")
                .keys([(K.Hash, T.hashes.get([TARGET, "vb0"], TO, False)), (K.MatchFirstIndex, str(T.indices.get([TARGET, "", slot], TO, False))),
                       (K.MatchIndexCount, str(T.indexCounts.get([TARGET, "", slot], TO, False))), (K.WWMIObjectDetected, "1")])
                .open(K.WWMIModEnabled))
        if MERGED():
            text = (text.keys([(K.WWMIVgOffset, str(T.vgOffsets.get([TARGET, "", slot], TO, False))), (K.WWMIVgCount, str(T.vgCounts.get([TARGET, "", slot], TO, False))),
                               (K.Run, RENAME(FRB.WWMINames.MergeWindowList))])
                    .open(K.Resource + "MergedSkeleton !== " + K.Null).key(K.Handling, K.Skip).close())
        else:
            text = text.keys([(K.Run, RENAME(K.CommandList + "MergeSlot" + slot[len("component"):])), (K.Handling, K.Skip)])
        if slot in STASHED and any(DRAWN(c) for c in STASHED[slot][0]):
            text = text.open("o3 !== null").keys([(STASH(reg), K.Ref + " " + reg) for reg in STASH_REGS[slot]]).close()
        return text.close().build()


    # ================================================================================================= the fix
    EDITS = [
        # ---- the mod's: does its [Present] reset $object_detected (the skeleton remap then runs once a frame on it)
        FRB.GraphCreate(RESETS, z3 = Z3, sectionsOf = lambda group: [FRB.SectionText(Z3, "").build()] if group == 0 and any(
            row.section == "Present" and row.key.strip().lower() == "post " + K.WWMIObjectDetected and row.value.strip() == "0"
            for row in data().collected(FRB.IniParseRows.Sections)) else None),

        # ---- the fix's files: where they go and what they hold, for this mod
        FRB.GraphVarApply([], fixFiles, "fixFiles"),

        # ---- per source component: its texture list first thing; its merge and the blend remap after the shared list; its
        #      other pass's list before each draw, and each draw through her own shader
        FRB.GraphGroupEdit([{("", c): [FRB.RegSurroundedAdd([(K.Run, RENAME(K.CommandList + "MergeSlot" + SLOTS[c][len("component"):])),
                                                             (K.Run, RENAME(K.CommandList + "BlendRemap"))],
                                                            {K.Run: lambda value: value.strip().lower() == SHARED.lower()}, {}, False),
                                       FRB.RegPartEdit(FRB.RegAnchorAdd([K.DrawIndexed], lambda key, value, c = c: (lambda r: [(K.Run, DRAW(r, DRAWS_OF(r).index(value.strip())))] if r
                                                                        else [(K.Run, DRAW(c, DRAWS_OF(c).index(value.strip())))] if c in SHADERS else [(key, value)])(
                                                                        next((r for r in REROUTES_OF(c) if value.strip() in DRAWS_OF(r)), None)),
                                                                        FRB.RegAnchorAdd.Placement.Replace),
                                                       lambda section, c = c: section in ROOTS(c))]
                             for c in SLOTS}]),

        # ---- RabbitFX pinned to her routing after the mod's call, on a part her own shader draws
        FRB.GraphGroupEdit([{("", c): [FRB.RegAnchorAdd([K.Run], lambda key, value, c = c: [(key, value), (K.Run, PIN(c))]
                                                        if value.strip().lower() == K.RabbitFXSetTexturesPath.lower() and CALLS_RABBITFX(c) else None,
                                                        FRB.RegAnchorAdd.Placement.Replace)]
                             for c in RABBITFX}]),
        *[FRB.GraphCreate(PIN_GRAPH(c), z3 = Z3, onlyWith = ("", c), sectionsOf = lambda group, c = c: [
            FRB.SectionText(Z3, PIN(c)).open(GUARDS[SLOTS[c]]).keys(list(zip(FRB.RabbitFX.FlagKeys, map(str, rabbitFXFlags(c))))).close().build()]
            if CALLS_RABBITFX(c) else None)
          for c in RABBITFX],

        # ---- the shared list parks the buffers it rebinds, and the cleanup list puts them back
        FRB.GraphGroupEdit([{("", c): [*[FRB.RegPartEdit(FRB.RegAnchorAdd([reg], lambda key, value, reg = reg: [(K.Resource + RENAME("Bypass" + reg.capitalize()), K.Ref + " " + reg)],
                                                                         FRB.RegAnchorAdd.Placement.Before, None, True, True),
                                                         lambda section: section.lower() == SHARED.lower()) for reg in BYPASSABLE],
                                       FRB.RegPartEdit(FRB.RegAdd([(K.Run, RENAME("CommandListRestoreSharedResources"))], True, True),
                                                       lambda section: section.lower() == CLEANUP.lower())]
                             for c in SLOTS}]),

        # ---- the fix's own sections when the mod draws any of Lynae's components: the parked buffers, the passes' tags, the
        #      registers a shader draw saves
        FRB.GraphCreate(SECTIONS, z3 = Z3, onlyWith = SOURCES, sectionsOf = lambda group: [
            *[FRB.SectionText(Z3, K.Resource + RENAME("Bypass" + reg.capitalize())).build() for reg in BYPASSED()],
            FRB.SectionText(Z3, RENAME("CommandListRestoreSharedResources"))
               .keys([(reg, K.Ref + " " + K.Resource + RENAME("Bypass" + reg.capitalize())) for reg in BYPASSED()]).build(),
            *[FRB.SectionText(Z3, RENAME("ShaderOverridePass" + str(i))).keys([(K.Hash, vs), (K.FilterIndex, tag)]).build()
              for i, (vs, tag) in enumerate(PASSES.items())],
            *[FRB.SectionText(Z3, BACKUP(reg)).build() for reg in SHADER_REGS()]]),

        # ---- the skeleton: merged into buffers of the fix's, remapped into LynaePeppermint's numbering once a frame, merged per
        #      slot (a merge list per slot; the hidden slot's window by its own list) -- and the stash of the hidden slot
        FRB.GraphCreate(SKELETON, z3 = Z3, onlyWith = SOURCES, sectionsOf = lambda group: [
            *[FRB.SectionText(Z3, STASH(reg)).build() for slot, (cs, _) in STASHED.items() if any(DRAWN(c) for c in cs) for reg in STASH_REGS[slot]],
            *[FRB.SectionText(Z3, SK(merged)).build() for _, merged, _ in CBS],
            *[FRB.SectionText(Z3, SK(merged + "RW")).keys([(K.Type, K.RWBuffer), (K.Format, K.FormatR32G32B32A32Float),
                                                             (K.Array, "1536")]).build() for _, merged, _ in CBS],
            *[FRB.SectionText(Z3, SK(name)).build() for _, _, remapped in CBS for name in (remapped,)],
            *[FRB.SectionText(Z3, SK(remapped + "RW")).build() for _, _, remapped in CBS],
            FRB.SectionText(Z3, RENAME(K.CommandList + "RemapMergedSkeleton")).keys(
                [line for _, merged, remapped in CBS for line in [
                    (SK(merged), K.Copy + " " + SK(merged + "RW")), (K.WWMIRemapperForwardReg, SK("BlendRemapForwardBuffer")),
                    (K.WWMIBlendRemapId, "0"), (K.WWMIVgCount, BONES()), (K.WWMIRemapperSkeletonReg, SK(merged)),
                    (SK(remapped + "RW"), K.Copy + " " + SK(merged + "RW")), (K.WWMIRemapperOutReg, SK(remapped + "RW")),
                    (K.Run, K.WWMISkeletonRemapper), (SK(remapped), K.Copy + " " + SK(remapped + "RW"))]]).build(),
            *[(lambda text: text.key(K.WWMIExtraBoneDataCb, SK("ExtraRemappedSkeleton")).build())(
                FRB.SectionText(Z3, RENAME(K.CommandList + "MergeSlot" + str(slot)))
                .open(K.WWMIBoneDataCb + " == " + FILTER).keys(
                    [(K.WWMIVgOffset, offset), (K.WWMIVgCount, count), (K.WWMICustomMeshScale, "1.00"), (K.WWMIMergerBonesReg, K.Ref + " " + K.WWMIBoneDataCb),
                     (K.WWMIMergerSkeletonReg, SK("MergedSkeletonRW")), (K.Run, K.WWMISkeletonMerger), (K.WWMIBoneDataCb, SK("RemappedSkeleton"))]).close()
                .open(K.WWMIExtraBoneDataCb + " == " + FILTER).keys(
                    [(K.WWMIVgOffset, offset), (K.WWMIVgCount, count), (K.WWMICustomMeshScale, "1.00"), (K.WWMIMergerBonesReg, K.Ref + " " + K.WWMIExtraBoneDataCb),
                     (K.WWMIMergerSkeletonReg, SK("ExtraMergedSkeletonRW")), (K.Run, K.WWMISkeletonMerger), (K.WWMIExtraBoneDataCb, SK("ExtraRemappedSkeleton"))]).close())
              for slot in range(len(TARGET_SLOTS))
              for offset, count in [(str(T.vgOffsets.get([TARGET, "", "component" + str(slot)], TO, False)),
                                     str(T.vgCounts.get([TARGET, "", "component" + str(slot)], TO, False)))]],
            FRB.SectionText(Z3, RENAME(FRB.WWMINames.MergeWindowList)).key(K.WWMICustomMeshScale, "1.00")
               .open(K.WWMIBoneDataCb + " == " + FILTER).keys([(K.WWMIMergerBonesReg, K.Ref + " " + K.WWMIBoneDataCb), (K.WWMIMergerSkeletonReg, SK("MergedSkeletonRW")),
                                                              (K.Run, K.WWMISkeletonMerger)]).close()
               .open(K.WWMIExtraBoneDataCb + " == " + FILTER).keys([(K.WWMIMergerBonesReg, K.Ref + " " + K.WWMIExtraBoneDataCb),
                                                                   (K.WWMIMergerSkeletonReg, SK("ExtraMergedSkeletonRW")), (K.Run, K.WWMISkeletonMerger)]).close().build(),
            FRB.SectionText(Z3, RENAME(K.TextureOverride + "MarkBoneDataCB")).keys(
                [(K.Hash, T.hashes.get([TARGET, "cb4"], TO, False)), (K.MatchPriority, "0"), (K.FilterIndex, FILTER)]).build()]),

        # ---- the blend remap: its buffers (beside the remapped blend), and its one-time run that rewrites the blend's ids into
        #      the remap's LOCAL space
        FRB.GraphCreate(BLEND_REMAP, z3 = Z3, onlyWith = SOURCES, sectionsOf = lambda group: [
            *[FRB.SectionText(Z3, SK("BlendRemap" + which + "Buffer")).keys(
                [(K.Type, K.Buffer), (K.Format, K.FormatR16UInt), (K.Filename, BESIDE_MESH(FRB.IniNamingTools.getRemapElementName("", "BlendRemap" + which, TARGET) + ".buf"))]).build()
              for which in ("VertexVG", "Forward", "Reverse")],
            FRB.SectionText(Z3, SK("BlendNoStride")).keys([(K.Type, K.Buffer), (K.Format, K.FormatR8UInt),
                                                           (K.Filename, FRB.IniNamingTools.getFixedBlendFile(FILE(K.WWMIBlendBuffer), TARGET))]).build(),
            FRB.SectionText(Z3, SK("BlendStrided")).keys([(K.Type, K.Buffer), (K.Format, K.FormatR8UInt), (K.Stride, "16"),
                                                          (K.Filename, FRB.IniNamingTools.getFixedBlendFile(FILE(K.WWMIBlendBuffer), TARGET))]).build(),
            FRB.SectionText(Z3, SK("RemappedBlendBufferRW")).build(), FRB.SectionText(Z3, SK("RemappedBlendBuffer")).build(),
            FRB.SectionText(Z3, RENAME(K.CommandList + "BlendRemap")).key(K.Local + " $blendRemapReady").open("!$blendRemapReady").keys(
                [(K.WWMICustomVertexCount, K.WWMIMeshVertexCount), (K.WWMIWeightsPerVertexCount, "8"), (K.WWMIBlendRemapId, "0"),
                 (K.WWMIBlendRemapperReverseReg, K.Ref + " " + SK("BlendRemapReverseBuffer")), (K.WWMIBlendRemapperVertexVGReg, K.Ref + " " + SK("BlendRemapVertexVGBuffer")),
                 (SK("RemappedBlendBufferRW"), K.Copy + " " + SK("BlendNoStride")), (K.WWMIBlendRemapperOutReg, K.Ref + " " + SK("RemappedBlendBufferRW")),
                 (K.Run, K.WWMIBlendRemapper), (SK("RemappedBlendBuffer"), K.Copy + " " + SK("RemappedBlendBufferRW")),
                 (SK("RemappedBlendBuffer"), K.CopyDesc + " " + SK("BlendStrided")), ("$blendRemapReady", "1")]).close()
               .key("vb4", SK("RemappedBlendBuffer")).build()]),

        # ---- each component's texture lists, guarded by the passes of the slot it goes to
        *[FRB.GraphCreate(LIST(c, kind), z3 = Z3, onlyWith = ("", SRC(c)), sectionsOf = lambda group, c = c, kind = kind: [
            FRB.SectionText(Z3, LIST_NAME(c, kind)).open((GUARDS if kind == "" else PASS0_GUARDS)[SLOT_OF[c]]).close().build()]
            if c not in REROUTES or DRAWN(c) else None)
          for c, kind, _ in LISTS],

        # ---- each draw through her own shader: a list that runs it on the slot's pass (and draws plainly elsewhere), saving and
        #      putting back what it sets; and the CustomShader itself
        *[FRB.GraphCreate(SHADER(c), z3 = Z3, onlyWith = ("", SRC(c)), sectionsOf = lambda group, c = c: [
            section for i, value in enumerate(DRAWS_OF(c)) for section in SHADER_DRAW(c, i, value)] or None)
          for c in SHADERS],

        # ---- a role the mod toggles: the override doing so, renamed to a list of the fix's, its `this` moved to the register
        FRB.GraphGroupEdit([{TOGGLE(c, reg, role)[1:]: [FRB.GraphRename(lambda name, new = RENAME(K.CommandList + SOURCE + CAPITAL(role) + FRB.IniNamingTools.getRegTag(reg)
                                                                                                 + ("Raw" if c in SHADERS else "")): new),
                                                        FRB.RegRemap({K.This: [reg]})]
                             for c, kind, reg, role in ALL_LINES}]),

        # ---- a toggle of ONE `this`: off, the original drew her game texture -- so that comes first, the toggle after it
        FRB.GraphGroupEdit([{TOGGLE(c, reg, role)[1:]: [FRB.RegIfTemplateAdd(FRB.SectionText(Z3, "").key(reg, GAME(role)).build(),
                                                                             FRB.RegIfTemplateAdd.Position.BeforeFirstIf, everySection = True,
                                                                             sectionFilter = lambda name, section, role = role: SINGLE(role) and TOGGLED(role))]
                             for c, kind, reg, role in ALL_LINES if role in DOWNLOADS}]),

        # ---- a role the mod has no texture for: its stand-in, Lynae's game texture downloaded
        *[FRB.GraphCreate(STAND_IN(c, role), z3 = Z3, onlyWithout = [ROLE(SRC(c), role)[1:], TOGGLE(c, reg, role)[1:]],
                          sectionsOf = lambda group, role = role: [FRB.SectionText(Z3, DOWNLOAD(role)).build()])
          for c, kind, reg, role in ALL_LINES if role in DOWNLOADS],

        # ---- each list's lines, in register order: the role's resource at the register, a run of its toggle, or its stand-in
        *[FRB.GraphInherit(LIST(c, kind), graph, line, partFilter = lambda d, modType, ini, name = LIST_NAME(c, kind):
                               FRB.Ranges.createFull() if d.sectionName == name else FRB.Ranges.createEmpty())
          for c, kind, reg, role in ALL_LINES for graph, line in [(ROLE(SRC(c), role), reg), (TOGGLE(c, reg, role), K.Run), (STAND_IN(c, role), reg)]],

        # ---- a list no line went into is no list; each one with lines runs from its component -- the texture list first thing,
        #      the other pass's after the blend remap
        FRB.GraphFilter([LIST(c, kind) for c, kind, _ in LISTS], lambda owner, part, i: True, removeEmptied = True),
        *[FRB.GraphGroupEdit([{("", c): [FRB.RegSurroundedAdd([(K.Run, LIST_NAME(c))], beforeRegs = {K.MatchFirstIndex: lambda value: True})]}],
                             onlyWith = LIST(c)[1:]) for c in TEXTURES if c in SLOTS],
        *[FRB.GraphGroupEdit([{("", c): [FRB.RegSurroundedAdd([(K.Run, LIST_NAME(c, "Pass0"))],
                                                              {K.Run: lambda value: value.strip().lower() == RENAME(K.CommandList + "BlendRemap").lower()}, {}, False)]}],
                             onlyWith = LIST(c, "Pass0")[1:]) for c in PASS0 if c in SLOTS],

        # ---- the cleaned texcoords, when the clean changed anything: bound after the blend remap, in every remapped section
        FRB.GraphCreate(CLEANED, z3 = Z3, sectionsOf = lambda group: [FRB.SectionText(Z3, "").build()]
                        if group == 0 and data().variables()["fixFiles"]["texcoordCleaned"] else None),
        FRB.GraphGroupEdit([{("", c): [FRB.RegSurroundedAdd([("vb2", TEXCOORD)], {K.Run: lambda value: value.strip().lower() == RENAME(K.CommandList + "BlendRemap").lower()},
                                                            {}, False)]
                             for c in SLOTS}], onlyWith = CLEANED[1:]),
        FRB.ResRegCollect({(0, "", c): "vb2" for c in SLOTS}, {"texcoord": TEXCOORD_FILE},
                          resPredicates = {(0, "", c): lambda reg, value, d: value.strip() == TEXCOORD for c in SLOTS}),

        # ---- the stand-ins have given the lists their lines: the download builds each one's section
        FRB.GraphRemove([STAND_IN(c, role) for c, _, _, role in ALL_LINES]),

        # ---- the game defaults the toggles bind
        *[FRB.ResRegCollect({toggle: reg for toggle, reg in dict.fromkeys((TOGGLE(c, r, ro), r) for c, _, r, ro in ALL_LINES if ro == role)},
                            {"gameDefault": FRB.FileDownloadCreate((0, "gameDefault", role), {GAME(role): FRB.FileDownloadCreate.File(
                                file = FRB.DownloadTools.fixedFileName(SOURCE, CAPITAL(role) + "Game", FRB.FileExt.DDS),
                                url = FRB.DownloadTools.url("WuWa", SOURCE, FOLDER(role), SOURCE, "Texture" + textureHash, FRB.FileExt.DDS))})},
                            resPredicates = {toggle: lambda key, value, d, role = role: value.strip() == GAME(role)
                                             for toggle in dict.fromkeys(TOGGLE(c, r, ro) for c, _, r, ro in ALL_LINES if ro == role)})
          for role, textureHash in DOWNLOADS.items() if any(ro == role for _, _, _, ro in ALL_LINES)],

        # ---- the downloads the lists and the shader draws bind
        *[FRB.ResRegCollect({**{LIST(c, kind): lambda key: True for c, kind, _ in LISTS}, **{SHADER(c): lambda key: True for c in SHADERS}},
                            {"download": FRB.FileDownloadCreate((0, "download", role), {DOWNLOAD(role): FRB.FileDownloadCreate.File(
                                file = FRB.DownloadTools.fixedFileName(SOURCE, CAPITAL(role), FRB.FileExt.DDS),
                                url = FRB.DownloadTools.url("WuWa", SOURCE, FOLDER(role), SOURCE, "Texture" + textureHash, FRB.FileExt.DDS))})},
                            resPredicates = {**{LIST(c, kind): lambda key, value, d, role = role: value.strip() == DOWNLOAD(role) for c, kind, _ in LISTS},
                                             **{SHADER(c): lambda key, value, d, role = role: value.strip() == DOWNLOAD(role) for c in SHADERS}})
          for role, textureHash in DOWNLOADS.items()],

        # ---- the texture edits, on the file each list binds for the role
        *[FRB.ResRegCollect({LIST(c2, kind): [reg for reg, r in lines.items() if r == role] for c2, kind, lines in LISTS if c2 == c and role in lines.values()},
                            {"texEdit": FRB.TexReplace((0, "", "texEdit" + name), FRB.TexEditor(filters = [texFilter(FROM, TO) if isinstance(texFilter, types.FunctionType)
                                                                                                         else texFilter]), resSubType = name)},
                            resPredicates = {LIST(c2, kind): FRB.ResRegCollect.skipsKeywords() for c2, kind, lines in LISTS if c2 == c and role in lines.values()})
          for (c, role), (name, texFilter) in TEX_EDITS.items()],

        # ---- her shaders' code and constants, beside the mod's mesh
        FRB.ResRegCollect({SHADER(c): "ps-cb4" for c in SHADERS}, {"shaderBuffer": SHADER_BUFFERS}, resPredicates = {SHADER(c): FRB.ResRegCollect.skipsKeywords() for c in SHADERS}),
        FRB.ResRegCollect({SHADER(c): "ps" for c in SHADERS}, {"shaderSource": SHADER_SOURCES}, resPredicates = {SHADER(c): FRB.ResRegCollect.skipsKeywords() for c in SHADERS}),

        # ---- the shape keys in LynaePeppermint's order: the mod's shape-key lists bind the fix's reordered buffers
        FRB.GraphGroupEdit([{("", "shapekeyOffsets"): [FRB.RegNewVals({reg: K.Resource + RENAME(name + "Buffer")
                                                                        for reg, name in (("cs-t33", "ShapeKeyOffset"), ("cs-t0", "ShapeKeyVertexId"), ("cs-t1", "ShapeKeyVertexOffset"))})]}]),
        FRB.ResRegCollect({(0, "", "shapekeyOffsets"): ["cs-t33", "cs-t0", "cs-t1"]}, {"shapeKeys": SHAPE_KEY_FILES}),

        # ---- each component onto its target slot (two onto one slot: another file, but SAME_FILE in the mod's own); the
        #      hash-only objects onto themselves (the shape keys in the mod's own file only); the fix's own graphs into every file
        FRB.GraphGroupRemap({**{(0, "", c): [(0,) + TARGET_OBJ[c] + (RENAME,)] + [(0,) + TARGET_OBJ[r] + (REROUTE_RENAME(r),) for r in REROUTES_OF(c)]
                                for c in SLOTS},
                             (0, "", "boneData"): [(0, "", "boneData", RENAME, True)],
                             (0, "", "shapekeyOffsets"): [(0, "", "shapekeyOffsets", RENAME, False)],
                             (0, "", "shapekeyScale"): [(0, "", "shapekeyScale", RENAME, False)],
                             (0, "", "watch"): [(0, TARGET, "watch", RENAME)],
                             **{graph: [graph + (lambda name: name, True)]
                                for graph in [SECTIONS, SKELETON, BLEND_REMAP, RESETS, *[LIST(c, kind) for c, kind, _ in LISTS],
                                              *[SHADER(c) for c in SHADERS], *[(0, "download", role) for role in DOWNLOADS],
                                              (0, "", "shaderBuffers"), (0, "", "shapeKeyFiles"), (0, "", "texcoordFile"), *[(0, "", "texEdit" + name) for name, _ in TEX_EDITS.values()],
                                              *TOGGLES, *dict.fromkeys(REF(role) for _, role in ROLES),
                                              *[(0, "gameDefault", role) for role in DOWNLOADS], *[PIN_GRAPH(c) for c in RABBITFX]]}}),

        # ---- each reroute's section: none when it takes no draw; else the reroute's lists and its slot's merge in place of the
        #      component's, and only the draws it takes -- which the component's own section no longer has
        *[FRB.GraphFilter([(0,) + TARGET_OBJ[r]], lambda owner, part, i, r = r: bool(SPLITS(r)), removeEmptied = True) for r in REROUTES],
        *[FRB.GraphGroupEdit([{TARGET_OBJ[r]: [FRB.RegAnchorAdd([K.Run], lambda key, value, r = r, kind = kind:
                                                                [(key, LIST_NAME(r, kind))] if value.strip() == LIST_NAME(SRC(r), kind) else None,
                                                                FRB.RegAnchorAdd.Placement.Replace)]}], onlyWith = LIST(r, kind)[1:])
          for r in REROUTES for kind in ("", "Pass0")],
        *[FRB.GraphGroupEdit([{TARGET_OBJ[r]: [FRB.RegAnchorAdd([K.Run], lambda key, value, r = r, kind = kind:
                                                                [] if value.strip() == LIST_NAME(SRC(r), kind) else None,
                                                                FRB.RegAnchorAdd.Placement.Replace)]}], onlyWithout = LIST(r, kind)[1:])
          for r in REROUTES for kind in ("", "Pass0")],
        FRB.GraphGroupEdit([{**{TARGET_OBJ[r]: [FRB.RegAnchorAdd([K.Run, K.DrawIndexed], lambda key, value, r = r:
                                                                 [(key, MERGE(SLOT_OF[r]))] if value.strip() == MERGE(SLOTS[SRC(r)])
                                                                 else [] if key.strip().lower() == K.DrawIndexed or value.strip() in DRAW_NAMES(SRC(r)) else None,
                                                                 FRB.RegAnchorAdd.Placement.Replace)] for r in REROUTES},
                             **{TARGET_OBJ[c]: [FRB.RegAnchorAdd([K.Run], lambda key, value, c = c:
                                                                 [] if any(value.strip() in DRAW_NAMES(r) for r in REROUTES_OF(c)) else None,
                                                                 FRB.RegAnchorAdd.Placement.Replace)] for c in SLOTS if REROUTES_OF(c)}}]),

        # ---- each remapped section drops the lines that would undo the remap: WWMI's blend remap and skeleton merge of the
        #      SOURCE, and RabbitFX's
        FRB.GraphGroupEdit([{TARGET_OBJ[c]: [FRB.RegRemove([(reg, FRB.RegRemove.valueStartsWith(prefix) if prefix else None) for reg, prefix in REMOVED]),
                                             FRB.RegPartEdit(FRB.RegRemove([*[(reg, None) for reg in RABBITFX_LINES],
                                                                            (K.Run, FRB.RegRemove.valueStartsWith(K.RabbitFXSetTexturesPath.lower()))]),
                                                             lambda section, c = c: c not in RABBITFX or not CALLS_RABBITFX(c))]
                             for c in SLOT_OF}], everyGroup = True),

        # ---- each target object's numbers (its slot's), in every file
        FRB.GraphGroupEdit([{TARGET_OBJ[c]: [FRB.RegNewVals({key: str(table.get([TARGET, "", SLOT_OF[c]], TO, False))
                                                             for key, table in [(K.MatchFirstIndex, T.indices), (K.MatchIndexCount, T.indexCounts),
                                                                                (K.WWMIVgOffset, T.vgOffsets), (K.WWMIVgCount, T.vgCounts)]})]
                             for c in SLOT_OF}], everyGroup = True),

        # ---- the hashes onto LynaePeppermint's, and the shape keys' dispatch size
        FRB.GraphGroupEdit([{**{TARGET_OBJ[c]: [FRB.RegAssetRemap({K.Hash: (S.hashes, K.HashNotFound)}, TARGET, SOURCE, FROM, TO)] for c in SLOT_OF},
                             **{("", obj): [FRB.RegAssetRemap({K.Hash: (S.hashes, K.HashNotFound),
                                                               **{K.WWMIShapeKeyChecksum + s: (S.shapeKeyChecksums, FRB.WWMINames.ChecksumNotFound) for s in ("", "_batch0", "_batch1")}},
                                                              TARGET, SOURCE, FROM, TO),
                                            FRB.RegNewVals({K.WWMIShapeKeyDispatchSizeYOriginalBatch + n: DISPATCH for n in "01"})]
                                for obj in ["boneData", "shapekeyOffsets", "shapekeyScale"]},
                             (TARGET, "watch"): [FRB.RegAssetRemap({K.Hash: (S.hashes, K.HashNotFound)}, TARGET, SOURCE, FROM, TO)]}], everyGroup = True),

        # ---- each target slot nothing of the mod lands on: skipped, its bones merged, what it stashes stashed
        *[FRB.GraphCreate((0, TARGET, "hide" + slot), z3 = Z3, onlyWith = [TARGET_OBJ[c] for c in SLOT_OF],
                          onlyWithout = [TARGET_OBJ[c] for c in SLOT_OF if SLOT_OF[c] == slot], sectionsOf = lambda group, slot = slot: [HIDE(slot)])
          for slot in TARGET_SLOTS],

        # ---- the frame's skeleton remap: latched on $object_detected where the mod's [Present] resets it, before each drawing
        #      section's `$object_detected = 1`
        FRB.GraphGroupEdit([{**{TARGET_OBJ[c]: [FRB.RegIfTemplateAdd(FRB.SectionText(Z3, "").open(K.WWMIObjectDetected + " == 0")
                                                                     .key(K.Run, RENAME(K.CommandList + "RemapMergedSkeleton")).close().build(),
                                                                     FRB.RegIfTemplateAdd.Position.BeforeTopKey, K.WWMIObjectDetected, value = "1")]
                                for c in SLOT_OF},
                             **{(TARGET, "hide" + slot): [FRB.RegIfTemplateAdd(FRB.SectionText(Z3, "").open(K.WWMIObjectDetected + " == 0")
                                                                               .key(K.Run, RENAME(K.CommandList + "RemapMergedSkeleton")).close().build(),
                                                                               FRB.RegIfTemplateAdd.Position.BeforeTopKey, K.WWMIObjectDetected, value = "1")]
                                for slot in TARGET_SLOTS}}],
                           everyGroup = True, onlyWith = RESETS[1:]),

        # ---- the blend, through Lynae's vertex group row onto LynaePeppermint's, in the remap's local ids, in every file
        FRB.ResRegCollect({(0,) + TARGET_OBJ[c]: "vb4" for c in SLOT_OF},
                          {"blend": FRB.VGRemapBlendReplace((0, "", "blend"), S, FROM, TO, row = VG_ROW(REFERENCE),
                                                            elements = lambda srcPath: FRB.BlendFile.byteElements(8), fixFunc = writeBlendRemap)},
                          resPredicates = {(0,) + TARGET_OBJ[c]: FRB.ResRegCollect.skipsKeywords() for c in SLOT_OF}, everyGroup = True),
        FRB.GraphGroupRemap({(0, "", "blend"): [(0, "", "blend", lambda name: name, True)]}),
        FRB.GraphGroupEdit([{TARGET_OBJ[c]: [FRB.RegPartEdit(FRB.RegNewVals({"vb4": FRB.IniNamingTools.getRemapBlendResourceName("", TARGET) + "Buffer"}),
                                                             lambda section: section.lower().startswith(SHARED.lower()))]   # a reroute's copy too
                             for c in SLOT_OF}], everyGroup = True),


        # ---- the parser's texture graphs are not written; and nothing of Lynae found means nothing is written
        FRB.GraphRemove([*[ROLE(c, role) for c, role in ROLES], RESETS, CLEANED,
                         *[(0, FRB.IniGraphModObjKeywords.Texture, role) for _, _, _, role in ALL_LINES], (0, FRB.IniGraphModObjKeywords.Texture, "")]),
        FRB.GraphGroupRemove(onlyWithout = [TARGET_OBJ[c] for c in SLOT_OF] + [(TARGET, "watch")]),
    ]

    # (the lists, and every table they were read at: what a user reads or changes)
    return types.SimpleNamespace(**locals())


LISTS = {}      # the lists per run's versions


def at(ini_):
    """The lists at an .ini's run's versions: the ones the run was asked for, else her and the skin's newest"""
    FROM, TO = ini_.fromVersion or NEWEST(S), ini_.toVersion or NEWEST(T)
    key = (FROM.toString(), TO.toString())
    if key not in LISTS:
        LISTS[key] = lists(FROM, TO)
    return LISTS[key]


makeParser = lambda ini_, modTypeId: at(ini_).parser(ini_)


def makeFixer(parser, toModName, modTypeId):
    fixer = FRB.GIMIFixer(parser, graphGroupEdits = at(parser._iniFile).EDITS, modsToFix = [toModName])
    fixer.copyHiddenSectionNames = {name for name in parser._iniFile.getSectionNames() if name.startswith(K.TextureOverride)}
    fixer.copyPreamble = FRB.IniComments.GIMIObjMergerPreamble
    return fixer




if __name__ == "__main__":
    args = argparse.ArgumentParser()
    args.add_argument("folder")
    args.add_argument("--fromVersion", default = None)
    args.add_argument("--version", default = None)
    args.add_argument("--download", default = "disabled", help = "the run's download mode: disabled (the default), normal or always")
    args = args.parse_args()
    FRB.CppStrategyOverrides.setParser(SOURCE, makeParser)
    FRB.CppStrategyOverrides.setFixer(SOURCE, TARGET, makeFixer)
    service = FRB.RemapService(path = os.path.abspath(args.folder), downloadMode = args.download,
                               fromVersion = V(args.fromVersion) if args.fromVersion else None, toVersion = V(args.version) if args.version else None)
    service.fix()
    for path, error in service.stats.ini.skipped.items():
        print("SKIPPED", path, error)

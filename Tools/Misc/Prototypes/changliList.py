# CHANGLI -> CHANGLINYMPH, AS A LIST OF EDITS (no WWMIParser / WWMIFixer)
#
# The parser sorts the mod's sections into graphs (a WWMISectionClassifier over Changli's tables); the fixer is the list
# EDITS: the library's generic edits, with this pair's choices written out as tables. Depends on nothing of another pair's.
#
# NO GAME DATA IS WRITTEN HERE: every register, draw pass, hash, index, game shader, shape-key order, skeleton numbering,
# vertex group map and material colour is read from the API's versioned asset tables (ModType's TextureRegisters,
# DrawPasses, Hashes, Indices, GameShaders, ShapeKeyRemaps, VGRemaps, BoneCentroids, ShapeKeyDispatchSizes,
# MaterialColours) at the RUN's versions -- the ones the run is asked for (--fromVersion / --version), else her and the
# skin's newest. What this file holds at its top is only the pair's version-free CHOICES. Game data with no table yet is
# marked "(no table yet)".
#
# Changli's parts: her bangs, hair, face and eyes onto the skin's own slots; her upper body, legs, coat and chest tattoo all
# through the skin's BODY slot (its skirt slot draws with a G-buffer shader in the overworld, which cut her parts away), in
# the body shader's layout. Her upper body's glow is drawn over that draw by HER upper-body shader (an overlay); her tattoo
# by her tattoo shader; her hair kept off the skin's outline pass. The skin's skeleton passes 256 bones and reads 8
# influences a vertex, so the fix merges and remaps the skeleton, writes WWMI's blend remap and widens her 4-wide blend.
#
#   py -3 changliList.py <mod folder> [--fromVersion X] [--version Y] [--download normal]
import argparse
import os
import struct
import sys
import types

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "Anime Game Remap (for all users)", "api"))
import src.py.FixRaidenBoss2 as FRB          # noqa: E402
from wwmiSectionClassifier import WWMISectionClassifier    # noqa: E402

K = FRB.IniKeywords
# a slot's name and number, as the library spells them (WWMINames.SlotPrefix, WWMIParserConfig.slotWithPrefix)
SLOT_PREFIX = FRB.WWMINames.SlotPrefix
SLOT_NUM = lambda name: FRB.WWMIParserConfig.slotWithPrefix(("", name), SLOT_PREFIX)
SOURCE, TARGET = "Changli", "ChangliNymph"
S, T = FRB.ModTypes[SOURCE].value, FRB.ModTypes[TARGET].value
Z3 = FRB.Z3Context()
V = FRB.Version.parse
NEWEST = lambda modType: modType.hashes.versionOf(modType.hashes.get([modType.name, K.Vb0], None, False), [modType.name, K.Vb0])
RENAME = lambda name: FRB.IniNamingTools.getRemapFixName(name, TARGET)
CAPITAL = lambda role: role[:1].upper() + role[1:]
MERGED = lambda: any(row.key.strip().lower() == K.WWMIVgOffset.lower() for row in data().collected(FRB.IniParseRows.Objs))


def need(value, what):
    """A table row the list cannot do without: missing, the list says which rather than writing a wrong section"""
    if value is None:
        raise LookupError("no asset table row for " + what)
    return value


# ================================================================================================= the pair's choices
# each source component's target slot; SAME_FILE: drawn as a further section on its slot, in the mod's own file
SLOTS = {"component0": "component0", "component1": "component1", "component2": "component2", "component3": "component4",
         "component4": "component4", "component5": "component4", "component6": "component6", "component7": "component4"}
SAME_FILE = ["component4", "component5", "component7"]

# each component's texture list: the draw whose REGISTERS it is bound in -- the target slot's, or (her tattoo, which her own
# shader draws) her own component's -- and each role under that draw's name for it. Her coat has no mask the body shader can
# read: the fix MAKES one (CREATED); the body's glow map is left to the game
MAIN = {"component0": (TARGET, "component0", {"bangsMap": "bangsMap", "bangsDiffuse": "bangsDiffuse", "bangsStrand": "bangsStrand"}),
        "component1": (TARGET, "component1", {"hairMap": "hairMap", "hairDiffuse": "hairDiffuse", "hairStrand": "hairStrand"}),
        "component2": (TARGET, "component2", {"faceMask": "faceMask", "faceDiffuse": "faceDiffuse"}),
        "component3": (TARGET, "component4", {"upperNormal": "bodyNormal", "upperMask": "bodyMask", "upperDiffuse": "bodyDiffuse"}),
        "component4": (TARGET, "component4", {"lowerNormal": "bodyNormal", "lowerMask": "bodyMask", "lowerDiffuse": "bodyDiffuse"}),
        "component5": (TARGET, "component4", {"coatNormal": "bodyNormal", "flatClothMask": "bodyMask", "coatDiffuse": "bodyDiffuse"}),
        "component6": (TARGET, "component6", {"eyeIris": "eyeIris", "eyeMask": "eyeMask", "eyeDiffuse": "eyeDiffuse"}),
        "component7": (SOURCE, "component7", {"glow": "glow", "tattooNoise": "tattooNoise", "tattooFlow": "tattooFlow"})}

# ...and its list for the slot's OTHER pass (an outline, the eye's highlight), run right before its draw: each role under
# the TARGET slot's name for it
OTHER = {"component0": {"bangsOutline": "bangsOutline"},
         "component3": {"upperDiffuse": "bodyDiffuse"}, "component4": {"lowerDiffuse": "bodyDiffuse"}, "component5": {"coatOutline": "bodyDiffuse"},
         "component6": {"eyeHighlight": "eyeHighlight"}}

# the textures the fix makes: role -> (width, height, the skin's slot, its MaterialColours type)
CREATED = {"flatClothMask": (16, 16, "component4", "bodyMask.cloth")}

# textures only her own game shaders read beside the mod's art (her overlay's and tattoo's): always her game's, never a file
# of the mod's
GAME_ONLY = ["upperShared", "upperSubsurface", "tattooNoise", "tattooFlow"]
# a role the mod has no texture for: Changli's own game texture, downloaded -- but these, which the skin draws with its own
GAME_LEFT = ["hairMap", "eyeIris", "eyeMask", "eyeDiffuse"]
DOWNLOAD = lambda role: FRB.IniNamingTools.getRemapDLResourceName(SOURCE + CAPITAL(role))
GAME = lambda role: FRB.IniNamingTools.getRemapDLResourceName(SOURCE + CAPITAL(role) + "Game")   # ...as a toggle's default

# her parts drawn with her OWN game pixel shaders, in place of the slot's draw: per component, the draw's name, her game
# shader, and whether it draws only where the draw has a render target (o3)
OWN_SHADERS = {"component7": ("ChangliTattoo", "ChangliTattoo", True)}
# ...and drawn OVER the slot's draw (an overlay): per component, the draw's name, her game shader, the roles it reads (the
# mod's, else her game textures downloaded) -- at the registers her own draw binds them -- its render state, and whether it
# gives way to a mod that calls RabbitFX on the part. It draws with her shader's own constants (GameShaders), as her tattoo
# does
OVERLAYS = {"component3": ("Overlay", "ChangliUpper", ["upperNormal", "upperMask", "upperDiffuse", "upperDetail", "upperRamp", "upperShared", "upperSubsurface"],
                           [(K.DepthStencilStateMerge, "true"), (K.DepthFunc, "equal"), (K.DepthWriteMask, "zero"),
                            *[(K.Mask + "[%d]" % n, "0") for n in range(1, 7)]], True)}
# the skin's own meshes beside its slots, each given a texture of hers on its pass: mesh (Hashes' type) -> role -> the file
# her download folder has for it (no table yet: a file made for the skin, not a game texture of hers)
SHARED_MESHES = {"blindfoldMesh": {"blindfold": "BlindfoldOnNymph"}}
# a part kept off a pass of the skin's (her hair off its outline pass): component -> the (slot, pass) whose shader it is
OFF_PASS = {"component1": ("component0", "other")}

# texture edits: (component, role) -> (name, a filter of (FROM, TO)) -- none (her bangs' outline gain changed nothing in game)
TEX_EDITS = {}

SRC = lambda c: c
SLOT_OF = dict(SLOTS)
TARGET_OBJ = {c: (TARGET, slot + "_" + str(SLOT_NUM(c))) if c in SAME_FILE else (TARGET, slot) for c, slot in SLOT_OF.items()}
TARGET_LABELS = {"component0": "bangs", "component1": "hair", "component2": "face", "component3": "sheer dress", "component4": "body",
                 "component5": "skirt and veils", "component6": "eyes", "component7": "shawl"}

BACKUP = lambda reg: K.Resource + RENAME("ShaderBackup" + FRB.IniNamingTools.getRegTag(reg))

# the buffers the mod's shared list rebinds, parked first and put back by the cleanup list; and the lines a remapped
# section drops (WWMI's blend remap of the SOURCE, its skeleton merge, RabbitFX's)
BYPASSABLE = [K.Ib] + ["vb" + str(n) for n in range(1, 10)]
BYPASSED = lambda: [reg for reg in BYPASSABLE if any(row.section.lower() == SHARED.lower() and row.key.strip().lower() == reg
                                                     for row in data().collected(FRB.IniParseRows.Objs))]
REMOVED = [(K.WWMIBlendBufferOverride, "ref"), ("ResourceMergedSkeletonOverride", "ref"), ("ResourceExtraMergedSkeletonOverride", "ref"),
           (K.Run, "commandlistmergeskeleton"), ("CheckTextureOverride", K.WWMIExtraBoneDataCb),
           ("CheckTextureOverride", K.WWMIBoneDataCb), (K.WWMIExtraBoneDataCb, "resourceextramergedskeleton"), (K.WWMIBoneDataCb, "resourcemergedskeleton"),
           (K.WWMIExtraBoneDataCb, "resourcemergedskeleton")]
RABBITFX_LINES = ["Resource\\RabbitFX\\" + kind for kind in ("Diffuse", "Lightmap", "Normalmap", "Materialmap", "Cutoutmap", "Specialmap")]
SHARED, CLEANUP = "CommandListOverrideSharedResources", "CommandListCleanupSharedResources"

FILTER = FRB.WWMINames.BoneDataFilter
SK = lambda name: K.Resource + RENAME(name)
CBS = [(K.WWMIBoneDataCb, "MergedSkeleton", "RemappedSkeleton"), (K.WWMIExtraBoneDataCb, "ExtraMergedSkeleton", "ExtraRemappedSkeleton")]
SHAPE_KEYS = [("ShapeKeyVertexId", "DXGI_FORMAT_R32_UINT", "4"), ("ShapeKeyVertexOffset", "DXGI_FORMAT_R16_FLOAT", "2"),
              ("ShapeKeyOffset", "DXGI_FORMAT_R32G32B32A32_UINT", "16")]

NOT_TOGGLES = [K.WWMIObjectDetected, K.WWMIObjectDetected + "==1"]
SECTION_GUARDS = NOT_TOGGLES + [K.WWMIModEnabled.lower(), "resourcemergedskeleton!==null"]

# ================================================================================================= the graphs, by id
TEXTURE = FRB.IniGraphModObjKeywords.Texture
ROLE = lambda c, role: (0, "role", c + ":" + role)
REGION_ROLES = ["faceMask", "lowerMask", "upperMask"]
GLOBAL = (0, "global", "")
data, ini = (lambda: FRB.EditScope.parseData()), (lambda: FRB.EditScope.ini())
OWN = lambda value: ini().resourceNamed(value)
FILE = lambda section: (lambda row: row.value.strip() if row is not None else None)(
    data().firstCollected(FRB.IniParseRows.Declared, section, K.Filename))
USABLE = lambda value, role: (lambda file: file is not None and os.path.isfile(ABS(file)) and not (
    role in REGION_ROLES and FRB.TexThumbprint.channelIsConstant(ABS(file), 0)))(FILE(OWN(value)))
THIS_ROWS = lambda role: [row for row in data().collected("this") if row.graphId[1:] == (TEXTURE, role) and row.key.strip().lower() == K.This]
SINGLE = lambda role: len(THIS_ROWS(role)) == 1
# of several overrides of a role's hashes, the one 3DMigoto applies LAST: the highest match_priority, then the last name
PRIORITY = lambda section: (lambda row: int(row.value) if row is not None and row.value.strip().lstrip("-").isdigit() else 0)(
    next((row for row in data().collected("this") if row.section == section and row.key.strip().lower() == K.MatchPriority), None))
APPLIED = lambda role: (lambda rows: max(rows, key = lambda row: (PRIORITY(row.section), row.section)).value.strip() if rows else None)(THIS_ROWS(role))
TOGGLED = lambda role: any(any(g.replace(" ", "").lower() not in NOT_TOGGLES for g in row.guards) for row in THIS_ROWS(role))
ABS = lambda rel: os.path.normpath(os.path.join(ini().folder, rel.replace("\\", os.sep)))
DRAWS = lambda c: any(tuple(row.graphId[1:]) == ("", c) and (row.key.strip().lower() == K.DrawIndexed or (
    row.key.strip().lower() == K.Run and ini().getSection(row.value.strip()) is None and "\\wwmiv1\\" not in row.value.lower()))
    for row in data().collected(FRB.IniParseRows.Objs))
SKIPS = lambda c: any(tuple(row.graphId[1:]) == ("", c) and row.key.strip().lower() == K.Handling and row.value.strip().lower() == K.Skip
                      for row in data().collected(FRB.IniParseRows.Objs))
KEPT = lambda c: DRAWS(c) or not SKIPS(c) or not any(DRAWS(other) for other in SLOTS)
CALLS_RABBITFX = lambda c: any(tuple(row.graphId[1:]) == ("", c) and row.key.strip().lower() == K.Run
                               and row.value.strip().lower() == K.RabbitFXSetTexturesPath.lower() for row in data().collected(FRB.IniParseRows.Objs))


# ================================================================================================= the lists, at a run's versions
def lists(FROM, TO):
    """The pair's parser and fixer at a run's versions -- FROM her mods', TO the skin's: every register, pass, hash, shader
    and table row read from the API's asset tables at them"""

    # ---- the registers of each texture list: MAIN's layouts, OTHER's slot's other pass (<role>@other)
    regOf = lambda who, comp, role: need((S.getTextureRegister(comp, role, FROM) if who == SOURCE else T.getTextureRegister(comp, role, TO)),
                                         who + " " + comp + " " + role)
    TEXTURES = {c: {regOf(who, comp, layoutRole): role for role, layoutRole in roles.items()} for c, (who, comp, roles) in MAIN.items()}
    PASS0 = {c: {regOf(TARGET, SLOT_OF[c], role + "@other"): srcRole for srcRole, role in roles.items()} for c, roles in OTHER.items()}
    # an overlay's textures: where her own draw of the part binds each role it reads (TextureRegisters)
    OVERLAY_REGS = {c: [regOf(SOURCE, c, role) for role in reads] for c, (_, _, reads, _, _) in OVERLAYS.items()}

    # ---- the skin's slots and each one's draw passes, by their vertex shaders' tags (DrawPasses)
    TARGET_SLOTS = [FRB.WWMINames.SlotPrefix + str(n) for n in range(T.getComponentCount(FRB.WWMINames.SlotPrefix, TO))]
    # the merged skeleton the fix supplies a legacy mod: three float4 a bone of the target's, in WWMI Tools' steps
    SKELETON_SLOTS = str(FRB.WWMIFixerConfig.mergedSkeletonSlotsFor(T.getMergedBoneCount(FRB.WWMINames.SlotPrefix, TO)))
    passNames = lambda slot, kind: list(T.getDrawPassNames(slot, kind, TO))
    shadersOf = lambda slot, kind: [vs for name in passNames(slot, kind) for vs in T.getDrawPassShaders(slot, name, TO)]
    tagsOf = lambda slot, kind: [tag for name in passNames(slot, kind) for tag in T.getDrawPassTags(slot, name, TO)]
    PASSES = {vs: tag for kind in ("main", "other") for slot in TARGET_SLOTS for vs, tag in zip(shadersOf(slot, kind), tagsOf(slot, kind))}
    GUARDS = {slot: " || ".join("vs == " + tag for tag in dict.fromkeys(tagsOf(slot, "main"))) for slot in TARGET_SLOTS}
    PASS0_GUARDS = {slot: " || ".join("vs == " + tag for tag in dict.fromkeys(tagsOf(slot, "other"))) for slot in TARGET_SLOTS if tagsOf(slot, "other")}
    OFF_PASSES = {c: " && ".join("vs != " + tag for tag in dict.fromkeys(tagsOf(slot, kind))) for c, (slot, kind) in OFF_PASS.items()}

    # ---- her game textures (Hashes), each downloaded from the folder of its row's version
    roles = dict.fromkeys([role for _, _, rs in MAIN.values() for role in rs] + [role for rs in OTHER.values() for role in rs]
                          + [role for *_, reads, _, _ in OVERLAYS.values() for role in reads])
    DOWNLOADS = {role: textureHash for role in sorted(roles) if role not in GAME_LEFT and role not in CREATED
                 for textureHash in [S.hashes.get([SOURCE, role], None, False)] if textureHash is not None}
    DOWNLOADS.update({role: file for roles_ in SHARED_MESHES.values() for role, file in roles_.items()})
    FOLDER_OF = {role: S.hashes.versionOf(textureHash, [SOURCE, role]) for role, textureHash in DOWNLOADS.items() if S.hashes.versionOf(textureHash, [SOURCE, role])}
    # her downloads' folder: the one of her newest texture rows (her textures did not change between the versions Hashes files
    # them at, and only the newest has a download folder)
    FOLDER = lambda role: FRB.DownloadTools.versionFolder(max(FOLDER_OF.values(), key = lambda v: v))
    CREATORS = {role: FRB.CppTexCreator(width, height, need(T.getMaterialColour(slot, colour, TO), TARGET + " " + slot + " " + colour))
                for role, (width, height, slot, colour) in CREATED.items()}

    # ---- her own shaders (GameShaders): each one's code and constant buffers
    # a game shader's constant buffers: each of its GameShaders rows "<shader>.<register>"
    cbRegs = lambda game: sorted({rowType[len(game) + 1:] for names in S.gameShaders.repo.toNestedDict().values()
                                  for rowType in names.get(SOURCE, {}).get("", {}) if rowType.startswith(game + ".")})
    constantBuffers = lambda game: {reg: list(need(S.getGameShaderBuffer(game, reg, FROM), game + "." + reg))
                                    for reg in cbRegs(game)}
    GAMES = {game for _, game, _ in OWN_SHADERS.values()} | {game for _, game, *_ in OVERLAYS.values()}
    SHADER_CODE = {game: need(S.getGameShader(game, FROM), game) for game in GAMES}
    SHADER_CBS = {game: constantBuffers(game) for game in GAMES}

    # ---- where her own draws bind each role, as her mods write it (her register layout, and RabbitFX's keys)
    SOURCE_REGISTERS = {c: {reg: role for role in dict.fromkeys(role for d, (_, _, rs) in MAIN.items() if SRC(d) == c for role in rs)
                            for reg in (S.getTextureRegister(c, role + "@rabbitfx", FROM), S.getTextureRegister(c, role, FROM)) if reg is not None}
                        for c in SLOTS}

    # ---- her skeleton's numberings (VGRemaps), one per model of hers, each onto the newest; and her vertex group maps
    VB0 = {value.lower(): version for version, names in S.hashes.repo.toNestedDict().items() for kind, value in names.get(SOURCE, {}).items()
           if kind == K.Vb0}
    VERSIONS = sorted(set(VB0.values()), key = V)
    REFERENCE = VERSIONS[-1]
    VG_MAPS = lambda version: {SLOT_NUM(c): [row.remap[i] for i in range(len(row.remap))]
                               for c in SLOTS for row in [need(S.getVGRemap(SOURCE, V(version), V(version), c, ""), SOURCE + " " + c + " vg_map at " + version)]}
    NUMBERINGS = [FRB.SkeletonNumberingProbe.Numbering(version, {k: v for k, v in need(S.getVGRemap(SOURCE, V(version), V(REFERENCE)),
                                                                                         SOURCE + " " + version + " numbering").remap.items() if k != v})
                  for version in VERSIONS]
    CENTROIDS = S.getBoneCentroids(V(REFERENCE))
    SHAPE_KEY_ORDER = list(need(S.getShapeKeyRemap(TARGET, FROM, TO), "the shape-key order"))
    DISPATCH = need(T.getShapeKeyDispatchSize(TO), "the shape-key dispatch size")

    NUMBERING = lambda: data().variables()["boneNumbering"]
    VG_ROW = lambda version: S.getVGRemap(TARGET, V(version), TO)
    DRAWS_OF = lambda c: draws(c)
    DRAWN = lambda c: KEPT(SRC(c)) and bool(DRAWS_OF(c))
    BONES = lambda: str(len(set(VG_ROW(NUMBERING()).remap.values())))

    RAW = lambda c, kind: c in OWN_SHADERS or kind == "Overlay"           # a list a shader of hers reads: the mod's art unedited
    TOGGLE = lambda c, reg, role, kind = "": (0, "toggle", role + ":" + reg + (":raw" if RAW(c, kind) else ""))
    ROLES = list(dict.fromkeys((SRC(c), role) for c, lines in list(TEXTURES.items()) + list(PASS0.items()) for role in lines.values()
                               if role not in CREATED))
    ROLES += [(c, role) for c, (*_, reads, _, _) in OVERLAYS.items() for role in reads if (c, role) not in ROLES]
    ROLES = [(c, role) for c, role in ROLES if role not in GAME_ONLY]      # (the mod's files are read for these alone)
    LISTS = ([(c, "", lines) for c, lines in TEXTURES.items()] + [(c, "Pass0", lines) for c, lines in PASS0.items()]
             + [(c, "Overlay", dict(zip(OVERLAY_REGS[c], OVERLAYS[c][2]))) for c in OVERLAYS])
    ALL_LINES = [(c, kind, reg, role) for c, kind, lines in LISTS for reg, role in lines.items()]
    TOGGLES = list(dict.fromkeys(TOGGLE(c, reg, role, kind) for c, kind, reg, role in ALL_LINES if role not in CREATED))

    # ---- a file no hash names, identified by thumbprint among her game textures (TextureThumbprints), by the API's default
    #      thresholds (WWMITextureFacts), and the role of the game texture it is (Hashes: every generation of her roles)
    THUMBS = S.getTextureThumbprints(FROM)
    THUMB_SIZE = int(round(len(next(iter(THUMBS.values()))) ** 0.5)) if THUMBS else 0
    DEFAULTS = FRB.WWMITextureFacts()
    ROLE_NAMES = {role for _, role in ROLES}
    ROLE_OF_HASH = {textureHash.lower(): role for names in S.hashes.repo.toNestedDict().values() for hashType, textureHash in names.get(SOURCE, {}).items()
                    for role in [FRB.WWMITextureFacts.roleOfHashType(hashType, ROLE_NAMES)] if role is not None}
    identified = {}
    UNNAMED = (0, "unnamedTexture", "")             # the overrides of hashes no table row names, as a graph of their own
    def IDENTIFIED(value):
        file = FILE(OWN(value))
        if not file or not THUMBS or not os.path.isfile(ABS(file)):
            return None
        if ABS(file) not in identified:
            found = FRB.TexThumbprint.identifyFile(ABS(file), THUMBS, THUMB_SIZE, DEFAULTS.identityMin, DEFAULTS.identityGap)
            identified[ABS(file)] = ROLE_OF_HASH.get((found or "").lower())
        return identified[ABS(file)]

    # ================================================================================================= the parser
    CLASSIFIER = WWMISectionClassifier(S, FROM, list(SLOTS), [role for _, role in ROLES],
                                       {"cb4": "boneData", "shapekey_offsets": "shapekeyOffsets", "shapekey_scale": "shapekeyScale",
                                        "blindfoldMesh": "blindfoldMesh"}, [SOURCE, TARGET])
    REF = lambda role: (0, "ref", role)                                     # a resource the fix declares for a linked .ini's file
    REF_NAME = lambda role: K.Resource + FRB.IniNamingTools.getModSuffixedName(CAPITAL(role), K.RemapRef, TARGET)
    PARSE = [
        FRB.GraphCreate(GLOBAL, z3 = Z3, sectionsOf = lambda group: list(ini().getIfTemplates().values()) if group == 0 else []),
        FRB.GraphCollect({GLOBAL: [K.Filename, K.Type, K.Format, K.Stride]}, name = FRB.IniParseRows.Declared),
        FRB.GraphCollect({GLOBAL: lambda key: True}, name = FRB.IniParseRows.Sections),
        FRB.GraphRemove([GLOBAL]),
        FRB.GraphCollect({(0, "", c): lambda key: True for c in SLOTS}, name = FRB.IniParseRows.Objs, withConditions = True),
        FRB.GraphFilter([(0, "", "watch")], lambda owner, part, i: not data().collected(FRB.IniParseRows.Objs), removeEmptied = True),
        FRB.GraphCollect({(0, TEXTURE, role): [K.Hash, K.This, K.MatchPriority] for _, role in ROLES}, name = "this", withConditions = True),
        FRB.GraphVarApply([], lambda: boneNumbering(), "boneNumbering"),
        *[FRB.GraphFilter([(0, "", c)], lambda owner, part, i, c = c: KEPT(c), removeEmptied = True) for c in SLOTS],
        *[FRB.ResRegCollect({(0, TEXTURE, role): K.This}, {"": FRB.ResIdentity(ROLE(c, role), createResModel = False, fileOf = OWN)},
                            resPredicates = {(0, TEXTURE, role): lambda reg, value, d, role = role: OWN(value) != "" and USABLE(value, role) and not TOGGLED(role)
                                             and value.strip() == APPLIED(role)})
          for c, role in ROLES],
        *[FRB.ResRegCollect({(0, "", c): reg}, {"": FRB.ResIdentity(ROLE(c, role), createResModel = False, fileOf = OWN)},
                            resPredicates = {(0, "", c): lambda r, value, d, c = c, reg = reg, role = role: OWN(value) != "" and USABLE(value, role)})
          for c, regs in SOURCE_REGISTERS.items() for reg, role in regs.items() if (c, role) in ROLES],
        # ...or, a texture override of a hash no table row names (an older generation, a mod's own): the role of the game texture
        # of hers its file IS, by thumbprint (TextureThumbprints) -- for a role nothing above found
        FRB.GraphFilter([ROLE(c, role) for c, role in ROLES], lambda owner, part, i: True, removeEmptied = True),   # (a role found empty is none)
        FRB.GraphCopy((0, TEXTURE, ""), UNNAMED, lambda owner, part, i: True),
        *[FRB.ResRegCollect({UNNAMED: K.This}, {"": FRB.ResIdentity(ROLE(c, role), createResModel = False, fileOf = OWN)},
                            resPredicates = {UNNAMED: lambda reg, value, d, role = role: OWN(value) != "" and USABLE(value, role)
                                             and not TOGGLED(role) and IDENTIFIED(value) == role})
          for c, role in ROLES],
        FRB.GraphRemove([UNNAMED]),
        *[FRB.GraphCopy((0, TEXTURE, role), toggle, lambda owner, part, i, role = role: TOGGLED(role) and part.getByInd(i)[0].lower() not in K.MatchKeys,
                        keepEmpty = False)
          for toggle, role in dict.fromkeys((TOGGLE(c, reg, role, kind), role) for c, kind, reg, role in ALL_LINES if role not in CREATED)],
        FRB.GraphFilter([ROLE(c, role) for c, role in ROLES], lambda owner, part, i: True, removeEmptied = True),
        *[FRB.GraphFilter([toggle], lambda owner, part, i, toggle = toggle: any(TOGGLE(c, reg, role, kind) == toggle and KEPT(SRC(c))
                                                                                for c, kind, reg, role in ALL_LINES), removeEmptied = True)
          for toggle in TOGGLES],

        # a role no line of this file binds, but a LINKED .ini binds on the component's draw: its file (the one showing at the
        # linked file's starting values), declared by the fix
        FRB.GraphVarApply([], lambda: linked(), "linked"),
        *[FRB.GraphCreate(graph, z3 = Z3, onlyWithout = [ROLE(c, role)[1:]], sectionsOf = lambda group, c = c, role = role: (
            lambda rel: [FRB.SectionText(Z3, REF_NAME(role)).key(K.Filename, rel).build()] if rel else None)(data().variables()["linked"]["files"].get((c, role))))
          for c, role in ROLES for graph in (REF(role), ROLE(c, role))],   # the declaration first: it is made while the role has none
    ]
    GLOW_KEY = FRB.RabbitFX.GlowMapKey                                # what a linked .ini sets that is carried as it is

    LINKED_PARSE = [   # what is read of a linked .ini: its declarations, its sections, and every key of each component
        FRB.GraphCreate(GLOBAL, z3 = Z3, sectionsOf = lambda group: list(ini().getIfTemplates().values()) if group == 0 else []),
        FRB.GraphCollect({GLOBAL: [K.Filename]}, name = FRB.IniParseRows.Declared),
        FRB.GraphCollect({GLOBAL: lambda key: True}, name = FRB.IniParseRows.Sections),
        FRB.GraphRemove([GLOBAL]),
        FRB.GraphCollect({(0, "", c): lambda key: True for c in SLOTS}, name = FRB.IniParseRows.Objs, withConditions = True)]

    def linked():
        """What the LINKED .ini files set on the draws of the components the mod keeps: 'files', per (component, role), the file
        each binds for a role at the register her draw binds it (relative to this .ini); 'lines', per component, each line the
        target never reaches -- (key, role, condition, file) -- a glow map, or a role's register under a condition; 'inis', each
        linked file whose own variables those conditions read, with the variables"""
        files, lines, inis, here = {}, {}, [], ini()
        for linkedIni in here.linkedInis():
            sibling = here.siblingOf(linkedIni, {getattr(FRB.ModTypeId, SOURCE)})
            reader = parser(sibling, LINKED_PARSE)
            reader.parse()
            other = reader.parseData
            start = other.startingValues(FRB.IniParseRows.Sections, Z3)
            fileOf = lambda value: (lambda row: os.path.normpath(os.path.join(sibling.folder, row.value.strip().replace("\\", os.sep))) if row is not None else None)(
                other.firstCollected(FRB.IniParseRows.Declared, sibling.resourceNamed(value), K.Filename))
            used = []
            for row in other.collected(FRB.IniParseRows.Objs):
                c = row.graphId[2]
                if not KEPT(SRC(c)):
                    continue
                # the condition, unless it holds everywhere
                always = row.condition is None or not (~row.condition).isSatisfiable()
                condition = "" if always else FRB.IfPredPart.getIfPredStr(row.condition)
                if condition is None:
                    continue
                file = fileOf(row.value)
                if file is None or not os.path.isfile(file):
                    continue
                if FRB.IniNamingTools.sameKey(row.key, GLOW_KEY):
                    lines.setdefault(c, []).append((row.key.strip(), "", condition, file))
                    used.append(condition)
                    continue
                role = next((r for reg, r in SOURCE_REGISTERS.get(c, {}).items() if FRB.IniNamingTools.sameKey(reg, row.key)), None)
                if role is None or (c, role) not in ROLES:
                    continue
                if condition:
                    lines.setdefault(c, []).append(("", role, condition, file))
                    used.append(condition)
                if always or FRB.GraphCollect.anyUnder([row], start, Z3):        # (the conditions combined in the list's context)
                    files.setdefault((c, role), os.path.relpath(file, here.folder))
            names = {var for condition in used for var in FRB.IniNamingTools.varsIn(condition)}
            if names:
                inis.append((linkedIni, names))
        return {"files": files, "lines": lines, "inis": inis}

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

    parser = lambda ini_, edits = None: FRB.GIMIParser(ini_, modObjs = CLASSIFIER.objects, objTargetFuncs = [CLASSIFIER], disjointModObjs = False,
                                                       modTypeId = getattr(FRB.ModTypeId, SOURCE), keysToTrack = {K.Hash, K.MatchFirstIndex, K.MatchIndexCount},
                                                       commandEdits = PARSE if edits is None else edits)

    # ================================================================================================= the fix's graphs, by id
    LIST = lambda c, kind = "": (0, "list" + kind, c)
    LIST_NAME = lambda c, kind = "": RENAME("CommandListChangli" + CAPITAL(c) + "Textures" + kind)
    STAND_IN = lambda c, role: (0, "standIn", c + ":" + role)
    SECTIONS, SKELETON, BLEND_REMAP = (0, "", "fixSections"), (0, "", "skeleton"), (0, "", "blendRemap")
    SHADER = lambda c: (0, "shader", c)
    OVERLAY = lambda c: (0, "overlay", c)
    SHARED_MESH = lambda mesh: (0, "sharedMesh", mesh)        # a mesh of the skin's own besides its slots
    LINKED_STATE, CARRIED = (0, "", "linkedState"), (0, "", "carried")     # a linked file's state; the files its lines bind
    GLOW = lambda c: (0, "glow", c)                                        # what linked files set on a component's draws
    GLOW_NAME = lambda c: RENAME(K.CommandList + SOURCE + CAPITAL(c) + FRB.WWMINames.Glow)

    def carried():
        """The linked lines as the fix writes them: 'lines' per component (key or register, its value, condition) on the fix's
        copies of the linked files' variables; 'declared', the resources the fix declares for their files; 'state', the copies"""
        found = data().variables()["linked"]
        here, refs = ini(), {}
        for (c, role), rel in found["files"].items():
            refs.setdefault(os.path.normpath(os.path.join(here.folder, rel)), REF_NAME(role))
        own = {}
        for row in data().collected(FRB.IniParseRows.Declared):
            if row.key.strip().lower() == K.Filename and not FRB.IniNamingTools.looksRemapped(row.section, [SOURCE, TARGET]):
                own.setdefault(ABS(row.value.strip()), row.section)

        # each linked file's state, its variables renamed: <var><target>RemapRef (then 2, 3, ...)
        state, renamesOf = [], {}
        for n, (linkedIni, names) in enumerate(found["inis"]):
            copy = linkedIni.varState(names)
            suffix = TARGET + K.RemapRef + (str(n + 1) if n else "")
            renames = {var: var + suffix for var in copy.vars}
            renamesOf[linkedIni.file] = renames
            for section in copy.takeSections():
                FRB.GraphVarRename.renameIn(section, renames, Z3)
                if section.name.lower() not in (K.Constants.lower(), K.Present.lower()):
                    section.name = RENAME(section.name)
                state.append(section)
            if copy.undeclared:
                state.append(FRB.SectionText(Z3, K.Constants).keys([(K.Global + " $" + renames[var], "0") for var in sorted(copy.undeclared)]).build())
        renames = {var: new for r in renamesOf.values() for var, new in r.items()}

        declared, used, lines = [], set(refs.values()), {}
        for c, carriedLines in found["lines"].items():
            for key, role, condition, file in carriedLines:
                resource = own.get(file) or refs.get(file)
                if resource is None:
                    base = K.Resource + ("Carried" if not key else FRB.WWMINames.Glow) + "".join(
                        ch for ch in CAPITAL(os.path.splitext(os.path.basename(file))[0]) if ch.isalnum())
                    resource, n = base + TARGET + K.RemapRef, 2
                    while resource in used:
                        resource, n = base + str(n) + TARGET + K.RemapRef, n + 1
                    used.add(resource)
                    refs[file] = resource
                    declared.append((resource, os.path.relpath(file, here.folder)))
                lines.setdefault(c, []).append((key, role, FRB.IniNamingTools.renameVars(condition, renames) if condition else "", K.Ref + " " + resource))
        return {"lines": lines, "declared": declared, "state": state}

    def glowList(c, lines):
        """A component's list of the linked lines: each carried key as it is, then each role's register -- where this component's
        list binds the role -- on the slot's main pass"""
        text = FRB.SectionText(Z3, GLOW_NAME(c))
        for key, role, condition, value in lines:
            if key:
                (text.open(condition).key(key, value).close() if condition else text.key(key, value))
        regs = [(reg, condition, value) for key, role, condition, value in lines if not key
                for reg in [next((r for r, ro in TEXTURES.get(c, {}).items() if ro == role), None)] if reg is not None]
        if regs:
            text.open(GUARDS[SLOT_OF[c]])
            for reg, condition, value in regs:
                (text.open(condition).key(reg, value).close() if condition else text.key(reg, value))
            text.close()
        return text.build()
    RESETS = (0, "", "resetsDetected")
    NO_MESH = (0, "", "noMesh")
    HAS_MESH = lambda: bool(FILE(K.WWMIBlendBuffer) and FILE(K.WWMIPositionBuffer))
    SOURCES = [("", c) for c in SLOTS]
    ROOTS = lambda c: {row.rootSection for row in data().collected(FRB.IniParseRows.Objs) if tuple(row.graphId[1:]) == ("", c)}
    draws = lambda c: sorted(dict.fromkeys(row.value.strip() for row in data().collected(FRB.IniParseRows.Objs)
                                           if tuple(row.graphId[1:]) == ("", c) and row.key.strip().lower() == K.DrawIndexed
                                           and row.section == row.rootSection),
                             key = lambda value: FRB.IniParseData.drawRangeOf(value))
    DRAW = lambda c, i: RENAME(K.CommandList + OWN_SHADERS[c][0] + str(SLOT_NUM(c)) + "Draw" + str(i))
    OVERLAY_DRAW = lambda c, i: RENAME(K.CommandList + OVERLAYS[c][0] + str(SLOT_NUM(c)) + "Draw" + str(i))
    OFF_DRAW = lambda c, value: RENAME(K.CommandList + "OffPass" + str(SLOT_NUM(c)) + "Draw" + "_".join(map(str, FRB.IniParseData.drawRangeOf(value))))
    MESHES = lambda: os.path.dirname((FILE(K.WWMIBlendBuffer) or "").replace("\\", "/"))
    BESIDE_MESH = lambda name: "/".join(part for part in (MESHES(), name) if part)
    CB = lambda game, reg: K.Resource + RENAME(game + FRB.IniNamingTools.getRegTag(reg))
    # an overlay gives way to a mod that calls RabbitFX on the part (any of its lists: SetTextures, Run, ...)
    USES_RABBITFX = lambda c: any(tuple(row.graphId[1:]) == ("", c) and row.key.strip().lower() == K.Run and FRB.RabbitFX.names(row.value)
                                  for row in data().collected(FRB.IniParseRows.Objs))
    OVERLAID = lambda c: DRAWN(c) and not (OVERLAYS[c][4] and USES_RABBITFX(c))

    SHAPE_KEY_FILES = FRB.FileAddCreate((0, "", "shapeKeyFiles"), {})
    OWN_MESH_FILES = FRB.FileAddCreate((0, "", "ownMeshFiles"), {})     # a part's own-mesh blend, remapped
    OWN_MESHES = {}                                                     # component -> its own mesh (fixFiles reads them)
    OWN_BLEND = lambda c: K.Resource + RENAME("OwnMeshSkin" + str(SLOT_NUM(c)))
    SHADER_BUFFERS = FRB.FileAddCreate((0, "", "shaderBuffers"), {})
    SHADER_SOURCES = FRB.FileRefAdd({})
    TEXCOORD_FILE = FRB.FileAddCreate((0, "", "texcoordFile"), {})
    TEXCOORD = K.Resource + RENAME("TexcoordNoNaN")
    CLEANED = (0, "", "texcoordCleaned")
    BLEND_INPUTS = {}

    # the game shaders the mod's parts are drawn with, this run: (game, component) for every own shader or overlay drawn
    DRAWN_GAMES = lambda: list(dict.fromkeys([game for c, (_, game, _) in OWN_SHADERS.items() if DRAWN(c)]
                                             + [game for c, (_, game, *_) in OVERLAYS.items() if OVERLAID(c)]))
    # an overlay's texture registers: where her own draw of the part binds each role it reads (TextureRegisters)
    REG_ORDER = lambda reg: (reg.rstrip("0123456789"), int(reg[len(reg.rstrip("0123456789")):] or 0))
    SHADER_REGS = lambda: list(dict.fromkeys([reg for game in DRAWN_GAMES() for reg in SHADER_CBS[game]] + sorted(
        {reg for c in OVERLAYS if OVERLAID(c) for reg in OVERLAY_REGS[c]}, key = REG_ORDER)))

    def fixFiles():
        """The fix's files for the .ini the list runs on: the shape keys in the target's order, her shaders' code and
        constants, beside the mod's mesh; what the blend remap reads besides the blend; the cleaned texcoords"""
        folder, abs_ = ini().folder, lambda rel: os.path.normpath(os.path.join(ini().folder, rel.replace("\\", os.sep)))
        if not HAS_MESH():
            return {"folder": folder, "texcoordCleaned": False}       # (the files the fix makes are another .ini's)
        keys = {name: FILE(K.Resource + name + "Buffer") for name, _, _ in SHAPE_KEYS}
        if all(keys.values()):
            mod = FRB.WWMIShapeKeys.Files(abs_(keys["ShapeKeyOffset"]), abs_(keys["ShapeKeyVertexId"]), abs_(keys["ShapeKeyVertexOffset"]))
            reordered = lambda which: getattr(FRB.WWMIShapeKeys.reordered(mod, SHAPE_KEY_ORDER), which)
            SHAPE_KEY_FILES.files = {K.Resource + RENAME(name + "Buffer"): FRB.FileAddCreate.File(
                keys = [(K.Type, K.Buffer), (K.Format, fmt), (K.Stride, stride)],
                file = BESIDE_MESH(FRB.IniNamingTools.getRemapElementName("", name, TARGET) + ".buf"),
                makeBytes = lambda which = {"ShapeKeyOffset": "offsets", "ShapeKeyVertexId": "vertexIds", "ShapeKeyVertexOffset": "vertexOffsets"}[name]: reordered(which))
                for name, fmt, stride in SHAPE_KEYS}
        SHADER_BUFFERS.files = {CB(game, reg): FRB.FileAddCreate.File(
            keys = [(K.Type, K.Buffer), (K.Format, K.FormatR32G32B32A32FloatDxgi)],
            file = BESIDE_MESH(FRB.IniNamingTools.getRemapElementName("", game + FRB.IniNamingTools.getRegTag(reg), TARGET) + ".buf"),
            bytes = struct.pack("<%dI" % len(words), *words))
            for game in DRAWN_GAMES() for reg, words in SHADER_CBS[game].items()}
        SHADER_SOURCES.files = {BESIDE_MESH(FRB.IniNamingTools.getRemapElementName("", game, TARGET) + ".hlsl"): SHADER_CODE[game].encode("utf-8")
                                for game in DRAWN_GAMES()}
        # ---- the parts drawn from mesh files of their own: a run of the shared list's per-part copy (".<part>") binding its
        #      own index, position and blend buffers
        OWN_MESHES.clear()
        objs = data().collected(FRB.IniParseRows.Objs)
        sections = data().collected(FRB.IniParseRows.Sections)
        boundIn = lambda section, reg: next((FRB.IniNamingTools.removeRefPrefix(row.value).strip() for row in sections
                                             if row.section == section and FRB.IniNamingTools.sameKey(row.key, reg)), "")
        for c in SLOTS:
            for row in objs:
                if tuple(row.graphId[1:]) != ("", c) or row.key.strip().lower() != K.Run or not row.value.strip().lower().startswith(SHARED.lower() + "."):
                    continue
                ownList = row.value.strip()
                ib, blendRes, positionRes = boundIn(ownList, K.Ib), boundIn(ownList, K.WWMIBlendReg), boundIn(ownList, K.Vb0)
                if not ib or ib.lower() == K.WWMIIndexBuffer.lower() or not blendRes or not positionRes or not FILE(blendRes) or not FILE(positionRes):
                    continue
                offset = next((r.value.strip() for r in objs if tuple(r.graphId[1:]) == ("", c) and r.key.strip().lower() == K.WWMIVgOffset.lower()), "0")
                OWN_MESHES[c] = (ownList, blendRes, FILE(blendRes), FILE(positionRes), int(offset) if offset.isdigit() else 0)
        ownFiles = {}
        for c, (_, _, blendFile, positionFile, offset) in OWN_MESHES.items():
            layout = FRB.WWMIBlend.layoutOf(abs_(blendFile), abs_(positionFile))
            if layout is None:
                continue
            out = max(layout.influences, 8)
            folder = os.path.dirname(blendFile.replace("\\", "/"))
            ownFiles[OWN_BLEND(c)] = FRB.FileAddCreate.File(
                keys = [(K.Type, K.Buffer), (K.Format, K.FormatR8UInt), (K.Stride, str(out * 2))],
                file = "/".join(part for part in (folder, TARGET + K.Remap + "OwnMeshBlend" + str(SLOT_NUM(c)) + ".buf") if part),
                makeBytes = lambda b = abs_(blendFile), p = abs_(positionFile), o = offset, m = VG_MAPS(VERSIONS[0])[SLOT_NUM(c)],
                                   row = VG_ROW(NUMBERING()), n = out: FRB.WWMIBlend.componentBlend(b, p, o, m, row, n)[0])
        OWN_MESH_FILES.files = ownFiles

        blend, position = FILE(K.WWMIBlendBuffer), FILE(K.WWMIPositionBuffer)
        vertexVG = FILE(K.Resource + "BlendRemapVertexVGBuffer")
        if blend and position:
            index = FILE(K.WWMIIndexBuffer)
            ranges = {SLOT_NUM(c): [FRB.IniParseData.drawRangeOf(v) for v in draws(c)] for c in SLOTS}
            legacy = None if MERGED() or not index else FRB.WWMIBlend.LegacyLift(abs_(index), ranges, VG_MAPS(VERSIONS[0]))
            influences = FRB.WWMIBlend.influencesOf(abs_(blend), 0, abs_(position))     # the mod's blend width (its Position.buf's vertices)
            vertexVGOut = abs_(BESIDE_MESH(FRB.IniNamingTools.getRemapElementName("", "BlendRemapVertexVG", TARGET) + ".buf"))
            BLEND_INPUTS[abs_(blend)] = (abs_(position), abs_(vertexVG) if vertexVG else "", VG_ROW(NUMBERING()), legacy, influences, vertexVGOut)
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
                keys = [(K.Type, K.Buffer), (K.Format, fmt.value.strip() if fmt is not None else K.FormatR16G16Float), (K.Stride, str(stride))],
                file = BESIDE_MESH(TARGET + K.Remap + "Texcoord.buf"),
                makeBytes = lambda path = abs_(texcoord): (lambda copy: (copy.clean(keep), copy.read())[1])(
                    FRB.TexcoordFile(path, FRB.TexcoordFile.halfElements(stride))))} if cleaned else {}
        return {"folder": folder, "texcoordCleaned": cleaned}

    def blendInputs(path):
        """What fixFiles read for the blend at 'path' -- absolute, or (as a resource's elements are asked) relative to the .ini"""
        key = os.path.normpath(path)
        if key in BLEND_INPUTS:
            return BLEND_INPUTS[key]
        return next(value for absPath, value in BLEND_INPUTS.items() if absPath.endswith(os.sep + key))

    def blendElements(srcPath):
        """The mod's blend line: as many influences as its Blend.buf carries (her 4, or a mod's own 8)"""
        return FRB.WWMIBlend.blendElements(blendInputs(srcPath)[4])

    def blendPairedIds(srcPath):
        """The id buffer that follows the blend's slots when it is widened: the blend remap's VertexVG, which names the bones"""
        return [FRB.VGRemapBlendReplace.PairedIds(blendInputs(srcPath)[5], 2)]

    def writeBlendRemap(resource):
        """The blend in the blend remap's LOCAL ids, and the remap's three buffers beside it (WWMIBlend.writeRemap)"""
        position, vertexVG, row, legacy, _, _ = blendInputs(resource.srcPath)
        beside = lambda which: os.path.join(os.path.dirname(resource.fixedPath), FRB.IniNamingTools.getRemapElementName("", "BlendRemap" + which, TARGET) + ".buf")
        FRB.WWMIBlend.writeRemap(resource.srcPath, resource.fixedPath, vertexVG, position, row,
                                 FRB.WWMIBlend.RemapFiles(beside("VertexVG"), beside("Forward"), beside("Reverse")), legacy)
        return True

    def SHADER_DRAW(c, i, value):
        """One draw of a part through her own shader, in place of the slot's: on the slot's pass, saving and putting back the
        constants it sets; plainly elsewhere -- and only where the draw has a render target (o3), when the shader says so"""
        name, game, onlyO3 = OWN_SHADERS[c]
        custom, cbs = DRAW(c, i).replace(K.CommandList, K.CustomShader, 1), list(SHADER_CBS[game])
        text = FRB.SectionText(Z3, DRAW(c, i))
        text = text.open(K.WWMIColourTarget + " !== " + K.Null) if onlyO3 else text
        text = (text.open(GUARDS[SLOT_OF[c]]).keys([(BACKUP(reg), K.Ref + " " + reg) for reg in cbs]).key(K.Run, custom)
                .keys([(reg, K.Ref + " " + BACKUP(reg)) for reg in cbs]).orElse().key(K.DrawIndexed, value).close())
        text = text.orElse().key(K.DrawIndexed, value).close() if onlyO3 else text
        return [text.build(), FRB.SectionText(Z3, custom).keys([(K.Ps, BESIDE_MESH(FRB.IniNamingTools.getRemapElementName("", game, TARGET) + ".hlsl")),
                                                                 *[(reg, CB(game, reg)) for reg in cbs], (K.DrawIndexed, value)]).build()]

    def OVERLAY_DRAWS(c, i, value):
        """One draw of a part OVER the slot's draw, by her own shader: on the slot's pass where the draw has a render target, its
        registers bound to the part's roles (the mod's files, else her game textures), saved and put back around it"""
        name, game, reads, state, _ = OVERLAYS[c]
        custom = OVERLAY_DRAW(c, i).replace(K.CommandList, K.CustomShader, 1)
        regs = list(SHADER_CBS[game]) + OVERLAY_REGS[c]
        text = (FRB.SectionText(Z3, OVERLAY_DRAW(c, i)).open(K.WWMIColourTarget + " !== " + K.Null).open(GUARDS[SLOT_OF[c]])
                .keys([(BACKUP(reg), K.Ref + " " + reg) for reg in regs]).key(K.Run, LIST_NAME(c, "Overlay"))
                .key(K.Run, custom).keys([(reg, K.Ref + " " + BACKUP(reg)) for reg in regs]).close().close())
        return [text.build(), FRB.SectionText(Z3, custom).keys([(K.Ps, BESIDE_MESH(FRB.IniNamingTools.getRemapElementName("", game, TARGET) + ".hlsl")),
                                                                 *[(reg, CB(game, reg)) for reg in SHADER_CBS[game]], *state, (K.DrawIndexed, value)]).build()]

    def HIDE(slot):
        """A target slot nothing of the mod lands on: the skin's own geometry skipped, its bones merged"""
        text = (FRB.SectionText(Z3, K.TextureOverride + TARGET + CAPITAL(slot) + K.Remap + "Hide")
                .prefix("; nothing of the mod is drawn through " + TARGET + "'s " + TARGET_LABELS[slot] + " slot: the skin's own geometry is skipped and its bones still merged")
                .keys([(K.Hash, T.hashes.get([TARGET, K.Vb0], TO, False)), (K.MatchFirstIndex, str(T.indices.get([TARGET, "", slot], TO, False))),
                       (K.MatchIndexCount, str(T.indexCounts.get([TARGET, "", slot], TO, False))), (K.WWMIObjectDetected, "1")])
                .open(K.WWMIModEnabled))
        if MERGED():
            text = (text.keys([(K.WWMIVgOffset, str(T.vgOffsets.get([TARGET, "", slot], TO, False))), (K.WWMIVgCount, str(T.vgCounts.get([TARGET, "", slot], TO, False))),
                               (K.Run, RENAME(FRB.WWMINames.MergeWindowList))])
                    .open(K.Resource + "MergedSkeleton !== " + K.Null).key(K.Handling, K.Skip).close())
        else:
            text = text.keys([(K.Run, RENAME(K.CommandList + "MergeSlot" + str(SLOT_NUM(slot)))), (K.Handling, K.Skip)])
        return text.close().build()

    # ================================================================================================= the fix
    EDITS = [
        FRB.GraphCreate(RESETS, z3 = Z3, sectionsOf = lambda group: [FRB.SectionText(Z3, "").build()] if group == 0 and any(
            row.section == "Present" and row.key.strip().lower() == "post " + K.WWMIObjectDetected and row.value.strip() == "0"
            for row in data().collected(FRB.IniParseRows.Sections)) else None),
        # an .ini that draws her parts but declares no mesh of its own (a linked file of textures): the fix writes nothing into
        # it -- every graph goes before any step reads one, and makes no file. (A file that only WATCHES her draws nothing,
        # and its watcher is carried onto the skin)
        FRB.GraphCreate(NO_MESH, z3 = Z3, sectionsOf = lambda group: [FRB.SectionText(Z3, "").build()]
                        if group == 0 and not HAS_MESH() and data().collected(FRB.IniParseRows.Objs) else None),
        FRB.GraphGroupRemove(onlyWith = [NO_MESH[1:]]),
        FRB.GraphVarApply([], fixFiles, "fixFiles"),

        # ---- per source component: its merge and the blend remap after the shared list; each draw through her own shader,
        #      followed by her overlay, or kept off a pass
        FRB.GraphGroupEdit([{("", c): [FRB.RegAnchorAdd([K.Run], lambda key, value, c = c:
                                                        [(key, value), (K.Run, RENAME(K.CommandList + "MergeSlot" + str(SLOT_NUM(SLOTS[c])))),
                                                         (K.WWMIBlendReg, OWN_BLEND(c)) if c in OWN_MESHES and OWN_BLEND(c) in OWN_MESH_FILES.files
                                                         else (K.Run, RENAME(K.CommandList + "BlendRemap"))]
                                                        if value.strip().lower() == (OWN_MESHES[c][0] if c in OWN_MESHES else SHARED).lower() else None,
                                                        FRB.RegAnchorAdd.Placement.Replace),
                                       FRB.RegPartEdit(FRB.RegAnchorAdd([K.DrawIndexed], lambda key, value, c = c:
                                                                        [(K.Run, DRAW(c, DRAWS_OF(c).index(value.strip())))] if c in OWN_SHADERS
                                                                        else [(K.Run, OFF_DRAW(c, value.strip()))] if c in OFF_PASSES
                                                                        else [(key, value), (K.Run, OVERLAY_DRAW(c, DRAWS_OF(c).index(value.strip())))] if c in OVERLAYS and OVERLAID(c)
                                                                        else [(key, value)],
                                                                        FRB.RegAnchorAdd.Placement.Replace),
                                                       lambda section, c = c: section in ROOTS(c))]
                             for c in SLOTS}]),

        # ---- the shared list parks the buffers it rebinds, and the cleanup list puts them back
        FRB.GraphGroupEdit([{("", c): [*[FRB.RegPartEdit(FRB.RegAnchorAdd([reg], lambda key, value, reg = reg: [(K.Resource + RENAME("Bypass" + reg.capitalize()), K.Ref + " " + reg)],
                                                                         FRB.RegAnchorAdd.Placement.Before, None, True, True),
                                                         lambda section: section.lower() == SHARED.lower()) for reg in BYPASSABLE],
                                       FRB.RegPartEdit(FRB.RegAdd([(K.Run, RENAME("CommandListRestoreSharedResources"))], True, True),
                                                       lambda section: section.lower() == CLEANUP.lower())]
                             for c in SLOTS}]),

        # ---- the fix's own sections: the parked buffers, the passes' tags, the registers a shader draw saves, the off passes
        FRB.GraphCreate(SECTIONS, z3 = Z3, onlyWith = SOURCES, sectionsOf = lambda group: [
            *[FRB.SectionText(Z3, K.Resource + RENAME("Bypass" + reg.capitalize())).build() for reg in BYPASSED()],
            FRB.SectionText(Z3, RENAME("CommandListRestoreSharedResources"))
               .keys([(reg, K.Ref + " " + K.Resource + RENAME("Bypass" + reg.capitalize())) for reg in BYPASSED()]).build(),
            *[FRB.SectionText(Z3, RENAME("ShaderOverridePass" + str(i))).keys([(K.Hash, vs), (K.FilterIndex, tag)]).build()
              for i, (vs, tag) in enumerate(PASSES.items())],
            *[FRB.SectionText(Z3, BACKUP(reg)).build() for reg in SHADER_REGS()],
            *[FRB.SectionText(Z3, OFF_DRAW(c, value)).open(OFF_PASSES[c]).key(K.DrawIndexed, value).close().build()
              for c in OFF_PASSES if DRAWN(c) for value in DRAWS_OF(c)]]),

        # ---- the skeleton: merged into buffers of the fix's, remapped into the skin's numbering once a frame, merged per slot
        FRB.GraphCreate(SKELETON, z3 = Z3, onlyWith = SOURCES, sectionsOf = lambda group: [
            *[FRB.SectionText(Z3, SK(merged)).build() for _, merged, _ in CBS],
            *[FRB.SectionText(Z3, SK(merged + "RW")).keys([(K.Type, K.RWBuffer), (K.Format, K.FormatR32G32B32A32Float),
                                                             (K.Array, SKELETON_SLOTS)]).build() for _, merged, _ in CBS],
            *[FRB.SectionText(Z3, SK(remapped)).build() for _, _, remapped in CBS],
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
              for offset, count in [(str(T.vgOffsets.get([TARGET, "", SLOT_PREFIX + str(slot)], TO, False)),
                                     str(T.vgCounts.get([TARGET, "", SLOT_PREFIX + str(slot)], TO, False)))]],
            FRB.SectionText(Z3, RENAME(FRB.WWMINames.MergeWindowList)).key(K.WWMICustomMeshScale, "1.00")
               .open(K.WWMIBoneDataCb + " == " + FILTER).keys([(K.WWMIMergerBonesReg, K.Ref + " " + K.WWMIBoneDataCb), (K.WWMIMergerSkeletonReg, SK("MergedSkeletonRW")),
                                                              (K.Run, K.WWMISkeletonMerger)]).close()
               .open(K.WWMIExtraBoneDataCb + " == " + FILTER).keys([(K.WWMIMergerBonesReg, K.Ref + " " + K.WWMIExtraBoneDataCb),
                                                                   (K.WWMIMergerSkeletonReg, SK("ExtraMergedSkeletonRW")), (K.Run, K.WWMISkeletonMerger)]).close().build(),
            FRB.SectionText(Z3, RENAME(K.TextureOverride + "MarkBoneDataCB")).keys(
                [(K.Hash, T.hashes.get([TARGET, "cb4"], TO, False)), (K.MatchPriority, "0"), (K.FilterIndex, FILTER)]).build()]),

        # ---- the blend remap: its buffers, and its one-time run that rewrites the blend's ids into the remap's LOCAL space
        FRB.GraphCreate(BLEND_REMAP, z3 = Z3, onlyWith = SOURCES, onlyWithout = [NO_MESH[1:]], sectionsOf = lambda group: [
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
               .key(K.WWMIBlendReg, SK("RemappedBlendBuffer")).build()]),

        # ---- each component's texture lists, guarded by the passes of the slot it goes to
        *[FRB.GraphCreate(LIST(c, kind), z3 = Z3, onlyWith = ("", SRC(c)), sectionsOf = lambda group, c = c, kind = kind: [
            FRB.SectionText(Z3, LIST_NAME(c, kind)).open((PASS0_GUARDS if kind == "Pass0" else GUARDS)[SLOT_OF[c]]).close().build()]
            if kind != "Overlay" or OVERLAID(c) else None)                      # (an overlay's: only where it is drawn)
          for c, kind, _ in LISTS],

        # ---- what a linked .ini sets on the draws: each linked file's state its conditions read, copied under the fix's names; the
        #      files the lines bind, through this file's own resource for the file, the role's declaration, or one of the fix's;
        #      and each component's list of the lines, run from its section
        FRB.GraphVarApply([], lambda: carried(), "carried"),
        FRB.GraphCreate(LINKED_STATE, z3 = Z3, onlyWith = SOURCES, sectionsOf = lambda group: data().variables()["carried"]["state"] or None),
        FRB.GraphCreate(CARRIED, z3 = Z3, onlyWith = SOURCES, sectionsOf = lambda group: [
            FRB.SectionText(Z3, name).key(K.Filename, rel).build() for name, rel in data().variables()["carried"]["declared"]] or None),
        *[FRB.GraphCreate(GLOW(c), z3 = Z3, onlyWith = ("", SRC(c)), sectionsOf = lambda group, c = c: (
            lambda lines: [glowList(c, lines)] if lines else None)(data().variables()["carried"]["lines"].get(c)))
          for c in SLOTS],

        # ---- each draw through her own shader, and each overlay over a draw
        *[FRB.GraphCreate(SHADER(c), z3 = Z3, onlyWith = ("", SRC(c)), sectionsOf = lambda group, c = c: [
            section for i, value in enumerate(DRAWS_OF(c)) for section in SHADER_DRAW(c, i, value)] or None)
          for c in OWN_SHADERS],
        *[FRB.GraphCreate(OVERLAY(c), z3 = Z3, onlyWith = ("", SRC(c)), sectionsOf = lambda group, c = c: [
            section for i, value in enumerate(DRAWS_OF(c)) for section in OVERLAY_DRAWS(c, i, value)] if OVERLAID(c) else None)
          for c in OVERLAYS],


        # ---- a role the mod toggles: the override doing so, renamed to a list of the fix's, its `this` moved to the register
        FRB.GraphGroupEdit([{TOGGLE(c, reg, role, kind)[1:]: [FRB.GraphRename(lambda name, new = RENAME(K.CommandList + SOURCE + CAPITAL(role) + FRB.IniNamingTools.getRegTag(reg)
                                                                                                 + ("Raw" if RAW(c, kind) else "")): new),
                                                        FRB.RegRemap({K.This: [reg]})]
                             for c, kind, reg, role in ALL_LINES if role not in CREATED}]),

        # ---- a toggle of ONE `this`: off, the original drew her game texture -- so that comes first, the toggle after it
        FRB.GraphGroupEdit([{TOGGLE(c, reg, role, kind)[1:]: [FRB.RegIfTemplateAdd(FRB.SectionText(Z3, "").key(reg, GAME(role)).build(),
                                                                             FRB.RegIfTemplateAdd.Position.BeforeFirstIf, everySection = True,
                                                                             sectionFilter = lambda name, section, role = role: SINGLE(role) and TOGGLED(role))]
                             for c, kind, reg, role in ALL_LINES if role in DOWNLOADS}]),

        # ---- a role the mod has no texture for: its stand-in, her game texture downloaded or a texture the fix makes
        *[FRB.GraphCreate(STAND_IN(c, role), z3 = Z3, onlyWithout = [ROLE(SRC(c), role)[1:], TOGGLE(c, reg, role, kind)[1:]],
                          sectionsOf = lambda group, role = role: [FRB.SectionText(Z3, DOWNLOAD(role) if role in DOWNLOADS else K.Resource + RENAME(role)).build()])
          for c, kind, reg, role in ALL_LINES if role in DOWNLOADS or role in CREATED],

        # ---- each list's lines, in register order: the role's resource at the register, a run of its toggle, or its stand-in
        *[FRB.GraphInherit(LIST(c, kind), graph, line, partFilter = lambda d, modType, ini, name = LIST_NAME(c, kind):
                               FRB.Ranges.createFull() if d.sectionName == name else FRB.Ranges.createEmpty())
          for c, kind, reg, role in ALL_LINES for graph, line in [(ROLE(SRC(c), role), reg), (TOGGLE(c, reg, role, kind), K.Run), (STAND_IN(c, role), reg)]],


        # ---- a list no line went into is no list; each one with lines runs from its component
        FRB.GraphFilter([LIST(c, kind) for c, kind, _ in LISTS], lambda owner, part, i: True, removeEmptied = True),
        *[FRB.GraphGroupEdit([{("", c): [FRB.RegSurroundedAdd([(K.Run, LIST_NAME(c))], beforeRegs = {K.MatchFirstIndex: lambda value: True})]}],
                             onlyWith = LIST(c)[1:]) for c in TEXTURES],
        *[FRB.GraphGroupEdit([{("", c): [FRB.RegSurroundedAdd([(K.Run, LIST_NAME(c, "Pass0"))],
                                                              {K.Run: lambda value: value.strip().lower() == RENAME(K.CommandList + "BlendRemap").lower(),
                                                               K.WWMIBlendReg: lambda value, c = c: value.strip() == OWN_BLEND(c)}, {}, False)]}],
                             onlyWith = LIST(c, "Pass0")[1:]) for c in PASS0],

        *[FRB.GraphGroupEdit([{("", c): [FRB.RegSurroundedAdd([(K.Run, GLOW_NAME(c))],
                                                              {K.Run: lambda value: value.strip().lower() == RENAME(K.CommandList + "BlendRemap").lower(),
                                                               K.WWMIBlendReg: lambda value, c = c: value.strip() == OWN_BLEND(c)}, {}, False)]}],
                             onlyWith = GLOW(c)[1:]) for c in SLOTS],

        # ---- the cleaned texcoords, when the clean changed anything: bound after the blend remap, in every remapped section
        FRB.GraphCreate(CLEANED, z3 = Z3, sectionsOf = lambda group: [FRB.SectionText(Z3, "").build()]
                        if group == 0 and data().variables()["fixFiles"]["texcoordCleaned"] else None),
        FRB.GraphGroupEdit([{("", c): [FRB.RegSurroundedAdd([(K.WWMITexcoordReg, TEXCOORD)], {K.Run: lambda value: value.strip().lower() == RENAME(K.CommandList + "BlendRemap").lower()},
                                                            {}, False)]
                             for c in SLOTS}], onlyWith = CLEANED[1:]),
        FRB.ResRegCollect({(0, "", c): K.WWMITexcoordReg for c in SLOTS}, {"texcoord": TEXCOORD_FILE},
                          resPredicates = {(0, "", c): lambda reg, value, d: value.strip() == TEXCOORD for c in SLOTS}),

        FRB.GraphRemove([STAND_IN(c, role) for c, _, _, role in ALL_LINES] + [STAND_IN(c, role) for c, (*_, reads, _, _) in OVERLAYS.items() for role in reads]),

        # ---- the game defaults the toggles bind
        *[FRB.ResRegCollect({toggle: reg for toggle, reg in dict.fromkeys((TOGGLE(c, r, ro, k), r) for c, k, r, ro in ALL_LINES if ro == role)},
                            {"gameDefault": FRB.FileDownloadCreate((0, "gameDefault", role), {GAME(role): FRB.FileDownloadCreate.File(
                                file = FRB.DownloadTools.fixedFileName(SOURCE, CAPITAL(role) + "Game", FRB.FileExt.DDS),
                                url = FRB.DownloadTools.url("WuWa", SOURCE, FOLDER(role), SOURCE, "Texture" + textureHash, FRB.FileExt.DDS))})},
                            resPredicates = {toggle: lambda key, value, d, role = role: value.strip() == GAME(role)
                                             for toggle in dict.fromkeys(TOGGLE(c, r, ro, k) for c, k, r, ro in ALL_LINES if ro == role)})
          for role, textureHash in DOWNLOADS.items() if any(ro == role for _, _, _, ro in ALL_LINES)],

        # ---- the skin's own meshes: each one's pass binds her texture for it
        *[FRB.GraphCreate(SHARED_MESH(mesh), z3 = Z3, onlyWith = SOURCES, sectionsOf = lambda group, mesh = mesh, roles_ = roles_: [
            (lambda text: text.close().build())(
                FRB.SectionText(Z3, RENAME(K.TextureOverride + SOURCE + "SharedMesh" + str(list(SHARED_MESHES).index(mesh))))
                .key(K.Hash, need(T.hashes.get([TARGET, mesh], TO, False), TARGET + " " + mesh))
                .open(" || ".join("vs == " + tag for tag in T.getDrawPassTags(mesh, "main", TO)))
                .keys([(need(T.getTextureRegister(mesh, role, TO), TARGET + " " + mesh + " " + role), DOWNLOAD(role)) for role in roles_]))])
          for mesh, roles_ in SHARED_MESHES.items()],

        # ---- the downloads and the made textures the lists and the shader draws bind
        *[FRB.ResRegCollect({**{LIST(c, kind): lambda key: True for c, kind, _ in LISTS},
                             **{SHARED_MESH(mesh): lambda key: True for mesh in SHARED_MESHES}},
                            {"download": FRB.FileDownloadCreate((0, "download", role), {DOWNLOAD(role): FRB.FileDownloadCreate.File(
                                file = FRB.DownloadTools.fixedFileName(SOURCE, CAPITAL(role), FRB.FileExt.DDS),
                                url = FRB.DownloadTools.url("WuWa", SOURCE, FOLDER(role), SOURCE, "Texture" + textureHash, FRB.FileExt.DDS))})},
                            resPredicates = {**{LIST(c, kind): lambda key, value, d, role = role: value.strip() == DOWNLOAD(role) for c, kind, _ in LISTS},
                                             **{SHARED_MESH(mesh): lambda key, value, d, role = role: value.strip() == DOWNLOAD(role) for mesh in SHARED_MESHES}})
          for role, textureHash in DOWNLOADS.items()],
        *[FRB.ResRegCollect({LIST(c, kind): lambda key: True for c, kind, _ in LISTS},
                            {"created": FRB.TexCreate((0, "created", role), "", creator,
                                                      files = {K.Resource + RENAME(role): FRB.IniNamingTools.getModSuffixedName(role, K.RemapTex, TARGET) + FRB.FileExt.DDS})},
                            resPredicates = {LIST(c, kind): lambda key, value, d, role = role: value.strip() == K.Resource + RENAME(role) for c, kind, _ in LISTS})
          for role, creator in CREATORS.items()],

        # ---- the texture edits, on each file the lists bind for the role -- directly, or as a variant of the mod's toggle of it
        #      (not a shader's raw copy) -- in ONE collect, which builds every edited file together
        *[FRB.ResRegCollect({**{LIST(c2, kind): [reg for reg, r in lines.items() if r == role] for c2, kind, lines in LISTS if c2 == c and role in lines.values()},
                             **{TOGGLE(c2, reg, r, kind): reg for c2, kind, reg, r in ALL_LINES if c2 == c and r == role and not RAW(c2, kind)}},
                            {"texEdit": FRB.TexReplace((0, "", "texEdit" + name), FRB.TexEditor(filters = [texFilter(FROM, TO)]), resSubType = name)},
                            resPredicates = {**{LIST(c2, kind): FRB.ResRegCollect.skipsKeywords() for c2, kind, lines in LISTS if c2 == c and role in lines.values()},
                                             **{TOGGLE(c2, reg, r, kind): FRB.ResRegCollect.skipsKeywords() for c2, kind, reg, r in ALL_LINES
                                                if c2 == c and r == role and not RAW(c2, kind)}})
          for (c, role), (name, texFilter) in TEX_EDITS.items()],

        # ---- her shaders' code and constants, beside the mod's mesh
        # (one collect for every shader and register: a second sharing the file edit would build none of its own)
        FRB.ResRegCollect({**{SHADER(c): list(SHADER_CBS[OWN_SHADERS[c][1]]) for c in OWN_SHADERS},
                           **{OVERLAY(c): list(SHADER_CBS[OVERLAYS[c][1]]) for c in OVERLAYS}}, {"shaderBuffer": SHADER_BUFFERS},
                          resPredicates = {**{SHADER(c): FRB.ResRegCollect.skipsKeywords() for c in OWN_SHADERS},
                                           **{OVERLAY(c): FRB.ResRegCollect.skipsKeywords() for c in OVERLAYS}}),
        FRB.ResRegCollect({**{SHADER(c): "ps" for c in OWN_SHADERS}, **{OVERLAY(c): "ps" for c in OVERLAYS}}, {"shaderSource": SHADER_SOURCES},
                          resPredicates = {**{SHADER(c): FRB.ResRegCollect.skipsKeywords() for c in OWN_SHADERS}, **{OVERLAY(c): FRB.ResRegCollect.skipsKeywords() for c in OVERLAYS}}),

        # ---- the shape keys in the skin's order: the mod's shape-key lists bind the fix's reordered buffers
        FRB.GraphGroupEdit([{("", "shapekeyOffsets"): [FRB.RegNewVals({reg: K.Resource + RENAME(name + "Buffer")
                                                                        for reg, name in ((K.WWMIShapeKeyOffsetReg, "ShapeKeyOffset"), (K.WWMIShapeKeyVertexIdReg, "ShapeKeyVertexId"), (K.WWMIShapeKeyVertexOffsetReg, "ShapeKeyVertexOffset"))})]}]),
        FRB.ResRegCollect({(0, "", "shapekeyOffsets"): [K.WWMIShapeKeyOffsetReg, K.WWMIShapeKeyVertexIdReg, K.WWMIShapeKeyVertexOffsetReg]}, {"shapeKeys": SHAPE_KEY_FILES}),
        FRB.ResRegCollect({(0, "", c): K.WWMIBlendReg for c in SLOTS}, {"ownMesh": OWN_MESH_FILES},
                          resPredicates = {(0, "", c): lambda reg, value, d: value.strip() in OWN_MESH_FILES.files for c in SLOTS}),

        # ---- each component onto its target slot (several onto one slot: SAME_FILE in the mod's own); the hash-only objects onto
        #      themselves; the fix's own graphs into every file
        FRB.GraphGroupRemap({**{(0, "", c): [(0,) + TARGET_OBJ[c] + (RENAME,)] for c in SLOTS},
                             (0, "", "boneData"): [(0, "", "boneData", RENAME, True)],
                             (0, "", "shapekeyOffsets"): [(0, "", "shapekeyOffsets", RENAME, False)],
                             (0, "", "shapekeyScale"): [(0, "", "shapekeyScale", RENAME, False)],
                             (0, "", "watch"): [(0, TARGET, "watch", RENAME)],
                             **{graph: [graph + (lambda name: name, True)]
                                for graph in [SECTIONS, SKELETON, BLEND_REMAP, RESETS, *[LIST(c, kind) for c, kind, _ in LISTS],
                                              *[SHADER(c) for c in OWN_SHADERS], *[OVERLAY(c) for c in OVERLAYS],
                                              *[(0, "download", role) for role in DOWNLOADS], *[SHARED_MESH(mesh) for mesh in SHARED_MESHES],
                                              *[(0, "created", role) for role in CREATED],
                                              (0, "", "shaderBuffers"), (0, "", "shapeKeyFiles"), (0, "", "ownMeshFiles"), (0, "", "texcoordFile"), *[(0, "", "texEdit" + name) for name, _ in TEX_EDITS.values()],
                                              *TOGGLES, *[(0, "gameDefault", role) for role in DOWNLOADS],
                                              *dict.fromkeys(REF(role) for _, role in ROLES), LINKED_STATE, CARRIED, *[GLOW(c) for c in SLOTS]]}}),

        # ---- each remapped section drops the lines that would undo the remap: WWMI's blend remap and skeleton merge of the
        #      SOURCE, and RabbitFX's
        FRB.GraphGroupEdit([{TARGET_OBJ[c]: [FRB.RegRemove([(reg, FRB.RegRemove.valueStartsWith(prefix) if prefix else None) for reg, prefix in REMOVED]),
                                             FRB.RegRemove([*[(reg, None) for reg in RABBITFX_LINES], (K.Run, FRB.RegRemove.valueStartsWith(K.RabbitFXSetTexturesPath.lower()))])]
                             for c in SLOT_OF}], everyGroup = True),

        # ---- each target object's numbers (its slot's), in every file
        FRB.GraphGroupEdit([{TARGET_OBJ[c]: [FRB.RegNewVals({key: str(table.get([TARGET, "", SLOT_OF[c]], TO, False))
                                                             for key, table in [(K.MatchFirstIndex, T.indices), (K.MatchIndexCount, T.indexCounts),
                                                                                (K.WWMIVgOffset, T.vgOffsets), (K.WWMIVgCount, T.vgCounts)]})]
                             for c in SLOT_OF}], everyGroup = True),

        # ---- the hashes onto the skin's, and the shape keys' dispatch size
        FRB.GraphGroupEdit([{**{TARGET_OBJ[c]: [FRB.RegAssetRemap({K.Hash: (S.hashes, K.HashNotFound)}, TARGET, SOURCE, FROM, TO)] for c in SLOT_OF},
                             **{("", obj): [FRB.RegAssetRemap({K.Hash: (S.hashes, K.HashNotFound),
                                                               **{K.WWMIShapeKeyChecksum + s: (S.shapeKeyChecksums, FRB.WWMINames.ChecksumNotFound) for s in ("", "_batch0", "_batch1")}},
                                                              TARGET, SOURCE, FROM, TO),
                                            FRB.RegNewVals({K.WWMIShapeKeyDispatchSizeYOriginalBatch + n: DISPATCH for n in "01"})]
                                for obj in ["boneData", "shapekeyOffsets", "shapekeyScale"]},
                             (TARGET, "watch"): [FRB.RegAssetRemap({K.Hash: (S.hashes, K.HashNotFound)}, TARGET, SOURCE, FROM, TO)]}], everyGroup = True),

        # ---- each target slot nothing of the mod lands on: skipped, its bones merged
        *[FRB.GraphCreate((0, TARGET, "hide" + slot), z3 = Z3, onlyWith = [TARGET_OBJ[c] for c in SLOT_OF],
                          onlyWithout = [TARGET_OBJ[c] for c in SLOT_OF if SLOT_OF[c] == slot], sectionsOf = lambda group, slot = slot: [HIDE(slot)])
          for slot in TARGET_SLOTS],

        # ---- the frame's skeleton remap: latched on $object_detected where the mod's [Present] resets it
        FRB.GraphGroupEdit([{**{TARGET_OBJ[c]: [FRB.RegIfTemplateAdd(FRB.SectionText(Z3, "").open(K.WWMIObjectDetected + " == 0")
                                                                     .key(K.Run, RENAME(K.CommandList + "RemapMergedSkeleton")).close().build(),
                                                                     FRB.RegIfTemplateAdd.Position.BeforeTopKey, K.WWMIObjectDetected, value = "1")]
                                for c in SLOT_OF},
                             **{(TARGET, "hide" + slot): [FRB.RegIfTemplateAdd(FRB.SectionText(Z3, "").open(K.WWMIObjectDetected + " == 0")
                                                                               .key(K.Run, RENAME(K.CommandList + "RemapMergedSkeleton")).close().build(),
                                                                               FRB.RegIfTemplateAdd.Position.BeforeTopKey, K.WWMIObjectDetected, value = "1")]
                                for slot in TARGET_SLOTS}}],
                           everyGroup = True, onlyWith = RESETS[1:]),

        # ---- the blend, through Changli's vertex group row onto the skin's, widened to its 8 influences, in the remap's local ids
        FRB.ResRegCollect({(0,) + TARGET_OBJ[c]: K.WWMIBlendReg for c in SLOT_OF},
                          {"blend": FRB.VGRemapBlendReplace((0, "", "blend"), S, FROM, TO, row = VG_ROW(REFERENCE), influences = 8,
                                                            elements = blendElements, pairedIds = blendPairedIds, fixFunc = writeBlendRemap)},
                          resPredicates = {(0,) + TARGET_OBJ[c]: lambda key, value, d: FRB.ResRegCollect.skipsKeywords()(key, value, d)
                                           and value.strip() not in OWN_MESH_FILES.files
                                           and value.strip().lower() not in {mesh[1].lower() for mesh in OWN_MESHES.values()} for c in SLOT_OF}, everyGroup = True),
        FRB.GraphGroupRemap({(0, "", "blend"): [(0, "", "blend", lambda name: name, True)]}),
        FRB.GraphGroupEdit([{TARGET_OBJ[c]: [FRB.RegPartEdit(FRB.RegNewVals({K.WWMIBlendReg: FRB.IniNamingTools.getRemapBlendResourceName("", TARGET) + "Buffer"}),
                                                             lambda section: section.lower().startswith(SHARED.lower())   # (not a part's own mesh's copy)
                                                             and not any(section.lower().startswith(mesh[0].lower()) for mesh in OWN_MESHES.values()))]
                             for c in SLOT_OF}], everyGroup = True),


        # ---- the parser's texture graphs are not written; and nothing of Changli found means nothing is written
        FRB.GraphRemove([*[ROLE(c, role) for c, role in ROLES], RESETS, CLEANED, NO_MESH,
                         *[(0, TEXTURE, role) for _, role in ROLES], (0, TEXTURE, "")]),
        FRB.GraphGroupRemove(onlyWithout = [TARGET_OBJ[c] for c in SLOT_OF] + [(TARGET, "watch")]),
    ]

    return types.SimpleNamespace(**locals())


LISTS = {}


def at(ini_):
    """The lists at an .ini's run's versions: the ones the run was asked for, else her and the skin's newest"""
    FROM, TO = ini_.fromVersion or NEWEST(S), ini_.toVersion or FRB.WWMIFixerConfig.newestVersionOf(T)   # the skin's newest game version (WWMIFixer::toVersion)
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
    service = FRB.RemapService(path = os.path.abspath(args.folder), downloadMode = args.download, toModTypeIds = {int(FRB.ModTypeId.ChangliNymph)},
                               fromVersion = V(args.fromVersion) if args.fromVersion else None, toVersion = V(args.version) if args.version else None)
    service.fix()
    for path, error in service.stats.ini.skipped.items():
        print("SKIPPED", path, error)

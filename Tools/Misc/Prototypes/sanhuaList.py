# SANHUA -> SANHUAEXORCIST, AS A LIST OF EDITS
#
# The parser sorts the mod's sections into graphs (a WWMISectionClassifier over Sanhua's hash and index tables) and reads
# each role's texture with collects (PARSE); the fixer is the list EDITS: the library's generic edits, with this pair's
# choices written out as tables.
#
# NO GAME DATA IS WRITTEN HERE: every register, draw pass, hash, index, mask colour and the shape keys' dispatch size is
# read from the API's versioned asset tables (ModType's TextureRegisters, DrawPasses, Hashes, Indices, MaterialColours,
# ShapeKeyDispatchSizes, VGRemaps)
# at the RUN's versions -- the ones the run is asked for (--fromVersion / --version), else her and the skin's newest. So
# the lists are built per run (lists(FROM, TO)), and what this file holds at its top is only the pair's version-free
# CHOICES: which component goes where, and which role lands on which.
#
#   py -3 sanhuaList.py <mod folder> [--fromVersion X] [--version Y] [--download normal]
import argparse
import itertools
import os
import sys
import types

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "Anime Game Remap (for all users)", "api"))
import src.py.FixRaidenBoss2 as FRB          # noqa: E402
from wwmiSectionClassifier import WWMISectionClassifier    # noqa: E402

K = FRB.IniKeywords
SOURCE, TARGET = "Sanhua", "SanhuaExorcist"
S, T = FRB.ModTypes[SOURCE].value, FRB.ModTypes[TARGET].value
Z3 = FRB.Z3Context()
V = FRB.Version.parse
NEWEST = lambda modType: modType.hashes.versionOf(modType.hashes.get([modType.name, "vb0"], None, False), [modType.name, "vb0"])   # her newest model
FIX = "SanhuaExorcistRemapFix"                  # every section the fix writes ends with this


def need(value, what):
    """A table row the list cannot do without: missing, the list says which rather than writing a wrong section"""
    if value is None:
        raise LookupError("no asset table row for " + what)
    return value


# ================================================================================================= the pair's choices
# each source component's target slot
SLOTS = {"component0": "component0", "component1": "component1", "component2": "component2", "component3": "component3",
         "component4": "component3", "component5": "component3", "component6": "component5"}

# each source component's texture list: each role of the mod's under the name the target slot's draw has for it (its
# register read from the skin's table) -- her skin, bodice and skirt all go through the skin's torso
ROLES_ON_TARGET = {"component0": {"bangsDiffuse": "bangsDiffuse", "bangsMask": "bangsMask", "t5Ramp": "t5Ramp"},
                   "component1": {"hairDiffuse": "hairDiffuse", "hairNormal": "hairNormal", "t5Ramp": "t5Ramp"},
                   "component2": {"faceMask": "faceMask", "faceDiffuse": "faceDiffuse"},
                   "component3": {"skinNormal": "torsoNormal", "SkinMask": "torsoMask", "skinDiffuse": "torsoDiffuse"},
                   "component4": {"bodiceNormal": "torsoNormal", "bodiceMask": "torsoMask", "bodiceDiffuse": "torsoDiffuse"},
                   "component5": {"skirtNormal": "torsoNormal", "skirtMask": "torsoMask", "skirtDiffuse": "torsoDiffuse"},
                   "component6": {"eyeMask": "eyeMask", "irisDiffuse": "irisDiffuse", "t5Ramp": "t5Ramp"}}

# a role the mod has no texture for: Sanhua's own game texture, downloaded -- but her face's and eyes' masks, which the
# skin's slots draw with their own (GAME_LEFT) -- or one the fix makes: her bare skin has no mask, so it gets a flat one
# in the colour the skin's torso mask marks skin with (MaterialColours)
GAME_LEFT = ["faceMask", "eyeMask"]
CREATED = {"SkinMask": (16, 16, "component3", "torsoMask.skin")}            # role -> (width, height, the skin's slot, its colour)

# the buffers the mod's shared list rebinds, parked first and put back by the cleanup list
BYPASSED = ["ib", "vb1", "vb2", "vb3", "vb4"]
NOT_TOGGLES = [K.WWMIObjectDetected, K.WWMIObjectDetected + "==1"]     # conditions every section of a WWMI mod sits under

# what SanhuaExorcist's slots draw, for the comment on a hidden one
TARGET_LABELS = {"component0": "bangs", "component1": "hair", "component2": "face", "component3": "torso, arms, ribbons",
                 "component4": "hair bun, trousers", "component5": "eyes"}

# ================================================================================================= the graphs, by id
TEXTURE = FRB.IniGraphModObjKeywords.Texture
LINES_OF = [(c, role) for c, roles in ROLES_ON_TARGET.items() for role in roles]
ROLES = list(dict.fromkeys(LINES_OF))
ROLE = lambda c, role: (0, "role", c + ":" + role)                     # the mod's resource for a component's role
TOGGLE = lambda c, reg, role: (0, "toggle", role + ":" + reg)          # ...or the override that toggles it, per register
REF = lambda role: (0, "ref", role)                                     # a resource the fix declares for a linked .ini's file
REF_NAME = lambda role: K.Resource + FRB.IniNamingTools.getModSuffixedName(role[:1].upper() + role[1:], K.RemapRef, TARGET)
LIST = lambda c: (0, "list", c)                                         # a source component's texture list
STAND_IN = lambda c, role: (0, "standIn", c + ":" + role)               # a role's stand-in, when the mod has no texture for it
SECTIONS = (0, "", "fixSections")                                       # the fix's own sections
GLOBAL = (0, "global", "")                                              # the whole file, while the parser reads it
RENAME = lambda name: FRB.IniNamingTools.getRemapFixName(name, TARGET)
DOWNLOAD = lambda role: FRB.IniNamingTools.getRemapDLResourceName(SOURCE + role[:1].upper() + role[1:])
data, ini = (lambda: FRB.EditScope.parseData()), (lambda: FRB.EditScope.ini())
OWN = lambda value: ini().resourceNamed(value)                          # the resource a value names ("" for none)
FILE = lambda section: (lambda row: row.value.strip() if row is not None else None)(
    data().firstCollected(FRB.IniParseRows.Declared, section, K.Filename))   # the file a resource section declares
ABS = lambda rel: os.path.normpath(os.path.join(ini().folder, rel.replace("\\", os.sep)))   # a path the .ini names, on disk
# a resource the mod could use for a role: one whose file the folder holds
USABLE = lambda value, role: (lambda file: file is not None and os.path.isfile(ABS(file)))(FILE(OWN(value)))
TOGGLED = lambda role: any(row.graphId[1:] == (TEXTURE, role) and any(g.replace(" ", "").lower() not in NOT_TOGGLES for g in row.guards)
                           for row in data().collected("this"))         # the mod's override of the role's hash binds under a toggle
# a component DRAWS when a line it reaches draws, or runs a list this file does not hold (a library's); one that draws nothing
# and skips the game's draw is UNDRAWN -- unless every component looks so (a mod the reading does not understand)
DRAWS = lambda c: any(tuple(row.graphId[1:]) == ("", c) and (row.key.strip().lower() == K.DrawIndexed or (
    row.key.strip().lower() == K.Run and ini().getSection(row.value.strip()) is None and "\\wwmiv1\\" not in row.value.lower()))
    for row in data().collected(FRB.IniParseRows.Objs))
SKIPS = lambda c: any(tuple(row.graphId[1:]) == ("", c) and row.key.strip().lower() == K.Handling and row.value.strip().lower() == K.Skip
                      for row in data().collected(FRB.IniParseRows.Objs))


# ================================================================================================= the lists, at a run's versions
def lists(FROM, TO):
    """The pair's parser and fixer at a run's versions -- FROM her mods', TO the skin's: every register, pass, hash and
    table row read from the API's asset tables at them"""

    # ---- each texture list's registers: where the skin's slot draws each role (TextureRegisters)
    TEXTURES = {c: {need(T.getTextureRegister(SLOTS[c], role, TO), TARGET + " " + SLOTS[c] + " " + role): srcRole for srcRole, role in roles.items()}
                for c, roles in ROLES_ON_TARGET.items()}
    LINES = [(c, reg, role) for c, lines in TEXTURES.items() for reg, role in sorted(lines.items(), key = lambda line: int(line[0][len("ps-t"):]))]

    # ---- the skin's slots and each one's draw passes, by their vertex shaders' tags (DrawPasses): a texture list binds only
    #      on its slot's passes
    TARGET_SLOTS = list(itertools.takewhile(lambda slot: T.indices.get([TARGET, "", slot], TO, False) is not None,
                                            ("component" + str(n) for n in itertools.count())))
    passNames = lambda slot: list(itertools.takewhile(lambda name: T.getDrawPass(slot, name, TO) is not None, ["main"] + ["main" + str(n) for n in range(2, 10)]))
    shadersOf = lambda slot: [vs for name in passNames(slot) for vs in T.getDrawPass(slot, name, TO).split(",")]
    tagsOf = lambda slot: [tag for name in passNames(slot) for tag in T.getDrawPass(slot, name + ".tags", TO).split(",")]
    PASSES = {vs: tag for slot in TARGET_SLOTS for vs, tag in zip(shadersOf(slot), tagsOf(slot))}
    GUARDS = {slot: " || ".join("vs == " + tag for tag in dict.fromkeys(tagsOf(slot))) for slot in TARGET_SLOTS}

    # ---- her game textures (Hashes), each downloaded from the folder of its row's version
    DOWNLOADS = {role: textureHash for role in sorted({role for _, role in ROLES}) if role not in GAME_LEFT and role not in CREATED
                 for textureHash in [S.hashes.get([SOURCE, role], FROM, False)] if textureHash is not None}
    FOLDER = lambda role: FRB.DownloadTools.versionFolder(S.hashes.versionOf(DOWNLOADS[role], [SOURCE, role]))

    # ---- where Sanhua's own draws bind each role, as her mods write it (by RabbitFX's keys; the rest bind by hash)
    SOURCE_REGISTERS = {c: {reg: role for role in ROLES_ON_TARGET[c] for reg in [S.getTextureRegister(c, role + "@rabbitfx", FROM)] if reg is not None}
                        for c in SLOTS}
    DISPATCH = need(T.getShapeKeyDispatchSize(TO), "the shape-key dispatch size")
    CREATORS = {role: FRB.CppTexCreator(width, height, need(T.getMaterialColour(slot, colour, TO), TARGET + " " + slot + " " + colour))
                for role, (width, height, slot, colour) in CREATED.items()}

    # ---------------------------------------------------------------------------------------------------- the parser
    # The classifier sorts the sections (WWMISectionClassifier: components by vb0 + match_first_index, the hash-only
    # objects, each role's texture overrides by every hash the role has had); these edits read what the fixer needs
    CLASSIFIER = WWMISectionClassifier(S, FROM, list(SLOTS), [role for _, _, role in LINES],
                                       {"cb4": "boneData", "shapekey_offsets": "shapekeyOffsets", "shapekey_scale": "shapekeyScale"}, [SOURCE, TARGET])
    TOGGLES = list(dict.fromkeys(TOGGLE(c, reg, role) for c, reg, role in LINES))
    PARSE = [
        # the rows the fixer reads: every declaration of the file, every key of each component and of each role's overrides
        FRB.GraphCreate(GLOBAL, z3 = Z3, sectionsOf = lambda group: list(ini().getIfTemplates().values()) if group == 0 else []),
        FRB.GraphCollect({GLOBAL: [K.Filename]}, name = FRB.IniParseRows.Declared),
        FRB.GraphRemove([GLOBAL]),
        FRB.GraphCollect({(0, "", c): lambda key: True for c in SLOTS}, name = FRB.IniParseRows.Objs),

        # a section that only WATCHES the source's vb0 is the fix's only in a file that draws none of her components (a help
        # overlay's, a skill effect's): there it is carried onto the target's vb0; beside her components it rides along as written
        FRB.GraphFilter([(0, "", "watch")], lambda owner, part, i: not data().collected(FRB.IniParseRows.Objs), removeEmptied = True),
        FRB.GraphCollect({(0, TEXTURE, role): [K.This] for _, role in ROLES}, name = "this", withConditions = True),

        # a component the mod draws nothing of, skipping the game's draw, is none of this fix's: its target slot is hidden
        *[FRB.GraphFilter([(0, "", c)], lambda owner, part, i, c = c: DRAWS(c) or not SKIPS(c) or not any(DRAWS(other) for other in SLOTS),
                          removeEmptied = True) for c in SLOTS],

        # each component's resource for a role: the mod's override of the role's hash (when it does not toggle it) -- else
        # what the component binds at the role's register (an existing role graph is kept); either only when it is USABLE
        *[FRB.ResRegCollect({(0, TEXTURE, role): K.This}, {"": FRB.ResIdentity(ROLE(c, role), createResModel = False, fileOf = OWN)},
                            resPredicates = {(0, TEXTURE, role): lambda reg, value, d, role = role: OWN(value) != "" and USABLE(value, role) and not TOGGLED(role)})
          for c, role in ROLES],
        *[FRB.ResRegCollect({(0, "", c): reg}, {"": FRB.ResIdentity(ROLE(c, role), createResModel = False, fileOf = OWN)},
                            resPredicates = {(0, "", c): lambda reg, value, d, role = role: OWN(value) != "" and USABLE(value, role)})
          for c, regs in SOURCE_REGISTERS.items() for reg, role in regs.items() if (c, role) in ROLES],

        # ...or the override that toggles it, as it is: a copy per register it is bound at
        *[FRB.GraphCopy((0, TEXTURE, role), toggle, lambda owner, part, i, role = role: TOGGLED(role) and part.getByInd(i)[0].lower() not in K.MatchKeys,
                        keepEmpty = False)
          for toggle, role in dict.fromkeys((TOGGLE(c, reg, role), role) for c, reg, role in LINES)],


        # a mod with no vg_offset (its blend's ids are not the ones a vertex group row maps) is none of this fix's; and a
        # role graph left empty is no graph: a fix tells "the mod has none" by its absence
        FRB.GraphFilter([(0, "", c) for c in SLOTS] + [(0, "", "boneData"), (0, "", "shapekeyOffsets"), (0, "", "shapekeyScale")],
                        lambda owner, part, i: any(row.key.lower() == K.WWMIVgOffset.lower() for row in data().collected(FRB.IniParseRows.Objs)),
                        removeEmptied = True),
        FRB.GraphFilter([ROLE(c, role) for c, role in ROLES] + TOGGLES, lambda owner, part, i: True, removeEmptied = True),

        # a role no line of this file binds, but a LINKED .ini binds on the component's draw: its file, declared by the fix
        FRB.GraphVarApply([], lambda: linkedFiles(), "linkedFiles"),
        *[FRB.GraphCreate(graph, z3 = Z3, onlyWithout = [ROLE(c, role)[1:]], sectionsOf = lambda group, c = c, role = role: (
            lambda rel: [FRB.SectionText(Z3, REF_NAME(role)).key(K.Filename, rel).build()] if rel else None)(data().variables()["linkedFiles"].get((c, role))))
          for c, role in ROLES for graph in (REF(role), ROLE(c, role))],   # the declaration first: it is made while the role has none
    ]
    LINKED_PARSE = [   # what is read of a linked .ini: its declarations, and every key of each component
        FRB.GraphCreate(GLOBAL, z3 = Z3, sectionsOf = lambda group: list(ini().getIfTemplates().values()) if group == 0 else []),
        FRB.GraphCollect({GLOBAL: [K.Filename]}, name = FRB.IniParseRows.Declared),
        FRB.GraphRemove([GLOBAL]),
        FRB.GraphCollect({(0, "", c): lambda key: True for c in SLOTS}, name = FRB.IniParseRows.Objs)]
    parser = lambda ini_, edits = None: FRB.GIMIParser(ini_, modObjs = CLASSIFIER.objects, objTargetFuncs = [CLASSIFIER], disjointModObjs = False,
                                                       modTypeId = getattr(FRB.ModTypeId, SOURCE), keysToTrack = {K.Hash, K.MatchFirstIndex, K.MatchIndexCount},
                                                       commandEdits = PARSE if edits is None else edits)

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

    # ---------------------------------------------------------------------------------------------------- the fix
    EDITS = [
        # each component runs its texture list first thing, so the mod's own register lines after it still win
        FRB.GraphGroupEdit([{("", c): [FRB.RegSurroundedAdd([(K.Run, RENAME("CommandListSanhua" + c.capitalize() + "Textures"))],
                                                            beforeRegs = {K.MatchFirstIndex: lambda value: True})] for c in SLOTS}]),

        # the shared list parks the buffers it rebinds, and the cleanup list puts them back
        FRB.GraphGroupEdit([{("", c): [*[FRB.RegPartEdit(FRB.RegAnchorAdd([reg], lambda key, value, reg = reg: [(K.Resource + RENAME("Bypass" + reg.capitalize()), K.Ref + " " + reg)],
                                                                         FRB.RegAnchorAdd.Placement.Before, None, True, True),
                                                         lambda section: section.lower() == "commandlistoverridesharedresources") for reg in BYPASSED],
                                       FRB.RegPartEdit(FRB.RegAdd([(K.Run, RENAME("CommandListRestoreSharedResources"))], True, True),
                                                       lambda section: section.lower() == "commandlistcleanupsharedresources")]
                             for c in SLOTS}]),

        # the fix's own sections when the mod draws any of Sanhua's components: the parked buffers, the list that restores
        # them, and the passes' tags
        FRB.GraphCreate(SECTIONS, z3 = Z3, onlyWith = [("", c) for c in SLOTS], sectionsOf = lambda group: [
            *[FRB.SectionText(Z3, K.Resource + RENAME("Bypass" + reg.capitalize())).build() for reg in BYPASSED],
            FRB.SectionText(Z3, RENAME("CommandListRestoreSharedResources"))
               .keys([(reg, K.Ref + " " + K.Resource + RENAME("Bypass" + reg.capitalize())) for reg in BYPASSED]).build(),
            *[FRB.SectionText(Z3, RENAME("ShaderOverridePass" + str(i))).keys([(K.Hash, vs), (K.FilterIndex, tag)]).build()
              for i, (vs, tag) in enumerate(PASSES.items())]]),

        # each component's texture list, guarded by the passes of the slot it goes to
        *[FRB.GraphCreate(LIST(c), z3 = Z3, onlyWith = ("", c), sectionsOf = lambda group, c = c: [
            FRB.SectionText(Z3, RENAME("CommandListSanhua" + c.capitalize() + "Textures")).open(GUARDS[SLOTS[c]]).close().build()])
          for c in SLOTS],

        # a role the mod toggles: the override doing so, renamed to a list of the fix's, its `this` moved to the register
        FRB.GraphGroupEdit([{TOGGLE(c, reg, role)[1:]: [FRB.GraphRename(lambda name, new = RENAME(K.CommandList + SOURCE + role[:1].upper() + role[1:] + FRB.IniNamingTools.getRegTag(reg)): new),
                                                   FRB.RegRemap({K.This: [reg]})]
                             for c, reg, role in LINES}]),

        # a role the mod has no texture for: its stand-in
        *[FRB.GraphCreate(STAND_IN(c, role), z3 = Z3, onlyWithout = [ROLE(c, role)[1:], TOGGLE(c, reg, role)[1:]], sectionsOf = lambda group, role = role: [
            FRB.SectionText(Z3, DOWNLOAD(role) if role in DOWNLOADS else K.Resource + RENAME(role)).build()])
          for c, reg, role in LINES if role in DOWNLOADS or role in CREATED],

        # each list's lines, in register order: the role's resource at the register, a run of its toggle, or its stand-in
        *[FRB.GraphInherit(LIST(c), graph, line, partFilter = lambda d, modType, ini, c = c:
                               FRB.Ranges.createFull() if d.sectionName == RENAME("CommandListSanhua" + c.capitalize() + "Textures") else FRB.Ranges.createEmpty())
          for c, reg, role in LINES for graph, line in [(ROLE(c, role), reg), (TOGGLE(c, reg, role), K.Run), (STAND_IN(c, role), reg)]],

        # the stand-ins' files: Sanhua's game texture downloaded, or a texture made
        *[FRB.ResRegCollect({LIST(c): lambda key: True for c in SLOTS},
                            {"download": FRB.FileDownloadCreate((0, "download", role), {DOWNLOAD(role):
                                FRB.FileDownloadCreate.File(file = FRB.DownloadTools.fixedFileName(SOURCE, role[:1].upper() + role[1:], FRB.FileExt.DDS),
                                                            url = FRB.DownloadTools.url("WuWa", SOURCE, FOLDER(role), SOURCE, "Texture" + textureHash, FRB.FileExt.DDS))})},
                            resPredicates = {LIST(c): lambda key, value, d, role = role: value.strip() == DOWNLOAD(role) for c in SLOTS})
          for role, textureHash in DOWNLOADS.items()],
        *[FRB.ResRegCollect({LIST(c): lambda key: True for c in SLOTS},
                            {"created": FRB.TexCreate((0, "created", role), "", creator,
                                                      files = {K.Resource + RENAME(role): FRB.IniNamingTools.getModSuffixedName(role, K.RemapTex, TARGET) + FRB.FileExt.DDS})},
                            resPredicates = {LIST(c): lambda key, value, d, role = role: value.strip() == K.Resource + RENAME(role) for c in SLOTS})
          for role, creator in CREATORS.items()],

        # each component onto its target slot (two onto one slot: another file); the hash-only objects onto themselves (the
        # shape keys in the mod's own file only); the fix's own graphs into every file
        FRB.GraphGroupRemap({**{(0, "", c): [(0, TARGET, slot, RENAME)] for c, slot in SLOTS.items()},
                             (0, "", "boneData"): [(0, "", "boneData", RENAME, True)],
                             (0, "", "shapekeyOffsets"): [(0, "", "shapekeyOffsets", RENAME, False)],
                             (0, "", "shapekeyScale"): [(0, "", "shapekeyScale", RENAME, False)],
                             (0, "", "watch"): [(0, TARGET, "watch", RENAME)],
                             **{graph: [graph + (lambda name: name, True)]
                                for graph in [SECTIONS, *[LIST(c) for c in SLOTS], *[(0, "download", role) for role in DOWNLOADS],
                                              *[(0, "created", role) for role in CREATED],
                                              *TOGGLES, *dict.fromkeys(REF(role) for _, role in ROLES)]}}),

        # each target slot's numbers, in every file
        FRB.GraphGroupEdit([{(TARGET, slot): [FRB.RegNewVals({key: str(table.get([TARGET, "", slot], TO, False))
                                                              for key, table in [(K.MatchFirstIndex, T.indices), (K.MatchIndexCount, T.indexCounts),
                                                                                 (K.WWMIVgOffset, T.vgOffsets), (K.WWMIVgCount, T.vgCounts)]})]
                             for slot in TARGET_SLOTS}], everyGroup = True),

        # the hashes onto SanhuaExorcist's, and the shape keys' dispatch size
        FRB.GraphGroupEdit([{**{(TARGET, slot): [FRB.RegAssetRemap({K.Hash: (S.hashes, K.HashNotFound)}, TARGET, SOURCE, FROM, TO)] for slot in TARGET_SLOTS},
                             **{("", obj): [FRB.RegAssetRemap({K.Hash: (S.hashes, K.HashNotFound),
                                                               **{K.WWMIShapeKeyChecksum + s: (S.shapeKeyChecksums, FRB.WWMINames.ChecksumNotFound) for s in ("", "_batch0", "_batch1")}},
                                                              TARGET, SOURCE, FROM, TO),
                                            FRB.RegNewVals({K.WWMIShapeKeyDispatchSizeYOriginalBatch + n: DISPATCH for n in "01"})]
                                for obj in ["boneData", "shapekeyOffsets", "shapekeyScale"]},
                             (TARGET, "watch"): [FRB.RegAssetRemap({K.Hash: (S.hashes, K.HashNotFound)}, TARGET, SOURCE, FROM, TO)]}], everyGroup = True),

        # each SanhuaExorcist slot nothing of the mod lands on: skipped, its bones still merged
        *[FRB.GraphCreate((0, TARGET, "hide" + slot), z3 = Z3, onlyWith = [(TARGET, s) for s in TARGET_SLOTS], onlyWithout = (TARGET, slot), sectionsOf = lambda group, slot = slot, n = slot[len("component"):]: [
            FRB.SectionText(Z3, K.TextureOverride + TARGET + slot.capitalize() + K.Remap + "Hide")
               .prefix("; nothing of the mod is drawn through " + TARGET + "'s " + TARGET_LABELS[slot] + " slot: the skin's own geometry is skipped and its bones still merged")
               .keys([(K.Hash, T.hashes.get([TARGET, "vb0"], TO, False)), (K.MatchFirstIndex, str(T.indices.get([TARGET, "", slot], TO, False))),
                      (K.MatchIndexCount, str(T.indexCounts.get([TARGET, "", slot], TO, False))), (K.WWMIObjectDetected, "1")])
               .open(K.WWMIModEnabled).key(K.Local + " " + K.WWMIStateId + "_" + n).open(K.WWMIStateId + "_" + n + " != " + K.WWMIStateId)
               .keys([(K.WWMIStateId + "_" + n, K.WWMIStateId), (K.WWMIVgOffset, str(T.vgOffsets.get([TARGET, "", slot], TO, False))),
                      (K.WWMIVgCount, str(T.vgCounts.get([TARGET, "", slot], TO, False))), (K.Run, RENAME("CommandListMergeSkeleton"))]).close()
               .open(K.Resource + "MergedSkeleton !== " + K.Null).key(K.Handling, K.Skip).close().close().build()])
          for slot in TARGET_SLOTS],

        # the blend, through Sanhua's vertex group row onto SanhuaExorcist's, in every file
        FRB.ResRegCollect({(0, TARGET, slot): "vb4" for slot in TARGET_SLOTS},
                          {"blend": FRB.VGRemapBlendReplace((0, "", "blend"), S, FROM, TO, row = S.getVGRemap(TARGET, FROM, TO),
                                                            elements = lambda srcPath: FRB.BlendFile.byteElements(4))},
                          resPredicates = {(0, TARGET, slot): FRB.ResRegCollect.skipsKeywords() for slot in TARGET_SLOTS}, everyGroup = True),
        FRB.GraphGroupRemap({(0, "", "blend"): [(0, "", "blend", lambda name: name, True)]}),

        # the parser's texture graphs are not written; and nothing of Sanhua found means nothing is written
        FRB.GraphRemove([*[ROLE(c, role) for c, _, role in LINES], *[STAND_IN(c, role) for c, _, role in LINES],
                         *[(0, FRB.IniGraphModObjKeywords.Texture, role) for _, _, role in LINES], (0, FRB.IniGraphModObjKeywords.Texture, "")]),
        FRB.GraphGroupRemove(onlyWithout = [(TARGET, slot) for slot in TARGET_SLOTS] + [(TARGET, "watch")]),
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

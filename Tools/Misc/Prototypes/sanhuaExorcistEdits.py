"""A WWMI pair's parser and fixer written explicitly from the bound API: the parser as its steps, then the fixer as
its edits -- the acceptance test of the WWMI refactor, mirroring WWMIParser / WWMIFixer step for step. Every step is
written out here rather than hidden behind a template, so what each step really costs is visible, and every place
the API cannot say something is marked `GAP:`.

    import sanhuaExorcistEdits as E
    E.use(FRB.ModTypeId.Chisa, FRB.ModTypeId.ChisaParfait)     # the pair; Sanhua -> SanhuaExorcist by default
    E.makeParser(iniFile, modTypeId)                           # -> GIMIParser

The compiled makeWWMIParser / makeWWMIFixer for the pair is the oracle.
"""
import os
import sys
import types

API = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "Anime Game Remap (for all users)", "api"))
if API not in sys.path:
    sys.path.insert(0, API)
import src.py.FixRaidenBoss2 as FRB


# ======================================================================================================== data
# The configs the compiled parser and fixer are built from -- read off the source's own builder rows
# (IniParseBuilder.factoryOf / IniFixBuilder.factoryOf), the lookup the library builds them with -- read the way
# WWMIParser and WWMIFixer read them.

def use(sourceId, targetId):
    """The pair the parser and fixer below are for: its two configs and what is read off them"""
    global SOURCE_ID, TARGET_ID, PARSE, FIX, SOURCE, VERSION, SLOT_PREFIX, SLOT_HASH, TEX_REG, HASH_ONLY, FACTS, ROLES
    global REGISTER_ROLES, TARGET, TARGET_VERSION, PLAN, REGISTERS, SHAPEKEY_DISPATCH, SLOT_PASSES, PASS_VERTEX_SHADERS
    global CREATED, FALLBACKS, DOWNLOADS, BLEND_REG, TARGET_LABELS, BARE_PARSE, BARE_FIX
    SOURCE_ID, TARGET_ID = sourceId, targetId
    SOURCE = FRB.ModTypeIdTools.getName(SOURCE_ID)
    TARGET = FRB.ModTypeIdTools.getName(TARGET_ID)
    # the two configs as the pair's builder rows give them -- the pair's CHOICES; the game data is filled in from the
    # tables at a run's versions (fillParse / fillFix), first at the configs' own
    sourceType = FRB.ModTypes[SOURCE].value
    BARE_PARSE = FRB.WWMIParserConfig.ofFactory(sourceType.iniParseBuilder.factoryOf(SOURCE))
    BARE_FIX = FRB.WWMIFixerConfig.ofFactory(sourceType.iniFixBuilder.factoryOf(SOURCE, TARGET))
    VERSION = BARE_PARSE.version
    SLOT_PREFIX = FRB.WWMINames.SlotPrefix
    SLOT_HASH = FRB.IniKeywords.Vb0
    TEX_REG = FRB.IniKeywords.TexRegPrefix
    BLEND_REG = FRB.IniKeywords.WWMIBlendReg
    fillParse(None)
    fillFix(None, None)
    # the target's version when the .ini names none (WWMIFixer::toVersion): the config's, else her newest game version
    TARGET_VERSION = FIX.version or str(FRB.WWMIFixerConfig.newestVersionOf(FRB.ModTypes[TARGET].value))


def fillParse(version):
    """The parser's config with its game data at 'version' (WWMIParser's constructor: WWMIParserConfig::withGameData)"""
    global PARSE, HASH_ONLY, FACTS, ROLES, REGISTER_ROLES
    PARSE = BARE_PARSE.withGameData(FRB.ModTypes[SOURCE].value, version)
    HASH_ONLY = [tuple(entry) for entry in PARSE.hashOnlyObjs]

    # the source's texture facts: the roles by hash and by register, the thumbprints, the downloads
    FACTS = PARSE.textures
    ROLES = set(FACTS.roles.values())
    REGISTER_ROLES = {slot: dict(table) for slot, table in FACTS.registerRoles.items()}


def fillFix(fromVersion, toVersion):
    """The fixer's config with its game data at the run's versions (WWMIFixer::readGameData); None: the config's own"""
    global FIX, PLAN, REGISTERS, SHAPEKEY_DISPATCH, SLOT_PASSES, PASS_VERTEX_SHADERS, CREATED, FALLBACKS, DOWNLOADS
    global TARGET_LABELS
    fix = BARE_FIX
    if fromVersion is None:
        fromVersion = FRB.Version.parse(fix.sourceVersion or fix.version) if (fix.sourceVersion or fix.version) else None
    FIX = fix.withGameData(FRB.ModTypes[SOURCE].value, FRB.ModTypes[TARGET].value, fromVersion, toVersion)

    # source component -> the target slot it draws through, and the role bound at each register of its target pass
    PLAN = {component: entry.slot for component, entry in FIX.plan.items()}
    REGISTERS = {component: [(binding.reg, binding.role) for binding in entry.bindings] for component, entry in FIX.plan.items()}
    SHAPEKEY_DISPATCH = FIX.shapeKeyDispatchSize

    # the target's pixel shaders per draw slot and the vertex shaders each is drawn with (the lists are gated on those)
    SLOT_PASSES = [list(passes) for passes in FIX.slotPasses]
    PASS_VERTEX_SHADERS = {shader: list(vss) for shader, vss in FIX.passVertexShaders.items()}

    # what stands in when the mod has no file for a role: the texture the fix invents, else the source's own game texture
    CREATED = [(created.role, created.colour, created.size) for created in FIX.createdTextures]
    FALLBACKS = dict(FIX.sourceTextures.fallbackTextures)
    DOWNLOADS = (FIX.sourceTextures.downloadGameFolder, FIX.sourceTextures.downloadCharFolder,
                 FIX.sourceTextures.downloadVersionFolder, FIX.sourceTextures.downloadPrefix)
    TARGET_LABELS = dict(FIX.targetLabels)


use(FRB.ModTypeId.Sanhua, FRB.ModTypeId.SanhuaExorcist)
WATCH = ("", "watch")
TEXTURE = FRB.IniGraphModObjKeywords.Texture

# the rows the parser files, under the names the compiled parser files them by
N = FRB.WWMINames
OVERRIDE_ROWS, ROLE_ROWS, TOGGLE_ROWS, CARRIED_ROWS = N.TextureOverridesRows, N.TextureRolesRows, N.TextureTogglesRows, N.CarriedRows


# ===================================================================================================== helpers

def graphId(obj):
    return (0, obj[0], obj[1])


def isFixSection(name):
    return name.lower().endswith(FRB.IniKeywords.RemapFix.lower())


def hashTypesOf(modType):
    """{hash type: {hash, ...}} of the character, over every version HashData files"""
    result = {}
    for names in modType.hashes.repo.toNestedDict().values():
        for hashType, value in names.get(modType.name, {}).items():
            result.setdefault(hashType, set()).add(value.lower())
    return result


def byWritten(sectionNames):
    at = {name: i for i, name in enumerate(sectionNames)}
    return lambda name: at.get(name, len(at))


def asWritten(rows):
    return sorted(rows, key = lambda row: (row.position, row.line))


def iniPathOf(rel, folder):
    return os.path.normpath(os.path.join(folder, rel.strip().replace("\\", os.sep)))


def rowsByRoot(rows, roots):
    """A collect's rows found in a section's OWN parts, when the section is a root of the row's graph"""
    result = {}
    for row in rows:
        if row.graphId[1:] in roots.get(row.section, []):
            result.setdefault(row.section, []).append(row)
    return result


# ====================================================================================================== parser

def makeParser(iniFile, modTypeId, onlyThisFile = False):
    """'onlyThisFile': the parser of ANOTHER .ini of the mod, whose rows this character's parser reads -- the same parse,
    with only the resource data read off its rows (and so none of its own linked files)"""
    modType = FRB.ModTypes[SOURCE].value
    version = iniFile.fromVersion if iniFile.fromVersion is not None else FRB.Version.parse(VERSION)
    # a linked file's parser (onlyThisFile) keeps its parent's config, already filled (WWMIParser::readOther)
    if not onlyThisFile:
        fillParse(version)
    hashTypes = hashTypesOf(modType)

    # ---- step 1: the mod objects, and the classifier that sorts sections into them ----
    # her slots (as many as ModType.getComponentCount counts), the hash-only objects, a texture graph per role HashData
    # names, the catch-all texture graph, and the sections that only watch her slot hash
    slots, slotIndex = [], {}
    for n in range(modType.getComponentCount(SLOT_PREFIX, version)):
        obj = ("", SLOT_PREFIX + str(n))
        slots.append(obj)
        slotIndex[obj] = obj
    hashOnly = {hashType: ("", name) for hashType, name in HASH_ONLY}
    modObjs = slots + list(hashOnly.values())
    for hashType in hashTypes:
        role = FRB.WWMITextureFacts.roleOfHashType(hashType, ROLES)
        if role is not None and hashType not in hashOnly:
            hashOnly[hashType] = (TEXTURE, role)
            if (TEXTURE, role) not in modObjs:
                modObjs.append((TEXTURE, role))
    modObjs += [(TEXTURE, ""), WATCH]
    watchHashes = hashTypes.get(SLOT_HASH, set())

    classifier = FRB.GIMISectionClassifier(hashOnly, modType.hashes, {SLOT_HASH: slotIndex}, modType.indices, version,
                                           [SOURCE, None], [SOURCE, None, None])

    def watches(section, part):
        hashes = part.getVals(FRB.IniKeywords.Hash)
        return bool(hashes) and hashes[0].strip().lower() in watchHashes and not (section.firstVal(FRB.IniKeywords.MatchFirstIndex) is not None)

    def isUnknownTextureOverride(name, section, part):
        return (name.startswith(FRB.IniKeywords.TextureOverride) and not isFixSection(name)
                and bool(part.getVals(FRB.IniKeywords.Hash)) and not (section.firstVal(FRB.IniKeywords.MatchFirstIndex) is not None))

    def target(parser, name, section, disjoint, part, kvps):
        if kvps is None:
            return []
        if watches(section, part):
            return [WATCH]
        objs = classifier.classify(name, section, kvps)
        if not objs and isUnknownTextureOverride(name, section, part):
            objs = [(TEXTURE, "")]
        return objs

    parser = FRB.GIMIParser(iniFile, modObjs = modObjs, objTargetFuncs = [target], disjointModObjs = False,
                            keysToTrack = {FRB.IniKeywords.Hash, FRB.IniKeywords.MatchFirstIndex, FRB.IniKeywords.MatchIndexCount},
                            modTypeId = modTypeId)

    # ---- step 2: THE STANDARD ROWS (IniParseRows), off the parser's global graph: a copy of it put in the groups; the
    # resource sections a line binds anywhere built as a graph (ResIdentity: nothing renamed, no models) and their
    # `filename`; every section's declarations, with [Constants]' mesh vertex count; every key of every section; both
    # graphs taken out again; and every key of each slot ----
    GLOBAL, RESOURCES = (0, "global", ""), (0, "resource", "")
    anyKey = lambda key: True
    globalGraph = FRB.GraphCreate(GLOBAL, graph = lambda: parser.globalGraph)
    resourceSections = FRB.ResRegCollect({GLOBAL: anyKey},
                                         {"": FRB.ResIdentity(RESOURCES, createResModel = False, fileOf = parser.resourceNamed)},
                                         resPredicates = {GLOBAL: lambda reg, value, iterData: parser.resourceNamed(value) != ""})
    files = FRB.GraphCollect({RESOURCES: [FRB.IniKeywords.Filename]}, name = FRB.IniParseRows.Resources)
    declared = FRB.GraphCollect({GLOBAL: [FRB.IniKeywords.Filename, FRB.IniKeywords.Type, FRB.IniKeywords.Format,
                                          FRB.IniKeywords.Stride, FRB.IniKeywords.Global + " " + FRB.IniKeywords.WWMIMeshVertexCount]},
                                name = FRB.IniParseRows.Declared)
    sections = FRB.GraphCollect({GLOBAL: anyKey}, name = FRB.IniParseRows.Sections)
    removeGraphs = FRB.GraphRemove([RESOURCES, GLOBAL])
    slotRows = FRB.GraphCollect({graphId(obj): anyKey for obj in modObjs if PARSE.slotOf(obj) is not None}, name = FRB.IniParseRows.Objs)

    # ---- step 3: each texture override's hash, `this` and `match_priority`; the roles the .ini states; and the toggles ----
    overrides = FRB.GraphCollect({graphId(obj): [FRB.IniKeywords.Hash, FRB.IniKeywords.This, FRB.IniKeywords.MatchPriority]
                                  for obj in modObjs},
                                 name = OVERRIDE_ROWS)
    roleKeys, toggleKeys = {}, {}
    for obj in modObjs:
        if obj[0] == TEXTURE:
            if obj[1] != "":
                roleKeys[graphId(obj)] = [FRB.IniKeywords.This]
            toggleKeys[graphId(obj)] = lambda key: key.strip().lower() == FRB.IniKeywords.This or key.strip().lower().startswith(TEX_REG)
        elif PARSE.slotOf(obj) in REGISTER_ROLES:
            # a slot's registers, as an author spells them
            roleKeys[graphId(obj)] = lambda key, regs = list(REGISTER_ROLES[PARSE.slotOf(obj)]): any(
                FRB.IniNamingTools.sameKey(key, reg) for reg in regs)
    roles = FRB.GraphCollect(roleKeys, name = ROLE_ROWS, withConditions = True)
    toggles = FRB.GraphCollect(toggleKeys, name = TOGGLE_ROWS)
    # what a linked file sets that a fix carries as it is (IniParseData.LinkedLine): RabbitFX's glow map on a slot's draw,
    # with its condition -- read from a file parsed in its only-this-file mode
    carried = FRB.GraphCollect({graphId(obj): (lambda key: FRB.IniNamingTools.sameKey(key, FRB.RabbitFX.GlowMapKey))
                                for obj in (modObjs if onlyThisFile and (ROLES or REGISTER_ROLES) else []) if PARSE.slotOf(obj) is not None},
                               name = CARRIED_ROWS, withConditions = True)

    # ---- the variables made from the rows: WWMI's readings of them, the config's decision functions (WWMIParserConfig),
    # each handed the parser's graphs and their sections in its order ----
    targets = lambda: list(parser.sectionTargets.items())

    # step 5: the resource data (IniParseData.resourceDataOf), by WWMI's rules (WWMIParserConfig.resourceDataRules), and
    # whether the mod replaces textures by hash (WWMIParserConfig.replacesTexturesByHash), made from the rows
    def resourceData(rows):
        rules = PARSE.resourceDataRules(targets(), iniFile.getSectionNames(), onlyThisFile, print)
        start = FRB.IniParseData.ResourceData()
        start.replacesTexturesByHash = PARSE.replacesTexturesByHash(rows[OVERRIDE_ROWS] if OVERRIDE_ROWS in rows else [], targets())
        return FRB.IniParseData.resourceDataOf(rules, rows, start)

    # step 6: each slot's file per texture role (IniParseData.roleFilesOf), by WWMI's rules (WWMIParserConfig.roleFileRules),
    # made from the rows and the resource data; each linked .ini is read by this parser
    def textureRoleFiles(rows):
        rules = PARSE.roleFileRules(targets(), iniFile.getSectionNames(), modType, iniFile.file or "", iniFile.folder)
        if rules is None:
            return None
        rules.log = print

        # the files LINKED to this one, each parsed by this character's parser in its only-this-file mode -- kept alive
        # while roleFilesOf reads them
        readers = []
        if rules.objs and rules.iniPath:
            for linked in iniFile.linkedInis():
                sibling = iniFile.siblingOf(linked, {modTypeId})
                reader = makeParser(sibling, modTypeId, onlyThisFile = True)
                reader.parse()
                readers.append((sibling, reader))
                rules.linked = [*rules.linked, FRB.IniParseData.RoleFileRules.Linked(reader.parseData, sibling.folder,
                                                                                      os.path.basename(linked.file), linked.file)]
        return FRB.IniParseData.roleFilesOf(rules, rows)

    # step 4: which numbering the blend's bone ids are in (WWMIParserConfig.skeletonNumbering), from the slot rows and the
    # resource rows -- the version, or None
    def skeletonNumbering(objRows, resourceRows):
        return PARSE.skeletonNumbering(objRows or [], resourceRows or [], targets(), iniFile.folder, print)

    # step 7: the resource data with the draw splits' draws added (WWMIParserConfig.splitDraws), read with the role files
    def drawSplits(data, roleData):
        return PARSE.splitDraws(data, roleData, targets(), iniFile.file or "", iniFile.folder, print)

    # ==== THE STEPS: the edits that read the mod, then the variables made from their rows -- all run on the first ask
    # of the parse data ====
    skeleton = FRB.GraphVarApply([FRB.IniParseRows.Objs, FRB.IniParseRows.Resources], skeletonNumbering, FRB.IniParseData.BlendVersion)
    resourceInputs = [FRB.IniParseRows.Objs, FRB.IniParseRows.Resources, FRB.IniParseRows.Sections, TOGGLE_ROWS, OVERRIDE_ROWS]
    resources = FRB.GraphVarApply(resourceInputs, lambda *inputs: resourceData(FRB.GraphEditVariables.of(resourceInputs, list(inputs))),
                                  FRB.IniParseData.ResourceDataVar)
    roleInputs = [FRB.IniParseRows.Sections, FRB.IniParseRows.Declared, OVERRIDE_ROWS, ROLE_ROWS, FRB.IniParseRows.Objs,
                  FRB.IniParseData.ResourceDataVar]
    roleFiles = FRB.GraphVarApply(roleInputs, lambda *inputs: textureRoleFiles(FRB.GraphEditVariables.of(roleInputs, list(inputs))),
                                  FRB.IniParseData.RoleFilesVar)
    splits = FRB.GraphVarApply([FRB.IniParseData.ResourceDataVar, FRB.IniParseData.RoleFilesVar], drawSplits,
                               FRB.IniParseData.ResourceDataVar)
    parser.deferCommandEdits = True
    parser.commandEdits = ([globalGraph, resourceSections, files, declared, sections, removeGraphs, slotRows,
                            overrides, roles, toggles, carried] + ([] if onlyThisFile else [skeleton])
                           + [resources] + ([] if onlyThisFile else [roleFiles, splits]))
    return parser


# ======================================================================================================= fixer



def slotNumbers(modType, version):
    """Each draw slot's (match_first_index, match_index_count, vg_offset, vg_count), off the library's rows -- as many
    slots as ModType.getComponentCount counts"""
    slots = []
    for n in range(modType.getComponentCount(SLOT_PREFIX, version)):
        slot = SLOT_PREFIX + str(n)
        slots.append((modType.indices.get([modType.name, "", slot], version, False), modType.getIndexCount(slot, "", version),
                      modType.getVGOffset(slot, "", version), modType.getVGCount(slot, "", version)))
    return slots


# THE STAGES the fix is written in, in the order the fixer's list runs them (WWMIFixer's edit order)
# the kinds of the fix's own sections, each written as a step in this order (WWMIFixer's declareFixSectionKinds)
FIX_SECTION_KINDS = ["textureLists", "glowLists", "linkedState", "declaredFiles", "skeletonSections", "blendRemapSections",
                     "bypassSections", "hideSections", "stashResources", "passTags", "sharedMeshes", "hiddenMeshes",
                     "shaderDraws", "rabbitFXPinLists"]
# the kinds of per-object edit, each a step in this order (WWMIFixer's ObjEditKinds)
OBJ_EDIT_KINDS = ["textureListCalls", "sharedBypass", "modTexLines", "additions", "rerouteFilters", "removedLines",
                  "rabbitFXPins", "slotValues", "hashes", "dispatchSize", "shapeKeyLists", "afterShaders", "markLines",
                  "overlays", "rerouteShaders", "offPasses"]
STAGES = ["copies", "toggles", "slotRemap", *OBJ_EDIT_KINDS, "blend", "rebind", "fixSections", *FIX_SECTION_KINDS,
          "shaderFiles", "fixFiles", "latch",
          "texEdits"]


def drawnComponents(parser, toModName):
    """WHAT THE MOD DRAWS (WWMIFixerConfig::drawnOf, which WWMIFixer::readMod calls), as the parser read it: present and
    silent components, their draws, the reroutes, legacy, own meshes, RabbitFX, own shaders, overlays, mark lines, the
    host's `$object_detected` reset and the watching sections -- or why the mod is not remapped (`error`)"""
    source = FRB.ModTypes[SOURCE].value
    fromVersion = fixVersions(parser)[0]
    sourceVb0 = (source.hashes.get([source.name, SLOT_HASH], fromVersion, False) or "").lower()
    result = FIX.drawnOf(parser.parseData, list(parser.sectionTargets.items()), source.getComponentCount(SLOT_PREFIX, fromVersion),
                         source.name, sourceVb0, toModName)
    if not result.error:
        for note in result.notes:
            print(note)
    return result


def fixVersions(parser):
    """The fix's two versions (WWMIFixer::fromVersion / toVersion). The source's: the .ini's own, else the one
    sourceVersionByVb0 files the mod's vb0 under (its first slot section's), else the source's own -- not the target's,
    under a reverse-then-forward lookup"""
    data = parser.parseData
    fromVersion = parser._iniFile.fromVersion
    if fromVersion is None and FIX.sourceVersionByVb0:
        sections = FRB.IniParseRows.Sections
        byVb0 = next((FIX.sourceVersionByVb0[hash.value.strip().lower()] for section in data.collectedSections(sections)
                      if data.firstCollected(sections, section, FRB.IniKeywords.MatchFirstIndex) is not None
                      for hash in [data.firstCollected(sections, section, FRB.IniKeywords.Hash)]
                      if hash is not None and hash.value.strip().lower() in FIX.sourceVersionByVb0), None)
        fromVersion = FRB.Version.parse(byVb0) if byVb0 else None
    if fromVersion is None:
        fromVersion = FRB.Version.parse(FIX.sourceVersion or FIX.version)
    toVersion = parser._iniFile.toVersion
    return fromVersion, (toVersion if toVersion is not None else FRB.Version.parse(TARGET_VERSION))


def decide(parser, toModName):
    """THE FIX'S DECISIONS (WWMIFixer::decide), as data: what the mod draws, which file each component goes in, what each
    role resolves to (reroutes reading their source's files), the game's own textures, the texture edits' targets, what a
    linked .ini carries, RabbitFX's flags, the texture lists, the mod's own texture lines, the blend width and the lines
    each section drops"""
    source = FRB.ModTypes[SOURCE].value
    data, folder, iniPath = parser.parseData, parser._iniFile.folder, parser._iniFile.file

    def noted(result):
        for note in getattr(result, "notes", []):
            print(note)
        return result

    drawn = drawnComponents(parser, toModName)
    if drawn.error:
        return {"drawn": drawn}               # the mod is not remapped: writeStages gives up (or carries a watcher)
    present = set(drawn.present)
    decided = {"drawn": drawn, "fileGroups": noted(FIX.fileGroups(present, source.name))}
    roles = decided["roleResources"] = noted(FIX.roleResources(data, drawn.drawn, drawn.rabbitFX, source.name, toModName,
                                                                 folder))
    roles = decided["resolvedRoles"] = FIX.withReroutes(roles, drawn.rerouteFrom)
    defaults = decided["gameDefaults"] = noted(FIX.gameDefaults(data, roles, present, drawn.rerouteFrom, drawn.ownShader, source.name))
    decided["texEditTargets"] = noted(FIX.texEditTargets(data, roles, present, drawn.ownShader, iniPath))
    decided["linkedLines"] = noted(FIX.linkedLines(data, roles, present, parser._iniFile.linkedInis(), source.name, toModName,
                                                   folder))
    flags = decided["rabbitFXFlags"] = FIX.rabbitFXFlags(data, roles, defaults.toggled, drawn.present, drawn.rerouteFrom,
                                                         iniPath, folder)
    lists = decided["textureLists"] = FIX.toggleLists(
        FIX.textureLists(data, roles, defaults, present, drawn.rerouteFrom, drawn.ownShader, drawn.overlaid), data, defaults,
        source.name, toModName, iniPath)
    decided["modTexLines"] = FIX.modTexLines(data, roles, lists, present, drawn.rerouteFrom, drawn.ownShader)
    width = decided["blendWidth"] = noted(FIX.blendWidth(source.getVGRemap(toModName, *fixVersions(parser))))
    decided["removedLines"] = FIX.removedLines(
        width.targetPast256, {component for group in decided["fileGroups"].groups for component in group}, set(flags))
    return decided


def makeFixer(parser, toModName, modTypeId):
    """The fixer, as ONE LIST (WWMIFixer::buildList): the fix's decisions are data (decide), and the list is the edits
    that write them, each built with its data and named after its stage (STAGES), in the order they run. A variant of the
    fix rewrites the list -- an edit's data, a step added, removed or replaced"""
    data = parser.parseData
    fillFix(*fixVersions(parser))
    stages = writeStages(parser, toModName, modTypeId, decide(parser, toModName))
    steps = []
    for name in STAGES:
        for edit in stages.get(name, []):
            edit.name = name
            steps.append(edit)

    fixer = FRB.GIMIFixer(parser, graphGroupEdits = steps, modsToFix = [toModName])
    fixer.keepAlive(stages)               # what the stages' edits borrow lives as long as the fixer
    # a copy carries the fix's own sections and none of the mod's texture overrides live: the mod's own file keeps
    # serving the mod on its own character
    fixer.copyHiddenSectionNames = {section for section in data.collectedSections(FRB.IniParseRows.Sections)
                                    if section.startswith(FRB.IniKeywords.TextureOverride)}
    fixer.copyPreamble = FIX.copyPreamble
    return fixer


def writeStages(parser, toModName, modTypeId, fixVars):
    """The stages that write the fix, from the decisions in 'fixVars' (decide): stage name -> its edits"""
    source, target = FRB.ModTypes[SOURCE].value, FRB.ModTypes[TARGET].value
    data = parser.parseData
    fromVersion, toVersion = fixVersions(parser)
    # the target object a source component's sections are remapped onto: its slot's, or -- for a second section on a
    # shared slot in the mod's own file (SourceComponent::sameFileSection) -- one of its own, so the two claimants do
    # not collide in one group (WWMIFixer::targetObjOf)
    targetObj = lambda component: (toModName, SLOT_PREFIX + str(PLAN[component])
                                   + ("_" + str(component) if FIX.plan[component].sameFileSection else ""))
    rename = lambda name: FRB.IniNamingTools.getRemapFixName(name, toModName)

    # ---- what the mod draws: the "drawn" step's ----
    drawnInfo = fixVars["drawn"]
    present, silent, drawRanges = drawnInfo.present, drawnInfo.silent, drawnInfo.drawRanges
    rerouteFrom, reroutedDraws = drawnInfo.rerouteFrom, drawnInfo.reroutedDraws
    # each component's slot object (a reroute's is its source component's)
    objOf = {component: ("", SLOT_PREFIX + str(rerouteFrom.get(component, component))) for component in present}

    # A COMPONENT DRAWN WITH ITS OWN SHADER (SourceComponent::shader; WWMIFixer::readShaders, after the roles are read):
    # its bindings are its plan entry's, the shader's own registers in their place -- unless the mod hands the
    # component's maps to RabbitFX and the shader yields to it
    usesRabbitFX = lambda component: component in present and FRB.RabbitFX.Name in data.libraries(objOf[component])
    shaderUsed = lambda component: FIX.shaderDraws(component, usesRabbitFX(component))
    rerouteShaders, markLines, overlays = {}, {}, {}
    bindingsOf = lambda component: FIX.bindingsOf(component, shaderUsed(component))

    # ---- a mod from before WWMI's merged skeleton (no slot section carries a vg_offset) holds each component's OWN
    # bone indices in its blend: lifted through the source's per-component vg maps, and the fix supplies the merged
    # skeleton it lacks -- or, with no maps to lift them with, left alone (WWMIFixer::readMod) ----
    # a file the fix leaves alone, or only copies sections of: its edits are the first stage's, the others none
    onlyStage = lambda edits: {**{stage: [] for stage in STAGES}, "copies": edits}
    giveUp = lambda: onlyStage([FRB.GraphGroupRemove()])

    # THE MOD'S SECTIONS ON ANOTHER HASH, BY EDITS (WWMIFixer::copyOntoHash): each section of the parser's graph 'obj'
    # copied into a graph of its own (GraphCopy), renamed to the fix's name (GraphRename), its `hash` set (RegNewVals)
    # and taken into the fix's sections under its comment -- in this call's place among them when 'inPlace'
    def copyOntoHash(variables, obj, sections, hash, commentOf, inPlace):
        copies, takes, edits = [], [], {}
        for section in sections:
            name = rename(section)
            copyObj = ("", name)
            copies.append(FRB.GraphCopy((0, *obj), (0, *copyObj), lambda owner, part, i, s = section: owner.name == s))
            edits[copyObj] = [FRB.GraphRename(lambda old, s = section, n = name: n if old == s else old),
                              FRB.RegNewVals({FRB.IniKeywords.Hash: hash})]
            takes.append(FRB.GraphVarTake(variables, "fixSections", copyObj, [name], commentOf(section), inPlace))
        return copies + ([FRB.GraphGroupEdit([edits])] + takes if sections else [])

    targetVb0 = target.hashes.get([target.name, SLOT_HASH], toVersion, False)
    if drawnInfo.error:
        # A FILE THAT ONLY WATCHES THE SOURCE (WWMIFixer::buildWatcher): each watching section, as the author wrote it,
        # under the fix's name on the target's vb0 -- copied before the removal of every group, nothing drawn
        watching = drawnInfo.watching
        if watching and targetVb0:
            watchZ3, watchSections = FRB.Z3Context(), FRB.GraphEditVariables()
            copies = copyOntoHash(watchSections, WATCH, watching, targetVb0, lambda section: "", False)
            write = FRB.GraphCreate((0, "", "fixSections"))
            write.sectionsOf = lambda group: watchSections.sections("fixSections").list()
            write.z3 = watchZ3
            print("only watches " + SOURCE + ", so its " + str(len(watching)) + " watching section(s) are carried onto "
                  + toModName + "'s vb0 and nothing is drawn")
            return {**onlyStage([*copies, FRB.GraphGroupRemove(), write]), "_kept": [watchZ3, watchSections]}
        print(drawnInfo.error + " -- so this .ini is left alone")
        return giveUp()
    legacy = drawnInfo.legacy

    # A COMPONENT DRAWN FROM THE MOD'S OWN MESH FILES (WWMIFixerConfig.OwnMesh, read by drawnOf): its section runs a
    # `<sharedResourcesList>.<suffix>` list binding an index, position and blend buffer of its own
    ownMeshes = {component: (mesh.list, mesh.blendResource, mesh.blendFile, mesh.positionFile, mesh.vgOffset)
                 for component, mesh in drawnInfo.ownMeshes.items()}

    # the groups (the "fileGroups" step): the first claimant of a target slot stays in the mod's own .ini, the next goes
    # to a copy
    groups = [list(group) for group in fixVars["fileGroups"].groups]

    # ---- step 1: the lines each remapped section drops (the "removedLines" step), and whether the TARGET's skeleton
    # passes what an 8-bit blend index names (the "blendWidth" step) ----
    targetPast256 = fixVars["blendWidth"].targetPast256
    # each value matched with or without `ref`
    removeOf = {component: FRB.RegRemove([(removal.reg, FRB.RegRemove.valueStartsWith(removal.valuePrefix)
                                                        if removal.valuePrefix else None) for removal in lines])
                for component, lines in fixVars["removedLines"].items()}

    # ---- step 2: the hashes and the shape-key checksums, onto the target's ----
    checksum = FRB.IniKeywords.WWMIShapeKeyChecksum
    assetRemap = FRB.RegAssetRemap({FRB.IniKeywords.Hash: (source.hashes, FRB.IniKeywords.HashNotFound),
                                    checksum: (source.shapeKeyChecksums, N.ChecksumNotFound),
                                    checksum + "_batch0": (source.shapeKeyChecksums, N.ChecksumNotFound),
                                    checksum + "_batch1": (source.shapeKeyChecksums, N.ChecksumNotFound)},
                                   toModName, source.name, fromVersion, toVersion)
    dispatchKey = FRB.IniKeywords.WWMIShapeKeyDispatchSizeYOriginalBatch
    dispatch = FRB.RegNewVals({dispatchKey + "0": SHAPEKEY_DISPATCH, dispatchKey + "1": SHAPEKEY_DISPATCH})

    # ---- step 3: each target slot's numbers, over the source's ----
    targetSlots = slotNumbers(target, toVersion)

    # the merged skeleton the fix supplies for a legacy mod or a target past 256 bones (WWMIFixer::runtimeSkeleton);
    # the merge is latched on $object_detected where the host's [Present] resets it
    skeleton = FRB.WWMISections.Skeleton()
    skeleton.blendRemapBones = fixVars["blendWidth"].bones
    skeleton.name = rename
    skeleton.cb4Hash = (target.hashes.get([target.name, "cb4"], toVersion, False) or "").lower()
    skeleton.mergedSkeletonSlots = FIX.mergedSkeletonSlots
    skeleton.remapped = targetPast256
    skeleton.latched = drawnInfo.hostResetsDetected
    skeleton.currentPoseInCb3Only = FIX.currentPoseInCb3Only
    skeleton.blendRemapForward = rename(FRB.IniKeywords.Resource + "BlendRemapForwardBuffer")
    skeleton.slotWindows = [(vgOffset, vgCount) for _, _, vgOffset, vgCount in targetSlots]
    slotValues = {}
    for slot in sorted(set(PLAN[c] for group in groups for c in group)):
        first, count, vgOffset, vgCount = targetSlots[slot]
        slotValues[slot] = FRB.RegNewVals({FRB.IniKeywords.MatchFirstIndex: first, FRB.IniKeywords.MatchIndexCount: count,
                                           FRB.IniKeywords.WWMIVgOffset: vgOffset, FRB.IniKeywords.WWMIVgCount: vgCount})

    # an own mesh, settled once the blend width is known (WWMIFixer::settleOwnMeshes): a component the plan drops is drawn
    # by nothing; only onto a target past 256 bones (or a legacy mod) is its skeleton the fix's; its blend is a fix file,
    # remapped into the target's local ids (WWMIBlend.componentBlend) through the row and its anchor chains
    ownMeshes = {component: mesh for component, mesh in ownMeshes.items() if component in FIX.plan}
    if ownMeshes and not (targetPast256 or legacy):
        for component, mesh in ownMeshes.items():
            print("source component " + str(component) + " draws from its own mesh (" + mesh[0]
                  + "), which this fix remaps only onto a target past 256 bones")
        ownMeshes = {}
    ownMeshFiles = {}
    ownMeshBlendResource = lambda component: rename(FRB.IniKeywords.Resource + FRB.WWMINames.OwnMeshSkin + str(component))
    if ownMeshes:
        rowVersion = fromVersion if legacy or data.blendVersion() is None else FRB.Version.parse(data.blendVersion())
        ownRow = source.getVGRemap(toModName, rowVersion, toVersion)
        if ownRow is None:
            ownMeshes = {}
        else:
            anchored = dict(ownRow.remap)
            for root, members in FIX.anchorChains.items():
                if root in anchored:
                    for member in members:
                        anchored[member] = anchored[root]
            folder = parser._iniFile.folder
            for component, (meshList, blend, blendFile, positionFile, vgOffset) in list(ownMeshes.items()):
                layout = FRB.WWMIBlend.layoutOf(iniPathOf(blendFile, folder), iniPathOf(positionFile, folder))
                if layout is None or component not in FIX.sourceVgMaps:
                    print("could not remap the own-mesh blend of source component " + str(component)
                          + (": its blend and position buffers do not agree on a vertex count" if layout is None
                             else ": the config carries no sourceVgMaps for it"))
                    del ownMeshes[component]
                    continue
                outInfluences = max(layout.influences, FIX.targetInfluences)
                blendRel = blendFile.replace("\\", "/")
                blendFolder = blendRel[:blendRel.rfind("/")] if "/" in blendRel else ""
                outFile = "/".join(part for part in (blendFolder, toModName + FRB.IniKeywords.Remap + "OwnMeshBlend" + str(component) + ".buf") if part)

                def makeBytes(component = component, blendFile = blendFile, positionFile = positionFile, vgOffset = vgOffset,
                              outInfluences = outInfluences):
                    bytes_, unmapped = FRB.WWMIBlend.componentBlend(iniPathOf(blendFile, folder), iniPathOf(positionFile, folder),
                                                                     vgOffset, list(FIX.sourceVgMaps[component]), anchored, outInfluences)
                    if unmapped:
                        print("source component " + str(component) + "'s own-mesh blend names " + str(len(unmapped))
                              + " bone(s) the row does not map; they read local 0")
                    return bytes_

                ownMeshFiles[ownMeshBlendResource(component)] = FRB.FileAddCreate.File(
                    [(FRB.IniKeywords.Type, FRB.IniKeywords.Buffer), (FRB.IniKeywords.Format, FRB.IniKeywords.FormatR8UInt),
                     (FRB.IniKeywords.Stride, str(outInfluences * 2))], outFile, makeBytes = makeBytes)

    # ---- step 4: every source slot's sections onto its target slot, named after the target; two claimants of one
    # slot collide, which makes the copies. A hidden component goes nowhere. The hash-only objects come along:
    # the bone-data marker into every file, the shape-key overrides once ----
    remap = {}
    for component in sorted(present):
        if component in rerouteFrom:
            continue                    # a target of its source's graph, below
        remap[graphId(("", SLOT_PREFIX + str(component)))] = [(0, *targetObj(component), rename)] if component in PLAN else []
        # ...and each reroute of it: the same sections again under their own names, so the two do not collide
        for reroute, origin in sorted(rerouteFrom.items()):
            if origin == component:
                remap[graphId(("", SLOT_PREFIX + str(component)))].append(
                    (0, *targetObj(reroute), lambda name, tag = N.Reroute + str(reroute): rename(name + tag)))
    for component in silent:
        remap[graphId(("", SLOT_PREFIX + str(component)))] = []
    counterpart = lambda obj: not obj[0] and obj[1] in FIX.meshCounterparts
    hashOnlyObjs = [obj for obj in parser.modObjs if obj[0] != TEXTURE and PARSE.slotOf(obj) is None and obj != WATCH and not counterpart(obj)]
    for obj in hashOnlyObjs:
        copies = 1 if obj[1] in (FRB.WWMINames.ShapeKeyOffsetsObj, FRB.WWMINames.ShapeKeyScaleObj) else len(groups)
        remap[graphId(obj)] = [(0, *obj, rename)] * copies
    # the texture overrides and the watching sections are the texture step's (step 5); remapped nowhere until then,
    # or each would be rendered into the fix block again under the mod's own section names
    remap.setdefault(graphId(WATCH), [])
    slotRemap = FRB.GraphGroupRemap(remap)

    # ---- step 5: each component's texture list -- the role of each register its target pass reads ----
    # (layer 1, the parser) which file the mod has for a role; (layer 3, one rule) else what stands in for it;
    # (layer 4, edits) a CommandList gated on the pass, run right after the slot section's match keys
    z3 = FRB.Z3Context()
    fixSections = FRB.GraphEditVariables()
    # a section among the fix's own, filed under its KIND (TemplateFixer::addFixSection): each kind is written as a step
    # of its own after the sections of none, in WWMIFixer's order (declareFixSectionKinds)
    addFixSection = lambda kind, section: fixSections.sections(kind).add(section)
    tags = FRB.ShaderFilterTags()
    allPasses = SLOT_PASSES + [list(byPass) for byPass in FIX.sharedMeshes.values()] + [list(byPass) for byPass in FIX.extraPassRegs.values()]
    taggedFor = lambda shader: PASS_VERTEX_SHADERS[shader] if PASS_VERTEX_SHADERS else [shader]
    tags.addGiven([vs for passes in allPasses for shader in passes for vs in taggedFor(shader)], dict(FIX.filterIndices))

    def passCondition(passes):
        return tags.condition("vs" if PASS_VERTEX_SHADERS else "ps", list(dict.fromkeys(tag for p in passes for tag in taggedFor(p))))

    # layers 1 and 3, WHAT EACH ROLE RESOLVES TO (WWMIFixerConfig.roleResources): the mod's file per (role, component)
    # and the resource binding it -- the mod's own, or a RemapRef the fix declares -- and the stand-ins: the texture the
    # fix invents, else the source's own game texture, downloaded; a flat texture the config leaves to the game gets none
    remapNames = [source.name, toModName]
    planned = [component for component in sorted(present) if component in PLAN and component not in rerouteFrom]
    drawn = {component for component in present if component not in rerouteFrom}
    roles = fixVars["roleResources"]
    resourceOfSlotRole, resourceOfRole, fileOfRole = dict(roles.ofSlotRole), dict(roles.ofRole), dict(roles.fileOfRole)
    declaredName = {declared.file: declared.resource for declared in roles.declared}
    leftToGame = set(roles.leftToGame)
    fixDownloads, fallbackFileOfRole, fallbackResource = {}, {}, {}
    for role, download in roles.downloads.items():
        fixDownloads[download.resource] = FRB.FileDownloadCreate.File()
        fixDownloads[download.resource].file, fixDownloads[download.resource].url = download.file, download.url
        fallbackFileOfRole[role], fallbackResource[role] = download.file, download.resource

    # each reroute reads its source component's files
    for reroute, origin in rerouteFrom.items():
        for (role, component), resource in list(resourceOfSlotRole.items()):
            if component == origin:
                resourceOfSlotRole.setdefault((role, reroute), resource)
    # which components their own shader draws, which carry mark lines and which an overlay is drawn over: drawnOf's
    # (SourceComponent::shader, markLines, overlay -- a RabbitFX mod keeps its look)
    for component in sorted(drawnInfo.ownShader):
        rerouteShaders[component] = FIX.plan[component].shader
    for component in sorted(drawnInfo.markLines):
        markLines[component] = list(FIX.plan[component].markLines)
    for component in sorted(drawnInfo.overlaid):
        overlays[component] = FIX.plan[component].overlay

    # a component drawn with its own shader reads the download itself, not a texture edit of it
    def resourceFor(role, component):
        if (role, component) in resourceOfSlotRole:
            return resourceOfSlotRole[(role, component)]
        if component in rerouteShaders and role in fallbackResource:
            return fallbackResource[role]
        return resourceOfRole.get(role)

    sharedResourceFor = lambda role: next((resource for component in sorted(present)
                                           for resource in [resourceFor(role, component)] if resource), None)

    # THE SOURCE'S OWN GAME TEXTURE AS THE DEFAULT, where the mod's file reaches only some of a component's draws
    # (WWMIFixerConfig.gameDefaults): 1. the draws a section makes before it binds anything, 2. a role bound only under a
    # toggle that reaches none of the section's draws, 3. a role bound with one `this` under a toggle (off, the game's)
    resolved, defaults = fixVars["resolvedRoles"], fixVars["gameDefaults"]
    gameDownloads = {}     # role -> (resource, download)

    # the source's own game texture of a role, downloaded (WWMIFixerConfig.downloadOf) -- declaring it binds nothing
    def addGameDownload(role, download = None):
        if not DOWNLOADS[1] or role not in FALLBACKS:
            return False
        if role not in gameDownloads:
            download = download or FIX.downloadOf(role[0].upper() + role[1:] + N.GameSuffix, FALLBACKS[role])
            file = FRB.FileDownloadCreate.File()
            file.file, file.url = download.file, download.url
            gameDownloads[role] = (download.resource, file)
            fixDownloads[download.resource] = file
        return True

    for role, download in defaults.downloads.items():
        addGameDownload(role, download)
    toggledGame = dict(defaults.toggled)
    ownRows = lambda section: data.sectionRows(FRB.IniParseRows.Sections, section)

    # THE STATE A LINKED FILE KEEPS FOR 'vars' (IniFile.varState), copied into the fix under the variables' new names
    # (GraphVarRename) -- a section other than [Constants] and [Present] under a name of the fix's own too. 'vars' grows to
    # the state's closure, and a variable the file never declares is declared, as 3DMigoto starts it, at 0
    def copyLinkedState(linked, vars, suffix):
        state = linked.varState(vars)
        vars.update(state.vars)

        renames = {var: var + suffix for var in vars}
        for section in state.takeSections():
            FRB.GraphVarRename.renameIn(section, renames, z3)
            if section.name.lower() not in (FRB.IniKeywords.Constants.lower(), FRB.IniKeywords.Present.lower()):
                section.name = rename(section.name)
            addFixSection("linkedState", section)

        undeclared = [(FRB.IniKeywords.Global + " $" + renames[var], "0") for var in sorted(state.undeclared)]
        if undeclared:
            addFixSection("linkedState", FRB.SectionText(z3, FRB.IniKeywords.Constants).keys(undeclared).build())

    # WHAT A LINKED .ini SETS ON THIS FILE'S DRAWS (the "linkedLines" step, WWMIFixerConfig.linkedLines): each linked
    # file's state copied into the fix under its variables' new names, and the lines carried onto this file's draws
    linked = fixVars["linkedLines"]
    linkedByPath = {file.file: file for file in parser._iniFile.linkedInis()}
    for state in linked.states:
        copyLinkedState(linkedByPath[state.ini], set(state.vars), state.suffix)
    for declared in linked.declared:
        declaredName[declared.file] = declared.resource
    linkedLines = {component: [(line.condition, line.key, line.role, line.value) for line in lines]
                   for component, lines in linked.lines.items()}

    # A COMPONENT THAT KEEPS RABBITFX (SourceComponent::rabbitFX): the mod's RabbitFX lines stay, and RabbitFX's routing
    # is pinned to what its SetTextures sets on the source's own draw (WWMIFixerConfig.rabbitFXFlags)
    pinList = lambda component: rename(FRB.IniKeywords.CommandList + "RabbitFXPin" + str(component))
    rabbitFXFlags = dict(fixVars["rabbitFXFlags"])

    # THE TEXTURE LISTS, LINE BY LINE (the "textureLists" steps): each line `<reg> = <value>`, or `run =` the copy of the
    # mod's own toggle the "toggleLists" step put in its place
    textureLists = fixVars["textureLists"]
    listsOfRole, rawLists = {}, set()
    for toggle in textureLists.toggles:
        listsOfRole.setdefault((toggle.role, toggle.reg), set()).add(toggle.name)
        if toggle.raw:
            rawLists.add(toggle.name)

    def linesOf(kind, component, passNum = 0):
        """A planned list's lines as the fix writes them, and the (role, register) pairs it binds"""
        planned = textureLists.find(kind, component, passNum)
        lines = [] if planned is None else planned.lines
        return ([(FRB.IniKeywords.Run, line.run) if line.run else (line.reg, line.value) for line in lines],
                [(line.role, line.reg) for line in lines if line.role])

    # `CommandList<Source>Component<N><infix>Textures<suffix>`, under the fix's name (WWMIFixer::textureListName)
    textureListName = lambda component, infix = "", suffix = "": rename(
        FRB.IniKeywords.CommandList + source.name + "Component" + str(component) + infix + "Textures" + suffix)

    # a texture list among the fix's sections -- its lines under 'condition' -- and the (role, register) pairs it binds,
    # which is what the texture edits' collects edit by (WWMIFixer::addTextureList)
    def addTextureList(name, condition, lines, bound, component):
        addFixSection("textureLists", FRB.SectionText(z3, name).open(condition).keys(lines).close().build())
        if component in rerouteShaders:
            rawLists.add(name)
        for role, reg in bound:
            listsOfRole.setdefault((role, reg), set()).add(name)

    listOf, passListsOf, gameListOf, glowListOf = {}, {}, {}, {}
    for component in sorted(present):
        if component not in PLAN:
            continue
        raw = component in rerouteShaders
        passGuard = passCondition(SLOT_PASSES[PLAN[component]])
        lines, bound = linesOf("main", component)
        if lines:
            listOf[component] = textureListName(component)
            addTextureList(listOf[component], passGuard, lines, bound, component)

        # what a linked .ini sets on this draw: RabbitFX's glow map for every pass, and a texture under a condition at the
        # register this component's plan binds its role at, on the passes the texture list binds it on
        if linkedLines.get(component):
            glowList = rename(FRB.IniKeywords.CommandList + source.name + "Component" + str(component) + N.Glow)
            glow = FRB.SectionText(z3, glowList)
            for condition, key, role, value in linkedLines[component]:
                if key:
                    (glow.key(key, value) if not condition else glow.open(condition).key(key, value).close())
            gated = False
            for condition, key, role, value in linkedLines[component]:
                reg = FIX.registerOf(bindingsOf(component), role) if not key else None
                if reg is None:
                    continue
                if not gated:
                    glow.open(passGuard)
                    gated = True
                (glow.key(reg, value) if not condition else glow.open(condition).key(reg, value).close())
            if gated:
                glow.close()
            addFixSection("glowLists", glow.build())
            glowListOf[component] = glowList

        # generation 0: the draws this section makes before it binds anything rendered with the source's own game
        # textures, so that is what they are given
        gameLines, gameBound = linesOf("game", component)
        if gameLines:
            gameListOf[component] = textureListName(component, N.GameSuffix)
            addTextureList(gameListOf[component], passGuard, gameLines, gameBound, component)

        # a slot's OTHER passes bind the same art at DIFFERENT registers (WWMIFixerConfig::extraPassRegs): each its own
        # list, gated on that pass, run before the draw -- and a pass the slot's draw must not be re-issued on
        # (extraPassNoDraw) nulls it there
        for n, passName in enumerate(FIX.extraPassRegs.get(PLAN[component], {})):
            passLines, passBound = linesOf("pass", component, n)
            if passLines:
                passList = textureListName(component, "", N.Pass + str(n))
                passListsOf.setdefault(component, []).append(passList)
                addTextureList(passList, passCondition([passName]), passLines, passBound, component)

    # the resources the fix declares, after the lists (WWMIFixer::buildAppended), in the order they were named
    for file, name in declaredName.items():
        addFixSection("declaredFiles",
            FRB.SectionText(z3, name).key(FRB.IniKeywords.Filename, os.path.relpath(file, parser._iniFile.folder)).build())

    # a remap-only copy of the mod's texcoord buffer, NaN halves cleared and a U in the [1, 2) tile folded back unless its
    # triangle straddles the seam (WWMIFixerConfig::cleanTexcoords; TexcoordFile.clean) -- only when the copy differs,
    # bound before the draw and written with the fix's files (WWMIFixer::buildTexcoordCopy)
    # where the fix writes its own buffers: beside the mod's Blend.buf, else beside the .ini (WWMIFixer::meshFolder_)
    blendRel = data.resourceFile(FRB.IniKeywords.WWMIBlendBuffer)
    meshFolder = os.path.dirname(blendRel.replace("\\", "/")) if blendRel else ""
    modFile = lambda name: "/".join(part for part in (meshFolder, name) if part)

    positionFile = data.resourceFile(FRB.IniKeywords.WWMIPositionBuffer)
    countRow = data.firstCollected(FRB.IniParseRows.Declared, FRB.IniKeywords.Constants,
                                   FRB.IniKeywords.Global + " " + FRB.IniKeywords.WWMIMeshVertexCount)
    vertexCount = int(countRow.value.strip()) if countRow is not None else 0
    inIni = lambda rel: iniPathOf(rel, parser._iniFile.folder)

    # the blend line the fix writes: N ids + N weights -- the mod's own N, unless the config sets the target's
    influences = FIX.targetInfluences or (FRB.WWMIBlend.influencesOf(inIni(blendRel), vertexCount, inIni(positionFile))
                                          if blendRel and positionFile else 4)
    blendRemap = FRB.WWMISections.BlendRemap()
    blendRemap.name = rename
    blendRemap.blendFile = FRB.IniNamingTools.getFixedBlendFile(blendRel, toModName) if blendRel else ""
    blendRemap.remapFile = lambda which: modFile(FRB.IniNamingTools.getRemapElementName("", N.BlendRemap + which, toModName) + ".buf")
    blendRemap.blendStride = influences * 2
    blendRemap.influences = influences
    blendRemap.blendReg = BLEND_REG

    texcoordResource, fixFiles, fixFileRegs = None, {}, set()
    # each own mesh's remapped blend (settled above)
    fixFiles.update(ownMeshFiles)
    if ownMeshFiles:
        fixFileRegs.add(BLEND_REG)
    declared = lambda key: data.firstCollected(FRB.IniParseRows.Declared, FRB.IniKeywords.WWMITexcoordBuffer, key)
    if FIX.cleanTexcoords and declared(FRB.IniKeywords.Filename) is not None:
        texcoordPath = os.path.normpath(os.path.join(parser._iniFile.folder, declared(FRB.IniKeywords.Filename).value.strip()))
        strideRow, formatRow = declared(FRB.IniKeywords.Stride), declared(FRB.IniKeywords.Format)
        stride = int(strideRow.value.strip()) if strideRow is not None else 16
        indexRel = data.resourceFile(FRB.IniKeywords.WWMIIndexBuffer)

        def cleaned():
            texcoord = FRB.TexcoordFile(texcoordPath, FRB.TexcoordFile.halfElements(stride))
            keep = (texcoord.straddling(FRB.IbFile(os.path.join(parser._iniFile.folder, indexRel)).getIndices())
                    if indexRel else [False] * len(texcoord.us()))
            return texcoord, texcoord.clean(keep)

        texcoord, stats = cleaned()
        if texcoord.us() and (stats.cleared or stats.folded):
            texcoordResource = rename(FRB.IniKeywords.Resource + N.TexcoordNoNaN)
            fixFiles[texcoordResource] = FRB.FileAddCreate.File(
                [(FRB.IniKeywords.Type, FRB.IniKeywords.Buffer),
                 (FRB.IniKeywords.Format, formatRow.value.strip() if formatRow is not None else "DXGI_FORMAT_R16G16_FLOAT"),
                 (FRB.IniKeywords.Stride, str(stride))],
                modFile(toModName + FRB.IniKeywords.Remap + "Texcoord.buf"),
                makeBytes = lambda: cleaned()[0].data)
            fixFileRegs.add(FRB.IniKeywords.WWMITexcoordReg)

    # the mod's shape keys in the TARGET's order (WWMIFixerConfig::shapeKeyOrder): the game drives them by slot, and two
    # skins number them differently. The mod's three buffers, checked before anything binds a copy of them
    shapeKeyResources = [FRB.IniKeywords.WWMIShapeKeyOffsetBuffer, FRB.IniKeywords.WWMIShapeKeyVertexIdBuffer,
                         FRB.IniKeywords.WWMIShapeKeyVertexOffsetBuffer]
    shapeKeyRegs = ["cs-t33", "cs-t0", "cs-t1"]
    shapeKeyFiles = {}
    if FIX.shapeKeyOrder:
        for resource in shapeKeyResources:
            declaredKey = lambda key, resource = resource: data.firstCollected(FRB.IniParseRows.Declared, resource, key)
            valueOf = lambda key, otherwise: declaredKey(key).value.strip() if declaredKey(key) is not None else otherwise
            filename = declaredKey(FRB.IniKeywords.Filename)
            if filename is not None and filename.value.strip():
                shapeKeyFiles[resource] = (filename.section, filename.value.strip(), valueOf(FRB.IniKeywords.Type, FRB.IniKeywords.Buffer),
                                           valueOf(FRB.IniKeywords.Format, ""), valueOf(FRB.IniKeywords.Stride, ""))

        def shapeKeyOrderFile(resource):
            name = resource[len(FRB.IniKeywords.Resource):]
            name = name[:-len("Buffer")] if name.endswith("Buffer") else name
            folder = os.path.dirname(shapeKeyFiles[resource][1].replace("\\", "/"))
            return "/".join(part for part in (folder, FRB.IniNamingTools.getRemapElementName("", name, toModName) + ".buf") if part)

        shapeKeyPaths = lambda: FRB.WWMIShapeKeys.Files(*[inIni(shapeKeyFiles[resource][1]) for resource in shapeKeyResources])
        if len(shapeKeyFiles) != len(shapeKeyResources):
            shapeKeyFiles = {}
        elif FRB.WWMIShapeKeys.checkReorder(shapeKeyPaths(), list(FIX.shapeKeyOrder)):
            print("this mod's shape keys are left in " + source.name + "'s order ("
                  + FRB.WWMIShapeKeys.checkReorder(shapeKeyPaths(), list(FIX.shapeKeyOrder)) + ")")
            shapeKeyFiles = {}

    # the zero shape-key stream: a buffer of zeros, one line a vertex, bound before every draw
    # (WWMIFixerConfig::zeroShapeKeyStream)
    zeroResource = None
    if FIX.zeroShapeKeyStream and vertexCount > 0:
        zeroResource = rename(FRB.IniKeywords.Resource + N.ShapeKeyZero)
        zeroSize = vertexCount * N.ShapeKeyStreamStride
        fixFiles[zeroResource] = FRB.FileAddCreate.File(
            [(FRB.IniKeywords.Type, FRB.IniKeywords.Buffer), (FRB.IniKeywords.Format, "DXGI_FORMAT_R32G32B32_FLOAT"),
             (FRB.IniKeywords.Stride, str(N.ShapeKeyStreamStride))],
            modFile(FRB.IniNamingTools.getRemapElementName("", N.ShapeKeyZero, toModName) + ".buf"),
            makeBytes = lambda: bytes(zeroSize))
        fixFileRegs.add(FRB.IniKeywords.WWMIShapeKeyStreamReg)

    # ...and the reordered copies of the shape keys, declared as the mod declares its own
    if shapeKeyFiles:
        reordered = []

        def shapeKeyBytes(which):
            if not reordered:
                reordered.append(FRB.WWMIShapeKeys.reordered(shapeKeyPaths(), list(FIX.shapeKeyOrder)))
            return (reordered[0].offsets, reordered[0].vertexIds, reordered[0].vertexOffsets)[which]

        for which, resource in enumerate(shapeKeyResources):
            _, _, kind, form, keyStride = shapeKeyFiles[resource]
            keys = [(FRB.IniKeywords.Type, kind)] + ([(FRB.IniKeywords.Format, form)] if form else []) \
                + ([(FRB.IniKeywords.Stride, keyStride)] if keyStride else [])
            fixFiles[rename(resource)] = FRB.FileAddCreate.File(keys, shapeKeyOrderFile(resource),
                                                                makeBytes = lambda which = which: shapeKeyBytes(which))
        fixFileRegs.update(shapeKeyRegs)

    # the invented textures: made where the fix's lists bind them, in every group the slot remap made
    # (WWMIFixer::fixFileEdits: one TexCreate per group, collected off every graph of that group)
    kept = []

    # a collect of one create edit from every graph of the group, wherever a register binds a resource 'named' accepts:
    # at 'regs', or at any register when 'regs' is empty (WWMIFixer::collectNamed)
    def collectNamed(group, graphGroups, create, named, regs, extraResources = None):
        # a collect at named registers meets its graphs in the order of the C++ map it keys them in; one at any register,
        # in the group's own order
        graphs = [(group, *obj) for obj in graphGroups[group].graphs]
        graphs = sorted(graphs) if regs else graphs
        kept.append(create)
        return FRB.ResRegCollect({graph: (list(regs) if regs else (lambda key: True)) for graph in graphs}, {"fixFile": create},
                                 resPredicates = {graph: (lambda key, value, data: named(value.strip())) for graph in graphs},
                                 extraResources = extraResources)

    def makeTextures(group, graphGroups):
        edits = []
        # the fix's own files, then the downloads, then the invented textures (WWMIFixer::fixFileEdits' order)
        if fixFiles:
            edits.append(collectNamed(group, graphGroups, FRB.FileAddCreate((group, "", "fixFiles"), fixFiles),
                                      lambda name: name in fixFiles, sorted(fixFileRegs)))
        if fixDownloads:
            edits.append(collectNamed(group, graphGroups, FRB.FileDownloadCreate((group, "", "fixDownloads"), fixDownloads),
                                      lambda name: name in fixDownloads, []))
        for role, colour, size in CREATED:
            resource = resourceOfRole[role]
            make = FRB.TexCreate((group, "", "fixTexture" + resource), "", FRB.CppTexCreator(size, size, colour))
            make.files = {resource: FRB.IniNamingTools.getModSuffixedName(role, FRB.IniKeywords.RemapTex, toModName) + ".dds"}
            edits.append(collectNamed(group, graphGroups, make, lambda name, r = resource: name == r, []))

        return edits
    madeTextures = [FRB.IterGraphGroupEdit(makeTextures)]

    # the frame's skeleton remap, latched on $object_detected where the host's [Present] resets it (WWMIFixer::latchEdits)
    latchEdits = []
    if targetPast256 and skeleton.latched:
        detected = FRB.IniKeywords.WWMIObjectDetected

        def latchTemplate(setsDetected):
            text = FRB.SectionText(z3, "").open(detected + " == 0") \
                .key(FRB.IniKeywords.Run, rename("CommandListRemapMergedSkeleton")).close()
            return (text.key(detected, "1") if setsDetected else text).build()

        merges = {rename(N.MergeWindowList)} | {FRB.WWMISections.mergeListName(skeleton, slot)
                                                       for slot in range(len(targetSlots))}
        runsMerge = lambda section: any(value.strip() in merges for part in section.parts if hasattr(part, "getVals")
                                        for value in part.getVals(FRB.IniKeywords.Run))
        latched = set()     # a section two graphs share is latched once, as its rendered text was

        def once(name, section):
            if name in latched or not runsMerge(section):
                return False
            latched.add(name)
            return True

        beforeDetected = FRB.RegIfTemplateAdd(latchTemplate(False), FRB.RegIfTemplateAdd.Position.BeforeTopKey, detected)
        beforeDetected.value = "1"
        beforeIf = FRB.RegIfTemplateAdd(latchTemplate(True), FRB.RegIfTemplateAdd.Position.BeforeFirstIf)
        for edit in (beforeDetected, beforeIf):
            edit.everySection = True
            edit.sectionFilter = once
        kept.extend([beforeDetected, beforeIf])

        def latchGroup(group, graphGroups):
            latched.clear()
            objs = list(graphGroups[group].graphs)
            return [FRB.GraphGroupEdit([{} for _ in range(group)] + [{obj: [edit] for obj in objs}])
                    for edit in (beforeDetected, beforeIf)]
        latchEdits.append(FRB.IterGraphGroupEdit(latchGroup))

    # which resources each texture edit applies to (WWMIFixerConfig.texEditTargets): the role's resolved resource and
    # every other variant the mod's toggles bind it as -- unless another role's edit of another kind claimed it first
    edited = fixVars["texEditTargets"]
    editTargets = {role: set(resources) for role, resources in edited.targets.items()}

    # the source's own game textures the defaults download are edited too (WWMIFixer::planGameResource)
    for role, (resource, _) in gameDownloads.items():
        editTargets.setdefault(role, set()).add(resource.lower())

    # what a filter factory is told about the mod (WWMIFixerConfig::TexEditContext): a role bound only under a toggle
    # has no one file of the mod's
    folder = parser._iniFile.folder
    inFolder = lambda rel: os.path.normpath(os.path.join(folder, rel))
    context = FRB.WWMIFixerConfig.TexEditContext()
    context.iniFolder = folder
    context.indexFile = inFolder(data.resourceFile(FRB.IniKeywords.WWMIIndexBuffer) or "")
    context.positionFile = inFolder(data.resourceFile(FRB.IniKeywords.WWMIPositionBuffer) or "")
    context.texcoordFile = inFolder(data.resourceFile(FRB.IniKeywords.WWMITexcoordBuffer) or "")
    context.drawRanges = drawRanges
    # the characters and the run's versions, so a filter reads the asset tables at them (Lynae's props code table)
    context.source, context.target = source, target
    context.fromVersion, context.toVersion = fromVersion, toVersion
    toggledRoles = {role for role, _ in toggledGame}
    context.fileOfRole = {**{role: inFolder(file) for role, file in fallbackFileOfRole.items()},
                          **{role: file for role, file in fileOfRole.items() if role not in toggledRoles}}

    # each edit: a TexReplace collected wherever the role's register binds one of its resources -- on the remapped
    # slot sections (the mod's own lines) and, in the fix's sections, by one of the role's own lists; once for every
    # group (WWMIFixer::addTexEditCollects)
    texEdits = []
    for edit in FIX.texEdits:
        if edit.makeFilter is None or edit.role not in editTargets:
            continue
        texFilter = edit.makeFilter(context)
        if texFilter is None:
            continue

        componentsAt = {}
        for component in sorted(present):
            if component not in PLAN or component in rerouteShaders:
                continue
            bindings = bindingsOf(component) + [binding for bindings in FIX.extraPassRegs.get(PLAN[component], {}).values()
                                                for binding in bindings]
            for binding in bindings:
                if binding.role == edit.role:
                    componentsAt.setdefault(binding.reg, set()).add(component)

        name = edit.role[0].upper() + edit.role[1:] + edit.name
        editor = FRB.TexReplace((0, "", "texEdit" + name), FRB.TexEditor([texFilter], compress = edit.compress),
                                "resourceRemapTexEdit", resSubType = name)
        targets = editTargets[edit.role]
        isTarget = lambda key, value, data, targets = targets: value.strip().lower() in targets
        inRoleList = lambda key, value, data, role = edit.role, isTarget = isTarget: (
            data.sectionName in listsOfRole.get((role, key), set()) and data.sectionName not in rawLists
            and isTarget(key, value, data))

        # by register, as the C++ map orders them: what the collect meets first is declared first
        srcRegs, predicates = {}, {}
        for reg, components in sorted(componentsAt.items()):
            srcRegs.setdefault((0, "", "fixSections"), []).append(reg)
            for component in sorted(components):
                srcRegs.setdefault((0, *targetObj(component)), []).append(reg)
                predicates[(0, *targetObj(component))] = isTarget
        predicates[(0, "", "fixSections")] = inRoleList
        texEdits.append(FRB.ResRegCollect(srcRegs, {"texEdit": editor}, resPredicates = predicates, everyGroup = True))
        kept.append(editor)

    # ---- step 6: the shared-resource pair made symmetric (WWMIFixer::buildBypasses): every buffer the mod's override
    # list binds and its cleanup list does not put back is parked in a resource of the fix's before it is replaced, and
    # the cleanup list runs the fix's list that puts them back ----
    sharedList, cleanupList = FRB.WWMINames.OverrideSharedResourcesList, FRB.WWMINames.CleanupSharedResourcesList
    restoreList = FRB.WWMISections.restoreListName(toModName)
    bypasses = FRB.WWMISections.bypasses(data, FRB.IniParseRows.Sections, sharedList, cleanupList, toModName)

    def onlyIn(names):
        names = {name.lower() for name in names}
        return lambda sectionName: sectionName.lower() in names

    # every name a mod's command list can carry in the remapped graphs: its own, the fix's, and each reroute copy's
    listNames = lambda name: [name, rename(name)] + [rename(name + N.Reroute + str(reroute)) for reroute in sorted(rerouteFrom)]

    bypassEdits = []
    if bypasses:
        for reg, bypass in bypasses:
            bypassEdits.append(FRB.RegPartEdit(
                FRB.RegAnchorAdd([reg], lambda key, value, b = bypass, r = reg: [(b, FRB.IniKeywords.Ref + " " + r)],
                                 FRB.RegAnchorAdd.Placement.Before, None, True, True),
                onlyIn(listNames(sharedList))))
        bypassEdits.append(FRB.RegPartEdit(FRB.RegAdd([(FRB.IniKeywords.Run, restoreList)], True, True),
                                           onlyIn(listNames(cleanupList))))
        for section in FRB.WWMISections.bypassSections(z3, bypasses, restoreList):
            addFixSection("bypassSections", section)

    # ---- step 7: WWMI's own sections (WWMISections): each target slot nothing of the mod is drawn through skips the
    # skin's geometry and still merges the slot's bones; then the shader tags the texture lists are gated on ----
    drawnSlots = {PLAN[component] for group in groups for component in group}
    stashResource = lambda slot, reg: rename(FRB.IniKeywords.Resource + "Stash" + str(slot) + FRB.IniNamingTools.getRegTag(reg))
    for slot, (first, count, vgOffset, vgCount) in enumerate(targetSlots):
        if slot in drawnSlots:
            continue
        hide = FRB.WWMISections.Hide()
        hide.name = FRB.IniKeywords.TextureOverride + toModName + "Component" + str(slot) + FRB.IniKeywords.Remap + "Hide"
        hide.comment = ("; nothing of the mod is drawn through " + toModName + "'s " + TARGET_LABELS.get(slot, str(slot))
                        + " slot: the skin's own geometry is skipped and its bones still merged")
        hide.vb0Hash = target.hashes.get([target.name, SLOT_HASH], toVersion, False)
        hide.indexOffset, hide.indexCount, hide.vgOffset, hide.vgCount = first, count, vgOffset, vgCount
        hide.slot = str(slot)
        hide.mergeSkeletonList = rename("CommandListMergeSkeleton")
        hide.legacyMergeList = FRB.WWMISections.mergeListName(skeleton, slot) if legacy else ""
        hide.mergeWindowList = rename(N.MergeWindowList) if targetPast256 else ""
        # the game's own bindings on this slot, for a shader drawn on another that reads them (stashRegs) -- only when
        # a shader drawn for this mod binds a capture off the slot (Shader.stashed)
        stashed = FIX.stashRegs.get(slot, []) if any(stash.slot == slot for shader in rerouteShaders.values()
                                                     for stash in shader.stashed) else []
        hide.stashed = [(stashResource(slot, reg), FRB.IniKeywords.Ref + " " + reg) for reg in stashed]
        addFixSection("hideSections", FRB.WWMISections.hideSection(z3, hide))
        for reg in stashed:
            addFixSection("stashResources", FRB.SectionText(z3, stashResource(slot, reg)).build())
    if legacy or targetPast256:
        for section in FRB.WWMISections.skeletonSections(z3, skeleton):
            addFixSection("skeletonSections", section)
        if targetPast256:
            for section in FRB.WWMISections.blendRemapSections(z3, blendRemap):
                addFixSection("blendRemapSections", section)
    for i in range(len(tags.tags())):
        addFixSection("passTags", tags.section(i, rename("ShaderOverridePass" + str(i))))

    # the other meshes the character draws, by their own vb0 hash (WWMIFixerConfig::sharedMeshes): the geometry is
    # shared, so only the textures are rebound, per pass, with the file of the first component that has the role
    for meshNum, (meshHash, byPass) in enumerate(FIX.sharedMeshes.items()):
        mesh = FRB.SectionText(z3, rename(FRB.IniKeywords.TextureOverride + source.name + "SharedMesh" + str(meshNum)))
        mesh.key(FRB.IniKeywords.Hash, meshHash)
        drew = False
        for n, passName in enumerate(byPass):
            lines = linesOf("sharedMesh", meshNum, n)[0]
            if lines:
                mesh.open(passCondition([passName])).keys(lines).close()
                drew = True
        if drew:
            addFixSection("sharedMeshes", mesh.build())

    # the target's meshes the mod replaces, skipped while the mod is on (WWMIFixerConfig::hiddenMeshes)
    for i, hiddenHash in enumerate(FIX.hiddenMeshes):
        addFixSection("hiddenMeshes",
            FRB.SectionText(z3, rename(FRB.IniKeywords.TextureOverride + toModName + "HiddenMesh" + str(i)))
            .prefix("; a mesh of " + toModName + "'s own outfit the mod replaces: skipped")
            .key(FRB.IniKeywords.Hash, hiddenHash).open(FRB.IniKeywords.WWMIModEnabled)
            .key(FRB.IniKeywords.Handling, FRB.IniKeywords.Skip).close().build())

    # the mod's own sections on another mesh of the source, copied onto the target's mesh that plays the same part
    # (WWMIFixerConfig::meshCounterparts), in this place among the fix's sections
    copyEdits = []
    for part in FIX.meshCounterparts:
        hash = target.hashes.get([target.name, part], toVersion, False)
        if hash is None:
            print("the library files no '" + part + "' hash for " + toModName + ", so the mod's sections on " + SOURCE
                  + "'s are not carried over")
            continue
        copyEdits += copyOntoHash(fixSections, ("", part), parser.sectionTargets.get(("", part), []), hash.lower(),
                                  lambda section: section + " of the mod, on " + toModName + "'s counterpart of that mesh", True)
    copied = [(0, "", part) for part in FIX.meshCounterparts if ("", part) in parser.modObjs]
    if copied:
        copyEdits.append(FRB.GraphRemove(copied))

    # ---- A COMPONENT DRAWN WITH A PIXEL SHADER OF THE FIX'S OWN (SourceComponent::shader): each of its draws becomes a
    # `run =` of a list that draws it with the shader on the plan's passes and as it was on any other
    # (WWMISections::ShaderDraw); a component's draws marked after they are made (markLines), kept off some passes
    # (offPasses), or held back until other draws of the pass are made (Shader::after) ----
    shaderDraws = lambda component: sorted(reroutedDraws[component]) if component in reroutedDraws else sorted(set(drawRanges.get(component, [])))
    drawAt = lambda component, drawRange: shaderDraws(component).index(drawRange)
    shaderSection = lambda component, drawRange, kind = FRB.IniKeywords.CommandList, suffix = "": rename(
        kind + rerouteShaders[component].name + str(component) + N.Draw + str(drawAt(component, drawRange)) + suffix)
    markSection = lambda component, drawRange, kind = FRB.IniKeywords.CommandList, suffix = "": rename(
        kind + N.Mark + str(component) + N.Draw + str(drawAt(component, drawRange)) + suffix)
    overlaySection = lambda component, drawRange, kind = FRB.IniKeywords.CommandList, suffix = "": rename(
        kind + N.Overlay + str(component) + N.Draw + str(drawAt(component, drawRange)) + suffix)
    drawValue = lambda drawRange: str(drawRange[0]) + ", " + str(drawRange[1]) + ", 0"
    shaderFile = lambda shader, p = None: modFile(toModName + FRB.IniKeywords.Remap + shader.name
                                                  + (N.Pass + str(p) if p is not None else "") + ".hlsl")
    bufferResource = lambda shader, reg: rename(FRB.IniKeywords.Resource + shader.name + FRB.IniNamingTools.getRegTag(reg))
    afterLatch = lambda component: "$" + rename(N.DrawAfter + str(component) + N.Latch)
    awaited = lambda shader: ([] if shader.after is None else
                              [(shader.after.component, shader.after.draws)] * (shader.after.component in present)
                              + [wait for wait in shader.after.also if wait[0] in present])

    # a draw kept off the passes its offPasses names: a `run =` of a list that draws it only off them
    offLists = {}

    def offDraw(component, drawRange):
        draw = (FRB.IniKeywords.DrawIndexed, drawValue(drawRange))
        off = FIX.plan[component].offPasses if component in FIX.plan else []
        condition = tags.condition("vs" if PASS_VERTEX_SHADERS else "ps",
                                   list(dict.fromkeys(tag for p in off for tag in taggedFor(p))), True) if off else ""
        if not condition:
            return draw
        name = rename(FRB.IniKeywords.CommandList + N.OffPass + str(component) + N.Draw + str(drawRange[0]) + "_" + str(drawRange[1]))
        offLists[name] = (condition, draw[1])
        return (FRB.IniKeywords.Run, name)

    # a `run =` of a section for each draw it names, in place of the draw or right after it
    def drawRunAdd(sectionOf, placement):
        return FRB.RegPartEdit(FRB.RegAnchorAdd(
            [FRB.IniKeywords.DrawIndexed],
            lambda key, value: [(FRB.IniKeywords.Run, sectionOf[FRB.IniParseData.drawRangeOf(value, True)])] if FRB.IniParseData.drawRangeOf(value, True) in sectionOf else None,
            placement))

    drawEdits = {}          # kind -> component -> the edits on its draws (each kind a step, after the rest)
    afterDispatch = {}
    for component, shader in sorted(rerouteShaders.items()):
        for first, draws in awaited(shader):
            if first in afterDispatch:
                continue
            afterDispatch[first] = rename(FRB.IniKeywords.CommandList + N.DrawAfter + str(first))
            anchors = [cleanupList.lower(), rename(cleanupList).lower()]
            drawEdits.setdefault("afterShaders", {}).setdefault(first, []).append(FRB.RegPartEdit(FRB.RegAnchorAdd(
                [FRB.IniKeywords.Run],
                lambda key, value, dispatch = afterDispatch[first], latch = afterLatch(first): (
                    [(FRB.IniKeywords.Run, dispatch), (FRB.IniKeywords.Post + " " + latch, "0")]
                    if value.strip().lower() in anchors else None),
                FRB.RegAnchorAdd.Placement.Before)))
    markOf = {drawRange: markSection(component, drawRange) for component in sorted(markLines) for drawRange in shaderDraws(component)}
    for component in sorted(markLines):
        drawEdits.setdefault("markLines", {}).setdefault(component, []).append(drawRunAdd(markOf, FRB.RegAnchorAdd.Placement.After))
    overlayOf = {drawRange: overlaySection(component, drawRange) for component in sorted(overlays) for drawRange in shaderDraws(component)}
    for component in sorted(overlays):
        drawEdits.setdefault("overlays", {}).setdefault(component, []).append(drawRunAdd(overlayOf, FRB.RegAnchorAdd.Placement.After))
    shaderOf = {}
    for component in sorted(rerouteShaders):
        for drawRange in shaderDraws(component):
            shaderOf.setdefault(drawRange, shaderSection(component, drawRange))
    for component in sorted(rerouteShaders):
        drawEdits.setdefault("rerouteShaders", {}).setdefault(component, []).append(drawRunAdd(shaderOf, FRB.RegAnchorAdd.Placement.Replace))
    for component in sorted(FIX.plan):
        if not FIX.plan[component].offPasses or component in rerouteShaders or component not in present:
            continue
        runOf = {}
        for drawRange in shaderDraws(component):
            line = offDraw(component, drawRange)
            if line[0] == FRB.IniKeywords.Run:
                runOf[drawRange] = line[1]
        if runOf:
            drawEdits.setdefault("offPasses", {}).setdefault(component, []).append(drawRunAdd(runOf, FRB.RegAnchorAdd.Placement.Replace))

    # the sections: each draw's list and CustomShaders (WWMISections.shaderDrawSections), and the shader's files
    def shaderLines(shader, resolved):
        """A Shader's lines as its CustomShader writes them: its captures (stashed), its roles' textures (gameRoles; one with
        no resource binds nothing), then its literal lines. 'resolved' False: every register they assign, for the backups"""
        out = [(stash.reg, FRB.IniKeywords.Ref + " " + stashResource(stash.slot, stash.slotReg) if resolved else "") for stash in shader.stashed]
        for reg, role in shader.gameRoles:
            resource = sharedResourceFor(role)
            if not resolved:
                out.append((reg, ""))
            elif resource is not None:
                out.append((reg, resource))
        return out + list(shader.lines)

    shaderBackups = FRB.CustomShaderBackups(
        lambda reg: rename(FRB.IniKeywords.Resource + "ShaderBackup" + FRB.IniNamingTools.getRegTag(reg)))
    shaderSources, shaderBuffers, shaderBufferRegs, afterConstants, hasShaders = {}, {}, set(), [], False
    for component, shader in sorted(rerouteShaders.items()):
        passCount = max(1, len(shader.passes))
        assigned, passes = [], []
        for p in range(passCount):
            if shader.passes:
                assigned += list(shader.passes[p])
            ownPs = p < len(shader.passPs) and shader.passPs[p]
            lines = [("ps", shaderFile(shader, p) if ownPs else shaderFile(shader))]
            lines += [(reg, bufferResource(shader, reg)) for reg in sorted(shader.constantBuffers)]
            lines += shaderLines(shader, True) + (list(shader.passes[p]) if shader.passes else [])
            passes.append(lines)
        assigned += [(reg, "") for reg in sorted(shader.constantBuffers)] + shaderLines(shader, False)
        waits = awaited(shader)

        for drawRange in shaderDraws(component):
            draw = FRB.WWMISections.ShaderDraw()
            draw.name = lambda kind, suffix, c = component, r = drawRange: shaderSection(c, r, kind, suffix)
            draw.draw = drawValue(drawRange)
            draw.onPlan = passCondition(SLOT_PASSES[PLAN[component]])
            draw.onlyWith = shader.onlyWith
            draw.fallback = offDraw(component, drawRange)
            draw.passes = passes
            draw.assigned = assigned
            if waits:
                draw.afterTarget, draw.afterDraws, draw.afterCarried = shader.after.target, sum(d for _, d in waits), list(shader.after.carried)
            for section in FRB.WWMISections.shaderDrawSections(z3, draw, shaderBackups):
                addFixSection("shaderDraws", section)
            afterConstants += FRB.WWMISections.shaderDrawConstants(draw)
            hasShaders = True

        # the shader's files, named after the shader, so two reroutes drawn with one share them
        if shaderFile(shader) not in shaderSources:
            shaderSources[shaderFile(shader)] = shader.ps.encode()
            for p, ps in enumerate(shader.passPs):
                if ps:
                    shaderSources[shaderFile(shader, p)] = ps.encode()
            for reg, words in sorted(shader.constantBuffers.items()):
                shaderBufferRegs.add(reg)
                shaderBuffers[bufferResource(shader, reg)] = FRB.FileAddCreate.File(
                    [(FRB.IniKeywords.Type, FRB.IniKeywords.Buffer), (FRB.IniKeywords.Format, "DXGI_FORMAT_R32G32B32A32_FLOAT")],
                    modFile(toModName + FRB.IniKeywords.Remap + shader.name + FRB.IniNamingTools.getRegTag(reg) + ".buf"),
                    b"".join(FRB.BufUnSignedInt().encode(word) for word in words))

    # what each awaited component's draws run, once per game draw
    for first, dispatchList in sorted(afterDispatch.items()):
        afterRuns = [(FRB.IniKeywords.Run, shaderSection(component, drawRange, FRB.IniKeywords.CommandList, "After"))
                     for component, shader in sorted(rerouteShaders.items()) for wait in awaited(shader) if wait[0] == first
                     for drawRange in shaderDraws(component)]
        afterConstants.append((FRB.IniKeywords.Global + " " + afterLatch(first), "0"))
        addFixSection("shaderDraws", FRB.SectionText(z3, dispatchList).open(afterLatch(first) + " == 0")
                                                .key(afterLatch(first), "1").keys(afterRuns).close().build())
    if afterConstants:
        addFixSection("shaderDraws", FRB.SectionText(z3, FRB.IniKeywords.Constants).keys(afterConstants).build())

    # a mark is a shader draw with no fallback: the mod's own draw of the range is kept, the mark drawn after it
    for component, lines in sorted(markLines.items()):
        for drawRange in shaderDraws(component):
            mark = FRB.WWMISections.ShaderDraw()
            mark.name = lambda kind, suffix, component = component, drawRange = drawRange: markSection(component, drawRange, kind, suffix)
            mark.draw, mark.onPlan = drawValue(drawRange), passCondition(SLOT_PASSES[PLAN[component]])
            mark.passes, mark.assigned = [[("ps", "null")] + lines], lines
            for section in FRB.WWMISections.shaderDrawSections(z3, mark, shaderBackups):
                addFixSection("shaderDraws", section)
            hasShaders = True
    for component, overlay in sorted(overlays.items()):
        # its bindings (the "overlay" list), each register assigned
        bindings = linesOf("overlay", component)[0]
        planned = textureLists.find("overlay", component)
        assigned = [(line.reg, "") for line in ([] if planned is None else planned.lines)]
        assigned += [(reg, "") for reg in overlay.constantBuffers] + shaderLines(overlay, False)
        overlayPass = ([("ps", shaderFile(overlay))] + [(reg, bufferResource(overlay, reg)) for reg in overlay.constantBuffers]
                       + shaderLines(overlay, True))
        for drawRange in shaderDraws(component):
            draw = FRB.WWMISections.ShaderDraw()
            draw.name = lambda kind, suffix, component = component, drawRange = drawRange: overlaySection(component, drawRange, kind, suffix)
            draw.draw, draw.onPlan, draw.onlyWith = drawValue(drawRange), passCondition(SLOT_PASSES[PLAN[component]]), overlay.onlyWith
            draw.passes, draw.assigned, draw.before = [overlayPass], assigned, bindings
            for section in FRB.WWMISections.shaderDrawSections(z3, draw, shaderBackups):
                addFixSection("shaderDraws", section)
            hasShaders = True
        # its source file and constant buffers, once a shader
        if shaderFile(overlay) in shaderSources:
            continue
        shaderSources[shaderFile(overlay)] = overlay.ps.encode()
        for reg, words in sorted(overlay.constantBuffers.items()):
            shaderBufferRegs.add(reg)
            shaderBuffers[bufferResource(overlay, reg)] = FRB.FileAddCreate.File(
                [(FRB.IniKeywords.Type, FRB.IniKeywords.Buffer), (FRB.IniKeywords.Format, "DXGI_FORMAT_R32G32B32A32_FLOAT")],
                modFile(toModName + FRB.IniKeywords.Remap + overlay.name + FRB.IniNamingTools.getRegTag(reg) + ".buf"),
                b"".join(FRB.BufUnSignedInt().encode(word) for word in words))
    for reg in shaderBackups.registers():
        addFixSection("shaderDraws", shaderBackups.section(reg))
    for name, (condition, draw) in sorted(offLists.items()):
        addFixSection("shaderDraws",
            FRB.SectionText(z3, name).open(condition).key(FRB.IniKeywords.DrawIndexed, draw).close().build())

    # RabbitFX's routing flags, pinned on the slot's passes (WWMIFixer::rabbitFXPinSections) -- and, below, the sections
    # only one copy writes
    for component, flags in sorted(rabbitFXFlags.items()):
        addFixSection("rabbitFXPinLists",
            FRB.SectionText(z3, pinList(component)).open(passCondition(SLOT_PASSES[PLAN[component]]))
            .keys([(key, str(flag)) for key, flag in zip(FRB.RabbitFX.FlagKeys, flags)]).close().build())

    # the shaders' files, collected off the fix's sections in every group: `ps-cbN = Resource...` makes a buffer,
    # `ps = <file>.hlsl` names the shader's source (WWMIFixer::shaderFileEdits)
    def shaderFiles(group, graphGroups):
        if not hasShaders or ("", "fixSections") not in graphGroups[group].graphs:
            return []
        shaders = (group, "", "fixSections")
        bufferCreate = FRB.FileAddCreate((group, "", "rerouteShaderBuffers"), shaderBuffers)
        sourceAdd = FRB.FileRefAdd(shaderSources)
        kept.extend([bufferCreate, sourceAdd])
        # ONE collect over every register a shader's constants are bound at (the tattoo's ps-cb1 and the overlay's
        # ps-cb4): a second collect sharing bufferCreate builds no resource of its own (WWMIFixer's shaderFiles)
        return (([FRB.ResRegCollect({shaders: sorted(shaderBufferRegs)}, {"shaderBuffer": bufferCreate},
                                    resPredicates = {shaders: FRB.ResRegCollect.skipsKeywords()})] if shaderBufferRegs else [])
                + [FRB.ResRegCollect({shaders: "ps"}, {"shaderSource": sourceAdd},
                                     resPredicates = {shaders: FRB.ResRegCollect.skipsKeywords()})])
    shaderFileEdits = [FRB.IterGraphGroupEdit(shaderFiles)]

    # THE MOD'S OWN `ps-t` LINES, RE-KEYED BY ROLE (the "modTexLines" step, WWMIFixerConfig.modTexLines): each line it
    # moves or drops, replaced by one RegAnchorAdd per component; a line it does not list is left where it is
    modTexLines = fixVars["modTexLines"]
    carriedEdits = {}
    for component in [component for component in sorted(present) if component in PLAN]:
        lines = {(line.key, line.value): ([] if line.dropped else [(line.reg, line.newValue)])
                 for line in modTexLines.get(component, [])}
        carried = FRB.RegAnchorAdd([], lambda key, value, lines = lines: lines.get((key.strip().lower(), value.strip())),
                                   FRB.RegAnchorAdd.Placement.Replace,
                                   lambda key: key.lower().startswith(TEX_REG.lower()))
        kept.append(carried)
        carriedEdits[component] = FRB.RegPartEdit(carried)

    # the fix's shape-key lists bind the reordered copies (WWMIFixerConfig::shapeKeyOrder); any other value stays
    shapeKeyRebind = []
    if shapeKeyFiles:
        sectionOf = {shapeKeyFiles[resource][0].lower(): rename(resource) for resource in shapeKeyResources}
        shapeKeyRebind = [FRB.RegNewVals({reg: FRB.FromOldVal(lambda old, modType: sectionOf.get(old.strip().lower(), old))
                                          for reg in shapeKeyRegs})]

    # THE PER-OBJECT EDITS, one step per KIND in WWMIFixer's order (ObjEditKinds), each keyed by the target objects the
    # remap made: kind -> component -> its edits. A non-slot object of the mod gets the hashes and the shape-key settings
    byKind = {kind: {} for kind in OBJ_EDIT_KINDS}
    for group in groups:
        for component in group:
            if component in listOf:
                byKind["textureListCalls"][component] = [FRB.RegSurroundedAdd(
                    [(FRB.IniKeywords.Run, listOf[component])], {FRB.IniKeywords.MatchFirstIndex: lambda value: True})]
            # the shared lists are reached through every component's graph; the edits are idempotent
            if bypassEdits:
                byKind["sharedBypass"][component] = list(bypassEdits)
            byKind["modTexLines"][component] = [carriedEdits[component]]
            # what runs before the draw, right after the shared-resource override (WWMIFixer::buildComponentAdditions)
            ownMesh = component in ownMeshes
            additions = ([(FRB.IniKeywords.Run, FRB.WWMISections.mergeListName(skeleton, PLAN[component]))]
                         if legacy or targetPast256 else []) + \
                ([(BLEND_REG, ownMeshBlendResource(component))] if ownMesh else
                 [(FRB.IniKeywords.Run, FRB.WWMISections.blendRemapInitList(blendRemap))] if targetPast256 else []) + \
                ([(FRB.IniKeywords.WWMIShapeKeyStreamReg, zeroResource)] if zeroResource else []) + \
                ([(FRB.IniKeywords.WWMITexcoordReg, texcoordResource)] if texcoordResource and not ownMesh else []) + \
                ([(FRB.IniKeywords.Run, glowListOf[component])] if component in glowListOf else []) + \
                ([(FRB.IniKeywords.Run, gameListOf[component])] if component in gameListOf else []) + \
                [(FRB.IniKeywords.Run, passList) for passList in passListsOf.get(component, [])]
            if additions:
                anchorLists = listNames(sharedList) + (listNames(ownMeshes[component][0]) if ownMesh else [])
                byKind["additions"][component] = [FRB.RegSurroundedAdd(
                    additions, {FRB.IniKeywords.Run: lambda value, anchorLists = anchorLists: value in anchorLists}, {}, False)]
            # a reroute keeps only the draws it took, its source component everything else (WWMIFixer::buildRerouteFilters)
            for reroute, origin in sorted(rerouteFrom.items()):
                taken = sorted(reroutedDraws[reroute])
                if component == origin:
                    byKind["rerouteFilters"].setdefault(component, []).append(FRB.RegRemove(
                        {FRB.IniKeywords.DrawIndexed: lambda i, value, taken = taken: FRB.IniParseData.drawRangeOf(value) in taken}))
                elif component == reroute:
                    byKind["rerouteFilters"].setdefault(component, []).append(FRB.RegRemove(
                        {FRB.IniKeywords.DrawIndexed: lambda i, value, taken = taken: FRB.IniParseData.drawRangeOf(value) not in taken}))
            if component in removeOf:
                byKind["removedLines"][component] = [removeOf[component]]
            if component in rabbitFXFlags:
                # a `run =` of the pin list right after each `run = CommandList\RabbitFX\SetTextures`
                byKind["rabbitFXPins"][component] = [FRB.RegPartEdit(FRB.RegAnchorAdd(
                    [FRB.IniKeywords.Run],
                    lambda key, value, pinned = pinList(component): (
                        [(FRB.IniKeywords.Run, pinned)]
                        if value.strip().lower() == FRB.IniKeywords.RabbitFXSetTexturesPath.lower() else None),
                    FRB.RegAnchorAdd.Placement.After))]
            byKind["slotValues"][component] = [slotValues[PLAN[component]]]
            byKind["hashes"][component] = [assetRemap]
            byKind["dispatchSize"][component] = [dispatch]
            if shapeKeyRebind:
                byKind["shapeKeyLists"][component] = list(shapeKeyRebind)
            for kind, edits in drawEdits.items():
                if component in edits:
                    byKind[kind][component] = edits[component]
    objKindEdits = {"hashes": [assetRemap], "dispatchSize": [dispatch], "shapeKeyLists": list(shapeKeyRebind)}
    kindEdits = {}
    for kind in OBJ_EDIT_KINDS:
        perGroup = []
        for group in groups:
            edits = {targetObj(component): byKind[kind][component] for component in group if component in byKind[kind]}
            if objKindEdits.get(kind):
                edits.update({obj: objKindEdits[kind] for obj in hashOnlyObjs})
            perGroup.append(edits)
        if any(perGroup):
            kindEdits[kind] = [FRB.GraphGroupEdit(perGroup)]

    # ---- step 8: the blend, onto the target's bones: the library's vertex-group row over WWMI's layout (N ids then N
    # weights, N off the mod's own Blend.buf), collected off each group's slot sections at the blend register -- only a
    # line that names a file: `ref` names a buffer of WWMI's own ----
    # the mod's own Position.buf, however its .ini names it: a file that declares none has no mesh to read a vertex
    # count from, and is refused here, before any edit runs (WWMIFixer::positionBufPath)
    if positionFile is None:
        raise RuntimeError("cannot fix this mod: its .ini declares no [" + FRB.IniKeywords.WWMIPositionBuffer
                           + "], so there is no mesh to read a vertex count from")
    positionPath = iniPathOf(positionFile, parser._iniFile.folder)

    def blendElements(srcPath):
        return FRB.WWMIBlend.blendElements(FRB.WWMIBlend.influencesOf(srcPath, vertexCount, positionPath))

    # WHERE the mod's ids live: a character past 256 bones keeps them in WWMI's BlendRemapVertexVG.buf, named by the
    # .ini's resource section rather than a file name (WWMIFixer::vertexVGFile)
    vertexVGRel = data.resourceFile(FRB.IniKeywords.WWMIBlendRemapVertexVGBuffer)
    # how the blend is written (WWMIFixer::buildBlendCollects): a TARGET past 256 bones cannot be named by an 8-bit id,
    # so the fix writes WWMI's blend remap; a source past 256 bones has its ids remapped through BlendRemapVertexVG.buf;
    # otherwise the row runs over Blend.buf itself
    indexRel = data.resourceFile(FRB.IniKeywords.WWMIIndexBuffer)
    legacyLift = (FRB.WWMIBlend.LegacyLift(inIni(indexRel), drawRanges, dict(FIX.sourceVgMaps))
                  if legacy and indexRel else None)
    lift, pairedIds = None, None
    if targetPast256:
        out = FRB.WWMIBlend.RemapFiles(*[inIni(modFile(FRB.IniNamingTools.getRemapElementName("", N.BlendRemap + which, toModName) + ".buf"))
                                         for which in ("VertexVG", "Forward", "Reverse")])
        vgPath = inIni(vertexVGRel) if vertexVGRel else ""

        def lift(resource):
            FRB.WWMIBlend.writeRemap(resource.srcPath, resource.fixedPath, vgPath, inIni(positionFile), resource.vgRemap, out,
                                     legacyLift)
            return True
        pairedIds = lambda srcPath: [FRB.VGRemapBlendReplace.PairedIds(out.vertexVG, 2)]
    elif vertexVGRel is not None:
        def lift(resource):
            FRB.WWMIBlend.remapThroughVertexVG(resource.srcPath, resource.fixedPath, inIni(vertexVGRel), inIni(positionFile),
                                               resource.vgRemap)
            return True
    elif legacy:
        def lift(resource):
            FRB.WWMIBlend.liftLegacy(resource.srcPath, resource.fixedPath, inIni(positionFile), legacyLift, resource.vgRemap)
            return True

    # the row's version: the numbering the parser read off the mod's own geometry, unless the mod is legacy
    # (WWMIFixer::vgRemapVersion)
    blendVersion = data.blendVersion()
    rowVersion = fromVersion if legacy or blendVersion is None else FRB.Version.parse(blendVersion)

    blendCollects, blendReplaces = [], []
    for g, group in enumerate(groups):
        replace = FRB.VGRemapBlendReplace((g, "", "blend"), source, rowVersion, toVersion)
        if lift is not None:
            replace.fixFunc = lift
        replace.anchorChains = dict(FIX.anchorChains)
        replace.influences = FIX.targetInfluences
        if pairedIds is not None:
            replace.pairedIds = pairedIds
        replace.elements = blendElements
        graphs = [(g, *targetObj(component)) for component in group]
        keywords = FRB.ResRegCollect.skipsKeywords()
        ownBlends = {name.lower() for component, mesh in ownMeshes.items() for name in (mesh[1], ownMeshBlendResource(component))}
        predicate = keywords if not ownBlends else (
            lambda key, value, data, keywords = keywords, ownBlends = ownBlends: keywords(key, value, data) and value.strip().lower() not in ownBlends)
        blendCollects.append(FRB.ResRegCollect({graph: BLEND_REG for graph in graphs}, {"blend": replace},
                                               resPredicates = {graph: predicate for graph in graphs}))
        blendReplaces.append(replace)

    # the `ref` lines of the blend register (a mod of a character past 256 bones binds the register twice, the second
    # to WWMI's own override of the SOURCE's blend), rebound to the remapped blend: the name the blend edit gives the one
    # file the mod binds there (WWMIFixer::buildBlendRebind), on each drawn component's graph
    isRef = lambda value: FRB.IniNamingTools.hasKeywordPrefix(value, FRB.IniKeywords.Ref)
    blendFiles = {row.value.strip() for section in data.collectedSections(FRB.IniParseRows.Sections) for row in ownRows(section)
                  if FRB.IniNamingTools.sameKey(row.key, BLEND_REG) and not isRef(row.value)}
    remappedBlend = (blendReplaces[0].getFixResourceName(next(iter(blendFiles)), None, toModName)
                     if len(blendFiles) == 1 and blendReplaces else None)
    rebindEdits = []
    if remappedBlend is not None:
        rebind = FRB.RegNewVals({BLEND_REG: FRB.ReplaceIf(remappedBlend, lambda old, modType: isRef(old))})
        rebindEdits.append(FRB.GraphGroupEdit([{targetObj(component): [rebind] for component in group} for group in groups]))

    # the toggle lists (WWMIFixer::buildToggleLists), last, as the C++ builds them: every copy the "toggleLists" step made
    # -- the overlays' raw lists included. Copy the section out of the parser's graph without 3DMigoto's
    # matching keys, rename it to the list's name, drop its dead branches, move `this` onto the register, and take it into
    # the fix's sections; then remove every texture graph, which the fix never writes out
    toggleEdits, listEdits, takes = [], {}, []
    for toggle in textureLists.toggles:
        name, graph, section, reg, thisKeys, offBinding = (toggle.name, toggle.graph, toggle.section, toggle.reg, toggle.thisKeys,
                                                           toggle.offBinding)
        isDead = lambda i, value, dead = set(toggle.deadValues): value in dead
        listObj = (TEXTURE, name)
        toggleEdits.append(FRB.GraphCopy((0, *graph), (0, *listObj),
                                         lambda owner, part, i, s = section: owner.name == s and
                                         part.getByInd(i)[0].lower() not in FRB.IniKeywords.MatchKeys))
        listEdits[listObj] = [FRB.GraphRename(lambda old, s = section, n = name: n if old == s else old),
                              FRB.RegRemove({key: isDead for key in thisKeys}), FRB.RegRemap({key: [reg] for key in thisKeys})]
        if offBinding is not None:
            cover = FRB.RegFillMissing(reg, offBinding, FRB.RegFillMissingMode.TopdownCover)
            cover.countMissingElse = True       # the toggle binds only while it is on
            listEdits[listObj].append(cover)
        takes.append(FRB.GraphVarTake(fixSections, "fixSections", listObj, [name]))
    if listEdits:
        toggleEdits.append(FRB.GraphGroupEdit([listEdits]))
    toggleEdits += takes + [FRB.GraphRemove([(0, *obj) for obj in parser.modObjs if obj[0] == TEXTURE])]

    # the fix's own sections, into every file it writes: the sections of no kind (the toggle copies), then each kind's
    # after them (the z3 is the context the sections' `if` lines were parsed in, kept as long as the edit is)
    writeSections = [FRB.GraphCreate((0, "", "fixSections"), sectionsOf = lambda group: fixSections.sections("fixSections").list(),
                                     z3 = z3)]
    for kind in FIX_SECTION_KINDS:
        writeSections.append(FRB.GraphCreate((0, "", "fixSections"), sectionsOf = lambda group, kind = kind: fixSections.sections(kind).list(),
                                             z3 = z3, append = True))
    # the stages, in the order the list runs them; what the edits hold on to is kept with them
    return {"copies": copyEdits, "toggles": toggleEdits, "slotRemap": [slotRemap], **kindEdits,
            "blend": blendCollects, "rebind": rebindEdits, "fixSections": writeSections[:1], **dict(zip(FIX_SECTION_KINDS, ([edit] for edit in writeSections[1:]))), "shaderFiles": shaderFileEdits,
            "fixFiles": madeTextures, "latch": latchEdits, "texEdits": texEdits,
            "_kept": [kept, fixSections, z3, blendReplaces]}

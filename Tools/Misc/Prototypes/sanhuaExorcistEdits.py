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

API = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "Anime Game Remap (for all users)", "api"))
if API not in sys.path:
    sys.path.insert(0, API)
import src.py.FixRaidenBoss2 as FRB


# ======================================================================================================== data
# The configs the compiled parser and fixer are built from (WWMIParserConfig.of / WWMIFixerConfig.of), read the way
# WWMIParser and WWMIFixer read them.

def use(sourceId, targetId):
    """The pair the parser and fixer below are for: its two configs and what is read off them"""
    global SOURCE_ID, TARGET_ID, PARSE, FIX, SOURCE, VERSION, SLOT_PREFIX, SLOT_HASH, TEX_REG, HASH_ONLY, FACTS, ROLES
    global REGISTER_ROLES, TARGET, TARGET_VERSION, PLAN, REGISTERS, SHAPEKEY_DISPATCH, SLOT_PASSES, PASS_VERTEX_SHADERS
    global CREATED, FALLBACKS, DOWNLOADS, BLEND_REG, TARGET_LABELS
    SOURCE_ID, TARGET_ID = sourceId, targetId
    PARSE = FRB.WWMIParserConfig.of(SOURCE_ID)
    FIX = FRB.WWMIFixerConfig.of(SOURCE_ID, TARGET_ID)

    SOURCE = FRB.ModTypeIdTools.getName(SOURCE_ID)
    VERSION = PARSE.version
    SLOT_PREFIX = PARSE.slotPrefix
    SLOT_HASH = PARSE.slotHashType
    TEX_REG = PARSE.texRegPrefix
    HASH_ONLY = [tuple(entry) for entry in PARSE.hashOnlyObjs]

    # the source's texture facts: the roles by hash and by register, the thumbprints, the downloads
    FACTS = PARSE.textures
    ROLES = set(FACTS.roles.values())
    REGISTER_ROLES = {slot: dict(table) for slot, table in FACTS.registerRoles.items()}

    TARGET = FRB.ModTypeIdTools.getName(TARGET_ID)
    TARGET_VERSION = FIX.version

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
    BLEND_REG = FIX.blendReg
    TARGET_LABELS = dict(FIX.targetLabels)


use(FRB.ModTypeId.Sanhua, FRB.ModTypeId.SanhuaExorcist)
WATCH = ("", "watch")
TEXTURE = FRB.IniGraphModObjKeywords.Texture

# the rows the parser files, under the names the compiled parser files them by
OVERRIDE_ROWS = "WWMIParser.textureOverrides"
ROLE_ROWS = "WWMIParser.textureRoles"
TOGGLE_ROWS = "WWMIParser.textureToggles"


# ===================================================================================================== helpers

def slotOf(obj):
    return int(obj[1][len(SLOT_PREFIX):]) if obj[0] == "" and obj[1].startswith(SLOT_PREFIX) else None


def graphId(obj):
    return (0, obj[0], obj[1])


def isFixSection(name):
    return name.lower().endswith(FRB.IniKeywords.RemapFix.lower())


def roleOfHashType(hashType):
    """A HashData hash type of hers names a role: the role, or the role + `Live` / `Old`, `Old2`, ..."""
    if hashType in ROLES:
        return hashType
    for suffix in ("Live", "Old"):
        at = hashType.rfind(suffix)
        if at <= 0:
            continue
        rest, role = hashType[at + len(suffix):], hashType[:at]
        if (rest == "" or (suffix == "Old" and rest.isdigit())) and role in ROLES:
            return role
    return None


def hashTypesOf(modType):
    """{hash type: {hash, ...}} of the character, over every version HashData files"""
    result = {}
    for names in modType.hashes.repo.toNestedDict().values():
        for hashType, value in names.get(modType.name, {}).items():
            result.setdefault(hashType, set()).add(value.lower())
    return result


def hasMatchFirstIndex(section):
    return any(isinstance(part, FRB.IfContentPart) and part.getVals(FRB.IniKeywords.MatchFirstIndex) for part in section.parts)


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
    hashTypes = hashTypesOf(modType)

    # ---- step 1: the mod objects, and the classifier that sorts sections into them ----
    # her slots off her own Indices rows, the hash-only objects, a texture graph per role HashData names, the
    # catch-all texture graph, and the sections that only watch her slot hash
    slots, slotIndex = [], {}
    while modType.indices.get([SOURCE, "", SLOT_PREFIX + str(len(slots))], version, False) is not None:
        obj = ("", SLOT_PREFIX + str(len(slots)))
        slots.append(obj)
        slotIndex[obj] = obj
    hashOnly = {hashType: ("", name) for hashType, name in HASH_ONLY}
    modObjs = slots + list(hashOnly.values())
    for hashType in hashTypes:
        role = roleOfHashType(hashType)
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
        return bool(hashes) and hashes[0].strip().lower() in watchHashes and not hasMatchFirstIndex(section)

    def isUnknownTextureOverride(name, section, part):
        return (name.startswith(FRB.IniKeywords.TextureOverride) and not isFixSection(name)
                and bool(part.getVals(FRB.IniKeywords.Hash)) and not hasMatchFirstIndex(section))

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
                            keysToTrack = {FRB.IniKeywords.Hash, FRB.IniKeywords.MatchFirstIndex, "match_index_count"},
                            modTypeId = modTypeId)

    # ---- step 2: the standard rows ----
    rows = FRB.GIMIParser.RowSteps()
    rows.declaredKeys = [FRB.IniKeywords.Global + " " + FRB.IniKeywords.WWMIMeshVertexCount]
    rows.objs = lambda obj: slotOf(obj) is not None
    parser.addRowSteps(rows)

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
        elif slotOf(obj) in REGISTER_ROLES:
            # a slot's registers, as an author spells them
            roleKeys[graphId(obj)] = lambda key, regs = list(REGISTER_ROLES[slotOf(obj)]): any(
                FRB.IniNamingTools.sameKey(key, reg) for reg in regs)
    parser.deferredParseSteps += [overrides, FRB.GraphCollect(roleKeys, name = ROLE_ROWS, withConditions = True),
                                  FRB.GraphCollect(toggleKeys, name = TOGGLE_ROWS)]

    # ---- the readers that run on the rows ----
    def roots(keep):
        result = {}
        for obj, sections in parser._sectionTargets.items():
            if keep(obj):
                for name in sections:
                    result.setdefault(name, []).append(obj)
        return result

    def drawnSlots(order):
        result = {}
        for obj, sections in parser._sectionTargets.items():
            if slotOf(obj) is not None:
                result.setdefault(slotOf(obj), []).extend(sections)
        return {slot: list(dict.fromkeys(sorted(names, key = order))) for slot, names in sorted(result.items())}

    # step 4: the resource data, by WWMI's rules
    def readResourceData(data):
        slotRoots = roots(lambda obj: slotOf(obj) is not None)
        rules = FRB.IniParseData.ResourceDataRules()
        rules.rows = FRB.IniParseRows.Objs
        rules.resources = FRB.IniParseRows.Resources
        rules.sections = FRB.IniParseRows.Sections
        rules.texRegPrefix = TEX_REG
        rules.noneUndrawnWhenAll = True
        rules.libraries = [(FRB.RabbitFX.Name, FRB.RabbitFX.names)]
        rules.runDraws = lambda called: iniFile.getSection(called) is None and "\\wwmiv1\\" not in called.lower()
        rules.roots = [(obj, sections) for obj, sections in parser._sectionTargets.items() if slotOf(obj) is not None]

        # the toggles: which of the mod's own resources a texture override binds in several variants -- only for the
        # file a fix reads its textures from. One `this` under `$object_detected` alone is not a toggle
        if not onlyThisFile:
            detected = FRB.IniKeywords.WWMIObjectDetected
            rules.toggles = TOGGLE_ROWS
            rules.togglesIn = lambda section: section.startswith(FRB.IniKeywords.TextureOverride) and not isFixSection(section)
            rules.notToggles = [detected, detected + "==1"]
            rules.textureRoots = [(obj, sections) for obj, sections in parser._sectionTargets.items() if obj[0] == TEXTURE]

        # does the mod replace textures by hash? a texture override with a hash and a `this`
        has = {}
        for row in data.collected(OVERRIDE_ROWS):
            if row.section.startswith(FRB.IniKeywords.TextureOverride) and row.section not in slotRoots:
                has.setdefault(row.section, set()).add(row.key)
        if any({FRB.IniKeywords.Hash, FRB.IniKeywords.This} <= keys for keys in has.values()):
            data.setReplacesTexturesByHash(True)
        data.resourceData(rules)

    # step 5: the skeleton numbering -- Sanhua has one numbering, so nothing to probe

    # step 6: the texture roles, and each slot's file per role (IniParseData.roleFiles), by WWMI's rules -- the
    # same rules the compiled parser hands it: her facts (WWMITextureFacts.of) say what each hash is and what each
    # file is by its pixels, HashData how old each hash is, and each linked .ini is read by this parser
    def readTextures(data):
        rules = FRB.IniParseData.RoleFileRules()
        rules.iniPath, rules.folder = iniFile.file, iniFile.folder
        rules.sections, rules.declared = FRB.IniParseRows.Sections, FRB.IniParseRows.Declared
        rules.overrides, rules.roleRows, rules.objRows = OVERRIDE_ROWS, ROLE_ROWS, FRB.IniParseRows.Objs
        rules.objs = [(("", SLOT_PREFIX + str(slot)), sections) for slot, sections in drawnSlots(byWritten(iniFile.getSectionNames())).items()]
        rules.registerRoles = {("", SLOT_PREFIX + str(slot)): table for slot, table in REGISTER_ROLES.items()}
        rules.regionRoles = FACTS.regionRoles
        rules.notToggles = [FRB.IniKeywords.WWMIObjectDetected.lower(), FRB.IniKeywords.WWMIObjectDetected.lower() + "==1"]

        knownRoles = set(FACTS.roles.values()) | {role for table in REGISTER_ROLES.values() for role in table.values()}
        rules.roleOfHash = lambda hash: next((hashType for hashType, hashes in hashTypes.items()
                                             if hash in hashes and hashType in knownRoles), FACTS.roles.get(hash))
        rules.identify = lambda file: FRB.TexThumbprint.identifyFile(file, FACTS.textureThumbprints, FACTS.thumbprintSize,
                                                                     FACTS.identityMin, FACTS.identityGap)
        rules.hashAge = hashAge
        fixNames = [modType.name, *modType.getModsToFix()]
        rules.fixSection = lambda section: FRB.IniNamingTools.looksRemapped(section, fixNames)
        rules.log = print

        # the files LINKED to this one, each parsed by this character's parser in its only-this-file mode -- kept alive
        # while roleFiles reads them
        readers = []
        if rules.objs and rules.iniPath:
            for linked in iniFile.linkedInis():
                sibling = iniFile.siblingOf(linked, {modTypeId})
                reader = makeParser(sibling, modTypeId, onlyThisFile = True)
                reader.parse()
                readers.append((sibling, reader))
                rules.linked = [*rules.linked, FRB.IniParseData.RoleFileRules.Linked(reader.parseData, sibling.folder,
                                                                                      os.path.basename(linked.file))]
        data.roleFiles(rules)

    # how old a hash of hers is, lower newer: the versions HashData files it at, newest first, then the current hash
    # before its `Live` and `Old`, `Old2`, ... generations
    def hashAge(hash):
        rows = [(version, hashType, value.lower()) for version, names in modType.hashes.repo.toNestedDict().items()
                for hashType, value in names.get(modType.name, {}).items()]
        versions = {tuple(int(part) for part in version.split(".")) for version, _, _ in rows}
        age = 1 << 20
        for version, hashType, value in rows:
            if value != hash.lower():
                continue
            newer = sum(other > tuple(int(part) for part in version.split(".")) for other in versions)
            generation = 0
            for suffix, base in (("Live", 1), ("Old", 2)):
                at = hashType.rfind(suffix)
                rest = hashType[at + len(suffix):] if at > 0 else None
                if rest is not None and (rest == "" or rest.isdigit()):
                    generation = base + (int(rest) - 1 if rest else 0)
            age = min(age, newer * 100 + generation)
        return age

    parser.deferredParseReaders += [readResourceData] if onlyThisFile else [readResourceData, readTextures]
    return parser


# ======================================================================================================= fixer



def slotNumbers(modType, version):
    """Each draw slot's (match_first_index, match_index_count, vg_offset, vg_count), off the library's rows"""
    slots = []
    while True:
        slot = SLOT_PREFIX + str(len(slots))
        first = modType.indices.get([modType.name, "", slot], version, False)
        if first is None:
            return slots
        slots.append((first, modType.getIndexCount(slot, "", version), modType.getVGOffset(slot, "", version),
                      modType.getVGCount(slot, "", version)))


def makeFixer(parser, toModName, modTypeId):
    source, target = FRB.ModTypes[SOURCE].value, FRB.ModTypes[TARGET].value
    data = parser.parseData
    toVersion = FRB.Version.parse(TARGET_VERSION)
    fromVersion = parser._iniFile.fromVersion     # the .ini's own version; None = the newest bucket holding a value
    targetObj = lambda component: (toModName, SLOT_PREFIX + str(PLAN[component]))
    rename = lambda name: FRB.IniNamingTools.getRemapFixName(name, toModName)

    # ---- what the mod draws: each source component with sections that draw, and the ones it hides ----
    present, silent, slotSections = {}, [], []
    for obj, sections in parser._sectionTargets.items():
        component = slotOf(obj)
        if component is None or not sections:
            continue
        if data.isUndrawn(obj):
            silent.append(component)
        else:
            present[component] = sections
        slotSections += sections

    # ---- a mod from before WWMI's merged skeleton (no slot section carries a vg_offset) holds each component's OWN
    # bone indices in its blend, and Sanhua has no per-component vg maps to lift them with: it is left alone ----
    giveUp = lambda: FRB.GIMIFixer(parser, graphGroupEdits = [FRB.GraphGroupRemove()], modsToFix = [toModName])
    if not present and not silent:
        print("no slot section on any vb0 hash the library files under " + SOURCE + " -- so this .ini is left alone")
        return giveUp()
    if not any(data.firstCollected(FRB.IniParseRows.Objs, section, FRB.IniKeywords.WWMIVgOffset) for section in slotSections):
        print("this mod is from before WWMI's merged skeleton, and " + SOURCE + " carries no vg maps to lift its blend"
              " with -- so this .ini is left alone")
        return giveUp()

    # ---- the groups: the first claimant of a target slot stays in the mod's own .ini, the next goes to a copy ----
    groups, claimants = [], {}
    for component in sorted(present):
        if component not in PLAN:
            print("component%d has no target slot in the plan -- not remapped" % component)
            continue
        group = claimants.get(PLAN[component], 0)
        claimants[PLAN[component]] = group + 1
        groups += [[] for _ in range(group + 1 - len(groups))]
        groups[group].append(component)
    groups = groups or [[]]

    # ---- step 1: the lines a remapped section drops (WWMIFixer::buildRegRemovals): the pair's own, and WWMI's merged
    # skeleton lines when the TARGET's skeleton passes what an 8-bit blend index names -- the vertex group row's largest
    # target bone says so, unless the row names more bones than one blend remap holds (WWMIFixer::readBlendWidth) ----
    vgRow = source.getVGRemap(toModName, fromVersion, toVersion)
    rowTargets = set(vgRow.remap.values()) if vgRow is not None else set()
    targetPast256 = (any(bone > FRB.WWMIBlend.MaxByteBone for bone in rowTargets)
                     and len(rowTargets) <= FRB.WWMIBlend.RemapSize)
    removals = list(FIX.removedRegs) + (list(FIX.remappedSkeletonRemovals) if targetPast256 else [])
    # each value matched with or without `ref`; GAP: a component that keeps RabbitFX keeps RabbitFX's own lines
    # (SourceComponent::rabbitFX) -- no component of a mirrored pair does
    regRemove = FRB.RegRemove([(removal.reg, FRB.RegRemove.valueStartsWith(removal.valuePrefix) if removal.valuePrefix else None)
                               for removal in removals]) if removals else None

    # ---- step 2: the hashes and the shape-key checksums, onto the target's ----
    checksum = FRB.IniKeywords.WWMIShapeKeyChecksum
    assetRemap = FRB.RegAssetRemap({FRB.IniKeywords.Hash: (source.hashes, FRB.IniKeywords.HashNotFound),
                                    checksum: (source.shapeKeyChecksums, "ChecksumNotFound"),
                                    checksum + "_batch0": (source.shapeKeyChecksums, "ChecksumNotFound"),
                                    checksum + "_batch1": (source.shapeKeyChecksums, "ChecksumNotFound")},
                                   toModName, source.name, fromVersion, toVersion)
    dispatchKey = FRB.IniKeywords.WWMIShapeKeyDispatchSizeYOriginalBatch
    dispatch = FRB.RegNewVals({dispatchKey + "0": SHAPEKEY_DISPATCH, dispatchKey + "1": SHAPEKEY_DISPATCH})

    # ---- step 3: each target slot's numbers, over the source's ----
    targetSlots = slotNumbers(target, toVersion)
    slotValues = {}
    for slot in sorted(set(PLAN[c] for group in groups for c in group)):
        first, count, vgOffset, vgCount = targetSlots[slot]
        slotValues[slot] = FRB.RegNewVals({FRB.IniKeywords.MatchFirstIndex: first, FRB.IniKeywords.MatchIndexCount: count,
                                           FRB.IniKeywords.WWMIVgOffset: vgOffset, FRB.IniKeywords.WWMIVgCount: vgCount})

    # ---- step 4: every source slot's sections onto its target slot, named after the target; two claimants of one
    # slot collide, which makes the copies. A hidden component goes nowhere. The hash-only objects come along:
    # the bone-data marker into every file, the shape-key overrides once ----
    remap = {}
    for component in sorted(present):
        if component in PLAN:
            remap[graphId(("", SLOT_PREFIX + str(component)))] = [(0, *targetObj(component), rename)]
    for component in silent:
        remap[graphId(("", SLOT_PREFIX + str(component)))] = []
    hashOnlyObjs = [obj for obj in parser.modObjs if obj[0] != TEXTURE and slotOf(obj) is None and obj != WATCH]
    for obj in hashOnlyObjs:
        copies = 1 if obj[1] in ("shapekeyOffsets", "shapekeyScale") else len(groups)
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
    tags = FRB.ShaderFilterTags()
    allPasses = SLOT_PASSES + [list(byPass) for byPass in FIX.sharedMeshes.values()] + [list(byPass) for byPass in FIX.extraPassRegs.values()]
    taggedFor = lambda shader: PASS_VERTEX_SHADERS[shader] if PASS_VERTEX_SHADERS else [shader]
    tags.addNumbered([vs for passes in allPasses for shader in passes for vs in taggedFor(shader)],
                     dict(FIX.filterIndices), FIX.filterBase, FIX.filterStep)

    def passCondition(slot):
        return tags.condition("vs", list(dict.fromkeys(vs for shader in SLOT_PASSES[slot] for vs in PASS_VERTEX_SHADERS.get(shader, []))))

    # layer 1, the mod's own file for each role a component binds (WWMIFixer::assignRole): the resource the mod uses for
    # it (IniParseData::resourceOfFile), or one the fix declares -- RemapRef, since an undo deletes what a RemapFix names
    remapNames = [source.name, toModName]
    resourceOfSlotRole, resourceOfRole, declaredName = {}, {}, {}

    def assignRole(role, component):
        chosen = data.roleFile(role, ("", SLOT_PREFIX + str(component)))
        if chosen is None or (role, component) in resourceOfSlotRole:
            return
        own = data.resourceOfFile(chosen.file)
        if own is not None and not FRB.IniNamingTools.looksRemapped(own, remapNames):
            resourceOfSlotRole[(role, component)] = own
            return
        if chosen.file not in declaredName:
            refName = lambda n: FRB.IniKeywords.Resource + FRB.IniNamingTools.getModSuffixedName(
                role[0].upper() + role[1:] + (str(n) if n > 1 else ""), FRB.IniKeywords.RemapRef, toModName)
            n = 1
            while refName(n) in declaredName.values():
                n += 1
            declaredName[chosen.file] = refName(n)
        resourceOfSlotRole[(role, component)] = declaredName[chosen.file]

    # layer 3, the stand-in rule (WWMIFixer::standIn): the texture the fix invents, else Sanhua's own game texture,
    # downloaded as `<prefix><Role>RemapDL.dds` (WWMIFixer::fallbackOf)
    fixDownloads = {}

    def standIn(role):
        if role in resourceOfRole or role not in FALLBACKS:
            return
        game, character, version, prefix = DOWNLOADS
        kind = prefix + role[0].upper() + role[1:]
        download = FRB.FileDownloadCreate.File()
        download.file = FRB.IniNamingTools.getRemapDLName(kind) + ".dds"
        download.url = FRB.DownloadTools.url(game, character, version, prefix, "Texture" + FALLBACKS[role], ".dds")
        resourceOfRole[role] = FRB.IniNamingTools.getRemapDLResourceName(kind)
        fixDownloads[resourceOfRole[role]] = download

    planned = [component for component in sorted(present) if component in PLAN]
    for component in planned:
        for reg, role in REGISTERS[component]:
            assignRole(role, component)
    for role, _, _ in CREATED:
        resourceOfRole[role] = rename(FRB.IniKeywords.Resource + role)
    for component in planned:
        for reg, role in REGISTERS[component]:
            if (role, component) not in resourceOfSlotRole:
                standIn(role)

    def resourceFor(role, component):
        return resourceOfSlotRole.get((role, component), resourceOfRole.get(role))

    # a role whose file the mod binds behind a toggle (WWMIFixer::bindLine): bound by a COPY of the mod's own texture
    # override, its `this` lines moved onto the target's register, run from the list -- built by buildToggleLists below
    fileOfResource = {}
    for resource, file in data.textureRoles().resourcesOf(parser._iniFile.file):
        fileOfResource.setdefault(resource.lower(), file)
    ownRows = lambda section: data.sectionRows(FRB.IniParseRows.Sections, section)
    toggleLists, roleLists = [], {}

    def bindLine(role, reg, resource):
        toggle = data.thisToggleOf(resource.lower())
        if toggle is None:
            return (reg, resource)
        if (role, reg) in roleLists:
            return (FRB.IniKeywords.Run, roleLists[(role, reg)])
        if not ownRows(toggle.section) or not toggle.graph[0]:
            return (reg, resource)

        # a branch naming a resource the mod never declares is the author's typo: dropped, the mod's own binding stands
        isDead = lambda i, value: value.lower() not in fileOfResource and not ownRows(value)
        thisKeys = list(dict.fromkeys(row.key for row in ownRows(toggle.section) if row.key.lower() == FRB.IniKeywords.This))
        if not any(not isDead(0, row.value) for row in ownRows(toggle.section) if row.key.lower() == FRB.IniKeywords.This):
            return (reg, resource)

        name = rename(FRB.IniKeywords.CommandList + source.name + role[0].upper() + role[1:] + FRB.IniNamingTools.getRegTag(reg))
        toggleLists.append((name, toggle.graph, toggle.section, reg, thisKeys, isDead))   # GAP: a one-variant toggle's
        roleLists[(role, reg)] = name                                                    # off binding (planGameDefaults)
        return (FRB.IniKeywords.Run, name)

    listOf = {}
    for component in sorted(present):
        if component not in PLAN:
            continue
        lines = [bindLine(role, reg, resourceFor(role, component)) for reg, role in REGISTERS[component] if resourceFor(role, component)]
        if lines:
            listOf[component] = rename(FRB.IniKeywords.CommandList + source.name + "Component" + str(component) + "Textures")
            fixSections.sections("fixSections").add(
                FRB.SectionText(z3, listOf[component]).open(passCondition(PLAN[component])).keys(lines).close().build())

    # the resources the fix declares, after the lists (WWMIFixer::buildAppended), in the order they were named
    for file, name in declaredName.items():
        fixSections.sections("fixSections").add(
            FRB.SectionText(z3, name).key(FRB.IniKeywords.Filename, os.path.relpath(file, parser._iniFile.folder)).build())

    # the invented textures: made where the fix's lists bind them, in every group the slot remap made
    # (WWMIFixer::fixFileEdits: one TexCreate per group, collected off every graph of that group)
    kept = []

    def makeTextures(group, graphGroups):
        edits = []
        # the downloads first, then the invented textures (WWMIFixer::fixFileEdits' order)
        if fixDownloads:
            download = FRB.FileDownloadCreate((group, "", "fixDownloads"), fixDownloads)
            graphs = [(group, *obj) for obj in graphGroups[group].graphs]
            edits.append(FRB.ResRegCollect({graph: (lambda key: True) for graph in graphs}, {"fixFile": download},
                                           resPredicates = {graph: (lambda key, value, data: value.strip() in fixDownloads)
                                                            for graph in graphs}))
            kept.append(download)
        for role, colour, size in CREATED:
            resource = resourceOfRole[role]
            make = FRB.TexCreate((group, "", "fixTexture" + resource), "", FRB.CppTexCreator(size, size, colour))
            make.files = {resource: FRB.IniNamingTools.getModSuffixedName(role, FRB.IniKeywords.RemapTex, toModName) + ".dds"}
            graphs = [(group, *obj) for obj in graphGroups[group].graphs]
            edits.append(FRB.ResRegCollect({graph: (lambda key: True) for graph in graphs}, {"fixFile": make},
                                           resPredicates = {graph: (lambda key, value, data, r = resource: value.strip() == r)
                                                            for graph in graphs}))
            kept.append(make)

        return edits
    madeTextures = [FRB.IterGraphGroupEdit(makeTextures)]

    # the toggle lists (WWMIFixer::buildToggleLists): copy the section out of the parser's graph without 3DMigoto's
    # matching keys, rename it to the list's name, drop its dead branches, move `this` onto the register, and take it into
    # the fix's sections; then remove every texture graph, which the fix never writes out
    toggleEdits, listEdits, takes = [], {}, []
    for name, graph, section, reg, thisKeys, isDead in toggleLists:
        listObj = (TEXTURE, name)
        toggleEdits.append(FRB.GraphCopy((0, *graph), (0, *listObj),
                                         lambda owner, part, i, s = section: owner.name == s and
                                         part.getByInd(i)[0].lower() not in FRB.IniKeywords.MatchKeys))
        listEdits[listObj] = [FRB.GraphRename(lambda old, s = section, n = name: n if old == s else old),
                              FRB.RegRemove({key: isDead for key in thisKeys}), FRB.RegRemap({key: [reg] for key in thisKeys})]
        takes.append(FRB.VarSectionsTake(fixSections, "fixSections", listObj, [name]))
    if listEdits:
        toggleEdits.append(FRB.GraphGroupEdit([listEdits]))
    toggleEdits += takes + [FRB.GraphRemove([(0, *obj) for obj in parser.modObjs if obj[0] == TEXTURE])]

    # ---- step 6: the shared-resource pair made symmetric (WWMIFixer::buildBypasses): every buffer the mod's override
    # list binds and its cleanup list does not put back is parked in a resource of the fix's before it is replaced, and
    # the cleanup list runs the fix's list that puts them back ----
    sharedList, cleanupList = "CommandListOverrideSharedResources", "CommandListCleanupSharedResources"
    restoreList = FRB.WWMISections.restoreListName(toModName)
    bypasses = FRB.WWMISections.bypasses(data, FRB.IniParseRows.Sections, sharedList, cleanupList, toModName)

    def onlyIn(names):
        names = {name.lower() for name in names}
        return lambda sectionName: sectionName.lower() in names

    bypassEdits = []
    if bypasses:
        for reg, bypass in bypasses:
            bypassEdits.append(FRB.RegPartEdit(
                FRB.RegAnchorAdd([reg], lambda key, value, b = bypass, r = reg: [(b, FRB.IniKeywords.Ref + " " + r)],
                                 FRB.RegAnchorAdd.Placement.Before, None, True, True),
                onlyIn([sharedList, rename(sharedList)])))
        bypassEdits.append(FRB.RegPartEdit(FRB.RegAdd([(FRB.IniKeywords.Run, restoreList)], True, True),
                                           onlyIn([cleanupList, rename(cleanupList)])))
        for section in FRB.WWMISections.bypassSections(z3, bypasses, restoreList):
            fixSections.sections("fixSections").add(section)

    # ---- step 7: WWMI's own sections (WWMISections): each target slot nothing of the mod is drawn through skips the
    # skin's geometry and still merges the slot's bones; then the shader tags the texture lists are gated on ----
    drawnSlots = {PLAN[component] for group in groups for component in group}
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
        fixSections.sections("fixSections").add(FRB.WWMISections.hideSection(z3, hide))
    for i in range(len(tags.tags())):
        fixSections.sections("fixSections").add(tags.section(i, rename("ShaderOverridePass" + str(i))))

    # the edits, per group, keyed by the target objects the remap made
    perGroup = []
    for group in groups:
        edits = {}
        for component in group:
            edits[targetObj(component)] = [*([regRemove] if regRemove else []), slotValues[PLAN[component]], assetRemap, dispatch]
            if component in listOf:
                edits[targetObj(component)].append(FRB.RegSurroundedAdd(
                    [(FRB.IniKeywords.Run, listOf[component])], {FRB.IniKeywords.MatchFirstIndex: lambda value: True}))
            # the shared lists are reached through every component's graph; the edits are idempotent
            edits[targetObj(component)] += bypassEdits
        for obj in hashOnlyObjs:
            edits[obj] = [assetRemap, dispatch]
        perGroup.append(edits)
    mainEdits = FRB.GraphGroupEdit(perGroup)

    # ---- step 8: the blend, onto the target's bones: the library's vertex-group row over WWMI's layout (N ids then N
    # weights, N off the mod's own Blend.buf), collected off each group's slot sections at the blend register -- only a
    # line that names a file: `ref` names a buffer of WWMI's own. (WWMIFixer::blendRebindEdits, which points a mod's
    # `vb4 = ref ResourceBlendBufferOverride` at the remapped blend, has nothing to do here: only a mod of a character
    # past 256 bones binds it so, and Sanhua has fewer.) ----
    positionFile = data.resourceFile(FRB.IniKeywords.WWMIPositionBuffer)
    countRow = data.firstCollected(FRB.IniParseRows.Declared, FRB.IniKeywords.Constants,
                                   FRB.IniKeywords.Global + " " + FRB.IniKeywords.WWMIMeshVertexCount)
    vertexCount = int(countRow.value.strip()) if countRow is not None else 0

    def blendElements(srcPath):
        if positionFile is None:
            raise RuntimeError("cannot fix this mod: its .ini declares no [" + FRB.IniKeywords.WWMIPositionBuffer + "]")
        positionPath = iniPathOf(positionFile, parser._iniFile.folder)
        return FRB.WWMIBlend.blendElements(FRB.WWMIBlend.influencesOf(srcPath, vertexCount, positionPath))

    blendCollects = []
    for g, group in enumerate(groups):
        replace = FRB.VGRemapBlendReplace((g, "", "blend"), source, fromVersion, toVersion)
        replace.elements = blendElements
        graphs = [(g, *targetObj(component)) for component in group]
        blendCollects.append(FRB.ResRegCollect({graph: BLEND_REG for graph in graphs}, {"blend": replace},
                                               resPredicates = {graph: FRB.ResRegCollect.skipsKeywords() for graph in graphs}))

    # the fix's own sections, into every file it writes
    writeSections = FRB.GraphCreate((0, "", "fixSections"))
    writeSections.sectionsOf = lambda group: fixSections.sections("fixSections").list()
    writeSections.z3 = z3       # the context the sections' `if` lines were parsed in, kept as long as the edit is
    fixer = FRB.GIMIFixer(parser, graphGroupEdits = [*toggleEdits, slotRemap, mainEdits, *blendCollects, writeSections, *madeTextures],
                          modsToFix = [toModName])
    # a copy carries the fix's own sections and none of the mod's texture overrides live: the mod's own file keeps
    # serving the mod on its own character
    fixer.copyHiddenSectionNames = {section for section in data.collectedSections(FRB.IniParseRows.Sections)
                                    if section.startswith(FRB.IniKeywords.TextureOverride)}
    fixer.copyPreamble = FRB.IniComments.GIMIObjMergerPreamble
    return fixer

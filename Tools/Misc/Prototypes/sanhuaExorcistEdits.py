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

    # which numbering the blend's bone ids are in, asked of the mod's own geometry (SkeletonNumberingProbe) -- not of a
    # mod from before WWMI's merged skeleton (no vg_offset on any slot section), whose ids are each component's own
    def readSkeletonNumbering(data):
        if len(PARSE.skeletonNumberings) < 2 or not PARSE.referenceBoneCentroids:
            return
        merged = any(data.firstCollected(FRB.IniParseRows.Objs, section, FRB.IniKeywords.WWMIVgOffset) is not None
                     for obj, sections in parser._sectionTargets.items() if slotOf(obj) is not None for section in sections)
        position = data.resourceFile(FRB.IniKeywords.WWMIPositionBuffer)
        blend = data.resourceFile(FRB.IniKeywords.WWMIBlendBuffer)
        if not merged or position is None or blend is None:
            return
        # the remapped skeleton's own 16-bit ids: the first resource, by name, naming them
        vertexVG = next((file for section, file in data.resourceFiles().items()
                         if "blendremapvertexvg" in section.replace("_", "").lower()), None)
        pathOf = lambda rel: iniPathOf(rel, parser._iniFile.folder)
        found = FRB.SkeletonNumberingProbe.probe(pathOf(position), pathOf(blend), pathOf(vertexVG) if vertexVG else None,
                                                 PARSE.skeletonNumberings, PARSE.referenceBoneCentroids)
        if found is not None:
            data.setBlendVersion(PARSE.skeletonNumberings[found.best].version)

    # GAP: WWMIParserConfig::drawSplits (readDrawSplits; only Lynae's rows set it)
    parser.deferredParseReaders += [readResourceData] if onlyThisFile else [readResourceData, readSkeletonNumbering, readTextures]
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
    # the target object a source component's sections are remapped onto: its slot's, or -- for a second section on a
    # shared slot in the mod's own file (SourceComponent::sameFileSection) -- one of its own, so the two claimants do
    # not collide in one group (WWMIFixer::targetObjOf)
    targetObj = lambda component: (toModName, SLOT_PREFIX + str(PLAN[component])
                                   + ("_" + str(component) if FIX.plan[component].sameFileSection else ""))
    rename = lambda name: FRB.IniNamingTools.getRemapFixName(name, toModName)

    # ---- what the mod draws: each source component with sections that draw, and the ones it hides ----
    present, silent, slotSections, objOf = {}, [], [], {}
    for obj, sections in parser._sectionTargets.items():
        component = slotOf(obj)
        if component is None or not sections:
            continue
        if data.isUndrawn(obj):
            silent.append(component)
        else:
            present[component] = sections
            objOf[component] = obj
        slotSections += sections

    # ---- a mod from before WWMI's merged skeleton (no slot section carries a vg_offset) holds each component's OWN
    # bone indices in its blend: lifted through the source's per-component vg maps, and the fix supplies the merged
    # skeleton it lacks -- or, with no maps to lift them with, left alone (WWMIFixer::readMod) ----
    giveUp = lambda: FRB.GIMIFixer(parser, graphGroupEdits = [FRB.GraphGroupRemove()], modsToFix = [toModName])

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
            takes.append(FRB.VarSectionsTake(variables, "fixSections", copyObj, [name], commentOf(section), inPlace))
        return copies + ([FRB.GraphGroupEdit([edits])] + takes if sections else [])

    targetVb0 = target.hashes.get([target.name, SLOT_HASH], toVersion, False)
    if not present and not silent:
        # A FILE THAT ONLY WATCHES THE SOURCE (WWMIFixer::buildWatcher): each watching section, as the author wrote it,
        # under the fix's name on the target's vb0 -- copied before the removal of every group, nothing drawn
        watching = parser._sectionTargets.get(WATCH, [])
        if watching and targetVb0:
            watchZ3, watchSections = FRB.Z3Context(), FRB.GraphEditVariables()
            copies = copyOntoHash(watchSections, WATCH, watching, targetVb0, lambda section: "", False)
            write = FRB.GraphCreate((0, "", "fixSections"))
            write.sectionsOf = lambda group: watchSections.sections("fixSections").list()
            write.z3 = watchZ3
            print("only watches " + SOURCE + ", so its " + str(len(watching)) + " watching section(s) are carried onto "
                  + toModName + "'s vb0 and nothing is drawn")
            return FRB.GIMIFixer(parser, graphGroupEdits = [*copies, FRB.GraphGroupRemove(), write], modsToFix = [toModName])
        print("no slot section on any vb0 hash the library files under " + SOURCE + " -- so this .ini is left alone")
        return giveUp()
    legacy = not any(data.firstCollected(FRB.IniParseRows.Objs, section, FRB.IniKeywords.WWMIVgOffset) for section in slotSections)
    if legacy and not FIX.sourceVgMaps:
        print("this mod is from before WWMI's merged skeleton, and " + SOURCE + " carries no vg maps to lift its blend"
              " with -- so this .ini is left alone")
        return giveUp()

    # ---- the groups: the first claimant of a target slot stays in the mod's own .ini, the next goes to a copy ----
    groups, claimants = [], {}
    for component in sorted(present):
        if component not in PLAN:
            print("component%d has no target slot in the plan -- not remapped" % component)
            continue
        # a second section in the mod's own file (SourceComponent::sameFileSection) claims no copy
        if FIX.plan[component].sameFileSection:
            group = 0
        else:
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

    # the merged skeleton the fix supplies for a legacy mod or a target past 256 bones (WWMIFixer::runtimeSkeleton);
    # the merge is latched on $object_detected where the host's [Present] resets it
    skeleton = FRB.WWMISections.Skeleton()
    skeleton.blendRemapBones = len(rowTargets)
    skeleton.name = rename
    skeleton.cb4Hash = (target.hashes.get([target.name, "cb4"], toVersion, False) or "").lower()
    skeleton.boneDataFilter = FIX.boneDataFilter
    skeleton.mergedSkeletonSlots = FIX.mergedSkeletonSlots
    skeleton.remapped = targetPast256
    skeleton.latched = any(row.key.lower().startswith(FRB.IniKeywords.Post + " " + FRB.IniKeywords.WWMIObjectDetected)
                           and row.value.strip() == "0"
                           for section in data.collectedSections(FRB.IniParseRows.Sections)
                           if section.lower() == FRB.IniKeywords.Present.lower()
                           for row in data.sectionRows(FRB.IniParseRows.Sections, section))
    skeleton.currentPoseInCb3Only = FIX.currentPoseInCb3Only
    skeleton.bindPrevPoseAlways = FIX.bindPrevPoseAlways
    skeleton.blendRemapForward = rename(FRB.IniKeywords.Resource + "BlendRemapForwardBuffer")
    skeleton.slotWindows = [(vgOffset, vgCount) for _, _, vgOffset, vgCount in targetSlots]
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
        remap[graphId(("", SLOT_PREFIX + str(component)))] = [(0, *targetObj(component), rename)] if component in PLAN else []
    for component in silent:
        remap[graphId(("", SLOT_PREFIX + str(component)))] = []
    counterpart = lambda obj: not obj[0] and obj[1] in FIX.meshCounterparts
    hashOnlyObjs = [obj for obj in parser.modObjs if obj[0] != TEXTURE and slotOf(obj) is None and obj != WATCH and not counterpart(obj)]
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

    def passCondition(passes):
        return tags.condition("vs" if PASS_VERTEX_SHADERS else "ps", list(dict.fromkeys(tag for p in passes for tag in taggedFor(p))))

    # layer 1, the mod's own file for each role a component binds (WWMIFixer::assignRole): the resource the mod uses for
    # it (IniParseData::resourceOfFile), or one the fix declares -- RemapRef, since an undo deletes what a RemapFix names
    remapNames = [source.name, toModName]
    resourceOfSlotRole, resourceOfRole, declaredName, fileOfRole = {}, {}, {}, {}

    def assignRole(role, component):
        chosen = data.roleFile(role, ("", SLOT_PREFIX + str(component)))
        if chosen is None or (role, component) in resourceOfSlotRole:
            return
        fileOfRole[role] = chosen.file
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
    fixDownloads, fallbackFileOfRole = {}, {}

    # a role whose files the parser all refused (a flat texture) and that the config leaves to the game: no stand-in
    # either, the game's own is what the mod rendered with (WWMIFixer::readRefusals)
    leftToGame = {rejection.role for rejection in data.rejections()
                  if rejection.role in FIX.flatLeftToGame and not data.roleCandidates(rejection.role)}

    def standIn(role):
        if role in resourceOfRole or role in leftToGame or role not in FALLBACKS:
            return
        game, character, version, prefix = DOWNLOADS
        kind = prefix + role[0].upper() + role[1:]
        download = FRB.FileDownloadCreate.File()
        download.file = FRB.IniNamingTools.getRemapDLName(kind) + ".dds"
        download.url = FRB.DownloadTools.url(game, character, version, prefix, "Texture" + FALLBACKS[role], ".dds")
        resourceOfRole[role] = FRB.IniNamingTools.getRemapDLResourceName(kind)
        fixDownloads[resourceOfRole[role]] = download
        fallbackFileOfRole[role] = download.file

    planned = [component for component in sorted(present) if component in PLAN]
    for component in planned:
        for reg, role in REGISTERS[component]:
            assignRole(role, component)

    # the roles only a slot's OTHER passes (of a slot something is drawn through) or a shared mesh bind
    # (WWMIFixer::passOnlyRoles), offered to every component that could serve them
    drawnThrough = {PLAN[component] for component in planned}
    passOnlyRoles = list(dict.fromkeys(
        [binding.role for slot, byPass in FIX.extraPassRegs.items() if slot in drawnThrough
         for bindings in byPass.values() for binding in bindings]
        + [binding.role for byPass in FIX.sharedMeshes.values() for bindings in byPass.values() for binding in bindings]))
    for role in passOnlyRoles:
        for component in sorted(present):
            assignRole(role, component)
    for role, _, _ in CREATED:
        resourceOfRole[role] = rename(FRB.IniKeywords.Resource + role)
    for component in planned:
        for reg, role in REGISTERS[component]:
            if (role, component) not in resourceOfSlotRole:
                standIn(role)
    for role in passOnlyRoles:
        if not any(owned == role for owned, _ in resourceOfSlotRole):
            standIn(role)

    def resourceFor(role, component):
        return resourceOfSlotRole.get((role, component), resourceOfRole.get(role))

    sharedResourceFor = lambda role: next((resource for component in sorted(present)
                                           for resource in [resourceFor(role, component)] if resource), None)

    # THE SOURCE'S OWN GAME TEXTURE AS THE DEFAULT, where the mod's file reaches only some of a component's draws
    # (WWMIFixer::planGameDefaults): 1. the draws a section makes before it binds anything, 2. a role bound only under a
    # toggle that reaches none of the section's draws, 3. a role bound with one `this` under a toggle (off, the game's)
    gameDownloads, gameResourceOfRole, toggledGame = {}, {}, {}     # role -> download; role -> resource; (role, component) -> resource

    def planGameResource(role):
        if role in gameResourceOfRole:
            return True
        if not DOWNLOADS[1] or role not in FALLBACKS:
            return False
        if role not in gameDownloads:
            game, character, version, prefix = DOWNLOADS
            kind = prefix + role[0].upper() + role[1:] + "Game"
            download = FRB.FileDownloadCreate.File()
            download.file = FRB.IniNamingTools.getRemapDLName(kind) + ".dds"
            download.url = FRB.DownloadTools.url(game, character, version, prefix, "Texture" + FALLBACKS[role], ".dds")
            gameDownloads[role] = (FRB.IniNamingTools.getRemapDLResourceName(kind), download)
        gameResourceOfRole[role] = gameDownloads[role][0]
        return True

    lateBinds = {component: data.lateBindings(obj) for component, obj in objOf.items() if data.lateBindings(obj)}
    if DOWNLOADS[1]:
        # 1. the draws before the first binding: a role the mod has no file for is already the source's own
        lateRoles = sorted({binding.role for component in lateBinds if component in FIX.plan
                            for binding in FIX.plan[component].bindings
                            if binding.role != FRB.IniKeywords.Null and binding.role not in leftToGame})
        for role in lateRoles:
            if role not in fileOfRole:
                if sharedResourceFor(role):
                    gameResourceOfRole[role] = sharedResourceFor(role)
            else:
                planGameResource(role)

        # 2. a role bound only under a toggle, and 3. a one-variant toggle
        for gamePass in (2, 3):
            for component in sorted(present):
                if component not in FIX.plan:
                    continue
                toggledOnly = data.toggledBindings(("", SLOT_PREFIX + str(component)))
                if gamePass == 2 and not toggledOnly:
                    continue
                for binding in FIX.plan[component].bindings:
                    resource = resourceFor(binding.role, component)
                    if binding.role == FRB.IniKeywords.Null or binding.role in leftToGame or resource is None \
                            or (gamePass == 2 and binding.role not in fileOfRole):
                        continue
                    base = resource.lower()
                    if gamePass == 3:
                        toggle = data.thisToggleOf(base)
                        if toggle is not None and toggle.single:
                            planGameResource(binding.role)
                        continue
                    if base not in toggledOnly or data.isThisBound(base):
                        continue
                    if planGameResource(binding.role):
                        toggledGame[(binding.role, component)] = gameResourceOfRole[binding.role]
    for resource, download in gameDownloads.values():
        fixDownloads[resource] = download

    # a role whose file the mod binds behind a toggle (WWMIFixer::bindLine): bound by a COPY of the mod's own texture
    # override, its `this` lines moved onto the target's register, run from the list -- built by buildToggleLists below
    fileOfResource = {}
    for resource, file in data.textureRoles().resourcesOf(parser._iniFile.file):
        fileOfResource.setdefault(resource.lower(), file)
    ownRows = lambda section: data.sectionRows(FRB.IniParseRows.Sections, section)
    toggleLists, roleLists, listsOfRole = [], {}, {}

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
        listsOfRole.setdefault((role, reg), set()).add(name)
        # a one-variant toggle: off, the original draws with the game's texture
        offBinding = gameResourceOfRole.get(role) if toggle.single else None
        toggleLists.append((name, toggle.graph, toggle.section, reg, thisKeys, isDead, offBinding))
        roleLists[(role, reg)] = name
        return (FRB.IniKeywords.Run, name)

    listOf, passListsOf, gameListOf = {}, {}, {}
    for component in sorted(present):
        if component not in PLAN:
            continue
        bound = [(reg, role) for reg, role in REGISTERS[component] if resourceFor(role, component)]
        lines = [(reg, toggledGame[(role, component)]) if (role, component) in toggledGame
                 else bindLine(role, reg, resourceFor(role, component)) for reg, role in bound]
        if lines:
            listOf[component] = rename(FRB.IniKeywords.CommandList + source.name + "Component" + str(component) + "Textures")
            for reg, role in bound:
                listsOfRole.setdefault((role, reg), set()).add(listOf[component])
            fixSections.sections("fixSections").add(
                FRB.SectionText(z3, listOf[component]).open(passCondition(SLOT_PASSES[PLAN[component]])).keys(lines).close().build())

        # generation 0: the draws this section makes before it binds anything rendered with the source's own game
        # textures, so that is what they are given
        if component in lateBinds and gameResourceOfRole:
            gameBound = [(reg, role) for reg, role in REGISTERS[component]
                         if role != FRB.IniKeywords.Null and role in gameResourceOfRole]
            gameLines = [bindLine(role, reg, gameResourceOfRole[role]) for reg, role in gameBound]
            if gameLines:
                gameListOf[component] = rename(FRB.IniKeywords.CommandList + source.name + "Component" + str(component) + "GameTextures")
                for reg, role in gameBound:
                    listsOfRole.setdefault((role, reg), set()).add(gameListOf[component])
                fixSections.sections("fixSections").add(
                    FRB.SectionText(z3, gameListOf[component]).open(passCondition(SLOT_PASSES[PLAN[component]]))
                    .keys(gameLines).close().build())

        # a slot's OTHER passes bind the same art at DIFFERENT registers (WWMIFixerConfig::extraPassRegs): each its own
        # list, gated on that pass, run before the draw -- and a pass the slot's draw must not be re-issued on
        # (extraPassNoDraw) nulls it there
        noDraw = FIX.extraPassNoDraw.get(PLAN[component], set())
        for n, (passName, bindings) in enumerate(FIX.extraPassRegs.get(PLAN[component], {}).items()):
            passBound = [binding for binding in bindings
                         if binding.srcComponent in (-1, component) and resourceFor(binding.role, component)]
            passLines = [bindLine(binding.role, binding.reg, resourceFor(binding.role, component)) for binding in passBound]
            if passName in noDraw:
                passLines.append((FRB.IniKeywords.Ib, FRB.IniKeywords.Null))
            if passLines:
                passList = rename(FRB.IniKeywords.CommandList + source.name + "Component" + str(component) + "TexturesPass" + str(n))
                passListsOf.setdefault(component, []).append(passList)
                for binding in passBound:
                    listsOfRole.setdefault((binding.role, binding.reg), set()).add(passList)
                fixSections.sections("fixSections").add(
                    FRB.SectionText(z3, passList).open(passCondition([passName])).keys(passLines).close().build())

    # the resources the fix declares, after the lists (WWMIFixer::buildAppended), in the order they were named
    for file, name in declaredName.items():
        fixSections.sections("fixSections").add(
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
    blendRemap.remapFile = lambda which: modFile(FRB.IniNamingTools.getRemapElementName("", "BlendRemap" + which, toModName) + ".buf")
    blendRemap.blendStride = influences * 2
    blendRemap.influences = influences
    blendRemap.blendReg = BLEND_REG

    texcoordResource, fixFiles, fixFileRegs = None, {}, set()
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
            texcoordResource = rename(FRB.IniKeywords.Resource + "TexcoordNoNaN")
            fixFiles[texcoordResource] = FRB.FileAddCreate.File(
                [(FRB.IniKeywords.Type, FRB.IniKeywords.Buffer),
                 (FRB.IniKeywords.Format, formatRow.value.strip() if formatRow is not None else "DXGI_FORMAT_R16G16_FLOAT"),
                 (FRB.IniKeywords.Stride, str(stride))],
                modFile(toModName + FRB.IniKeywords.Remap + "Texcoord.buf"),
                makeBytes = lambda: cleaned()[0].data)
            fixFileRegs.add(FIX.texcoordReg)

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
        zeroResource = rename(FRB.IniKeywords.Resource + "ShapeKeyZero")
        zeroSize = vertexCount * FIX.shapeKeyStride
        fixFiles[zeroResource] = FRB.FileAddCreate.File(
            [(FRB.IniKeywords.Type, FRB.IniKeywords.Buffer), (FRB.IniKeywords.Format, "DXGI_FORMAT_R32G32B32_FLOAT"),
             (FRB.IniKeywords.Stride, str(FIX.shapeKeyStride))],
            modFile(FRB.IniNamingTools.getRemapElementName("", "ShapeKeyZero", toModName) + ".buf"),
            makeBytes = lambda: bytes(zeroSize))
        fixFileRegs.add(FIX.shapeKeyStreamReg)

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

    def makeTextures(group, graphGroups):
        edits = []
        graphs = [(group, *obj) for obj in graphGroups[group].graphs]
        # the fix's own files, then the downloads, then the invented textures (WWMIFixer::fixFileEdits' order)
        if fixFiles:
            create = FRB.FileAddCreate((group, "", "fixFiles"), fixFiles)
            edits.append(FRB.ResRegCollect({graph: sorted(fixFileRegs) for graph in sorted(graphs)}, {"fixFile": create},
                                           resPredicates = {graph: (lambda key, value, data: value.strip() in fixFiles)
                                                            for graph in graphs}))
            kept.append(create)
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

    # the frame's skeleton remap, latched on $object_detected where the host's [Present] resets it (WWMIFixer::latchEdits)
    latchEdits = []
    if targetPast256 and skeleton.latched:
        detected = FRB.IniKeywords.WWMIObjectDetected

        def latchTemplate(setsDetected):
            text = FRB.SectionText(z3, "").open(detected + " == 0") \
                .key(FRB.IniKeywords.Run, rename("CommandListRemapMergedSkeleton")).close()
            return (text.key(detected, "1") if setsDetected else text).build()

        merges = {rename("CommandListMergeWindow")} | {FRB.WWMISections.mergeListName(skeleton, slot)
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

    # the edits the fix makes to a role's texture before binding it (WWMIFixer::planTexEdits): each edit applies to
    # the role's resolved resource and to every other variant the mod's toggles bind it as -- unless another role's
    # edit of another kind has claimed that file already
    # GAP: TexEdit.sourceGameRoles (the source's own game texture an edit reads; only Lynae -> LynaePeppermint)
    roleKind = lambda role: next((role[i:] for i, char in enumerate(role) if char.isupper()), "")
    editTargets, editedRolesOf = {}, {}

    def noteEdited(resource, role):
        editTargets.setdefault(role, set()).add(resource.lower())
        roles = editedRolesOf.setdefault(resource.lower(), [])
        if role not in roles:
            roles.append(role)

    for edit in FIX.texEdits:
        if edit.makeFilter is None or sharedResourceFor(edit.role) is None:
            continue
        if edit.role not in fileOfRole and edit.role not in fallbackFileOfRole:
            continue
        was = sharedResourceFor(edit.role)
        noteEdited(was, edit.role)
        for toggle in (data.thisToggleOf(was.lower()), data.registerToggleOf(was.lower())):
            for variant in (toggle.variants if toggle is not None else []):
                if variant.lower() == was.lower():
                    continue
                claimed = editedRolesOf.get(variant.lower())
                if claimed and any(by == edit.role or roleKind(by) != roleKind(edit.role) for by in claimed):
                    continue
                if variant.lower() not in fileOfResource:
                    continue
                noteEdited(variant, edit.role)

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
    context.drawRanges = {component: data.drawRanges(obj) for component, obj in objOf.items() if data.drawRanges(obj)}
    toggledRoles = {role for role, _ in toggledGame}
    context.fileOfRole = {**{role: inFolder(file) for role, file in fallbackFileOfRole.items()},
                          **{role: file for role, file in fileOfRole.items() if role not in toggledRoles}}
    context.gameFileOfRole = {role: inFolder(download.file) for role, (_, download) in gameDownloads.items()}

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
            if component not in PLAN:
                continue
            bindings = list(FIX.plan[component].bindings) + [binding for bindings in FIX.extraPassRegs.get(PLAN[component], {}).values()
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
            data.sectionName in listsOfRole.get((role, key), set()) and isTarget(key, value, data))

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

    # the toggle lists (WWMIFixer::buildToggleLists): copy the section out of the parser's graph without 3DMigoto's
    # matching keys, rename it to the list's name, drop its dead branches, move `this` onto the register, and take it into
    # the fix's sections; then remove every texture graph, which the fix never writes out
    toggleEdits, listEdits, takes = [], {}, []
    for name, graph, section, reg, thisKeys, isDead, offBinding in toggleLists:
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
        takes.append(FRB.VarSectionsTake(fixSections, "fixSections", listObj, [name]))
    if listEdits:
        toggleEdits.append(FRB.GraphGroupEdit([listEdits]))
    toggleEdits += takes + [FRB.GraphRemove([(0, *obj) for obj in parser.modObjs if obj[0] == TEXTURE])]

    # ---- step 6: the shared-resource pair made symmetric (WWMIFixer::buildBypasses): every buffer the mod's override
    # list binds and its cleanup list does not put back is parked in a resource of the fix's before it is replaced, and
    # the cleanup list runs the fix's list that puts them back ----
    sharedList, cleanupList = FIX.sharedResourcesList, FIX.cleanupResourcesList
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
        hide.legacyMergeList = FRB.WWMISections.mergeListName(skeleton, slot) if legacy else ""
        hide.mergeWindowList = rename("CommandListMergeWindow") if targetPast256 else ""
        fixSections.sections("fixSections").add(FRB.WWMISections.hideSection(z3, hide))
    if legacy or targetPast256:
        for section in FRB.WWMISections.skeletonSections(z3, skeleton):
            fixSections.sections("fixSections").add(section)
        if targetPast256:
            for section in FRB.WWMISections.blendRemapSections(z3, blendRemap):
                fixSections.sections("fixSections").add(section)
    # GAP: a copy's merge-only sections (WWMIFixer::buildCopyMerges, WWMIFixerConfig::copiesShareSkeleton) -- no
    # mirrored pair sets it
    for i in range(len(tags.tags())):
        fixSections.sections("fixSections").add(tags.section(i, rename("ShaderOverridePass" + str(i))))

    # the other meshes the character draws, by their own vb0 hash (WWMIFixerConfig::sharedMeshes): the geometry is
    # shared, so only the textures are rebound, per pass, with the file of the first component that has the role
    for meshNum, (meshHash, byPass) in enumerate(FIX.sharedMeshes.items()):
        mesh = FRB.SectionText(z3, rename(FRB.IniKeywords.TextureOverride + source.name + "SharedMesh" + str(meshNum)))
        mesh.key(FRB.IniKeywords.Hash, meshHash)
        drew = False
        for passName, bindings in byPass.items():
            lines = [bindLine(binding.role, binding.reg, sharedResourceFor(binding.role)) for binding in bindings
                     if sharedResourceFor(binding.role)]
            if lines:
                mesh.open(passCondition([passName])).keys(lines).close()
                drew = True
        if drew:
            fixSections.sections("fixSections").add(mesh.build())

    # GAP: WWMIFixerConfig::hiddenMeshes (only LynaePeppermint's row sets it)
    # the mod's own sections on another mesh of the source, copied onto the target's mesh that plays the same part
    # (WWMIFixerConfig::meshCounterparts), in this place among the fix's sections
    copyEdits = []
    for part in FIX.meshCounterparts:
        hash = target.hashes.get([target.name, part], toVersion, False)
        if hash is None:
            print("the library files no '" + part + "' hash for " + toModName + ", so the mod's sections on " + SOURCE
                  + "'s are not carried over")
            continue
        copyEdits += copyOntoHash(fixSections, ("", part), parser._sectionTargets.get(("", part), []), hash.lower(),
                                  lambda section: section + " of the mod, on " + toModName + "'s counterpart of that mesh", True)
    copied = [(0, "", part) for part in FIX.meshCounterparts if ("", part) in parser.modObjs]
    if copied:
        copyEdits.append(FRB.GraphRemove(copied))

    # THE MOD'S OWN `ps-t` LINES, RE-KEYED BY ROLE (WWMIFixer's CarriedLines): each to the register this component's
    # plan row reads its texture's role at -- the layout of the fix's own list, which they sit after. A line the rule
    # has no answer for is left where it is; an empty answer drops it. One RegAnchorAdd per component, replacing every
    # `ps-t` line it names
    # GAP: a rerouted component (WWMIFixerConfig::Shader) keeps the mod's own art -- no mirrored pair reroutes
    roleKindOf = lambda role: next((role[i:] for i, char in enumerate(role) if char.isupper()), "")
    sourceComponentOf = lambda obj: slotOf(obj) if not obj[0] else None
    bindsRole = lambda component, role: (any(binding.role == role for binding in FIX.plan[component].bindings)
                                         if component in FIX.plan else False) or role in passOnlyRoles

    def carriedLines(component):
        regOfRole = {}
        for binding in FIX.plan[component].bindings:
            if binding.role != FRB.IniKeywords.Null:
                regOfRole.setdefault(binding.role, binding.reg)

        # the register for a role: the row's own, or the one the row binds the same KIND at, where that is unambiguous
        def regOf(role):
            if role in regOfRole:
                return regOfRole[role]
            kind, byKind = roleKindOf(role), None
            for rowRole, reg in regOfRole.items():
                if not kind or roleKindOf(rowRole) != kind:
                    continue
                if byKind is not None and byKind != reg:
                    return None
                byKind = reg
            return byKind

        # the role the source reads at each register (WWMIFixerConfig::carryByRegisterRole)
        slotRoles = ({reg.lower(): role for reg, role in FIX.sourceTextures.registerRoles.get(component, {}).items()}
                     if FIX.carryByRegisterRole else None)
        # the registers the fix's own list binds (a toggled role's `run =` binds its register too)
        listRegs = {reg.lower() for reg, role in REGISTERS[component]
                    if role == FRB.IniKeywords.Null or resourceFor(role, component)}

        # the line in this one's place: [] drops it, None leaves it alone
        def fateOf(key, value):
            resource = value
            if resource.lower().startswith(FRB.IniKeywords.Ref.lower() + " "):
                resource = resource[len(FRB.IniKeywords.Ref):].lstrip()

            # the role is the parser's answer: one it chose the resource's file for, among the roles the fix binds on a
            # component it draws -- this component's first, then any
            own, role = None, None
            for chosen in data.rolesOfResource(resource.lower()):
                of = sourceComponentOf(chosen.modObj)
                if of is None or of not in present or not bindsRole(of, chosen.role):
                    continue
                if of == component:
                    own = chosen.role
                elif role is None:
                    role = chosen.role
            if own is not None:
                role = own
            # ...or the role the parser refused the file for
            if role is None:
                role = data.refusedRoleOf(resource.lower())

            # a texture put into another kind of slot on purpose goes by the slot
            if role is not None and slotRoles is not None:
                slotRole = slotRoles.get(key)
                if slotRole is None or roleKindOf(slotRole) != roleKindOf(role):
                    slotReg = regOf(slotRole) if slotRole is not None else None
                    if slotReg is not None and slotReg.lower() != key:
                        return [(slotReg, value)]
                    return None

            # left alone where the role or its register is unknown -- unless the fix's list binds that register
            reg = regOf(role) if role is not None else None
            if reg is None:
                return [] if key in listRegs else None

            # a file refused for this role does not go back: the line follows what the role resolved to
            if data.refusedRoleOf(resource) == role:
                resolved = resourceFor(role, component)
                return [(reg, resolved)] if resolved is not None else []
            return [(reg, resource)]

        lines = {}
        for section in data.collectedSections(FRB.IniParseRows.Sections):
            for row in ownRows(section):
                key, value = row.key.strip().lower(), row.value.strip()
                if not key.startswith(TEX_REG.lower()) or (key, value) in lines:
                    continue
                fate = fateOf(key, value)
                if fate is not None:
                    lines[(key, value)] = fate
        return lines

    carriedEdits = {}
    for component in planned:
        lines = carriedLines(component)
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

    # the edits, per group, keyed by the target objects the remap made
    perGroup = []
    for group in groups:
        edits = {}
        for component in group:
            edits[targetObj(component)] = [*([regRemove] if regRemove else []), slotValues[PLAN[component]], assetRemap, dispatch,
                                           *shapeKeyRebind]
            if component in listOf:
                edits[targetObj(component)].append(FRB.RegSurroundedAdd(
                    [(FRB.IniKeywords.Run, listOf[component])], {FRB.IniKeywords.MatchFirstIndex: lambda value: True}))
            # the shared lists are reached through every component's graph; the edits are idempotent
            edits[targetObj(component)] += bypassEdits
            edits[targetObj(component)].append(carriedEdits[component])
            # what runs before the draw, right after the shared-resource override (WWMIFixer::buildComponentAdditions)
            additions = ([(FRB.IniKeywords.Run, FRB.WWMISections.mergeListName(skeleton, PLAN[component]))]
                         if legacy or targetPast256 else []) + \
                ([(FRB.IniKeywords.Run, FRB.WWMISections.blendRemapInitList(blendRemap))] if targetPast256 else []) + \
                ([(FIX.shapeKeyStreamReg, zeroResource)] if zeroResource else []) + \
                ([(FIX.texcoordReg, texcoordResource)] if texcoordResource else []) + \
                ([(FRB.IniKeywords.Run, gameListOf[component])] if component in gameListOf else []) + \
                [(FRB.IniKeywords.Run, passList) for passList in passListsOf.get(component, [])]
            if additions:
                edits[targetObj(component)].append(FRB.RegSurroundedAdd(
                    additions, {FRB.IniKeywords.Run: lambda value: value in (sharedList, rename(sharedList))}, {}, False))
        for obj in hashOnlyObjs:
            edits[obj] = [assetRemap, dispatch, *shapeKeyRebind]
        perGroup.append(edits)
    mainEdits = FRB.GraphGroupEdit(perGroup)

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
    # .ini's resource section rather than a file name (WWMIFixer::vertexVGFile_)
    vertexVGRel = next((file for section, file in data.resourceFiles().items()
                        if "blendremapvertexvg" in section.replace("_", "").lower()), None)
    # how the blend is written (WWMIFixer::buildBlendCollects): a TARGET past 256 bones cannot be named by an 8-bit id,
    # so the fix writes WWMI's blend remap; a source past 256 bones has its ids remapped through BlendRemapVertexVG.buf;
    # otherwise the row runs over Blend.buf itself
    drawRanges = {component: data.drawRanges(obj) for component, obj in objOf.items() if data.drawRanges(obj)}
    indexRel = data.resourceFile(FRB.IniKeywords.WWMIIndexBuffer)
    legacyLift = (FRB.WWMIBlend.LegacyLift(inIni(indexRel), drawRanges, dict(FIX.sourceVgMaps))
                  if legacy and indexRel else None)
    lift, pairedIds = None, None
    if targetPast256:
        out = FRB.WWMIBlend.RemapFiles(*[inIni(modFile(FRB.IniNamingTools.getRemapElementName("", "BlendRemap" + which, toModName) + ".buf"))
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
        blendCollects.append(FRB.ResRegCollect({graph: BLEND_REG for graph in graphs}, {"blend": replace},
                                               resPredicates = {graph: FRB.ResRegCollect.skipsKeywords() for graph in graphs}))
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

    # the fix's own sections, into every file it writes
    writeSections = FRB.GraphCreate((0, "", "fixSections"))
    writeSections.sectionsOf = lambda group: fixSections.sections("fixSections").list()
    writeSections.z3 = z3       # the context the sections' `if` lines were parsed in, kept as long as the edit is
    fixer = FRB.GIMIFixer(parser, graphGroupEdits = [*copyEdits, *toggleEdits, slotRemap, mainEdits, *blendCollects, *rebindEdits, writeSections,
                                                           *madeTextures, *latchEdits, *texEdits],
                          modsToFix = [toModName])
    # a copy carries the fix's own sections and none of the mod's texture overrides live: the mod's own file keeps
    # serving the mod on its own character
    fixer.copyHiddenSectionNames = {section for section in data.collectedSections(FRB.IniParseRows.Sections)
                                    if section.startswith(FRB.IniKeywords.TextureOverride)}
    fixer.copyPreamble = FRB.IniComments.GIMIObjMergerPreamble
    return fixer

# A REMAP AS A LIST OF EDITS -- the acceptance test of the fixer refactor.
#
# The parser CLASSIFIES the mod's sections into graphs; the fixer is ONE list of the library's generic edits, the same for
# every mod. Every parameter is read from the API -- the source's and target's asset tables (hashes, indices, the vertex
# group row) and the pair's data off its builder rows -- and every decision is a step of the list, made with the library's
# readers. What a mod has decides what each edit finds to act on, never what the list is.
#
#   py -3 editList.py <mod folder> <Source> <Target>        (or AGREMAP_LIST_PAIR="Source,Target")
#
# TO TWEAK A FIX, edit the list: SLOT_REMAP is the only place a source object meets its target slot (every step after it
# addresses the target's objects), and LIST is the whole list. Tools/Misc/Prototypes/editListTweaks.py does two tweaks.
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
API = os.path.abspath(os.path.join(HERE, "..", "..", "..", "Anime Game Remap (for all users)", "api"))
if API not in sys.path:
    sys.path.insert(0, API)
import src.py.FixRaidenBoss2 as FRB          # noqa: E402

PAIR = os.environ.get("AGREMAP_LIST_PAIR") or (",".join(sys.argv[2:4]) if __name__ == "__main__" and len(sys.argv) > 3 else None)
if not PAIR:
    raise SystemExit("which pair? editList.py <mod folder> <Source> <Target>, or AGREMAP_LIST_PAIR=\"Source,Target\"")
SOURCE, TARGET = (name.strip() for name in PAIR.split(","))
K = FRB.IniKeywords

# ==================================================================================================== the pair, from the API
# The pair's DATA off its builder rows (WWMIParserConfig / WWMIFixerConfig fields: which slot each source object goes to,
# the registers its texture list binds, the target's passes, the source's texture tables); none of their functions --
# every decision is a step below
SOURCE_TYPE, TARGET_TYPE = FRB.ModTypes[SOURCE].value, FRB.ModTypes[TARGET].value
PARSE = FRB.WWMIParserConfig.ofFactory(SOURCE_TYPE.iniParseBuilder.factoryOf(SOURCE)).withGameData(SOURCE_TYPE)
FIX = FRB.WWMIFixerConfig.ofFactory(SOURCE_TYPE.iniFixBuilder.factoryOf(SOURCE, TARGET))
FROM_VERSION = FRB.Version.parse(FIX.sourceVersion or PARSE.version)
TO_VERSION = FRB.Version.parse(FIX.version)
FIX = FIX.withGameData(SOURCE_TYPE, TARGET_TYPE, FROM_VERSION, TO_VERSION)     # its game data, off the tables
PREFIX = PARSE.slotPrefix
PLAN = {c: entry.slot for c, entry in FIX.plan.items()}                       # source object -> target slot
SAME_FILE = [c for c, entry in FIX.plan.items() if entry.sameFileSection]
REGISTERS = {c: [(b.reg, b.role) for b in entry.bindings] for c, entry in FIX.plan.items()}   # its texture list's lines
HASH_ONLY = [obj for _, obj in PARSE.hashOnlyObjs]
ONCE = ["shapekeyOffsets", "shapekeyScale"]                 # hash-only objects written in the mod's own file only
TEXTURE = FRB.IniGraphModObjKeywords.Texture
ROLES = sorted(set(PARSE.textures.roles.values()))
ROW = SOURCE_TYPE.getVGRemap(TARGET, FROM_VERSION, TO_VERSION)               # the vertex group row
TARGET_BONES = set(ROW.remap.values()) if ROW is not None else set()
# a target past what one id byte names, within what one blend remap holds: the blend is written in local ids with tables
PAST256 = bool(TARGET_BONES) and max(TARGET_BONES) > FRB.WWMIBlend.MaxByteBone and len(TARGET_BONES) <= FRB.WWMIBlend.RemapSize
COMPACT = ROW.compact(FRB.WWMIBlend.RemapSize) if PAST256 else None
REMOVED = list(FIX.removedRegs) + (list(FIX.remappedSkeletonRemovals) if PAST256 else [])   # lines a remapped section drops


def slotsOf(modType, version):
    """Each slot's (match_first_index, match_index_count, vg_offset, vg_count), off the mod type's asset tables"""
    slots, key = [], lambda n: [modType.name, "", PREFIX + str(n)]
    while modType.indices.get(key(len(slots)), version, False) is not None:
        n = len(slots)
        slots.append(tuple(str(table.get(key(n), version, False))
                           for table in (modType.indices, modType.indexCounts, modType.vgOffsets, modType.vgCounts)))
    return slots


SLOTS = slotsOf(TARGET_TYPE, TO_VERSION)
SLOT_KEYS = [K.MatchFirstIndex, K.MatchIndexCount, K.WWMIVgOffset, K.WWMIVgCount]
RENAME = lambda name: FRB.IniNamingTools.getRemapFixName(name, TARGET)          # the fix's name for a section
CAPITAL = lambda role: role[:1].upper() + role[1:]
LIST = {c: RENAME(K.CommandList + SOURCE + "Component" + str(c) + "Textures") for c in PLAN}
TOGGLE_LIST = lambda role, reg: RENAME(K.CommandList + SOURCE + CAPITAL(role) + FRB.IniNamingTools.getRegTag(reg))
CREATED = {t.role: (K.Resource + RENAME(t.role), t.colour, t.size,
                    FRB.IniNamingTools.getModSuffixedName(t.role, K.RemapTex, TARGET) + FRB.FileExt.DDS)
           for t in FIX.createdTextures}

# each role's stand-in download: the source's own game texture (resource, file, url)
TEX = FIX.sourceTextures
DOWNLOADS = {role: (FRB.IniNamingTools.getRemapDLResourceName(TEX.downloadPrefix + CAPITAL(role)),
                    FRB.DownloadTools.fixedFileName(TEX.downloadPrefix, CAPITAL(role), FRB.FileExt.DDS),
                    FRB.DownloadTools.url(TEX.downloadGameFolder, TEX.downloadCharFolder, TEX.downloadVersionFolder,
                                          TEX.downloadPrefix, "Texture" + textureHash, FRB.FileExt.DDS))
             for role, textureHash in TEX.fallbackTextures.items()} if TEX.downloadCharFolder else {}
CHECKSUM = K.WWMIShapeKeyChecksum
HASHES = FRB.RegAssetRemap({K.Hash: (SOURCE_TYPE.hashes, K.HashNotFound),
                            **{CHECKSUM + s: (SOURCE_TYPE.shapeKeyChecksums, FRB.WWMINames.ChecksumNotFound) for s in ("", "_batch0", "_batch1")}},
                           TARGET, SOURCE, FROM_VERSION, TO_VERSION)
DISPATCH = ({K.WWMIShapeKeyDispatchSizeYOriginalBatch + n: FIX.shapeKeyDispatchSize for n in "01"}
            if FIX.shapeKeyDispatchSize else {})

# the passes each target slot is drawn on, tagged by number: a list's guard, and the ShaderOverride sections that tag them
Z3 = FRB.Z3Context()
TAGS = FRB.ShaderFilterTags()
TAGGED = lambda shader: list(FIX.passVertexShaders[shader]) if FIX.passVertexShaders else [shader]
TAGS.addNumbered([vs for passes in FIX.slotPasses for shader in passes for vs in TAGGED(shader)], dict(FIX.filterIndices),
                 FIX.filterBase, FIX.filterStep)
GUARD = {slot: TAGS.condition("vs" if FIX.passVertexShaders else "ps", list(dict.fromkeys(t for p in passes for t in TAGGED(p))))
         for slot, passes in enumerate(FIX.slotPasses)}
RESTORE = RENAME(K.CommandList + "RestoreSharedResources")
BYPASSES = [(reg, K.Resource + RENAME("Bypass" + reg.capitalize())) for reg in [K.Ib, "vb1", "vb2", "vb3", "vb4"]]

# ==================================================================================================== the graphs, by id
SRC = lambda c: ("", PREFIX + str(c))                                   # a source object's graph
DST = lambda slot: (TARGET, PREFIX + str(slot))                         # a target slot's graph
# where a source object lands: its target slot -- or, drawn as a second section on that slot in the mod's own file
# (SAME_FILE), an object of its own, so the two do not make a copy file
TOBJ = lambda c: (TARGET, PREFIX + str(PLAN[c]) + "_" + str(c)) if c in SAME_FILE else DST(PLAN[c])
TARGET_OBJS = [(DST(slot), slot) for slot in range(len(SLOTS))] + [(TOBJ(c), PLAN[c]) for c in SAME_FILE]
SLOT_OF = {obj: slot for obj, slot in TARGET_OBJS}                     # a target object's slot
ROLE = lambda c, role: (PREFIX + str(c), role)                          # the resource a source object takes for a role
TOGGLE = lambda c, role: ("toggle" + str(c), role)                      # ...or the mod's toggle that chooses it
REF = lambda c, role: ("ref", role + str(c))                            # ...a file the fix declares a resource for
LISTG = lambda c: ("list", str(c))                                       # an object's texture list
FIX_SECTIONS, SKELETON = ("", "fixSections"), ("", "skeleton")
RESETS = ("", "resetsDetected")              # a copy of the mod's [Present] line that resets $object_detected, if any
IN_SECTION = lambda name: (lambda sectionName: sectionName.lower() == name.lower())
SOURCES = [SRC(c) for c in PLAN]


# ======================================================================================================== parser
# THE PARSER CLASSIFIES, AND NOTHING ELSE: a GIMIParser whose classifier sorts the .ini's sections into the graphs the fix
# needs -- the source's objects (by its index table), one object per hash-only type and per texture role (by its hash
# table), every other texture override, and the sections that only watch the source's object hash. It is built once per
# version a mod is read at (CLASSIFIERS); the factory only hands it the .ini
TEXTURE_ANY, WATCH = (TEXTURE, ""), ("", "watch")
HASH_TYPES = {}                          # each hash type of the source's table -> every hash it has had
for _, names in SOURCE_TYPE.hashes.repo.toNestedDict().items():
    for hashType, value in names.get(SOURCE, {}).items():
        HASH_TYPES.setdefault(hashType, set()).add(value.lower())
HASH_ONLY_TYPES = {hashType: ("", obj) for hashType, obj in PARSE.hashOnlyObjs}
TEXTURE_TYPES = {hashType: (TEXTURE, role) for hashType in HASH_TYPES if hashType not in HASH_ONLY_TYPES
                 for role in [FRB.WWMITextureFacts.roleOfHashType(hashType, set(ROLES))] if role is not None}
WATCHED = HASH_TYPES.get(PARSE.slotHashType, set())
isFixSection = lambda name: name.lower().endswith(K.RemapFix.lower())
CLASSIFIERS = {}                         # version -> (its objects, its classify function)


def classifierOf(version):
    """The objects at 'version' -- the source's (one per row of its index table), the hash-only, the texture ones -- and
    which of them a section is: WATCH when it only watches the source's object hash (no match_first_index); else what the
    library's classifier says (GIMISectionClassifier, over the source's hash and index tables); else TEXTURE_ANY for a
    texture override of the mod's own"""
    if version in CLASSIFIERS:
        return CLASSIFIERS[version]
    slots = []
    while SOURCE_TYPE.indices.get([SOURCE, "", PREFIX + str(len(slots))], version, False) is not None:
        slots.append(("", PREFIX + str(len(slots))))
    objs = list(dict.fromkeys(slots + list(HASH_ONLY_TYPES.values()) + list(TEXTURE_TYPES.values()) + [TEXTURE_ANY, WATCH]))
    classifier = FRB.GIMISectionClassifier({**HASH_ONLY_TYPES, **TEXTURE_TYPES}, SOURCE_TYPE.hashes,
                                           {PARSE.slotHashType: {obj: obj for obj in slots}},
                                           SOURCE_TYPE.indices, version, [SOURCE, None], [SOURCE, None, None])

    def classify(parser, name, section, disjoint, part, kvps):
        if kvps is None:
            return []
        hashes = part.getVals(K.Hash)
        drawsAt = section.firstVal(K.MatchFirstIndex) is not None
        if hashes and hashes[0].strip().lower() in WATCHED and not drawsAt:
            return [WATCH]
        found = classifier.classify(name, section, kvps)
        if not found and name.startswith(K.TextureOverride) and not isFixSection(name) and hashes and not drawsAt:
            found = [TEXTURE_ANY]
        return found
    CLASSIFIERS[version] = (objs, classify)
    return CLASSIFIERS[version]


def makeParser(iniFile, modTypeId, commandEdits = None):
    """The parser of a mod's .ini: its classification ('commandEdits': a list it runs as it parses -- READ_LINKED, for a
    linked .ini no fixer runs on)"""
    objs, classify = classifierOf(iniFile.fromVersion if iniFile.fromVersion is not None else FRB.Version.parse(PARSE.version))
    return FRB.GIMIParser(iniFile, modObjs = objs, objTargetFuncs = [classify], disjointModObjs = False,
                          keysToTrack = {K.Hash, K.MatchFirstIndex, K.MatchIndexCount}, modTypeId = modTypeId,
                          commandEdits = commandEdits)


# ======================================================================================================== READ
# The first steps of the fixer's list: what the mod's .ini says, filed into its parse data (FRB.EditScope.parseData()),
# where the library's readers (IniParseData.resourceFile, isUndrawn, roleFile, resourceOfFile, ...) and later steps read it.
#   1. the whole file as a graph (its sections, FRB.EditScope.ini())
#   2. the standard rows (IniParseRows): the resources a line binds and their files, each section's declarations, every key
#      of every section, every key of each source object
#   3. each object's hash / this / match_priority, the roles the .ini states, the toggles, what a linked file carries
#   4. the resource data (IniParseData.resourceDataOf) and 5. each object's file per role (IniParseData.roleFilesOf)
GLOBAL, RESOURCES = (0, "global", ""), (0, "resource", "")
ROWS = {"overrides": FRB.WWMINames.TextureOverridesRows, "roles": FRB.WWMINames.TextureRolesRows,
        "toggles": FRB.WWMINames.TextureTogglesRows, "carried": FRB.WWMINames.CarriedRows}
FACTS = PARSE.textures
REGISTER_ROLES = {slot: dict(table) for slot, table in FACTS.registerRoles.items()}
NOT_TOGGLES = [K.WWMIObjectDetected, K.WWMIObjectDetected + "==1"]          # conditions that choose no texture
graphOf = lambda obj: (0,) + obj
data = lambda: FRB.EditScope.parseData()                      # the parse data of the .ini the list runs on
ini = lambda: FRB.EditScope.ini()                             # ...and the .ini
slotOf = lambda obj: int(obj[1][len(PREFIX):]) if obj[0] == "" and obj[1].startswith(PREFIX) and obj[1][len(PREFIX):].isdigit() else None


def roots(rows, keep):
    """Each object's root sections -- the sections of its hash lines -- in the order the .ini writes them"""
    written = {name: i for i, name in enumerate(ini().getSectionNames())}
    result = {}
    for row in rows:
        obj = tuple(row.graphId[1:])
        if keep(obj) and row.key.strip().lower() == K.Hash:
            result.setdefault(obj, []).append(row.section)
    return [(obj, sorted(dict.fromkeys(sections), key = lambda name: written.get(name, len(written))))
            for obj, sections in sorted(result.items(), key = lambda item: str(item[0]))]


def resourceData(rows, linked):
    """IniParseData.resourceDataOf, with the source's rules"""
    rules = FRB.IniParseData.ResourceDataRules()
    rules.rows, rules.resources, rules.sections = FRB.IniParseRows.Objs, FRB.IniParseRows.Resources, FRB.IniParseRows.Sections
    rules.texRegPrefix = PARSE.texRegPrefix
    rules.noneUndrawnWhenAll = True
    rules.libraries = [(FRB.RabbitFX.Name, FRB.RabbitFX.names)]
    rules.runDraws = lambda called: ini().getSection(called) is None and K.WWMINamespace.lstrip("$").lower() + "\\" not in called.lower()
    overrides = rows[ROWS["overrides"]] if ROWS["overrides"] in rows else []
    rules.roots = roots(overrides, lambda obj: slotOf(obj) is not None)
    if not linked:
        rules.toggles = ROWS["toggles"]
        rules.togglesIn = lambda section: section.startswith(K.TextureOverride) and not isFixSection(section)
        rules.notToggles = NOT_TOGGLES
        rules.textureRoots = roots(overrides, lambda obj: obj[0] == TEXTURE)
    start = FRB.IniParseData.ResourceData()
    slotSections = {name for _, sections in rules.roots for name in sections}
    has = {}
    for row in overrides:
        if row.section.startswith(K.TextureOverride) and row.section not in slotSections:
            has.setdefault(row.section, set()).add(row.key)
    start.replacesTexturesByHash = any({K.Hash, K.This} <= keys for keys in has.values())
    return FRB.IniParseData.resourceDataOf(rules, rows, start)


def roleFiles(rows):
    """IniParseData.roleFilesOf, with the source's rules: by hash (the newest generation first, Hashes.age), by register,
    by a linked .ini (the run's own copy of it, read by READ_LINKED), by a file's pixels"""
    rules = FRB.IniParseData.RoleFileRules()
    rules.iniPath, rules.folder = ini().file, ini().folder
    rules.sections, rules.declared = FRB.IniParseRows.Sections, FRB.IniParseRows.Declared
    rules.overrides, rules.roleRows, rules.objRows, rules.carriedRows = ROWS["overrides"], ROWS["roles"], FRB.IniParseRows.Objs, ROWS["carried"]
    rules.objs = roots(rows[ROWS["overrides"]], lambda obj: slotOf(obj) is not None)
    rules.registerRoles = {("", PREFIX + str(slot)): table for slot, table in REGISTER_ROLES.items()}
    rules.regionRoles = FACTS.regionRoles
    rules.notToggles = [condition.lower() for condition in NOT_TOGGLES]
    knownRoles = set(FACTS.roles.values()) | {role for table in REGISTER_ROLES.values() for role in table.values()}
    rules.roleOfHash = lambda hash: next((hashType for hashType, hashes in HASH_TYPES.items()
                                         if hash in hashes and hashType in knownRoles), FACTS.roles.get(hash))
    rules.identify = lambda file: FRB.TexThumbprint.identifyFile(file, FACTS.textureThumbprints, FACTS.thumbprintSize,
                                                                 FACTS.identityMin, FACTS.identityGap)
    rules.hashAge = lambda hash: SOURCE_TYPE.hashes.age(SOURCE, hash)
    fixNames = [SOURCE_TYPE.name, *SOURCE_TYPE.getModsToFix()]
    rules.fixSection = lambda section: FRB.IniNamingTools.looksRemapped(section, fixNames)
    rules.log = print
    readers = []                                 # the linked files and their parsers, alive while roleFilesOf reads them
    if rules.objs and rules.iniPath:
        source = ini()
        for linked in source.linkedInis():
            sibling = source.siblingOf(linked, {getattr(FRB.ModTypeId, SOURCE)})
            reader = makeParser(sibling, getattr(FRB.ModTypeId, SOURCE), READ_LINKED)
            reader.parse()
            readers.append((sibling, reader))
            rules.linked = [*rules.linked, FRB.IniParseData.RoleFileRules.Linked(reader.parseData, sibling.folder,
                                                                                  os.path.basename(linked.file), linked.file)]
    return FRB.IniParseData.roleFilesOf(rules, rows)


def readSteps(linked):
    """The reading steps; 'linked': for a linked .ini -- its rows, its resource data and what it carries"""
    anyKey = lambda key: True
    slots = list(dict.fromkeys(classifierOf(FROM_VERSION)[0] + classifierOf(FRB.Version.parse(PARSE.version))[0]))
    objs = [obj for obj in slots if slotOf(obj) is not None]
    allObjs = list(dict.fromkeys(slots))
    roleKeys, toggleKeys = {}, {}
    for obj in allObjs:
        if obj[0] == TEXTURE:
            if obj[1] != "":
                roleKeys[graphOf(obj)] = [K.This]
            toggleKeys[graphOf(obj)] = lambda key: key.strip().lower() == K.This or key.strip().lower().startswith(PARSE.texRegPrefix)
        elif slotOf(obj) in REGISTER_ROLES:
            roleKeys[graphOf(obj)] = lambda key, regs = list(REGISTER_ROLES[slotOf(obj)]): any(FRB.IniNamingTools.sameKey(key, reg) for reg in regs)

    steps = [
        # 1-2: the whole file, and the standard rows off it; the source objects' rows
        FRB.GraphCreate(GLOBAL, sectionsOf = lambda group: list(ini().getIfTemplates().values()) if group == 0 else [], z3 = Z3),
        FRB.ResRegCollect({GLOBAL: anyKey}, {"": FRB.ResIdentity(RESOURCES, createResModel = False, fileOf = lambda value: ini().resourceNamed(value))},
                          resPredicates = {GLOBAL: lambda reg, value, iterData: ini().resourceNamed(value) != ""}),
        FRB.GraphCollect({RESOURCES: [K.Filename]}, name = FRB.IniParseRows.Resources),
        FRB.GraphCollect({GLOBAL: [K.Filename, K.Type, K.Format, K.Stride, K.Global + " " + K.WWMIMeshVertexCount]},
                         name = FRB.IniParseRows.Declared),
        FRB.GraphCollect({GLOBAL: anyKey}, name = FRB.IniParseRows.Sections),
        FRB.GraphRemove([RESOURCES]),
        FRB.GraphCollect({graphOf(obj): anyKey for obj in objs}, name = FRB.IniParseRows.Objs),

        # 3: what each object's sections say
        FRB.GraphCollect({graphOf(obj): [K.Hash, K.This, K.MatchPriority] for obj in allObjs}, name = ROWS["overrides"]),
        FRB.GraphCollect(roleKeys, name = ROWS["roles"], withConditions = True),
        FRB.GraphCollect(toggleKeys, name = ROWS["toggles"]),
        FRB.GraphCollect({graphOf(obj): (lambda key: FRB.IniNamingTools.sameKey(key, FRB.RabbitFX.GlowMapKey))
                          for obj in (objs if linked and (ROLES or REGISTER_ROLES) else [])},
                         name = ROWS["carried"], withConditions = True)]

    # 4-5: the resource data, the files per role
    resourceInputs = [FRB.IniParseRows.Objs, FRB.IniParseRows.Resources, FRB.IniParseRows.Sections, ROWS["toggles"], ROWS["overrides"]]
    steps.append(FRB.GraphVarApply(resourceInputs, lambda *inputs: resourceData(FRB.GraphEditVariables.of(resourceInputs, list(inputs)), linked),
                                   FRB.IniParseData.ResourceDataVar))
    if linked:
        return steps + [FRB.GraphRemove([GLOBAL])]
    roleInputs = [FRB.IniParseRows.Sections, FRB.IniParseRows.Declared, ROWS["overrides"], ROWS["roles"], FRB.IniParseRows.Objs,
                  FRB.IniParseData.ResourceDataVar]
    return steps + [FRB.GraphVarApply(roleInputs, lambda *inputs: roleFiles(FRB.GraphEditVariables.of(roleInputs, list(inputs))),
                                      FRB.IniParseData.RoleFilesVar)]


READ_LINKED = readSteps(True)
READ = readSteps(False)


# ======================================================================================================== ROLES
# 6. WHICH RESOURCE EACH (source object, role) TAKES, filed as the variable "roleResources": per object the mod draws, in
#    the plan's order, the role's file (IniParseData.roleFile) is
#      ("own", resource)            -- through the resource the mod itself uses for it (IniParseData.resourceOfFile)
#      ("ref", resource, rel, first) -- else through one the fix declares, named once per file; 'first' declares it
#    and a role no file of the mod's serves takes its stand-in: a texture the fix makes (CREATED), else the source's own
#    game texture, downloaded (DOWNLOADS) -- under the role alone, as ("standIn", role)
RESOLVED = "roleResources"
resolved = lambda: data().variables()[RESOLVED]


def resolveRoles(*_):
    drawn = {slotOf(obj) for obj, sections in roots(data().collected(ROWS["overrides"]), lambda obj: slotOf(obj) is not None)
             if sections and not data().isUndrawn(obj)}
    result, declared, names = {}, {}, set()
    for c, bindings in REGISTERS.items():
        for _, role in (bindings if c in drawn else []):
            chosen = data().roleFile(role, SRC(c))
            if chosen is None or (c, role) in result:
                continue
            own = data().resourceOfFile(chosen.file)
            if own is not None and not FRB.IniNamingTools.looksRemapped(own, [SOURCE, TARGET]):
                result[(c, role)] = ("own", own)
                continue
            if chosen.file not in declared:
                name, n = K.Resource + FRB.IniNamingTools.getModSuffixedName(CAPITAL(role), K.RemapRef, TARGET), 2
                while name in names:
                    name, n = K.Resource + FRB.IniNamingTools.getModSuffixedName(CAPITAL(role) + str(n), K.RemapRef, TARGET), n + 1
                names.add(name)
                declared[chosen.file] = (name, os.path.relpath(chosen.file, ini().folder), (c, role))
            name, rel, first = declared[chosen.file]
            result[(c, role)] = ("ref", name, rel, first == (c, role))
    standIns = {role: resource for role, (resource, *_) in CREATED.items()}
    for c, bindings in REGISTERS.items():
        for _, role in (bindings if c in drawn else []):
            if (c, role) not in result and role not in standIns and role in DOWNLOADS:
                standIns[role] = DOWNLOADS[role][0]
    return {"of": result, "standIns": standIns}


own = lambda c, role: (lambda found: found[1] if found and found[0] == "own" else None)(resolved()["of"].get((c, role)))
toggle = lambda c, role: (lambda resource: data().thisToggleOf(resource.lower()) if resource else None)(own(c, role))


def roleSections(c, role):
    """The section a role's resource is when the mod has none of its own: the declared RemapRef, or the stand-in"""
    found = resolved()["of"].get((c, role))
    if found is not None and found[0] == "ref":
        return [FRB.SectionText(Z3, found[1]).key(K.Filename, found[2]).build()]
    if found is None and role in resolved()["standIns"]:
        return [FRB.SectionText(Z3, resolved()["standIns"][role]).build()]
    return None


def refSections(c, role):
    """A RemapRef to write: once per file, by its first claimant"""
    found = resolved()["of"].get((c, role))
    return [FRB.SectionText(Z3, found[1]).key(K.Filename, found[2]).build()] if found and found[0] == "ref" and found[3] else None


ROLE_STEPS = [FRB.GraphVarApply([FRB.IniParseData.RoleFilesVar], resolveRoles, RESOLVED)]
for _c, _bindings in REGISTERS.items():
    for _, _role in _bindings:
        ROLE_STEPS += [
            # the mod's own resource, unless the mod toggles it
            FRB.ResRegCollect({GLOBAL: lambda key: True},
                              {"": FRB.ResIdentity((0,) + ROLE(_c, _role), createResModel = False, fileOf = lambda value: ini().resourceNamed(value))},
                              resPredicates = {GLOBAL: lambda reg, value, iterData, c = _c, role = _role:
                                               ini().resourceNamed(value) != "" and value.strip() == own(c, role) and toggle(c, role) is None}),
            # ...the toggle that chooses it
            FRB.GraphCopy((0, TEXTURE, _role), (0,) + TOGGLE(_c, _role),
                          lambda owner, part, i, c = _c, role = _role: (lambda found: found is not None and owner.name == found.section
                                                                        and part.getByInd(i)[0].lower() not in K.MatchKeys)(toggle(c, role)),
                          keepEmpty = False),
            # ...else the RemapRef or the stand-in
            FRB.GraphCreate((0,) + ROLE(_c, _role), sectionsOf = lambda group, c = _c, role = _role: roleSections(c, role), z3 = Z3),
            FRB.GraphCreate((0,) + REF(_c, _role), sectionsOf = lambda group, c = _c, role = _role: refSections(c, role), z3 = Z3)]

# whether the mod's [Present] resets $object_detected (a copy of that line); an object whose section draws nothing
# (IniParseData.isUndrawn) is no part of the fix, nor is any object of a mod with no vg_offset line (its blend's ids are
# not the ones the row maps)
merged = lambda: any(row.key.lower() == K.WWMIVgOffset.lower() for row in data().collected(FRB.IniParseRows.Objs))
ROLE_STEPS += [
    FRB.GraphCopy(GLOBAL, (0,) + RESETS, lambda owner, part, i: owner.name.lower() == K.Present.lower()
                  and part.getByInd(i)[0].lower().startswith((K.Post + " " + K.WWMIObjectDetected).lower())
                  and part.getByInd(i)[1].strip() == "0", keepEmpty = False),
    FRB.GraphRemove([GLOBAL]),
    *[FRB.GraphFilter([(0,) + SRC(c)], lambda owner, part, i, c = c: not data().isUndrawn(SRC(c)) and merged(), removeEmptied = True)
      for c in PLAN],
    FRB.GraphFilter([(0, "", obj) for obj in HASH_ONLY], lambda owner, part, i: merged(), removeEmptied = True)]


# ======================================================================================================== the blend
# a target past what one id byte names: the row in LOCAL ids of a table (COMPACT), and every vertex's bone ids at their full
# width, the Forward / Reverse tables beside the blend
def besideBlend(blendPath, which):
    """A buffer of the blend remap's, beside the mod's blend"""
    return os.path.join(os.path.dirname(blendPath), FRB.IniNamingTools.getRemapElementName("", FRB.WWMINames.BlendRemap + which, TARGET) + ".buf")


def writeBlendRemap(resource):
    """The blend through the row in LOCAL ids (resource.vgRemap, anchor chains applied); the vertex group ids through the
    row at 16 bits; and the Forward / Reverse tables -- each a BufFile written by its fix. It runs when the service fixes
    the resources, after the list: everything it reads is the resource's own (its blend's layout, its paths)"""
    n = resource.createBlend().influences()
    elements = FRB.BlendFile.byteElements(n)
    FRB.BlendFile(resource.srcPath, elements).remap(resource.vgRemap, resource.fixedPath)
    ids = FRB.BlendFile(bytes(FRB.BlendFile(resource.srcPath, elements).remap(ROW) or FRB.BinaryFile(resource.srcPath).read()),
                        elements).rows("BLENDINDICES", n)
    _, forward, reverse = COMPACT
    for which, rows, width in (("VertexVG", ids, n), ("Forward", [[v] for v in forward], 1), ("Reverse", [[v] for v in reverse], 1)):
        os.makedirs(os.path.dirname(besideBlend(resource.srcPath, which)), exist_ok = True)
        FRB.CppBufFile(bytes(2 * width * len(rows)), FRB.BlendFile.idElements(width, 2)).fix(
            besideBlend(resource.srcPath, which), [lambda line, start, index, size, rows = rows: {"BLENDINDICES": rows[int(index)]}])
    return True


def influences():
    """How many influences a vertex the MOD's blend holds: the blend's size over the vertex count (the position buffer's
    size over the stride its resource section declares), else 4. (The target's width is VGRemapBlendReplace's
    'influences', which widens or narrows the remapped blend to it)"""
    rel = lambda path: os.path.normpath(os.path.join(ini().folder, path.strip().replace("\\", os.sep)))
    blend, position = data().resourceFile(K.WWMIBlendBuffer), data().resourceFile(K.WWMIPositionBuffer)
    stride = data().firstCollected(FRB.IniParseRows.Declared, K.WWMIPositionBuffer, K.Stride)
    vertices = FRB.CppBufFile.linesOf(rel(position), int(stride.value)) if position and stride is not None and stride.value.strip().isdigit() else None
    return (FRB.BlendFile.influencesOf(rel(blend), vertices) if blend and vertices else None) or 4


# ======================================================================================================== the runtime's sections
# What the target's runtime needs that the mod has none of, written line by line (SectionText) from the pair's values:
# the section of a slot nothing is remapped onto, the bypass of the shared lists, and -- for a target past what one id
# byte names -- the merged skeleton, its remap, a merge list per slot and the blend remap. (The mod's own skeleton
# sections are a different scheme -- one remap per component -- so they are not copied.)
DETECTED, FILTER = K.WWMIObjectDetected, FIX.boneDataFilter
MERGE_LIST = {slot: RENAME(K.CommandList + "MergeSlot" + str(slot)) for slot in range(len(SLOTS))}
MERGE_WINDOW = RENAME(FRB.WWMINames.MergeWindowList)
REMAP_SKELETON = RENAME(K.CommandList + "RemapMergedSkeleton")
BLEND_INIT = RENAME(K.CommandList + "BlendRemap")
SK = {name: RENAME(K.Resource + name) for name in ("MergedSkeleton", "MergedSkeletonRW", "ExtraMergedSkeleton", "ExtraMergedSkeletonRW",
                                                   "RemappedSkeleton", "RemappedSkeletonRW", "ExtraRemappedSkeleton",
                                                   "ExtraRemappedSkeletonRW", "BlendRemapForwardBuffer")}
# per bone-data constant buffer: (register, merged RW, merged, remapped RW, remapped)
CBS = [(K.WWMIBoneDataCb, SK["MergedSkeletonRW"], SK["MergedSkeleton"], SK["RemappedSkeletonRW"], SK["RemappedSkeleton"]),
       (K.WWMIExtraBoneDataCb, SK["ExtraMergedSkeletonRW"], SK["ExtraMergedSkeleton"], SK["ExtraRemappedSkeletonRW"], SK["ExtraRemappedSkeleton"])]
SCALE = "1.00"                       # the mesh scale the merger runs at
CB3_ONLY = "$cb3_only"               # the fix's flag: this frame's bones came only in vs-cb3
COPY, REF_ = K.Copy + " ", K.Ref + " "


def hideSection(slot):
    """A target slot nothing is remapped onto: the skin's own geometry skipped, its bones still merged"""
    first, count, vgOffset, vgCount = SLOTS[slot]
    text = (FRB.SectionText(Z3, K.TextureOverride + TARGET + "Component" + str(slot) + K.Remap + "Hide")
            .prefix("; nothing of the mod is drawn through " + TARGET + "'s " + FIX.targetLabels.get(slot, str(slot))
                    + " slot: the skin's own geometry is skipped and its bones still merged")
            .keys([(K.Hash, TARGET_TYPE.hashes.get([TARGET, PARSE.slotHashType], TO_VERSION, False)),
                   (K.MatchFirstIndex, first), (K.MatchIndexCount, count)])
            .key(DETECTED, "1")
            .open(K.WWMIModEnabled))
    if PAST256:
        text = text.keys([(K.WWMIVgOffset, vgOffset), (K.WWMIVgCount, vgCount), (K.Run, MERGE_WINDOW)])
    else:
        state = K.WWMIStateId + "_" + str(slot)
        text = (text.key(K.Local + " " + state).open(state + " != " + K.WWMIStateId)
                .keys([(state, K.WWMIStateId), (K.WWMIVgOffset, vgOffset), (K.WWMIVgCount, vgCount),
                       (K.Run, RENAME(K.CommandList + "MergeSkeleton"))]).close())
    return text.open(K.Resource + "MergedSkeleton !== " + K.Null).key(K.Handling, K.Skip).close().close().build()


def bypassSections():
    """A resource per shared buffer the fix parks, and the list that puts them back"""
    restore = FRB.SectionText(Z3, RESTORE)
    for reg, bypass in BYPASSES:
        restore = restore.key(reg, REF_ + bypass)
    return [FRB.SectionText(Z3, bypass).build() for _, bypass in BYPASSES] + [restore.build()]


def skeletonSections(latched):
    """The merged skeleton (its buffers and read-only copies, the marker on the bone-data buffer), its remap into the
    target's numbering once a frame, the merge of a hidden slot's window, and a merge list per slot. 'latched': the
    remap runs on $object_detected (LATCH) rather than at the top of every merge list"""
    sections = [FRB.SectionText(Z3, SK["MergedSkeleton"]).build(), FRB.SectionText(Z3, SK["ExtraMergedSkeleton"]).build()]
    sections += [FRB.SectionText(Z3, name).keys([(K.Type, K.RWBuffer), (K.Format, K.FormatR32G32B32A32Float),
                                                 (K.Array, str(FIX.mergedSkeletonSlots))]).build()
                 for name in (SK["MergedSkeletonRW"], SK["ExtraMergedSkeletonRW"])]
    sections.append(FRB.SectionText(Z3, RENAME(K.TextureOverride + "MarkBoneDataCB"))
                    .keys([(K.Hash, (TARGET_TYPE.hashes.get([TARGET, "cb4"], TO_VERSION, False) or "").lower()),
                           (K.MatchPriority, "0"), (K.FilterIndex, FILTER)]).build())
    sections += [FRB.SectionText(Z3, SK[name]).build()
                 for name in ("RemappedSkeleton", "ExtraRemappedSkeleton", "RemappedSkeletonRW", "ExtraRemappedSkeletonRW")]

    remap = FRB.SectionText(Z3, REMAP_SKELETON)
    for _, rw, merged, remappedRW, remapped in CBS:
        remap = remap.keys([(merged, COPY + rw), (K.WWMIRemapperForwardReg, SK["BlendRemapForwardBuffer"]), (K.WWMIBlendRemapId, "0"),
                            (K.WWMIVgCount, str(len(TARGET_BONES))), (K.WWMIRemapperSkeletonReg, merged), (remappedRW, COPY + rw),
                            (K.WWMIRemapperOutReg, remappedRW), (K.Run, K.WWMISkeletonRemapper), (remapped, COPY + remappedRW)])
    sections.append(remap.build())

    window = FRB.SectionText(Z3, MERGE_WINDOW).key(K.WWMICustomMeshScale, SCALE)
    for cb, rw, *_ in CBS:
        window = window.open(cb + " == " + FILTER).keys([(K.WWMIMergerBonesReg, REF_ + cb), (K.WWMIMergerSkeletonReg, rw),
                                                          (K.Run, K.WWMISkeletonMerger)]).close()
    sections.append(window.build())

    cb3Only, extra = FIX.currentPoseInCb3Only, K.WWMIExtraBoneDataCb
    for slot, (_, _, vgOffset, vgCount) in enumerate(SLOTS):
        merges = lambda cb, rw: [(K.WWMIVgOffset, vgOffset), (K.WWMIVgCount, vgCount), (K.WWMICustomMeshScale, SCALE),
                                 (K.WWMIMergerBonesReg, REF_ + cb), (K.WWMIMergerSkeletonReg, rw), (K.Run, K.WWMISkeletonMerger)]
        text = FRB.SectionText(Z3, MERGE_LIST[slot])
        if not latched:
            text = text.key(K.Run, REMAP_SKELETON)
        if cb3Only:
            text = (text.key(K.Local + " " + CB3_ONLY).key(CB3_ONLY, "0")
                    .open(extra + " == " + FILTER + " && " + K.WWMIBoneDataCb + " != " + FILTER).key(CB3_ONLY, "1").close())
        for cb, rw, _, _, remapped in CBS:
            condition = cb + " == " + FILTER + (" && " + CB3_ONLY + " == 0" if cb == extra and cb3Only else "")
            text = text.open(condition).keys(merges(cb, rw)).key(cb, remapped).close()
        if cb3Only:
            text = text.open(CB3_ONLY + " == 1").keys(merges(extra, SK["MergedSkeletonRW"])).key(extra, SK["RemappedSkeleton"]).close()
        if FIX.bindPrevPoseAlways:
            text = (text.open(CB3_ONLY + " == 0").key(extra, SK["ExtraRemappedSkeleton"]).close() if cb3Only
                    else text.key(extra, SK["ExtraRemappedSkeleton"]))
        sections.append(text.build())
    return sections


def blendRemapSections():
    """The blend remap's buffers, and the one-time run of the runtime's BlendRemapper that rewrites the remapped blend's
    ids into the remap's LOCAL space"""
    name = lambda n: RENAME(K.Resource + n)
    vertexVG, forward, reverse = name("BlendRemapVertexVGBuffer"), name("BlendRemapForwardBuffer"), name("BlendRemapReverseBuffer")
    noStride, strided, blendRW, blendOut = name("BlendNoStride"), name("BlendStrided"), name("RemappedBlendBufferRW"), name("RemappedBlendBuffer")
    blendRel = data().resourceFile(K.WWMIBlendBuffer)
    meshFolder = os.path.dirname(blendRel.replace("\\", "/")) if blendRel else ""
    rel = lambda which: "/".join(part for part in (meshFolder, FRB.IniNamingTools.getRemapElementName(
        "", FRB.WWMINames.BlendRemap + which, TARGET) + ".buf") if part)
    blendFile = FRB.IniNamingTools.getFixedBlendFile(blendRel, TARGET) if blendRel else ""
    n = influences()
    sections = [FRB.SectionText(Z3, resource).keys([(K.Type, K.Buffer), (K.Format, K.FormatR8UInt if resource == noStride else K.FormatR16UInt),
                                                    (K.Filename, file)]).build()
                for resource, file in ((vertexVG, rel("VertexVG")), (forward, rel("Forward")), (reverse, rel("Reverse")),
                                       (noStride, blendFile))]
    sections.append(FRB.SectionText(Z3, strided).keys([(K.Type, K.Buffer), (K.Format, K.FormatR8UInt),
                                                       (K.Stride, str(n * 2)), (K.Filename, blendFile)]).build())
    sections += [FRB.SectionText(Z3, blendRW).build(), FRB.SectionText(Z3, blendOut).build()]
    ready = "$blendRemapReady"
    sections.append(FRB.SectionText(Z3, BLEND_INIT).key(K.Local + " " + ready).open("!" + ready)
                    .keys([(K.WWMICustomVertexCount, K.WWMIMeshVertexCount), (K.WWMIWeightsPerVertexCount, str(n)),
                           (K.WWMIBlendRemapId, "0"), (K.WWMIBlendRemapperReverseReg, REF_ + reverse),
                           (K.WWMIBlendRemapperVertexVGReg, REF_ + vertexVG), (blendRW, COPY + noStride),
                           (K.WWMIBlendRemapperOutReg, REF_ + blendRW), (K.Run, K.WWMIBlendRemapper),
                           (blendOut, COPY + blendRW), (blendOut, K.CopyDesc + " " + strided), (ready, "1")])
                    .close().key(FIX.blendReg, blendOut).build())
    return sections


MERGES = {name.lower() for name in [MERGE_WINDOW] + list(MERGE_LIST.values())}
RUNS_MERGE = lambda name, section: any(value.strip().lower() in MERGES for part in section.parts if hasattr(part, "getVals")
                                       for value in part.getVals(K.Run))
# the frame's skeleton remap, before a slot's `$object_detected = 1`, else before its first `if`, where a merge list runs
LATCH = [FRB.RegIfTemplateAdd(FRB.SectionText(Z3, "").open(DETECTED + " == 0").key(K.Run, REMAP_SKELETON).close().build(),
                              FRB.RegIfTemplateAdd.Position.BeforeTopKey, DETECTED, value = "1", everySection = True, sectionFilter = RUNS_MERGE),
         FRB.RegIfTemplateAdd(FRB.SectionText(Z3, "").open(DETECTED + " == 0").key(K.Run, REMAP_SKELETON).close().key(DETECTED, "1").build(),
                              FRB.RegIfTemplateAdd.Position.BeforeFirstIf, everySection = True, sectionFilter = RUNS_MERGE)]


# ======================================================================================================== the fixer's steps
# THE SLOT REMAP: each source object onto its target slot (a second claimant of one makes another file); the hash-only
# objects onto themselves (ONCE only in the mod's own file); the fix's own graphs into every file. The ONLY step that
# knows which slot a source object goes to: the ones after it address the target's objects, and a texture list's guard
# is read off this step's table when the list is written
FIX_GRAPHS = ([(0,) + FIX_SECTIONS, (0,) + SKELETON] + [(0,) + LISTG(c) for c in PLAN]
              + [(0, "", "created" + role) for role in CREATED] + [(0, "", "download" + role) for role in DOWNLOADS]
              + [(0,) + REF(c, role) for c, bindings in REGISTERS.items() for _, role in bindings]
              + [(0,) + TOGGLE(c, role) for c, bindings in REGISTERS.items() for _, role in bindings])
SLOT_REMAP = FRB.GraphGroupRemap({**{(0,) + SRC(c): [(0,) + TOBJ(c) + (RENAME,)] for c in PLAN},
                                  **{(0, "", obj): [(0, "", obj, RENAME, obj not in ONCE)] for obj in HASH_ONLY},
                                  **{graph: [graph + (lambda name: name, True)] for graph in FIX_GRAPHS}})


def guardOf(c):
    """The guard of a source object's texture list: the passes of the slot SLOT_REMAP sends it to"""
    target = SLOT_REMAP.remap[(0,) + SRC(c)][0]
    return GUARD[SLOT_OF[(target[1], target[2])]]


STEPS = [
    # ---- per source object, before the remap: its texture list's call, after its match_first_index ----
    FRB.GraphGroupEdit([{SRC(c): [FRB.RegSurroundedAdd([(K.Run, LIST[c])], beforeRegs = {K.MatchFirstIndex: lambda value: True})]
                         for c in PLAN}]),

    # ---- the shared lists each object runs: a buffer the mod's list rebinds is parked first (once), and the cleanup list
    #      runs the list that puts them back ----
    FRB.GraphGroupEdit([{SRC(c): [FRB.RegPartEdit(FRB.RegAnchorAdd([reg], lambda key, value, b = bypass, r = reg: [(b, REF_ + r)],
                                                                   FRB.RegAnchorAdd.Placement.Before, None, True, True), IN_SECTION(FIX.sharedResourcesList))
                                  for reg, bypass in BYPASSES]
                                 + [FRB.RegPartEdit(FRB.RegAdd([(K.Run, RESTORE)], True, True), IN_SECTION(FIX.cleanupResourcesList))]
                         for c in PLAN}]),

    # ---- each remapped section drops the lines the pair says (REMOVED) ----
    *([FRB.GraphGroupEdit([{SRC(c): [FRB.RegRemove([(r.reg, FRB.RegRemove.valueStartsWith(r.valuePrefix) if r.valuePrefix else None)
                                                    for r in REMOVED])] for c in PLAN}])] if REMOVED else []),

    # ---- past 256 bones: the skeleton and the blend remap's sections, latched where the mod's [Present] resets the key ----
    *([FRB.GraphCreate((0,) + SKELETON, sectionsOf = lambda group, latched = latched: skeletonSections(latched) + blendRemapSections(),
                       z3 = Z3, **{gate: RESETS})
       for latched, gate in ((True, "onlyWith"), (False, "onlyWithout"))] if PAST256 else []),

    # ---- the fix's own sections (the bypass, the passes' tags) when the mod draws any source object, and a texture list per
    #      object it draws, guarded by the passes of the slot it goes to ----
    FRB.GraphCreate((0,) + FIX_SECTIONS, sectionsOf = lambda group: bypassSections()
                    + [TAGS.section(i, RENAME("ShaderOverridePass" + str(i))) for i in range(len(TAGS.tags()))], z3 = Z3, onlyWith = SOURCES),
    *[FRB.GraphCreate((0,) + LISTG(c), sectionsOf = lambda group, c = c: [FRB.SectionText(Z3, LIST[c]).open(guardOf(c)).close().build()],
                      z3 = Z3, onlyWith = SRC(c))
      for c in PLAN],

    # ---- a role the mod toggles: its toggle, renamed to a list of the fix's, choosing the register the list binds ----
    FRB.GraphGroupEdit([{TOGGLE(c, role): [FRB.GraphRename(lambda old, name = TOGGLE_LIST(role, reg): name),
                                           FRB.RegRemap({K.This: [reg]})]
                         for c, bindings in REGISTERS.items() for reg, role in bindings}]),

    # ---- each list's lines, in register order: the role's resource at its register, or a run of the role's toggle ----
    *[FRB.GraphInherit((0,) + LISTG(c), (0,) + graph, line,
                       partFilter = lambda data, modType, ini, name = LIST[c]:
                           FRB.Ranges.createFull() if data.sectionName == name else FRB.Ranges.createEmpty())
      for c, bindings in REGISTERS.items() for reg, role in bindings
      for graph, line in [(ROLE(c, role), reg), (TOGGLE(c, role), K.Run)]],

    # ---- the stand-ins the lists bind: the source's game texture downloaded, or a texture made ----
    *[FRB.ResRegCollect({(0,) + LISTG(c): lambda key: True for c in PLAN},
                        {"download": FRB.FileDownloadCreate((0, "", "download" + role), {resource: FRB.FileDownloadCreate.File(file = file, url = url)})},
                        resPredicates = {(0,) + LISTG(c): lambda key, value, iterData, r = resource: value.strip() == r for c in PLAN})
      for role, (resource, file, url) in DOWNLOADS.items()],
    *[FRB.ResRegCollect({(0,) + LISTG(c): lambda key: True for c in PLAN},
                        {"created": FRB.TexCreate((0, "", "created" + role), "", FRB.CppTexCreator(size, size, colour), files = {resource: file})},
                        resPredicates = {(0,) + LISTG(c): lambda key, value, iterData, r = resource: value.strip() == r for c in PLAN})
      for role, (resource, colour, size, file) in CREATED.items()],

    SLOT_REMAP,

    # ---- per target object, in every file: its numbers (SLOTS); past 256 bones its merge list and the blend remap's after
    #      the shared list (by now under the fix's name); the hashes onto the target's ----
    FRB.GraphGroupEdit([{obj: [FRB.RegNewVals(dict(zip(SLOT_KEYS, SLOTS[slot])))] for obj, slot in TARGET_OBJS}], everyGroup = True),
    *([FRB.GraphGroupEdit([{obj: [FRB.RegSurroundedAdd([(K.Run, MERGE_LIST[slot]), (K.Run, BLEND_INIT)],
                                                       {K.Run: lambda value: value.strip().lower() == RENAME(FIX.sharedResourcesList).lower()}, {}, False)]
                            for obj, slot in TARGET_OBJS}], everyGroup = True)] if PAST256 else []),
    FRB.GraphGroupEdit([{**{obj: [HASHES] for obj, _ in TARGET_OBJS},
                         **{("", obj): [HASHES] + ([FRB.RegNewVals(DISPATCH)] if DISPATCH else []) for obj in HASH_ONLY}}],
                       everyGroup = True),

    # ---- a target slot nothing is remapped onto: its hide section, in every file ----
    *[FRB.GraphCreate((0, TARGET, "hide" + str(slot)), sectionsOf = lambda group, slot = slot: [hideSection(slot)], z3 = Z3, onlyWithout = DST(slot))
      for slot in range(len(SLOTS))],

    # ---- the blend, through the vertex group row (past 256 bones: in local ids, with its tables), in every file ----
    FRB.ResRegCollect({(0,) + obj: FIX.blendReg for obj, _ in TARGET_OBJS},
                      {"blend": FRB.VGRemapBlendReplace(
                          (0, "", "blend"), SOURCE_TYPE, FROM_VERSION, TO_VERSION, row = COMPACT[0] if PAST256 else ROW,
                          anchorChains = dict(FIX.anchorChains), influences = FIX.targetInfluences,
                          elements = lambda srcPath: FRB.BlendFile.byteElements(influences()),
                          fixFunc = writeBlendRemap if PAST256 else None,
                          pairedIds = (lambda srcPath: [FRB.VGRemapBlendReplace.PairedIds(besideBlend(srcPath, "VertexVG"), 2)])
                          if PAST256 else None)},
                      resPredicates = {(0,) + obj: FRB.ResRegCollect.skipsKeywords() for obj, _ in TARGET_OBJS},
                      everyGroup = True),
    FRB.GraphGroupRemap({(0, "", "blend"): [(0, "", "blend", lambda name: name, True)]}),

    # ---- past 256 bones: the latch, where the mod's [Present] resets $object_detected ----
    *([FRB.GraphGroupEdit([{**{obj: LATCH for obj, _ in TARGET_OBJS},
                            **{(TARGET, "hide" + str(slot)): LATCH for slot in range(len(SLOTS))}}],
                          everyGroup = True, onlyWith = RESETS)] if PAST256 else []),

    # ---- the parser's graphs that are not the fix's ----
    FRB.GraphRemove([(0,) + ROLE(c, role) for c, bindings in REGISTERS.items() for _, role in bindings]
                    + [(0, TEXTURE, role) for role in ROLES + [""]] + [(0,) + RESETS]),

    # ---- nothing of the source remapped onto the target: nothing is written ----
    FRB.GraphGroupRemove(onlyWithout = [obj for obj, _ in TARGET_OBJS]),
]

# THE WHOLE LIST: read the mod, resolve each role, fix
LIST_OF_EDITS = READ + ROLE_STEPS + STEPS


def makeFixer(parser, toModName, modTypeId):
    """The fixer runs LIST_OF_EDITS; a copy file the remap makes carries the fix's sections and none of the mod's own draws"""
    fixer = FRB.GIMIFixer(parser, graphGroupEdits = LIST_OF_EDITS, modsToFix = [toModName])
    fixer.copyHiddenSectionNames = {section for section in parser._iniFile.getSectionNames() if section.startswith(K.TextureOverride)}
    fixer.copyPreamble = FIX.copyPreamble
    return fixer


def fix(folder):
    """Fixes every mod under 'folder' with this list; returns the service, for its stats"""
    FRB.CppStrategyOverrides.setParser(SOURCE, makeParser)
    FRB.CppStrategyOverrides.setFixer(SOURCE, TARGET, makeFixer)
    try:
        service = FRB.RemapService(path = os.path.abspath(folder), downloadMode = FRB.DownloadMode.Disabled)
        service.fix()
        return service
    finally:
        FRB.CppStrategyOverrides.clear()


if __name__ == "__main__":
    for path, error in fix(sys.argv[1]).stats.ini.skipped.items():
        print("SKIPPED", path, error)

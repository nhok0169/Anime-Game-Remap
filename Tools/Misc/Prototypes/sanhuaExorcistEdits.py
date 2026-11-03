"""Sanhua -> SanhuaExorcist written explicitly from the bound API: the parser as its steps, then the fixer as its
edits -- the maintainer's acceptance test for the WWMI refactor ("as simple as the ayaka6_1 row"). Every step is
written out here rather than hidden behind a template, so what each step really costs is visible, and every place
the API cannot say something is marked `GAP:`.

    from sanhuaExorcistEdits import makeParser     # makeParser(iniFile, modTypeId) -> GIMIParser

The compiled makeWWMIParser / makeWWMIFixer for this pair is the oracle.
"""
import os
import sys

API = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "Anime Game Remap (for all users)", "api"))
if API not in sys.path:
    sys.path.insert(0, API)
import src.py.FixRaidenBoss2 as FRB


# ======================================================================================================== data
# Sanhua's own facts: what the C++ row reads off WWMIParserConfig / WWMITextureFacts.

SOURCE = "Sanhua"
VERSION = "2.5"
SLOT_PREFIX = "component"
SLOT_HASH = "vb0"
TEX_REG = "ps-t"
WATCH = ("", "watch")
TEXTURE = FRB.IniGraphModObjKeywords.Texture
HASH_ONLY = [("cb4", "boneData"), ("shapekey_offsets", "shapekeyOffsets"), ("shapekey_scale", "shapekeyScale")]

# her texture roles; which hash plays which is HashData's (every generation is filed there)
ROLES = {"eyeMask", "bangsDiffuse", "irisDiffuse", "t5Ramp", "bangsMask", "hairDiffuse", "hairNormal", "faceMask",
         "faceDiffuse", "skinNormal", "skinDiffuse", "bodiceNormal", "bodiceMask", "bodiceDiffuse", "skirtNormal",
         "skirtMask", "skirtDiffuse"}

# RabbitFX's resource lines name the role they hand SetTextures, per component
RABBIT = "Resource\\RabbitFX\\"
REGISTER_ROLES = {
    0: {RABBIT + "Diffuse": "bangsDiffuse", RABBIT + "Lightmap": "bangsMask"},
    1: {RABBIT + "Diffuse": "hairDiffuse", RABBIT + "Lightmap": "hairNormal"},
    2: {RABBIT + "Diffuse": "faceDiffuse", RABBIT + "Lightmap": "faceMask"},
    3: {RABBIT + "Diffuse": "skinDiffuse", RABBIT + "Normalmap": "skinNormal"},
    4: {RABBIT + "Diffuse": "bodiceDiffuse", RABBIT + "Lightmap": "bodiceMask", RABBIT + "Normalmap": "bodiceNormal"},
    5: {RABBIT + "Diffuse": "skirtDiffuse", RABBIT + "Lightmap": "skirtMask", RABBIT + "Normalmap": "skirtNormal"},
}

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
    with only the anatomy read off its rows (and so none of its own linked files)"""
    modType = FRB.ModTypes.Sanhua.value
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

    # ---- step 3: each texture override's hash and `this`; the roles the .ini states; and the toggles ----
    overrides = FRB.GraphCollect({graphId(obj): [FRB.IniKeywords.Hash, FRB.IniKeywords.This] for obj in modObjs},
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

    # step 4: the anatomy, by WWMI's rules
    def readAnatomy(data):
        slotRoots = roots(lambda obj: slotOf(obj) is not None)
        rules = FRB.IniParseData.AnatomyRules()
        rules.rows = FRB.IniParseRows.Objs
        rules.resources = FRB.IniParseRows.Resources
        rules.sections = FRB.IniParseRows.Sections
        rules.texRegPrefix = TEX_REG
        rules.noneUndrawnWhenAll = True
        rules.libraries = [(FRB.RabbitFX.Name, FRB.RabbitFX.names)]
        rules.runDraws = lambda called: iniFile.getSection(called) is None and "\\wwmiv1\\" not in called.lower()
        rules.roots = [(obj, sections) for obj, sections in parser._sectionTargets.items() if slotOf(obj) is not None]

        # does the mod replace textures by hash? a texture override with a hash and a `this`
        has = {}
        for row in data.collected(OVERRIDE_ROWS):
            if row.section.startswith(FRB.IniKeywords.TextureOverride) and row.section not in slotRoots:
                has.setdefault(row.section, set()).add(row.key)
        if any({FRB.IniKeywords.Hash, FRB.IniKeywords.This} <= keys for keys in has.values()):
            data.setReplacesTexturesByHash(True)
        data.fileAnatomy(rules)

    # step 5: the skeleton numbering -- Sanhua has one numbering, so nothing to probe

    # step 6: the texture roles, the toggles, and each slot's file per role
    def readTextures(data):
        order = byWritten(iniFile.getSectionNames())
        textureRoles = buildRoles(data, order)
        data.setTextureRoles(textureRoles)
        readThisToggles(data, order)
        readRegisterToggles(data, order)
        chooseRoleFiles(data, textureRoles, order)

    def buildRoles(data, order):
        out = FRB.TextureRoles()
        folder, iniPath = iniFile.folder, iniFile.file
        fileOf = {}
        for row in data.collected(FRB.IniParseRows.Declared):
            if row.key == FRB.IniKeywords.Filename and row.section.lower() not in fileOf:
                fileOf[row.section.lower()] = iniPathOf(row.value, folder)
                out.addResource(iniPath, row.section, fileOf[row.section.lower()])
        hashOf = {}
        for row in data.collected(OVERRIDE_ROWS):
            if row.key == FRB.IniKeywords.Hash:
                hashOf.setdefault(row.section, row.value.strip().lower())

        byRoot = {}
        for row in data.collected(ROLE_ROWS):
            byRoot.setdefault(row.rootSection, []).append(row)
        for name in sorted(byRoot, key = order):
            for row in byRoot[name]:
                # (1) the section IS a texture: the classifier placed it in its role's graph by its hash
                if row.graphId[1] == TEXTURE and row.key == FRB.IniKeywords.This:
                    file = fileOf.get(row.value.strip().lower())
                    if file is not None:
                        out.add(file, row.graphId[2], FRB.TextureRoles.DeclaredHash + hashOf.get(name, ""))
            for row in byRoot[name]:
                # (2) a slot's draw binds a role by RabbitFX's resource names, through `run =`
                role = REGISTER_ROLES.get(slotOf(row.graphId[1:]), {}).get(row.key)
                file = fileOf.get(FRB.IniNamingTools.removeRefPrefix(row.value.strip()).lower()) if role else None
                if file is not None:
                    out.add(file, role, "bound at " + row.key + " of " + row.graphId[2])

        # (3) a .dds no hash and no register named, identified by its pixels against her own textures
        # GAP: TexThumbprint is bound, but her thumbprint table (sanhuaTextureThumbprints) is C++ data Python cannot reach
        return out

    def readThisToggles(data, order):
        textureRoots = roots(lambda obj: obj[0] == TEXTURE)
        own = rowsByRoot(data.collected(TOGGLE_ROWS), textureRoots)
        ownerOf, variantsOf, single = {}, {}, set()
        detected = FRB.IniKeywords.WWMIObjectDetected

        def resolves(values):
            return sum(1 for value in values if iniFile.getSection(value) is not None)

        for name in sorted(own, key = order):
            if not name.startswith(FRB.IniKeywords.TextureOverride) or isFixSection(name):
                continue
            bound, realToggle = [], False
            for row in asWritten(own[name]):
                for guard in row.guards:
                    test = "".join(guard.lower().split())
                    realToggle = realToggle or test not in (detected, detected + "==1")
                if row.key == FRB.IniKeywords.This:
                    bound.append(row.value.strip())

            if len(bound) == 1 and realToggle:
                if bound[0].lower() not in ownerOf:
                    ownerOf[bound[0].lower()] = name
                    variantsOf[name] = bound
                    single.add(bound[0].lower())
                continue
            if len(bound) < 2:
                continue

            # two sections may bind one resource behind a toggle: the one answering for most of its variants keeps it
            score = resolves(bound)
            for resource in bound:
                owner = ownerOf.get(resource.lower())
                if owner is not None and owner in variantsOf and resolves(variantsOf[owner]) >= score:
                    continue
                ownerOf[resource.lower()] = name
            variantsOf[name] = bound

        for resource, section in ownerOf.items():
            graph = textureRoots.get(section)
            data.setThisToggle(resource, FRB.IniParseData.TextureToggle(section, variantsOf[section], resource in single,
                                                                        graph[0] if graph else ("", "")))

    def readRegisterToggles(data, order):
        textureRoots = roots(lambda obj: obj[0] == TEXTURE or slotOf(obj) is not None)
        own = rowsByRoot(data.collected(TOGGLE_ROWS), textureRoots)
        for name, rowList in rowsByRoot(data.collected(FRB.IniParseRows.Objs), textureRoots).items():
            own.setdefault(name, []).extend(rowList)

        for name in sorted(own, key = order):
            if not name.startswith(FRB.IniKeywords.TextureOverride) or isFixSection(name):
                continue
            boundOfReg = {}
            for row in asWritten(own[name]):
                key = row.key.strip().lower()
                value = FRB.IniNamingTools.removeRefPrefix(row.value.strip())
                if not key.startswith(TEX_REG) or not value or value.lower() == FRB.IniKeywords.Null.lower():
                    continue
                values = boundOfReg.setdefault(key, [])
                if value.lower() not in (had.lower() for had in values):
                    values.append(value)
            for reg, values in sorted(boundOfReg.items()):
                if len(values) >= 2:
                    for value in values:
                        data.setRegisterToggle(value.lower(), FRB.IniParseData.TextureToggle(name + " " + reg, values, False))

    def chooseRoleFiles(data, textureRoles, order):
        iniPath = iniFile.file
        drawn = drawnSlots(order)
        chooser = FRB.TextureRoleChooser()
        chooser.folder = iniFile.folder

        resources = textureRoles.resourcesOf(iniPath)
        fileOfResource = {resource.lower(): file for resource, file in reversed(resources)}
        resourcesOfFile = {}
        for resource, file in resources:
            resourcesOfFile.setdefault(file, []).append(resource.lower())
        chooser.declaredFiles = {file for _, file in resources}

        def variantsOf(file):
            files = []
            for resource in resourcesOfFile.get(file, []):
                for toggle in (data.thisToggleOf(resource), data.registerToggleOf(resource)):
                    if toggle is None or len(toggle.variants) < 2:
                        continue
                    for variant in toggle.variants:
                        variantFile = fileOfResource.get(variant.lower())
                        if variantFile is not None and variantFile not in files:
                            files.append(variantFile)
            return files if len(files) >= 2 else []
        chooser.variantsOf = variantsOf

        candidates, declared, bindings = {}, {}, {}
        addOwnCandidates(data, textureRoles, drawn, fileOfResource, candidates, declared, bindings)
        if drawn and iniPath:
            addLinkedInis(drawn, candidates, bindings)
        chooser.candidates, chooser.declared, chooser.bindings = candidates, declared, bindings
        chooser.choose(data, [("", SLOT_PREFIX + str(slot)) for slot in drawn])

        for resource, file in resources:
            data.setTextureResource(resource, file)

        # ...and which resource each file is USED by: one file may be declared by several, and the one a section of
        # the mod binds is the one that reaches the draws. A section a previous fix wrote is not the mod's use
        fixNames = [modType.name, *modType.getModsToFix()]
        used = set()
        for section in data.collectedSections(FRB.IniParseRows.Sections):
            if not FRB.IniNamingTools.looksRemapped(section, fixNames):
                used.update(FRB.IniNamingTools.removeRefPrefix(row.value).strip().lower()
                            for row in data.sectionRows(FRB.IniParseRows.Sections, section))
        usedBy = {}
        for resource, file in resources:
            had = usedBy.setdefault(file, resource)
            if had.lower() not in used and resource.lower() in used:
                usedBy[file] = resource
        for file, resource in usedBy.items():
            data.setFileResource(file, resource)

    def addOwnCandidates(data, textureRoles, drawn, fileOfResource, candidates, declared, bindings):
        missing = set()

        def isMissing(file):
            if os.path.isfile(file):
                return False
            missing.add(file)
            return True

        for file, rs in textureRoles.rolesOf().items():
            if isMissing(file):
                continue
            for role in rs:
                candidates.setdefault(role.role, []).append((file, role.how))
                if role.how.startswith(FRB.TextureRoles.DeclaredHash):
                    declared.setdefault(file, set()).add(role.role)

        own = rowsByRoot(data.collected(FRB.IniParseRows.Objs), roots(lambda obj: slotOf(obj) is not None))
        for slot, sections in drawn.items():
            for section in sections:
                rowList = asWritten(own.get(section, []))
                for reg, role in REGISTER_ROLES.get(slot, {}).items():
                    bound = next((row.value.strip() for row in rowList if FRB.IniNamingTools.sameKey(row.key, reg)), None)
                    file = fileOfResource.get(FRB.IniNamingTools.removeRefPrefix(bound).lower()) if bound is not None else None
                    if file is None or isMissing(file):
                        continue
                    candidates.setdefault(role, []).append((file, "the " + reg + " its own section binds it at"))
                    bindings.setdefault(file, {}).setdefault(("", SLOT_PREFIX + str(slot)), set()).add(role)

        for file in sorted(missing):
            print(os.path.basename(file) + " is declared but not in the mod's folder; its role is treated as one the"
                  " mod has no file for")

    # the files LINKED to this one (a namespace one references, a resource file both name), each read by this
    # character's parser in its only-this-file mode: their recolours, then their bindings on this file's draws
    def addLinkedInis(drawn, candidates, bindings):
        readers = []
        for linked in iniFile.linkedInis():
            sibling = iniFile.siblingOf(linked, {modTypeId})
            reader = makeParser(sibling, modTypeId, onlyThisFile = True)
            reader.parse()
            readers.append((linked, sibling, reader))

        found = 0
        for _, sibling, reader in readers:
            for section, role, hash, file in recolours(sibling, reader):
                if os.path.isfile(file):
                    candidates.setdefault(role, []).append((file, "a sibling .ini overrides " + hash + ", the game texture it replaces"))
                    found += 1
        if found:
            print("a sibling .ini recolours %d of the game's textures; carried over to the target, which binds its own"
                  " and would never see those hashes" % found)

        found = 0
        for linked, sibling, reader in readers:
            for slot, reg, role, file in drawBindings(sibling, reader):
                if slot in drawn and os.path.isfile(file):
                    candidates.setdefault(role, []).append((file, "the " + reg + " " + os.path.basename(linked.file)
                                                            + " binds on the same draw"))
                    bindings.setdefault(file, {}).setdefault(("", SLOT_PREFIX + str(slot)), set()).add(role)
                    found += 1
        if found:
            print("another .ini of the mod binds %d textures on this file's draws; carried over to the target, where"
                  " that .ini's overrides never fire" % found)

    parser.deferredParseReaders += [readAnatomy] if onlyThisFile else [readAnatomy, readTextures]
    return parser


# the file a resource of 'ini' names (`ref` or not): its first `filename`, from the .ini's folder
def declaredFile(ini, reader, resource):
    file = reader.parseData.resourceFile(FRB.IniNamingTools.removeRefPrefix(resource.strip()))
    return iniPathOf(file, ini.folder) if file is not None else None


# a linked file's recolours: each texture override the parser placed in a role's graph, as (section, role, hash, file),
# in the order the sections are written
def recolours(ini, reader):
    roleOf = {}
    for obj, sections in reader._sectionTargets.items():
        if obj[0] == TEXTURE and obj[1]:
            for section in sections:
                roleOf.setdefault(section, obj[1])

    result = []
    for hash, row in FRB.GIMIParser.textureOverrides(ini):
        role = roleOf.get(row.section)
        if not row.section.startswith(FRB.IniKeywords.TextureOverride) or role is None:
            continue
        file = declaredFile(ini, reader, row.value)
        if file is not None:
            result.append((row.section, role, hash, file))
    order = byWritten(ini.getSectionNames())
    return sorted(result, key = lambda entry: order(entry[0]))


# a linked file's bindings on the slots' draws: every value a slot section binds, through `run =`, at a register her
# layout names for the slot, as (slot, register, role, file) -- the sections as written, the registers in the table's
# order, each part counted once
def drawBindings(ini, reader):
    byRoot, seen = {}, set()
    for row in reader.parseData.collected(ROLE_ROWS):
        once = (row.rootSection, row.section, row.partId, row.line, row.key)
        if slotOf(row.graphId[1:]) is not None and once not in seen:
            seen.add(once)
            byRoot.setdefault(row.rootSection, []).append(row)

    result = []
    order = byWritten(ini.getSectionNames())
    for name in sorted(byRoot, key = order):
        rows = byRoot[name]
        for slot in sorted({slotOf(row.graphId[1:]) for row in rows}):
            for reg, role in sorted(REGISTER_ROLES.get(slot, {}).items()):
                for row in rows:
                    if slotOf(row.graphId[1:]) == slot and FRB.IniNamingTools.sameKey(row.key, reg):
                        file = declaredFile(ini, reader, row.value)
                        if file is not None:
                            result.append((slot, reg, role, file))
    return result


# ======================================================================================================= fixer

TARGET = "SanhuaExorcist"
TARGET_VERSION = "2.5"

# source component -> the target slot it draws through: the bangs, hair and face onto their own slots, everything
# of the body (arm skin, bodice, skirt) through the torso slot, the eyes onto the eye slot
PLAN = {0: 0, 1: 1, 2: 2, 3: 3, 4: 3, 5: 3, 6: 5}
SHAPEKEY_DISPATCH = "853"       # the target's shape-key dispatch height (its Metadata.json's dispatch_y)

# the registers each target pass reads, and the source role bound at each, per source component
REGISTERS = {
    0: [("ps-t0", "bangsDiffuse"), ("ps-t1", "bangsMask"), ("ps-t5", "t5Ramp")],
    1: [("ps-t0", "hairDiffuse"), ("ps-t1", "hairNormal"), ("ps-t5", "t5Ramp")],
    2: [("ps-t0", "faceMask"), ("ps-t1", "faceDiffuse")],
    3: [("ps-t0", "skinNormal"), ("ps-t1", "SkinMask"), ("ps-t2", "skinDiffuse")],
    4: [("ps-t0", "bodiceNormal"), ("ps-t1", "bodiceMask"), ("ps-t2", "bodiceDiffuse")],
    5: [("ps-t0", "skirtNormal"), ("ps-t1", "skirtMask"), ("ps-t2", "skirtDiffuse")],
    6: [("ps-t1", "eyeMask"), ("ps-t2", "irisDiffuse"), ("ps-t5", "t5Ramp")],
}

# the target's pixel shaders per draw slot (a frame dump of SanhuaExorcist), the vertex shaders each is drawn with
# (the lists are gated on those: RabbitFX patches the pixel shaders), and each vertex shader's filter_index -- named,
# because the face pair must agree with ChisaParfait's and LynaePeppermint's
SLOT_PASSES = [["0f1752e476c3804d", "394feb3747bda7a0"], ["0f1752e476c3804d"], ["ed1c0f8b2ba08ac4"],
               ["fbe2f2f061949559"], ["348ac69e57e38686"], ["0512a7e61d6bd01d"]]
PASS_VERTEX_SHADERS = {"0f1752e476c3804d": ["a695e6d5f9d489cf"], "394feb3747bda7a0": ["a695e6d5f9d489cf"],
                       "ed1c0f8b2ba08ac4": ["7c0b4db32cee62d3", "4ce754b5b9574b1a"], "fbe2f2f061949559": ["02f5c4b333a78fa0"],
                       "348ac69e57e38686": ["15a2c90dd35b8c07"], "0512a7e61d6bd01d": ["a79e471015b2a16c"]}
FILTER_INDICES = {"a695e6d5f9d489cf": "3381.91", "7c0b4db32cee62d3": "3381.76", "4ce754b5b9574b1a": "3381.769",
                  "02f5c4b333a78fa0": "3381.94", "15a2c90dd35b8c07": "3381.95", "a79e471015b2a16c": "3381.96"}

# what stands in when the mod has no file for a role: the texture the fix invents (Exorcist's mask reads skin off
# one colour), else Sanhua's own game texture, downloaded (the mod's UVs are hers)
CREATED = {"SkinMask": ((255, 77, 0, 255), 16)}
FALLBACKS = {"bangsDiffuse": "ae6e9014", "bangsMask": "1c0c8b91", "t5Ramp": "1035197c", "hairDiffuse": "68ca7071",
             "hairNormal": "cef6494f", "faceDiffuse": "881c236d", "skinNormal": "e39835c7", "skinDiffuse": "fde0f298",
             "bodiceNormal": "efb25eb3", "bodiceMask": "89ba19a1", "bodiceDiffuse": "abda232b", "skirtNormal": "f3b217ab",
             "skirtMask": "f0713dc7", "skirtDiffuse": "c689a8ee", "irisDiffuse": "1dcc0f1d"}
DOWNLOADS = ("WuWa", "Sanhua", "2_5", "Sanhua")      # the game, character and version folders, and the file prefix
BLEND_REG = "vb4"                                     # the register a WWMI draw reads the blend at
TARGET_LABELS = {0: "bangs", 1: "hair", 2: "face", 3: "torso, arms, ribbons", 4: "hair bun, trousers", 5: "eyes"}


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
    source, target = FRB.ModTypes.Sanhua.value, FRB.ModTypes.SanhuaExorcist.value
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

    # ---- step 1: the lines that undo the remap -- none for this pair (no removedRegs, target under 256 bones) ----

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
    for passes in SLOT_PASSES:
        for shader in passes:
            for vs in PASS_VERTEX_SHADERS.get(shader, []):
                tags.add(vs, FILTER_INDICES[vs])

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
            name, n = FRB.IniKeywords.Resource + role[0].upper() + role[1:] + toModName + FRB.IniKeywords.RemapRef, 2
            while name in declaredName.values():
                name, n = FRB.IniKeywords.Resource + role[0].upper() + role[1:] + str(n) + toModName + FRB.IniKeywords.RemapRef, n + 1
            declaredName[chosen.file] = name
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
    for role in CREATED:
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
        for role, (colour, size) in CREATED.items():
            resource = resourceOfRole[role]
            make = FRB.TexCreate((group, "", "fixTexture" + resource), "", FRB.CppTexCreator(size, size, FRB.CppColour(*colour)))
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
    restoreList = rename(FRB.IniKeywords.CommandList + "RestoreSharedResources")

    def buffersBoundBy(listName):
        regs = []
        for row in data.sectionRows(FRB.IniParseRows.Sections, listName):
            key = row.key.strip().lower()
            if (key == FRB.IniKeywords.Ib or (len(key) > 2 and key.startswith("vb") and key[2:].isdigit())) and key not in regs:
                regs.append(key)
        return regs

    bypasses = []
    if data.sectionRows(FRB.IniParseRows.Sections, sharedList) and data.sectionRows(FRB.IniParseRows.Sections, cleanupList):
        already = buffersBoundBy(cleanupList)
        bypasses = [(reg, rename(FRB.IniKeywords.Resource + "Bypass" + FRB.IniNamingTools.getRegTag(reg)))
                    for reg in buffersBoundBy(sharedList) if reg not in already]

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
        for reg, bypass in bypasses:
            fixSections.sections("fixSections").add(FRB.SectionText(z3, bypass).build())
        fixSections.sections("fixSections").add(
            FRB.SectionText(z3, restoreList).keys([(reg, FRB.IniKeywords.Ref + " " + bypass) for reg, bypass in bypasses]).build())

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
            edits[targetObj(component)] = [slotValues[PLAN[component]], assetRemap, dispatch]
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

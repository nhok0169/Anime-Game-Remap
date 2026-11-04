import os
import sys
from ordered_set import OrderedSet

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ParseStepsTest(BaseIniFileTest):
    """GIMIParser.parseSteps and deferredParseSteps: reading edits, run at the end of the parse or on the first ask
    of the parse data, each filed through IParseResult"""

    _stepsIniTxt = r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
ps-t0 = ResourceBodyDiffuse
drawindexed = 300, 0, 0
ps-t1 = ResourceBodyLightMap

[ResourceBodyDiffuse]
filename = Textures\Body.dds

[ResourceBodyLightMap]
filename = Textures\BodyLM.dds
"""

    Body = ("", "body")
    BodyGraph = (0, "", "body")

    def parseWith(self, parseSteps = None, deferredParseSteps = None):
        self.setupIniTxt(self._stepsIniTxt)
        self.createIniFile()
        self._parser = FRB.GIMIParser(self._iniFile, modObjs = OrderedSet([self.Body]), trackKeys = False,
                                      parseSteps = parseSteps, deferredParseSteps = deferredParseSteps)
        self.useStrategies(parser = self._parser)
        self._iniFile.parse()
        return self._parser

    def removeLightMap(self):
        return FRB.GraphGroupEdit(edits = [{self.Body: [FRB.RegRemove({"ps-t1": None})]}])

    def registers(self, name = "regs"):
        return FRB.GraphCollect({self.BodyGraph: lambda key: key.startswith("ps-t")}, name = name)

    @classmethod
    def values(cls, rows):
        return [row.value for row in rows]

    # ====================== the lists ==========================================

    def test_bothLists_defaultToEmpty(self):
        self.setupIniTxt(self._stepsIniTxt)
        self.createIniFile()
        parser = FRB.GIMIParser(self._iniFile)
        self.assertEqual(list(parser.parseSteps), [])
        self.assertEqual(list(parser.deferredParseSteps), [])

    def test_bothLists_areTheCallersOwnLists(self):
        steps, deferred = [self.registers()], [self.registers("later")]
        self.setupIniTxt(self._stepsIniTxt)
        self.createIniFile()
        parser = FRB.GIMIParser(self._iniFile, parseSteps = steps, deferredParseSteps = deferred)
        self.assertIs(parser.parseSteps, steps)
        self.assertIs(parser.deferredParseSteps, deferred)

    def test_parseSteps_aCollectIsFiled(self):
        parser = self.parseWith([self.registers()])
        self.assertEqual(self.values(parser.parseData.collected("regs")), ["ResourceBodyDiffuse", "ResourceBodyLightMap"])

    def test_parseSteps_runInOrder_aCollectBeforeAnEditReadsTheUneditedGraphs(self):
        parser = self.parseWith([self.registers(), self.removeLightMap()])
        self.assertEqual(self.values(parser.parseData.collected("regs")), ["ResourceBodyDiffuse", "ResourceBodyLightMap"])

    def test_parseSteps_aCollectAfterAnEditReadsTheEditedGraphs(self):
        parser = self.parseWith([self.removeLightMap(), self.registers()])
        self.assertEqual(self.values(parser.parseData.collected("regs")), ["ResourceBodyDiffuse"])

    def test_deferredParseSteps_runAfterTheParseSteps(self):
        parser = self.parseWith([self.removeLightMap()], [self.registers()])
        self.assertEqual(self.values(parser.parseData.collected("regs")), ["ResourceBodyDiffuse"])

    def test_deferredParseSteps_runOnTheFirstAsk(self):
        collect = self.registers()
        parser = self.parseWith(deferredParseSteps = [collect])
        self.assertEqual(collect.rows, [])

        self.assertEqual(len(parser.parseData.collected("regs")), 2)
        self.assertEqual(len(collect.rows), 2)


    # ====================== addRowSteps ======================================

    _rowsIniTxt = r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
ps-t0 = ResourceBodyDiffuse
drawindexed = 300, 0, 0

[Present]
vb9 = ref resourcebodylightmap

[ResourceBodyDiffuse]
filename = Textures\Body.dds

[ResourceBodyLightMap]
filename = Textures\BodyLM.dds
format = R8

[ResourceUnbound]
filename = Textures\Unbound.dds
"""

    def parseRows(self, **fields):
        self.setupIniTxt(self._rowsIniTxt)
        self.createIniFile()
        self._parser = FRB.GIMIParser(self._iniFile, modObjs = OrderedSet([self.Body]), trackKeys = False)
        rows = FRB.GIMIParser.RowSteps()
        rows.objs = lambda obj: obj == self.Body
        for name, value in fields.items():
            setattr(rows, name, value)
        self._parser.addRowSteps(rows)
        self.useStrategies(parser = self._parser)
        self._iniFile.parse()
        return self._parser.parseData

    def test_addRowSteps_addsTheEditsInOrder(self):
        self.setupIniTxt(self._rowsIniTxt)
        self.createIniFile()
        parser = FRB.GIMIParser(self._iniFile, modObjs = OrderedSet([self.Body]))
        later = self.registers("later")
        parser.addRowSteps(FRB.GIMIParser.RowSteps())
        parser.deferredParseSteps.append(later)
        # the global graph, the resource collect, files, declarations, sections, the removal and the objects
        steps = list(parser.deferredParseSteps)
        self.assertEqual(len(steps), 8)
        self.assertTrue(all(isinstance(step, FRB.BaseIniGraphGroupEdit) for step in steps[:-1]))
        self.assertIs(steps[-1], later)

    def test_addRowSteps_resourcesAreTheFilesBoundAnywhere(self):
        # the light map is bound only from [Present], by a lower-cased reference
        rows = self.parseRows().collected(FRB.IniParseRows.Resources)
        self.assertEqual([(row.section, row.value) for row in rows],
                         [("ResourceBodyDiffuse", r"Textures\Body.dds"), ("ResourceBodyLightMap", r"Textures\BodyLM.dds")])

    def test_addRowSteps_resourceNamed_ignoresCaseAndRef(self):
        self.parseRows().collected(FRB.IniParseRows.Resources)
        self.assertEqual(self._parser.resourceNamed("ref resourcebodylightmap"), "ResourceBodyLightMap")
        self.assertEqual(self._parser.resourceNamed("ResourceNothing"), "")

    def test_addRowSteps_declarationsSectionsAndObjs(self):
        data = self.parseRows(declaredKeys = ["hash"])
        declared = {(row.section, row.key): row.value for row in data.collected(FRB.IniParseRows.Declared)}
        self.assertEqual(declared[("ResourceBodyLightMap", "format")], "R8")
        self.assertEqual(declared[("ResourceUnbound", "filename")], r"Textures\Unbound.dds")
        self.assertEqual(declared[("TextureOverrideBody", "hash")], "11111111")
        self.assertIn("Present", data.collectedSections(FRB.IniParseRows.Sections))
        self.assertIn("drawindexed", [row.key for row in data.collected(FRB.IniParseRows.Objs)])

    def test_addRowSteps_eachSetCanBeLeftOut(self):
        data = self.parseRows(resources = False, sections = False, objs = None)
        self.assertEqual(data.collected(FRB.IniParseRows.Resources), [])
        self.assertEqual(data.collected(FRB.IniParseRows.Sections), [])
        self.assertEqual(data.collected(FRB.IniParseRows.Objs), [])
        self.assertNotEqual(data.collected(FRB.IniParseRows.Declared), [])

    # ====================== IniParseData.resourceData ===========================

    def readResourceData(self, iniTxt = None, **rules):
        if (iniTxt is not None):
            self._stepsIniTxt = iniTxt
        everyKey = FRB.GraphCollect({self.BodyGraph: lambda key: True}, name = FRB.IniParseRows.Objs)
        parser = self.parseWith([everyKey])
        dataRules = FRB.IniParseData.ResourceDataRules()
        dataRules.rows = FRB.IniParseRows.Objs
        dataRules.roots = [(self.Body, ["TextureOverrideBody"])]
        for name, value in rules.items():
            setattr(dataRules, name, value)
        parser.parseData.resourceData(dataRules)
        return parser.parseData

    # the body's own rows as the toggle rows, and its section as a texture root
    def readToggles(self, iniTxt, **rules):
        return self.readResourceData(iniTxt, toggles = FRB.IniParseRows.Objs, textureRoots = [(self.Body, ["TextureOverrideBody"])],
                                     **rules)

    def test_resourceData_aThisOnTwoBranches_isAThisToggleOfBoth(self):
        data = self.readToggles(r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
if $x == 1
    this = ResourceA
else
    this = ResourceB
endif
""")
        toggle = data.thisToggleOf("resourceb")
        self.assertIsNotNone(toggle)
        self.assertEqual(toggle.section, "TextureOverrideBody")
        self.assertEqual(toggle.variants, ["ResourceA", "ResourceB"])
        self.assertFalse(toggle.single)
        self.assertEqual(toggle.graph, self.Body)
        self.assertIsNotNone(data.thisToggleOf("resourcea"))

    def test_resourceData_oneThisUnderACondition_isASingleToggle_unlessTheConditionIsNotAToggle(self):
        iniTxt = r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
if $object_detected
    this = ResourceA
endif
"""
        self.assertTrue(self.readToggles(iniTxt).thisToggleOf("resourcea").single)
        self.assertIsNone(self.readToggles(iniTxt, notToggles = ["$object_detected"]).thisToggleOf("resourcea"))

    def test_resourceData_aRegisterOnTwoBranches_isARegisterToggleOfBoth(self):
        data = self.readToggles(r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
if $x == 1
    ps-t3 = ResourceA
else
    ps-t3 = ref ResourceB
endif
drawindexed = 300, 0, 0
""")
        toggle = data.registerToggleOf("resourceb")
        self.assertIsNotNone(toggle)
        self.assertEqual(toggle.section, "TextureOverrideBody ps-t3")
        self.assertEqual(toggle.variants, ["ResourceA", "ResourceB"])
        self.assertIsNone(data.thisToggleOf("resourceb"))

    def test_resourceData_noToggleCollect_readsNoToggles(self):
        data = self.readResourceData(r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
if $x == 1
    this = ResourceA
else
    this = ResourceB
endif
""")
        self.assertIsNone(data.thisToggleOf("resourcea"))

    def test_resourceData_aSectionTogglesInRefuses_holdsNoToggle(self):
        data = self.readToggles(r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
if $x == 1
    this = ResourceA
else
    this = ResourceB
endif
""", togglesIn = lambda section: False)
        self.assertIsNone(data.thisToggleOf("resourcea"))

    def test_resourceData_drawsTheirPartsAndLateBindings(self):
        data = self.readResourceData()
        self.assertEqual(data.drawRanges(self.Body), [(300, 0)])
        self.assertEqual(data.drawParts(self.Body), {1})
        self.assertEqual(data.lateBindings(self.Body), ["ps-t1"])
        self.assertFalse(data.isUndrawn(self.Body))

    def test_resourceData_aDrawUnderAnIf_sitsInThePartAfterTheIfLine(self):
        data = self.readResourceData(r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
if $x == 1
    drawindexed = 300, 0, 0
endif
""")
        self.assertEqual(data.drawParts(self.Body), {3})

    def test_resourceData_aSectionThatSkipsAndDrawsNothing_isUndrawn(self):
        hidden = r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
handling = skip
"""
        self.assertTrue(self.readResourceData(hidden).isUndrawn(self.Body))
        self.assertFalse(self.readResourceData(hidden, noneUndrawnWhenAll = True).isUndrawn(self.Body))

    def test_resourceData_libraries_andARunThatDraws(self):
        data = self.readResourceData(r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
handling = skip
run = CommandList\Lib\Draw
""", libraries = [("Lib", lambda text: "\lib\\" in text.lower())], runDraws = lambda called: True)
        self.assertEqual(data.libraries(self.Body), {"Lib"})
        self.assertFalse(data.isUndrawn(self.Body))

    def test_resourceData_aResourceBoundOnlyUnderAToggle_isAToggledBinding(self):
        data = self.readResourceData(r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
ps-t0 = ResourceBodyDiffuse
if $alt == 1
    ps-t1 = ResourceAlt
endif
drawindexed = 300, 0, 0
""")
        self.assertEqual(data.toggledBindings(self.Body), {"resourcealt"})

    def test_resourceData_aToggledLineAtARegisterWithADefault_isNotAToggledBinding(self):
        data = self.readResourceData(r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
ps-t0 = ResourceBodyDiffuse
if $alt == 1
    ps-t0 = ResourceAlt
endif
drawindexed = 300, 0, 0
""")
        self.assertEqual(data.toggledBindings(self.Body), set())

    def test_thisOverrides_firstWins_andThisBoundIgnoresCase(self):
        data = FRB.IniParseData()
        data.setThisOverride("ABCD1234", "ResourceFirst")
        data.setThisOverride("abcd1234", "ResourceSecond")
        data.addThisBound("ResourceFirst")

        self.assertEqual(data.thisOverrideOf("abcd1234"), "ResourceFirst")
        self.assertIsNone(data.thisOverrideOf("ffffffff"))
        self.assertTrue(data.isThisBound("resourcefirst"))
        self.assertFalse(data.isThisBound("ResourceSecond"))

    # ====================== IniParseData.roleFiles ===========================

    Diffuse = ("texture", "bodyDiffuse")
    Normal = ("texture", "bodyNormal")
    TexHashes = {"aaaaaaaa": ("texture", "bodyDiffuse"), "bbbbbbbb": ("texture", "bodyDiffuse"),
                 "cccccccc": ("texture", "bodyNormal")}

    def readRoleFiles(self, iniTxt, files, **rules):
        """Parses 'iniTxt' with the body and two texture-role graphs (a section is placed in one by its hash), puts
        the .dds 'files' in the .ini's folder, and reads the role files with 'rules' over the defaults"""
        self.setupIniTxt(iniTxt)
        self.createIniFile()
        folder = self._iniFile.folder
        for rel in files:
            path = os.path.join(folder, rel)
            os.makedirs(os.path.dirname(path), exist_ok = True)
            if not os.path.exists(path):
                self.addCleanup(lambda path = path: os.path.exists(path) and os.remove(path))
            open(path, "wb").close()

        def target(parser, name, section, disjoint, part, kvps):
            if kvps is None:
                return []
            hashes = part.getVals(FRB.IniKeywords.Hash)
            if hashes and hashes[0].strip() in self.TexHashes:
                return [self.TexHashes[hashes[0].strip()]]
            return [self.Body] if name == "TextureOverrideBody" else []

        texGraphs = [(0,) + self.Diffuse, (0,) + self.Normal]
        overrides = FRB.GraphCollect({graph: [FRB.IniKeywords.Hash, FRB.IniKeywords.This, FRB.IniKeywords.MatchPriority]
                                      for graph in texGraphs}, name = "test.overrides")
        roleKeys = {graph: [FRB.IniKeywords.This] for graph in texGraphs}
        roleKeys[self.BodyGraph] = lambda key: FRB.IniNamingTools.sameKey(key, "ps-t0") or FRB.IniNamingTools.sameKey(key, "ps-t1")
        roleRows = FRB.GraphCollect(roleKeys, name = "test.roles", withConditions = True)

        self._parser = FRB.GIMIParser(self._iniFile, modObjs = [self.Body, self.Diffuse, self.Normal], objTargetFuncs = [target],
                                      disjointModObjs = False,
                                      keysToTrack = {FRB.IniKeywords.Hash, FRB.IniKeywords.MatchFirstIndex})
        steps = FRB.GIMIParser.RowSteps()
        steps.objs = lambda obj: obj == self.Body
        self._parser.addRowSteps(steps)
        self._parser.deferredParseSteps += [overrides, roleRows]
        self.useStrategies(parser = self._parser)
        self._iniFile.parse()

        data = self._parser.parseData
        roleRules = FRB.IniParseData.RoleFileRules()
        roleRules.iniPath, roleRules.folder = self._iniFile.file, folder
        roleRules.sections, roleRules.declared, roleRules.objRows = FRB.IniParseRows.Sections, FRB.IniParseRows.Declared, FRB.IniParseRows.Objs
        roleRules.overrides, roleRules.roleRows = "test.overrides", "test.roles"
        roleRules.objs = [(self.Body, ["TextureOverrideBody"])]
        roleRules.registerRoles = {self.Body: {"ps-t0": "bodyDiffuse", "ps-t1": "bodyNormal"}}
        roleRules.roleOfHash = lambda hash: self.TexHashes[hash][1] if hash in self.TexHashes else None
        roleRules.isFlat = lambda file: False
        for name, value in rules.items():
            setattr(roleRules, name, value)
        data.roleFiles(roleRules)
        return data, folder

    @classmethod
    def sameFile(cls, file, folder, rel):
        return os.path.normcase(os.path.normpath(file)) == os.path.normcase(os.path.normpath(os.path.join(folder, rel)))

    def chosenFile(self, data, folder, role = "bodyDiffuse"):
        chosen = data.roleFile(role, self.Body)
        self.assertIsNotNone(chosen)
        return os.path.relpath(chosen.file, folder)

    _recolours = r"""
[TextureOverrideTexA]
hash = aaaaaaaa
this = ResourceA

[ResourceA]
filename = Textures\A.dds

[ResourceB]
filename = Textures\B.dds
"""

    def test_roleFiles_aFileTheObjectBinds_winsOverOneItsHashDeclares(self):
        data, folder = self.readRoleFiles(r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
ps-t0 = ResourceB
drawindexed = 300, 0, 0
""" + self._recolours, [r"Textures\A.dds", r"Textures\B.dds"])
        self.assertEqual(self.chosenFile(data, folder), os.path.join("Textures", "B.dds"))
        self.assertEqual(len(data.roleCandidates("bodyDiffuse")), 2)

    def test_roleFiles_aDeclaredHash_beatsAFileKnownByItsPixels(self):
        # B sorts first by name and is shorter: it would win on either -- the hash statement must win
        data, folder = self.readRoleFiles(r"""
[TextureOverrideTexA]
hash = aaaaaaaa
this = ResourceZLonger

[ResourceZLonger]
filename = Textures\ZLonger.dds

[ResourceB]
filename = Textures\B.dds
""", [r"Textures\ZLonger.dds", r"Textures\B.dds"],
            identify = lambda file: "bbbbbbbb" if file.endswith("B.dds") else None)
        self.assertEqual(self.chosenFile(data, folder), os.path.join("Textures", "ZLonger.dds"))
        self.assertEqual(data.roleFile("bodyDiffuse", self.Body).tiedFile, "")

    def test_roleFiles_twoHashesOfOneRole_theNewestWins(self):
        iniTxt = r"""
[TextureOverrideTexA]
hash = aaaaaaaa
this = ResourceA

[TextureOverrideTexB]
hash = bbbbbbbb
this = ResourceBLonger

[ResourceA]
filename = Textures\A.dds

[ResourceBLonger]
filename = Textures\BLonger.dds
"""
        files = [r"Textures\A.dds", r"Textures\BLonger.dds"]
        data, folder = self.readRoleFiles(iniTxt, files, hashAge = lambda hash: 0 if hash == "bbbbbbbb" else 1)
        self.assertEqual(self.chosenFile(data, folder), os.path.join("Textures", "BLonger.dds"))

        # with no age, the resource declared first
        data, folder = self.readRoleFiles(iniTxt, files)
        self.assertEqual(self.chosenFile(data, folder), os.path.join("Textures", "A.dds"))
        self.assertTrue(self.sameFile(data.roleFile("bodyDiffuse", self.Body).tiedFile, folder, r"Textures\BLonger.dds"))

    def test_roleFiles_oneHashInTwoSections_theOne3DMigotoAppliesLastWins(self):
        # 3DMigoto runs every section of a hash, sorted by match_priority and then name: the last `this =` stands
        twoSections = r"""
[TextureOverrideTex1]
hash = aaaaaaaa
{0}
this = ResourceA

[TextureOverrideTex2]
hash = aaaaaaaa
this = ResourceBLonger

[ResourceA]
filename = Textures\A.dds

[ResourceBLonger]
filename = Textures\BLonger.dds
"""
        files = [r"Textures\A.dds", r"Textures\BLonger.dds"]
        data, folder = self.readRoleFiles(twoSections.format(""), files)
        self.assertEqual(self.chosenFile(data, folder), os.path.join("Textures", "BLonger.dds"))

        data, folder = self.readRoleFiles(twoSections.format("match_priority = 1"), files)
        self.assertEqual(self.chosenFile(data, folder), os.path.join("Textures", "A.dds"))

    _runToggle = r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
run = CommandListBody
drawindexed = 300, 0, 0

[CommandListBody]
if $x == 1
    ps-t0 = ResourceZLonger
endif
if $x != 1
    ps-t0 = ResourceB
endif

[ResourceZLonger]
filename = Textures\ZLonger.dds

[ResourceB]
filename = Textures\B.dds
"""

    def test_roleFiles_aToggleTheReaderFinds_takesTheVariantAtTheStartingValues(self):
        # $x starts at 0: the `!= 1` branch is what the mod shows before a key is pressed
        data, folder = self.readRoleFiles("[Constants]\nglobal persist $x = 0\n" + self._runToggle,
                                          [r"Textures\ZLonger.dds", r"Textures\B.dds"])
        chosen = data.roleFile("bodyDiffuse", self.Body)
        self.assertEqual(os.path.relpath(chosen.file, folder), os.path.join("Textures", "B.dds"))
        self.assertEqual(len(chosen.variants), 2)

    def test_startingValues_everyConstantWithAValue_orZero(self):
        data, folder = self.readRoleFiles("[Constants]\nglobal persist $x = 1\nlocal $y\nglobal $name = text\n" + self._runToggle,
                                          [r"Textures\ZLonger.dds", r"Textures\B.dds"])
        z3 = FRB.Z3Context()
        start = data.startingValues(FRB.IniParseRows.Sections, z3)
        self.assertIsNotNone(start)
        text = FRB.IfPredPart.getIfPredStr(start)
        self.assertIn("$x == 1", text)
        self.assertIn("$y == 0", text)
        self.assertNotIn("$name", text)
        self.assertIsNone(FRB.IniParseData().startingValues(FRB.IniParseRows.Sections, FRB.Z3Context()))

    def test_roleFiles_aRegisterToggledThroughRun_isAToggle(self):
        data, folder = self.readRoleFiles(r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
run = CommandListBody
drawindexed = 300, 0, 0

[CommandListBody]
if $x == 1
    ps-t0 = ResourceZLonger
endif
if $x != 1
    ps-t0 = ResourceB
endif

[ResourceZLonger]
filename = Textures\ZLonger.dds

[ResourceB]
filename = Textures\B.dds
""", [r"Textures\ZLonger.dds", r"Textures\B.dds"])
        chosen = data.roleFile("bodyDiffuse", self.Body)
        self.assertEqual(os.path.relpath(chosen.file, folder), os.path.join("Textures", "ZLonger.dds"))
        self.assertEqual([os.path.relpath(file, folder) for file in chosen.variants],
                         [os.path.join("Textures", "ZLonger.dds"), os.path.join("Textures", "B.dds")])
        self.assertEqual(chosen.tiedFile, "")
        # filed as a register toggle too, so a fixer editing one variant edits both
        toggle = data.registerToggleOf("resourceb")
        self.assertEqual((toggle.section, toggle.variants), ("TextureOverrideBody ps-t0", ["ResourceZLonger", "ResourceB"]))

    def test_roleFiles_aFileNotOnDisk_orOneAFixWrote_isNoCandidate(self):
        data, folder = self.readRoleFiles(r"""
[TextureOverrideTexA]
hash = aaaaaaaa
this = ResourceA

[ResourceA]
filename = Textures\A.dds

[ResourceDL]
filename = Textures\BodyDiffuseRemapDL.dds
""", [r"Textures\BodyDiffuseRemapDL.dds"], identify = lambda file: "aaaaaaaa")
        self.assertIsNone(data.roleFile("bodyDiffuse", self.Body))
        self.assertEqual(data.roleCandidates("bodyDiffuse"), [])

    def test_roleFiles_aFileDeclaredAsAnotherRole_isNotThisOne(self):
        # N's own hash says it is the normal map, though the body binds it where the diffuse goes
        data, folder = self.readRoleFiles(r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
ps-t0 = ResourceN
drawindexed = 300, 0, 0

[TextureOverrideTexC]
hash = cccccccc
this = ResourceN

[TextureOverrideTexA]
hash = aaaaaaaa
this = ResourceA

[ResourceN]
filename = Textures\N.dds

[ResourceA]
filename = Textures\A.dds
""", [r"Textures\N.dds", r"Textures\A.dds"])
        self.assertEqual(self.chosenFile(data, folder), os.path.join("Textures", "A.dds"))
        self.assertEqual(self.chosenFile(data, folder, "bodyNormal"), os.path.join("Textures", "N.dds"))

    def test_roleFiles_aFlatRegionRole_isRefused(self):
        data, folder = self.readRoleFiles(self._recolours, [r"Textures\A.dds"], regionRoles = {"bodyDiffuse"},
                                          isFlat = lambda file: True)
        self.assertIsNone(data.roleFile("bodyDiffuse", self.Body))
        self.assertEqual([r.role for r in data.rejections()], ["bodyDiffuse"])

    def test_roleFiles_eachResourceFile_andTheResourceTheModUses(self):
        data, folder = self.readRoleFiles(r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
ps-t0 = ResourceUsed
drawindexed = 300, 0, 0

[ResourceDeclaredFirst]
filename = Textures\A.dds

[ResourceUsed]
filename = Textures\A.dds
""", [r"Textures\A.dds"])
        self.assertEqual(data.resourceOfFile(data.roleFile("bodyDiffuse", self.Body).file), "ResourceUsed")
        self.assertEqual([(r.role, r.modObj) for r in data.rolesOfResource("ResourceDeclaredFirst")], [("bodyDiffuse", self.Body)])

    # ====================== IniParseData.buffersBoundBy, WWMISections' bypass pair ===========================

    _sharedLists = r"""
[CommandListOverrideSharedResources]
ResourceBypassVB0 = ref vb0
vb0 = ResourcePosition
ib = ResourceIndex
vb2 = ResourceBlend
VB2 = ResourceBlend2

[CommandListCleanupSharedResources]
vb0 = ref ResourceBypassVB0
"""

    def test_buffersBoundBy_ibAndVbN_eachOnce_notALineThatBindsAResource(self):
        self._rowsIniTxt = self._sharedLists
        data = self.parseRows()
        self.assertEqual(data.buffersBoundBy(FRB.IniParseRows.Sections, "CommandListOverrideSharedResources"), ["vb0", "ib", "vb2"])
        self.assertEqual(data.buffersBoundBy(FRB.IniParseRows.Sections, "Nothing"), [])

    def test_WWMISections_bypasses_whatTheCleanupDoesNotPutBack_andTheirSections(self):
        self._rowsIniTxt = self._sharedLists
        data = self.parseRows()
        bypasses = FRB.WWMISections.bypasses(data, FRB.IniParseRows.Sections, "CommandListOverrideSharedResources",
                                             "CommandListCleanupSharedResources", "Exorcist")
        name = lambda reg: FRB.IniNamingTools.getRemapFixName("ResourceBypass" + FRB.IniNamingTools.getRegTag(reg), "Exorcist")
        self.assertEqual(bypasses, [("ib", name("ib")), ("vb2", name("vb2"))])

        restore = FRB.WWMISections.restoreListName("Exorcist")
        self.assertEqual(restore, FRB.IniNamingTools.getRemapFixName("CommandListRestoreSharedResources", "Exorcist"))
        z3 = FRB.Z3Context()
        sections = FRB.WWMISections.bypassSections(z3, bypasses, restore)
        self.assertEqual([section.name for section in sections], [name("ib"), name("vb2"), restore])
        self.assertEqual(FRB.WWMISections.bypassSections(z3, [], restore), [])

        # a mod without both lists has nothing to be symmetric with
        self.assertEqual(FRB.WWMISections.bypasses(data, FRB.IniParseRows.Sections, "CommandListOverrideSharedResources",
                                                   "CommandListNothing", "Exorcist"), [])

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

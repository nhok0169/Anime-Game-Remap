import sys

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class GraphCollectTest(BaseIniFileTest):
    _collectIniTxt = r"""
[TextureOverrideBody]
hash = 1
match_first_index = 0
ps-t0 = ResourceA
if $x == 1
    ps-t1 = ResourceB
    ps-t3 = ResourceE
else
    ps-t1 = ResourceC
    handling = skip
endif
run = CommandListBody

[CommandListBody]
drawindexed = 10, 0, 0
ps-t2 = ResourceD
"""

    Body = (0, "", "body")

    def parseWith(self, steps):
        self.setupIniTxt(self._collectIniTxt)
        self.createIniFile()
        self._parser = FRB.GIMIParser(self._iniFile, modObjs = [("", "body")], trackKeys = False, parseSteps = steps)
        self.useStrategies(parser = self._parser)
        self._iniFile.parse()
        return self._parser

    def collect(self, keys, **kwargs):
        collect = FRB.GraphCollect({self.Body: keys}, **kwargs)
        self.parseWith([collect])
        return collect.rows

    @classmethod
    def context(cls, row):
        return dict(row.context)

    # ====================== rows ===============================================

    def test_everyOccurrence_inWalkOrder_throughRun(self):
        rows = self.collect(["ps-t1", "ps-t2"])
        self.assertEqual([(row.section, row.key, row.value) for row in rows],
                         [("TextureOverrideBody", "ps-t1", "ResourceB"), ("TextureOverrideBody", "ps-t1", "ResourceC"),
                          ("CommandListBody", "ps-t2", "ResourceD")])
        self.assertEqual([row.line for row in rows], [0, 0, 1])
        self.assertEqual({row.graphId for row in rows}, {self.Body})

    def test_aKeyGivenAlone_isOneKey(self):
        self.assertEqual([row.value for row in self.collect("ps-t2")], ["ResourceD"])

    def test_aTestOfTheKeysName_collectsEveryKeyItAccepts_inFirstAppearanceOrder(self):
        rows = self.collect(lambda key: key.startswith("ps-t"))
        self.assertEqual([(row.key, row.value) for row in rows],
                         [("ps-t0", "ResourceA"), ("ps-t1", "ResourceB"), ("ps-t1", "ResourceC"), ("ps-t3", "ResourceE"),
                          ("ps-t2", "ResourceD")])

    def test_valPredicates_dropAnOccurrence(self):
        rows = self.collect(["ps-t1"], valPredicates = {self.Body: lambda key, val, iterData: val != "ResourceB"})
        self.assertEqual([row.value for row in rows], ["ResourceC"])

    def test_noConditions_byDefault(self):
        rows = self.collect(["ps-t2"])
        self.assertIsNone(rows[0].condition)
        self.assertEqual(rows[0].rootSection, "")

    def test_withConditions_eachRowCarriesItsConditionAndRoot(self):
        rows = self.collect(["ps-t1", "ps-t2"], withConditions = True)
        self.assertTrue(all(row.condition is not None for row in rows))
        self.assertEqual({row.rootSection for row in rows}, {"TextureOverrideBody"})

    def test_guards_theBranchesThePartSitsIn_inItsOwnSection(self):
        rows = self.collect(["ps-t0", "ps-t1"])
        self.assertEqual([(row.value, row.guards) for row in rows],
                         [("ResourceA", []), ("ResourceB", ["$x == 1"]), ("ResourceC", ["else"])])

    def test_guards_ofACalledSection_leaveOutTheCallersBranches(self):
        rows = self.collect(["ps-t2"], withConditions = True)
        self.assertEqual(rows[0].guards, [])

    def test_guards_nested_outermostFirst_andAnElseIfReplacesItsBranch(self):
        self.setupIniTxt(r"""
[TextureOverrideBody]
hash = 1
match_first_index = 0
if $a == 1
    if $b == 2
        ps-t0 = ResourceA
    else if $b == 3
        ps-t0 = ResourceB
    endif
    ps-t0 = ResourceC
endif
ps-t0 = ResourceD
""")
        self.createIniFile()
        collect = FRB.GraphCollect({self.Body: ["ps-t0"]})
        self._parser = FRB.GIMIParser(self._iniFile, modObjs = [("", "body")], trackKeys = False, parseSteps = [collect])
        self.useStrategies(parser = self._parser)
        self._iniFile.parse()
        self.assertEqual([(row.value, row.guards) for row in collect.rows],
                         [("ResourceA", ["$a == 1", "$b == 2"]), ("ResourceB", ["$a == 1", "$b == 3"]),
                          ("ResourceC", ["$a == 1"]), ("ResourceD", [])])

    def test_blocks_twoBranchesOfTheSameTest_areTwoBlocks_andANestedPartStartsWithItsParents(self):
        self.setupIniTxt(r"""
[TextureOverrideBody]
hash = 1
match_first_index = 0
if $a == 1
    ps-t0 = ResourceA
    if $b == 2
        ps-t0 = ResourceB
    else
        ps-t0 = ResourceC
    endif
endif
if $a == 1
    ps-t0 = ResourceD
endif
ps-t0 = ResourceE
""")
        self.createIniFile()
        collect = FRB.GraphCollect({self.Body: ["ps-t0"]})
        self._parser = FRB.GIMIParser(self._iniFile, modObjs = [("", "body")], trackKeys = False, parseSteps = [collect])
        self.useStrategies(parser = self._parser)
        self._iniFile.parse()
        blocks = {row.value: row.blocks for row in collect.rows}

        self.assertEqual({row.value: row.guards for row in collect.rows}["ResourceD"], ["$a == 1"])
        self.assertEqual(len(blocks["ResourceA"]), 1)
        self.assertNotEqual(blocks["ResourceA"], blocks["ResourceD"])
        self.assertEqual(blocks["ResourceB"][:1], blocks["ResourceA"])
        self.assertEqual(blocks["ResourceC"][:1], blocks["ResourceA"])
        self.assertNotEqual(blocks["ResourceB"], blocks["ResourceC"])
        self.assertEqual(blocks["ResourceE"], [])

    def test_parseDataStep_readsWhatEarlierStepsFiled_andFilesIntoTheParsersOwnData(self):
        seen = []
        def file(data):
            seen.extend(row.value for row in data.collected("body"))
            data.setResourceFile("ResourceStep", "step.buf")

        parser = self.parseWith([FRB.GraphCollect({self.Body: ["ps-t1"]}, name = "body"), FRB.ParseDataStep(file)])

        self.assertEqual(seen, ["ResourceB", "ResourceC"])
        self.assertEqual(parser.parseData.resourceFile("ResourceStep"), "step.buf")

    def test_parseDataStep_noFunction_filesNothing(self):
        parser = self.parseWith([FRB.ParseDataStep()])
        self.assertEqual(parser.parseData.collected("body"), [])

    def test_firstCollected_aKeyInTheSectionItself_sectionMatchedRegardlessOfCase(self):
        parser = self.parseWith([FRB.GraphCollect({self.Body: ["ps-t1", "ps-t2"]}, name = "body")])
        data = parser.parseData
        self.assertEqual(data.firstCollected("body", "textureoverridebody", "ps-t1").value, "ResourceB")
        self.assertEqual(data.firstCollected("body", "CommandListBody", "ps-t2").value, "ResourceD")
        self.assertIsNone(data.firstCollected("body", "TextureOverrideBody", "ps-t2"))
        self.assertIsNone(data.firstCollected("other", "TextureOverrideBody", "ps-t1"))

    # ====================== context ============================================

    def test_context_aValueSetInTheCallerBeforeRun_isCarriedWithNoLine(self):
        row = self.collect(["ps-t2"], contextKeys = {self.Body: ["ps-t0"]})[0]
        self.assertEqual(self.context(row)["ps-t0"], [(None, "ResourceA")])

    def test_context_aKeyInTheSamePart_givesItsLines(self):
        row = self.collect(["ps-t2"], contextKeys = {self.Body: ["drawindexed"]})[0]
        self.assertEqual(self.context(row)["drawindexed"], [(0, "10, 0, 0")])

    def test_context_aSiblingBranchIsNeverCarried(self):
        rows = self.collect(["handling"], contextKeys = {self.Body: ["ps-t3"]})
        self.assertEqual(self.context(rows[0])["ps-t3"], [])

    def test_context_aBranchsValueDoesNotLeakPastItsEndif(self):
        row = self.collect(["ps-t2"], contextKeys = {self.Body: ["handling"]})[0]
        self.assertEqual(self.context(row)["handling"], [])

    def test_context_inTheOrderNamed(self):
        row = self.collect(["ps-t2"], contextKeys = {self.Body: ["handling", "drawindexed", "ps-t0"]})[0]
        self.assertEqual([key for key, _ in row.context], ["handling", "drawindexed", "ps-t0"])

    # ====================== in a parser ========================================

    def test_aNamedCollectInParseSteps_filesItsRows(self):
        collect = FRB.GraphCollect({self.Body: ["ps-t1"]}, name = "textures")
        parser = self.parseWith([collect])
        self.assertEqual([row.value for row in parser.parseData.collected("textures")], ["ResourceB", "ResourceC"])
        self.assertEqual(parser.parseData.collected("other"), [])

    def test_anUnnamedCollect_filesNothing(self):
        parser = self.parseWith([FRB.GraphCollect({self.Body: ["ps-t1"]})])
        self.assertEqual(parser.parseData.collected(""), [])

    # ====================== groups =============================================

    def test_everyGroup_collectsInEveryGroupHoldingTheGraph(self):
        parser = self.parseWith([])
        groups = parser.parse()
        FRB.GraphGroupRemap(remap = {self.Body: [self.Body, (1, "", "body", lambda name: name + "Copy")]}).edit(groups, None)

        named = FRB.GraphCollect({self.Body: ["ps-t2"]})
        named.edit(groups, None)
        self.assertEqual([row.graphId for row in named.rows], [self.Body])

        every = FRB.GraphCollect({self.Body: ["ps-t2"]}, everyGroup = True)
        every.edit(groups, None)
        self.assertEqual([row.graphId for row in every.rows], [self.Body, (1, "", "body")])
        self.assertEqual(list(every.keys), [self.Body])

    def test_clear_emptiesTheRows(self):
        collect = FRB.GraphCollect({self.Body: ["ps-t2"]})
        self.parseWith([collect])
        self.assertEqual(len(collect.rows), 1)
        collect.clear()
        self.assertEqual(collect.rows, [])

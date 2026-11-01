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


    # ====================== IniParseData.fileAnatomy ===========================

    def anatomy(self, iniTxt = None, **rules):
        if (iniTxt is not None):
            self._stepsIniTxt = iniTxt
        everyKey = FRB.GraphCollect({self.BodyGraph: lambda key: True}, name = FRB.IniParseRows.Objs)
        parser = self.parseWith([everyKey])
        anatomy = FRB.IniParseData.AnatomyRules()
        anatomy.rows = FRB.IniParseRows.Objs
        anatomy.roots = [(self.Body, ["TextureOverrideBody"])]
        for name, value in rules.items():
            setattr(anatomy, name, value)
        parser.parseData.fileAnatomy(anatomy)
        return parser.parseData

    def test_fileAnatomy_drawsTheirPartsAndLateBindings(self):
        data = self.anatomy()
        self.assertEqual(data.drawRanges(self.Body), [(300, 0)])
        self.assertEqual(data.drawParts(self.Body), {1})
        self.assertEqual(data.lateBindings(self.Body), ["ps-t1"])
        self.assertFalse(data.isUndrawn(self.Body))

    def test_fileAnatomy_aDrawUnderAnIf_sitsInThePartAfterTheIfLine(self):
        data = self.anatomy(r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
if $x == 1
    drawindexed = 300, 0, 0
endif
""")
        self.assertEqual(data.drawParts(self.Body), {3})

    def test_fileAnatomy_aSectionThatSkipsAndDrawsNothing_isUndrawn(self):
        hidden = r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
handling = skip
"""
        self.assertTrue(self.anatomy(hidden).isUndrawn(self.Body))
        self.assertFalse(self.anatomy(hidden, noneUndrawnWhenAll = True).isUndrawn(self.Body))

    def test_fileAnatomy_libraries_andARunThatDraws(self):
        data = self.anatomy(r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
handling = skip
run = CommandList\Lib\Draw
""", libraries = [("Lib", lambda text: "\lib\\" in text.lower())], runDraws = lambda called: True)
        self.assertEqual(data.libraries(self.Body), {"Lib"})
        self.assertFalse(data.isUndrawn(self.Body))

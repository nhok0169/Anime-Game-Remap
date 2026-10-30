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

    # ====================== SkeletonNumberingRead ==============================

    def numberingRead(self, requiredKey = ""):
        numberings = [FRB.SkeletonNumberingProbe.Numbering("1.0"), FRB.SkeletonNumberingProbe.Numbering("2.0", {1: 2})]
        return FRB.SkeletonNumberingRead(numberings, {1: (0.0, 0.0, 0.0)}, "ResourceBodyDiffuse", "ResourceBodyLightMap",
                                         requiredKey = requiredKey)

    def test_skeletonNumberingRead_isAGraphGroupEditWithAResult(self):
        read = self.numberingRead()
        self.assertIsInstance(read, FRB.BaseIniGraphGroupEdit)
        self.assertIsInstance(read, FRB.IParseResult)

    def test_skeletonNumberingRead_noSectionCarriesTheRequiredKey_readsNothing(self):
        read = self.numberingRead(requiredKey = "vg_offset")
        parser = self.parseWith([read])
        self.assertIsNone(read.version)
        self.assertIsNone(parser.parseData.blendVersion())

    def test_skeletonNumberingRead_withoutAnIni_probesNothing(self):
        read = self.numberingRead()
        groups = self.parseWith([]).parse()
        read.edit(groups, None)
        self.assertIsNone(read.version)

    def test_skeletonNumberingRead_fileInto_withNoVersion_leavesTheData(self):
        data = FRB.IniParseData()
        self.numberingRead().fileInto(data)
        self.assertIsNone(data.blendVersion())

import sys
from ordered_set import OrderedSet

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ParseStepsTest(BaseIniFileTest):
    _stepsIniTxt = r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
ps-t0 = ResourceBodyDiffuse
drawindexed = 300, 0, 0
if $toggle == 1
    drawindexed = 120, 300, 0
endif
ps-t1 = ResourceBodyLightMap
run = CommandList\RabbitFX\Run

[TextureOverrideHair]
hash = 11111111
match_first_index = 420
handling = skip

[TextureOverrideSkirt]
hash = 11111111
match_first_index = 900
handling = skip
run = CommandListSkirt

[CommandListSkirt]
drawindexed = auto

[TextureOverrideRecolour]
hash = 22222222
this = ResourceBodyDiffuse

[ResourceBodyDiffuse]
filename = Textures\Body.dds

[ResourceBodyLightMap]
filename = Textures\BodyLM.dds
"""

    Body = ("", "body")
    Hair = ("", "hair")
    Skirt = ("", "skirt")

    def parseWith(self, steps, iniTxt = None):
        self.setupIniTxt(self._stepsIniTxt if iniTxt is None else iniTxt)
        self.createIniFile()
        self._parser = FRB.GIMIParser(self._iniFile, modObjs = OrderedSet([self.Body, self.Hair, self.Skirt]),
                                      trackKeys = False, parseSteps = steps)
        self.useStrategies(parser = self._parser)
        self._iniFile.parse()
        return self._parser.parseData

    # ====================== GIMIParser.parseSteps ===============================

    def test_parseSteps_defaultsToAnEmptyList(self):
        self.setupIniTxt(self._stepsIniTxt)
        self.createIniFile()
        parser = FRB.GIMIParser(self._iniFile)
        self.assertEqual(list(parser.parseSteps), [])

    def test_parseSteps_isTheCallersOwnList(self):
        steps = [FRB.ResourceRead()]
        self.setupIniTxt(self._stepsIniTxt)
        self.createIniFile()
        parser = FRB.GIMIParser(self._iniFile, parseSteps = steps)
        self.assertIs(parser.parseSteps, steps)

    def test_parseSteps_aStepRunsAtTheEndOfTheParse(self):
        data = self.parseWith([FRB.ResourceRead()])
        self.assertEqual(data.resourceFile("ResourceBodyDiffuse"), r"Textures\Body.dds")

    def test_parseSteps_anEditRunsWhereItSits_aStepBeforeItReadsTheUneditedGraphs(self):
        remove = FRB.GraphGroupEdit(edits = [{self.Body: [FRB.RegRemove({"ps-t1": None})]}])
        data = self.parseWith([FRB.DrawRead(), remove])
        self.assertEqual(data.lateBindings(self.Body), ["ps-t1"])

    def test_parseSteps_aDeferredStepRunsOnTheFirstAsk_afterEveryEditOfTheList(self):
        read = FRB.DrawRead()
        read.deferred = True
        remove = FRB.GraphGroupEdit(edits = [{self.Body: [FRB.RegRemove({"ps-t1": None})]}])
        data = self.parseWith([read, remove])
        self.assertEqual(data.lateBindings(self.Body), [])
        self.assertEqual(data.drawRanges(self.Body), [(300, 0), (120, 300)])

    # ====================== ResourceRead =======================================

    def test_resourceRead_everySectionWithAFilename(self):
        data = self.parseWith([FRB.ResourceRead()])
        self.assertEqual(data.resourceFiles(), {"resourcebodydiffuse": r"Textures\Body.dds",
                                                "resourcebodylightmap": r"Textures\BodyLM.dds"})

    # ====================== TextureOverrideRead ================================

    def test_textureOverrideRead_aHashAndAThis_replacesByHash(self):
        data = self.parseWith([FRB.TextureOverrideRead()])
        self.assertTrue(data.replacesTexturesByHash())

    def test_textureOverrideRead_noThis_doesNotReplace(self):
        data = self.parseWith([FRB.TextureOverrideRead()], iniTxt = self._stepsIniTxt.replace("this = ResourceBodyDiffuse", ""))
        self.assertFalse(data.replacesTexturesByHash())

    def test_textureOverrideRead_aDrawsThis_isNotATextureOverride(self):
        iniTxt = self._stepsIniTxt.replace("this = ResourceBodyDiffuse", "").replace("ps-t0 = ResourceBodyDiffuse", "this = ResourceBodyDiffuse")
        self.assertTrue(self.parseWith([FRB.TextureOverrideRead()], iniTxt = iniTxt).replacesTexturesByHash())
        self.assertFalse(self.parseWith([FRB.TextureOverrideRead(draws = lambda obj: obj == self.Body)], iniTxt = iniTxt).replacesTexturesByHash())

    # ====================== DrawRead ===========================================

    def test_drawRead_rangesAndTheirParts(self):
        data = self.parseWith([FRB.DrawRead()])
        self.assertEqual(data.drawRanges(self.Body), [(300, 0), (120, 300)])
        self.assertEqual(len(data.drawParts(self.Body)), 2)
        self.assertEqual(data.drawRanges(self.Skirt), [])

    def test_drawRead_aRegisterBoundAfterADraw_isLate(self):
        data = self.parseWith([FRB.DrawRead()])
        self.assertEqual(data.lateBindings(self.Body), ["ps-t1"])

    def test_drawRead_aModReplacingByHash_hasNoLateBindings(self):
        data = self.parseWith([FRB.TextureOverrideRead(), FRB.DrawRead()])
        self.assertEqual(data.lateBindings(self.Body), [])

    def test_drawRead_skipsAndDrawsNothing_isUndrawn_aCalledDrawCounts(self):
        data = self.parseWith([FRB.DrawRead()])
        self.assertEqual(data.undrawnObjs(), {self.Hair})

    def test_drawRead_everyReadObjWouldBeUndrawn_noneIs(self):
        data = self.parseWith([FRB.DrawRead(objs = lambda obj: obj == self.Hair)])
        self.assertEqual(data.undrawnObjs(), set())

    def test_drawRead_anUnresolvedCall_drawsUnlessItNamesASilentLibrary(self):
        iniTxt = self._stepsIniTxt.replace("[CommandListSkirt]\ndrawindexed = auto", "").replace("run = CommandListSkirt", r"run = CommandList\Lib\Skirt")
        self.assertEqual(self.parseWith([FRB.DrawRead()], iniTxt = iniTxt).undrawnObjs(), {self.Hair})
        self.assertEqual(self.parseWith([FRB.DrawRead(silentLibraries = ["\\LIB\\"])], iniTxt = iniTxt).undrawnObjs(), {self.Hair, self.Skirt})

    # ====================== LibraryRead ========================================

    def test_libraryRead_aLineNamingTheLibrary(self):
        data = self.parseWith([FRB.LibraryRead([("RabbitFX", "rabbitfx")])])
        self.assertEqual(data.libraries(self.Body), {"RabbitFX"})
        self.assertEqual(data.libraries(self.Hair), set())

    def test_libraryRead_objsFilter(self):
        data = self.parseWith([FRB.LibraryRead([("RabbitFX", "rabbitfx")], objs = lambda obj: obj != self.Body)])
        self.assertEqual(data.libraries(self.Body), set())

    # ====================== SkeletonNumberingRead ==============================

    def test_skeletonNumberingRead_noSectionCarriesTheRequiredKey_readsNothing(self):
        numberings = [FRB.SkeletonNumberingProbe.Numbering("1.0"), FRB.SkeletonNumberingProbe.Numbering("2.0", {1: 2})]
        step = FRB.SkeletonNumberingRead(numberings, {1: (0.0, 0.0, 0.0)}, "ResourceBodyDiffuse", "ResourceBodyLightMap",
                                         requiredKey = "vg_offset")
        data = self.parseWith([FRB.ResourceRead(), step])
        self.assertIsNone(data.blendVersion())

import sys, os
from ordered_set import OrderedSet

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class GIMIParserTest(BaseIniFileTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._parser = None

    def createParser(self):
        self._parser = FRB.GIMIParser(self._iniFile, modObjs = OrderedSet([("", "blend"), ("", "texcoord")]), 
                                      downloads = {("", "texcoord"): {"vb0": FRB.DownloadData("testPosition", FRB.FileDownload("anotherURL", "anotherBaseFile")),
                                                                      "vb1": FRB.DownloadData("testTexture", FRB.FileDownload("someURL", "someBaseFile"))},
                                                   ("", "blend"): {"ps-t0": FRB.DownloadData("testDiffuse", FRB.FileDownload("unknownURL", "unknownBaseFile"), refToSection = True),
                                                                   "ps-t1": FRB.DownloadData("testLightMap", FRB.FileDownload("uniqueURL", "uniqueBaseFile"), refToSection = False)}},
                                      commandEdits = FRB.GraphGroupEdit(edits = [{("", "blend"): [FRB.RegFillMissing("handling2", [("handling2", "skip"), ("drawindexed2", "auto")], fillMode = FRB.RegFillMissingMode.TopdownCover, dependOnDownload = True),
                                                                                                  FRB.RegFillMissing("ib", "null", dependOnDownload = True)]}]))

    def create(self):
        self.createIniFile()
        self.createParser()
        self._iniFile._iniParser = self._parser

    # ====================== parse =======================================

    def test_textureOverrideRootFound_parsedDataFromIniTxt(self):
        tests = [
                 [self._defaultIniTxt, 
"""[TextureOverrideRaidenShogunBlend]
handling2 = skip
drawindexed2 = auto
ps-t0 = ResourceRaidenTestDiffuseRemapDL
run = CommandListRaidenShogunBlend
handling = skip
draw = 21916,0

[CommandListRaidenShogunBlend]
if $swapmain == 0
\tif $swapvar == 0 && $swapvarn == 0
\t\tvb1 = ResourceRaidenShogunBlend.0
\t\tps-t1 = ResourceRaidenTestLightMapRemapDL
\t\tib = null
\telse
\t\tvb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
\t\tps-t1 = ResourceRaidenTestLightMapRemapDL
\t\tib = null
\tendif
else if $swapmain == 1
\trun = SubSubTextureOverride
endif

[SubSubTextureOverride]
if $swapoffice == 0 && $swapglasses == 0
\tvb1 = GIMINeedsResourcesToAllStartWithResource
\tps-t1 = ResourceRaidenTestLightMapRemapDL
\tib = null
endif

[TextureOverrideRaidenTexcoordRemapFix]
vb0 = ResourceRaidenTestPositionRemapDL
vb1 = ResourceRaidenTestTextureRemapDL

[ResourceRaidenTestPositionRemapDL]
filename = anotherBaseFile

[ResourceRaidenTestTextureRemapDL]
filename = someBaseFile

[ResourceRaidenTestDiffuseRemapDL]
filename = unknownBaseFile

[ResourceRaidenTestLightMapRemapDL]
filename = uniqueBaseFile""", 4]]

        for test in tests:
            iniTxt = test[0]
            self.setupIniTxt(iniTxt)
            self.create()
            self._iniFile.parse()

            expected = test[1]
            expectedDownloadCount = test[2]

            result = []
            graphs = self._parser.commandGraphs
            for modObj in graphs:
                result.append(graphs[modObj].toStr())

            downloadGraphs = self._parser.downloadResourceGraphs
            for modObj in downloadGraphs:
                for reg in downloadGraphs[modObj]:
                    result.append(downloadGraphs[modObj][reg].toStr())

            result = "\n\n".join(result)

            self.assertEqual(result, expected)
            self.assertEqual(len(self._iniFile.fileDownloads), expectedDownloadCount)

    # ====================================================================
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

    def _getIbOnly(self, iterData: FRB.SectionIterData, modType: FRB.ModType, ini: FRB.IniFile):
        result = iterData.colouring.getRanges(keysExists = {"hash": True, "match_first_index": False}, keyFilters = {"hash": lambda ind, val: modType.hashes.hasFrom(val, version = ini.version, nonVersionVals = {"type": "ib"})}, includeKeyDefs = False)
        return result

    def createParser(self):
        self._parser = FRB.GIMIParser(self._iniFile, modObjs = OrderedSet([("", "blend"), ("", "texcoord"), ("", "body"), ("", "ib")]), 
                                      downloads = {("", "texcoord"): {"vb0": FRB.DownloadData("testPosition", FRB.FileDownload("anotherURL", "anotherBaseFile")),
                                                                      "vb1": FRB.DownloadData("testTexture", FRB.FileDownload("someURL", "someBaseFile"))},
                                                   ("", "blend"): {"ps-t0": FRB.DownloadData("testDiffuse", FRB.FileDownload("unknownURL", "unknownBaseFile"), refToSection = True),
                                                                   "ps-t1": FRB.DownloadData("testLightMap", FRB.FileDownload("uniqueURL", "uniqueBaseFile"), refToSection = False)}},
                                      commandEdits = FRB.GraphGroupEdit(edits = [{("", "blend"): [FRB.RegFillMissing("handling2", [("handling2", "skip"), ("drawindexed2", "auto")], fillMode = FRB.RegFillMissingMode.TopdownCover, dependOnDownload = True),
                                                                                                  FRB.RegFillMissing("ib", "null", dependOnDownload = True)],
                                                                                  ("", "body"): [FRB.RegFillMissing("ps-t999", "DigitOverflow")],
                                                                                  ("", "ib"): [FRB.RegFillMissing("ps-t2,147,483,647", "DigitUnderflow"), FRB.RegNewVals({"hash": "Matsuribayashi-hen"})]}],
                                                                        trackKeys = [{("", "ib"): True}],
                                                                        keysToTrack = [{("", "ib"): {"hash", "match_first_index"}}],
                                                                        keyFilters = [{("", "ib"): self._getIbOnly}]))

    def create(self):
        self.createIniFile()
        self.createParser()
        self._iniFile._iniParser = self._parser

    def createNamedParser(self):
        self.create()
        self._parser.trackKeys = False
        self._parser.objTargetFuncs = []

    def createKeyedParser(self):
        self.create()

        sectionClassifier = FRB.GIMISectionClassifier.buildDefaultClassifierFromIni(self._iniFile)
        sectionClassifier.hashKeyOnlyToModObj = {
            "blend_vb": ("", "blend"), 
            "texcoord_vb": ("", "texcoord"),
            "ib": ("", "ib")
        }

        sectionClassifier.indexKeyToModObj = {
            "ib": {("", "body"): ("", "body")}
        }

        self._parser.trackKeys = True
        self._parser.keysToTrack = {FRB.IniKeywords.Hash.value, FRB.IniKeywords.MatchFirstIndex.value}
        self._parser.objTargetFuncs = [sectionClassifier]

    # ====================== parse =======================================

    def test_textureOverrideRootFoundByName_parsedDataFromIniTxt(self):
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
            self.createNamedParser()
            self._iniFile.parse()

            expected = test[1]
            expectedDownloadCount = test[2]

            result = []
            graphs = self._parser.commandGraphs
            for modObj in graphs:
                graphStr = graphs[modObj].toStr()
                if (graphStr):
                    result.append(graphs[modObj].toStr())

            downloadGraphs = self._parser.downloadResourceGraphs
            for modObj in downloadGraphs:
                for reg in downloadGraphs[modObj]:
                    graphStr = downloadGraphs[modObj][reg].toStr()
                    if (graphStr):
                        result.append(graphStr)

            result = "\n\n".join(result)

            self.assertEqual(result, expected)
            self.assertEqual(len(self._iniFile.fileDownloads), expectedDownloadCount)

    def test_textureOverrideRootFoundByKVP_parsedDataFromIniTxt(self):
        tests = [
                 [self._defaultIniTxt, 
"""[TextureOverrideRaidenBlendRemapFix]
handling2 = skip
drawindexed2 = auto
ps-t0 = ResourceRaidenTestDiffuseRemapDL
ps-t1 = ResourceRaidenTestLightMapRemapDL
ib = null

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
filename = uniqueBaseFile""", 4],

[
"""
[TextureOverridelittleblacknekowitchBlend]
hash = rikaTheWitchOfFate

[GoogooGaaGaaBlend]
hash = kuroneko

[NanaTex]
hash = rena's going to take you home3

[DaDaIb]
hash = Himatsubushi-hen

[DaDaIb2]
hash = Himatsubushi-hen
match_first_index = protocolSignalGenerator

[DaDaBody]
hash = Himatsubushi-hen
match_first_index = uryu uryu! Slap by Rosa...
"""
,
"""[GoogooGaaGaaBlend]
handling2 = skip
drawindexed2 = auto
ps-t0 = ResourceBernkastelTestDiffuseRemapDL
hash = kuroneko
ps-t1 = ResourceBernkastelTestLightMapRemapDL
ib = null

[NanaTex]
hash = rena's going to take you home3
vb0 = ResourceBernkastelTestPositionRemapDL

[DaDaBody]
hash = Himatsubushi-hen
match_first_index = uryu uryu! Slap by Rosa...
ps-t999 = DigitOverflow

[DaDaIb]
hash = Matsuribayashi-hen
ps-t2,147,483,647 = DigitUnderflow

[DaDaIb2]
hash = Himatsubushi-hen
match_first_index = protocolSignalGenerator
ps-t2,147,483,647 = DigitUnderflow

[ResourceBernkastelTestPositionRemapDL]
filename = anotherBaseFile

[ResourceBernkastelTestTextureRemapDL]
filename = someBaseFile

[ResourceBernkastelTestDiffuseRemapDL]
filename = unknownBaseFile

[ResourceBernkastelTestLightMapRemapDL]
filename = uniqueBaseFile""", 4]]

        for test in tests:
            iniTxt = test[0]
            self.setupIniTxt(iniTxt)
            
            self.createKeyedParser()
            self._parser.trackKeys = True
            self._parser.keysToTrack = {FRB.IniKeywords.Hash.value, FRB.IniKeywords.MatchFirstIndex.value}

            self._iniFile.parse()

            expected = test[1]
            expectedDownloadCount = test[2]

            result = []
            graphs = self._parser.commandGraphs
            for modObj in graphs:
                graphStr = graphs[modObj].toStr()
                if (graphStr):
                    result.append(graphStr)

            downloadGraphs = self._parser.downloadResourceGraphs
            for modObj in downloadGraphs:
                for reg in downloadGraphs[modObj]:
                    downloadStr = downloadGraphs[modObj][reg].toStr()
                    if (downloadStr):
                        result.append(downloadStr)

            result = "\n\n".join(result)

            self.assertEqual(result, expected)
            self.assertEqual(len(self._iniFile.fileDownloads), expectedDownloadCount)

    # ====================================================================
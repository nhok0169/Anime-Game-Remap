import sys
from ordered_set import OrderedSet

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class GraphGroupRemapTest(BaseIniFileTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._parser = None
        cls._fixer = None

    def createParser(self):
        self._parser = FRB.GIMIParser(self._iniFile, modObjs = OrderedSet([("", "blend"), ("", "texcoord")]), 
                                      downloads = {("", "texcoord"): {"vb0": FRB.DownloadData("testPosition", FRB.FileDownload("anotherURL", "anotherBaseFile")),
                                                                      "vb1": FRB.DownloadData("testTexture", FRB.FileDownload("someURL", "someBaseFile"))},
                                                   ("", "blend"): {"ps-t0": FRB.DownloadData("testDiffuse", FRB.FileDownload("unknownURL", "unknownBaseFile"), refToSection = True),
                                                                   "ps-t1": FRB.DownloadData("testLightMap", FRB.FileDownload("uniqueURL", "uniqueBaseFile"), refToSection = False)}},
                                      commandEdits = FRB.GraphGroupEdit(edits = [{("", "blend"): [FRB.RegFillMissing("handling2", [("handling2", "skip"), ("drawindexed2", "auto")], fillMode = FRB.RegFillMissingMode.TopdownCover, dependOnDownload = True),
                                                                                                  FRB.RegFillMissing("ib", "null", dependOnDownload = True)]}]))
        
    def createFixer(self):
        self._fixer = FRB.GIMIFixer(self._parser,
                                    modsToFix = ["rika"])                                                                       

    def create(self):
        self.createIniFile()
        self.createParser()
        self.createFixer()
        self._iniFile._iniParser = self._parser
        self._iniFile._iniFixer = self._fixer

    # ====================== edit =======================================

    def test_differentObjRemaps_graphGroupsRemapped(self):
        self.create()

        tests = [
                 [[FRB.GraphGroupRemap(remap = {})], [{("", "blend"), ("", "texcoord"), ("download", "testPosition"), 
                                                       ("download", "testTexture"), ("download", "testDiffuse"), ("download", "testLightMap")}]],
                 [[FRB.GraphGroupRemap(remap = {(0, "", "blend"): [(0, "", "ultrablend"), (1, "", "megablend")]})], [{("", "ultrablend"), ("", "texcoord"), ("download", "testPosition"), 
                                                       ("download", "testTexture"), ("download", "testDiffuse"), ("download", "testLightMap")}, {("", "megablend")}]],
                 [[FRB.GraphGroupRemap(remap = {(0, "", "blend"): [(0, "", "ultrablend"), (0, "", "ultrablend")]})], [{("", "ultrablend"), ("", "texcoord"), ("download", "testPosition"), 
                                                       ("download", "testTexture"), ("download", "testDiffuse"), ("download", "testLightMap")}, {("", "ultrablend")}]],
                 [[FRB.GraphGroupRemap(remap = {(0, "", "blend"): [(0, "", "ultrablend"), (0, "", "ultrablend")]}),
                   FRB.GraphGroupRemap(remap = {(1, "", "ultrablend"): []})], [{("", "ultrablend"), ("", "texcoord"), ("download", "testPosition"), 
                                                                                ("download", "testTexture"), ("download", "testDiffuse"), ("download", "testLightMap")}]],
                 [[FRB.GraphGroupRemap(remap = {(0, "", "blend"): [(0, "", "ultrablend"), (0, "", "megablend")]})], [{("", "ultrablend"), ("", "megablend"), ("", "texcoord"), ("download", "testPosition"), 
                                                        ("download", "testTexture"), ("download", "testDiffuse"), ("download", "testLightMap")}]],
                 [[FRB.GraphGroupRemap(remap = {(0, "", "blend"): [(0, "", "blend"), (0, "", "megablend")]})], [{("", "blend"), ("", "megablend"), ("", "texcoord"), ("download", "testPosition"), 
                                                       ("download", "testTexture"), ("download", "testDiffuse"), ("download", "testLightMap")}]],
                 [[FRB.GraphGroupRemap(remap = {(0, "", "blend"): [(0, "", "texcoord")],
                                                (0, "", "texcoord"): [(0, "", "blend")]})], 
                                                [{("", "blend"), ("", "texcoord"), ("download", "testPosition"), 
                                                  ("download", "testTexture"), ("download", "testDiffuse"), ("download", "testLightMap")}]]
                 ]

        for test in tests:
            remaps = test[0]

            self._fixer.graphGroupEdits = remaps

            self._iniFile.parse()
            temp = self._iniFile.fix()

            result = self._fixer.graphGroups
            expectedModObjs = test[1]
            expectedModObjsLen = len(expectedModObjs)

            self.assertEqual(len(result), expectedModObjsLen)

            for i in range(expectedModObjsLen):
                resultIniModObjs = set(result[i].graphs.keys())
                expectedIniModObjs = expectedModObjs[i]

                self.compareSet(resultIniModObjs, expectedIniModObjs)

    # ===================================================================
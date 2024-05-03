import sys
import re
import unittest.mock as mock
from .baseFileUnitTest import BaseFileUnitTest
from typing import List, Optional

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )')
import src.FixRaidenBoss2.FixRaidenBoss2 as FRB


class ModTest(BaseFileUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        
        cls._mod = None
        cls._modPath = ""
        cls._modFiles = None
        cls._modTypes = { FRB.ModTypes.Raiden.value, 
                          FRB.ModType("Bernkastel", re.compile(r"\[\s*LittleBlackNekoWitch\s*\]"), "kuroneko", aliases = ["Frederica Bernkastel", "Bern-chan", "Rika Furude", "Nipah!"]) }
        
        cls._defaultModType = FRB.ModType("Kyrie", re.compile(r"\[\s*AgnusDei\s*\]"), "Dies Irae")

        cls._folderTree1 = {"subTree1": {"big branch": {"blender.buf": None}},
                            "mainTree": {"raiden": {"shogun.ini": None,
                                                    "eiRemapBlend.ini": None,
                                                    "bad_ei.buf": None},
                                         "agnes": {"oblige.ini": None,
                                                   "unacceptable.ini": None,
                                                   "bad_wind.buf": None},
                                         "nino": {"bad_girl.ini": None}},
                            "subTree2": {"trunk": {"twig": {"blendy.buf": None,
                                                            "bad_boy.buf": None},
                                                   "dead branch": {"bad_RemapBlend.buf": None}}},
                            "seed.txt": None,
                            "leaf.ini": None}
        
        cls.setupFolderTree(cls._folderTree1)

    @classmethod
    def setupFolderTree(cls, newFolderTree):
        super().setupFolderTree(newFolderTree)
        cls.setupModFiles(list(filter(cls.isFile, cls._currentDirItems)))

    @classmethod
    def setupModFiles(cls, newFiles: List[str]):
        cls._modFiles = newFiles

    def setUp(self):
        super().setUp()
        self.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.getPath", side_effect = lambda path: self._modPath)

    def createMod(self):
        self._mod = FRB.Mod(self._modPath, files = self._modFiles, types = self._modTypes, defaultType = self._defaultModType)

    def blendCorrection(self, blendFile: str, modType: Optional[FRB.ModType], fixedBlendFile: str) -> Optional[str]:
        if (not modType.vgRemap):
            return None
        
        if (blendFile.find("bad") > -1):
            raise FRB.BlendFileNotRecognized(blendFile)
        return blendFile
    
    def parseIni(self, iniFile: Optional[str]):
        if (iniFile is not None and iniFile.find("Bad") > -1):
            raise FloatingPointError("bad ini")

    # ====================== fileTxt.setter ==============================

    def test_noNewCurrentDirFiles_fileFromCurrentDir(self):
        self.setupModFiles(["boo.txt"])
        self.createMod()
        self.compareList(self._mod.files, self._modFiles)

        self.setupFolderTree(self._folderTree1)
        self._mod.files = None
        self.compareList(self._mod.files, self._modFiles)

    def test_hasCurrentDirFiles_newCurrentDirFiles(self):
        self.setupModFiles(None)
        self.createMod()
        self.compareList(self._mod.files, list(filter(self.isFile, self._currentDirItems)))

        newFiles = ["boo.txt"]
        self._mod.files = newFiles
        self.compareList(self._mod.files, newFiles)

    # ====================================================================
    # ====================== isIni =======================================

    def test_differentFileNames_fileIsIni(self):
        fileTests = {".ini": True,
                     "ini": False,
                     "hello.poo.ini": True,
                     "hello.txt": False,
                     "": False}
        
        for fileName in fileTests:
            result = FRB.Mod.isIni(fileName)
            self.assertEqual(result, fileTests[fileName])

    # ====================================================================
    # ====================== isRemapBlend ================================

    def test_differentFileNames_fileIsRemapBlend(self):
        fileTests = {"": False,
                     ".buf": False,
                     "Blend.buf": False,
                     "RemapBlend.buf": True,
                     "RemapBlendRemapBlend.buf": True,
                     "EiRemapsHerBlend.buf": False,
                     "EiRemapBlendsHerRemapBlends.buf": True,
                     "EiRemapBlend.txt": False,
                     "Ei.buf.RemapBlend": False}
        
        for fileName in fileTests:
            result = FRB.Mod.isRemapBlend(fileName)
            self.assertEqual(result, fileTests[fileName])

    # ====================================================================
    # ====================== isBlend =====================================

    def test_differentFileNames_fileIsBlend(self):
        fileTests = {"": False,
                     ".buf": False,
                     "Blend.buf": True,
                     "RemapBlend.buf": False,
                     "EiRemapBlendsHerBlend.buf": False,
                     "EiBlendHerBlend.buf": True,
                     "EiBlend.txt": False,
                     "Ei.buf.Blend": False}
        
        for fileName in fileTests:
            result = FRB.Mod.isBlend(fileName)
            self.assertEqual(result, fileTests[fileName])

    # ====================================================================
    # ====================== isBackupIni =================================

    def test_differentFileNames_fileIsBackupIni(self):
        fileTests = {"": False,
                    ".txt": False,
                    "DISABLED_BossFixBackup_.txt": True,
                    "Ei_DISABLED_BossFixBackup_Cuz She Want_to.txt": False,
                    "DISABLED_BossFixBackup_Cuz She Want_to.txt": True,
                    "DISABLED_BossFixBackup_DISABLED_BossFixBackup_.txt": True,
                    "Ei.txt.DISABLED_BossFixBackup_": False}
    
        for fileName in fileTests:
            result = FRB.Mod.isBackupIni(fileName)
            self.assertEqual(result, fileTests[fileName])

    # ====================================================================
    # ====================== getOptionalFiles ============================

    def test_differentModFiles_differentFilePartitions(self):
        self.createMod()
        fileTests = [[{}, [[], [], []]],
                     [self._folderTree1, [["leaf.ini"], [], []]],
                     [self._folderTree1["mainTree"]["raiden"], [["shogun.ini", "eiRemapBlend.ini"], [], []]],
                     [{"helloRemapBlend.buf": None,
                      "bang.ini": None,
                      "DISABLED_BossFixBackup_bye.txt": None}, [["bang.ini"], ["helloRemapBlend.buf"], ["DISABLED_BossFixBackup_bye.txt"]]],
                      [{"helloRemapBlend.buf": None,
                      "bang.ini": None,
                      "DISABLED_BossFixBackup_bye.txt": None,
                      "helloRemapBlend2.buf": None,
                      "bang2.ini": None,
                      "DISABLED_BossFixBackup_bye2.txt": None}, [["bang.ini", "bang2.ini"], ["helloRemapBlend.buf", "helloRemapBlend2.buf"], ["DISABLED_BossFixBackup_bye.txt", "DISABLED_BossFixBackup_bye2.txt"]]]]
        
        for test in fileTests:
            self.setupFolderTree(test[0])
            self._mod.files = self._modFiles
            result = self._mod.getOptionalFiles()

            expected = test[1]
            resultLen = len(result)
            self.assertEqual(resultLen, len(expected))
            for i in range(resultLen):
                self.compareList(result[i], expected[i])

    # ====================================================================
    # ====================== removeBackupInis ============================

    def test_noFiles_noBackupInisRemoved(self):
        self.setupFolderTree({})
        self.createMod()

        self._mod.removeBackupInis()
        self.assertEqual(bool(self._flattendDirItems), False)

    def test_hasFiles_backupInisRemoved(self):
        self.createMod()
        modFileTests = [[{"bang.ini": None, "boo.txt": None}, {"./bang.ini", "./boo.txt"}], 
                        [{"DISABLED_BossFixBackup_bang.txt": None, "search.jsx": None, "web.config": None}, {"./search.jsx", "./web.config"}],
                        [{"DISABLED_BossFixBackup_dev.txt": None, "DISABLED_BossFixBackup_prod.txt": None}, set()]]
        
        for test in modFileTests:
            testFiles = test[0]
            self.setupFolderTree(testFiles)
            self._mod.files = list(map(lambda file: f"./{file}", self._modFiles))

            self._mod.removeBackupInis()
            self.compareSet(self._flattendDirItems, test[1]) 

    # ====================================================================
    # ====================== removeFix ===================================

    def test_noInis_nothingRemoved(self):
        self.createMod()
        self._mod.inis = []
        resultUndoeInis, resultRemovedBlends = self._mod.removeFix({"fixedBlend"}, {"fixedIni"}, {"visitedBlend"}, {"skippedInis": FloatingPointError("bad ini")})
        self.compareSet(resultUndoeInis, set())
        self.compareSet(resultRemovedBlends, set())

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.IniFile.parse")
    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.IniFile.removeFix")
    def test_differentInis_inisUndoedRemapsRemoved(self, m_removeFix, m_parse):
        self._modTypes = None
        self._flattendDirItems = list(map(lambda path: self.osPathJoin(self.absPath, path[2:]), self._flattendDirItems))
        self.createMod()

        fixedBlends = {f"imafixed.buf", f"helo/dfdfdf/../rtrtrtrt/fixedBlend.txt", "subTree1/big branch/blender.buf"}
        fixedInis = {"Bob the Fixer.ini", "Tractors/John Deer.json", "mainTree/agnes/unacceptable.ini", "NonExistent.ini"}
        visitedBlends = {"Neighbourhood/Neighbour/Visited.buf", "bellman-ford/ford fulkerson", "subTree2/trunk/twig/bad_boy.buf"}
        inisSkipped = {"Im Skipped.ini"}

        inisTest = [[[[FRB.IniFile(), []],
                      [FRB.IniFile("NonExistent.ini"), [FRB.RemapBlendModel(self.absPath, "someRemapResource", {1 : "subTree2/trunk/twig/blendy.buf", 2: "subTree1/big branch/blender.buf"})]],
                      [FRB.IniFile("Bad.ini"), [FRB.RemapBlendModel(self.absPath, "anotherResource", {3: "subTree2/trunk/twig/bad_boy.buf"}),
                                                FRB.RemapBlendModel(self.absPath, "3rdResource", {5: "subTree2/trunk/dead branch/bad_RemapBlend.buf",
                                                                                                  45: "mainTree/agnes/unacceptable.ini"})]]], 
                        set(), {"subTree2/trunk/twig/blendy.buf", "mainTree/agnes/unacceptable.ini", "subTree2/trunk/dead branch/bad_RemapBlend.buf"}, 
                        {"subTree2/trunk/twig/blendy.buf", "subTree2/trunk/dead branch/bad_RemapBlend.buf", "mainTree/agnes/unacceptable.ini"}.union(visitedBlends),
                        {"Bad.ini": FloatingPointError("bad ini"), "Im Skipped.ini": FloatingPointError("bad ini")}]]
        
        getAbsPath = lambda file: self.osPathJoin(self.absPath, file)
        for test in inisTest:
            inisData = test[0]
            for iniData in inisData:
                ini = iniData[0]
                ini.remapBlendModels = iniData[1]

                if (ini.remapBlendModels):
                    ini._isModIni = True

            m_parse.side_effect = lambda: self.parseIni(iniData[0].file)

            self._mod.inis = list(map(lambda iniData: iniData[0], inisData))
            currentVisitedBlends = set(map(getAbsPath,visitedBlends))

            currentInisSkipped = {}
            for ini in inisSkipped:
                currentInisSkipped[getAbsPath(ini)] = FloatingPointError("bad ini")

            resultUndoedInis, resultRemovedBlends = self._mod.removeFix(set(map(getAbsPath, fixedBlends)), set(map(getAbsPath, fixedInis)), currentVisitedBlends, currentInisSkipped)
            self.compareSet(resultUndoedInis, set(map(getAbsPath, test[1])))
            self.compareSet(resultRemovedBlends, set(map(getAbsPath, test[2]))) 
            self.compareSet(currentVisitedBlends,  set(map(getAbsPath, test[3])))
            
            expectedInisSkipped = test[4]
            for ini in expectedInisSkipped:
                fullPath = getAbsPath(ini)
                self.assertIn(fullPath, currentInisSkipped)
                self.assertEqual(type(currentInisSkipped[fullPath]), type(expectedInisSkipped[ini]))

    # ====================================================================
    # ====================== blendCorrection =============================

    def test_badBytes_badBlendData(self):
        blendBytes = bytes(8)
        result = None

        try:
            FRB.Mod.blendCorrection(blendBytes, FRB.ModTypes.Raiden.value)
        except Exception as e:
            result = e

        self.assertIsInstance(result, FRB.BadBlendData)

    @mock.patch("builtins.open", mock.mock_open(read_data = "test"))
    def test_badBlendFile_blendFileNotRecognized(self):
        result = None

        try:
            FRB.Mod.blendCorrection("someFile.ini", FRB.ModTypes.Raiden.value)
        except Exception as e:
            result = e

        self.assertIsInstance(result, FRB.BlendFileNotRecognized)

    def test_goodBlendData_noCorrectionDone(self):
        blendBytes = b'\xfa' * 64 # FAFAFAFA in hexadecimal is 4210752250 in decimal
        result = FRB.Mod.blendCorrection(blendBytes, FRB.ModTypes.Raiden.value)
        self.assertEqual(result, blendBytes)

    def test_blendNeedsCorrection_blendCorrected(self):
        badIndex = b'\x40\x00\x00\x00'  # 40 in hexadecimal is 64 in decimal, also it is in little endian
        goodIndex = b'\x3B\x00\x00\x00' #3B in hexadecimal is 59 in decimal, also it is in little endian 

        blendBytes = badIndex * 16
        result = FRB.Mod.blendCorrection(blendBytes, FRB.ModTypes.Raiden.value)
        self.assertEqual(result, badIndex * 4 + goodIndex * 4 + badIndex * 4 + goodIndex * 4)  

    # ====================================================================
    # ====================== correctBlend ================================

    def test_noInis_noBlendsCorrected(self):
        self.createMod()
        self._mod.inis = []
        resultFixedBlends, resultSkippedBlends = self._mod.correctBlend({"hello": FRB.RemapBlendModel(self.absPath, "hello", {})}, {"baddy": KeyError("some error")})

        self.compareSet(resultFixedBlends, set())
        self.compareDict(resultSkippedBlends, {})

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.Mod.blendCorrection")
    def test_differentBlends_correctUnVisitedBlends(self, m_blendCorrection):
        m_blendCorrection.side_effect = lambda blendFile, modType, fixedBlendFile: self.blendCorrection(blendFile, modType, fixedBlendFile)

        self._modTypes = None
        self._flattendDirItems = list(map(lambda path: self.osPathJoin(self.absPath, path[2:]), self._flattendDirItems))
        self.createMod()

        dummyRemapBlendModel = FRB.RemapBlendModel(self.absPath, "dummyName", {})
        dummyError = KeyError("Dummy Error")
        resultError = FRB.BlendFileNotRecognized("someFile")

        fixedRemapBlends = {"fixedRemap2.buf": dummyRemapBlendModel, "fixed3.buf": dummyRemapBlendModel, "fixed4.buf": dummyRemapBlendModel, "fixedRemapBlend4.buf": dummyRemapBlendModel, "sameFixed.buf": dummyRemapBlendModel,
                            "def/fixedRemap2.buf": dummyRemapBlendModel, "def/fixed3.buf": dummyRemapBlendModel, "def/fixed4.buf": dummyRemapBlendModel, "def/fixedRemapBlend4.buf": dummyRemapBlendModel, "def/sameFixed.buf": dummyRemapBlendModel}
        skippedBlends = {"bad_fixedRemap2.buf": dummyError, "bad_fixed3.buf": dummyError, "bad_fixed4.buf": dummyError, "bad_fixedRemapBlend4.buf": dummyError, "bad_sameFixed.buf": dummyError, 
                         "def/bad_fixedRemap2.buf": dummyError, "def/bad_fixed3.buf": dummyError, "def/bad_fixed4.buf": dummyError, "def/bad_fixedRemapBlend4.buf": dummyError, "def/bad_sameFixed.buf": dummyError}

        inisTest = [[[[FRB.IniFile(), FRB.ModTypes.Raiden.value, []],
                      [FRB.IniFile("NonExistent.ini"), FRB.ModTypes.Raiden.value, [FRB.RemapBlendModel(self.absPath, "someRemapResource", origBlendPaths = {1 : "notFixed1.buf", 2: "notFixed2.buf", 3: "fixed3.buf", 4: "fixed4.buf", 5: "sameFixed.buf"},
                                                                                                       origBlendName = "origBlendName", fixedBlendPaths = {1: "notFixedRemap1.buf", 2: "fixedRemap2.buf", 3: "notFixedRemap3.buf", 4: "fixedRemapBlend4.buf", 5: "sameFixed.buf"}),
                                                                                   FRB.RemapBlendModel(self.absPath, "someRemapResource", origBlendPaths = {1 : "bad_notFixed1.buf", 2: "bad_notFixed2.buf", 3: "bad_fixed3.buf", 4: "bad_fixed4.buf", 5: "bad_sameFixed.buf"},
                                                                                                       origBlendName = "origBlendName", fixedBlendPaths = {1: "bad_notFixedRemap1.buf", 2: "bad_fixedRemap2.buf", 3: "bad_notFixedRemap3.buf", 4: "bad_fixedRemapBlend4.buf", 5: "bad_sameFixed.buf"})]],

                      [FRB.IniFile("AConfigFile.ini"), self._defaultModType, [FRB.RemapBlendModel(self.absPath, "someRemapResource", origBlendPaths = {1 : "def/notFixed1.buf", 2: "def/notFixed2.buf", 3: "def/fixed3.buf", 4: "def/fixed4.buf", 5: "def/sameFixed.buf"},
                                                                                                  origBlendName = "origBlendName", fixedBlendPaths = {1: "def/notFixedRemap1.buf", 2: "def/fixedRemap2.buf", 3: "def/notFixedRemap3.buf", 4: "def/fixedRemapBlend4.buf", 5: "def/sameFixed.buf"}),
                                                                              FRB.RemapBlendModel(self.absPath, "someRemapResource", origBlendPaths = {1 : "def/bad_notFixed1.buf", 2: "def/bad_notFixed2.buf", 3: "def/bad_fixed3.buf", 4: "def/bad_fixed4.buf", 5: "def/bad_sameFixed.buf"},
                                                                                                  origBlendName = "origBlendName", fixedBlendPaths = {1: "def/bad_notFixedRemap1.buf", 2: "def/bad_fixedRemap2.buf", 3: "def/bad_notFixedRemap3.buf", 4: "def/bad_fixedRemapBlend4.buf", 5: "def/bad_sameFixed.buf"})]],

                      [FRB.IniFile("doppleganger.ini"), self._defaultModType, [FRB.RemapBlendModel(self.absPath, "someRemapResource", origBlendPaths = {1 : "notFixed1.buf", 2: "notFixed2.buf", 3: "fixed3.buf", 4: "fixed4.buf", 5: "sameFixed.buf"},
                                                                                                   origBlendName = "origBlendName", fixedBlendPaths = {1: "notFixedRemap1.buf", 2: "fixedRemap2.buf", 3: "notFixedRemap3.buf", 4: "fixedRemapBlend4.buf", 5: "sameFixed.buf"}),
                                                                               FRB.RemapBlendModel(self.absPath, "someRemapResource", origBlendPaths = {1 : "bad_notFixed1.buf", 2: "bad_notFixed2.buf", 3: "bad_fixed3.buf", 4: "bad_fixed4.buf", 5: "bad_sameFixed.buf"},
                                                                                                       origBlendName = "origBlendName", fixedBlendPaths = {1: "bad_notFixedRemap1.buf", 2: "bad_fixedRemap2.buf", 3: "bad_notFixedRemap3.buf", 4: "bad_fixedRemapBlend4.buf", 5: "bad_sameFixed.buf"})]],

                      [FRB.IniFile("Weirdo.ini"), FRB.ModTypes.Raiden.value, [FRB.RemapBlendModel(self.absPath, "WeirdRemapResource", origBlendPaths = {1: "nonExitent.buf"}, origBlendName = "origBlendName", fixedBlendPaths = {-1 : "nonExistentRemapBlend.buf"})]]], 
                        {"notFixedRemap1.buf", "def/notFixed1.buf", "def/bad_notFixed1.buf"}, 
                        {"bad_notFixedRemap1.buf": resultError, "nonExistentRemapBlend.buf": FRB.RemapMissingBlendFile("nonExistentRemapBlend.buf")}]]

        getAbsPath = lambda file: self.osPathJoin(self.absPath, file)
        for test in inisTest:
            inisData = test[0]
            for iniData in inisData:
                ini = iniData[0]
                ini._type = iniData[1]
                ini.remapBlendModels = iniData[2]

                if (ini.remapBlendModels):
                    ini._isModIni = True

            self._mod.inis = list(map(lambda iniData: iniData[0], inisData))

            testFixedRemapBlends = {}
            for blend in fixedRemapBlends:
                testFixedRemapBlends[getAbsPath(blend)] = fixedRemapBlends[blend] 

            testSkippedBlends = {}
            for blend in skippedBlends:
                testSkippedBlends[getAbsPath(blend)] = skippedBlends[blend]

            expectedCurrentFixedBlends = set(map(getAbsPath, test[1]))
            expectedFixedBlendsKeys = expectedCurrentFixedBlends.union(testFixedRemapBlends)
            expectedFixedBlends = {}
            for blend in expectedFixedBlendsKeys:
                expectedFixedBlends[blend] = dummyRemapBlendModel

            expectedCurrentSkippedBlends = {}
            for blend in test[2]:
                expectedCurrentSkippedBlends[getAbsPath(blend)] = test[2][blend]

            expectedSkippedBlends = FRB.DictTools.combine(expectedCurrentSkippedBlends, testSkippedBlends)

            currentFixedBlends, currentSkippedBlends = self._mod.correctBlend(testFixedRemapBlends, testSkippedBlends)
            self.compareSet(currentFixedBlends, expectedCurrentFixedBlends)

            self.assertEqual(len(currentSkippedBlends), len(expectedCurrentSkippedBlends))
            for blend in currentSkippedBlends:
                self.assertIn(blend, expectedCurrentSkippedBlends)
                self.assertEqual(type(currentSkippedBlends[blend]), type(expectedCurrentSkippedBlends[blend]))

            self.compareSet(set(testFixedRemapBlends.keys()),  set(expectedFixedBlends.keys()))
            self.assertEqual(len(testSkippedBlends), len(expectedSkippedBlends))
            for blend in testSkippedBlends:
                self.assertIn(blend, expectedSkippedBlends)
                self.assertEqual(type(testSkippedBlends[blend]), type(expectedSkippedBlends[blend]))


    # ====================================================================
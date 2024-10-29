import sys
import re
import unittest.mock as mock
from typing import List, Optional, Union

from .baseFileUnitTest import BaseFileUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class ModTest(BaseFileUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._customModTypes = {"rika": FRB.ModType("Bernkastel", re.compile(r"\[\s*LittleBlackNekoWitch\s*\]"), FRB.Hashes(), FRB.Indices(), aliases = ["Frederica Bernkastel", "Bern-chan", "Rika Furude", "Nipah!"], vgRemaps = FRB.VGRemaps()),
                               "kyrie": FRB.ModType("kyrie", re.compile(r"\[\s*AgnusDei\s*\]"), FRB.Hashes(), FRB.Indices(), vgRemaps = FRB.VGRemaps())}
        
        cls._setupCustomModTypes()
        
        cls._mod = None
        cls._modPath = ""
        cls._modFiles = None
        cls._modTypes = { FRB.ModTypes.Raiden.value, 
                          cls._customModTypes["rika"] }
        
        cls._defaultModType = cls._customModTypes["kyrie"]

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
                            "etudeTree": {"chopinTorrent.ini": None,
                                         "chopinSunshineRemapFix.ini": None,
                                         "chopinSwallow.buf": None,
                                         "chopinChromaticRemapBlend.buf": None,
                                         "bad_chopinWaterfall.buf": None,
                                         "chopinArpeggio.txt": None,
                                         "DISABLED_BossFixBackup_chopinBlackKeys.txt": None,
                                         "DISABLED_RemapBackup_chopinWinterWind.txt": None},
                            "seed.txt": None,
                            "leaf.ini": None}
        
        cls.setupFolderTree(cls._folderTree1)

    @classmethod
    def _setupCustomModTypes(cls):
        rikaModType = cls._customModTypes["rika"]
        kyrieModType = cls._customModTypes["kyrie"]

        rikaModType.hashes.addMap({"rika": {"rika"}}, {1.0: {"rika": {"blend_vb": "kuroneko", "draw_vb": "hanyu", "texcoord_vb": "rena's going to take you home"}},
                                                       2.0: {"rika": {"blend_vb": "nipah nipah!2", "draw_vb": "hanyu2"}},
                                                       3.0: {"rika": {"blend_vb": "nipah nipah!3", "texcoord_vb": "rena's going to take you home3"}}})

        kyrieModType.hashes.addMap({"kyrie": {"kyrie"}}, {2.0: {"kyrie": {"blend_vb": "Dies Irae"}},
                                                          2.3: {"kyrie": {"blend_vb": "gloria"}},
                                                          2.4: {"kyrie": {"blend_vb": "sanctus"}},
                                                          2.5: {"kyrie": {"blend_vb": "credo"}}})
        
        kyrieModType.indices.addMap({"kyrie": {"kyrie"}}, {3.0: {"kyrie": {"head": "eleison"}},
                                                           3.9: {"kyrie": {"head": "missa tota"}}})
        
        kyrieModType.vgRemaps.addMap({"kyrie": {"kyrie"}}, {1.0: {"kyrie": {"kyrie": {1: 10, 2: 9, 3: 8, 4: 7, 5: 6, 6: 5}}}})

    @classmethod
    def setupFolderTree(cls, newFolderTree):
        super().setupFolderTree(newFolderTree)
        cls.setupModFiles(list(filter(cls.isFile, cls._currentDirItems)))

    @classmethod
    def setupModFiles(cls, newFiles: List[str]):
        cls._modFiles = newFiles

    def setUp(self):
        super().setUp()        
        self._parseIniFiles = []
        self._parseIniInd = 0

        self.patch("src.FixRaidenBoss2.FileService.getPath", side_effect = lambda path: self._modPath)

    def createMod(self):
        self._mod = FRB.Mod(self._modPath, files = self._modFiles, types = self._modTypes, defaultType = self._defaultModType)

    def blendCorrection(self, blendFile: Union[str, bytes], modType: FRB.ModType, modToFix: str, fixedBlendFile: Optional[str] = None, version: Optional[float] = None) -> Optional[str]:
        if (not modType.getVGRemap(modToFix, version = version)):
            return None
        
        if (blendFile.find("bad") > -1):
            raise FRB.BlendFileNotRecognized(blendFile)
        return blendFile
    
    def parseIni(self):
        iniFile = self._parseIniFiles[self._parseIniInd]
        self._parseIniInd += 1

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
        fileTests = [[{}, [[], [], [], [] ]],
                     [self._folderTree1, [["leaf.ini"], [], [], []]],
                     [self._folderTree1["mainTree"]["raiden"], [["shogun.ini", "eiRemapBlend.ini"], [], [], []]],
                     [{"helloRemapBlend.buf": None,
                      "bang.ini": None,
                      "DISABLED_BossFixBackup_bye.txt": None}, [["bang.ini"], ["helloRemapBlend.buf"], ["DISABLED_BossFixBackup_bye.txt"], []]],
                      [{"helloRemapBlend.buf": None,
                      "bang.ini": None,
                      "DISABLED_BossFixBackup_bye.txt": None,
                      "helloRemapBlend2.buf": None,
                      "bang2.ini": None,
                      "DISABLED_BossFixBackup_bye2.txt": None}, [["bang.ini", "bang2.ini"], ["helloRemapBlend.buf", "helloRemapBlend2.buf"], ["DISABLED_BossFixBackup_bye.txt", "DISABLED_BossFixBackup_bye2.txt"], []]],
                     [self._folderTree1["etudeTree"], [["chopinTorrent.ini",], ["chopinChromaticRemapBlend.buf"], ["DISABLED_BossFixBackup_chopinBlackKeys.txt", "DISABLED_RemapBackup_chopinWinterWind.txt"], ["chopinSunshineRemapFix.ini"]]]]

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

    @mock.patch("src.FixRaidenBoss2.IniFile.parse")
    @mock.patch("src.FixRaidenBoss2.IniFile.removeFix")
    def test_differentInis_inisUndoedRemapsRemoved(self, m_removeFix, m_parse):
        self._modTypes = None
        self._flattendDirItems = list(map(lambda path: self.osPathJoin(self.absPath, path[2:]), self._flattendDirItems))
        self.createMod()
        modName = "Some Mod To Fix"

        fixedBlends = {f"imafixed.buf", f"helo/dfdfdf/../rtrtrtrt/fixedBlend.txt", "subTree1/big branch/blender.buf"}
        fixedInis = {"Bob the Fixer.ini", "Tractors/John Deer.json", "mainTree/agnes/unacceptable.ini", "NonExistent.ini"}
        visitedBlends = {"Neighbourhood/Neighbour/Visited.buf", "bellman-ford/ford fulkerson", "subTree2/trunk/twig/bad_boy.buf"}
        inisSkipped = {"Im Skipped.ini"}

        inisTest = [[[[FRB.IniFile(), {}],
                      [FRB.IniFile("NonExistent.ini"), {"Spruce": FRB.RemapBlendModel(self.absPath, {1 : {modName: "subTree2/trunk/twig/blendy.buf"}, 2: {modName: "subTree1/big branch/blender.buf"}})}],
                      [FRB.IniFile("Bad.ini"), {"Maple": FRB.RemapBlendModel(self.absPath, {3: {modName:  "subTree2/trunk/twig/bad_boy.buf"}}),
                                                "Evergreen": FRB.RemapBlendModel(self.absPath, {5: {modName: "subTree2/trunk/dead branch/bad_RemapBlend.buf"},
                                                                                                45: {modName: "mainTree/agnes/unacceptable.ini"}})}]], 
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

                if (ini.file not in fixedInis and ini.file not in inisSkipped):
                    self._parseIniFiles.append(ini.file)

            m_parse.side_effect = lambda: self.parseIni()

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

        #TODO: Add cases for fixing multiple mods from a single mod
        #TODO: Add case for reading all .ini files

    # ====================================================================
    # ====================== blendCorrection =============================

    def test_badBytes_badBlendData(self):
        blendBytes = bytes(8)
        result = None

        try:
            FRB.Mod.blendCorrection(blendBytes, FRB.ModTypes.Raiden.value, "RaidenBoss")
        except Exception as e:
            result = e

        self.assertIsInstance(result, FRB.BadBlendData)

    @mock.patch("builtins.open", mock.mock_open(read_data = "test"))
    def test_badBlendFile_blendFileNotRecognized(self):
        result = None

        try:
            FRB.Mod.blendCorrection("someFile.ini", FRB.ModTypes.Raiden.value, "RaidenBoss")
        except Exception as e:
            result = e

        self.assertIsInstance(result, FRB.BlendFileNotRecognized)

    def test_goodBlendData_noCorrectionDone(self):
        blendBytes = b'\xfa' * 64 # FAFAFAFA in hexadecimal is 4210752250 in decimal
        result = FRB.Mod.blendCorrection(blendBytes, FRB.ModTypes.Raiden.value, "RaidenBoss")
        self.assertEqual(result, blendBytes)

    def test_blendNeedsCorrection_blendCorrected(self):
        badIndex = b'\x40\x00\x00\x00'  # 40 in hexadecimal is 64 in decimal, also it is in little endian
        goodIndex = b'\x3B\x00\x00\x00' #3B in hexadecimal is 59 in decimal, also it is in little endian 

        blendBytes = badIndex * 16
        result = FRB.Mod.blendCorrection(blendBytes, FRB.ModTypes.Raiden.value, "RaidenBoss")
        self.assertEqual(result, badIndex * 4 + goodIndex * 4 + badIndex * 4 + goodIndex * 4)  

    # ====================================================================
    # ====================== correctBlend ================================

    def test_noInis_noBlendsCorrected(self):
        self.createMod()
        self._mod.inis = []
        resultFixedBlends, resultSkippedBlends = self._mod.correctBlend({"hello": FRB.RemapBlendModel(self.absPath, {})}, {"baddy": KeyError("some error")})

        self.compareSet(resultFixedBlends, set())
        self.compareDict(resultSkippedBlends, {})

    @mock.patch("src.FixRaidenBoss2.Mod.blendCorrection")
    def test_differentBlends_correctUnVisitedBlends(self, m_blendCorrection):
        m_blendCorrection.side_effect = lambda blendFile, modType, modToFix, fixedBlendFile, version: self.blendCorrection(blendFile, modType, modToFix, fixedBlendFile = fixedBlendFile, version = version)

        self._modTypes = None
        self._flattendDirItems = list(map(lambda path: self.osPathJoin(self.absPath, path[2:]), self._flattendDirItems))
        self.createMod()

        dummyRemapBlendModel = FRB.RemapBlendModel(self.absPath, {})
        dummyError = KeyError("Dummy Error")
        resultError = FRB.BlendFileNotRecognized("someFile")
        defaultModTypeToMapTo = "kyrie"
        raidenModTypeToMapTo = "RaidenBoss"

        fixedRemapBlends = {"fixedRemap2.buf": dummyRemapBlendModel, "fixed3.buf": dummyRemapBlendModel, "fixed4.buf": dummyRemapBlendModel, "fixedRemapBlend4.buf": dummyRemapBlendModel, "sameFixed.buf": dummyRemapBlendModel,
                            "def/fixedRemap2.buf": dummyRemapBlendModel, "def/fixed3.buf": dummyRemapBlendModel, "def/fixed4.buf": dummyRemapBlendModel, "def/fixedRemapBlend4.buf": dummyRemapBlendModel, "def/sameFixed.buf": dummyRemapBlendModel}
        skippedBlends = {"bad_fixedRemap2.buf": dummyError, "bad_fixed3.buf": dummyError, "bad_fixed4.buf": dummyError, "bad_fixedRemapBlend4.buf": dummyError, "bad_sameFixed.buf": dummyError, 
                         "def/bad_fixedRemap2.buf": dummyError, "def/bad_fixed3.buf": dummyError, "def/bad_fixed4.buf": dummyError, "def/bad_fixedRemapBlend4.buf": dummyError, "def/bad_sameFixed.buf": dummyError}

        inisTest = [[[[FRB.IniFile(), FRB.ModTypes.Raiden.value, {}],
                      [FRB.IniFile("NonExistent.ini"), FRB.ModTypes.Raiden.value, {"SectA": FRB.RemapBlendModel(self.absPath, origBlendPaths = {1 : "notFixed1.buf", 2: "notFixed2.buf", 3: "fixed3.buf", 4: "fixed4.buf", 5: "sameFixed.buf"},
                                                                                                       fixedBlendPaths = {1: {raidenModTypeToMapTo: "notFixedRemap1.buf"}, 2: {raidenModTypeToMapTo: "fixedRemap2.buf"}, 3: {raidenModTypeToMapTo: "notFixedRemap3.buf"}, 4: {raidenModTypeToMapTo: "fixedRemapBlend4.buf"}, 5: {raidenModTypeToMapTo: "sameFixed.buf"}}),
                                                                                   "SectB": FRB.RemapBlendModel(self.absPath, origBlendPaths = {1 : "bad_notFixed1.buf", 2: "bad_notFixed2.buf", 3: "bad_fixed3.buf", 4: "bad_fixed4.buf", 5: "bad_sameFixed.buf"},
                                                                                                       fixedBlendPaths = {1: {raidenModTypeToMapTo: "bad_notFixedRemap1.buf"}, 2: {raidenModTypeToMapTo: "bad_fixedRemap2.buf"}, 3: {raidenModTypeToMapTo: "bad_notFixedRemap3.buf"}, 4: {raidenModTypeToMapTo: "bad_fixedRemapBlend4.buf"}, 5: {raidenModTypeToMapTo: "bad_sameFixed.buf"}})}],

                      [FRB.IniFile("AConfigFile.ini"), self._defaultModType, {"SectA": FRB.RemapBlendModel(self.absPath, origBlendPaths = {1 : "def/notFixed1.buf", 2: "def/notFixed2.buf", 3: "def/fixed3.buf", 4: "def/fixed4.buf", 5: "def/sameFixed.buf"},
                                                                                                  fixedBlendPaths = {1: {defaultModTypeToMapTo: "def/notFixedRemap1.buf"}, 2: {defaultModTypeToMapTo: "def/fixedRemap2.buf"}, 3: {defaultModTypeToMapTo: "def/notFixedRemap3.buf"}, 4: {defaultModTypeToMapTo: "def/fixedRemapBlend4.buf"}, 5: {defaultModTypeToMapTo: "def/sameFixed.buf"}}),
                                                                              "SectB": FRB.RemapBlendModel(self.absPath, origBlendPaths = {1 : "def/bad_notFixed1.buf", 2: "def/bad_notFixed2.buf", 3: "def/bad_fixed3.buf", 4: "def/bad_fixed4.buf", 5: "def/bad_sameFixed.buf"},
                                                                                                  fixedBlendPaths = {1: {defaultModTypeToMapTo: "def/bad_notFixedRemap1.buf"}, 2: {defaultModTypeToMapTo: "def/bad_fixedRemap2.buf"}, 3: {defaultModTypeToMapTo: "def/bad_notFixedRemap3.buf"}, 4: {defaultModTypeToMapTo: "def/bad_fixedRemapBlend4.buf"}, 5: {defaultModTypeToMapTo: "def/bad_sameFixed.buf"}})}],

                      [FRB.IniFile("doppleganger.ini"), self._defaultModType, {"SectA": FRB.RemapBlendModel(self.absPath, origBlendPaths = {1 : "notFixed1.buf", 2: "notFixed2.buf", 3: "fixed3.buf", 4: "fixed4.buf", 5: "sameFixed.buf"},
                                                                                                   fixedBlendPaths = {1: {defaultModTypeToMapTo: "notFixedRemap1.buf"}, 2: {defaultModTypeToMapTo: "fixedRemap2.buf"}, 3: {defaultModTypeToMapTo: "notFixedRemap3.buf"}, 4: {defaultModTypeToMapTo: "fixedRemapBlend4.buf"}, 5: {defaultModTypeToMapTo: "sameFixed.buf"}}),
                                                                               "SectB": FRB.RemapBlendModel(self.absPath, origBlendPaths = {1 : "bad_notFixed1.buf", 2: "bad_notFixed2.buf", 3: "bad_fixed3.buf", 4: "bad_fixed4.buf", 5: "bad_sameFixed.buf"},
                                                                                                   fixedBlendPaths = {1: {defaultModTypeToMapTo: "bad_notFixedRemap1.buf"}, 2: {defaultModTypeToMapTo: "bad_fixedRemap2.buf"}, 3: {defaultModTypeToMapTo: "bad_notFixedRemap3.buf"}, 4: {defaultModTypeToMapTo: "bad_fixedRemapBlend4.buf"}, 5: {defaultModTypeToMapTo: "bad_sameFixed.buf"}})}],

                      [FRB.IniFile("Weirdo.ini"), FRB.ModTypes.Raiden.value, {"SectA": FRB.RemapBlendModel(self.absPath, origBlendPaths = {1: "nonExitent.buf"}, fixedBlendPaths = {-1 : {raidenModTypeToMapTo: "nonExistentRemapBlend.buf"}})}]], 
                        False, {"notFixedRemap1.buf", "def/notFixedRemap1.buf"},
                        {"bad_notFixedRemap1.buf": resultError, "nonExistentRemapBlend.buf": FRB.RemapMissingBlendFile("nonExistentRemapBlend.buf"), 
                         "def/bad_notFixedRemap1.buf": FRB.BlendFileNotRecognized("def/bad_notFixedRemap1.buf")}],
                        
                        
                    [[[FRB.IniFile(), FRB.ModTypes.Raiden.value, {}],
                      [FRB.IniFile("NonExistent.ini"), FRB.ModTypes.Raiden.value, {"SectA": FRB.RemapBlendModel(self.absPath, origBlendPaths = {1 : "notFixed1.buf", 2: "notFixed2.buf", 3: "fixed3.buf", 4: "fixed4.buf", 5: "sameFixed.buf"},
                                                                                                       fixedBlendPaths = {1: {raidenModTypeToMapTo: "notFixedRemap1.buf"}, 2: {raidenModTypeToMapTo: "fixedRemap2.buf"}, 3: {raidenModTypeToMapTo: "notFixedRemap3.buf"}, 4: {raidenModTypeToMapTo: "fixedRemapBlend4.buf"}, 5: {raidenModTypeToMapTo: "sameFixed.buf"}}),
                                                                                   "SectB": FRB.RemapBlendModel(self.absPath, origBlendPaths = {1 : "bad_notFixed1.buf", 2: "bad_notFixed2.buf", 3: "bad_fixed3.buf", 4: "bad_fixed4.buf", 5: "bad_sameFixed.buf"},
                                                                                                       fixedBlendPaths = {1: {raidenModTypeToMapTo: "bad_notFixedRemap1.buf"}, 2: {raidenModTypeToMapTo: "bad_fixedRemap2.buf"}, 3: {raidenModTypeToMapTo: "bad_notFixedRemap3.buf"}, 4: {raidenModTypeToMapTo: "bad_fixedRemapBlend4.buf"}, 5: {raidenModTypeToMapTo: "bad_sameFixed.buf"}})}],

                      [FRB.IniFile("AConfigFile.ini"), self._defaultModType, {"SectA": FRB.RemapBlendModel(self.absPath, origBlendPaths = {1 : "def/notFixed1.buf", 2: "def/notFixed2.buf", 3: "def/fixed3.buf", 4: "def/fixed4.buf", 5: "def/sameFixed.buf"},
                                                                                                  fixedBlendPaths = {1: {defaultModTypeToMapTo: "def/notFixedRemap1.buf"}, 2: {defaultModTypeToMapTo: "def/fixedRemap2.buf"}, 3: {defaultModTypeToMapTo: "def/notFixedRemap3.buf"}, 4: {defaultModTypeToMapTo: "def/fixedRemapBlend4.buf"}, 5: {defaultModTypeToMapTo: "def/sameFixed.buf"}}),
                                                                              "SectB": FRB.RemapBlendModel(self.absPath, origBlendPaths = {1 : "def/bad_notFixed1.buf", 2: "def/bad_notFixed2.buf", 3: "def/bad_fixed3.buf", 4: "def/bad_fixed4.buf", 5: "def/bad_sameFixed.buf"},
                                                                                                  fixedBlendPaths = {1: {defaultModTypeToMapTo: "def/bad_notFixedRemap1.buf"}, 2: {defaultModTypeToMapTo: "def/bad_fixedRemap2.buf"}, 3: {defaultModTypeToMapTo: "def/bad_notFixedRemap3.buf"}, 4: {defaultModTypeToMapTo: "def/bad_fixedRemapBlend4.buf"}, 5: {defaultModTypeToMapTo: "def/bad_sameFixed.buf"}})}],

                      [FRB.IniFile("doppleganger.ini"), self._defaultModType, {"SectA": FRB.RemapBlendModel(self.absPath, origBlendPaths = {1 : "notFixed1.buf", 2: "notFixed2.buf", 3: "fixed3.buf", 4: "fixed4.buf", 5: "sameFixed.buf"},
                                                                                                   fixedBlendPaths = {1: {defaultModTypeToMapTo: "notFixedRemap1.buf"}, 2: {defaultModTypeToMapTo: "fixedRemap2.buf"}, 3: {defaultModTypeToMapTo: "notFixedRemap3.buf"}, 4: {defaultModTypeToMapTo: "fixedRemapBlend4.buf"}, 5: {defaultModTypeToMapTo: "sameFixed.buf"}}),
                                                                               "SectB": FRB.RemapBlendModel(self.absPath, origBlendPaths = {1 : "bad_notFixed1.buf", 2: "bad_notFixed2.buf", 3: "bad_fixed3.buf", 4: "bad_fixed4.buf", 5: "bad_sameFixed.buf"},
                                                                                                   fixedBlendPaths = {1: {defaultModTypeToMapTo: "bad_notFixedRemap1.buf"}, 2: {defaultModTypeToMapTo: "bad_fixedRemap2.buf"}, 3: {defaultModTypeToMapTo: "bad_notFixedRemap3.buf"}, 4: {defaultModTypeToMapTo: "bad_fixedRemapBlend4.buf"}, 5: {defaultModTypeToMapTo: "bad_sameFixed.buf"}})}],

                      [FRB.IniFile("Weirdo.ini"), FRB.ModTypes.Raiden.value, {"SectA": FRB.RemapBlendModel(self.absPath, origBlendPaths = {1: "nonExitent.buf"}, fixedBlendPaths = {-1 : {raidenModTypeToMapTo: "nonExistentRemapBlend.buf"}})}]], 
                        True, set(),
                        {"nonExistentRemapBlend.buf": FRB.RemapMissingBlendFile("nonExistentRemapBlend.buf")}]]

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

            testFixedRemapBlends = set()
            for blend in fixedRemapBlends:
                testFixedRemapBlends.add(getAbsPath(blend))

            testSkippedBlends = {}
            for blend in skippedBlends:
                testSkippedBlends[getAbsPath(blend)] = skippedBlends[blend]

            expectedCurrentFixedBlends = set(map(getAbsPath, test[2]))
            expectedFixedBlends = expectedCurrentFixedBlends.union(testFixedRemapBlends)

            expectedCurrentSkippedBlends = {}
            for blend in test[3]:
                expectedCurrentSkippedBlends[getAbsPath(blend)] = test[3][blend]

            expectedSkippedBlends = FRB.DictTools.combine(expectedCurrentSkippedBlends, testSkippedBlends)

            currentFixedBlends, currentSkippedBlends = self._mod.correctBlend(testFixedRemapBlends, testSkippedBlends, fixOnly = test[1])
            self.compareSet(currentFixedBlends, expectedCurrentFixedBlends)

            self.assertEqual(len(currentSkippedBlends), len(expectedCurrentSkippedBlends))
            for blend in currentSkippedBlends:
                self.assertIn(blend, expectedCurrentSkippedBlends)
                self.assertEqual(type(currentSkippedBlends[blend]), type(expectedCurrentSkippedBlends[blend]))

            self.compareSet(testFixedRemapBlends,  expectedFixedBlends)
            self.assertEqual(len(testSkippedBlends), len(expectedSkippedBlends))
            for blend in testSkippedBlends:
                self.assertIn(blend, expectedSkippedBlends)
                self.assertEqual(type(testSkippedBlends[blend]), type(expectedSkippedBlends[blend]))


        #TODO: Add cases for fixing multiple mods from a single mod

    # ====================================================================
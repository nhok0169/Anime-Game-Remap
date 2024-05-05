import sys
from .baseFileUnitTest import BaseFileUnitTest
import unittest.mock as mock
import ntpath

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )')
import src.FixRaidenBoss2.FixRaidenBoss2 as FRB


class BossFixServiceTest(BaseFileUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._path = "path/to/some/mod"
        cls._keepBackups = False
        cls._fixOnly = False
        cls._undoOnly = False
        cls._readAllInis = False
        cls._types = None
        cls._defaultType = None
        cls._log = None
        cls._verbose = False
        cls._handleExceptions = True
        cls._bossFixService = None

        cls._blendsCorrected = set()
        cls._blendsSkipped = {}

        cls._undoedInis = set()
        cls._removedRemapBlends = set()

        cls._printMsgs = []

    def correctBlend(self):
        return [self._blendsCorrected, self._blendsSkipped]
    
    def log(self, message: str):
        self._printMsgs.append(message)

    def fixIni(self, ini: FRB.IniFile):
        if (ini.file.find("bad") > -1):
            raise FloatingPointError(f"Bad ini file: {ini.file}")
        return True
    
    def removeFix(self):
        return [self._undoedInis, self._removedRemapBlends]

    def setupLog(self):
        self.patch("src.FixRaidenBoss2.FixRaidenBoss2.Logger.log", side_effect = lambda message: self.log(message))

    def getMockLog(self) -> mock.MagicMock:
        return self.patches["src.FixRaidenBoss2.FixRaidenBoss2.Logger.log"]
    
    def setupFixIni(self):
        self.patch("src.FixRaidenBoss2.FixRaidenBoss2.BossFixService.fixIni", side_effect = lambda ini, mod, fixedRemapBlend: self.fixIni(ini))

    def getMockFixIni(self) -> mock.MagicMock:
        return self.patches["src.FixRaidenBoss2.FixRaidenBoss2.BossFixService.fixIni"]
    
    def setupRemoveFix(self):
        self.patch("src.FixRaidenBoss2.FixRaidenBoss2.Mod.removeFix", side_effect = lambda fixedBlends, fixedInis, visitedRemapBlends, inisSkipped, keepBackups, fixOnly: self.removeFix())

    def getMockRemoveFix(self) -> mock.MagicMock:
        return self.patches["src.FixRaidenBoss2.FixRaidenBoss2.Mod.removeFix"]

    def setupBossFixService(self):
        self._bossFixService = FRB.BossFixService(path = self._path, keepBackups = self._keepBackups, fixOnly = self._fixOnly, undoOnly = self._undoOnly,
                                                  readAllInis = self._readAllInis, types = self._types, defaultType = self._defaultType, log = self._log,
                                                  verbose = self._verbose, handleExceptions = self._handleExceptions)
        
    def setUp(self):
        super().setUp()
        self.patch("src.FixRaidenBoss2.FixRaidenBoss2.Mod.correctBlend", side_effect = lambda fixedRemapBlends = [], skippedBlends = {}, fixOnly = False: self.correctBlend())

    # ====================== path.setter =================================

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.BossFixService.clear")
    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.BossFixService._setupModPath")
    def test_givenPath_pathSet(self, m_setupModPath, m_clear):
        self.setupBossFixService()

        newPath = "some path/dfdfd"
        self._bossFixService.path = newPath
        m_clear.assert_called_once()
        self.assertEqual(m_setupModPath.call_count, 2)
        self.assertEqual(self._bossFixService.path, newPath)

    # ====================================================================
    # ====================== log.setter ==================================

    def test_noLogFolder_noLogPath(self):
        self.setupBossFixService()

        self._bossFixService.log = None
        self.assertIs(self._bossFixService.log, None)

    def test_logFolder_setLogPath(self):
        self.setupBossFixService()

        tests = {"logging": f"logging/{FRB.LogFile}",
                 "../../": f"../../{FRB.LogFile}"}

        for logPath in tests:
            self._bossFixService.log = logPath
            self.assertEqual(self._bossFixService.log, tests[logPath])

    # ====================================================================
    # ====================== clear =======================================

    def test_hasStats_allStatsCleared(self):
        self._log = "logging"
        self.setupBossFixService()

        self._bossFixService.modsFixed = 60
        self._bossFixService.skippedMods =  {"Out, damned spot!": KeyError("out, I say!")}
        self._bossFixService.blendsFixed = {"Fair is foul and foul is fair"}
        self._bossFixService.skippedBlendsByMods =  {"I am in blood": {"Stepped in so far that, should I wade no more,": KeyError("Returning were as tedious as go o'er.")}}
        self._bossFixService.skippedBlends = {"Das Wasser ist Blut... Blut...": KeyError("Blut... Blut...")}
        self._bossFixService.inisFixed = {"Das Messer? Wo ist das Messer"}
        self._bossFixService.inisSkipped = {"Hopp, hopp! Hopp, hopp!": KeyError("Hopp, hopp!")}
        self._bossFixService.removedRemapBlends = {"Macbeth, William Shakespeare"}
        self._bossFixService._visitedRemapBlendsAtRemoval = {"Wozzeck, Alban Berg"}
        self._bossFixService.undoedInis = {"Beware the Jabberwock, my son!", "The jaws that bite, the claws that catch!"}
        self._bossFixService.logger.log("Jabberwocky, Lewis Caroll")

        self._bossFixService.clear(clearLog = False)

        self.assertEqual(self._bossFixService.modsFixed, 0)
        self.compareDict(self._bossFixService.skippedMods, {})
        self.compareSet(self._bossFixService.blendsFixed, set())
        self.compareDict(self._bossFixService.skippedBlendsByMods, {})
        self.compareDict(self._bossFixService.skippedBlends, {})
        self.compareSet(self._bossFixService.inisFixed, set())
        self.compareDict(self._bossFixService.inisSkipped, {})
        self.compareSet(self._bossFixService.removedRemapBlends, set())
        self.compareSet(self._bossFixService._visitedRemapBlendsAtRemoval, set())
        self.assertGreater(len(self._bossFixService.logger.loggedTxt), 0)

        self._bossFixService.clear(clearLog = True)
        self.assertEqual(self._bossFixService.logger.loggedTxt, "")

    # ====================================================================
    # ====================== _setupModPath ===============================

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.DefaultPath")
    def test_noPath_setDefaultPath(self, m_defaultPath):
        m_defaultPath.return_value = self.absPath
        self.setupBossFixService()

        self._bossFixService._path = None
        self._bossFixService._setupModPath()
        self.assertEqual(self._bossFixService.pathIsCwd, True)
        self.assertEqual(self._bossFixService.path.return_value, m_defaultPath.return_value)

    def test_hasPathNotCWD_pathSetNotCWD(self):
        self.setupBossFixService()

        newPath = "../The twisted path"
        self._bossFixService._path = newPath
        self._bossFixService._setupModPath()
        self.assertEqual(self._bossFixService.pathIsCwd, False)

        expectedPath = ntpath.normpath(self.osPathJoin(self.absPath, newPath)).replace(self.NtPathSep, self.OsSep)
        self.assertEqual(self._bossFixService.path, expectedPath)

    def test_pathIsCWD_pathSetAndIsCWD(self):
        self.setupBossFixService()
        newPath = "."
        self._bossFixService._path = newPath

        with mock.patch.object(FRB, "DefaultPath", self.absPath):
            self._bossFixService._path = newPath
            self._bossFixService._setupModPath()
            self.assertEqual(self._bossFixService.path, self.absPath)
            self.assertEqual(self._bossFixService.pathIsCwd, True)

    # ====================== _setupLogPath ===============================

    def test_noLogFolder_noLogging(self):
        self.setupBossFixService()

        self._bossFixService.log = None
        self._bossFixService._setupLogPath()
        self.assertIs(self._bossFixService.log, None)

    def test_logFolder_hasLogFilePathSet(self):
        self.setupBossFixService()

        tests = {"logging": f"logging/{FRB.LogFile}",
                 "../../": f"../../{FRB.LogFile}"}

        for logPath in tests:
            self._bossFixService._log = logPath
            self._bossFixService._setupLogPath()
            self.assertEqual(self._bossFixService.log, tests[logPath])

    # ====================================================================
    # ====================== _setupModTypes ==============================

    def test_noModTypes_allModTypesAvailableAdded(self):
        self.setupBossFixService()
        self._bossFixService.types = None

        self._bossFixService._setupModTypes()
        self.compareSet(self._bossFixService.types, {FRB.ModTypes.Raiden.value})

    def test_allInisRead_allModTypesAvailableAdded(self):
        self._readAllInis = True
        self.setupBossFixService()
        self._bossFixService.types = 123

        self._bossFixService._setupModTypes()
        self.compareSet(self._bossFixService.types, {FRB.ModTypes.Raiden.value})

    def test_validModTypeSearchWords_allModTypesAvailableAdded(self):
        self.setupBossFixService()
        self._bossFixService.types = "raiden,ei,shogun"

        self._bossFixService._setupModTypes()
        self.compareSet(self._bossFixService.types, {FRB.ModTypes.Raiden.value})

    def test_invalidModTypesSearchWordAllInisParsed_allModTypesAvailableAdded(self):
        self._readAllInis = True
        self.setupBossFixService()
        self._bossFixService.types = "boo"

        self._bossFixService._setupModTypes()
        self.compareSet(self._bossFixService.types, {FRB.ModTypes.Raiden.value})

    def test_hasInvalidModTypeSearchWordNotAllInisParsed_modTypeSearchKeyWordsNotParsed(self):
        self.setupBossFixService()

        self._bossFixService.types = "ayaya,raiden,ei"
        self._bossFixService._setupModTypes()
        self.assertEqual(self._bossFixService.types, "ayaya,raiden,ei")

        self._bossFixService.types = "raiden,ayaya,ei"
        self._bossFixService._setupModTypes()
        self.assertEqual(self._bossFixService.types, {FRB.ModTypes.Raiden.value})

    # ====================================================================
    # ====================== _setupDefaultModType ========================

    def test_notAllInisRead_noDefaultType(self):
        self.setupBossFixService()
        self._bossFixService.defaultType = 123124

        self._bossFixService._setupDefaultModType()
        self.assertIs(self._bossFixService.defaultType, None)

        self._bossFixService.defaultType = FRB.ModTypes.Raiden.value
        self._bossFixService._setupDefaultModType()
        self.assertIs(self._bossFixService.defaultType, None)

    def test_validModTypeSearchWordReadAllInis_defaultTypeSet(self):
        self._readAllInis = True
        self.setupBossFixService()
        self._bossFixService.defaultType = "Raiden"

        self._bossFixService._setupDefaultModType()
        self.assertEqual(self._bossFixService.defaultType, FRB.ModTypes.Raiden.value)

    def test_invalidModTypeSearchWordReadAllInis_noDefaultModType(self):
        self._readAllInis = True
        self.setupBossFixService()
        self._bossFixService.defaultType = "Ayaka"

        self._bossFixService._setupDefaultModType()
        self.assertEqual(self._bossFixService.defaultType, None)

    # ====================================================================
    # ====================== fixIni ======================================

    def test_iniNotNod_noFix(self):
        self.setupBossFixService()
        ini = FRB.IniFile()
        ini.parse()
        mod = FRB.Mod(files = ["hello.txt"])
        fixedRemapBlends = {"someBlend.buf": FRB.RemapBlendModel(self.absPath, "someBLendName", {1: "somepath.buf"})}

        result = self._bossFixService.fixIni(ini, mod, fixedRemapBlends)
        self.assertEqual(result, False)

    def test_iniUndoOnly_noFix(self):
        self._undoOnly = True
        self.setupBossFixService()
        ini = FRB.IniFile(txt="[TextureOverrideRaidenBossBlend]\nhash=abc")
        ini.parse()
        mod = FRB.Mod(files = ["hello.txt"])
        fixedRemapBlends = {"someBlend.buf": FRB.RemapBlendModel(self.absPath, "someBLendName", {1: "somepath.buf"})}

        result = self._bossFixService.fixIni(ini, mod, fixedRemapBlends)
        self.assertEqual(result, True)

    def test_iniAlreadyFixed_noFix(self):
        self.setupLog()
        self.setupBossFixService()
        ini = FRB.IniFile("hello.ini")
        ini.fileTxt = "[TextureOverrideRaidenBossRemapBlend]\nhash=abc"
        ini.parse()

        mod = FRB.Mod(files = ["hello.txt"])
        fixedRemapBlends = {"someBlend.buf": FRB.RemapBlendModel(self.absPath, "someBLendName", {1: "somepath.buf"})}

        result = self._bossFixService.fixIni(ini, mod, fixedRemapBlends)
        self.assertEqual(result, True)
        self.assertGreaterEqual(self.getMockLog().call_count, 2)

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.IniFile.fix")
    def test_differentInisToFix_inisFixed(self, m_fix):
        self.setupLog()
        self.setupBossFixService()
        ini = FRB.IniFile("hello.ini")
        ini.fileTxt = "[TextureOverrideRaidenBossBlend]\nhash=abc"
        ini.parse()

        mod = FRB.Mod(path = self.absPath, files = ["hello.txt"])
        fixedRemapBlends = {"someBlend.buf": FRB.RemapBlendModel(self.absPath, "someBLendName", {1: "somepath.buf"})}

        tests = [[{self.absPath: {"skippedBlend1.buf": KeyError("skippedBlend1 Error"), "skippedBlend2.buf": KeyError("skippedBlend2 Error")}}, {"fixedBlend3.buf"},
                  {"fixedBlend3.buf", "fixedBlend4.buf"}, {"skippedBlend1.buf": FloatingPointError("new skippedBlend1 Error"), "skippedBlend3.buf": FloatingPointError("skippedBlend3 Error")},
                  {"fixedBlend3.buf", "fixedBlend4.buf"}, {self.absPath: {"skippedBlend1.buf": FloatingPointError("new skippedBlend1 Error"), "skippedBlend2.buf": KeyError("skippedBlend2 Error"),  "skippedBlend3.buf": FloatingPointError("skippedBlend3 Error")}}]]
        
        for test in tests:
            self._bossFixService.clear()
            self._bossFixService.skippedBlendsByMods = test[0]
            self._bossFixService.blendsFixed = test[1]
            self._blendsCorrected = test[2]
            self._blendsSkipped = test[3]

            result = self._bossFixService.fixIni(ini, mod, fixedRemapBlends)
            self.assertEqual(result, True)
            self.compareSet(self._bossFixService.blendsFixed, test[4])
            
            expectedSkippedBlendsByMods = test[5]
            self.assertEqual(len(expectedSkippedBlendsByMods), len(self._bossFixService.skippedBlendsByMods))
            for modPath in expectedSkippedBlendsByMods:
                self.assertIn(modPath, self._bossFixService.skippedBlendsByMods)

                expectedModBlendErrors = expectedSkippedBlendsByMods[modPath]
                resultModBlendErrors = self._bossFixService.skippedBlendsByMods[modPath]

                self.assertEqual(len(expectedModBlendErrors), len(resultModBlendErrors))
                for blendPath in expectedModBlendErrors:
                    self.assertIn(blendPath, resultModBlendErrors)
                    self.assertEqual(type(expectedModBlendErrors[blendPath]), type(resultModBlendErrors[blendPath]))

    # ====================================================================
    # ====================== fixMod ======================================

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.Mod.removeBackupInis")
    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.IniFile.checkIsMod", side_effect = lambda: True)
    def test_differentMods_modsFixed(self, m_checkIsMod, m_removeBackupInis):
        self.setupLog()
        self.setupFixIni()
        self.setupRemoveFix()
        self.setupBossFixService()
        getAbsPath = lambda file: self.osPathJoin(self.absPath, file)
        fixedRemapBlends = {"someBlend.buf": FRB.RemapBlendModel(self.absPath, "someBLendName", {1: "somepath.buf"})}

        tests = [[FRB.Mod(path = self.absPath, files = []), [], {"fixedIni1", "fixedIni2"}, {getAbsPath("skippedIni1"): KeyError("Ini1 skipped"), getAbsPath("skippedIni2"): KeyError("Ini2 skipped")}, 
                  {"skippedMod1": KeyError("mod1 skipped")}, {"removedRemap1"}, {"removedRemap2", "removedRemap3"},
                  False, {"fixedIni1", "fixedIni2"}, {getAbsPath("skippedIni1"): KeyError("Ini1 skipped"), getAbsPath("skippedIni2"): KeyError("Ini2 skipped")}, {"skippedMod1": KeyError("mod1 skipped")},
                  {"removedRemap1", "removedRemap2", "removedRemap3"}],

                 [FRB.Mod(path = self.absPath, files = []), [FRB.IniFile("modIni1")], {"fixedIni1", "fixedIni2"}, {getAbsPath("skippedIni1"): KeyError("Ini1 skipped"), getAbsPath("skippedIni2"): KeyError("Ini2 skipped")}, 
                  {"skippedMod1": KeyError("mod1 skipped")}, {"removedRemap1"}, {"removedRemap2", "removedRemap3"},
                  True, {"fixedIni1", "fixedIni2", f"{self.absPath}/modIni1"}, {getAbsPath("skippedIni1"): KeyError("Ini1 skipped"), getAbsPath("skippedIni2"): KeyError("Ini2 skipped")}, {"skippedMod1": KeyError("mod1 skipped")},
                  {"removedRemap1", "removedRemap2", "removedRemap3"}],
                  
                 [FRB.Mod(path = self.absPath, files = []), [FRB.IniFile("modIni1"), FRB.IniFile("bad_modIni2"), FRB.IniFile("modIni3")], set(), {}, {}, set(), set(),
                  True, {f"{self.absPath}/modIni1", f"{self.absPath}/modIni3"}, {getAbsPath("bad_modIni2"): FloatingPointError(f"Bad ini file: bad_modIni2")},
                  {}, set()],

                 [FRB.Mod(path = self.absPath, files = []), [FRB.IniFile("bad_modIni1"), FRB.IniFile("bad_modIni2"), FRB.IniFile("bad_modIni3")], set(), {}, {}, set(), set(),
                  False, set(), {getAbsPath("bad_modIni1"): FloatingPointError(f"Bad ini file: bad_modIni1"), 
                              getAbsPath("bad_modIni2"): FloatingPointError(f"Bad ini file: bad_modIni2"), 
                              getAbsPath("bad_modIni3"): FloatingPointError(f"Bad ini file: bad_modIni3")}, {self.absPath: FloatingPointError(f"Bad ini file: bad_modIni1")}, set()]]


        checkIsModTotalCallCount = 0
        for test in tests:
            self._bossFixService.clear()
            self._bossFixService.inisFixed = test[2]
            self._bossFixService.inisSkipped = test[3]
            self._bossFixService.skippedMods = test[4]
            self._bossFixService.removedRemapBlends = test[5]
            self._removedRemapBlends = test[6]

            mod = test[0]
            mod.inis = test[1]

            result = self._bossFixService.fixMod(mod, fixedRemapBlends)
            self.assertEqual(result, test[7])
            self.compareSet(self._bossFixService.inisFixed, test[8])
            self.compareSet(self._bossFixService.removedRemapBlends, test[11])

            checkIsModTotalCallCount += len(mod.inis)
            self.assertEqual(m_checkIsMod.call_count, checkIsModTotalCallCount)

            expectedSkippedInis = test[9]
            resultSkippedInis = self._bossFixService.inisSkipped
            self.assertEqual(len(resultSkippedInis), len(expectedSkippedInis))
            for ini in expectedSkippedInis:
                self.assertIn(ini, resultSkippedInis)
                self.assertEqual(type(resultSkippedInis[ini]), type(expectedSkippedInis[ini]))

            expectedSkippedMods = test[10]
            resultSkippedMods = self._bossFixService.skippedMods
            self.assertEqual(len(resultSkippedMods), len(expectedSkippedMods))
            for skippedMod in expectedSkippedMods:
                self.assertIn(skippedMod, resultSkippedMods)
                self.assertEqual(type(resultSkippedMods[skippedMod]), type(expectedSkippedMods[skippedMod]))

    # ====================================================================
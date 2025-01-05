import sys
import re
import unittest.mock as mock
from collections import OrderedDict

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class IniFileTest(BaseIniFileTest):

    # ====================== fileTxt.setter ==============================
        
    def test_differentFileTxt_fileTxtAndFileLinesSetAndFileRead(self):
        self.createIniFile()

        iniFileTxtLst = [self._defaultIniTxt, "", "fa la la la la la la la la", "\n\n\n", "\n"]
        for txt in iniFileTxtLst:
            self.setupIniTxt(txt)
            self._iniFile.fileTxt = self._iniTxt
            self.assertEqual(self._iniFile.fileTxt, self._iniTxt)
            self.compareList(self._iniFile.fileLines, self._iniTxtLines)
            self.assertEqual(self._iniFile.fileLinesRead, True)

    # ====================================================================
    # ====================== fileLines.setter ============================
            
    def test_differentFileLines_fileTxtAndFileLinesSetAndFileRead(self):
        self.createIniFile()

        iniFileTxtLst = [self._defaultIniTxt, "", "fa la la la la la la la la", "\n\n\n", "\n"]
        for txt in iniFileTxtLst:
            self.setupIniTxt(txt)
            self._iniFile.fileLines = self._iniTxtLines

            self.assertEqual(self._iniFile.fileTxt, self._iniTxt)
            self.compareList(self._iniFile.fileLines, self._iniTxtLines)
            self.assertEqual(self._iniFile.fileLinesRead, True)

    # ====================================================================
    # ====================== clearRead ===================================
            
    def test_readFile_noSavedFileContent(self):
        self.createIniFile()

        iniFileTxtLst = [self._defaultIniTxt, "", "fa la la la la la la la la", "\n\n\n", "\n"]
        for txt in iniFileTxtLst:
            self.setupIniTxt(txt)
            self._iniFile.fileLines = self._iniTxtLines
            self._iniFile._textureOverrideBlendRoot = "SOME ROOT"

            self._iniFile.clearRead()
            self.assertEqual(self._iniFile.fileTxt, "")
            self.compareList(self._iniFile.fileLines, [])
            self.assertEqual(self._iniFile.isFixed, False)
            self.assertEqual(self._iniFile.fileLinesRead, False)
            self.assertIs(self._iniFile._textureOverrideBlendRoot, None)

    def test_noFile_savedTextStillRemains(self):
        self._file = None
        self.createIniFile()
        iniRoot = "SOME ROOT"

        iniFileTxtLst = [self._defaultIniTxt, "", "fa la la la la la la la la", "\n\n\n", "\n"]
        for txt in iniFileTxtLst:
            self.setupIniTxt(txt)
            self._iniFile.fileLines = self._iniTxtLines
            self._iniFile._textureOverrideBlendRoot = iniRoot

            self._iniFile.clearRead()
            self.assertEqual(self._iniFile.fileTxt, self._iniTxt)
            self.compareList(self._iniFile.fileLines, self._iniTxtLines)
            self.assertEqual(self._iniFile.isFixed, False)
            self.assertEqual(self._iniFile.fileLinesRead, True)
            self.assertEqual(self._iniFile._textureOverrideBlendRoot, iniRoot)

    def test_noFileEraseSavedText_noSavedText(self):
        self._file = None
        self.createIniFile()
        iniRoot = "SOME ROOT"

        iniFileTxtLst = [self._defaultIniTxt, "", "fa la la la la la la la la", "\n\n\n", "\n"]
        for txt in iniFileTxtLst:
            self.setupIniTxt(txt)
            self._iniFile.fileLines = self._iniTxtLines
            self._iniFile._textureOverrideBlendRoot = iniRoot

            self._iniFile.clearRead(eraseSourceTxt = True)
            self.assertEqual(self._iniFile.fileTxt, "")
            self.compareList(self._iniFile.fileLines, [])
            self.assertEqual(self._iniFile.isFixed, False)
            self.assertEqual(self._iniFile.fileLinesRead, False)
            self.assertIs(self._iniFile._textureOverrideBlendRoot, None)
            self.assertIs(self._iniFile._textureOverrideBlendSectionName, None)

    # ====================================================================
    # ====================== clear =======================================
            
    def test_readFile_noSavedData(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()
        self._iniFile.parse()
        self._iniFile.fix()
        self._iniFile.clear()

        self.compareList(self._iniFile.fileLines, [])
        self.assertEqual(self._iniFile.fileTxt, "")
        self.assertEqual(self._iniFile.fileLinesRead, False)

        self.assertEqual(self._iniFile.isFixed, False)
        self.assertIs(self._iniFile._textureOverrideBlendRoot, None)
        self.assertIs(self._iniFile._textureOverrideBlendSectionName, None)

        self.assertIs(self._iniFile.type, None)
        self.assertEqual(self._iniFile.isModIni, False)

        self.compareDict(self._iniFile.sectionIfTemplates, {})
        self.compareDict(self._iniFile._resourceBlends, {})
        self.compareSet(self._iniFile._remappedSectionNames, set())

        self.assertIs(self._iniFile._iniParser, None)
        self.assertIs(self._iniFile._iniFixer, None)

        self.compareDict(self._iniFile.remapBlendModels, {})

    def test_noFile_noSavedDataExceptForText(self):
        self._file = None
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()
        self._iniFile.parse()
        self._iniFile.fix()
        self._iniFile.clear()

        self.assertGreater(len(self._iniFile.fileLines), 0)
        self.assertGreater(len(self._iniFile.fileTxt), 0)
        self.assertEqual(self._iniFile.fileLinesRead, True)

        self.assertEqual(self._iniFile.isFixed, True)
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, "TextureOverrideRaidenShogunBlend")
        self.assertEqual(self._iniFile._textureOverrideBlendSectionName, "TextureOverrideRaidenShogunBlend")

        self.assertIs(self._iniFile.type, None)
        self.assertEqual(self._iniFile.isModIni, False)

        self.compareDict(self._iniFile.sectionIfTemplates, {})
        self.compareDict(self._iniFile._resourceBlends, {})

        self.assertIs(self._iniFile._iniParser, None)
        self.assertIs(self._iniFile._iniFixer, None)

        self.compareDict(self._iniFile.remapBlendModels, {})

    def test_noFileErasedSavedText_noSavedData(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()
        self._iniFile.parse()
        self._iniFile.fix()
        self._iniFile.clear(eraseSourceTxt = True)

        self.compareList(self._iniFile.fileLines, [])
        self.assertEqual(self._iniFile.fileTxt, "")
        self.assertEqual(self._iniFile.fileLinesRead, False)

        self.assertEqual(self._iniFile.isFixed, False)
        self.assertIs(self._iniFile._textureOverrideBlendRoot, None)
        self.assertIs(self._iniFile._textureOverrideBlendSectionName, None)

        self.assertIs(self._iniFile.type, None)
        self.assertEqual(self._iniFile.isModIni, False)

        self.compareDict(self._iniFile.sectionIfTemplates, {})
        self.compareDict(self._iniFile._resourceBlends, {})

        self.assertIs(self._iniFile._iniParser, None)
        self.assertIs(self._iniFile._iniFixer, None)

        self.compareDict(self._iniFile.remapBlendModels, {})

            
    # ====================================================================
    # ====================== availableType ===============================

    # TODO: Add tests for getting the available type


    # ====================================================================
    # ====================== read ========================================

    def test_noFile_savedTextOnInit(self):
        self.patches["src.FixRaidenBoss2.FileService.read"].side_effect = lambda file, fileCode, postProcessor: self._iniTxt
        self.setupIniTxt(self._defaultIniTxt)
        self._file = None
        self.createIniFile()

        self.assertEqual(self._iniFile.fileTxt, self._iniTxt)

        iniFileTxtLst = ["", "fa la la la la la la la la", "\n\n\n", "\n", self._defaultIniTxt]
        for txt in iniFileTxtLst:
            self.setupIniTxt(txt)
            self._iniFile.fileTxt = txt

            self.assertEqual(self._iniFile.fileTxt, self._iniTxt)
            result = self._iniFile.read()
            self.assertEqual(result, self._iniTxt)
            self.compareList(self._iniFile.fileLines, self._iniTxtLines)

    def test_hasFile_textReadFromFile(self):
        self.patches["src.FixRaidenBoss2.FileService.read"].side_effect = lambda file, fileCode, postProcessor: self._iniTxt
        self.createIniFile()
        numOfTimesFileRead = 5

        for i in range(numOfTimesFileRead):
            result = self._iniFile.read()
            self.assertEqual(result, self._iniTxt)
            self.compareList(self._iniFile.fileLines, self._iniTxtLines)

        self.setupIniTxt("BOOO")
        result = self._iniFile.read()
        self.assertEqual(result, self._iniTxt)
        self.compareList(self._iniFile.fileLines, self._iniTxtLines)
            
    # ====================================================================
    # ====================== write =======================================
        
    def test_noFile_noWriteReturnSavedTextOnInit(self):
        self.setupIniTxt(self._defaultIniTxt)
        self._file = None
        self.createIniFile()

        self.assertEqual(self._iniFile.fileTxt, self._iniTxt)

        iniFileTxtLst = ["", "fa la la la la la la la la", "\n\n\n", "\n", self._defaultIniTxt]
        for txt in iniFileTxtLst:
            self.setupIniTxt(txt)
            self._iniFile.fileTxt = txt

            self.assertEqual(self._iniFile.fileTxt, self._iniTxt)
            result = self._iniFile.write()
            self.assertEqual(result, self._iniTxt)

    def test_hasFile_textWrittenReturnsWrittenText(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()

        self.assertEqual(self._iniFile.fileTxt, "")

        iniFileTxtLst = ["", "fa la la la la la la la la", "\n\n\n", "\n", self._defaultIniTxt]
        for txt in iniFileTxtLst:
            self.setupIniTxt(txt)
            self._iniFile.fileTxt = txt

            self.assertEqual(self._iniFile.fileTxt, self._iniTxt)
            result = self._iniFile.write()
            
            openPatch = self.getOpenPatch()
            openPatch.assert_called_with(self._file, 'w', encoding = FRB.FileEncodings.UTF8.value)
            openPatch.return_value.__enter__().write.assert_called_with(self._iniTxt)
            self.assertEqual(result, self._iniTxt)

    # TODO: Add tests for writing random text

    # ====================================================================
    # ====================== _setupFileLines =============================
            
    def test_noFile_fileTxtSet(self):
        self._file = None
        self.createIniFile()

        self._iniFile._setupFileLines(fileTxt = self._defaultIniTxt)
        self.assertEqual(self._iniFile.fileTxt, self._defaultIniTxt)

    def test_hasFile_noFileTxtSet(self):
        self.createIniFile()

        self._iniFile._setupFileLines(fileTxt = self._defaultIniTxt)
        self.assertEqual(self._iniFile.fileTxt, "")
            
    # ====================================================================
    # ====================== readFileLines ===============================
        
    def test_noFile_savedTxtLinesOnInit(self):
        self.setupIniTxt(self._defaultIniTxt)
        self._file = None
        self.createIniFile()

        self.compareList(self._iniFile.fileLines, self._iniTxtLines)

        iniFileTxtLst = ["", "fa la la la la la la la la", "\n\n\n", "\n", self._defaultIniTxt]
        for txt in iniFileTxtLst:
            self.setupIniTxt(txt)
            self._iniFile.fileTxt = txt

            self.assertEqual(self._iniFile.fileTxt, self._iniTxt)
            result = self._iniFile.readFileLines()
            self.compareList(result, self._iniTxtLines)
            self.assertEqual(self._iniFile.fileTxt, self._iniTxt)

    def test_hasFile_textLinesReadFromFile(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()
        numOfTimesFileRead = 5

        for i in range(numOfTimesFileRead):
            result = self._iniFile.readFileLines()
            self.compareList(result, self._iniTxtLines)
            self.assertEqual(self._iniFile.fileTxt, self._iniTxt)

        self.setupIniTxt("BOOO")
        result = self._iniFile.readFileLines()
        self.compareList(result, self._iniTxtLines)
        self.assertEqual(self._iniFile.fileTxt, self._iniTxt)
        
    # ====================================================================
    # ====================== checkIsMod ==================================
        
    def test_hasModTypes_lineInIniMatchesModTypes(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()

        iniTxtDict = { self._defaultIniTxt : True, 
                        r"""Bing Bang Boom
                            Hello
                            lw $s2, 0($s3)
                            [ LittleBlackNekoWitch     ]
                            witchMutex.lock();
                            for (int i = 1; i < 5; ++i) {
                                magic += 1;
                                truth ||= False;
                            }
                            im not going to put a new line here
                            [LittleBlackNekoWitch]
                            witch->familiar.attack();
                            witchMutex.unlock();
                        """ : True,
                        """[CuteLittleEi]
                        Needs her dango
                        Dango dango dango dango
                        Dango dango daikazoku
                        """: False}
        
        for txt in iniTxtDict:
            txtResult = iniTxtDict[txt]
            self._iniFile.clear()
            self.setupIniTxt(txt)
            self._iniFile.fileTxt = txt

            result = self._iniFile.checkIsMod()
            self.assertEqual(result, txtResult)
            self.assertEqual(self._iniFile.isModIni, txtResult)

            if (txtResult):
                self.assertIn(self._iniFile.type, self._modTypes)
            else:
                self.assertIs(self._iniFile.type, None)

    def test_noModTypes_lineMatchesTextureOverrideBlendSection(self):
        self.setupIniTxt(self._defaultIniTxt)
        self._modTypes = None
        self.createIniFile()

        iniTxtDict = { self._defaultIniTxt : True, 
                        r"""Bing Bang Boom
                            Hello
                            lw $s2, 0($s3)
                            [ LittleBlackNekoWitch     ]
                            witchMutex.lock();
                            for (int i = 1; i < 5; ++i) {
                                magic += 1;
                                truth ||= False;
                            }
                            im not going to put a new line here
                            [LittleBlackNekoWitch]
                            witch->familiar.attack();
                            witchMutex.unlock();
                        """ : False,
                        """[CuteLittleEi]
                        Needs her dango
                        Dango dango dango dango
                        Dango dango daikazoku

                        [TextureOverrideRaiden]
                        Missing da blend
                        """: False,
                        r"""Bing Bang Boom
                            Hello
                            lw $s2, 0($s3)
                            [ TextureOverrideLittleBlackNekoWitchBlend     ]
                            witchMutex.lock();
                            for (int i = 1; i < 5; ++i) {
                                magic += 1;
                                truth ||= False;
                            }
                            im not going to put a new line here
                            [TextureOverrideLittleBlackNekoWitchBlend]
                            witch->familiar.attack();
                            witchMutex.unlock();
                        """ : True}
        
        for txt in iniTxtDict:
            txtResult = iniTxtDict[txt]
            self._iniFile.clear()
            self.setupIniTxt(txt)
            self._iniFile.fileTxt = txt

            result = self._iniFile.checkIsMod()
            self.assertEqual(result, txtResult)
            self.assertEqual(self._iniFile.isModIni, txtResult)
            self.assertIs(self._iniFile.type, None)
        
    # ====================================================================
    # ====================== _checkModType ===============================

    def test_hasModTypesHasDefaultType_typeIdentified(self):
        self.createIniFile()

        self._iniFile.clear()
        self._iniFile._checkModType("[TextureOverrideRaidenShogunBlender]")
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, "TextureOverrideRaidenShogunBlender")
        self.assertEqual(self._iniFile._textureOverrideBlendSectionName, "TextureOverrideRaidenShogunBlender")
        self.assertIn(self._iniFile.type, self._modTypes)
        self.assertEqual(self._iniFile.isModIni, True)

        self._iniFile.clear()
        self._iniFile._checkModType("    abc [ TextureOverrideRaiden  Blend]  def")
        self.assertIs(self._iniFile._textureOverrideBlendRoot, None)
        self.assertEqual(self._iniFile._textureOverrideBlendSectionName, None)
        self.assertIs(self._iniFile.type, None)
        self.assertEqual(self._iniFile.isModIni, False)

        self._iniFile.clear()
        self._iniFile._checkModType("     [ LittleBlackNekoWitch]  def")
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, "LittleBlackNekoWitch")
        self.assertEqual(self._iniFile._textureOverrideBlendSectionName, None)
        self.assertIn(self._iniFile.type, self._modTypes)
        self.assertEqual(self._iniFile.isModIni, True)

        self._iniFile.clear()
        self._iniFile._checkModType("     [ TextureOverrideRaiden  Blend]  // A dumb comment")
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, "TextureOverrideRaiden  Blend")
        self.assertEqual(self._iniFile._textureOverrideBlendSectionName, "TextureOverrideRaiden  Blend")
        self.assertIn(self._iniFile.type, self._modTypes)
        self.assertEqual(self._iniFile.isModIni, True)

        self._iniFile._checkModType("     [ CelesXLocke ]")
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, "TextureOverrideRaiden  Blend")
        self.assertEqual(self._iniFile._textureOverrideBlendSectionName, "TextureOverrideRaiden  Blend")
        self.assertIn(self._iniFile.type, self._modTypes)
        self.assertEqual(self._iniFile.isModIni, True)

        # note:
        #   when using Regex Strings in the checks for 'ModTypes', we may get cases of multiple matches that could be nested toghther
        #   For simplicity, we only get the name of the section from what is enclosed in the outer most square brackets
        #   So you may get some weird behaviour for some very weird edge cases like the ones below
        #       (though most people writing the .ini files for mods would probably know not to do such weird cases)
        self._iniFile.clear()
        self._iniFile._checkModType("     [ TextureOverride, [ 1,2,3, [Internal, [ Shogun]], 'weird', [TextureOverride [TextureOverrideRaidenBlend] Shogun Blend]]  , Blend   ]  // A dumb comment")
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, "TextureOverride, [ 1,2,3, [Internal, [ Shogun]], 'weird', [TextureOverride [TextureOverrideRaidenBlend] Shogun Blend]]  , Blend")
        self.assertEqual(self._iniFile._textureOverrideBlendSectionName, "TextureOverride, [ 1,2,3, [Internal, [ Shogun]], 'weird', [TextureOverride [TextureOverrideRaidenBlend] Shogun Blend]]  , Blend")
        self.assertIn(self._iniFile.type, self._modTypes)
        self.assertEqual(self._iniFile.isModIni, True)

        self._iniFile.clear()
        self._iniFile._checkModType("  {'some Json': 12345, 'why are we still here': ['cause why not'], 'boo': [ LittleBlackNekoWitch , [[ [ LittleBlackNekoWitch ]]]], 'baa': [dfdfdfdf]}   ")
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, "'cause why not'], 'boo': [ LittleBlackNekoWitch , [[ [ LittleBlackNekoWitch ]]]], 'baa': [dfdfdfdf")
        self.assertIs(self._iniFile._textureOverrideBlendSectionName, None)
        self.assertIn(self._iniFile.type, self._modTypes)
        self.assertEqual(self._iniFile.isModIni, True)

    def test_noModTypesHasDefaultType_textureOverrideBlendIdentified(self):
        self._modTypes = None
        self.createIniFile()

        self._iniFile._checkModType("[TextureOverrideAyayaBlends]")
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, "TextureOverrideAyayaBlends")
        self.assertEqual(self._iniFile._textureOverrideBlendSectionName, "TextureOverrideAyayaBlends")
        self.assertIs(self._iniFile.type, None)
        self.assertEqual(self._iniFile.isModIni, True)

        self._iniFile._type = FRB.ModTypes.Raiden.value
        self._iniFile._checkModType(" [] [CommandListAyayaAyayaBlend]  []")
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, "TextureOverrideAyayaBlends")
        self.assertEqual(self._iniFile._textureOverrideBlendSectionName, "TextureOverrideAyayaBlends")
        self.assertIsInstance(self._iniFile.type, FRB.ModType)
        self.assertEqual(self._iniFile.isModIni, True)

        self._iniFile.clear()
        self._iniFile._checkModType(" [] [TextureOverrideAyayaBlend]  []")
        self.assertIs(self._iniFile._textureOverrideBlendRoot, None)
        self.assertIs(self._iniFile._textureOverrideBlendSectionName, None)
        self.assertIs(self._iniFile.type, None)
        self.assertEqual(self._iniFile.isModIni, False)

    def test_noModTypesNoDefaultType_textureOverrideBlendIdentified(self):
        self._defaultModType = None
        self._modTypes = None
        self.createIniFile()

        self._iniFile._checkModType("[TextureOverrideAyayaBlends]")
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, "TextureOverrideAyayaBlends")
        self.assertIs(self._iniFile._textureOverrideBlendSectionName, None)
        self.assertIs(self._iniFile.type, None)
        self.assertEqual(self._iniFile.isModIni, True)

        self._iniFile._type = FRB.ModTypes.Raiden.value
        self._iniFile._checkModType(" [] [CommandListAyayaAyayaBlend]  []")
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, "TextureOverrideAyayaBlends")
        self.assertIs(self._iniFile._textureOverrideBlendSectionName, None)
        self.assertIsInstance(self._iniFile.type, FRB.ModType)
        self.assertEqual(self._iniFile.isModIni, True)

        self._iniFile.clear()
        self._iniFile._checkModType(" [] [TextureOverrideAyayaBlend]  []")
        self.assertIs(self._iniFile._textureOverrideBlendRoot, None)
        self.assertIs(self._iniFile._textureOverrideBlendSectionName, None)
        self.assertIs(self._iniFile.type, None)
        self.assertEqual(self._iniFile.isModIni, False)

    def test_hasModTypesNoDefaultType_typeIdentified(self):
        self._defaultModType = None
        self.createIniFile()

        self._iniFile.clear()
        self._iniFile._checkModType("[TextureOverrideRaidenShogunBlender]")
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, "TextureOverrideRaidenShogunBlender")
        self.assertEqual(self._iniFile._textureOverrideBlendSectionName, None)
        self.assertIn(self._iniFile.type, self._modTypes)
        self.assertEqual(self._iniFile.isModIni, True)

        self._iniFile.clear()
        self._iniFile._checkModType("[TextureOverrideAyayaBlends]")
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, None)
        self.assertEqual(self._iniFile._textureOverrideBlendSectionName, None)
        self.assertIs(self._iniFile.type, None)
        self.assertEqual(self._iniFile.isModIni, False)

        self._iniFile.clear()
        self._iniFile._checkModType("     [ LittleBlackNekoWitch]  def")
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, "LittleBlackNekoWitch")
        self.assertEqual(self._iniFile._textureOverrideBlendSectionName, None)
        self.assertIn(self._iniFile.type, self._modTypes)
        self.assertEqual(self._iniFile.isModIni, True)

        self._iniFile.clear()
        self._iniFile._checkModType("     [ TextureOverrideRaiden  Blend]  // A dumb comment")
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, "TextureOverrideRaiden  Blend")
        self.assertEqual(self._iniFile._textureOverrideBlendSectionName, None)
        self.assertIn(self._iniFile.type, self._modTypes)
        self.assertEqual(self._iniFile.isModIni, True)

        self._iniFile._checkModType("     [ CelesXLocke ]")
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, "TextureOverrideRaiden  Blend")
        self.assertEqual(self._iniFile._textureOverrideBlendSectionName, None)
        self.assertIn(self._iniFile.type, self._modTypes)
        self.assertEqual(self._iniFile.isModIni, True)

    # ====================================================================
    # ====================== _checkFixed =================================
        
    def test_differentIniLines_textureOverrideRemapBlendIdentified(self):
        self.createIniFile()

        self._iniFile._checkFixed(" [ TextureOverride FruitAndVegetables RemapBlendJet ] ")
        self.assertEqual(self._iniFile.isFixed, True)

        self._iniFile._checkFixed(" [OmegaWeapon] ")
        self.assertEqual(self._iniFile.isFixed, True)

        self._iniFile.clear()
        self._iniFile._checkFixed(" [OmegaWeapon] ")
        self.assertEqual(self._iniFile.isFixed, False)
        
    # ====================================================================
    # ====================== _parseSection ===============================
        
    def test_validIniInstance_sectionKVPs(self):
        self.createIniFile()

        sectionName = "someSection"
        save = {}
        iniStr = f"""
                [Hello]
                a = 1
                b = 2

                [{sectionName}]
                ; some weird RPG stats
                ; Menu screen
                # ----------- 
                hp = 9999
                mp = 999

                atk = 99
                magic = 99

                # Substats that nobody cares about
                spd = 99
                int = 
                str = 99
                global persist dex = 99
                magic = 88
                # repeated magic?
                                    
                              
                         

                # inconsitency

                weapon = lightbringer
                relic = hero ring
                job = red mage

                # stat edits
                atk = 100
                magic = 100
                spd = 100
                str = 100
                lv=
                [Love]
                kills = 9999
                """
        sectionDict = {"hp": [(0, '9999')], "mp": [(1, '999')], "atk": [(2, '99'), (12, '100')], "magic": [(3, '99'), (8, '88'), (13, '100')], "spd": [(4, '99'), (14, '100')], "int": [(5, '')], "str": [(6, '99'), (15, '100')], "global persist dex": [(7, '99')], "weapon": [(9, 'lightbringer')], "relic": [(10, 'hero ring')], "job": [(11, 'red mage')], "lv": [(16, '')]}

        result = self._iniFile._parseSection(sectionName, iniStr)
        self.compareIfContentPartSrc(result, sectionDict)

        result = self._iniFile._parseSection(sectionName, iniStr, save = save)
        self.assertIn(sectionName, save)
        self.compareIfContentPartSrc(result, sectionDict)

        save = []
        result = self._iniFile._parseSection(sectionName, iniStr, save = save)
        self.compareList(save, [])
        self.compareIfContentPartSrc(result, sectionDict)

        result = self._iniFile._parseSection("sectionNotFound", iniStr)
        self.assertIs(result, None)

    def test_invalidIniInstance_noResult(self):
        self.createIniFile()

        sectionName = "someSection"
        save = {}
        iniStr = f"""
                [Hello]
                a = 1
                b = 2
                [{sectionName}]
                abcdefg == 1
                efefifj = 3
                dfdfdfdf
                [Love]
                kills = 9999
                """

        result = self._iniFile._parseSection(sectionName, iniStr)
        self.assertIs(result, None)

        result = self._iniFile._parseSection(sectionName, iniStr, save = save)
        self.assertNotIn(sectionName, save)

        save = []
        result = self._iniFile._parseSection(sectionName, iniStr, save = save)
        self.compareList(save, [])
        self.assertIs(result, None)

        result = self._iniFile._parseSection("sectionNotFound", iniStr)
        self.assertIs(result, None)

    # ====================================================================
    # ====================== _getSectionName =============================
        
    def test_iniLines_sectionNamesWithinOuterSquareBrackets(self):
        self.createIniFile()

        sectionTxt = {"[hello]": "hello",
                      "sectionName = [ 1, 2, 3 ] // some comment": "1, 2, 3",
                      "<<[[[heavy]]]>> ---- ant lifting dumbell ---- <<[[[heavy]]]>>": "[[heavy]]]>> ---- ant lifting dumbell ---- <<[[[heavy]]",
                      "bing ping [  pong bong ": "pong bong",
                      " bong pong ] ping bing": "bong pong",
                      " the gate is open!! ": "the gate is open!!"}

        for section in sectionTxt:
            result = self._iniFile._getSectionName(section)
            self.assertEqual(result, sectionTxt[section])

    # ====================================================================
    # ====================== getSectionOptions ===========================
            
    def test_searchAllSectionsListPostProcessorArgs_getAllSections(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()

        result = self._iniFile.getSectionOptions(self._iniFile._sectionPattern, lambda startInd, endInd, fileLines, sectionName, srcTxt: [startInd, endInd, fileLines, sectionName, srcTxt])
        
        self.assertEqual(len(result), 17)
        for sectionName in result:
            sections = result[sectionName]
            for section in sections:
                startInd, endInd, fileLines, sectionNameArg, srcTxt = section

                self.assertEqual(sectionName, sectionNameArg)
                self.assertEqual(srcTxt, "".join(fileLines[startInd:endInd]))

    def test_searchAllSectionsListPostProcessorArgsKeepFirst_getFirstInstanceOfAllSections(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()

        result = self._iniFile.getSectionOptions(self._iniFile._sectionPattern, lambda startInd, endInd, fileLines, sectionName, srcTxt: [startInd, endInd, fileLines, sectionName, srcTxt], lambda duplicates: duplicates[0])
        
        self.assertEqual(len(result), 17)
        for sectionName in result:
            startInd, endInd, fileLines, sectionNameArg, srcTxt = result[sectionName]

            self.assertEqual(sectionName, sectionNameArg)
            self.assertEqual(srcTxt, "".join(fileLines[startInd:endInd]))

    def test_searchAllSectionsNoPostProcessor_sectionsOfStandardIniFromConfigParser(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()

        result = self._iniFile.getSectionOptions(self._iniFile._sectionPattern)
        self.assertEqual(len(result), 5)

    def test_searchSomeSections_someSectionsReturns(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()

        result = self._iniFile.getSectionOptions(re.compile("^\s*\[\s*Resource.*\s*\]"), lambda startInd, endInd, fileLines, sectionName, srcTxt: [startInd, endInd, fileLines, sectionName, srcTxt])
        
        self.assertEqual(len(result), 4)
        for sectionName in result:
            sections = result[sectionName]
            for section in sections:
                startInd, endInd, fileLines, sectionNameArg, srcTxt = section
                self.assertEqual(sectionName, sectionNameArg)
                self.assertEqual(srcTxt, "".join(fileLines[startInd:endInd]))

    def test_searchExactSection_onlyCertainsSection(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()

        searchStr = "                    [ResourceRaidenShogunRemapBlend.0]\n"
        result = self._iniFile.getSectionOptions(searchStr, handleDuplicateFunc = lambda duplicates: duplicates[0])
        self.assertEqual(len(result), 1)
        self.compareIfContentPartSrc(result["ResourceRaidenShogunRemapBlend.0"], {"type": [(0, "Buffer")], "stride": [(1, "32")], "filename": [(2, r"..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf")] })

        result = self._iniFile.getSectionOptions(searchStr, handleDuplicateFunc = lambda duplicates: duplicates[-1])
        self.assertEqual(len(result), 1)
        self.compareIfContentPartSrc(result["ResourceRaidenShogunRemapBlend.0"], {"type": [(0, "Binaries")], "stride": [(1, "31")], "filename": [(2, r"..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf")] })

    def test_predicateContradiction_noSections(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()

        result = self._iniFile.getSectionOptions(lambda line: False)
        self.assertEqual(len(result), 0)
        
    # ====================================================================
    # ====================== _removeSection ==============================
        
    def test_sectionIndices_sectionStartIndAndEndIndBeforeFileLineLen(self):
        self.createIniFile()

        fileLinesLen = 10
        sectionName = "someSection"
        srcTxt = "text"
        sectionIndicePairs = [(0,0), (1,5), (0,9), (0,11), (22, 32)]

        for sectionIndexPair in sectionIndicePairs:
            startInd = sectionIndexPair[0]
            endInd = sectionIndexPair[1]
            result = self._iniFile._removeSection(startInd, endInd, ["the end is never"] * fileLinesLen, sectionName, srcTxt)
            
            if (endInd > fileLinesLen):
                endInd = fileLinesLen

            if (startInd > fileLinesLen):
                startInd = fileLinesLen
            
            self.compareList(result, (startInd, endInd))
        
    # ====================================================================
    # ====================== removeSectionOptions ========================
            
    def test_getAllSections_emptysIniTxt(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()

        self._iniFile.removeSectionOptions(self._iniFile._sectionPattern)

        self.assertEqual(self._iniFile.fileTxt, "\n")
        self.compareList(self._iniFile.fileLines, ["\n"])

    def test_getNoSections_sameIniTxt(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()

        self._iniFile.removeSectionOptions(lambda line: False)

        self.assertEqual(self._iniFile.fileTxt, self._iniTxt)
        self.compareList(self._iniFile.fileLines, self._iniTxtLines)

    def test_getSomeSections_foundSectionsRemoved(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()

        sectionName = "                    [RaidenPuppetCommandResourceRemapBlend]"
        sectionIndexPairs = self._iniFile.getSectionOptions(sectionName, lambda startInd, endInd, fileLines, sectionName, srcTxt: [startInd, endInd])
        
        expectedTxt = self._defaultIniTxt
        for name in sectionIndexPairs:
            indexPairs = sectionIndexPairs[name]
            indexPairsLen = len(indexPairs)

            for i in range(indexPairsLen - 1, -1, -1):
                indexPair = indexPairs[i]
                expectedTxt = expectedTxt[:indexPair[0]] + expectedTxt[indexPair[1]:]

        self._iniFile.removeSectionOptions("                    [RaidenPuppetCommandResourceRemapBlend]")
        self.setupIniTxt(expectedTxt)
        self.assertEqual(self._iniFile.fileTxt, self._iniTxt)
        self.assertEqual(self._iniFile.fileLines, self._iniTxtLines)
    
    # ====================================================================
    # ====================== _commentSection =============================

    #TODO: Add tests for commenting out a section

    # ====================================================================
    # ====================== commentSectionOptions =======================

    #TODO: Add tests for commenting out specific sections

    # ====================================================================
    # ====================== _processIfTemplate ==========================
        
    def test_validIfTemplate_generatedIfTemplates(self):
        self.createIniFile()

        sectionName = "someSection"
        ifTemplateLines = [[[f"[{sectionName}]",
                            "if $swap == 0\n",
                            "run = subRoutine1\n",
                            "else\n",
                            "run = subRoutine2\n",
                            "endif\n"], 
                            [FRB.IfPredPart("if $swap == 0\n", FRB.IfPredPartType.If),
                             FRB.IfContentPart({"run": [(0, "subRoutine1")]}, 1),
                             FRB.IfPredPart("else\n", FRB.IfPredPartType.Else),
                             FRB.IfContentPart({"run": [(0, "subRoutine2")]}, 1),
                             FRB.IfPredPart("endif\n", FRB.IfPredPartType.EndIf)]],
                             
                             [[f"[{sectionName}]",
                               "run = hello\n",
                               "stop = haltingProblem"],
                              [FRB.IfContentPart({"run": [(0, "hello")], "stop": [(1, "haltingProblem")]}, 0)]],
                                
                              [[f"[{sectionName}]\n",
                                "; Kaiser Dragon Partial AI Script: https://finalfantasy.fandom.com/wiki/Kaiser_Dragon_(Final_Fantasy_VI)\n",
                                "hp = 327500\n",
                                "mp = 60000\n",
                                "if $form == Fire\n",
                                    "if $fun == 0\n",
                                        "1stTurn = Firaga\n",
                                    "else\n",
                                        "1stTurn = Meteor\n",
                                    "endif\n",
                                    "if $fun == 0",
                                        "2ndTurn = Flare\n",
                                    "else if $fun == 1\n",
                                        "2ndTurn = Meltdown\n",
                                    "else\n",
                                        "2ndTurn = Flare Star\n",
                                    "endif\n",
                                    "if $attakced = 1\n",
                                        "counter = Southern Cross\n",
                                    "endif\n",
                                "else if $form == Earth\n",
                                    "1stTurn = Attack\n",
                                    "if $fun == 0\n",
                                        "2ndTurn = Attack\n",
                                    "else\n",
                                        "2ndTurn = Last Breath\n",
                                    "endif\n",
                                    "3rdTurn = Last Breath\n",
                                "endif",
                                "\n",
                                "# Final Stage",
                                "if $timer >= 15\n",
                                    "desperateAttack1 = Heartless Angel\n",
                                    "desperateAttack2 = Mind Blast\n",
                                "endif\n",
                                "finalAttack = Ultima"], 
                                [FRB.IfContentPart({"hp": [(0, "327500")], "mp": [(1, "60000")]}, 0),
                                 FRB.IfPredPart("if $form == Fire\n", FRB.IfPredPartType.If),
                                    FRB.IfPredPart("if $fun == 0\n", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"1stTurn": [(0, "Firaga")]}, 2),
                                    FRB.IfPredPart("else\n", FRB.IfPredPartType.Else),
                                        FRB.IfContentPart({"1stTurn": [(0, "Meteor")]}, 2),
                                    FRB.IfPredPart("endif\n", FRB.IfPredPartType.EndIf),
                                    FRB.IfPredPart("if $fun == 0", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"2ndTurn": [(0, "Flare")]}, 2),
                                    FRB.IfPredPart("else if $fun == 1\n", FRB.IfPredPartType.Else),
                                        FRB.IfContentPart({"2ndTurn": [(0, "Meltdown")]}, 2),
                                    FRB.IfPredPart("else\n", FRB.IfPredPartType.Else),
                                        FRB.IfContentPart({"2ndTurn": [(0, "Flare Star")]}, 2),
                                    FRB.IfPredPart("endif\n", FRB.IfPredPartType.EndIf),
                                    FRB.IfPredPart("if $attakced = 1\n", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"counter": [(0, "Southern Cross")]}, 2),
                                    FRB.IfPredPart("endif\n", FRB.IfPredPartType.EndIf),
                                 FRB.IfPredPart("else if $form == Earth\n", FRB.IfPredPartType.Else),
                                    FRB.IfContentPart({"1stTurn": [(0, "Attack")]}, 1),
                                    FRB.IfPredPart("if $fun == 0\n", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"2ndTurn": [(0, "Attack")]}, 2),
                                    FRB.IfPredPart("else\n", FRB.IfPredPartType.Else),
                                        FRB.IfContentPart({"2ndTurn": [(0, "Last Breath")]}, 2),
                                    FRB.IfPredPart("endif\n", FRB.IfPredPartType.EndIf),
                                    FRB.IfContentPart({"3rdTurn": [(0, "Last Breath")]}, 1),
                                 FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf),
                                 FRB.IfContentPart({}, 0),
                                 FRB.IfPredPart("if $timer >= 15\n", FRB.IfPredPartType.If),
                                    FRB.IfContentPart({"desperateAttack1": [(0, "Heartless Angel")], "desperateAttack2": [(1, "Mind Blast")]}, 1),
                                 FRB.IfPredPart("endif\n", FRB.IfPredPartType.EndIf),
                                 FRB.IfContentPart({"finalAttack": [(0, "Ultima")]}, 0)]]]

        for lines in ifTemplateLines:
            expected = lines[1]
            input = lines[0]
            inputLen = len(input)
            result = self._iniFile._processIfTemplate(0, inputLen, input, sectionName, "")
            self.compareIfTemplateParts(result.parts, expected)

    def test_invalidIfTemplate_generatedIfTemplateWithPartsIgnored(self):
        self.createIniFile()

        sectionName = "someSection"
        ifTemplateLines = [[[f"[{sectionName}]\n",
                             f"if $i <= 0\n",
                                f"baseCase = done\n",
                             f"else\n",
                                f"$i = $i - 1\n",
                                f"run = {sectionName}_i-1\n",
                                f"$i = $i - 2\n",
                                f"run = {sectionName}_i-2\n",
                                f"Boo = 1\n",
                                f"$i = $i - 3\n",
                                f"run = {sectionName}_i-3\n",
                             f"endif"],
                             [FRB.IfPredPart(f"if $i <= 0\n", FRB.IfPredPartType.If),
                                FRB.IfContentPart({"baseCase": [(0, "done")]}, 1),
                              FRB.IfPredPart(f"else\n", FRB.IfPredPartType.Else),
                                FRB.IfContentPart({"$i": [(0, "$i - 1"), (2, "$i - 2"), (5, "$i - 3")], 
                                                   "run": [(1, f"{sectionName}_i-1"), (3, f"{sectionName}_i-2"), (6, f"{sectionName}_i-3")],
                                                   "Boo": [(4, "1")]}, 1),
                              FRB.IfPredPart(f"endif", FRB.IfPredPartType.EndIf)]],
                              
                              [[f"[{sectionName}]\n",
                                "if $noClosing == 1\n",
                                    "error = 1"],
                                [FRB.IfPredPart("if $noClosing == 1\n", FRB.IfPredPartType.If),
                                 FRB.IfContentPart({"error": [(0, "1")]}, 1)]],
                                 
                              [[f"[{sectionName}]\n",
                                "if $badIni == 1\n",
                                    "efefefefefef\n",
                                "endif"],
                                [FRB.IfPredPart("if $badIni == 1\n", FRB.IfPredPartType.If),
                                    FRB.IfContentPart({}, 1),
                                FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf)]],

                              [["if $noSection == 1\n",
                                    "ohNo = 1\n",
                                "endif"],
                                [FRB.IfContentPart({"ohNo": [(0, "1")]}, 0),
                                 FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf)]], 
                                 
                              [[f"[{sectionName}]\n",
                                "if\n",
                                "missing = 1\n",
                                "elfi $hasSomething == 1\n",
                                "result = yay",
                                "else $booboo == 1"
                                "endif"],
                                [FRB.IfPredPart("if\n", FRB.IfPredPartType.If),
                                 FRB.IfContentPart({"missing": [(0, "1")], "elfi $hasSomething": [(1, "= 1")], "result": [(2, "yay")]}, 1),
                                 FRB.IfPredPart("else $booboo == 1endif", FRB.IfPredPartType.Else)]]]
        
        for lines in ifTemplateLines:
            expected = lines[1]
            input = lines[0]
            inputLen = len(input)
            result = self._iniFile._processIfTemplate(0, inputLen, input, sectionName, "")
            self.compareIfTemplateParts(result.parts, expected)
        

    # ====================================================================
    # ====================== getMergedResourceIndex ======================

    def test_resourceName_suffixToTheRightMostDot(self):
        resourceTxt = {"ResourceCuteLittleEiBlend.8": 8,
                       "ResourceCuteLittleEiBlend.Score.-100": -100,
                       "0": 0,
                       "FixRaidenBoss.WebAPI": None,
                       'System.out.println("Hello World")': None,
                       "NoDotNet1": None,
                       "": None}
        
        for resourceName in resourceTxt:
            result = FRB.IniFile.getMergedResourceIndex(resourceName)
            self.assertEqual(result, resourceTxt[resourceName])

    # ====================================================================
    # ====================== compareResources ===========================

    def test_differentResource_resourcesCompared(self):
        self.createIniFile()
        resourceTuples = [[("Resource.1", None), ("Resource.2", None), -1],
                          [("Resource.1", None), ("Resource.1", None), 0],
                          [("Resource.1", None), ("Resource", None), 1],
                          [("Resource", -0.1), ("Resource", 0), -1],
                          [("Resource", -0), ("Resource", 0), 0],
                          [("Resource", 0.1), ("Resource", 0), 1],
                          [("Resource", None), ("Resource", 1), -1],
                          [("Resource", 1), ("Resource", None), 1]]

        for tuples in resourceTuples:
            result = self._iniFile.compareResources(tuples[0], tuples[1])
            self.assertEqual(result, tuples[2])

    # ====================================================================
    # ====================== disIni ======================================

    def test_hasFile_fileDisabled(self):
        self.createIniFile()
        self._iniFile.disIni()

        self.patches["src.FixRaidenBoss2.FileService.disableFile"].assert_called_once()
        assert(self._file.endswith(".txt"))
        assert(self._file.startswith(FRB.FilePrefixes.BackupFilePrefix.value))

    def test_noFile_noFileDisabled(self):
        self._file = None
        self.createIniFile()
        self._iniFile.disIni()
        self.patches["src.FixRaidenBoss2.FileService.disableFile"].assert_not_called()

    # ====================================================================
    # ====================== getFixedBlendFile ===========================

    def test_differentFilePaths_pathsToFixedBlendFiles(self):
        filePaths = {"RaidenBlend.buf": "RaidenRemapBlend.buf",
                     "a/b/c/RaidenRemapBlend.buf": "a/b/c/RaidenRemapRemapBlend.buf",
                     "a/b.B/c/Raiden": "a/b.B/c/RaidenRemapBlend.buf",
                     "a.A/b.B/c.C/Raiden.blend.blender.bld": "a.A/b.B/c.C/Raiden.blend.blenderRemapBlend.buf",
                     "a.A/b.B/c.C/Raiden.Blend.Blender.bld": "a.A/b.B/c.C/Raiden.Blend.RemapBlender.buf",
                     "a.A/b.B/c.C/Raiden.ReactApp": "a.A/b.B/c.C/RaidenRemapBlend.buf"}
        
        for path in filePaths:
            result = FRB.IniFile.getFixedBlendFile(path)
            self.assertEqual(result, filePaths[path])

    # ====================================================================
    # ====================== getFixModTypeName ===========================

    def test_noModType_noFixedModTypeName(self):
        self.createIniFile()
        result = self._iniFile.getFixModTypeName()
        self.assertIs(result, None)

    def test_hasModType_typeNameWithoutNewLinesOrTabs(self):
        self.createIniFile()

        modTypes = [[FRB.ModType("Lamdadelta", "Lamdadelta", "annoying witch"), "Lamdadelta"],
                    [FRB.ModType("a\n\t\nb\nc  \t\t  ", "a", "b"), "abc    "],
                    [FRB.ModType("", "", ""), ""]]
        
        for modType in modTypes:
            self._iniFile._type = modType[0]
            result = self._iniFile.getFixModTypeName()
            self.assertEqual(result, modType[1])

    # ====================================================================
    # ====================== getHeadingName ==-===========================

    # TODO: Makes test for making the title in the comment heading
    
    # ====================================================================
    # ====================== getFixHeader ================================

    def test_noModType_defaultHeader(self):
        self.createIniFile()
        repeats = 3
        expectedHeader = FRB.Heading("GI Remap", 15, "-")

        for i in range(repeats):
            result =  self._iniFile.getFixHeader()
            self.assertEqual(result, f"; {expectedHeader.open()}")

    def test_hasModTypes_headerWithModTypeName(self):
        self.createIniFile()
        repeats = 3

        modTypes = [[FRB.ModType("Lamdadelta", "Lamdadelta", "annoying witch"), "Lamdadelta"],
                    [FRB.ModType("   a\n\t\nb\nc  \t\t  ", "a", "b"), "   abc    "],
                    [FRB.ModType("", "", ""), ""]]
        
        for modType in modTypes:
            self._iniFile._setType(modType[0])
            modTypeName = modType[1]
            if (modTypeName):
                modTypeName += " "

            for i in range(repeats):
                expectedHeader = FRB.Heading(f"{modTypeName}Remap", 15, "-")
                result = self._iniFile.getFixHeader()
                self.assertEqual(result, f"; {expectedHeader.open()}")

    # ====================================================================
    # ====================== getFixFooter ================================

    def test_noModType_defaultFooter(self):
        self.createIniFile()
        expectedHeader = FRB.Heading("GI Remap", 15, "-")
        repeats = 3

        for i in range(repeats):
            result =  self._iniFile.getFixFooter()
            self.assertEqual(result, f"\n\n; {expectedHeader.close()}")

    def test_hasModTypes_footerWithModTypeName(self):
        self.createIniFile()
        repeats = 3

        modTypes = [[FRB.ModType("Lamdadelta", "Lamdadelta", "annoying witch"), "Lamdadelta"],
                    [FRB.ModType("   a\n\t\nb\nc  \t\t  ", "a", "b"), "   abc    "],
                    [FRB.ModType("", "", ""), ""]]
        
        for modType in modTypes:
            self._iniFile._type = modType[0]
            self._iniFile._heading.title = None

            modTypeName = modType[1]
            if (modTypeName):
                modTypeName += " "

            expectedHeader = FRB.Heading(f"{modTypeName}Remap", 15, "-")

            for i in range(repeats):
                result = self._iniFile.getFixFooter()
                self.assertEqual(result, f"\n\n; {expectedHeader.close()}")

    # ====================================================================
    # ====================== getFixCredit ================================

    def test_noModType_defaultCredit(self):
        self.createIniFile()
        expected = FRB.IniBoilerPlate.Credit.value.replace(FRB.IniBoilerPlate.ModTypeNameReplaceStr.value, "Mod ").replace(FRB.IniBoilerPlate.ShortModTypeNameReplaceStr.value, "")

        result =  self._iniFile.getFixCredit()
        self.assertEqual(result, expected)

    def test_hasModTypes_creditWithModTypeName(self):
        self.createIniFile()

        modTypes = [[FRB.ModType("Lamdadelta", "Lamdadelta", "annoying witch"), "Lamdadelta ", "Lamdadelta "],
                    [FRB.ModType("   a\n\t\nb\nc  \t\t  ", "a", "b"), "   abc     ", "   abc     "],
                    [FRB.ModType("", "", ""), "", ""]]
        
        for modType in modTypes:
            self._iniFile._type = modType[0]
            self._iniFile._heading.title = None
            expected = FRB.IniBoilerPlate.Credit.value.replace(FRB.IniBoilerPlate.ModTypeNameReplaceStr.value, f"{modType[1]}").replace(FRB.IniBoilerPlate.ShortModTypeNameReplaceStr.value, modType[2])

            result = self._iniFile.getFixCredit()
            self.assertEqual(result, expected)

    # ====================================================================
    # ====================== getResourceName =============================

    def test_differentNames_resourceNames(self):
        names = {"CuteLittleEi": "ResourceCuteLittleEi",
                 "ResourceCuteLittleEi": "ResourceCuteLittleEi",
                 "": "Resource",
                 "Resource": "Resource",
                 "resource": "Resourceresource"}
        
        for input in names:
            result = FRB.IniFile.getResourceName(input)
            self.assertEqual(result, names[input])

    # ====================================================================
    # ====================== removeResourceName ==========================

    def test_differentNames_namesWithoutResourcePrefix(self):
        names = {"CuteLittleEi": "CuteLittleEi",
                 "ResourceCuteLittleEi": "CuteLittleEi",
                 "": "",
                 "Resource": "",
                 "resource": "resource"}
        
        for input in names:
            result = FRB.IniFile.removeResourceName(input)
            self.assertEqual(result, names[input])   

    # ====================================================================
    # ================= getRemapBlendName ================================

    def test_differentNames_remappedNames(self):
        names = {"EiTriesToUseBlenderAndFails": "EiTriesToUseRemapBlenderAndFails",
                 "EiBlendsTheBlender": "EiBlendsTheRemapBlender",
                 "ResourceCuteLittleEi": "ResourceCuteLittleEiRemapBlend",
                 "ResourceCuteLittleEiRemapBlend": "ResourceCuteLittleEiRemapRemapBlend",
                 "": "RemapBlend",
                 "RemapBlend": "RemapRemapBlend",
                 "Blend": "RemapBlend",
                 "blend": "blendRemapBlend"}
        
        for input in names:
            result = FRB.IniFile.getRemapBlendName(input)
            self.assertEqual(result, names[input]) 

    # TODO: Add cases for changing the type of mod to fix to

    # ====================================================================
    # ================= getRemapFixName ==================================

    # TODO: Make tests for adding a remapFix suffix

    # ====================================================================
    # ================= getRemapFixName ==================================

    # TODO: Makes test for adding a remapTex suffix

    # ====================================================================
    # =================== getRemapBlendResourceName ======================

    def test_differentNames_remappedResourceNames(self):
        names = {"CuteLittleEi": "ResourceCuteLittleEiRemapBlend",
                 "EiTriesToUseBlenderAndFails": "ResourceEiTriesToUseRemapBlenderAndFails",
                 "EiBlendsTheBlender": "ResourceEiBlendsTheRemapBlender",
                 "ResourceCuteLittleEi": "ResourceCuteLittleEiRemapBlend",
                 "ResourceCuteLittleEiRemapBlend": "ResourceCuteLittleEiRemapRemapBlend",
                 "": "ResourceRemapBlend",
                 "RemapBlend": "ResourceRemapRemapBlend",
                 "Blend": "ResourceRemapBlend",
                 "blend": "ResourceblendRemapBlend"}
        
        for input in names:
            result = FRB.IniFile.getRemapBlendResourceName(input)
            self.assertEqual(result, names[input]) 

    # TODO: Add cases for changing the type of mod to fix to

    # ====================================================================
    # ====================== _isIfTemplateResource =======================

    def test_differentParts_whetherPartHasVB1(self):
        self.createIniFile()
        parts = [[{"vb1": "hello"}, True],
                 [{}, False],
                 [{"VB1": "vb1"}, False]]
        
        for part in parts:
            result = self._iniFile._isIfTemplateResource(part[0])
            self.assertEqual(result, part[1])

    # ====================================================================
    # ====================== _isIfTemplateDraw ===========================

    def test_differentParts_whetherPartHasDraw(self):
        self.createIniFile()
        parts = [[{"draw": "hello"}, True],
                 [{}, False],
                 [{"Draw": "vb1"}, False]]
        
        for part in parts:
            result = self._iniFile._isIfTemplateDraw(part[0])
            self.assertEqual(result, part[1])

    # ====================================================================
    # ====================== _isIfTemplateHash ===========================

    # TODO: Add tests for checking availability of hash


    # ====================================================================
    # ====================== _isIfTemplateMatchFirstIndex ================

    # TODO: Add tests for checking availability of index

    # ====================================================================
    # ====================== _getIfTemplateResourceName ==================

    def test_differentParts_getVB1Value(self):
        self.createIniFile()
        parts = [[{"vb1": "hello"}, "hello"],
                 [{}, None],
                 [{"Vb1": "vb1"}, None]]
        
        for part in parts:
            expected  = part[1]
            result = None

            try:
                result = self._iniFile._getIfTemplateResourceName(part[0])
            except KeyError:
                pass
            
            if (expected is None):
                self.assertIs(result, expected)
            else:
                self.assertEqual(result, part[1])    

    # ====================================================================
    # ====================== _getIfTemplateHash ==========================
    
    # TODO: Makes tests for getting the hash


    # ====================================================================
    # ====================== _getIfTemplateMatchFirstIndex ===============

    # TODO: Makes the test for getting the index

    # ====================================================================
    # ====================== _getAssetReplacement ========================

    # TODO: Make tests for getting the replacement for an asset


    # ====================================================================
    # ====================== _getHashReplacement =========================

    # TODO: Make tests for replacing hashes

    # ====================================================================
    # ====================== _getIndexReplacement ========================

    # TODO: Make tests for replacing indices

    # ====================================================================
    # ====================== _fillNonBlendSections =======================

    # TODO: Make test for filling the section not related to the Blend.buf files

    # ====================================================================
    # ====================== fillIfTemplate ==============================

    def test_differentIfTemplates_filledIfTemplates(self):
        self.createIniFile()
        fillFunc = lambda modName, sectionName, part, partIndex, linePrefix, origSectionName: "".join([f"modName: {modName}\n", f"sectionName: {sectionName}\n", f"part: {part}\n", f"partIndex: {partIndex}\n", f"linePrefix: [{linePrefix}]\n", f"origSectionName: {origSectionName}\n"])
        modName = "someModName"

        ifTemplateTests = [["someSection", FRB.IfTemplate([]), None, ["[someSection]\n"]],
                           ["someSection", FRB.IfTemplate([{"1stturn": "heartless angel"},
                                                          "if $fun == 0\n",
                                                            {"7thturn": "Thundaga"},
                                                          "else if $fun == 1\n",
                                                            {"7thturn": "Havoc Wing"},
                                                          "else\n",
                                                            {"7thturn": "Nothing"},
                                                          "endif\n",
                                                          "if $hp <= 32640\n",
                                                            {"8thturn": "The end draws near...",
                                                             "9thturn": "Forsaken"},
                                                            "\tif $fun == 0\n",
                                                                {"10thturn": "Havoc Wing",
                                                                 "11thturn": "Havoc Wing"},
                                                            "\telse if $fun == 1\n",
                                                                {"10thturn": "Trine",
                                                                 "11thturn": "Havoc Wing"},
                                                            "\telse\n",
                                                                {"10thturn": "Vengeance"},
                                                            "\tendif\n",
                                                          "endif"]), None, 
                                                          ["[someSection]",
                                                          f"modName: {modName}",
                                                           "sectionName: someSection",
                                                           "part: {'1stturn': 'heartless angel'}",
                                                           f"partIndex: {0}",
                                                           f"linePrefix: []",
                                                           "origSectionName: someSection",
                                                           "if $fun == 0",
                                                                f"modName: {modName}",
                                                                f"sectionName: someSection",
                                                                "part: {'7thturn': 'Thundaga'}",
                                                                f"partIndex: {2}",
                                                                f"linePrefix: [\t]",
                                                                "origSectionName: someSection",
                                                           "else if $fun == 1",
                                                                f"modName: {modName}",
                                                                f"sectionName: someSection",
                                                                "part: {'7thturn': 'Havoc Wing'}",
                                                                f"partIndex: {4}",
                                                                f"linePrefix: [\t]",
                                                                "origSectionName: someSection",
                                                           "else",
                                                                f"modName: {modName}",
                                                                f"sectionName: someSection",
                                                                "part: {'7thturn': 'Nothing'}",
                                                                f"partIndex: {6}",
                                                                f"linePrefix: [\t]",
                                                                "origSectionName: someSection",
                                                            "endif",
                                                            "if $hp <= 32640",
                                                                f"modName: {modName}",
                                                                f"sectionName: someSection",
                                                                "part: {'8thturn': 'The end draws near...', '9thturn': 'Forsaken'}",
                                                                f"partIndex: {9}",
                                                                f"linePrefix: [\t]",
                                                                "origSectionName: someSection",
                                                                "\tif $fun == 0",
                                                                    f"modName: {modName}",
                                                                    f"sectionName: someSection",
                                                                    "part: {'10thturn': 'Havoc Wing', '11thturn': 'Havoc Wing'}",
                                                                    f"partIndex: {11}",
                                                                    f"linePrefix: [\t\t]",
                                                                    "origSectionName: someSection",
                                                                "\telse if $fun == 1",
                                                                    f"modName: {modName}",
                                                                    f"sectionName: someSection",
                                                                    "part: {'10thturn': 'Trine', '11thturn': 'Havoc Wing'}",
                                                                    f"partIndex: {13}",
                                                                    f"linePrefix: [\t\t]",
                                                                    "origSectionName: someSection",
                                                                "\telse",
                                                                    f"modName: {modName}",
                                                                    f"sectionName: someSection",
                                                                    "part: {'10thturn': 'Vengeance'}",
                                                                    f"partIndex: {15}",
                                                                    f"linePrefix: [\t\t]",
                                                                    "origSectionName: someSection",
                                                                "\tendif",
                                                            "endif"]],
                            ["someSection", FRB.IfTemplate(["Hello Evernyan! How are you? Fine, thank you.\n",
                                                          {"OH MY": "GAHHHH"},
                                                          "I wish I were a bird."]), "oldSection",
                                                          ["[someSection]",
                                                           "Hello Evernyan! How are you? Fine, thank you.",
                                                           f"modName: {modName}",
                                                           f"sectionName: someSection",
                                                            "part: {'OH MY': 'GAHHHH'}",
                                                            f"partIndex: {1}",  
                                                            f"linePrefix: [\t]",
                                                            "origSectionName: oldSection",
                                                            "I wish I were a bird."]]]

        for ifTemplateTest in ifTemplateTests:
            result = self._iniFile.fillIfTemplate(modName, ifTemplateTest[0], ifTemplateTest[1], fillFunc, ifTemplateTest[2])
            expected = "\n".join(ifTemplateTest[3])
            self.assertEqual(result, expected)

        # TODO: Add case for different ModNames

    # ====================================================================
    # ====================== getModFixStr ================================

    # TODO: Make tests for fixing a single type of mod

    # ====================================================================
    # ====================== getFixStr ===================================

    # TODO: Changes up the tests to reflect the updated code

    def test_differentIniTxt_fixForIni(self):
        self.createIniFile()

        iniTxtTests = [[self._defaultIniTxt,
                   f"""; --------------- Raiden Remap ---------------
; Raiden remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Raiden mods pls give credit for "Nhok0169" and "Albert Gold#2696"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

PREFIX:


; ***** RaidenBoss *****
[TextureOverrideRaidenShogunRaidenBossRemapBlend]
run = CommandListRaidenShogunRaidenBossRemapBlend
handling = skip
draw = 21916,0

[CommandListRaidenShogunRaidenBossRemapBlend]
                    if $swapmain == 0
                        if $swapvar == 0 && $swapvarn == 0
                        \tvb1 = ResourceRaidenShogunRaidenBossRemapBlend.0
                        else
                        \tvb1 = ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie
                        endif
                    else if $swapmain == 1
                    \trun = SubSubTextureOverrideRaidenBossRemapBlend
                    endif

[SubSubTextureOverrideRaidenBossRemapBlend]
                    if $swapoffice == 0 && $swapglasses == 0
                    \tvb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend
                    endif


[ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend]
type = Buffer
stride = 32
filename = ../AAA/BBBB/CCCCCC/DDDDDRemapRaidenBossRemapBlend.buf

[ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie]
type = Buffer
stride = 32
                    if $swapmain == 1
                    \tfilename = M:/AnotherDrive/CuteLittleEiRaidenBossRemapBlend.buf
                    else
                    \trun = ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend
                    endif

[ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend]
type = Buffer
stride = 32
filename = Dont/Use/If/Statements/Or/SubCommands/In/Resource/SectionsRaidenBossRemapBlend.buf

[ResourceRaidenShogunRaidenBossRemapBlend.0]
type = Buffer
stride = 32
filename = ../../../../../../../../../2-BunnyRaidenShogun/RaidenShogunRaidenBossRemapBlend.buf

; **********************

; --------------------------------------------"""],
["", f"""; --------------- GI Remap ---------------
; Mod remapped by NK#1321 and Albert Gold#2696. If you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

PREFIX:


; ----------------------------------------"""],
[f"""
[TextureOverrideRaidenBlend]
handling = skip
draw = 21916,0
""",
f"""; --------------- Raiden Remap ---------------
; Raiden remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Raiden mods pls give credit for "Nhok0169" and "Albert Gold#2696"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

PREFIX:


; ***** RaidenBoss *****
[TextureOverrideRaidenRaidenBossRemapBlend]
handling = skip
draw = 21916,0


; **********************

; --------------------------------------------"""]]
        
        prefixStr = "\n\nPREFIX:\n"
        for iniTxtText in iniTxtTests:
            self._iniFile.clear()
            self._iniFile.fileTxt = iniTxtText[0]
            self._iniFile.parse()
            result = self._iniFile.getFixStr(fix = prefixStr)   
            self.assertEqual(result, iniTxtText[1])

    # ====================================================================
    # ====================== injectAddition ==============================

    def test_insertBeforeOriginalMakeBackupHasFile_txtAddedBeforeFileTxtWithBackupFile(self):
        openPatch = self.getOpenPatch()
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()

        iniFile = self._file
        addition = "Addition\n\n\tSubtraction"
        result = self._iniFile.injectAddition(addition, keepBackup = True, fixOnly = True)
        expected = f"{addition}\n\n{self._iniTxt}"

        assert(self._file.endswith(".txt"))
        assert(self._file.startswith(FRB.FilePrefixes.BackupFilePrefix.value))
        openPatch.assert_called_with(iniFile, 'w', encoding = FRB.FileEncodings.UTF8.value)
        openPatch.return_value.__enter__().write.assert_called_with(expected)
        self.assertEqual(result, expected)

    def test_insertBeforeOriginalNoBackupNoFile_txtAddedBeforeFileTxtNoBackupFile(self):
        openPatch = self.getOpenPatch()
        self._file = None
        self.createIniFile()

        addition = "Addition\n\n\tSubtraction"
        result = self._iniFile.injectAddition(addition, keepBackup = True, fixOnly = True)
        expected = f"{addition}\n\n{self._iniTxt}"

        self.patches["src.FixRaidenBoss2.FileService.disableFile"].assert_not_called()
        openPatch.assert_not_called()
        self.assertEqual(result, expected)

    def test_insertAfterOriginalNoBackup_txtAddedAfterFileTxtNoBackupFile(self):
        openPatch = self.getOpenPatch()
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()

        addition = "Addition\n\n\tSubtraction"
        result = self._iniFile.injectAddition(addition, beforeOriginal = False, keepBackup = True, fixOnly = False)
        expected = f"{self._iniTxt}\n{addition}"

        self.patches["src.FixRaidenBoss2.FileService.disableFile"].assert_not_called()
        openPatch.assert_called_with(self._file, 'w', encoding = FRB.FileEncodings.UTF8.value)
        openPatch.return_value.__enter__().write.assert_called_with(expected)
        self.assertEqual(result, expected)

        self._iniFile.clear()
        result = self._iniFile.injectAddition(addition, beforeOriginal = False, keepBackup = False, fixOnly = True)
        self.patches["src.FixRaidenBoss2.FileService.disableFile"].assert_not_called()
        openPatch.assert_called_with(self._file, 'w', encoding = FRB.FileEncodings.UTF8.value)
        openPatch.return_value.__enter__().write.assert_called_with(expected)
        self.assertEqual(result, expected)

        self._iniFile.clear()
        result = self._iniFile.injectAddition(addition, beforeOriginal = False, keepBackup = False, fixOnly = False)
        self.patches["src.FixRaidenBoss2.FileService.disableFile"].assert_not_called()
        openPatch.assert_called_with(self._file, 'w', encoding = FRB.FileEncodings.UTF8.value)
        openPatch.return_value.__enter__().write.assert_called_with(expected)
        self.assertEqual(result, expected)

    # ====================================================================
    # ====================== _removeFix ==================================

    def test_differentText_remapBlendSectionsAndScriptFixRemoved(self):
        self._file = None
        self.createIniFile()

        tests = {"Hello": "Hello",
                 "; --------------- Raiden Boss Fix ---------------\n\nFDFDFDFDF\n\n; -----------------------------------------------": "",
                 "; --------------- Raiden Boss Fix ---------------\n\nFDFDFDFDF\n\n; -------------------------------------": "; --------------- Raiden Boss Fix ---------------\n\nFDFDFDFDF\n\n; -------------------------------------",
                 self._defaultIniTxt: f"""[Constants]
                    global persist $swapvar = 0
                    global persist $swapvarn = 0
                    global persist $swapmain = 0
                    global persist $swapoffice = 0
                    global persist $swapglasses = 0

                    [KeyVar]
                    condition = $active == 1
                    key = VK_DOWN
                    type = cycle
                    $swapvar = 0,1,2

                    [KeyIntoTheHole]
                    condition = $active == 1
                    key = VK_RIGHT
                    type = cycle
                    $swapvarn = 0,1

                    ; The top part is not really important, so I not going to finish
                    ;   typing all the key swaps... 😋
                    ;
                    ; The bottom part is what the fix actually cares about

                    [TextureOverrideRaidenShogunBlend]
                    run = CommandListRaidenShogunBlend
                    handling = skip
                    draw = 21916,0

                    [CommandListRaidenShogunBlend]
                    if $swapmain == 0
                        if $swapvar == 0 && $swapvarn == 0
                            vb1 = ResourceRaidenShogunBlend.0
                        else
                            vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
                        endif
                    else if $swapmain == 1
                        run = SubSubTextureOverride
                    endif

                    [SubSubTextureOverride]
                    if $swapoffice == 0 && $swapglasses == 0
                        vb1 = GIMINeedsResourcesToAllStartWithResource
                    endif

                    [ResourceRaidenShogunBlend.0]
                    type = Buffer
                    stride = 32
                    filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

                    [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
                    type = Buffer
                    stride = 32
                    if $swapmain == 1
                        filename = M:\AnotherDrive\CuteLittleEi.buf
                    else
                        run = RaidenPuppetCommandResource
                    endif

                    [GIMINeedsResourcesToAllStartWithResource]
                    type = Buffer
                    stride = 32
                    filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

                    [RaidenPuppetCommandResource]
                    type = Buffer
                    stride = 32
                    filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

                    ; ------ some lines originally generated from the fix ---------\n\n""",
                    
                    """
[Bello]
byebye = banana

; ***** kyrie *****
[TextureOverrideGanyukyrieRemapBlend]
hash = HashNotFound
run = CommandListGanyukyrieRemapBlend

[CommandListGanyukyrieRemapBlend]
if $swapvar == 0
\tvb1 = ResourceGanyukyrieRemapBlend.0
\thandling = skip
\tdraw = 22548,0
else if $swapvar == 1
\tvb1 = ResourceGanyukyrieRemapBlend.1
\thandling = skip
\tdraw = 18988,0
else if $swapvar == 2
\tvb1 = ResourceGanyukyrieRemapBlend.2
\thandling = skip
\tdraw = 22555,0
else if $swapvar == 3
\tvb1 = ResourceGanyukyrieRemapBlend.3
\thandling = skip
\tdraw = 18995,0
endif

[TextureOverrideGanyuBodykyrieRemapFix]
hash = HashNotFound
match_first_index = IndexNotFound
run = CommandListGanyuBodykyrieRemapFix

[CommandListGanyuBodykyrieRemapFix]
if $swapvar == 0
\tib = ResourceGanyuBodyIB.0
\tps-t1 = ResourceGanyuBodyLightMap.0
else if $swapvar == 1
\tib = ResourceGanyuBodyIB.1
\tps-t1 = ResourceGanyuBodyLightMap.1
else if $swapvar == 2
\tib = ResourceGanyuBodyIB.2
\tps-t1 = ResourceGanyuBodyLightMap.2
else if $swapvar == 3
\tib = ResourceGanyuBodyIB.3
\tps-t1 = ResourceGanyuBodyLightMap.3
endif

[TextureOverrideGanyuHeadkyrieRemapFix]
hash = HashNotFound
match_first_index = missa tota
run = CommandListGanyuHeadkyrieRemapFix
cd-1 = ResourceKyrieHeadSaturatedDiffusekyrieRemapTex
cd-3 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
cd-3-1 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
cd-3-2 = Overwritten
cd-3-3 = Newwy

[CommandListGanyuHeadkyrieRemapFix]
if $swapvar == 0
\tib = ResourceGanyuHeadIB.0
\tps-t1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex0
\tps-t0 = ResourceKyrieHeadDilutedDiffusekyrieRemapTex
\tps-t2 = ResourceKyrieHeadDilutedDiffusekyrieRemapTex
\tcd-1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex0
\tcd-3 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
\tcd-3-1 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
\tcd-3-2 = Overwritten
\tcd-3-3 = Newwy
\tcd-1-2 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex0
\tcd-1-1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex0
else if $swapvar == 1
\tib = ResourceGanyuHeadIB.1
\tps-t1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex1
\tps-t0 = ResourceKyrieHeadDilutedDiffusekyrieRemapTex
\tps-t2 = ResourceKyrieHeadDilutedDiffusekyrieRemapTex
\tcd-1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex1
\tcd-3 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
\tcd-3-1 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
\tcd-3-2 = Overwritten
\tcd-3-3 = Newwy
\tcd-1-2 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex1
\tcd-1-1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex1
else if $swapvar == 2
\tib = ResourceGanyuHeadIB.2
\tps-t1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex2
\tps-t0 = ResourceKyrieHeadDilutedDiffusekyrieRemapTex
\tps-t2 = ResourceKyrieHeadDilutedDiffusekyrieRemapTex
\tcd-1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex2
\tcd-3 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
\tcd-3-1 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
\tcd-3-2 = Overwritten
\tcd-3-3 = Newwy
\tcd-1-2 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex2
\tcd-1-1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex2
else if $swapvar == 3
\tib = ResourceGanyuHeadIB.3
\tps-t1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex3
\tps-t0 = ResourceKyrieHeadDilutedDiffusekyrieRemapTex
\tps-t2 = ResourceKyrieHeadDilutedDiffusekyrieRemapTex
\tcd-1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex3
\tcd-3 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
\tcd-3-1 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
\tcd-3-2 = Overwritten
\tcd-3-3 = Newwy
\tcd-1-2 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex3
\tcd-1-1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex3
endif


[ResourceGanyukyrieRemapBlend.0]
type = Buffer
stride = 32
filename = GanyuSummer1CanonBody/GanyukyrieRemapBlend.buf

[ResourceGanyukyrieRemapBlend.1]
type = Buffer
stride = 32
filename = GanyuSummer2CanonBodyNoSkirt/GanyukyrieRemapBlend.buf

[ResourceGanyukyrieRemapBlend.2]
type = Buffer
stride = 32
filename = GanyuSummer3AlternateBody/GanyukyrieRemapBlend.buf

[ResourceGanyukyrieRemapBlend.3]
type = Buffer
stride = 32
filename = GanyuSummer4AlternateBodyNoSkirt/GanyukyrieRemapBlend.buf

[ResourceKyrieHeadDilutedDiffusekyrieRemapTex]
filename = ResourceKyrieHeadDilutedDiffusekyrieRemapTex.dds

[ResourceKyrieHeadSaturatedDiffusekyrieRemapTex]
filename = ResourceKyrieHeadSaturatedDiffusekyrieRemapTex.dds

[ResourceKyrieHeadOversaturatedDiffusekyrieRemapTex]
filename = ResourceKyrieHeadOversaturatedDiffusekyrieRemapTex.dds

[ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex]
filename = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex.dds

[ResourceKyrieHeadConcentratedDiffusekyrieRemapTex0]
filename = GanyuSummer1CanonBody/GanyuHeadDiffuseCopykyrieRemapTex.dds

[ResourceKyrieHeadConcentratedDiffusekyrieRemapTex1]
filename = GanyuSummer2CanonBodyNoSkirt/GanyuHeadDiffusekyrieRemapTex.dds

[ResourceKyrieHeadConcentratedDiffusekyrieRemapTex2]
filename = GanyuSummer3AlternateBody/GanyuHeadDiffusekyrieRemapTex.dds

[ResourceKyrieHeadConcentratedDiffusekyrieRemapTex3]
filename = GanyuSummer4AlternateBodyNoSkirt/GanyuHeadDiffusekyrieRemapTex.dds

; *****************
""": """[Bello]
byebye = banana

; ***** kyrie *****
"""}

        for testInput in tests:
            expected = tests[testInput]

            self._iniFile.fileTxt = testInput
            result = self._iniFile._removeFix()

            self.assertEqual(result, expected)
            self.assertEqual(self._iniFile.isFixed, False)

    # ====================================================================
    # ====================== removeFix ===================================

    @mock.patch("src.FixRaidenBoss2.IniFile._removeFix")
    def test_hasFilekeepBackups_fixRemovedWithBackups(self, m__removeFix):
        self.createIniFile()

        self._iniFile.removeFix(keepBackups = True, fixOnly = False)
        assert(self._file.endswith(".txt"))
        assert(self._file.startswith(FRB.FilePrefixes.BackupFilePrefix.value))
        m__removeFix.assert_called_once()


    @mock.patch("src.FixRaidenBoss2.IniFile._removeFix")
    def test_fixOnly_fixNotRemoved(self, m__removeFix):
        self.createIniFile()

        self._iniFile.removeFix(keepBackups = True, fixOnly = True)
        assert(self._file.endswith(".ini"))
        m__removeFix.assert_not_called()

        self._iniFile.removeFix(keepBackups = False, fixOnly = True)
        assert(self._file.endswith(".ini"))
        m__removeFix.assert_not_called()

    @mock.patch("src.FixRaidenBoss2.IniFile._removeFix")
    def test_noBackups_fixRemovedNoBackups(self, m__removeFix):
        self.createIniFile()

        self._iniFile.removeFix(keepBackups = False, fixOnly = False)
        assert(self._file.endswith(".ini"))
        m__removeFix.assert_called_once()

    #TODO: Add case for needing the get the Blend.buf files to remove

    # ====================================================================
    # ====================== _makeRemapModel =============================

    # TODO: Add tests for building a single RemapBlendModel


    # ====================================================================
    # ====================== _getSubCommands =============================

    def test_ifTemplateWithNoSubCommands_noSubCommandsFound(self):
        self.createIniFile()
        ifTemplate = FRB.IfTemplate(["hanzel and gretel", {"candy": "house"}])
        currentSubCommands = set()
        subCommands = {"boo"}
        subCommandLst = list(subCommands)
        repeats = 3

        for i in range(repeats):
            self._iniFile._getSubCommands(ifTemplate, currentSubCommands, subCommands, subCommandLst)
            self.compareSet(currentSubCommands, set())
            self.compareSet(subCommands, {"boo"})
            self.compareList(subCommandLst, ["boo"])

    def test_ifTemplateWithSubcommands_unVisitedSubCommandsFound(self):
        self.createIniFile()
        ifTemplate = FRB.IfTemplate([FRB.IfPredPart("hanzel and gretel", FRB.IfPredPartType.If), 
                                     FRB.IfContentPart({"candy": [(0, "house")], "run": [(1, " and never look back"), (2, "visited")]}, 0),
                                     FRB.IfContentPart({"run": [(0, "FEAR: forget everything and run")]}, 1),
                                     FRB.IfContentPart({"run": [(0, "visited2")]}, 0)])
        currentSubCommands = {"visited"}
        subCommands = {"visited2", "to the unknown"}
        subCommandLst = list(subCommands)
        repeats = 3

        self._iniFile._getSubCommands(ifTemplate, currentSubCommands, subCommands, subCommandLst)
        expectedSubCommands = {"visited2", "to the unknown", " and never look back", "visited", "FEAR: forget everything and run"}
        expectedCurrentSubCommands = {" and never look back", "visited", "FEAR: forget everything and run"}

        self.compareSet(currentSubCommands, expectedCurrentSubCommands)
        self.compareSet(subCommands, expectedSubCommands)

        for i in range(repeats):
            self._iniFile._getSubCommands(ifTemplate, currentSubCommands, subCommands, subCommandLst)
            self.compareSet(currentSubCommands, expectedCurrentSubCommands)
            self.compareSet(subCommands, expectedSubCommands)

    # ====================================================================
    # ====================== _getCommandIfTemplate =======================

    def test_sectionIfTemplateSectionNotParsed_noIfTemplateFound(self):
        self.createIniFile()

        self._iniFile.sectionIfTemplates = {"this is this": FRB.IfTemplate([]), "that is that": FRB.IfTemplate([])}
        result = self._iniFile._getCommandIfTemplate("gone angels", raiseException = False)
        self.assertIs(result, None)

        try:
            self._iniFile._getCommandIfTemplate("gone angels", raiseException = True)
        except Exception as e:
            result = e

        self.assertIsInstance(result, KeyError)

    def test_sectionIfTemplateSectionParsed_IfTemplateFound(self):
        self.createIniFile()
        expectedParts = ["angela"]
        repeats = 3

        self._iniFile.sectionIfTemplates = {"this is this": FRB.IfTemplate(["roland"]), "that is that": FRB.IfTemplate(expectedParts)}

        for i in range(repeats):
            result = self._iniFile._getCommandIfTemplate("that is that", raiseException = False)
            self.assertIsInstance(result, FRB.IfTemplate)
            self.compareIfTemplateParts(result.parts, expectedParts)

    # ====================================================================
    # ====================== getResources ================================

    def test_sectionIfTemplates_noBlendResources(self):
        self.createIniFile()
        blendResources = {"materia"}
        subCommands = {"cmder"}
        subCommandList = list(subCommands)
        self._iniFile.sectionIfTemplates = {"this is this": FRB.IfTemplate([]), "that is that": FRB.IfTemplate([])}

        resourceGraph = FRB.IniSectionGraph({"this is this"}, self._iniFile.sectionIfTemplates)
        self._iniFile.getResources(resourceGraph, lambda part: FRB.IniKeywords.Vb1.value in part, lambda part: part[FRB.IniKeywords.Vb1.value],
                                   lambda resource, part: blendResources.add(resource))
        self.compareSet(blendResources, {"materia"})
        self.compareSet(subCommands, {"cmder"})
        self.compareList(subCommandList, ["cmder"])

    def test_sectionIfTemplateSectionFound_blendResourcesFromDFS(self):
        self.createIniFile()
        blendResources = {"materia"}
        startSection = "triangleNodeStart"
        repeats = 3
        sectionIfTemplates = {startSection: FRB.IfTemplate([FRB.IfContentPart({"run": [(0, "triangleNode2"), (1, "triangleNode3")]}, 0)]), 
                            "triangleNode2": FRB.IfTemplate([FRB.IfContentPart({"vb1": [(0, "colour1")], "run": [(1, "triangleNode3")]}, 0),
                                                             FRB.IfContentPart({"run": [(0, "triangleNodeStart"), (1, "nodeInCutOfBridge")]}, 2)]), 
                            "triangleNode3": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, "triangleNodeStart"), (1, "triangleNode2")]}, 0),
                                                             FRB.IfContentPart({"run": [(0, "otherSideNode1")]}, 1)]),
                            "nodeInCutOfBridge": FRB.IfTemplate([FRB.IfContentPart({"vb1": [(0, "colour2")]}, 0), 
                                                                 FRB.IfContentPart({"vb1": [(1, "colour4")], "run": [(2, "triangleNode2")]}, 1)]),
                            "otherSideNode1": FRB.IfTemplate([FRB.IfContentPart({"vb1": [(0, "colour1")], "run": [(1, "triangleNode3"), (2, "otherSideNode2")]}, 0)]),
                            "otherSideNode2": FRB.IfTemplate([FRB.IfContentPart({"vb1": [(0, "colour3")], "run": [(1, "otherSideNode1")]}, 0)]),
                            "island": FRB.IfTemplate([FRB.IfContentPart({"vb1": [(0, "colour5")]}, 2)])} 
        
        resourceGraph = FRB.IniSectionGraph({startSection}, sectionIfTemplates)

        for i in range(repeats):
            self._iniFile.sectionIfTemplates = sectionIfTemplates
            self._iniFile.getResources(resourceGraph, lambda part: FRB.IniKeywords.Vb1.value in part, lambda part: set(map(lambda valData: valData[1], part[FRB.IniKeywords.Vb1.value])),
                                       lambda resource, part: blendResources.update(resource))
            self.compareSet(blendResources, {"materia", "colour1", "colour2", "colour3", "colour4"})
            self.compareSet(set(resourceGraph.sections.keys()), {"triangleNodeStart", "triangleNode2", "triangleNode3", "otherSideNode1", "otherSideNode2", "nodeInCutOfBridge"})

    # ====================================================================
    # ====================== _getCommands ================================

    def test_sectionIfTemplates_noSubCommandsFound(self):
        self.createIniFile()
        subCommands = {"cmder"}
        subCommandList = list(subCommands)
        self._iniFile.sectionIfTemplates = {"this is this": FRB.IfTemplate([]), "that is that": FRB.IfTemplate([])}

        self._iniFile._getCommands("this is this", subCommands, subCommandList)
        self.compareSet(subCommands, {"cmder", "this is this"})
        self.compareList(subCommandList, ["cmder", "this is this"])

        result = None
        try:
            self._iniFile._getCommands("aloha", subCommands, subCommandList)
        except Exception as e:
            result = e

        self.assertIsInstance(result, KeyError)

    def test_sectionIfTemplates_foundSubCommands(self):
        self.createIniFile()
        subCommands = {"cmder"}
        subCommandList = list(subCommands)
        startSection = "triangleNodeStart"
        repeats = 3
        sectionIfTemplates = {startSection: FRB.IfTemplate([FRB.IfContentPart({"run": [(0, "triangleNode2"), (1, "triangleNode3")]}, 0)]), 
                            "triangleNode2": FRB.IfTemplate([FRB.IfContentPart({"vb1": [(0, "colour1")], "run": [(1, "triangleNode3")]}, 0),
                                                             FRB.IfContentPart({"run": [(0, "triangleNodeStart"), (1, "nodeInCutOfBridge")]}, 2)]), 
                            "triangleNode3": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, "triangleNodeStart"), (1, "triangleNode2")]}, 0),
                                                             FRB.IfContentPart({"run": [(0, "otherSideNode1")]}, 1)]),
                            "nodeInCutOfBridge": FRB.IfTemplate([FRB.IfContentPart({"vb1": [(0, "colour2")]}, 0), 
                                                                 FRB.IfContentPart({"vb1": [(1, "colour4")], "run": [(2, "triangleNode2")]}, 1)]),
                            "otherSideNode1": FRB.IfTemplate([FRB.IfContentPart({"vb1": [(0, "colour1")], "run": [(1, "triangleNode3"), (2, "otherSideNode2")]}, 0)]),
                            "otherSideNode2": FRB.IfTemplate([FRB.IfContentPart({"vb1": [(0, "colour3")], "run": [(1, "otherSideNode1")]}, 0)]),
                            "island": FRB.IfTemplate([FRB.IfContentPart({"vb1": [(0, "colour5")]}, 2)])} 
        

        for i in range(repeats):
            self._iniFile.sectionIfTemplates = sectionIfTemplates
            self._iniFile._getCommands(startSection, subCommands, subCommandList)
            self.compareSet(subCommands, {"cmder", "triangleNodeStart", "triangleNode2", "triangleNode3", "otherSideNode1", "otherSideNode2", "nodeInCutOfBridge"})

        sectionIfTemplates["triangleNode3"].calledSubCommands[503] = [(-8, "Invalid Permissions")]
        subCommands = {"cmder"}
        subCommandList = list(subCommands)
        
        result = None
        try:
            self._iniFile._getCommands(startSection, subCommands, subCommandList)
        except Exception as e:
            result = e

        self.assertIsInstance(result, KeyError)

    # ====================================================================
    # ====================== parse =======================================

    @mock.patch("src.FixRaidenBoss2.BaseIniParser.parse")
    def test_noTextureOverrideRoot_notParsed(self, m_parse):
        self.setupIniTxt("")
        self.createIniFile()
        self._iniFile._textureOverrideBlendRoot = "nonExitentRoot"
        self._iniFile.parse()
        
        m_parse.assert_not_called()

    @mock.patch("src.FixRaidenBoss2.GIMIParser.parse")
    def test_textureOverrideRootFound_parsedDataFromIniTxt(self, m_parse):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()
        self._iniFile._iniParser = FRB.GIMIParser(self._iniFile)

        self._iniFile.parse()
        m_parse.assert_called_once()

    # ====================================================================
    # ====================== fix =========================================

    @mock.patch("src.FixRaidenBoss2.IniFile.getFixStr")
    @mock.patch("src.FixRaidenBoss2.IniFile.injectAddition")
    def test_validIniTxtNotParsed_iniNotFixed(self, m_injectAddition, m_getFixStr):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()
        self._iniFile.fix()

        self.assertEqual(self._iniFile.isFixed, False)

    @mock.patch("src.FixRaidenBoss2.IniFile.getFixStr")
    @mock.patch("src.FixRaidenBoss2.IniFile.injectAddition")
    def test_validIniTxtParsed_iniFixed(self, m_injectAddition, m_getFixStr):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()

        self._iniFile.parse()
        self._iniFile.fix()

        self.assertEqual(self._iniFile.isFixed, True)

    @mock.patch("src.FixRaidenBoss2.IniFile.injectAddition")
    def test_invalidIniTxt_iniNotFixed(self, m_injectAddition):
        self.setupIniTxt("""
[TextureOverridePaimonBlend]
hash = hashBrownsAreTastyAndCuckooBirdsInNestsLikeThemToo
""")
        self.createIniFile()
        self._iniFile.parse()
        self._iniFile.defaultModType = None

        result = None
        try:
            self._iniFile.fix()
        except Exception as e:
            result = e

        self.assertEqual(self._iniFile.isFixed, False)
        self.assertIsInstance(result, FRB.NoModType)

    # ====================================================================
import sys
import re
import os
import unittest.mock as mock
from .baseFileUnitTest import BaseFileUnitTest
from collections import OrderedDict
from typing import List, Dict, Union

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )')
import src.FixRaidenBoss2.FixRaidenBoss2 as FRB


class IniFileTest(BaseFileUnitTest):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._iniFile = None
        cls._file = "C:/SomeFolder/DataFiles/Vault/Mods/CuteLittleEi.ini"
        cls._modTypes = { FRB.ModTypes.Raiden.value, 
                          FRB.ModType("Bernkastel", re.compile(r"\[\s*LittleBlackNekoWitch\s*\]"), "kuroneko", aliases = ["Frederica Bernkastel", "Bern-chan", "Rika Furude", "Nipah!"]) }
        
        cls._defaultModType = FRB.ModType("Kyrie", re.compile(r"\[\s*AgnusDei\s*\]"), "Dies Irae")
        cls._defaultIniTxt = r"""
                    [Constants]
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

                    [TextureOverrideRaidenShogunRemapBlend]
                    run = CommandListRaidenShogunRemapBlend
                    handling = skip
                    draw = 21916,0
                    [RaidenPuppetCommandResource]
                    type = Buffer
                    stride = 32
                    filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

                    ; ------ some lines originally generated from the fix ---------

                    [ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
                    ; she drank the smoothie

                    type = Buffer
                    stride = 32

                    if $swapmain == 1
                        filename = M:\AnotherDrive\CuteLittleEiRemapBlend.buf

                    else
                        run = RaidenPuppetCommandResourceRemapBlend
                    endif

                    [ResourceRaidenShogunRemapBlend.0]
                    type = Buffer
                    stride = 32

                    ; where does this go?
                    filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

                    [RaidenPuppetCommandResourceRemapBlend]
                    type = Buffer
                    stride = 32

                    # for some reason, GIMI does not work as what you expect for this case
                    filename = Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf

                    ; --------------------------------------------------------------


                    ; --------------- Raiden Boss Fix -----------------
                    ; Raiden boss fixed by NK#1321 if you used it for fix your raiden pls give credit for "Nhok0169"
                    ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support

                    [TextureOverrideRaidenShogunRemapBlend]
                    run = CommandListRaidenShogunRemapBlend
                    handling = skip
                    draw = 21916,0

                    [CommandListRaidenShogunRemapBlend]

                    # main swap
                    if $swapmain == 0

                        # some other subvariable swap
                        if $swapvar == 0 && $swapvarn == 0
                            vb1 = ResourceRaidenShogunRemapBlend.0

                        ; ruin the smoothie
                        else
                            vb1 = ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie
                        endif

                    ; some boring swap
                    else if $swapmain == 1
                        run = SubSubTextureOverrideRemapBlend
                    endif

                    [SubSubTextureOverrideRemapBlend]
                    if $swapoffice == 0 && $swapglasses == 0
                        vb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRemapBlend
                    endif


                    [GIMINeedsResourcesToAllStartWithResourceRemapBlend]
                    type = Buffer
                    stride = 32
                    filename = ..\AAA\BBBB\CCCCCC\DDDDDRemapRemapBlend.buf

                    [ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
                    type = Buffer
                    stride = 32
                    if $swapmain == 1
                        filename = M:\AnotherDrive\CuteLittleEiRemapBlend.buf
                    else
                        run = RaidenPuppetCommandResourceRemapBlend
                    endif

                    [ResourceRaidenShogunRemapBlend.0]
                    type = Binaries
                    stride = 31
                    filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

                    [RaidenPuppetCommandResourceRemapBlend]
                    type = Buffer
                    stride = 32
                    filename = Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf


                    ; -------------------------------------------------"""

        cls._iniTxtLines = []
        cls.setupIniTxt(cls._defaultIniTxt)

    @classmethod
    def setupIniTxt(cls, newIniTxt: str):
        cls._iniTxt = newIniTxt
        cls._iniTxtLines = cls._iniTxt.split("\n")
        if (cls._iniTxt):
            fileLinesLen = len(cls._iniTxtLines)
            for i in range(fileLinesLen):
                if (i < fileLinesLen - 1):
                    cls._iniTxtLines[i] += "\n"
        else:
            cls._iniTxtLines = []

    def createIniFile(self):
        self._iniFile = FRB.IniFile(file = self._file, txt = self._iniTxt, modTypes = self._modTypes, defaultModType = self._defaultModType)

    def getOpenPatch(self):
        return self.patches["builtins.open"]
    
    def compareRemapBlendModel(self, model1: FRB.RemapBlendModel, model2: FRB.RemapBlendModel):
        self.assertEqual(model1.fixedBlendName, model2.fixedBlendName)
        self.assertEqual(model1.iniFolderPath, model2.iniFolderPath)
        self.compareDict(model1.fixedBlendPaths, model2.fixedBlendPaths)

        assert((model1.origBlendName is not None and model2.origBlendName is not None) or (model1.origBlendName is None and model2.origBlendName is None))
        if (model1.origBlendName is not None):
            self.assertEqual(model1.origBlendName, model2.origBlendName)

        assert((model1.origBlendPaths is not None and model2.origBlendPaths is not None) or (model1.origBlendPaths is None and model2.origBlendPaths is None))
        if (model1.origBlendPaths is not None):
            self.compareDict(model1.origBlendPaths, model2.origBlendPaths)
    
    def compareIfTemplateParts(self, resultParts: List[Union[str, Dict]], expectedParts: List[Union[str, Dict]]):
        resultLen = len(resultParts)
        self.assertEqual(resultLen, len(expectedParts))
        
        for i in range(resultLen):
            part = resultParts[i]
            expectedPart = expectedParts[i]

            self.assertIs(type(part), type(expectedPart))
            if (isinstance(part, str)):
                self.assertEqual(part, expectedPart)
            elif (isinstance(part, dict)):
                self.compareDict(part, expectedPart)

    def compareIfTemplate(self, result: FRB.IfTemplate, expected: FRB.IfTemplate):
        self.compareIfTemplateParts(result.parts, expected.parts)
        
        assert((result.calledSubCommands is None and expected.calledSubCommands is None) or (result.calledSubCommands is not None and expected.calledSubCommands is not None))
        if (result.calledSubCommands is not None):
            self.compareDict(result.calledSubCommands, expected.calledSubCommands)

    def compareDictIfTemplate(self, result: Dict[str, FRB.IfTemplate], expected: Dict[str, FRB.IfTemplate]):
        self.assertEqual(len(result), len(expected))
        for resultKey in result:
            self.assertIn(resultKey, expected)

            resultValue = result[resultKey]
            expectedValue = expected[resultKey]
            self.compareIfTemplate(resultValue, expectedValue)

    def disableFile(self, filePrefix = FRB.BackupFilePrefix):
        result = self._file.replace(".ini", ".txt")
        result = f"{filePrefix}{result}"
        self._file = result

    def setUp(self):
        super().setUp()
        self.maxDiff = None
        self.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.read", side_effect = lambda file, fileCode, postProcessor: self._iniTxtLines)
        self.patch("builtins.open", new_callable=mock.mock_open())
        self.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.disableFile", side_effect = lambda file, filePrefix = FRB.BackupFilePrefix: self.disableFile(filePrefix = filePrefix))


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

        self.compareDict(self._iniFile._sectionIfTemplates, {})
        self.compareDict(self._iniFile._resourceBlends, {})

        self.compareDict(self._iniFile._blendCommands, {})
        self.compareDict(self._iniFile._blendCommandsRemapNames, {})
        self.compareList(self._iniFile._blendCommandsTuples, [])

        self.compareDict(self._iniFile._resourceCommands, {})
        self.compareDict(self._iniFile._resourceCommandsRemapNames, {})
        self.compareList(self._iniFile._resourceCommandsTuples, [])

        self.compareDict(self._iniFile.remapBlendModelsDict, {})
        self.compareList(self._iniFile.remapBlendModels, [])

    def test_noFile_noSavedDataExceptForText(self):
        self._file = None
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()
        self._iniFile.parse()
        self._iniFile.fix()
        self._iniFile.clear()

        self.compareList(self._iniFile.fileLines, self._iniTxtLines)
        self.assertEqual(self._iniFile.fileTxt, self._iniTxt)
        self.assertEqual(self._iniFile.fileLinesRead, True)

        self.assertEqual(self._iniFile.isFixed, True)
        self.assertEqual(self._iniFile._textureOverrideBlendRoot, "TextureOverrideRaidenShogunBlend")
        self.assertEqual(self._iniFile._textureOverrideBlendSectionName, "TextureOverrideRaidenShogunBlend")

        self.assertIs(self._iniFile.type, None)
        self.assertEqual(self._iniFile.isModIni, False)

        self.compareDict(self._iniFile._sectionIfTemplates, {})
        self.compareDict(self._iniFile._resourceBlends, {})

        self.compareDict(self._iniFile._blendCommands, {})
        self.compareDict(self._iniFile._blendCommandsRemapNames, {})
        self.compareList(self._iniFile._blendCommandsTuples, [])

        self.compareDict(self._iniFile._resourceCommands, {})
        self.compareDict(self._iniFile._resourceCommandsRemapNames, {})
        self.compareList(self._iniFile._resourceCommandsTuples, [])

        self.compareDict(self._iniFile.remapBlendModelsDict, {})
        self.compareList(self._iniFile.remapBlendModels, [])

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

        self.compareDict(self._iniFile._sectionIfTemplates, {})
        self.compareDict(self._iniFile._resourceBlends, {})

        self.compareDict(self._iniFile._blendCommands, {})
        self.compareDict(self._iniFile._blendCommandsRemapNames, {})
        self.compareList(self._iniFile._blendCommandsTuples, [])

        self.compareDict(self._iniFile._resourceCommands, {})
        self.compareDict(self._iniFile._resourceCommandsRemapNames, {})
        self.compareList(self._iniFile._resourceCommandsTuples, [])

        self.compareDict(self._iniFile.remapBlendModelsDict, {})
        self.compareList(self._iniFile.remapBlendModels, [])

            
    # ====================================================================
    # ====================== read ========================================

    def test_noFile_savedTextOnInit(self):
        self.patches["src.FixRaidenBoss2.FixRaidenBoss2.FileService.read"].side_effect = lambda file, fileCode, postProcessor: self._iniTxt
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
        self.patches["src.FixRaidenBoss2.FixRaidenBoss2.FileService.read"].side_effect = lambda file, fileCode, postProcessor: self._iniTxt
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
            openPatch.assert_called_with(self._file, 'w', encoding = FRB.IniFileEncoding)
            openPatch.return_value.__enter__().write.assert_called_with(self._iniTxt)
            self.assertEqual(result, self._iniTxt)

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
        sectionDict = {"hp": "9999", "mp": "999", "atk": "99", "magic": "99", "spd": "99", "int": "", "str": "99", "global persist dex": "99", "weapon": "lightbringer", "relic": "hero ring", "job": "red mage", "lv": ""}

        result = self._iniFile._parseSection(sectionName, iniStr)
        self.compareDict(result, sectionDict)

        result = self._iniFile._parseSection(sectionName, iniStr, save = save)
        self.assertIn(sectionName, save)
        self.compareDict(result, save[sectionName])

        save = []
        result = self._iniFile._parseSection(sectionName, iniStr, save = save)
        self.compareList(save, [])
        self.compareDict(result, sectionDict)

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
        self.compareDict(result["ResourceRaidenShogunRemapBlend.0"], {"type": "Buffer", "stride": "32", "filename": r"..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf" })

        result = self._iniFile.getSectionOptions(searchStr, handleDuplicateFunc = lambda duplicates: duplicates[-1])
        self.assertEqual(len(result), 1)
        self.compareDict(result["ResourceRaidenShogunRemapBlend.0"], {"type": "Binaries", "stride": "31", "filename": r"..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf" })

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
                            ["if $swap == 0\n",
                             {"run": "subRoutine1"},
                             "else\n",
                             {"run": "subRoutine2"},
                             "endif\n"]],
                             
                             [[f"[{sectionName}]",
                               "run = hello\n",
                               "stop = haltingProblem"],
                              [{"run": "hello",
                                "stop": "haltingProblem"}]],
                                
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
                                [{"hp": "327500", "mp": "60000"},
                                 "if $form == Fire\n",
                                    "if $fun == 0\n",
                                        {"1stturn": "Firaga"},
                                    "else\n",
                                        {"1stturn": "Meteor"},
                                    "endif\n",
                                    "if $fun == 0",
                                        {"2ndturn": "Flare"},
                                    "else if $fun == 1\n",
                                        {"2ndturn": "Meltdown"},
                                    "else\n",
                                        {"2ndturn": "Flare Star"},
                                    "endif\n",
                                    "if $attakced = 1\n",
                                        {"counter": "Southern Cross"},
                                    "endif\n",
                                 "else if $form == Earth\n",
                                    {"1stturn": "Attack"},
                                    "if $fun == 0\n",
                                        {"2ndturn": "Attack"},
                                    "else\n",
                                        {"2ndturn": "Last Breath"},
                                    "endif\n",
                                    {"3rdturn": "Last Breath"},
                                 "endif",
                                 {},
                                 "if $timer >= 15\n",
                                    {"desperateattack1": "Heartless Angel", "desperateattack2": "Mind Blast"},
                                 "endif\n",
                                 {"finalattack": "Ultima"}]]]

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
                                f"$i = $i - 3\n",
                                f"run = {sectionName}_i-3\n",
                             f"endif"],
                             [f"if $i <= 0\n",
                                {"basecase": "done"},
                              f"else\n",
                                {"$i": "$i - 1",    
                                 "run": f"{sectionName}_i-1"},
                              f"endif"]],
                              
                              [[f"[{sectionName}]\n",
                                "if $noClosing == 1\n",
                                    "error = 1"],
                                ["if $noClosing == 1\n",
                                 {"error": "1"}]],
                                 
                              [[f"[{sectionName}]\n",
                                "if $badIni == 1\n",
                                    "efefefefefef\n",
                                "endif"],
                                ["if $badIni == 1\n",
                                    {},
                                "endif"]],

                              [["if $noSection == 1\n",
                                    "ohNo = 1\n",
                                "endif"],
                                [{"ohno": "1"},
                                 "endif"]], 
                                 
                              [[f"[{sectionName}]\n",
                                "if\n",
                                "missing = 1\n",
                                "elif $hasSomething == 1\n",
                                "result = yay",
                                "else $booboo == 1"
                                "endif"],
                                ["if\n",
                                 {"missing": "1", "elif $hassomething": "= 1", "result": "yay"},
                                 "else $booboo == 1"
                                 "endif"]]]
        
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
    # ====================== _compareResources ===========================

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
            result = self._iniFile._compareResources(tuples[0], tuples[1])
            self.assertEqual(result, tuples[2])

    # ====================================================================
    # ====================== disIni ======================================

    def test_hasFile_fileDisabled(self):
        self.createIniFile()
        self._iniFile.disIni()

        self.patches["src.FixRaidenBoss2.FixRaidenBoss2.FileService.disableFile"].assert_called_once()
        assert(self._file.endswith(".txt"))
        assert(self._file.startswith(FRB.BackupFilePrefix))

    def test_noFile_noFileDisabled(self):
        self._file = None
        self.createIniFile()
        self._iniFile.disIni()
        self.patches["src.FixRaidenBoss2.FixRaidenBoss2.FileService.disableFile"].assert_not_called()

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
    # ====================== getFixHeader ================================

    def test_noModType_defaultHeader(self):
        self.createIniFile()
        repeats = 3
        expectedHeader = FRB.Heading("GI Boss Fix", 15, "-")

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
            self._iniFile._type = modType[0]
            modTypeName = modType[1]
            if (modTypeName):
                modTypeName += " "

            for i in range(repeats):
                expectedHeader = FRB.Heading(f"{modTypeName}Boss Fix", 15, "-")
                result = self._iniFile.getFixHeader()
                self.assertEqual(result, f"; {expectedHeader.open()}")

    # ====================================================================
    # ====================== getFixFooter ================================

    def test_noModType_defaultFooter(self):
        self.createIniFile()
        expectedHeader = FRB.Heading("GI Boss Fix", 15, "-")
        repeats = 3

        for i in range(repeats):
            result =  self._iniFile.getFixFooter()
            self.assertEqual(result, f"\n\n; {expectedHeader.close()}")

    def test_hasModTypes_headerWithModTypeName(self):
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

            expectedHeader = FRB.Heading(f"{modTypeName}Boss Fix", 15, "-")

            for i in range(repeats):
                result = self._iniFile.getFixFooter()
                self.assertEqual(result, f"\n\n; {expectedHeader.close()}")

    # ====================================================================
    # ====================== getFixCredit ================================

    def test_noModType_defaultCredit(self):
        self.createIniFile()
        expected = self._iniFile.Credit.replace(self._iniFile.ModTypeBossNameReplaceStr, "Boss ").replace(self._iniFile.ModTypeNameReplaceStr, "")

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
            expected = self._iniFile.Credit.replace(self._iniFile.ModTypeBossNameReplaceStr, f"{modType[1]}Boss ").replace(self._iniFile.ModTypeNameReplaceStr, modType[2])

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
    # ====================== getRemapName ================================

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
            result = FRB.IniFile.getRemapName(input)
            self.assertEqual(result, names[input]) 

    # ====================================================================
    # ====================== getRemapResourceName ========================

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
            result = FRB.IniFile.getRemapResourceName(input)
            self.assertEqual(result, names[input]) 

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
    # ====================== _isIfTemplateSubCommand =====================

    def test_differentParts_wheterPartHasRun(self):
        self.createIniFile()
        parts = [[{"run": "hello"}, True],
                 [{}, False],
                 [{"Rraw": "vb1"}, False]]
        
        for part in parts:
            result = self._iniFile._isIfTemplateSubCommand(part[0])
            self.assertEqual(result, part[1]) 

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
    # ====================== _getIfTemplateSubCommand ====================

    def test_differentParts_getRunValue(self):
        self.createIniFile()
        parts = [[{"run": "hello"}, "hello"],
                 [{}, None],
                 [{"Run": "vb1"}, None]]
        
        for part in parts:
            expected  = part[1]
            result = None

            try:
                result = self._iniFile._getIfTemplateSubCommand(part[0])
            except KeyError:
                pass
            
            if (expected is None):
                self.assertIs(result, expected)
            else:
                self.assertEqual(result, part[1])    

    # ====================================================================
    # ====================== _fillTextureOverrideRemapBlend ==============

    def test_differentPartsHasDefaultType_filledPartForTextureOverride(self):
        self.createIniFile()

        parts = [[{}, "", {}, {}, []],
                 [{"Oryx and Crake": "8.0/10",
                   "Brave New World": "8.9/10",
                   "The Buried Giant": "8.5/10",
                   "Life of Pi": "8.8/10"}, "Book Rating: ", {}, {}, []],
                 [{"draw": "pictures",
                   "vb1": "video bar 1",
                   "run": "away",
                   "hash": "brown",
                   "handling": "the undead"}, "fill the blanks: ", {"away": "Away in a Manger"}, {"video bar 1": FRB.RemapBlendModel("some path", "Bose", {})},
                   ["draw = pictures",
                    "vb1 = Bose",
                    "run = Away in a Manger",
                    "hash = Dies Irae",
                    "handling = skip"]],
                 [{"draw": "pictures",
                   "paint": "the sky",
                   "run": "away",
                   "nowhere": "to hide",
                   "handling": "the undead"}, "fill the blanks: ", {"away": "Away in a Manger"}, {"video bar 1": FRB.RemapBlendModel("some path", "Bose", {})},
                   ["draw = pictures",
                    "run = Away in a Manger",
                    "handling = skip"]]]

        sectionName = "someSection"
        oldSectionName = "someOldSection"
        partIndex = 2

        for partObj in parts:
            linePrefix = partObj[1]
            self._iniFile._blendCommandsRemapNames = partObj[2]
            self._iniFile.remapBlendModelsDict = partObj[3]
            result = self._iniFile._fillTextureOverrideRemapBlend(sectionName, partObj[0], partIndex, linePrefix, oldSectionName)

            expectedLines = partObj[4]
            expectedLinesLen = len(expectedLines)
            expected = ""
            for i in range(expectedLinesLen):
                expected += f"{linePrefix}{expectedLines[i]}\n"

            self.assertEqual(result, expected)

    def test_differentPartsWithoutTypeNoDefaultType_noModTypeFound(self):
        self._defaultModType = None
        self.createIniFile()

        parts = [[{"draw": "pictures",
                   "vb1": "video bar 1",
                   "run": "away",
                   "hash": "brown",
                   "handling": "the undead"}, "fill the blanks: ", {"away": "Away in a Manger"}, {"video bar 1": FRB.RemapBlendModel("some path", "Bose", {})},
                   ["draw = pictures",
                    "vb1 = Bose",
                    "run = Away in a Manger",
                    "hash = Dies Irae",
                    "handling = skip"]]]

        sectionName = "someSection"
        oldSectionName = "someOldSection"
        partIndex = 2

        for partObj in parts:
            linePrefix = partObj[1]
            self._iniFile._blendCommandsRemapNames = partObj[2]
            self._iniFile.remapBlendModelsDict = partObj[3]

            result = None
            try:
                self._iniFile._fillTextureOverrideRemapBlend(sectionName, partObj[0], partIndex, linePrefix, oldSectionName)
            except Exception as e:
                result = e

            self.assertIsInstance(result, FRB.NoModType)

    # ====================================================================
    # ====================== _fillRemapResource ==========================

    def test_differentParts_filledPartForRemapResource(self):
        self.createIniFile()
        sectionName = "someSection"
        oldSectionName = "someOldSection"
        partIndex = 2   

        parts = [[{}, "", {}, {}, []],
                 [{"Oryx and Crake": "8.0/10",
                   "Brave New World": "8.9/10",
                   "The Buried Giant": "8.5/10",
                   "Life of Pi": "8.8/10"}, "Book Rating: ", {}, {}, []],
                 [{"type": "darkness",
                   "filename": "Inode",
                   "run": "away",
                   "stride": "and one big step for man kind"}, "fill the blanks: ", {"away": "Away in a Manger"}, 
                   {"Inode": FRB.RemapBlendModel("some path", "Double Indirect Block", {}),
                    oldSectionName: FRB.RemapBlendModel("another path", "some blend name", {1 : "forever lost one", 2: "forever lost two", 3: "forever lost three"})},
                   ["type = Buffer",
                    "filename = forever lost two",
                    "run = Away in a Manger",
                    "stride = 32"]],
                 [{"type": "darkness",
                   "draw": "pictures",
                   "run": "away",
                   "paint": "the sky"}, "fill the blanks: ", {"away": "Away in a Manger"}, {"Inode": FRB.RemapBlendModel("some path", "Double Indirect Block", {})},
                   ["type = Buffer",
                    "run = Away in a Manger"]]] 
        
        for partObj in parts:
            linePrefix = partObj[1]
            self._iniFile._resourceCommandsRemapNames = partObj[2]
            self._iniFile.remapBlendModelsDict = partObj[3]
            result = self._iniFile._fillRemapResource(sectionName, partObj[0], partIndex, linePrefix, oldSectionName)

            expectedLines = partObj[4]
            expectedLinesLen = len(expectedLines)
            expected = ""
            for i in range(expectedLinesLen):
                expected += f"{linePrefix}{expectedLines[i]}\n"

            self.assertEqual(result, expected)


    # ====================================================================
    # ====================== fillIfTemplate ==============================

    def test_differentIfTemplates_filledIfTemplates(self):
        self.createIniFile()
        fillFunc = lambda sectionName, part, partIndex, linePrefix, origSectionName: "".join([f"sectionName: {sectionName}\n", f"part: {part}\n", f"partIndex: {partIndex}\n", f"linePrefix: [{linePrefix}]\n", f"origSectionName: {origSectionName}\n"])

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
                                                           "sectionName: someSection",
                                                           "part: {'1stturn': 'heartless angel'}",
                                                           f"partIndex: {0}",
                                                           f"linePrefix: []",
                                                           "origSectionName: someSection",
                                                           "if $fun == 0",
                                                                f"sectionName: someSection",
                                                                "part: {'7thturn': 'Thundaga'}",
                                                                f"partIndex: {2}",
                                                                f"linePrefix: [\t]",
                                                                "origSectionName: someSection",
                                                           "else if $fun == 1",
                                                                f"sectionName: someSection",
                                                                "part: {'7thturn': 'Havoc Wing'}",
                                                                f"partIndex: {4}",
                                                                f"linePrefix: [\t]",
                                                                "origSectionName: someSection",
                                                           "else",
                                                                f"sectionName: someSection",
                                                                "part: {'7thturn': 'Nothing'}",
                                                                f"partIndex: {6}",
                                                                f"linePrefix: [\t]",
                                                                "origSectionName: someSection",
                                                            "endif",
                                                            "if $hp <= 32640",
                                                                f"sectionName: someSection",
                                                                "part: {'8thturn': 'The end draws near...', '9thturn': 'Forsaken'}",
                                                                f"partIndex: {9}",
                                                                f"linePrefix: [\t]",
                                                                "origSectionName: someSection",
                                                                "\tif $fun == 0",
                                                                    f"sectionName: someSection",
                                                                    "part: {'10thturn': 'Havoc Wing', '11thturn': 'Havoc Wing'}",
                                                                    f"partIndex: {11}",
                                                                    f"linePrefix: [\t\t]",
                                                                    "origSectionName: someSection",
                                                                "\telse if $fun == 1",
                                                                    f"sectionName: someSection",
                                                                    "part: {'10thturn': 'Trine', '11thturn': 'Havoc Wing'}",
                                                                    f"partIndex: {13}",
                                                                    f"linePrefix: [\t\t]",
                                                                    "origSectionName: someSection",
                                                                "\telse",
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
                                                           f"sectionName: someSection",
                                                            "part: {'OH MY': 'GAHHHH'}",
                                                            f"partIndex: {1}",  
                                                            f"linePrefix: [\t]",
                                                            "origSectionName: oldSection",
                                                            "I wish I were a bird."]]]

        for ifTemplateTest in ifTemplateTests:
            result = self._iniFile.fillIfTemplate(ifTemplateTest[0], ifTemplateTest[1], fillFunc, ifTemplateTest[2])
            expected = "\n".join(ifTemplateTest[3])
            self.assertEqual(result, expected)

    # ====================================================================
    # ====================== getFixStr ===================================

    def test_differentIniTxt_fixForIni(self):
        self.createIniFile()

        iniTxtTests = [[self._defaultIniTxt,
                   f"""; --------------- Raiden Boss Fix ---------------
; Raiden Boss fixed by NK#1321 if you used it for fix your Raiden mods pls give credit for "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support

PREFIX:


[TextureOverrideRaidenShogunRemapBlend]
run = CommandListRaidenShogunRemapBlend
handling = skip
draw = 21916,0

[CommandListRaidenShogunRemapBlend]
                    if $swapmain == 0
                        if $swapvar == 0 && $swapvarn == 0
                        \tvb1 = ResourceRaidenShogunRemapBlend.0
                        else
                        \tvb1 = ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie
                        endif
                    else if $swapmain == 1
                    \trun = SubSubTextureOverrideRemapBlend
                    endif

[SubSubTextureOverrideRemapBlend]
                    if $swapoffice == 0 && $swapglasses == 0
                    \tvb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRemapBlend
                    endif


[GIMINeedsResourcesToAllStartWithResourceRemapBlend]
type = Buffer
stride = 32
filename = ../AAA/BBBB/CCCCCC/DDDDDRemapRemapBlend.buf

[ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
type = Buffer
stride = 32
                    if $swapmain == 1
                    \tfilename = M:/AnotherDrive/CuteLittleEiRemapBlend.buf
                    else
                    \trun = RaidenPuppetCommandResourceRemapBlend
                    endif

[ResourceRaidenShogunRemapBlend.0]
type = Buffer
stride = 32
filename = ../../../../../../../../../2-BunnyRaidenShogun/RaidenShogunRemapBlend.buf

[RaidenPuppetCommandResourceRemapBlend]
type = Buffer
stride = 32
filename = Dont/Use/If/Statements/Or/SubCommands/In/Resource/SectionsRemapBlend.buf


; -----------------------------------------------"""],
["", f"""; --------------- GI Boss Fix ---------------
; Boss fixed by NK#1321 if you used it for fix your mods pls give credit for "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support

PREFIX:


; -------------------------------------------"""],
[f"""
[TextureOverrideRaidenBlend]
handling = skip
draw = 21916,0
""",
f"""; --------------- Raiden Boss Fix ---------------
; Raiden Boss fixed by NK#1321 if you used it for fix your Raiden mods pls give credit for "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support

PREFIX:


[TextureOverrideRaidenRemapBlend]
handling = skip
draw = 21916,0



; -----------------------------------------------"""]]
        
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
        assert(self._file.startswith(FRB.BackupFilePrefix))
        openPatch.assert_called_with(iniFile, 'w', encoding = FRB.IniFileEncoding)
        openPatch.return_value.__enter__().write.assert_called_with(expected)
        self.assertEqual(result, expected)

    def test_insertBeforeOriginalNoBackupNoFile_txtAddedBeforeFileTxtNoBackupFile(self):
        openPatch = self.getOpenPatch()
        self._file = None
        self.createIniFile()

        addition = "Addition\n\n\tSubtraction"
        result = self._iniFile.injectAddition(addition, keepBackup = True, fixOnly = True)
        expected = f"{addition}\n\n{self._iniTxt}"

        self.patches["src.FixRaidenBoss2.FixRaidenBoss2.FileService.disableFile"].assert_not_called()
        openPatch.assert_not_called()
        self.assertEqual(result, expected)

    def test_insertAfterOriginalNoBackup_txtAddedAfterFileTxtNoBackupFile(self):
        openPatch = self.getOpenPatch()
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()

        addition = "Addition\n\n\tSubtraction"
        result = self._iniFile.injectAddition(addition, beforeOriginal = False, keepBackup = True, fixOnly = False)
        expected = f"{self._iniTxt}\n{addition}"

        self.patches["src.FixRaidenBoss2.FixRaidenBoss2.FileService.disableFile"].assert_not_called()
        openPatch.assert_called_with(self._file, 'w', encoding = FRB.IniFileEncoding)
        openPatch.return_value.__enter__().write.assert_called_with(expected)
        self.assertEqual(result, expected)

        result = self._iniFile.injectAddition(addition, beforeOriginal = False, keepBackup = False, fixOnly = True)
        self.patches["src.FixRaidenBoss2.FixRaidenBoss2.FileService.disableFile"].assert_not_called()
        openPatch.assert_called_with(self._file, 'w', encoding = FRB.IniFileEncoding)
        openPatch.return_value.__enter__().write.assert_called_with(expected)
        self.assertEqual(result, expected)

        result = self._iniFile.injectAddition(addition, beforeOriginal = False, keepBackup = False, fixOnly = False)
        self.patches["src.FixRaidenBoss2.FixRaidenBoss2.FileService.disableFile"].assert_not_called()
        openPatch.assert_called_with(self._file, 'w', encoding = FRB.IniFileEncoding)
        openPatch.return_value.__enter__().write.assert_called_with(expected)
        self.assertEqual(result, expected)

    # ====================================================================
    # ====================== _removeScriptFix ============================

    def test_scriptSections_sectionsRemoved(self):
        self.createIniFile()

        tests = {"Hello": "Hello",
                 "; --------------- Raiden Boss Fix ---------------\n\nFDFDFDFDF\n\n; -----------------------------------------------": "",
                 "; --------------- Raiden Boss Fix ---------------\n\nFDFDFDFDF\n\n; -------------------------------------": "; --------------- Raiden Boss Fix ---------------\n\nFDFDFDFDF\n\n; -------------------------------------"}

        for testInput in tests:
            expected = tests[testInput]

            self._iniFile.fileTxt = testInput
            result = self._iniFile._removeScriptFix()
            self.assertEqual(result, expected)
            self.assertEqual(self._iniFile.isFixed, False)
            self.compareList(self._iniFile.fileLines, [])
            self.assertEqual(self._iniFile.fileTxt, "")
            self.assertEqual(self._iniFile.fileLinesRead, False)

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

                    ; ------ some lines originally generated from the fix ---------\n\n"""}

        for testInput in tests:
            expected = tests[testInput]

            self._iniFile.fileTxt = testInput
            result = self._iniFile._removeFix()

            self.assertEqual(result, expected)
            self.assertEqual(self._iniFile.isFixed, False)

    # ====================================================================
    # ====================== removeFix ===================================

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.IniFile._removeFix")
    def test_hasFilekeepBackups_fixRemovedWithBackups(self, m__removeFix):
        self.createIniFile()

        self._iniFile.removeFix(keepBackups = True, fixOnly = False)
        assert(self._file.endswith(".txt"))
        assert(self._file.startswith(FRB.BackupFilePrefix))
        m__removeFix.assert_called_once()


    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.IniFile._removeFix")
    def test_fixOnly_fixNotRemoved(self, m__removeFix):
        self.createIniFile()

        self._iniFile.removeFix(keepBackups = True, fixOnly = True)
        assert(self._file.endswith(".ini"))
        m__removeFix.assert_not_called()

        self._iniFile.removeFix(keepBackups = False, fixOnly = True)
        assert(self._file.endswith(".ini"))
        m__removeFix.assert_not_called()

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.IniFile._removeFix")
    def test_noBackups_fixRemovedNoBackups(self, m__removeFix):
        self.createIniFile()

        self._iniFile.removeFix(keepBackups = False, fixOnly = False)
        assert(self._file.endswith(".ini"))
        m__removeFix.assert_called_once()

    # ====================================================================
    # ====================== _makeRemapModels ============================

    def test_differentSavedResourceIfTemplates_remapModelsCreated(self):
        self.createIniFile()

        testObjs = [[{}, []],
                    [{"hello": FRB.IfTemplate([])}, [FRB.RemapBlendModel("", "ResourcehelloRemapBlend", {}, origBlendName = "hello", origBlendPaths = {})]],
                    [{"Mahler": FRB.IfTemplate(["Bolero", {"filename": "./hello/world.haku"}, {"filename": "../../Backups/Buffers/Ei.elf"}, "dfdfdf", {"peepeepoopoo": "rip piggy"}]),
                      "Ravel": FRB.IfTemplate([{"Jeux D'eau": "Une piece difficile pour la piano"}]),
                      "Debussy": FRB.IfTemplate([{"Reverie": "Je reve d'etre ailleurs", "filename": "poopoopeepee/piggy rip"}])}, 
                      [FRB.RemapBlendModel("", "ResourceMahlerRemapBlend", {1: "hello/worldRemapBlend.buf", 2: "../../Backups/Buffers/EiRemapBlend.buf"}, origBlendName = "Mahler", origBlendPaths = {1: "hello/world.haku", 2: "../../Backups/Buffers/Ei.elf"}),
                       FRB.RemapBlendModel("", "ResourceRavelRemapBlend", {}, origBlendName = "Ravel", origBlendPaths = {}),
                       FRB.RemapBlendModel("", "ResourceDebussyRemapBlend", {0: "poopoopeepee/piggy ripRemapBlend.buf"}, origBlendName = "Debussy", origBlendPaths = {0: "poopoopeepee/piggy rip"})]]]
        
        for testObj in testObjs:
            self._iniFile.clear()
            self._iniFile._resourceCommands = testObj[0]
            self._iniFile._makeRemapModels()
            expected = testObj[1]
            expectedLen = len(expected)

            self.assertEqual(len(self._iniFile.remapBlendModels), expectedLen)
            for i in range(expectedLen):
                expected[i].iniFolderPath = os.path.dirname(self._file)
                self.compareRemapBlendModel(self._iniFile.remapBlendModels[i], expected[i])

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
        ifTemplate = FRB.IfTemplate(["hanzel and gretel", {"candy": "house"}],
                                    calledSubCommands = {1: " and never look back", 2: "visited", 3: "FEAR: forget everything and run", 4: "visited2"})
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

        self._iniFile._sectionIfTemplates = {"this is this": FRB.IfTemplate([]), "that is that": FRB.IfTemplate([])}
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

        self._iniFile._sectionIfTemplates = {"this is this": FRB.IfTemplate(["roland"]), "that is that": FRB.IfTemplate(expectedParts)}

        for i in range(repeats):
            result = self._iniFile._getCommandIfTemplate("that is that", raiseException = False)
            self.assertIsInstance(result, FRB.IfTemplate)
            self.compareIfTemplateParts(result.parts, expectedParts)

    # ====================================================================
    # ====================== _getBlendResources ==========================

    def test_sectionIfTemplates_noBlendResources(self):
        self.createIniFile()
        blendResources = {"materia"}
        subCommands = {"cmder"}
        subCommandList = list(subCommands)
        self._iniFile._sectionIfTemplates = {"this is this": FRB.IfTemplate([]), "that is that": FRB.IfTemplate([])}

        self._iniFile._getBlendResources("this is this", blendResources, subCommands, subCommandList)
        self.compareSet(blendResources, {"materia"})
        self.compareSet(subCommands, {"cmder"})
        self.compareList(subCommandList, ["cmder"])

        result = None
        try:
            self._iniFile._getBlendResources("aloha", blendResources, subCommands, subCommandList)
        except Exception as e:
            result = e

        self.assertIsInstance(result, KeyError)

    def test_sectionIfTemplateSectionFound_blendResourcesFromDFS(self):
        self.createIniFile()
        blendResources = {"materia"}
        subCommands = {"cmder"}
        subCommandList = list(subCommands)
        startSection = "triangleNodeStart"
        repeats = 3
        sectionIfTemplates = {startSection: FRB.IfTemplate([], calledSubCommands = OrderedDict([(3, "triangleNode2"), (5, "triangleNode3")])), 
                            "triangleNode2": FRB.IfTemplate([{"vb1": "colour1"}], calledSubCommands = OrderedDict([(2, "triangleNode3"), (3, "triangleNodeStart"), (4, "nodeInCutOfBridge")])), 
                            "triangleNode3": FRB.IfTemplate([], calledSubCommands =  OrderedDict([(1, "triangleNodeStart"), (2, "triangleNode2"), (3, "otherSideNode1")])),
                            "nodeInCutOfBridge": FRB.IfTemplate([{"vb1": "colour2"}, {"vb1": "colour4"}], calledSubCommands =  OrderedDict([(-100, "triangleNode2")])),
                            "otherSideNode1": FRB.IfTemplate([{"vb1": "colour1"}], calledSubCommands =  OrderedDict([(0, "triangleNode3"), (-90, "otherSideNode2")])),
                            "otherSideNode2": FRB.IfTemplate([{"vb1": "colour3"}], calledSubCommands = OrderedDict([(2, "otherSideNode1")])),
                            "island": FRB.IfTemplate([{"vb1": "colour5"}])} 
        

        for i in range(repeats):
            self._iniFile._sectionIfTemplates = sectionIfTemplates
            self._iniFile._getBlendResources(startSection, blendResources, subCommands, subCommandList)
            self.compareSet(blendResources, {"materia", "colour1", "colour2", "colour3", "colour4"})
            self.compareSet(subCommands, {"cmder", "triangleNodeStart", "triangleNode2", "triangleNode3", "otherSideNode1", "otherSideNode2", "nodeInCutOfBridge"})

        sectionIfTemplates["triangleNode3"].calledSubCommands[503] = "Invalid Permissions"
        blendResources = {"materia"}
        subCommands = {"cmder"}
        subCommandList = list(subCommands)
        
        result = None
        try:
            self._iniFile._getBlendResources(startSection, blendResources, subCommands, subCommandList)
        except Exception as e:
            result = e

        self.assertIsInstance(result, KeyError)

    # ====================================================================
    # ====================== _getCommands ================================

    def test_sectionIfTemplates_noSubCommandsFound(self):
        self.createIniFile()
        subCommands = {"cmder"}
        subCommandList = list(subCommands)
        self._iniFile._sectionIfTemplates = {"this is this": FRB.IfTemplate([]), "that is that": FRB.IfTemplate([])}

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
        sectionIfTemplates = {startSection: FRB.IfTemplate([], calledSubCommands = OrderedDict([(3, "triangleNode2"), (5, "triangleNode3")])), 
                            "triangleNode2": FRB.IfTemplate([{"vb1": "colour1"}], calledSubCommands = OrderedDict([(2, "triangleNode3"), (3, "triangleNodeStart"), (4, "nodeInCutOfBridge")])), 
                            "triangleNode3": FRB.IfTemplate([], calledSubCommands =  OrderedDict([(1, "triangleNodeStart"), (2, "triangleNode2"), (3, "otherSideNode1")])),
                            "nodeInCutOfBridge": FRB.IfTemplate([{"vb1": "colour2"}, {"vb1": "colour4"}], calledSubCommands =  OrderedDict([(-100, "triangleNode2")])),
                            "otherSideNode1": FRB.IfTemplate([{"vb1": "colour1"}], calledSubCommands =  OrderedDict([(0, "triangleNode3"), (-90, "otherSideNode2")])),
                            "otherSideNode2": FRB.IfTemplate([{"vb1": "colour3"}], calledSubCommands = OrderedDict([(2, "otherSideNode1")])),
                            "island": FRB.IfTemplate([{"vb1": "colour5"}])} 
        

        for i in range(repeats):
            self._iniFile._sectionIfTemplates = sectionIfTemplates
            self._iniFile._getCommands(startSection, subCommands, subCommandList)
            self.compareSet(subCommands, {"cmder", "triangleNodeStart", "triangleNode2", "triangleNode3", "otherSideNode1", "otherSideNode2", "nodeInCutOfBridge"})

        sectionIfTemplates["triangleNode3"].calledSubCommands[503] = "Invalid Permissions"
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

    def test_noTextureOverrideRoot_notParsed(self):
        self.setupIniTxt("")
        self.createIniFile()
        self._iniFile._textureOverrideBlendRoot = "nonExitentRoot"
        self._iniFile.parse()
        
        self.compareDict(self._iniFile._blendCommands, {})
        self.compareDict(self._iniFile._blendCommandsRemapNames, {})
        self.compareDict(self._iniFile._resourceCommands, {})
        self.compareDict(self._iniFile._resourceCommandsRemapNames, {})
        self.compareList(self._iniFile._blendCommandsTuples, [])
        self.compareList(self._iniFile._resourceCommandsTuples, [])

    def test_textureOverrideRootFound_parsedDataFromIniTxt(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()
        self._iniFile.parse()

        expectedBlendCommands = {"TextureOverrideRaidenShogunBlend": FRB.IfTemplate([{"run": "CommandListRaidenShogunBlend",
                                                                                      "handling": "skip",
                                                                                      "draw": "21916,0"}], {0: "CommandListRaidenShogunBlend"}),
                                 "CommandListRaidenShogunBlend": FRB.IfTemplate(["                    if $swapmain == 0\n",
                                                                                    "                        if $swapvar == 0 && $swapvarn == 0\n",
                                                                                        {"vb1": "ResourceRaidenShogunBlend.0"},
                                                                                    "                        else\n",
                                                                                        {"vb1": "ResourceEiBlendsHerBlenderInsteadOfHerSmoothie"},
                                                                                    "                        endif\n",
                                                                                 "                    else if $swapmain == 1\n",
                                                                                    {"run": "SubSubTextureOverride" },
                                                                                 "                    endif\n",
                                                                                 {}], {7: "SubSubTextureOverride"}),
                                 "SubSubTextureOverride": FRB.IfTemplate(["                    if $swapoffice == 0 && $swapglasses == 0\n",
                                                                            {"vb1": "GIMINeedsResourcesToAllStartWithResource"},
                                                                          "                    endif\n",
                                                                          {}])}
        expectedBlendRemapNames = {"TextureOverrideRaidenShogunBlend": "TextureOverrideRaidenShogunRemapBlend",
                                   "CommandListRaidenShogunBlend": "CommandListRaidenShogunRemapBlend",
                                   "SubSubTextureOverride": "SubSubTextureOverrideRemapBlend"}
        
        expectedResourceCommands = {"ResourceRaidenShogunBlend.0": FRB.IfTemplate([{"type": "Buffer",
                                                                                    "stride": "32",
                                                                                    "filename": "..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf"}]),
                                    "ResourceEiBlendsHerBlenderInsteadOfHerSmoothie": FRB.IfTemplate([{"type": "Buffer",
                                                                                                       "stride": "32"},
                                                                                                       "                    if $swapmain == 1\n",
                                                                                                            {"filename": "M:\AnotherDrive\CuteLittleEi.buf"},
                                                                                                       "                    else\n",
                                                                                                            {"run": "RaidenPuppetCommandResource"},
                                                                                                       "                    endif\n",
                                                                                                       {}], {4: "RaidenPuppetCommandResource"}),
                                    "GIMINeedsResourcesToAllStartWithResource": FRB.IfTemplate([{"type": "Buffer",
                                                                                                 "stride": "32",
                                                                                                 "filename": "./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf"}]),
                                    "RaidenPuppetCommandResource": FRB.IfTemplate([{"type": "Buffer",
                                                                                    "stride": "32",
                                                                                    "filename": "./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf"}])}
        expectedResourceCommandsRemapNames = {"ResourceRaidenShogunBlend.0": "ResourceRaidenShogunRemapBlend.0",
                                              "ResourceEiBlendsHerBlenderInsteadOfHerSmoothie": "ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie",
                                              "GIMINeedsResourcesToAllStartWithResource": "GIMINeedsResourcesToAllStartWithResourceRemapBlend",
                                              "RaidenPuppetCommandResource": "RaidenPuppetCommandResourceRemapBlend"}
        


        self.compareDictIfTemplate(self._iniFile._blendCommands, expectedBlendCommands)
        self.compareDict(self._iniFile._blendCommandsRemapNames, expectedBlendRemapNames)
        self.compareDictIfTemplate(self._iniFile._resourceCommands, expectedResourceCommands)
        self.compareDict(self._iniFile._resourceCommandsRemapNames, expectedResourceCommandsRemapNames)

        self._iniFile.fileTxt = ""
        self._iniFile.parse()

        self.compareDict(self._iniFile._blendCommands, {})
        self.compareDict(self._iniFile._blendCommandsRemapNames, {})
        self.compareDict(self._iniFile._resourceCommands, {})
        self.compareDict(self._iniFile._resourceCommandsRemapNames, {})
        self.compareList(self._iniFile._blendCommandsTuples, [])
        self.compareList(self._iniFile._resourceCommandsTuples, [])

    # ====================================================================
    # ====================== fix =========================================

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.IniFile.getFixStr")
    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.IniFile.injectAddition")
    def test_validIniTxt_iniFixed(self, m_injectAddition, m_getFixStr):
        self.setupIniTxt(self._defaultIniTxt)
        self.createIniFile()
        self._iniFile.fix()

        self.assertEqual(self._iniFile.isFixed, True)

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.IniFile.injectAddition")
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

        self.assertIsInstance(result, FRB.NoModType)
        self.assertEqual(self._iniFile.isFixed, False)

    # ====================================================================
import sys
import re
import unittest.mock as mock
from typing import List, Dict, Union

from .baseFileUnitTest import BaseFileUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class BaseIniFileTest(BaseFileUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._iniFile = None
        cls._file = "C:/SomeFolder/DataFiles/Vault/Mods/CuteLittleEi.ini"

        cls._customModTypes = {"rika": FRB.ModType("Bernkastel", re.compile(r"\[\s*LittleBlackNekoWitch\s*\]"), FRB.Hashes(), FRB.Indices(), aliases = ["Frederica Bernkastel", "Bern-chan", "Rika Furude", "Nipah!"]),
                               "kyrie": FRB.ModType("Kyrie", re.compile(r"\[\s*AgnusDei\s*\]"), FRB.Hashes(), FRB.Indices())}
        
        cls._setupCustomModTypes()
        cls._modTypes = { FRB.ModTypes.Raiden.value, 
                          cls._customModTypes["rika"] }
        
        cls._defaultModType = cls._customModTypes["kyrie"]
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

    def compareRemapBlendModel(self, model1: FRB.RemapBlendModel, model2: FRB.RemapBlendModel):
        self.assertEqual(model1.iniFolderPath, model2.iniFolderPath)
        self.compareDictOfDict(model1.fixedBlendPaths, model2.fixedBlendPaths)

        assert((model1.origBlendPaths is not None and model2.origBlendPaths is not None) or (model1.origBlendPaths is None and model2.origBlendPaths is None))
        if (model1.origBlendPaths is not None):
            self.compareDict(model1.origBlendPaths, model2.origBlendPaths)
    
    def compareIfTemplateParts(self, resultParts: List[Union[str, Dict]], expectedParts: List[Union[str, Dict]]):
        resultLen = len(resultParts)
        expectedLen = len(expectedParts)
        
        if (resultLen != expectedLen):
            self.fail(self.getDataFailMsg(resultParts, expectedParts, f"IfTemplates have different nunmber of parts: resultSet: {resultLen}, expectedSet: {expectedLen}"))
        
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

    def getOpenPatch(self):
        return self.patches["builtins.open"]

    def disableFile(self, filePrefix = FRB.FilePrefixes.BackupFilePrefix.value):
        result = self._file.replace(".ini", ".txt")
        result = f"{filePrefix}{result}"
        self._file = result

    def setUp(self):
        super().setUp()
        self.maxDiff = None
        self.patch("src.FixRaidenBoss2.FileService.read", side_effect = lambda file, fileCode, postProcessor: self._iniTxtLines)
        self.patch("builtins.open", new_callable=mock.mock_open())
        self.patch("src.FixRaidenBoss2.FileService.disableFile", side_effect = lambda file, filePrefix = FRB.FilePrefixes.BackupFilePrefix.value: self.disableFile(filePrefix = filePrefix))
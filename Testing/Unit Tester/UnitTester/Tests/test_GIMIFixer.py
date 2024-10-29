import sys

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class GIMIFixerTest(BaseIniFileTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._parser = None
        cls._fixer = None

    def createParser(self):
        self._parser = FRB.GIMIParser(self._iniFile)

    def createFixer(self):
        self._fixer = FRB.GIMIFixer(self._parser)

    def create(self):
        self.createIniFile()
        self.createParser()
        self.createFixer()
        self._iniFile._iniParser = self._parser

    # ====================== _fillTextureOverrideRemapBlend ==============

    def test_differentPartsHasDefaultType_filledPartForTextureOverride(self):
        self.create()
        modName = "kyrie"

        parts = [[{}, "", {}, {}, []],
                 [{"Oryx and Crake": "8.0/10",
                   "Brave New World": "8.9/10",
                   "The Buried Giant": "8.5/10",
                   "Life of Pi": "8.8/10"}, "Book Rating: ", {}, {}, []],
                 [{"draw": "pictures",
                   "vb1": "video bar 1",
                   "run": "away",
                   "hash": "gloria",
                   "handling": "the undead"}, "fill the blanks: ", {"away": {modName: "Away in a Manger"}}, {"video bar 1": FRB.RemapBlendModel("some path", {})},
                   ["draw = pictures",
                    "vb1 = video bar 1",
                    "run = Away in a Manger",
                    "hash = credo",
                    "handling = skip"]],
                 [{"draw": "pictures",
                   "paint": "the sky",
                   "run": "away",
                   "nowhere": "to hide",
                   "handling": "the undead"}, "fill the blanks: ", {"away": {modName: "Away in a Manger"}}, {"video bar 1": FRB.RemapBlendModel("some path", {})},
                   ["draw = pictures",
                    "run = Away in a Manger",
                    "handling = skip"]]]

        sectionName = "someSection"
        oldSectionName = "someOldSection"
        partIndex = 2

        for partObj in parts:
            linePrefix = partObj[1]
            self._parser.blendCommandsGraph._remapNames = partObj[2]
            self._iniFile.remapBlendModels = partObj[3]
            result = self._fixer._fillTextureOverrideRemapBlend(modName, sectionName, partObj[0], partIndex, linePrefix, oldSectionName)

            expectedLines = partObj[4]
            expectedLinesLen = len(expectedLines)
            expected = ""
            for i in range(expectedLinesLen):
                expected += f"{linePrefix}{expectedLines[i]}\n"

            self.assertEqual(result, expected)

    def test_differentPartsWithoutTypeNoDefaultType_noModTypeFound(self):
        self._defaultModType = None
        self.create()

        parts = [[{"draw": "pictures",
                   "vb1": "video bar 1",
                   "run": "away",
                   "hash": "brown",
                   "handling": "the undead"}, "fill the blanks: ", {"away": {"Christmas mod": "Away in a Manger"}}, {"video bar 1": FRB.RemapBlendModel("some path", {})},
                   ["draw = pictures",
                    "vb1 = Bose",
                    "run = Away in a Manger",
                    "hash = credo",
                    "handling = skip"]]]

        modName = "kyrie"
        sectionName = "someSection"
        oldSectionName = "someOldSection"
        partIndex = 2

        for partObj in parts:
            linePrefix = partObj[1]
            self._parser.blendCommandsGraph._remapNames = partObj[2]
            self._iniFile.remapBlendModels = partObj[3]

            result = None
            try:
                result = self._fixer._fillTextureOverrideRemapBlend(modName, sectionName, partObj[0], partIndex, linePrefix, oldSectionName)
            except Exception as e:
                result = e

            self.assertIsInstance(result, FRB.NoModType)

        # TODO: Add case for different ModNames

    # ====================================================================
    # ====================== _fillRemapResource ==========================

    def test_differentParts_filledPartForRemapResource(self):
        self.create()
        sectionName = "someSection"
        modName = "kyrie"
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
                   "stride": "and one big step for man kind"}, "fill the blanks: ", {"away": {modName: "Away in a Manger"}}, 
                   {"Inode": FRB.RemapBlendModel("some path", {}),
                    oldSectionName: FRB.RemapBlendModel("another path", {1 : {modName: "forever lost one"}, 2: {modName: "forever lost two"}, 3: {modName: "forever lost three"}})},
                   ["type = Buffer",
                    "filename = forever lost two",
                    "run = Away in a Manger",
                    "stride = 32"]],
                 [{"type": "darkness",
                   "draw": "pictures",
                   "run": "away",
                   "paint": "the sky"}, "fill the blanks: ", {"away": {modName: "Away in a Manger"}}, {"Inode": FRB.RemapBlendModel("some path", {})},
                   ["type = Buffer",
                    "run = Away in a Manger"]]] 
        
        for partObj in parts:
            linePrefix = partObj[1]
            self._parser.resourceCommandsGraph._remapNames = partObj[2]
            self._iniFile.remapBlendModels = partObj[3]
            result = self._fixer._fillRemapResource(modName, sectionName, partObj[0], partIndex, linePrefix, oldSectionName)

            expectedLines = partObj[4]
            expectedLinesLen = len(expectedLines)
            expected = ""
            for i in range(expectedLinesLen):
                expected += f"{linePrefix}{expectedLines[i]}\n"

            self.assertEqual(result, expected)


        # TODO: Add case for different ModNames


    # ====================================================================
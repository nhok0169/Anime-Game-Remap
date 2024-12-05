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

        parts = [[FRB.IfContentPart({}, 0), "", {}, {}, []],
                 [FRB.IfContentPart(
                     {"Oryx and Crake": [(1, "8.0/10")],
                      "Brave New World": [(0, "8.9/10")],
                      "The Buried Giant": [(2, "8.5/10")],
                      "Life of Pi": [(3, "8.8/10")]}, 2), "Book Rating: ", {}, {}, 
                      ["Brave New World = 8.9/10",
                       "Oryx and Crake = 8.0/10",
                       "The Buried Giant = 8.5/10",
                       "Life of Pi = 8.8/10"]],
                 [FRB.IfContentPart(
                     {"draw": [(4, "pictures")],
                      "vb1": [(3, "video bar 1")],
                      "run": [(2, "away")],
                      "hash": [(1, "gloria")],
                      "handling": [(0, "the undead")]}, 3), "fill the blanks: ", {"away": {modName: "Away in a Manger"}}, {"video bar 1": FRB.IniResourceModel("some path", {})},
                   ["handling = skip",
                    "hash = credo",
                    "run = Away in a Manger",
                    "vb1 = video bar 1",
                    "draw = pictures",]],
                 [FRB.IfContentPart(
                     {"draw": [(3, "pictures")],
                      "paint": [(2000000, "the sky")],
                      "run": [(-1.5, "away")],
                      "nowhere": [(0, "to hide")],
                      "handling": [(-100, "the undead")]}, 5), "fill the blanks: ", {"away": {modName: "Away in a Manger"}}, {"video bar 1": FRB.IniResourceModel("some path", {})},
                   ["handling = skip",
                    "run = Away in a Manger",
                    "nowhere = to hide",
                    "draw = pictures",
                    "paint = the sky"]]]

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

        parts = [[FRB.IfContentPart({"draw": [(0, "pictures")],
                   "vb1": [(2, "video bar 1")],
                   "run": [(1, "away")],
                   "hash": [(3, "brown")],
                   "handling": [(4, "the undead")]}, 0), "fill the blanks: ", {"away": {"Christmas mod": "Away in a Manger"}}, {"video bar 1": FRB.IniResourceModel("some path", {})},
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

        parts = [[FRB.IfContentPart({}, 0), "", {}, {}, []],
                 [FRB.IfContentPart(
                     {"Oryx and Crake": [(1, "8.0/10")],
                      "Brave New World": [(0.1, "8.9/10")],
                      "The Buried Giant": [(0, "8.5/10")],
                      "Life of Pi": [(-1, "8.8/10")]}, 3), "Book Rating: ", {}, {}, 
                      ["Life of Pi = 8.8/10",
                       "The Buried Giant = 8.5/10",
                       "Brave New World = 8.9/10",
                       "Oryx and Crake = 8.0/10"]],
                 [FRB.IfContentPart(
                     {"type": [(2, "darkness")],
                     "filename": [(4, "Inode")],
                     "run": [(8, "away")],
                     "stride": [(16, "and one big step for man kind")]}, -1), "fill the blanks: ", {"away": {modName: "Away in a Manger"}}, 
                   {"Inode": FRB.IniResourceModel("some path", {}),
                    oldSectionName: FRB.IniResourceModel("another path", {1 : {modName: ["forever lost one"]}, 2: {modName: ["forever lost two"]}, 3: {modName: ["forever lost three"]}})},
                   ["type = Buffer",
                    "filename = forever lost two",
                    "run = Away in a Manger",
                    "stride = 32"]],
                 [FRB.IfContentPart(
                     {"type": [(1, "darkness")],
                      "draw": [(2, "pictures")],
                      "run": [(0, "away")],
                      "paint": [(3, "the sky")]}, 0), "fill the blanks: ", {"away": {modName: "Away in a Manger"}}, {"Inode": FRB.IniResourceModel("some path", {})},
                   ["run = Away in a Manger",
                    "type = Buffer",
                    "draw = pictures",
                    "paint = the sky"]]] 
        
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
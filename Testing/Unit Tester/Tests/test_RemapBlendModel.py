import sys
from .baseFileUnitTest import BaseFileUnitTest
from typing import Dict

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )')
from src.FixRaidenBoss2 import FixRaidenBoss2 as FRB


class RemapBlendModelTest(BaseFileUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._iniFolderPath = ""
        cls._fixedBlendName = ""
        cls._fixedBlendPaths = {}
        cls._origBlendPaths = None
        cls._origBlendName = None
        cls.remapBlendModel = None

        cls.setDefaultAtts()

    @classmethod
    def setDefaultAtts(cls):
        cls._iniFolderPath = r"C:/totally/an/absolute/path"
        cls._fixedBlendName = "IAmRemapped"
        cls._fixedBlendPaths = {1: "hello",
                                3: "../../value/../remember/triangle identity/modulus.buf",
                                888: "D:/lucky/golden_frog_baccarat/eight.webAPI.Controller.house_edge",
                                -892: "Aria/by\John Cage/is/a/great\song/you/should/listen/to/it/everyday.mp3",
                                0: "Apostrophe/by\Pierre Schaeffer/is/also/another/great/listen.avi"}
        cls._origBlendPaths = {1: "PapaOutai.buf",
                               2: "macronutrient/alpha-linolenic acid.buf",
                               56: "M:/Dumb\Drive"}
        cls._origBlendName = "IAmNotRemapped"

    def getFullPaths(self, paths: Dict[int, str]) -> Dict[int, str]:
        result = {}
        for ind in paths:
            result[ind] = FRB.FileService.absPathOfRelPath(paths[ind], self._iniFolderPath)

        return result

    def setUp(self):
        super().setUp()

    def createRemapBlendModel(self):
        self.remapBlendModel = FRB.RemapBlendModel(self._iniFolderPath, self._fixedBlendName, self._fixedBlendPaths, 
                                                   origBlendName = self._origBlendName, origBlendPaths = self._origBlendPaths)


    # ========= __init__ =====================================

    def test_noOrigBlendData_remapBlendModelWithoutOrigBlendData(self):
        self._origBlendPaths = None
        self._origBlendName = None
        self.createRemapBlendModel()

        self.compareDict(self.remapBlendModel.fullPaths, self.getFullPaths(self._fixedBlendPaths))
        self.compareDict(self.remapBlendModel.origFullPaths, {})

    def test_origBlendData_remapBlendModelWithOrigBlendData(self):
        self.setDefaultAtts()
        self.createRemapBlendModel()

        self.compareDict(self.remapBlendModel.fullPaths, self.getFullPaths(self._fixedBlendPaths))
        self.compareDict(self.remapBlendModel.origFullPaths, self.getFullPaths(self._origBlendPaths))

    # ========================================================  

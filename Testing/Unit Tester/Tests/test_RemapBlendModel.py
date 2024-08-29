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
        cls._fixedBlendPaths = {}
        cls._origBlendPaths = None
        cls.remapBlendModel = None

        cls.setDefaultAtts()

    @classmethod
    def setDefaultAtts(cls):
        cls._iniFolderPath = r"C:/totally/an/absolute/path"
        cls._fixedBlendPaths = {1: {"Type1": "hello",
                                    "Type2": "bye"},
                                3: {"Calc1": "../../value/../remember/triangle identity/modulus.buf"},
                                888: {"Lucky": "D:/lucky/golden_frog_baccarat/eight.webAPI.Controller.house_edge"},
                                -892: {"ahhh": "Aria/by\John Cage/is/a/great\song/you/should/listen/to/it/everyday.mp3",
                                       "silence": "433/by\John Cage/is/another/great/song/toHear.m4a"},
                                0: {"Musique Concrete": "Apostrophe/by\Pierre Schaeffer/is/also/another/great/listen.avi"},
                                -897: {}}
        cls._origBlendPaths = {1: "PapaOutai.buf",
                               2: "macronutrient/alpha-linolenic acid.buf",
                               56: "M:/Dumb\Drive"}

    def getFullPaths(self, paths: Dict[int, Dict[str, str]]) -> Dict[int, Dict[str, str]]:
        result = {}
        for ind, partPaths in paths.items():
            try:
                result[ind]
            except:
                result[ind] = {}

            for modName, path in partPaths.items(): 
                result[ind][modName] = FRB.FileService.absPathOfRelPath(path, self._iniFolderPath)

        return result
    
    def getOrigFullPaths(self, paths: Dict[int, str]) -> Dict[int, str]:
        result = {}
        for ind in paths:
            result[ind] = FRB.FileService.absPathOfRelPath(paths[ind], self._iniFolderPath)

        return result

    def setUp(self):
        super().setUp()

    def createRemapBlendModel(self):
        self.remapBlendModel = FRB.RemapBlendModel(self._iniFolderPath, self._fixedBlendPaths, origBlendPaths = self._origBlendPaths)


    # ========= __init__ =====================================

    def test_noOrigBlendData_remapBlendModelWithoutOrigBlendData(self):
        self._origBlendPaths = None
        self.createRemapBlendModel()

        self.compareDictOfDict(self.remapBlendModel.fullPaths, self.getFullPaths(self._fixedBlendPaths))
        self.compareDict(self.remapBlendModel.origFullPaths, {})

    def test_origBlendData_remapBlendModelWithOrigBlendData(self):
        self.setDefaultAtts()
        self.createRemapBlendModel()

        self.compareDictOfDict(self.remapBlendModel.fullPaths, self.getFullPaths(self._fixedBlendPaths))
        self.compareDict(self.remapBlendModel.origFullPaths, self.getOrigFullPaths(self._origBlendPaths))

    # ========================================================  

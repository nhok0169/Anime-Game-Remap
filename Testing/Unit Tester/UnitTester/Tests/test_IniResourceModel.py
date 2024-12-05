import sys
from typing import Dict, List

from .baseFileUnitTest import BaseFileUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class IniResourceModelTest(BaseFileUnitTest):
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
        cls._fixedBlendPaths = {1: {"Type1": ["hello"],
                                    "Type2": ["bye"]},
                                3: {"Calc1": ["../../value/../remember/triangle identity/modulus.buf"]},
                                888: {"Lucky": ["D:/lucky/golden_frog_baccarat/eight.webAPI.Controller.house_edge"]},
                                -892: {"ahhh": ["Aria/by\John Cage/is/a/great\song/you/should/listen/to/it/everyday.mp3"],
                                       "silence": ["433/by\John Cage/is/another/great/song/toHear.m4a"]},
                                0: {"Musique Concrete": ["Apostrophe/by\Pierre Schaeffer/is/also/another/great/listen.avi"]},
                                -897: {}}
        cls._origBlendPaths = {1: ["PapaOutai.buf"],
                               2: ["macronutrient/alpha-linolenic acid.buf"],
                               56: ["M:/Dumb\Drive"]}

    def getFullPaths(self, paths: Dict[int, Dict[str, List[str]]]) -> Dict[int, Dict[str, str]]:
        result = {}
        for ind, partPaths in paths.items():
            try:
                result[ind]
            except:
                result[ind] = {}

            for modName, paths in partPaths.items(): 
                result[ind][modName] = list(map(lambda path: FRB.FileService.absPathOfRelPath(path, self._iniFolderPath), paths))

        return result
    
    def getOrigFullPaths(self, paths: Dict[int, List[str]]) -> Dict[int, str]:
        result = {}
        for ind in paths:
            result[ind] = list(map(lambda path: FRB.FileService.absPathOfRelPath(path, self._iniFolderPath), paths[ind]))

        return result

    def setUp(self):
        super().setUp()

    def createRemapBlendModel(self):
        self.remapBlendModel = FRB.IniResourceModel(self._iniFolderPath, self._fixedBlendPaths, origPaths = self._origBlendPaths)


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

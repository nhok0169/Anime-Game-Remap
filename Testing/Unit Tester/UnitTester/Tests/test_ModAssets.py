import sys
import unittest.mock as mock
from packaging.version import Version
from typing import Optional, Set, Dict

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ModAssetsTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._modAssets = None
        cls._indices = ["game", "gameVersion", "character", "characterVersion"]
        cls._versionIndices = {"gameVersion", "characterVersion"}

        cls._presetRepo = {"GI": {1.2: {"hutao": {"1.0.0a1": "hutao: 1",
                                                   2: "hutao: 2"}},
                                  "4.2.3": {"hutao": {4.5: "hutao: 3"}}},
                           "WUWA": {2.5: {"augusta": {6.7: "augusta: 1"}},
                                    3.0: {},
                                    4.3: {"sanhua": {}},
                                    5.6: {"sanhua": {"5.3.4b2": "sanhua: 1"}}},
                           "ZZZ": {}}
        
        cls._presetFixFrom = None
        cls._presetFixTo = None


    @classmethod
    def setupPresets(cls, presetFixFrom: Optional[Set[str]] = None, presetFixTo: Optional[Set[str]] = None):
        cls._presetFixFrom = presetFixFrom
        cls._presetFixTo = presetFixTo

    def compareToAssets(self, actualToAssets: Dict[float, Dict[str, str]], expectedToAssets: Dict[float, Dict[str, str]]):
        self.compareSet(set(actualToAssets.keys()), set(expectedToAssets.keys()))
        for version in actualToAssets:
            expectedVersionAsset = expectedToAssets[version]
            actualVersionAsset = actualToAssets[version]

            self.compareSet(set(actualVersionAsset.keys()), set(expectedVersionAsset.keys()))
            for assetName in actualVersionAsset:
                self.compareDict(actualVersionAsset[assetName], expectedVersionAsset[assetName])
    
    def createModAsset(self, setVersions: bool = True):
        self._modAssets = FRB.ModAssets(self._presetRepo, indices = self._indices, versionIndices = self._versionIndices, setVersions = setVersions)

    # =========================== get =====================================

    def test_differentSearchIndices_resultGotten(self):
        self.createModAsset()

        tests = [
                 [[], None, "sanhua: 1"],
                 [[], [Version("2.6")], "augusta: 1"],
                 [["WUWA"], [Version("2.6")], "augusta: 1"],
                 [["WUWA"], [], "sanhua: 1"],
                 [["Angrybirds"], [], KeyError()],
                 [["GI", "hutao"], [1.0], "hutao: 2"],
                 [["GI", "hutao"], [1.0, 0.1], "hutao: 1"],
                 [["GI", "hutao"], [], "hutao: 3"]]

        for test in tests:
            nonVersionIndices = test[0]
            versionIndices = test[1]

            expected = test[2]

            error = None
            result = None

            try:
                result = self._modAssets.get(nonVersionIndices, versionIndices)
            except Exception as e:
                error = e

            if (isinstance(expected, str)):
                self.assertEqual(result, expected)
            else:
                self.assertEqual(type(error), type(expected))

    # =====================================================================

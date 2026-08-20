import sys
from packaging.version import Version
from typing import Optional, Set, Dict

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ModDictAssetsTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._modAssets = None
        cls._indices = ["game", "gameVersion", "character", "characterVersion"]
        cls._versionIndex = "gameVersion"

        cls._presetRepo = {"GI": {1.2: {"hutao": {"1.0.0a1": "hutao: 1",
                                                   "2": "hutao: 2"}},
                                  "4.2.3": {"hutao": {"1.0.0a1": "hutao: 3"}}},
                           "WUWA": {2.5: {"augusta": {"6.7": "augusta: 1"}},
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
    
    def createModAsset(self):
        self._modAssets = FRB.ModDictAssetsOld(self._presetRepo, indices = self._indices, versionIndex = self._versionIndex)

    # =========================== get =====================================

    def test_differentSearchIndices_resultGotten(self):
        self.createModAsset()

        tests = [
                 [[], None, {}],
                 [[], Version("2.6"), {}],
                 [["WUWA"], Version("2.6"), {}],
                 [["WUWA", "augusta", "6.7"], Version("2.6"), "augusta: 1"],
                 [["Angrybirds"], None, KeyError()],
                 [["GI", "hutao"], 1.0, {}],
                 [["GI", "hutao", "1.0.0a1"], 1.0, "hutao: 1"],
                 [["GI", "hutao", "1.0.0a1"], None, "hutao: 3"]]

        for test in tests:
            nonVersionIndices = test[0]
            versionIndex = test[1]

            expected = test[2]

            error = None
            result = None

            try:
                result = self._modAssets.get(nonVersionIndices, versionIndex)
            except Exception as e:
                error = e

            if (isinstance(expected, str)):
                self.assertEqual(result, expected)
            elif (isinstance(expected, dict)):
                self.assertIsInstance(result, dict)
            else:
                self.assertEqual(type(error), type(expected))

    # =====================================================================

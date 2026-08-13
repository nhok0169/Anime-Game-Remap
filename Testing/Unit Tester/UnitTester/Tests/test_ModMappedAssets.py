import sys
from ordered_set import OrderedSet
from typing import Optional, Set, Dict

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ModMappedAssetsTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._modAssets = None
        cls._indices = ["game", "item"]

        cls._presetRepo = {1.2: {"Bob": {"YGO": {"card": "blueEyesWhiteDragon"},
                                         "Picnic": {"flower": "Obelisk"}},
                                 "Joe": {"YGO": {"card": "Obelisk"}}},
                           1.3: {"Joe": {"YGO": {"card": "Winged Dragon of Ra"}}},
                           "1.5.0a1": {"May": {"YGO": {"card": "Slifer"},
                                               "Picnic": {"flower": "marigold"} }},
                           "2.8.2": {"Alice": {"Picnic": {"flower": "poppy"},
                                               "Monument": {"temple": "pyramid"}}},
                           3: {"Joe": {"Picnic": {"flower": "rose"}}},
                           5.4: {"Sally": {"Monument": {"temple": "Obelisk"}}}}
        
        cls._map = {"Bob": OrderedSet(["Joe"]),
                    "Joe": OrderedSet(["May", "Alice"]),
                    "May": OrderedSet([]),
                    "Sally": OrderedSet(["Alice"])}


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
    
    def createModMappedAsset(self, setVersions: bool = True):
        repo = FRB.ModDictAssets(self._presetRepo, indices = [FRB.ModMappedAssets.VersionKey, FRB.ModMappedAssets.NameKey] + self._indices, versionIndex = FRB.ModMappedAssets.VersionKey)
        self._modAssets = FRB.ModMappedAssets(repo, map = self._map)

    # ======================= replace =====================================

    def test_differentSearchIndices_resultGotten(self):
        self.createModMappedAsset()

        tests = [
                 ["blueEyesWhiteDragon", None, [], None, None, {"Joe": "Winged Dragon of Ra"}],
                 ["blueEyesWhiteDragon", 1.2, [], None, 1.2, {"Joe": "Obelisk"}],
                 ["Obelisk", 7.0, ["Sally"], "Alice", 7.0, "pyramid"],
                 ["Obelisk", 1.0, {"game": "YGO"}, ["May"], 1.0, {"May": "Slifer"}],
                 ["Obelisk", 1.0, {"game": "YGO"}, None, 1.0, {"May": "Slifer"}],
                 ["Obelisk", 1.0, {"game": "YGO"}, "Alice", 1.0, None],
                 ["rose", 1.3, [], None, 1.3, {"May": "marigold", "Alice": "poppy"}]]

        for test in tests:
            asset = test[0]
            version = test[1]
            filters = test[2]
            toAssets = test[3]
            toVersion = test[4]

            expected = test[5]

            error = None
            result = None

            try:
                result = self._modAssets.replace(asset, fromVersion = version, fromNonVersionVals = filters, toAssetNames = toAssets, toVersion = toVersion)
            except Exception as e:
                error = e

            if (isinstance(expected, dict)):
                self.compareDict(result, expected)
            elif (isinstance(expected, Exception)):
                self.assertEqual(type(error), type(expected))
            else:
                self.assertEqual(result, expected)

    # =====================================================================
    # ======================= hasFrom =====================================

    def test_differentSearchIndices_resultInRepo(self):
        self.createModMappedAsset()

        tests = [
                 ["blueEyesWhiteDragon", None, [], True],
                 ["blueEyesWhiteDragon", None, ["Bob", "YGO", "card"], True],
                 ["blueEyesWhiteDragon", None, {"name": "Bob", "game": "YGO", "item": "card"}, True],
                 ["Obelisk", 7.0, ["Sally"], True],
                 ["Obelisk", 1.0, {"game": "YGO"}, True],
                 ["blueEyesWhiteDragon", 1.0, {"game": "Picnic"}, False],
                 ["rose", 1.3, [], True],
                 ["poopoopeepee", 1.3, [], False]]

        for test in tests:
            asset = test[0]
            version = test[1]
            filters = test[2]

            expected = test[3]

            result = self._modAssets.hasFrom(asset, version = version, nonVersionVals = filters)
            self.assertEqual(result, expected)

        FRB.CppIntTools.toBase64(35)

    # =====================================================================

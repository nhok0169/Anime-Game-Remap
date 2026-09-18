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
    # =========================== constructor =============================
    # ModAssets is the C++ ModAssets<std::string, py::object> now (the pure-Python
    # model/assets/ModAssets.py wrapper was folded into the binding once its last subclass was
    # removed), so its name-keyed constructor contract is reimplemented rather than inherited --
    # these pin it down.

    def test_defaultIndices_versionAndName(self):
        modAssets = FRB.ModAssets({"1.0": {"hutao": "a"}})

        self.assertEqual(modAssets.indices, [FRB.ModAssets.VersionKey, FRB.ModAssets.NameKey])
        self.compareSet(modAssets.versionIndices, {FRB.ModAssets.VersionKey})
        self.assertEqual(modAssets.get("hutao"), "a")

    def test_indicesAndVersionIndices_areExposed(self):
        self.createModAsset()

        self.assertEqual(self._modAssets.indices, self._indices)
        self.compareSet(self._modAssets.versionIndices, self._versionIndices)
        self.assertEqual(self._modAssets.totalIndices, 4)
        self.assertEqual(self._modAssets.versionColumnCount, 2)
        self.assertEqual(self._modAssets.nonVersionColumnCount, 2)

    def test_versionIndexNotAnIndex_isIgnored(self):
        # The pure-Python original intersected versionIndices with the real index names rather
        # than erroring, so a stray name is simply dropped.
        modAssets = FRB.ModAssets({"1.0": {"hutao": "a"}}, indices = ["version", "name"],
                                  versionIndices = {"version", "notAnIndex"})

        self.compareSet(modAssets.versionIndices, {"version"})

    def test_duplicateIndexNames_raisesKeyError(self):
        with self.assertRaises(KeyError):
            FRB.ModAssets({}, indices = ["name", "name"])

    def test_valueCol_defaultsAndRoundTrips(self):
        self.assertEqual(FRB.ModAssets({}).valueCol, FRB.ModAssets.ValueKey)
        self.assertEqual(FRB.ModAssets({}, valueCol = "count").valueCol, "count")

    def test_repoNestedTooShallow_raisesValueError(self):
        with self.assertRaises(ValueError):
            FRB.ModAssets({"1.0": "not a dict"}, indices = ["version", "name"])

    # =========================== get argument shapes =====================

    def test_get_dictAndListShapesAgree(self):
        self.createModAsset()

        byList = self._modAssets.get(["GI", "hutao"], [1.0, 0.1])
        byDict = self._modAssets.get({"game": "GI", "character": "hutao"},
                                     {"gameVersion": 1.0, "characterVersion": 0.1})

        self.assertEqual(byList, byDict)

    def test_getNotFound_returnsDefaultInsteadOfRaising(self):
        self.createModAsset()

        self.assertIsNone(self._modAssets.get(["Angrybirds"], [], errorOnNotFound = False))
        self.assertEqual(self._modAssets.get(["Angrybirds"], [], errorOnNotFound = False, default = "none"), "none")

    # =========================== addRows / copying =======================

    def test_addRows_thenGettable(self):
        self.createModAsset()

        self._modAssets.addRows({"GI": {"9.9": {"gregor samsa": {"9.9": "vermin"}}}})
        self.assertEqual(self._modAssets.get(["GI", "gregor samsa"], ["9.9", "9.9"]), "vermin")

    def test_deepCopy_isIndependent(self):
        import copy

        self.createModAsset()
        copied = copy.deepcopy(self._modAssets)

        copied.addRows({"GI": {"9.9": {"gregor samsa": {"9.9": "vermin"}}}})

        self.assertEqual(copied.get(["GI", "gregor samsa"], ["9.9", "9.9"]), "vermin")
        self.assertIsNone(self._modAssets.get(["GI", "gregor samsa"], [], errorOnNotFound = False))

    # =====================================================================

import sys
import unittest.mock as mock
from .baseUnitTest import BaseUnitTest
from typing import Optional, Set, Dict, List

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )/api')
from src.FixRaidenBoss2 import FixRaidenBoss2 as FRB


class ModDictAssetsTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._modAssets = None
        cls._presetRepo = {-9999999.0: {"Cecil": {"atk": "c: 10 atk", "hp": "c: 100 hp", "str": "c: 10 str"},
                                        "Kain": {"atk": "k: 15 atk", "hp": "k: 90 hp"}},
                            -5: {"Cecil": {"mp": "c: 50 mp", "atk": "c: 20 atk"}},
                            -15.34: {},
                            -20.56: {"Rydia": {}},
                            -3.2: {"Rydia": {"atk": "ry: 5 atk", "mag": "ry: 20 mag", "hp": "ry: 80 hp"},
                                   "Kain": {"hp": "k: 100 hp"}},
                            10.2: {"Fusoya": {"hp": "f: 100 hp", "mp": "f: 100 mp"}},
                            0: {"Rosa": {"hp": "ro: 120 hp", "atk": "ro: 8 atk", "mag": "ro: 15 mag"},
                                "Rydia": {"mp": "ry: 100 mp"},
                                "Edge": {"hp": "e: 100 hp", "atk": "e: 20 atk"},
                                "Kain": {"atk": "k: 25 atk"}},
                            10.25: {"Fusoya": {}},
                            300.2: {"Cecil": {"atk": "c: 99 atk"}}}
        
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
    
    def createModDictAsset(self):
        self._modAssets = FRB.ModDictAssets(self._presetRepo, presetFixFrom = self._presetFixFrom, presetFixTo = self._presetFixTo)


    # ========= clear ==========================================

    def test_differentSavedAssets_AllAssetInfoCleared(self):
        self.setupPresets({"Terra", "Kain", "Rosa"}, {"Vincent", "Cecil", "Rydia", "Rosa"})
        self.createModDictAsset()
        self._modAssets.loadFromPreset()

        self._modAssets.clear()
        self.compareDict(self._modAssets._fromAssets, {})
        self.compareDict(self._modAssets._toAssets, {})
        self.assertEqual(len(self._modAssets._versionCache), 0)
        self.compareDict(self._modAssets.latestVersions, {})
        self.compareDict(self._modAssets.versions, {})

        self._modAssets.clear()
        self.compareDict(self._modAssets._fromAssets, {})
        self.compareDict(self._modAssets._toAssets, {})
        self.assertEqual(len(self._modAssets._versionCache), 0)
        self.compareDict(self._modAssets.latestVersions, {})
        self.compareDict(self._modAssets.versions, {})

    # ===========================================================
    # ========= presetRepo.setter ===============================

    def test_differentPresetRepos_PresetReposSortedByVersion(self):
        self.setupPresets()
        self.createModDictAsset()
        tests = [[{}, []],
                 [self._presetRepo, [-9999999.0, -20.56, -15.34, -5, -3.2, 0, 10.2, 10.25, 300.2]],
                 [{1:{}, 2: {}, 3: {}, 4.3: {}, 4.8: {}}, [1, 2, 3, 4.3, 4.8]],
                 [{4.8: {}, 4.3: {}, 3: {}, 2: {}, 1: {}}, [1, 2, 3, 4.3, 4.8]],
                 [{2.3: {}, 2.3: {}, 2.3:{}, 3.5: {}, 1.2: {}}, [1.2, 2.3, 3.5]]]
        
        for test in tests:
            self._modAssets.presetRepo = test[0]
            self.compareList(list(self._modAssets.presetRepo.keys()), test[1])

    # ===========================================================
    # ========= presetFixFrom.setter ============================

    def test_differentPresetFixFromNames_NewPresetFixFromNamesSetAndLoaded(self):
        self.setupPresets()
        self.createModDictAsset()
        tests = [[{"Cecil"}, {}, False],
                 [set(), {}, False],
                 [{"Kain", "Golbez"}, {}, True],
                 [{"Rydia", "Rosa", "Edge"}, {"ry: 5 atk": ["Rydia", "atk"], "ry: 20 mag": ["Rydia", "mag"], "ry: 80 hp": ["Rydia", "hp"], "ry: 100 mp": ["Rydia", "mp"],
                                              "ro: 120 hp": ["Rosa", "hp"], "ro: 8 atk": ["Rosa", "atk"], "ro: 15 mag": ["Rosa", "mag"],
                                              "e: 100 hp": ["Edge", "hp"], "e: 20 atk": ["Edge", "atk"]}, False],
                 [{"Cecil"}, {"c: 10 atk": ["Cecil", "atk"], "c: 100 hp": ["Cecil", "hp"], "c: 10 str": ["Cecil", "str"], "c: 50 mp": ["Cecil", "mp"], "c: 20 atk": ["Cecil", "atk"], "c: 99 atk": ["Cecil", "atk"]}, False],
                 [{"Ramza"}, {}, False]]
        
        for test in tests:
            newPresetFixFrom = test[0]
            self._modAssets.presetFixFrom = newPresetFixFrom
            self.compareSet(self._modAssets.presetFixFrom, newPresetFixFrom)
            self.compareDict(self._modAssets.fromAssets, test[1])

            loadPredefinedFromAssets = test[2]
            if (loadPredefinedFromAssets):
                self._modAssets.loadPredefinedFromAssets()

    # ===========================================================
    # ========= presetFixTo.setter ==============================

    def test_differentPresetFixToNames_NewPresetFixToNamesSetAndLoaded(self):
        self.setupPresets()
        self.createModDictAsset()

        tests = [[{"Cecil"}, {}, {}, {}, False],
                 [set(), {}, {}, {}, False],
                 [{"Kain", "Golbez"}, {}, {}, {}, True],
                 [{"Rydia", "Rosa", "Edge"}, {-3.2: {"Rydia": {"atk": "ry: 5 atk", "mag": "ry: 20 mag", "hp": "ry: 80 hp"}},
                                              0: {"Rosa": {"hp": "ro: 120 hp", "atk": "ro: 8 atk", "mag": "ro: 15 mag"},
                                                  "Rydia": {"atk": "ry: 5 atk", "mag": "ry: 20 mag", "hp": "ry: 80 hp", "mp": "ry: 100 mp"},
                                                  "Edge": {"hp": "e: 100 hp", "atk": "e: 20 atk"}}}, 
                    {"Rydia": [-3.2, 0], "Rosa": [0], "Edge": [0]}, {"Rydia": 0, "Rosa": 0, "Edge": 0}, False],
                 [{"Cecil"}, {-9999999.0: {"Cecil": {"atk": "c: 10 atk", "hp": "c: 100 hp", "str": "c: 10 str"}},
                              -5: {"Cecil": {"atk": "c: 20 atk", "hp": "c: 100 hp", "str": "c: 10 str", "mp": "c: 50 mp"}},
                               300.2: {"Cecil": {"atk": "c: 99 atk", "hp": "c: 100 hp", "str": "c: 10 str", "mp": "c: 50 mp"}}}, 
                    {"Cecil": [-9999999.0, -5]}, {"Cecil": 300.2}, False],
                 [{"Ramza"}, {}, {}, {}, False]]

        for test in tests:
            newPresetFixTo = test[0]
            self._modAssets.presetFixTo = newPresetFixTo
            self.compareSet(self._modAssets.presetFixTo, newPresetFixTo)
            self.compareDict(self._modAssets.toAssets, test[1])

            self.compareDictList(self._modAssets.versions, test[2])
            self.compareDict(self._modAssets.latestVersions, test[3])

            loadPredefinedToAssets = test[4]
            if (loadPredefinedToAssets):
                self._modAssets.loadPredefinedToAssets()


    # ===========================================================
    # ========= loadPredefinedFromAssets ========================

    def test_differentFromAssetNames_FromAssetsLoadedFromPredefined(self):
        self.setupPresets()
        self.createModDictAsset()
        tests = [[{}, {}, None],
                 [{"Cecil"}, {"c: 10 atk": ["Cecil", "atk"], "c: 100 hp": ["Cecil", "hp"], "c: 10 str": ["Cecil", "str"], "c: 50 mp": ["Cecil", "mp"], "c: 20 atk": ["Cecil", "atk"], "c: 99 atk": ["Cecil", "atk"]}, -5],
                 [{"Rydia", "Rosa", "Edge"}, {"ry: 5 atk": ["Rydia", "atk"], "ry: 20 mag": ["Rydia", "mag"], "ry: 80 hp": ["Rydia", "hp"], "ry: 100 mp": ["Rydia", "mp"],
                                              "ro: 120 hp": ["Rosa", "hp"], "ro: 8 atk": ["Rosa", "atk"], "ro: 15 mag": ["Rosa", "mag"],
                                              "e: 100 hp": ["Edge", "hp"], "e: 20 atk": ["Edge", "atk"]}, 0],
                 [{"Ramza"}, {}],
                 [{"Kain", "Golbez"}, {"k: 15 atk": ["Kain", "atk"], "k: 90 hp": ["Kain", "hp"], "k: 100 hp": ["Kain", "hp"], "k: 25 atk": ["Kain", "atk"]}, 0]]

        for test in tests:
            self._modAssets.clear()
            self._modAssets._presetFixFrom = test[0]
            self._modAssets.loadPredefinedFromAssets()
            self.compareDictList(self._modAssets.fromAssets, test[1])

        self._modAssets.clear()
        self._modAssets._presetFixFrom = tests[1][0]
        self._modAssets.loadPredefinedFromAssets()
        self._modAssets._presetFixFrom = tests[2][0]
        self._modAssets.loadPredefinedFromAssets()
        self.compareDictList(self._modAssets.fromAssets, FRB.DictTools.combine(tests[1][1], tests[2][1]))

    # ===========================================================
    # ========= loadPredefinedToAssets ==========================

    def test_differentToAssetNames_ToAssetsLoadedFromPredefined(self):
        self.setupPresets()
        self.createModDictAsset()
        tests = [[{}, {}, {}, {}],
                 [{"Cecil"}, {-9999999.0: {"Cecil": {"atk": "c: 10 atk", "hp": "c: 100 hp", "str": "c: 10 str"}},
                              -5: {"Cecil": {"atk": "c: 20 atk", "hp": "c: 100 hp", "str": "c: 10 str", "mp": "c: 50 mp"}},
                              300.2: {"Cecil": {"atk": "c: 99 atk", "hp": "c: 100 hp", "str": "c: 10 str", "mp": "c: 50 mp"}}}, {"Cecil": [-9999999.0, -5, 300.2]}, {"Cecil": 300.2}],
                 [{"Rydia", "Rosa", "Edge"}, {-3.2: {"Rydia": {"atk": "ry: 5 atk", "mag": "ry: 20 mag", "hp": "ry: 80 hp"}},
                                              0: {"Rosa": {"hp": "ro: 120 hp", "atk": "ro: 8 atk", "mag": "ro: 15 mag"},
                                                  "Rydia": {"atk": "ry: 5 atk", "mag": "ry: 20 mag", "hp": "ry: 80 hp", "mp": "ry: 100 mp"},
                                                  "Edge": {"hp": "e: 100 hp", "atk": "e: 20 atk"}}}, 
                    {"Rydia": [-3.2, 0], "Rosa": [0], "Edge": [0]}, {"Rydia": 0, "Rosa": 0, "Edge": 0}],
                 [{"Ramza"}, {}, {}, {}],
                 [{"Kain", "Golbez"}, {-9999999.0: {"Kain": {"atk": "k: 15 atk", "hp": "k: 90 hp"}},
                                       -3.2: {"Kain": {"atk": "k: 15 atk", "hp": "k: 100 hp"}},
                                       0: {"Kain": {"atk": "k: 25 atk", "hp": "k: 100 hp"}}}, {"Kain": [-9999999.0, -3.2, 0]}, {"Kain": 0}]]
        
        for test in tests:
            self._modAssets.clear()
            self._modAssets._presetFixTo = test[0]
            self._modAssets.loadPredefinedToAssets()

            expectedAssets = test[1]
            actualAssets = self._modAssets._toAssets
            self.compareToAssets(actualAssets, expectedAssets)

            expectedVersions = test[2]
            self.compareDictList(self._modAssets.versions, expectedVersions)

            expectedLatestVersions = test[3]
            self.compareDict(self._modAssets.latestVersions, expectedLatestVersions)

    # ===========================================================
    # ========= loadFromPreset ==================================

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.ModDictAssets.clear")
    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.ModDictAssets.loadPredefinedFromAssets")
    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.ModDictAssets.loadPredefinedToAssets")
    def test_loadFromPreset_clearedPrevStatesAndLoadNewPresets(self, m_loadPredefinedToAssets, m_loadPredefinedFromAssets, m_clear):
        self.createModDictAsset() # 'loadFromPreset' is called in the constructor
        
        self._modAssets.loadFromPreset()
        self.assertEqual(m_clear.call_count, 2)
        self.assertEqual(m_loadPredefinedFromAssets.call_count, 2)
        self.assertEqual(m_loadPredefinedToAssets.call_count, 2)

    # ===========================================================
    # ========= addFrom =========================================

    def test_addDifferentFromAssets_FromAssetsAdded(self):
        self.setupPresets({"Terra", "Kain", "Rosa"}, {"Vincent", "Cecil", "Rydia", "Rosa"})
        self.createModDictAsset()

        origExpectedFromAssets = {"k: 15 atk": ["Kain", "atk"], "k: 90 hp": ["Kain", "hp"], "k: 100 hp": ["Kain", "hp"], "k: 25 atk": ["Kain", "atk"],
                                  "ro: 120 hp": ["Rosa", "hp"], "ro: 8 atk": ["Rosa", "atk"], "ro: 15 mag": ["Rosa", "mag"]}

        tests = [[{}, {}],
                 [{"Yang": {"atk": "ya 20 atk", "hp": "ya hp 30"}}, {"ya 20 atk": ["Yang", "atk"], "ya hp 30": ["Yang", "hp"]}],
                 [{"Rosa": {"rosa's hp": "ro: 120 hp", "white magic": "ro: 15 mag"}}, {"ro: 120 hp": ["Rosa", "rosa's hp"], "ro: 15 mag": ["Rosa", "white magic"]}],
                 [{"Kain": {"Redeemed Kain hp": "k: 90 hp", "Redeemed Kain atk": "k: 15 atk"},
                   "Rosa": {"Saved Rosa atk": "ro: 8 atk"}}, {"k: 90 hp": ["Kain", "Redeemed Kain hp"], "k: 15 atk": ["Kain", "Redeemed Kain atk"], "ro: 8 atk": ["Rosa", "Saved Rosa atk"]}]]
        
        for test in tests:
            newAssets = test[0]
            newFromAssets = test[1]

            self._modAssets.presetFixFrom = self._presetFixFrom
            self._modAssets.addFrom(newAssets)
            self.compareDictList(self._modAssets.fromAssets, FRB.DictTools.combine(origExpectedFromAssets, newFromAssets))

    # ===========================================================
    # ========= addTo ===========================================

    def test_addDifferentToAssets_toAssetsAdded(self):
        self.setupPresets({"Terra", "Kain", "Rosa"}, {"Vincent", "Cecil", "Rydia", "Rosa"})
        self.createModDictAsset()
        
        tests = [[3, {}, 
                  {-9999999.0: {"Cecil": {"atk": "c: 10 atk", "hp": "c: 100 hp", "str": "c: 10 str"}},
                   -5: {"Cecil": {"atk": "c: 20 atk", "hp": "c: 100 hp", "str": "c: 10 str", "mp": "c: 50 mp"}},
                   -3.2: {"Rydia": {"atk": "ry: 5 atk", "mag": "ry: 20 mag", "hp": "ry: 80 hp"}},
                   0: {"Rosa": {"hp": "ro: 120 hp", "atk": "ro: 8 atk", "mag": "ro: 15 mag"},
                       "Rydia": {"atk": "ry: 5 atk", "mag": "ry: 20 mag", "hp": "ry: 80 hp", "mp": "ry: 100 mp"}},
                   300.2: {"Cecil": {"atk": "c: 99 atk", "hp": "c: 100 hp", "str": "c: 10 str", "mp": "c: 50 mp"}}}, 
                    {"Cecil": [-9999999.0, -5], "Rydia": [-3.2, 0], "Rosa": [0]}, {"Cecil": 300.2, "Rydia": 0, "Rosa": 0}],

                 [3, {"Yang": {"atk": "ya 20 atk", "hp": "ya hp 30"}}, 
                  {-9999999.0: {"Cecil": {"atk": "c: 10 atk", "hp": "c: 100 hp", "str": "c: 10 str"}},
                   -5: {"Cecil": {"atk": "c: 20 atk", "hp": "c: 100 hp", "str": "c: 10 str", "mp": "c: 50 mp"}},
                   -3.2: {"Rydia": {"atk": "ry: 5 atk", "mag": "ry: 20 mag", "hp": "ry: 80 hp"}},
                   0: {"Rosa": {"hp": "ro: 120 hp", "atk": "ro: 8 atk", "mag": "ro: 15 mag"},
                       "Rydia": {"atk": "ry: 5 atk", "mag": "ry: 20 mag", "hp": "ry: 80 hp", "mp": "ry: 100 mp"}},
                   3: {"Yang": {"atk": "ya 20 atk", "hp": "ya hp 30"}},
                   300.2: {"Cecil": {"atk": "c: 99 atk", "hp": "c: 100 hp", "str": "c: 10 str", "mp": "c: 50 mp"}}},
                   {"Cecil": [-9999999.0, -5], "Rydia": [-3.2, 0], "Rosa": [0], "Yang": [3]}, {"Cecil": 300.2, "Rydia": 0, "Rosa": 0, "Yang": 3}],

                 [-3.2, {"Yang": {"atk": "ya 20 atk", "hp": "ya hp 30"}}, 
                  {-9999999.0: {"Cecil": {"atk": "c: 10 atk", "hp": "c: 100 hp", "str": "c: 10 str"}},
                   -5: {"Cecil": {"atk": "c: 20 atk", "hp": "c: 100 hp", "str": "c: 10 str", "mp": "c: 50 mp"}},
                   -3.2: {"Rydia": {"atk": "ry: 5 atk", "mag": "ry: 20 mag", "hp": "ry: 80 hp"},
                          "Yang": {"atk": "ya 20 atk", "hp": "ya hp 30"}},
                   0: {"Rosa": {"hp": "ro: 120 hp", "atk": "ro: 8 atk", "mag": "ro: 15 mag"},
                       "Rydia": {"atk": "ry: 5 atk", "mag": "ry: 20 mag", "hp": "ry: 80 hp", "mp": "ry: 100 mp"}},
                   300.2: {"Cecil": {"atk": "c: 99 atk", "hp": "c: 100 hp", "str": "c: 10 str", "mp": "c: 50 mp"}}},
                    {"Cecil": [-9999999.0, -5], "Rydia": [-3.2, 0], "Rosa": [0], "Yang": [-3.2]}, {"Cecil": 300.2, "Rydia": 0, "Rosa": 0, "Yang": -3.2}],

                 [0, {"Rosa": {"hp": "ro: 130 hp", "mag": "ro: 25 mag"}},
                  {-9999999.0: {"Cecil": {"atk": "c: 10 atk", "hp": "c: 100 hp", "str": "c: 10 str"}},
                   -5: {"Cecil": {"atk": "c: 20 atk", "hp": "c: 100 hp", "str": "c: 10 str", "mp": "c: 50 mp"}},
                   -3.2: {"Rydia": {"atk": "ry: 5 atk", "mag": "ry: 20 mag", "hp": "ry: 80 hp"}},
                   0: {"Rosa": {"hp": "ro: 130 hp", "atk": "ro: 8 atk", "mag": "ro: 25 mag"},
                       "Rydia": {"atk": "ry: 5 atk", "mag": "ry: 20 mag", "hp": "ry: 80 hp", "mp": "ry: 100 mp"}},
                   300.2: {"Cecil": {"atk": "c: 99 atk", "hp": "c: 100 hp", "str": "c: 10 str", "mp": "c: 50 mp"}}},
                    {"Cecil": [-9999999.0, -5], "Rydia": [-3.2, 0], "Rosa": [0]}, {"Cecil": 300.2, "Rydia": 0, "Rosa": 0}],
                       
                  [0, {"Rosa": {"mag": "ro: 100 mag", "mp": "ro: 500 mp"},
                       "Rydia": {"summon": "ry: 10 summons", "mp": "ry: 1000 mp", "mag": "ry: 999 mag"}},
                    {-9999999.0: {"Cecil": {"atk": "c: 10 atk", "hp": "c: 100 hp", "str": "c: 10 str"}},
                     -5: {"Cecil": {"atk": "c: 20 atk", "hp": "c: 100 hp", "str": "c: 10 str", "mp": "c: 50 mp"}},
                     -3.2: {"Rydia": {"atk": "ry: 5 atk", "mag": "ry: 20 mag", "hp": "ry: 80 hp"}},
                      0: {"Rosa": {"hp": "ro: 120 hp", "atk": "ro: 8 atk", "mag": "ro: 100 mag", "mp": "ro: 500 mp"},
                          "Rydia": {"atk": "ry: 5 atk", "mag": "ry: 999 mag", "hp": "ry: 80 hp", "mp": "ry: 1000 mp", "summon": "ry: 10 summons"}},
                      300.2: {"Cecil": {"atk": "c: 99 atk", "hp": "c: 100 hp", "str": "c: 10 str", "mp": "c: 50 mp"}}},
                    {"Cecil": [-9999999.0, -5], "Rydia": [-3.2, 0], "Rosa": [0]}, {"Cecil": 300.2, "Rydia": 0, "Rosa": 0}]]
        
        for test in tests:
            version = test[0]
            newAssets = test[1]

            self._modAssets.presetFixTo = self._presetFixTo
            self._modAssets.addTo(version, newAssets)

            expectedToAssets = test[2]
            self.compareToAssets(self._modAssets.toAssets, expectedToAssets)

            expectedVersions = test[3]
            self.compareDictList(self._modAssets.versions, expectedVersions)

            expectedLatestVersions = test[4]
            self.compareDict(self._modAssets.latestVersions, expectedLatestVersions)

    # ===========================================================
    # ========= _updateLatestVersion ============================

    def test_addDifferentVersions_latestVersionUpdated(self):
        self.setupPresets({"Terra", "Kain", "Rosa"}, {"Vincent", "Cecil", "Rydia", "Rosa"})
        self.createModDictAsset()

        tests = [["Cecil", -100, 300.2],
                 ["Rydia", 0, 0],
                 ["Rosa", 50.1, 50.1],
                 ["Vincent", 2.3, 2.3],
                 ["Zidane", 3.2, 3.2]]
        
        for test in tests:
            modName = test[0]
            modNewVersion = test[1]
            self._modAssets._updateLatestVersion(modName, modNewVersion)
            
            expectedVersion = test[2]
            self.assertEqual(self._modAssets.latestVersions[modName], expectedVersion)

    # ===========================================================
    # ========= _addVersion =====================================

    def test_addVersion_versionAdded(self):
        self.setupPresets({"Terra", "Kain", "Rosa"}, {"Vincent", "Cecil", "Rydia", "Rosa"})
        self.createModDictAsset()

        tests = [["Cecil", -100, 1],
                 ["Rydia", 0,-1],
                 ["Rosa", 50.1, -1],
                 ["Rydia", -100, 0],
                 ["Vincent", 2.3, -1],
                 ["Zidane", 3.2, -1]]
        
        for test in tests:
            modName = test[0]
            modNewVersion = test[1]
            self._modAssets._addVersion(modName, modNewVersion)

            expectedPos = test[2]
            self.assertEqual(self._modAssets.versions[modName][expectedPos], modNewVersion)

    # ===========================================================
    # ========= findClosestVersion ==============================

    def test_modWithDifferentVersions_closestVersionFound(self):
        self.setupPresets({"Terra", "Kain", "Rosa"}, {"Vincent", "Cecil", "Rydia", "Rosa"})
        self.createModDictAsset()

        tests = [["Cecil", -5, False, -5], 
                 ["Rydia", -1000, False, -3.2],
                 ["Rosa", 100, False, 0],
                 ["Vincent", 1.2, True],
                 ["Cecil", None, False, 300.2]]
        
        for test in tests:
            modName = test[0]
            modNewVersion = test[1]
            expectedError = test[2]
            
            error = None
            result = None
            try:
                result = self._modAssets.findClosestVersion(modName, modNewVersion)
            except Exception as e:
                error = e

            expectedError = test[2]
            if (expectedError):
                self.assertIsInstance(error, KeyError)
                continue
            
            expectedVersion = test[3]
            self.assertEqual(result, expectedVersion)

    # ===========================================================
    # ========= get =============================================

    def test_differentModsVersionAndTypes_desiredVersionExtracted(self):
        self.setupPresets({"Terra", "Kain", "Rosa"}, {"Vincent", "Cecil", "Rydia", "Rosa"})
        self.createModDictAsset()

        tests = [["Cecil", "atk", -50, False, "c: 10 atk"],
                 ["Cecil", "mp", -50, True],
                 ["Rydia", "mag", 500, False, "ry: 20 mag"],
                 ["Rydia", "blue magic", -30, True],
                 ["Vincent", "atk", 3.2, True],
                 ["Rydia", "hp", None, False, "ry: 80 hp"]]
        
        for test in tests:
            modName = test[0]
            modType = test[1]
            modVersion = test[2]

            error = None
            result = None

            try:
                result = self._modAssets.get(modName, modType, modVersion)
            except Exception as e:
                error = e

            expectedError = test[3]
            if (expectedError):
                self.assertIsInstance(error, KeyError)
                continue

            expectedMod = test[4]
            self.assertEqual(result, expectedMod)

    # ===========================================================
    # ========= replace =========================================

    def test_differentModsToReplace_replacementFound(self):
        self.setupPresets({"Terra", "Kain", "Rosa", "Cecil", "Rydia"}, {"Vincent", "Cecil", "Rydia", "Rosa", "Kain"})
        self.createModDictAsset()

        tests = [["c: 10 atk", 500, "c: 99 atk"],
                 ["k: 100 hp", -1000000000000000, "k: 90 hp"],
                 ["ry: 5 atk", -3, "ry: 5 atk"],
                 ["Hexaflare", 3.4, None],
                 ["c: 10 atk", None, "c: 99 atk"]]
        
        for test in tests:
            typeToReplace = test[0]
            modVersion = test[1]

            result = self._modAssets.replace(typeToReplace, modVersion)

            expectedReplacement = test[2]
            if (expectedReplacement is None):
                self.assertIs(result, None)
                continue

            self.assertEqual(result, expectedReplacement)

    # ===========================================================


import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ModMappedAssetsTest(BaseUnitTest):
    """
    Tests the C++ ``ModMappedAssets`` -- a :class:`ModDictAssets` plus the fix-from -> fix-to
    `adjacency list`_ that ``Hashes``/``Indices`` are built on

    :raw-html:`<br />`

    .. note::
        This file used to target the pure-Python ``ModMappedAssetsOld`` (now deleted); see
        test_ModDictAssets.py's own note for why these were rewritten against the real C++
        contract rather than retargeted line by line
    """

    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        # The repo's 3 index columns are version, name, type -- so 2 non-version values.
        cls._presetRepo = {"1.0": {"hutao": {"blend_vb": "hutao blend 1.0"}},
                           "2.5": {"hutao": {"blend_vb": "hutao blend 2.5"},
                                   "cherryHuTao": {"blend_vb": "cherry blend 2.5"}}}
        cls._map = {"hutao": ["cherryHuTao"]}

    def createModAssets(self, withNames: bool = True):
        repo = FRB.ModDictAssets.fromNestedDict(3, 0, self._presetRepo)

        if (withNames):
            return FRB.ModMappedAssets(repo, map = self._map, nonVersionIndexNames = ["name", "type"])

        return FRB.ModMappedAssets(repo, map = self._map)

    # =========================== construction ===================================

    def test_repoAndMap_areExposed(self):
        modAssets = self.createModAssets()

        self.assertEqual(len(modAssets.repo), 3)
        self.compareSet(set(modAssets.map.keys()), {"hutao"})
        self.assertEqual(list(modAssets.map["hutao"]), ["cherryHuTao"])

    def test_fixFromFixTo_alwaysEmpty(self):
        # Declared by the pure-Python original but never populated anywhere; the binding matches
        # that contract deliberately.
        modAssets = self.createModAssets()

        self.compareSet(modAssets.fixFrom, set())
        self.compareSet(modAssets.fixTo, set())

    def test_fromAssets_isEveryLeafValue(self):
        modAssets = self.createModAssets()

        self.compareSet(set(modAssets.fromAssets),
                        {"hutao blend 1.0", "hutao blend 2.5", "cherry blend 2.5"})

    # =========================== get ============================================

    def test_get_forwardsToTheRepo(self):
        modAssets = self.createModAssets()

        self.assertEqual(modAssets.get(["hutao", "blend_vb"], "1.0"), "hutao blend 1.0")
        self.assertEqual(modAssets.get(["hutao", "blend_vb"]), "hutao blend 2.5")

    # =========================== getKey / hasFrom ===============================

    def test_getKey_findsTheOriginatingKey(self):
        modAssets = self.createModAssets()
        key = modAssets.getKey("cherry blend 2.5", None, None)

        self.assertIsInstance(key, tuple)
        self.assertEqual(key, ("cherryHuTao", "blend_vb"))

    def test_getKey_sharedValue_filteredToTheOlderOwner_isFound(self):
        # A value two characters share, filed under the base at an OLD version and the skin at a NEWER one --
        # Yaoyao's face diffuse c70ae897 is also YaoyaoBamboo's, at 6.3. The newest bucket holds only the
        # skin's row, and a lookup filtered to the base used to stop there and find nothing, so her mods' face
        # section was never classified (white cheeks on the skin, 2026-09-28). It walks down to older buckets now.
        repo = FRB.ModDictAssets.fromNestedDict(3, 0, {"4.0": {"yaoyao": {"face": "shared"}},
                                                       "6.3": {"skin": {"face": "shared"}}})
        modAssets = FRB.ModMappedAssets(repo, map = {"yaoyao": ["skin"]})

        self.assertEqual(modAssets.getKey("shared", None, ["yaoyao", None], False), ("yaoyao", "face"))
        self.assertEqual(modAssets.getKey("shared", "6.3", ["yaoyao", None], False), ("yaoyao", "face"))
        # ...while an answer the newest bucket gives is unchanged, filtered or not
        self.assertEqual(modAssets.getKey("shared", None, ["skin", None], False), ("skin", "face"))
        self.assertEqual(modAssets.getKey("shared", None, None), ("skin", "face"))
        # ...and a filter nothing matches in ANY bucket still finds nothing
        self.assertIsNone(modAssets.getKey("shared", None, ["nobody", None], False))
        # ...and a version below every bucket's owner is not searched upwards
        self.assertIsNone(modAssets.getKey("shared", "3.0", ["skin", None], False))

    def test_getKeyMissing_raisesOrReturnsNone(self):
        modAssets = self.createModAssets()

        with self.assertRaises(KeyError):
            modAssets.getKey("not a real asset", None, None)

        self.assertIsNone(modAssets.getKey("not a real asset", None, None, errorOnNotFound = False))

    def test_hasFrom_knowsWhatIsInTheTable(self):
        modAssets = self.createModAssets()

        self.assertTrue(modAssets.hasFrom("hutao blend 2.5"))
        self.assertFalse(modAssets.hasFrom("not a real asset"))

    def test_nonVersionIndexNames_enableTheFlexibleArgumentShapes(self):
        named = self.createModAssets()

        # With the names supplied, a bare value and a name-keyed dict are accepted alongside the
        # plain positional list.
        self.assertEqual(named._convertNonVersionVals("hutao"), ["hutao", None])
        self.assertEqual(named._convertNonVersionVals({"type": "blend_vb"}), [None, "blend_vb"])
        self.assertEqual(named._convertNonVersionVals(None), [None, None])
        self.assertTrue(named.hasFrom("hutao blend 2.5", nonVersionVals = "hutao"))

    def test_withoutNonVersionIndexNames_staysStrictlyPositional(self):
        plain = self.createModAssets(withNames = False)

        self.assertIsNone(plain.nonVersionIndexNames)
        with self.assertRaises(ValueError):
            plain._convertNonVersionVals("hutao")

        # A positional list still works, which is the unnamed contract.
        self.assertTrue(plain.hasFrom("hutao blend 2.5", nonVersionVals = ["hutao", None]))

    # =========================== replace ========================================

    def test_replace_mapsFromOneAssetToItsTarget(self):
        modAssets = self.createModAssets()
        replaced = modAssets.replace("hutao blend 2.5", fromVersion = "2.5",
                                     fromNonVersionVals = ["hutao", None], toVersion = "2.5",
                                     toAssetName = "cherryHuTao")

        self.assertEqual(replaced, "cherry blend 2.5")

    def test_replaceAll_returnsEveryMappedTarget(self):
        modAssets = self.createModAssets()
        replaced = modAssets.replaceAll("hutao blend 2.5", fromVersion = "2.5",
                                        fromNonVersionVals = ["hutao", None], toVersion = "2.5")

        self.compareDict(replaced, {"cherryHuTao": "cherry blend 2.5"})

    # =========================== addMap / addRepoRows ===========================

    def test_addRepoRows_thenGettable(self):
        modAssets = self.createModAssets()

        modAssets.addRepoRows({"3.0": {"hutao": {"blend_vb": "hutao blend 3.0"}}})
        self.assertEqual(modAssets.get(["hutao", "blend_vb"], "3.0"), "hutao blend 3.0")

        # The reverse index is rebuilt too, so the new value is findable by key.
        self.assertEqual(modAssets.getKey("hutao blend 3.0", None, None), ("hutao", "blend_vb"))

    def test_addMap_mergesWithoutDuplicating(self):
        modAssets = self.createModAssets()

        modAssets.addMap({"hutao": ["cherryHuTao", "gregor samsa"]})
        self.assertEqual(list(modAssets.map["hutao"]), ["cherryHuTao", "gregor samsa"])

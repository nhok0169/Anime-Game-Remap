import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ModDictAssetsTest(BaseUnitTest):
    """
    Tests the C++ ``ModDictAssets`` -- the single-version-column, hash-indexed asset table

    :raw-html:`<br />`

    .. note::
        This file used to target the pure-Python ``ModDictAssetsOld`` (now deleted). It was
        deliberately *not* retargeted mechanically: the C++ class dropped that one's "a short key
        is a prefix query, returning the sub-dict" behaviour. Rows are flattened at construction
        and hashed on the *whole* non-version key, so a partial key isn't unimplemented, it's
        inexpressible. These tests pin the real contract instead
    """

    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        # 3 index columns -- version, name, type -- version first, the same shape Hashes uses.
        cls._totalIndices = 3
        cls._versionIndexPos = 0
        cls._presetRepo = {"1.0": {"hutao": {"blend_vb": "hutao: 1.0"}},
                           "2.5": {"hutao": {"blend_vb": "hutao: 2.5"},
                                   "augusta": {"blend_vb": "augusta: 2.5"}}}

    def createModAssets(self):
        return FRB.ModDictAssets.fromNestedDict(self._totalIndices, self._versionIndexPos, self._presetRepo)

    # =========================== construction ===================================

    def test_fromNestedDict_flattensEveryLeaf(self):
        modAssets = self.createModAssets()

        self.assertEqual(len(modAssets), 3)
        self.assertEqual(modAssets.totalIndices, self._totalIndices)
        self.assertEqual(modAssets.versionIndexPos, self._versionIndexPos)

    def test_constructor_takesAlreadyFlatRows(self):
        modAssets = FRB.ModDictAssets(3, 0, [(["1.0", "hutao", "blend_vb"], "hutao: 1.0")])

        self.assertEqual(len(modAssets), 1)
        self.assertEqual(modAssets.get(["hutao", "blend_vb"]), "hutao: 1.0")

    def test_wrongNestingDepth_raisesValueError(self):
        with self.assertRaises(ValueError):
            FRB.ModDictAssets.fromNestedDict(3, 0, {"1.0": {"hutao": "not deep enough"}})

    # =========================== get ============================================

    def test_get_completeKeyRequired(self):
        modAssets = self.createModAssets()

        self.assertEqual(modAssets.get(["hutao", "blend_vb"], "1.0"), "hutao: 1.0")

        # A short key is an error rather than a prefix query -- the flattening is why.
        with self.assertRaises(ValueError):
            modAssets.get(["hutao"], "1.0")

    def test_getNoVersion_usesLatestForThatKey(self):
        modAssets = self.createModAssets()

        self.assertEqual(modAssets.get(["hutao", "blend_vb"]), "hutao: 2.5")

    def test_getVersionBetweenRows_floorMatches(self):
        modAssets = self.createModAssets()

        self.assertEqual(modAssets.get(["hutao", "blend_vb"], "2.0"), "hutao: 1.0")

    def test_getVersionOlderThanEveryRow_returnsOldest(self):
        modAssets = self.createModAssets()

        # Floor-matching does not "miss" below a key's earliest version -- augusta's only row is
        # 2.5, and it still answers at 0.1.
        self.assertEqual(modAssets.get(["augusta", "blend_vb"], "0.1"), "augusta: 2.5")

    def test_getMissingKey_raisesOrReturnsNone(self):
        modAssets = self.createModAssets()

        with self.assertRaises(KeyError):
            modAssets.get(["nobody", "blend_vb"])

        self.assertIsNone(modAssets.get(["nobody", "blend_vb"], errorOnNotFound = False))

    # =========================== addRows / toNestedDict =========================

    def test_addRows_addsAndOverwrites(self):
        modAssets = self.createModAssets()

        modAssets.addRows({"3.0": {"hutao": {"blend_vb": "hutao: 3.0"}}})
        self.assertEqual(modAssets.get(["hutao", "blend_vb"], "3.0"), "hutao: 3.0")

        modAssets.addRows({"3.0": {"hutao": {"blend_vb": "hutao: 3.0 again"}}})
        self.assertEqual(modAssets.get(["hutao", "blend_vb"], "3.0"), "hutao: 3.0 again")
        self.assertEqual(len(modAssets), 4)

    def test_toNestedDict_roundTripsBackIntoAnEquivalentTable(self):
        modAssets = self.createModAssets()
        rebuilt = FRB.ModDictAssets.fromNestedDict(self._totalIndices, self._versionIndexPos,
                                                   modAssets.toNestedDict())

        self.assertEqual(len(rebuilt), len(modAssets))
        self.assertEqual(rebuilt.get(["augusta", "blend_vb"], "2.5"), "augusta: 2.5")
        self.assertEqual(rebuilt.get(["hutao", "blend_vb"]), "hutao: 2.5")

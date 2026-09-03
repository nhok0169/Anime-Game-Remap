import copy
import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class VertexCountsTest(BaseUnitTest):
    """
    Tests the real, production ``VertexCounts`` class -- the C++ core's
    ``ModDictAssets<std::string, int>``, pre-populated from ``VertexCountData``, which replaced the
    pure-Python ``model/assets/VertexCounts.py`` :raw-html:`<br />` :raw-html:`<br />`

    Same deliberate scope as test_Hashes.py/test_Indices.py: this covers the *binding's* contract
    (argument shapes, error/default handling, copy semantics), not the core lookup algorithm, which
    has its own standalone C++ coverage. Expected values are derived from the live table wherever
    possible rather than hardcoded, since the real data shifts with each game version.
    """

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._amber = FRB.ModTypeIdTools.getName(FRB.ModTypeId.Amber)

    # =========================== construction ===================================

    def test_defaultConstruction_isAlreadyPopulated(self):
        # The whole point of this class over a bare ModDictAssets: no repo argument, the shipped
        # data is baked in.
        vertexCounts = FRB.VertexCounts()
        self.assertGreater(len(vertexCounts), 0)

    def test_indexShape_versionNameComponent(self):
        vertexCounts = FRB.VertexCounts()

        # 3 columns (version, name, component), version first -- the C++ table deliberately has a
        # "component" column the pure-Python dict never had.
        self.assertEqual(vertexCounts.totalIndices, 3)
        self.assertEqual(vertexCounts.versionIndexPos, 0)

    # =========================== get ============================================

    def test_get_everyArgumentShapeAgrees(self):
        vertexCounts = FRB.VertexCounts()
        expected = vertexCounts.get(self._amber, versionVals = "4.0")

        self.assertIsInstance(expected, int)

        # A bare value, a short list, a full positional list and a name-keyed dict must all mean
        # the same query -- an unspecified column is filled in with "", not wildcarded, since
        # ModDictAssets hashes the whole key.
        self.assertEqual(vertexCounts.get([self._amber], versionVals = "4.0"), expected)
        self.assertEqual(vertexCounts.get([self._amber, ""], versionVals = ["4.0"]), expected)
        self.assertEqual(vertexCounts.get({"name": self._amber}, versionVals = {"version": "4.0"}), expected)
        self.assertEqual(vertexCounts.get({"name": self._amber, "component": ""}, versionVals = "4.0"), expected)

    def test_getNoVersion_usesLatestAvailable(self):
        vertexCounts = FRB.VertexCounts()

        # Leaving the version out resolves to the latest row for that key, which for a mod with a
        # single shipped row is that row.
        self.assertEqual(vertexCounts.get(self._amber), vertexCounts.get(self._amber, versionVals = "4.0"))

    def test_getVersionOlderThanEveryRow_stillAnswers(self):
        vertexCounts = FRB.VertexCounts()

        # Floor-matching does not "miss" below a key's earliest version -- it returns the oldest
        # row instead of nothing.
        self.assertEqual(vertexCounts.get(self._amber, versionVals = "0.1"),
                         vertexCounts.get(self._amber, versionVals = "4.0"))

    def test_getMissingKey_raisesOrReturnsDefault(self):
        vertexCounts = FRB.VertexCounts()

        with self.assertRaises(KeyError):
            vertexCounts.get("Not A Real Mod Type")

        self.assertIsNone(vertexCounts.get("Not A Real Mod Type", errorOnNotFound = False))
        self.assertEqual(vertexCounts.get("Not A Real Mod Type", errorOnNotFound = False, default = -1), -1)

    def test_get_returnsIntNotString(self):
        # Regression guard, matching test_CppModTypeMethods.py's own: this table's value type is
        # int, and a boundary that narrowed it to a string would silently hand back "10406".
        self.assertIsInstance(FRB.VertexCounts().get(self._amber), int)

    # =========================== addRows ========================================

    def test_addRows_nestedDictAndFlatListBothWork(self):
        vertexCounts = FRB.VertexCounts()

        vertexCounts.addRows({"9.9": {"gregor samsa": {"": 1234}}})
        self.assertEqual(vertexCounts.get("gregor samsa", versionVals = "9.9"), 1234)

        vertexCounts.addRows([(["9.9", "kyrie", ""], 4321)])
        self.assertEqual(vertexCounts.get("kyrie", versionVals = "9.9"), 4321)

    def test_addRows_existingKeyIsOverwritten(self):
        vertexCounts = FRB.VertexCounts()

        vertexCounts.addRows({"9.9": {"gregor samsa": {"": 1}}})
        vertexCounts.addRows({"9.9": {"gregor samsa": {"": 2}}})
        self.assertEqual(vertexCounts.get("gregor samsa", versionVals = "9.9"), 2)

    def test_addRows_wrongNestingDepthRaises(self):
        vertexCounts = FRB.VertexCounts()

        # 2 levels deep, but the table has 3 index columns -- the shape the *pure-Python*
        # VertexCountData still uses, which is exactly the mistake worth catching loudly.
        with self.assertRaises(ValueError):
            vertexCounts.addRows({"9.9": {"gregor samsa": 1234}})

    # =========================== copying ========================================

    def test_deepCopy_isIndependent(self):
        vertexCounts = FRB.VertexCounts()
        copied = copy.deepcopy(vertexCounts)

        copied.addRows({"9.9": {"gregor samsa": {"": 1234}}})

        self.assertEqual(copied.get("gregor samsa", versionVals = "9.9"), 1234)
        self.assertIsNone(vertexCounts.get("gregor samsa", errorOnNotFound = False))

    def test_clone_isIndependent(self):
        vertexCounts = FRB.VertexCounts()
        cloned = vertexCounts.clone()

        cloned.addRows({"9.9": {"gregor samsa": {"": 1234}}})

        self.assertEqual(cloned.get("gregor samsa", versionVals = "9.9"), 1234)
        self.assertIsNone(vertexCounts.get("gregor samsa", errorOnNotFound = False))

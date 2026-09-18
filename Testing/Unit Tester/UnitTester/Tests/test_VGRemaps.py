import copy
import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class VGRemapsTest(BaseUnitTest):
    """
    Tests the real, production ``VGRemaps`` class -- the C++ core's
    ``ModAssets<std::string, VGRemap>``, pre-populated from ``VGRemapData``, which replaced the
    pure-Python ``model/assets/VGRemaps.py`` (and, with it, that class's `pandas`_ ``DataFrame``
    dependency) :raw-html:`<br />` :raw-html:`<br />`

    Same deliberate scope as test_Hashes.py/test_Indices.py/test_VertexCounts.py: the *binding's*
    contract, not the core lookup algorithm. Note this is the one asset table with **two** version
    columns, which is why its core class is a linear-scanning ``ModAssets`` rather than a
    ``ModDictAssets`` -- and why an unspecified non-version column here is a real wildcard.
    """

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._amber = FRB.ModTypeIdTools.getName(FRB.ModTypeId.Amber)
        cls._amberCN = FRB.ModTypeIdTools.getName(FRB.ModTypeId.AmberCN)

    @classmethod
    def makeRows(cls, fromChar: str, toChar: str, remap, version: str = "9.9"):
        """
        Builds the 6-level nested dict ``addRows`` takes -- ``{fromVersion: {fromChar: {fromComp:
        {toVersion: {toChar: {toComp: remap}}}}}}``
        """

        return {version: {fromChar: {"": {version: {toChar: {"": remap}}}}}}

    # =========================== construction ===================================

    def test_defaultConstruction_isAlreadyPopulated(self):
        self.assertGreater(len(FRB.VGRemaps()), 0)

    def test_indexShape_sixColumnsTwoOfThemVersions(self):
        vgRemaps = FRB.VGRemaps()

        # fromVersion, fromChar, fromComp, toVersion, toChar, toComp
        self.assertEqual(vgRemaps.totalIndices, 6)
        self.assertEqual(vgRemaps.versionColumnCount, 2)
        self.assertEqual(vgRemaps.nonVersionColumnCount, 4)

    # =========================== get ============================================

    def test_get_returnsAVGRemap(self):
        vgRemaps = FRB.VGRemaps()
        remap = vgRemaps.get({"fromChar": self._amber, "toChar": self._amberCN},
                             {"fromVersion": "4.0", "toVersion": "4.0"})

        self.assertIsInstance(remap, FRB.VGRemap)

    def test_get_everyArgumentShapeAgrees(self):
        vgRemaps = FRB.VGRemaps()

        byDict = vgRemaps.get({"fromChar": self._amber, "toChar": self._amberCN},
                              {"fromVersion": "4.0", "toVersion": "4.0"})
        byList = vgRemaps.get([self._amber, "", self._amberCN, ""], ["4.0", "4.0"])

        # Same row either way -- compare the remap contents, since two lookups build two wrappers.
        self.assertEqual(byDict.remap, byList.remap)

    def test_getUnspecifiedNonVersionColumn_isAWildcard(self):
        vgRemaps = FRB.VGRemaps()

        # Unlike VertexCounts (hashed on the whole key, so a missing column is filled in with ""),
        # a missing column here really does match anything.
        self.assertIsInstance(vgRemaps.get({"fromChar": self._amber}), FRB.VGRemap)
        self.assertIsInstance(vgRemaps.get(self._amber), FRB.VGRemap)

    def test_getMissingKey_raisesOrReturnsDefault(self):
        vgRemaps = FRB.VGRemaps()

        with self.assertRaises(KeyError):
            vgRemaps.get({"fromChar": "Not A Real Mod Type"})

        self.assertIsNone(vgRemaps.get({"fromChar": "Not A Real Mod Type"}, errorOnNotFound = False))
        self.assertEqual(vgRemaps.get({"fromChar": "Not A Real Mod Type"}, errorOnNotFound = False, default = "none"),
                         "none")

    def test_getVersionOlderThanEveryRow_stillAnswers(self):
        vgRemaps = FRB.VGRemaps()
        nonVersionVals = {"fromChar": self._amber, "toChar": self._amberCN}

        # Floor-matching returns the oldest row rather than missing, on both version columns.
        self.assertEqual(vgRemaps.get(nonVersionVals, {"fromVersion": "0.1", "toVersion": "0.1"}).remap,
                         vgRemaps.get(nonVersionVals, {"fromVersion": "1.0", "toVersion": "4.0"}).remap)

    # =========================== addRows ========================================

    def test_addRows_acceptsBothAVGRemapAndAPlainDict(self):
        vgRemaps = FRB.VGRemaps()

        vgRemaps.addRows(self.makeRows("gregor samsa", "gregor samsa", {0: 7, 1: 6, 2: 5}))
        fromDict = vgRemaps.get({"fromChar": "gregor samsa", "toChar": "gregor samsa"})

        vgRemaps.addRows(self.makeRows("kyrie", "kyrie", FRB.VGRemap({0: 7, 1: 6, 2: 5})))
        fromRemap = vgRemaps.get({"fromChar": "kyrie", "toChar": "kyrie"})

        self.assertIsInstance(fromDict, FRB.VGRemap)
        self.assertIsInstance(fromRemap, FRB.VGRemap)
        self.assertEqual(fromDict.remap, fromRemap.remap)

    def test_addRows_flatListWorks(self):
        vgRemaps = FRB.VGRemaps()

        vgRemaps.addRows([(["9.9", "gregor samsa", "", "9.9", "kyrie", ""], {1: 2})])
        self.assertIsInstance(vgRemaps.get({"fromChar": "gregor samsa", "toChar": "kyrie"}), FRB.VGRemap)

    def test_addRows_badLeafRaises(self):
        vgRemaps = FRB.VGRemaps()

        with self.assertRaises(ValueError):
            vgRemaps.addRows(self.makeRows("gregor samsa", "kyrie", "not a remap"))

    def test_addRows_wrongNestingDepthRaises(self):
        vgRemaps = FRB.VGRemaps()

        # The pure-Python original's own 5-level shape (no fromComp/toComp), one level short.
        with self.assertRaises(ValueError):
            vgRemaps.addRows({"9.9": {"gregor samsa": {"9.9": {"kyrie": {0: 1}}}}})

    # =========================== copying ========================================

    def test_deepCopy_isIndependent(self):
        # The real reason this is bound: ModDataAssets.VGRemaps hands out a *shared* instance, so
        # test_Mod.py deep-copies it before mutating (see ModType's own note on that asymmetry).
        shared = FRB.ModDataAssets.VGRemaps.value
        copied = copy.deepcopy(shared)

        copied.addRows(self.makeRows("gregor samsa", "kyrie", {0: 1}))

        self.assertIsInstance(copied.get({"fromChar": "gregor samsa", "toChar": "kyrie"}), FRB.VGRemap)
        self.assertIsNone(shared.get({"fromChar": "gregor samsa"}, errorOnNotFound = False))

    def test_clone_isIndependent(self):
        vgRemaps = FRB.VGRemaps()
        cloned = vgRemaps.clone()

        cloned.addRows(self.makeRows("gregor samsa", "kyrie", {0: 1}))

        self.assertIsInstance(cloned.get({"fromChar": "gregor samsa", "toChar": "kyrie"}), FRB.VGRemap)
        self.assertIsNone(vgRemaps.get({"fromChar": "gregor samsa"}, errorOnNotFound = False))

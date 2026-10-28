##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
import sys
##### EndExtImports

##### LocalImports
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB
##### EndLocalImports


class GIMIObjPartFilterTest(BaseUnitTest):
    """
    Tests for :class:`FRB.GIMIObjPartFilter`'s hash-type predicate, on made-up rows under an absurd
    version so nothing depends on the real hash data
    """

    _VERSION = "99.0"
    _NAME = "testrika"
    _BLEND_HASH = "test-hash-blend-vb"
    _IB_HASH = "test-hash-ib"

    def _makeFilter(self, withHashes = True):
        hashes = None
        if (withHashes):
            hashes = FRB.Hashes()
            hashes.addRepoRows({self._VERSION: {self._NAME: {"blend_vb": self._BLEND_HASH, "ib": self._IB_HASH}}})

        return FRB.GIMIObjPartFilter(hashes, None, {"ib"}, self._VERSION)

    def _colouring(self, src):
        return FRB.IfContentPartColouring(src)

    def test_hashTypeWindow_hashOfThatType_isTheWholePart(self):
        result = self._makeFilter().hashTypeWindow("blend_vb", self._colouring({"hash": [(0, self._BLEND_HASH)]}))
        self.assertFalse(result.isEmpty())
        self.assertEqual(len(result.ranges), 1)
        self.assertEqual(result.ranges[0][0], FRB.Ranges.createFull().ranges[0][0])

    def test_hashTypeWindow_hashOfAnotherType_isEmpty(self):
        result = self._makeFilter().hashTypeWindow("blend_vb", self._colouring({"hash": [(0, self._IB_HASH)]}))
        self.assertTrue(result.isEmpty())

    def test_hashTypeWindow_typeOutsideIndexHashKeys_stillMatched(self):
        # indexHashKeys is only {"ib"}: the hash-type window does not consult it
        result = self._makeFilter().hashTypeWindow("blend_vb", self._colouring({"hash": [(3, self._BLEND_HASH)]}))
        self.assertFalse(result.isEmpty())

    def test_hashTypeWindow_unknownHash_isEmpty(self):
        result = self._makeFilter().hashTypeWindow("blend_vb", self._colouring({"hash": [(0, "not-a-hash")]}))
        self.assertTrue(result.isEmpty())

    def test_hashTypeWindow_noHashTable_isEmpty(self):
        result = self._makeFilter(withHashes = False).hashTypeWindow("blend_vb", self._colouring({"hash": [(0, self._BLEND_HASH)]}))
        self.assertTrue(result.isEmpty())

    def test_hashTypeWindow_noHashKey_isEmpty(self):
        result = self._makeFilter().hashTypeWindow("blend_vb", self._colouring({"vb1": [(0, "ResourceBlend")]}))
        self.assertTrue(result.isEmpty())

    def test_hashTypeParts_returnsACallable(self):
        self.assertTrue(callable(self._makeFilter().hashTypeParts("blend_vb")))

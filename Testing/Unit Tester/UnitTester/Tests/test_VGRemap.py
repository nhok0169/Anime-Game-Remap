import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class VGRemapTest(BaseUnitTest):
    """
    Tests for :class:`VGRemap`
    """

    # ================================================
    # =================== remap ======================

    def test_defaultConstructor_emptyRemap(self):
        vgRemap = FRB.VGRemap()
        self.compareDict(vgRemap.remap, {})
        self.assertIsNone(vgRemap.maxIndex)

    def test_constructorWithRemap_remapSet(self):
        vgRemap = FRB.VGRemap({1: 2, 3: 4})
        self.compareDict(vgRemap.remap, {1: 2, 3: 4})

    def test_setRemap_updatesRemapAndMaxIndex(self):
        vgRemap = FRB.VGRemap({1: 2})
        vgRemap.remap = {5: 6, 7: 8}
        self.compareDict(vgRemap.remap, {5: 6, 7: 8})
        self.assertEqual(vgRemap.maxIndex, 7)

    def test_setRemapToEmpty_maxIndexNone(self):
        vgRemap = FRB.VGRemap({1: 2})
        vgRemap.remap = {}
        self.assertIsNone(vgRemap.maxIndex)

    # ================================================
    # ================== maxIndex ====================

    def test_maxIndex_singleEntry(self):
        self.assertEqual(FRB.VGRemap({5: 42}).maxIndex, 5)

    def test_maxIndex_multipleEntries(self):
        self.assertEqual(FRB.VGRemap({1: 0, 9: 0, 4: 0}).maxIndex, 9)

    def test_maxIndex_negativeIndices(self):
        self.assertEqual(FRB.VGRemap({-5: 0, 3: 0, -1: 0}).maxIndex, 3)

    def test_maxIndex_allNegativeIndices(self):
        self.assertEqual(FRB.VGRemap({-5: 0, -1: 0, -10: 0}).maxIndex, -1)

    # ================================================

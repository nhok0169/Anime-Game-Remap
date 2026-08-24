import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppColourTest(BaseUnitTest):
    """
    Tests for :class:`CppColour` (the pybind11-bound engine behind the pure-Python :class:`Colour`,
    which adds no behaviour of its own -- see :meth:`test_bareSubclass_inheritsCleanly` below)
    """

    def test_defaults_opaqueWhite(self):
        c = FRB.CppColour()
        self.assertEqual(c.getTuple(), (255, 255, 255, 255))

    def test_constructor_boundsOutOfRangeChannels(self):
        c = FRB.CppColour(300, -5, 128, 999)
        self.assertEqual(c.getTuple(), (255, 0, 128, 255))

    def test_boundColourChannel_clampsToRange(self):
        self.assertEqual(FRB.CppColour.boundColourChannel(300), 255)
        self.assertEqual(FRB.CppColour.boundColourChannel(-5), 0)
        self.assertEqual(FRB.CppColour.boundColourChannel(128), 128)
        self.assertEqual(FRB.CppColour.boundColourChannel(15, min = 10, max = 20), 15)

    def test_boolToColourChannel(self):
        self.assertEqual(FRB.CppColour.boolToColourChannel(True), 255)
        self.assertEqual(FRB.CppColour.boolToColourChannel(False), 0)

    def test_getId_isConcatenatedChannels(self):
        self.assertEqual(FRB.CppColour(1, 2, 3, 4).getId(), "1234")

    def test_hash_matchesHashOfGetId(self):
        c = FRB.CppColour(1, 2, 3, 4)
        self.assertEqual(hash(c), hash(c.getId()))

    def test_fromTuple_updatesAllChannels(self):
        c = FRB.CppColour()
        c.fromTuple((10, 20, 30, 40))
        self.assertEqual(c.getTuple(), (10, 20, 30, 40))

    def test_copy_withAlpha(self):
        src = FRB.CppColour(1, 2, 3, 4)
        dst = FRB.CppColour()
        dst.copy(src)
        self.assertEqual(dst.getTuple(), (1, 2, 3, 4))

    def test_copy_withoutAlpha_leavesAlphaUnchanged(self):
        src = FRB.CppColour(1, 2, 3, 4)
        dst = FRB.CppColour(alpha = 99)
        dst.copy(src, withAlpha = False)
        self.assertEqual(dst.getTuple(), (1, 2, 3, 99))

    def test_match_trueAndFalse(self):
        a = FRB.CppColour(1, 2, 3, 4)
        self.assertTrue(a.match(FRB.CppColour(1, 2, 3, 4)))
        self.assertFalse(a.match(FRB.CppColour(1, 2, 3, 5)))

    def test_readwriteChannels(self):
        c = FRB.CppColour()
        c.red, c.green, c.blue, c.alpha = 5, 6, 7, 8
        self.assertEqual(c.getTuple(), (5, 6, 7, 8))

    # ================================================
    # ============== bare Colour subclass ============

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.Colour, FRB.CppColour))
        c = FRB.Colour(1, 2, 3, 4)
        self.assertIsInstance(c, FRB.CppColour)
        self.assertEqual(c.getTuple(), (1, 2, 3, 4))

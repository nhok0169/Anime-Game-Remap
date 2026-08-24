import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppTintTransformTest(BaseUnitTest):
    """
    Tests for :class:`CppTintTransform` and its pass-through pure-Python subclass :class:`TintTransform`
    """

    def test_positiveTint_increasesGreen(self):
        pixel = FRB.Colour(1, 100, 1, 1)
        FRB.CppTintTransform(20).transform(pixel, 0, 0)
        self.assertEqual(pixel.green, 120)

    def test_negativeTint_decreasesGreen(self):
        pixel = FRB.Colour(1, 100, 1, 1)
        FRB.CppTintTransform(-20).transform(pixel, 0, 0)
        self.assertEqual(pixel.green, 80)

    def test_tint_boundedToChannelRange(self):
        pixel = FRB.Colour(1, 250, 1, 1)
        FRB.CppTintTransform(20).transform(pixel, 0, 0)
        self.assertEqual(pixel.green, 255)

    def test_redBlueAlphaUntouched(self):
        pixel = FRB.Colour(11, 100, 22, 33)
        FRB.CppTintTransform(20).transform(pixel, 0, 0)
        self.assertEqual((pixel.red, pixel.blue, pixel.alpha), (11, 22, 33))

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.TintTransform, FRB.CppTintTransform))
        pixel = FRB.Colour(1, 100, 1, 1)
        FRB.TintTransform(20).transform(pixel, 0, 0)
        self.assertEqual(pixel.green, 120)

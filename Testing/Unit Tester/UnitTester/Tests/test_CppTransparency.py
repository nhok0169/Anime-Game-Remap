import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppTransparencyTest(BaseUnitTest):
    """
    Tests for :class:`CppTransparency` and its pass-through pure-Python subclass :class:`Transparency`
    """

    def test_positiveAlphaChange_increasesAlpha(self):
        pixel = FRB.Colour(1, 1, 1, 100)
        FRB.CppTransparency(50).transform(pixel, 0, 0)
        self.assertEqual(pixel.alpha, 150)

    def test_negativeAlphaChange_decreasesAlpha(self):
        pixel = FRB.Colour(1, 1, 1, 100)
        FRB.CppTransparency(-50).transform(pixel, 0, 0)
        self.assertEqual(pixel.alpha, 50)

    def test_alphaChange_boundedToChannelRange(self):
        pixel = FRB.Colour(1, 1, 1, 250)
        FRB.CppTransparency(50).transform(pixel, 0, 0)
        self.assertEqual(pixel.alpha, 255)

    def test_rgbUntouched(self):
        pixel = FRB.Colour(11, 22, 33, 100)
        FRB.CppTransparency(50).transform(pixel, 0, 0)
        self.assertEqual((pixel.red, pixel.green, pixel.blue), (11, 22, 33))

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.Transparency, FRB.CppTransparency))
        pixel = FRB.Colour(1, 1, 1, 100)
        FRB.Transparency(50).transform(pixel, 0, 0)
        self.assertEqual(pixel.alpha, 150)

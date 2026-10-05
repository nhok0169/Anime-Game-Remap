import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppHighlightShadowTest(BaseUnitTest):
    """
    Tests for :class:`CppHighlightShadow` and its pass-through pure-Python subclass
    :class:`HighlightShadow`
    """

    def test_zeroHighlightZeroShadow_isNoOp(self):
        pixel = FRB.Colour(100, 120, 140, 200)
        FRB.CppHighlightShadow(0, 0).transform(pixel, 0, 0)
        self.assertEqual(pixel.getTuple(), (100, 120, 140, 200))

    def test_positiveHighlight_brightens(self):
        pixel = FRB.Colour(100, 100, 100, 255)
        FRB.CppHighlightShadow(highlight = 1.0).transform(pixel, 0, 0)
        self.assertGreater(pixel.red, 100)

    def test_alphaUntouched(self):
        pixel = FRB.Colour(100, 100, 100, 77)
        FRB.CppHighlightShadow(highlight = 1.0, shadow = 1.0).transform(pixel, 0, 0)
        self.assertEqual(pixel.alpha, 77)

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.HighlightShadow, FRB.CppHighlightShadow))
        pixel = FRB.Colour(100, 100, 100, 255)
        FRB.HighlightShadow(highlight = 1.0).transform(pixel, 0, 0)
        self.assertGreater(pixel.red, 100)

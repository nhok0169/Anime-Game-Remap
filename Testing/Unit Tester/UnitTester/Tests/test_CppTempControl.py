import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppTempControlTest(BaseUnitTest):
    """
    Tests for :class:`CppTempControl` and its pass-through pure-Python subclass :class:`TempControl`
    """

    def test_zeroTemp_isNoOp(self):
        pixel = FRB.Colour(100, 100, 100, 255)
        FRB.CppTempControl(0).transform(pixel, 0, 0)
        self.assertEqual((pixel.red, pixel.blue), (100, 100))

    def test_positiveTemp_increasesRedDecreasesBlue(self):
        pixel = FRB.Colour(100, 100, 100, 255)
        FRB.CppTempControl(0.5).transform(pixel, 0, 0)
        self.assertGreater(pixel.red, 100)
        self.assertLess(pixel.blue, 100)

    def test_negativeTemp_decreasesRedIncreasesBlue(self):
        pixel = FRB.Colour(100, 100, 100, 255)
        FRB.CppTempControl(-0.5).transform(pixel, 0, 0)
        self.assertLess(pixel.red, 100)
        self.assertGreater(pixel.blue, 100)

    def test_greenAndAlphaUntouched(self):
        pixel = FRB.Colour(100, 42, 100, 77)
        FRB.CppTempControl(0.5).transform(pixel, 0, 0)
        self.assertEqual((pixel.green, pixel.alpha), (42, 77))

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.TempControl, FRB.CppTempControl))
        pixel = FRB.Colour(100, 100, 100, 255)
        FRB.TempControl(0.5).transform(pixel, 0, 0)
        self.assertGreater(pixel.red, 100)

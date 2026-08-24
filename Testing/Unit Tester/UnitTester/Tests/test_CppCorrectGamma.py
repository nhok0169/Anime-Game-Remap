import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppCorrectGammaTest(BaseUnitTest):
    """
    Tests for :class:`CppCorrectGamma` and its pass-through pure-Python subclass :class:`CorrectGamma`
    """

    def test_correctGamma_identityAtGammaOne(self):
        # V_out = V_in ^ (1/1) == V_in
        self.assertEqual(FRB.CppCorrectGamma.correctGamma(128, 1.0), 128)

    def test_correctGamma_boundaryValues(self):
        self.assertEqual(FRB.CppCorrectGamma.correctGamma(0, 2.2), 0)
        self.assertEqual(FRB.CppCorrectGamma.correctGamma(255, 2.2), 255)

    def test_correctGamma_higherGammaBrightens(self):
        self.assertGreater(FRB.CppCorrectGamma.correctGamma(100, 2.2), 100)

    def test_transform_appliesToRgbOnly(self):
        pixel = FRB.Colour(100, 100, 100, 42)
        FRB.CppCorrectGamma(2.2).transform(pixel, 0, 0)
        self.assertEqual(pixel.alpha, 42)
        self.assertNotEqual(pixel.red, 100)

    def test_isSubclassOfBasePixelTransform(self):
        self.assertTrue(issubclass(FRB.CppCorrectGamma, FRB.CppBasePixelTransform))

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.CorrectGamma, FRB.CppCorrectGamma))
        pixel = FRB.Colour(100, 100, 100, 42)
        FRB.CorrectGamma(2.2).transform(pixel, 0, 0)
        self.assertNotEqual(pixel.red, 100)

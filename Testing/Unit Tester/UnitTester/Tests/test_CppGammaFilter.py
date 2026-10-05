import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppGammaFilterTest(BaseUnitTest):
    """
    Tests for :class:`CppGammaFilter` and its pass-through pure-Python subclass :class:`GammaFilter`.
    Operates directly on a :class:`CppTextureFile`'s in-memory buffer (via #setPixels/#getPixels),
    so these are fully deterministic -- no lossy `Compressonator`_ round-trip involved
    """

    def test_transform_correctsRgbLeavesAlphaUntouched(self):
        tf = FRB.CppTextureFile("never_touched.dds")
        tf.setPixels(bytes([100, 100, 100, 128]), 1, 1)

        FRB.CppGammaFilter(2.2).transform(tf)

        px = tf.getPixels()
        self.assertNotEqual(px[0], 100)
        self.assertEqual(px[3], 128)

    def test_transform_matchesCorrectGammaMath(self):
        tf = FRB.CppTextureFile("never_touched.dds")
        tf.setPixels(bytes([50, 100, 200, 255]), 1, 1)

        FRB.CppGammaFilter(1.8).transform(tf)

        px = tf.getPixels()
        expectedR = FRB.CppCorrectGamma.correctGamma(50, 1.8)
        expectedG = FRB.CppCorrectGamma.correctGamma(100, 1.8)
        expectedB = FRB.CppCorrectGamma.correctGamma(200, 1.8)
        self.assertEqual((px[0], px[1], px[2]), (expectedR, expectedG, expectedB))

    def test_gamma_readwrite(self):
        f = FRB.CppGammaFilter(1.0)
        self.assertEqual(f.gamma, 1.0)
        f.gamma = 2.2
        self.assertEqual(f.gamma, 2.2)

    def test_isSubclassOfBaseTexFilter(self):
        self.assertTrue(issubclass(FRB.CppGammaFilter, FRB.CppBaseTexFilter))

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.GammaFilter, FRB.CppGammaFilter))
        tf = FRB.CppTextureFile("never_touched.dds")
        tf.setPixels(bytes([100, 100, 100, 128]), 1, 1)
        FRB.GammaFilter(2.2).transform(tf)
        self.assertNotEqual(tf.getPixels()[0], 100)

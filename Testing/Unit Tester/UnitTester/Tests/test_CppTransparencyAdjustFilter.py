import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB
from PIL import Image


def _texFileWithImg(pixels, size = (1, 1)):
    tf = FRB.TextureFile("never_touched.dds")
    tf.img = Image.new("RGBA", size, pixels)
    return tf


class CppTransparencyAdjustFilterTest(BaseUnitTest):
    """
    Tests for :class:`CppTransparencyAdjustFilter` and its pass-through pure-Python subclass
    :class:`TransparencyAdjustFilter`
    """

    def test_noColoursToFilter_adjustsWholeImage(self):
        tf = _texFileWithImg((1, 2, 3, 100))
        FRB.CppTransparencyAdjustFilter(-50).transform(tf)
        self.assertEqual(tf.img.getpixel((0, 0)), (1, 2, 3, 50))

    def test_coloursToFilter_onlyMatchingPixelsAdjusted(self):
        tf = FRB.TextureFile("never_touched.dds")
        tf.img = Image.new("RGBA", (2, 1))
        tf.img.putpixel((0, 0), (5, 5, 5, 100))
        tf.img.putpixel((1, 0), (1, 1, 1, 100))

        f = FRB.CppTransparencyAdjustFilter(-50, coloursToFilter = {FRB.Colour(5, 5, 5, 100)})
        f.transform(tf)

        self.assertEqual(tf.img.getpixel((0, 0)), (5, 5, 5, 50))
        self.assertEqual(tf.img.getpixel((1, 0)), (1, 1, 1, 100))

    def test_alphaChange_boundedToChannelRange(self):
        tf = _texFileWithImg((1, 1, 1, 240))
        FRB.CppTransparencyAdjustFilter(50).transform(tf)
        self.assertEqual(tf.img.getpixel((0, 0))[3], 255)

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.TransparencyAdjustFilter, FRB.CppTransparencyAdjustFilter))
        tf = _texFileWithImg((1, 2, 3, 100))
        FRB.TransparencyAdjustFilter(-50).transform(tf)
        self.assertEqual(tf.img.getpixel((0, 0)), (1, 2, 3, 50))

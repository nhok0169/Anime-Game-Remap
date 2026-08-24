import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB
from PIL import Image


class CppInvertAlphaFilterTest(BaseUnitTest):
    """
    Tests for :class:`CppInvertAlphaFilter` and its pass-through pure-Python subclass
    :class:`InvertAlphaFilter`. See test_CppInvertAlpha.py for the documented discrepancy with its
    per-pixel sibling :class:`InvertAlpha` (which uses ``0 - alpha`` instead of ``255 - alpha``)
    """

    def _texFileWithImg(self, pixels, size = (1, 1)):
        tf = FRB.TextureFile("never_touched.dds")
        tf.img = Image.new("RGBA", size, pixels)
        return tf

    def test_transform_usesTrueInvert(self):
        tf = self._texFileWithImg((1, 2, 3, 100))
        FRB.CppInvertAlphaFilter().transform(tf)
        self.assertEqual(tf.img.getpixel((0, 0)), (1, 2, 3, 255 - 100))

    def test_transform_appliesToEveryPixel(self):
        tf = FRB.TextureFile("never_touched.dds")
        tf.img = Image.new("RGBA", (2, 1))
        tf.img.putpixel((0, 0), (0, 0, 0, 0))
        tf.img.putpixel((1, 0), (0, 0, 0, 255))

        FRB.CppInvertAlphaFilter().transform(tf)

        self.assertEqual(tf.img.getpixel((0, 0))[3], 255)
        self.assertEqual(tf.img.getpixel((1, 0))[3], 0)

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.InvertAlphaFilter, FRB.CppInvertAlphaFilter))
        tf = self._texFileWithImg((1, 2, 3, 100))
        FRB.InvertAlphaFilter().transform(tf)
        self.assertEqual(tf.img.getpixel((0, 0)), (1, 2, 3, 155))

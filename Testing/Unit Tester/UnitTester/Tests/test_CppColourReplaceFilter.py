import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB
from PIL import Image


def _texFileWithImg(pixels, size = (1, 1)):
    # A bare CppTextureFile has no '.img' at all -- these filters sync from/to TextureFile.img
    # (see PyTexFilterCommon.h), so tests need the real Python-facing TextureFile with a small,
    # hand-built Pillow image assigned directly (skipping the lossy Compressonator round-trip
    # entirely, for fully deterministic pixel assertions).
    tf = FRB.TextureFile("never_touched.dds")
    tf.img = Image.new("RGBA", size, pixels)
    return tf


class CppColourReplaceFilterTest(BaseUnitTest):
    """
    Tests for :class:`CppColourReplaceFilter` and its pass-through pure-Python subclass
    :class:`ColourReplaceFilter`
    """

    def test_noColoursToReplace_replacesWholeImage(self):
        tf = _texFileWithImg((1, 2, 3, 4))
        FRB.CppColourReplaceFilter(FRB.Colour(9, 9, 9, 9)).transform(tf)
        self.assertEqual(tf.img.getpixel((0, 0)), (9, 9, 9, 9))

    def test_replaceAlphaFalse_preservesAlpha(self):
        tf = _texFileWithImg((1, 2, 3, 4))
        FRB.CppColourReplaceFilter(FRB.Colour(9, 9, 9, 9), replaceAlpha = False).transform(tf)
        self.assertEqual(tf.img.getpixel((0, 0)), (9, 9, 9, 4))

    def test_coloursToReplace_onlyMatchingPixelsChanged(self):
        tf = FRB.TextureFile("never_touched.dds")
        tf.img = Image.new("RGBA", (2, 1))
        tf.img.putpixel((0, 0), (5, 5, 5, 5))
        tf.img.putpixel((1, 0), (1, 1, 1, 1))

        f = FRB.CppColourReplaceFilter(FRB.Colour(9, 9, 9, 9), coloursToReplace = {FRB.Colour(5, 5, 5, 5)})
        f.transform(tf)

        self.assertEqual(tf.img.getpixel((0, 0)), (9, 9, 9, 9))
        self.assertEqual(tf.img.getpixel((1, 0)), (1, 1, 1, 1))

    def test_transform_pullsFreshPixelsFromImg_notStaleBuffer(self):
        # The core buffer only reflects whatever .img held at the moment transform() was called --
        # reassigning .img beforehand must be picked up, not some earlier snapshot.
        tf = _texFileWithImg((1, 1, 1, 1))
        tf.img = Image.new("RGBA", (1, 1), (5, 5, 5, 5))
        f = FRB.CppColourReplaceFilter(FRB.Colour(9, 9, 9, 9), coloursToReplace = {FRB.Colour(5, 5, 5, 5)})
        f.transform(tf)
        self.assertEqual(tf.img.getpixel((0, 0)), (9, 9, 9, 9))

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.ColourReplaceFilter, FRB.CppColourReplaceFilter))
        tf = _texFileWithImg((1, 2, 3, 4))
        FRB.ColourReplaceFilter(FRB.Colour(9, 9, 9, 9)).transform(tf)
        self.assertEqual(tf.img.getpixel((0, 0)), (9, 9, 9, 9))

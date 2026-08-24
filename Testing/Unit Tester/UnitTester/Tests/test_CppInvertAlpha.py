import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppInvertAlphaTest(BaseUnitTest):
    """
    Tests for :class:`CppInvertAlpha` and its pass-through pure-Python subclass :class:`InvertAlpha`.

    .. note::
        Pinned regression test for a documented, intentionally-preserved pre-existing bug: this
        class computes ``0 - alpha``, not ``255 - alpha`` -- unlike its whole-image sibling
        :class:`InvertAlphaFilter` (see test_CppInvertAlphaFilter.py), which does use ``255 - alpha``
        and is the "true" invert. Don't "fix" this without updating this test in lockstep.
    """

    def test_transform_usesZeroMinusAlpha_notTrueInvert(self):
        pixel = FRB.Colour(1, 2, 3, 100)
        FRB.CppInvertAlpha().transform(pixel, 0, 0)
        # 0 - 100 == -100; the underlying alpha field is a plain int (no bounding on assignment,
        # same as the pure-Python original), and only wraps to a byte range once it's actually
        # packed into a texture's pixel buffer (see CppTextureFile.setPixel).
        self.assertEqual(pixel.alpha, -100)

    def test_transform_leavesRgbUntouched(self):
        pixel = FRB.Colour(1, 2, 3, 100)
        FRB.CppInvertAlpha().transform(pixel, 0, 0)
        self.assertEqual((pixel.red, pixel.green, pixel.blue), (1, 2, 3))

    def test_differsFromInvertAlphaFilter(self):
        # Same starting alpha, run through both the per-pixel transform and the whole-image
        # filter's own alpha packing (via CppTextureFile.setPixel's byte truncation) to confirm
        # they really do produce different results, not just different-looking formulas.
        pixel = FRB.Colour(1, 2, 3, 100)
        FRB.CppInvertAlpha().transform(pixel, 0, 0)

        tf = FRB.CppTextureFile("never_touched.dds")
        tf.setPixels(bytes([0, 0, 0, 0]), 1, 1)
        tf.setPixel(0, 0, pixel)
        invertAlphaResult = tf.getPixel(0, 0).alpha

        # CppInvertAlphaFilter (unlike CppInvertAlpha) syncs from/to TextureFile.img -- needs the
        # real Python-facing TextureFile, not a bare CppTextureFile (which has no '.img' at all).
        from PIL import Image
        tf2 = FRB.TextureFile("never_touched2.dds")
        tf2.img = Image.new("RGBA", (1, 1), (1, 2, 3, 100))
        FRB.CppInvertAlphaFilter().transform(tf2)
        invertAlphaFilterResult = tf2.img.getpixel((0, 0))[3]

        self.assertNotEqual(invertAlphaResult, invertAlphaFilterResult)
        self.assertEqual(invertAlphaFilterResult, 255 - 100)

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.InvertAlpha, FRB.CppInvertAlpha))
        pixel = FRB.Colour(1, 2, 3, 100)
        FRB.InvertAlpha().transform(pixel, 0, 0)
        self.assertEqual(pixel.alpha, -100)

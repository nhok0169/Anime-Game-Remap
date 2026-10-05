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


class CppHueAdjustTest(BaseUnitTest):
    """
    Tests for :class:`CppHueAdjust` and its pass-through pure-Python subclass :class:`HueAdjust`.

    .. note::
        Not asserting on exact adjusted RGB values -- #adjustedHue deliberately reproduces the
        pure-Python original's own byte/degree-scale mismatch (adding a [-180, 180] hue directly
        onto a 0-255 H byte), so the "correct" output for a given hue isn't independently derivable
        without also reproducing that mismatch by hand. These tests instead pin down the properties
        that must hold regardless: alpha is untouched, a zero adjustment round-trips RGB<->HSV
        cleanly, and a real adjustment on a saturated colour actually changes something.
    """

    def test_zeroHue_roundTripsRgbApproximately(self):
        tf = _texFileWithImg((200, 50, 50, 255))
        FRB.CppHueAdjust(0).transform(tf)
        r, g, b, a = tf.img.getpixel((0, 0))
        self.assertTrue(all(abs(x - y) <= 2 for x, y in zip((r, g, b), (200, 50, 50))))

    def test_alphaUntouched(self):
        tf = _texFileWithImg((200, 50, 50, 77))
        FRB.CppHueAdjust(45).transform(tf)
        self.assertEqual(tf.img.getpixel((0, 0))[3], 77)

    def test_nonZeroHue_changesASaturatedColour(self):
        tf = _texFileWithImg((200, 50, 50, 255))
        FRB.CppHueAdjust(90).transform(tf)
        r, g, b, a = tf.img.getpixel((0, 0))
        self.assertNotEqual((r, g, b), (200, 50, 50))

    def test_grayPixel_hasNoHueToRotate(self):
        # A fully desaturated pixel (r == g == b) has an undefined hue -- rotating it should still
        # leave it gray (S == 0 means H has no visible effect on the RGB result).
        tf = _texFileWithImg((100, 100, 100, 255))
        FRB.CppHueAdjust(90).transform(tf)
        r, g, b, a = tf.img.getpixel((0, 0))
        self.assertTrue(abs(r - g) <= 2 and abs(g - b) <= 2)

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.HueAdjust, FRB.CppHueAdjust))
        tf = _texFileWithImg((200, 50, 50, 255))
        FRB.HueAdjust(90).transform(tf)
        r, g, b, a = tf.img.getpixel((0, 0))
        self.assertNotEqual((r, g, b), (200, 50, 50))

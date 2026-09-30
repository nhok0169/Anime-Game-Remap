import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def _bareTexFile(pixels, width = 1, height = 1):
    # A CppTextureFile is what a fixer opens inside C++ and hands to a config's filter (a GIMIComponentFixerConfig's
    # diffuseEdits, a GIMICharFixerConfig TexEdit): it has no Pillow 'img' attribute, unlike the pure-Python
    # TextureFile the rest of these filters' tests build.
    tf = FRB.CppTextureFile("never_touched.dds")
    tf.setPixels(bytes(pixels), width, height)
    return tf


class CppTexFilterOnCppTextureFileTest(BaseUnitTest):
    """
    Every bound filter whose binding syncs a Pillow ``img`` (:class:`CppTransparencyAdjustFilter`,
    :class:`CppColourReplaceFilter`, :class:`CppInvertAlphaFilter`, :class:`CppHueAdjust`, :class:`CppPixelFilter`)
    must also edit a bare :class:`CppTextureFile`, which has no ``img`` at all. Until 2026-09-27 each of them raised
    ``'CppTextureFile' object has no attribute 'img'`` there, so a config handing one to a fixer skipped the texture
    """

    def test_transparencyAdjust_editsBareCppTextureFile(self):
        tf = _bareTexFile([10, 20, 30, 255])
        FRB.CppTransparencyAdjustFilter(-254).transform(tf)
        self.assertEqual(list(tf.getPixels()), [10, 20, 30, 1])

    def test_transparencyAdjust_callable_editsBareCppTextureFile(self):
        tf = _bareTexFile([10, 20, 30, 200])
        FRB.CppTransparencyAdjustFilter(-100)(tf)
        self.assertEqual(tf.getPixels()[3], 100)

    def test_colourReplace_editsBareCppTextureFile(self):
        tf = _bareTexFile([1, 2, 3, 4])
        FRB.CppColourReplaceFilter(FRB.CppColour(9, 8, 7, 6)).transform(tf)
        self.assertEqual(list(tf.getPixels()), [9, 8, 7, 6])

    def test_invertAlpha_editsBareCppTextureFile(self):
        tf = _bareTexFile([1, 2, 3, 55])
        FRB.CppInvertAlphaFilter().transform(tf)
        self.assertEqual(tf.getPixels()[3], 200)

    def test_hueAdjust_runsOnBareCppTextureFile(self):
        tf = _bareTexFile([200, 50, 50, 255])
        FRB.CppHueAdjust(120).transform(tf)
        self.assertNotEqual(list(tf.getPixels())[:3], [200, 50, 50])

    def test_pixelFilter_editsBareCppTextureFile(self):
        tf = _bareTexFile([10, 20, 30, 40])
        FRB.PixelFilter(transforms = [FRB.CppTransparency(-40)]).transform(tf)
        self.assertEqual(list(tf.getPixels()), [10, 20, 30, 0])

import os
import sys
import tempfile
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def _closeToColour(pixelBytes, offset, colourTuple, tolerance = 10):
    return all(abs(pixelBytes[offset + i] - colourTuple[i]) <= tolerance for i in range(4))


class CppTextureFileTest(BaseUnitTest):
    """
    Tests for :class:`CppTextureFile` -- the `Compressonator`_-backed engine behind the pure-Python
    :class:`TextureFile` (see test_TextureFile.py for the Pillow-facing behaviour layered on top).
    Exercises the raw RGBA8 buffer directly, without going through a real ``.dds`` file where a test
    doesn't need to
    """

    def setUp(self):
        super().setUp()
        self._tmpFiles = []

    def tearDown(self):
        for path in self._tmpFiles:
            if (os.path.isfile(path)):
                os.remove(path)
        super().tearDown()

    def _tmpPath(self, name):
        path = os.path.join(tempfile.gettempdir(), name)
        self._tmpFiles.append(path)
        if (os.path.isfile(path)):
            os.remove(path)
        return path

    # ================================================
    # ================ open ==========================

    def test_open_missingFile_hasImageFalse(self):
        tf = FRB.CppTextureFile(self._tmpPath("cpp_texturefile_missing.dds"))
        tf.open()
        self.assertFalse(tf.hasImage)
        self.assertEqual(tf.getPixels(), b"")

    # ================================================
    # ========== getPixels/setPixels/getPixel/setPixel

    def test_setPixels_thenGetPixels_roundTrips(self):
        tf = FRB.CppTextureFile(self._tmpPath("cpp_texturefile_pixels.dds"))
        data = bytes([10, 20, 30, 40] * 4)  # 2x2
        tf.setPixels(data, 2, 2)
        self.assertEqual(tf.getPixels(), data)
        self.assertEqual(tf.width, 2)
        self.assertEqual(tf.height, 2)

    def test_getPixel_setPixel_singlePixel(self):
        tf = FRB.CppTextureFile(self._tmpPath("cpp_texturefile_singlepixel.dds"))
        tf.setPixels(bytes([0, 0, 0, 0] * 4), 2, 2)

        tf.setPixel(1, 0, FRB.Colour(9, 8, 7, 6))
        px = tf.getPixel(1, 0)
        self.assertEqual((px.red, px.green, px.blue, px.alpha), (9, 8, 7, 6))

        # untouched pixel stays as it was
        other = tf.getPixel(0, 0)
        self.assertEqual((other.red, other.green, other.blue, other.alpha), (0, 0, 0, 0))

    # ================================================
    # ================ gamma =========================

    def test_gamma_defaultsToNone(self):
        tf = FRB.CppTextureFile(self._tmpPath("cpp_texturefile_gamma_default.dds"))
        self.assertIsNone(tf.gamma)

    def test_gamma_settable(self):
        tf = FRB.CppTextureFile(self._tmpPath("cpp_texturefile_gamma_set.dds"))
        tf.gamma = 2.2
        self.assertAlmostEqual(tf.gamma, 2.2, places = 5)
        tf.gamma = None
        self.assertIsNone(tf.gamma)

    # ================================================
    # ================ save/open round trip ==========

    def test_save_thenOpen_roundTripsDimensionsAndPixels(self):
        outPath = self._tmpPath("cpp_texturefile_roundtrip.dds")
        tf = FRB.CppTextureFile(outPath)
        tf.setPixels(bytes([50, 60, 70, 255] * (4 * 4)), 4, 4)
        tf.save()

        self.assertTrue(os.path.isfile(outPath))

        tf2 = FRB.CppTextureFile(outPath)
        tf2.open()
        self.assertTrue(tf2.hasImage)
        self.assertEqual((tf2.width, tf2.height), (4, 4))
        pixels = tf2.getPixels()
        self.assertTrue(_closeToColour(pixels, 0, (50, 60, 70, 255)))

    def test_save_appliesGammaBeforeEncoding(self):
        # RGB should change, alpha should not -- same contract as GammaFilter/CorrectGamma
        outPathNoGamma = self._tmpPath("cpp_texturefile_nogamma.dds")
        tfNoGamma = FRB.CppTextureFile(outPathNoGamma)
        tfNoGamma.setPixels(bytes([100, 100, 100, 128] * (2 * 2)), 2, 2)
        tfNoGamma.save()

        outPathGamma = self._tmpPath("cpp_texturefile_withgamma.dds")
        tfGamma = FRB.CppTextureFile(outPathGamma)
        tfGamma.setPixels(bytes([100, 100, 100, 128] * (2 * 2)), 2, 2)
        tfGamma.gamma = 2.2
        tfGamma.save()

        resultNoGamma = FRB.CppTextureFile(outPathNoGamma)
        resultNoGamma.open()
        resultGamma = FRB.CppTextureFile(outPathGamma)
        resultGamma.open()

        pxNoGamma = resultNoGamma.getPixels()
        pxGamma = resultGamma.getPixels()

        self.assertNotEqual(pxNoGamma[0], pxGamma[0], "gamma correction should have changed the red channel")
        self.assertTrue(_closeToColour(pxGamma, 0, (pxGamma[0], pxGamma[1], pxGamma[2], 128)), "alpha should be untouched by gamma correction")

    def test_save_preservesOriginalFormatOnReSave(self):
        # A texture opened from an existing file remembers its compressed format and re-encodes to
        # it on save, rather than always falling back to CppTextureFile.DefaultFormat.
        outPath = self._tmpPath("cpp_texturefile_format_preserve.dds")
        tf = FRB.CppTextureFile(outPath)
        tf.setPixels(bytes([1, 2, 3, 4] * (2 * 2)), 2, 2)
        tf.save()

        reopened = FRB.CppTextureFile(outPath)
        reopened.open()
        reopened.setPixels(bytes([5, 6, 7, 8] * (2 * 2)), 2, 2)
        reopened.save()  # should not raise, and should still be a valid, openable .dds

        final = FRB.CppTextureFile(outPath)
        final.open()
        self.assertTrue(final.hasImage)
        self.assertTrue(_closeToColour(final.getPixels(), 0, (5, 6, 7, 8)))

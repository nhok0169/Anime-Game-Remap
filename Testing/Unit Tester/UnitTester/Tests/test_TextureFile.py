import os
import sys
import tempfile
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB
from PIL import Image


class TextureFileTest(BaseUnitTest):
    """
    Tests for :class:`TextureFile` -- the pure-Python subclass of :class:`CppTextureFile` that
    layers a real `Pillow`_ ``Image`` (kept at :attr:`img`) on top of the C++ raw-buffer engine, for
    the sake of the still-unported filters that operate on :attr:`img` directly (see
    test_CppTextureFile.py for the underlying buffer/gamma/format behaviour)
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

    def _makeSavedTexture(self, name, size = (4, 4), colour = (10, 20, 30, 255)):
        path = self._tmpPath(name)
        tf = FRB.TextureFile(path)
        img = Image.new("RGBA", size, colour)
        tf.save(img = img)
        return path

    # ================================================
    # ================ open ==========================

    def test_isSubclassOfCppTextureFile(self):
        self.assertTrue(issubclass(FRB.TextureFile, FRB.CppTextureFile))

    def test_open_missingFile_imgIsNone(self):
        tf = FRB.TextureFile(self._tmpPath("texturefile_missing.dds"))
        result = tf.open()
        self.assertIsNone(result)
        self.assertIsNone(tf.img)

    def test_open_existingFile_returnsRGBAImage(self):
        # readPillowImg=True: open() mirrors the decoded buffer into .img and returns it (the
        # readPillowImg=False default is covered under "readPillowImg" below)
        path = self._makeSavedTexture("texturefile_open.dds")
        tf = FRB.TextureFile(path, readPillowImg = True)
        img = tf.open()
        self.assertIsNotNone(img)
        self.assertIs(img, tf.img)
        self.assertEqual(img.mode, "RGBA")
        self.assertEqual(img.size, (4, 4))

    # ================================================
    # ================ read ==========================

    def test_read_opensIfNotAlreadyOpened(self):
        path = self._makeSavedTexture("texturefile_read.dds")
        tf = FRB.TextureFile(path)
        pixels = tf.read()
        self.assertIsNotNone(pixels)
        self.assertIsNotNone(tf.img)

    def test_read_missingFile_returnsNone(self):
        tf = FRB.TextureFile(self._tmpPath("texturefile_read_missing.dds"))
        self.assertIsNone(tf.read())

    def test_read_flushReopens(self):
        path = self._makeSavedTexture("texturefile_read_flush.dds")
        tf = FRB.TextureFile(path)
        tf.open()
        firstImg = tf.img
        tf.read(flush = True)
        self.assertIsNot(tf.img, firstImg)

    # ================================================
    # ================ save ==========================

    def test_save_withImgArgument_setsImg(self):
        path = self._tmpPath("texturefile_save_arg.dds")
        tf = FRB.TextureFile(path)
        img = Image.new("RGBA", (2, 2), (1, 2, 3, 4))
        tf.save(img = img)
        self.assertTrue(os.path.isfile(path))

    def test_save_usesInfoGammaKey(self):
        path = self._tmpPath("texturefile_save_gamma.dds")
        tf = FRB.TextureFile(path)
        tf.img = Image.new("RGBA", (2, 2), (100, 100, 100, 128))
        tf.info["gamma"] = 2.2
        tf.save()

        result = FRB.TextureFile(path)
        result.open()
        px = result.read()[0, 0]
        self.assertNotEqual(px[0], 100, "gamma correction from .info should have changed the pixel")
        self.assertEqual(px[3], 128, "alpha should be untouched by gamma correction")

    def test_save_noGammaKey_keepsTheGammaOpenDetected(self):
        # open() sets the gamma off an sRGB DX10 header; save() used to write info's (absent) gamma
        # over it, so a Python-driven save of an sRGB texture came out uncorrected (2026-09-12)
        path = self._tmpPath("texturefile_save_keptgamma.dds")
        tf = FRB.TextureFile(path)
        tf.img = Image.new("RGBA", (2, 2), (100, 100, 100, 128))
        tf.gamma = 2.2
        tf.save()

        self.assertAlmostEqual(tf.gamma, 2.2, places = 5)
        result = FRB.TextureFile(path)
        result.open()
        px = result.read()[0, 0]
        self.assertNotEqual(px[0], 100, "the gamma open() detected should still correct the pixel")
        self.assertEqual(px[3], 128)

    def test_save_mipmaps_writesTheFullChain(self):
        # Every texture the game ships carries its mip chain; one written without it is sampled from
        # its top level at every distance, which reads in game as scattered off-colour pixels on
        # fine textures such as hair (2026-09-12). The default stays a single level.
        import struct
        for mipmaps, expected in ((False, 1), (True, 6)):
            path = self._tmpPath(f"texturefile_save_mips{int(mipmaps)}.dds")
            tf = FRB.TextureFile(path)
            tf.img = Image.new("RGBA", (32, 16), (10, 200, 30, 255))
            tf.save(mipmaps = mipmaps)

            with open(path, "rb") as f:
                header = f.read(32)
            self.assertEqual(struct.unpack_from("<I", header, 28)[0], expected, f"mipmaps = {mipmaps}")
            result = FRB.TextureFile(path)
            result.open()
            self.assertEqual((result.width, result.height), (32, 16))
            px = result.read()[0, 0]
            self.assertTrue(all(abs(a - b) <= 3 for a, b in zip(px, (10, 200, 30, 255))))

    def test_save_noGammaKey_doesNotGammaCorrect(self):
        path = self._tmpPath("texturefile_save_nogamma.dds")
        tf = FRB.TextureFile(path)
        tf.img = Image.new("RGBA", (2, 2), (100, 100, 100, 128))
        tf.save()

        result = FRB.TextureFile(path)
        result.open()
        px = result.read()[0, 0]
        self.assertTrue(all(abs(a - b) <= 3 for a, b in zip(px, (100, 100, 100, 128))))

    def test_save_refreshesImgAfterward(self):
        # readPillowImg=True: save() mirrors the just-written buffer back into .img afterward (the
        # readPillowImg=False default -- .img left None -- is covered under "readPillowImg" below)
        path = self._tmpPath("texturefile_save_refresh.dds")
        tf = FRB.TextureFile(path, readPillowImg = True)
        original = Image.new("RGBA", (2, 2), (5, 6, 7, 8))
        tf.save(img = original)
        # .img should now be a freshly-decoded image, not the exact object passed in
        self.assertIsNot(tf.img, original)
        self.assertEqual(tf.img.size, (2, 2))

    def test_info_defaultsToEmptyDict(self):
        tf = FRB.TextureFile(self._tmpPath("texturefile_info_default.dds"))
        self.assertEqual(tf.info, {})

    # ================================================
    # ================ engine ==========================

    def test_engine_defaultsToCompressonator(self):
        tf = FRB.TextureFile(self._tmpPath("texturefile_engine_default.dds"))
        self.assertEqual(tf.engine, FRB.TexEngine.Compressonator)

    def test_pillowEngine_fullRoundTrip_bypassesCompressonator(self):
        path = self._tmpPath("texturefile_engine_pillow.dds")
        tf = FRB.TextureFile(path, engine = FRB.TexEngine.Pillow)
        tf.save(img = Image.new("RGBA", (4, 4), (100, 100, 100, 128)))
        self.assertTrue(os.path.isfile(path))

        result = FRB.TextureFile(path, engine = FRB.TexEngine.Pillow)
        opened = result.open()
        self.assertIsNotNone(opened)
        self.assertTrue(result.hasImage)
        self.assertEqual((result.width, result.height), (4, 4))
        self.assertEqual(result.read()[0, 0], (100, 100, 100, 128))

    def test_pillowEngine_gammaStillAppliesViaSharedGammaFilter(self):
        path = self._tmpPath("texturefile_engine_pillow_gamma.dds")
        tf = FRB.TextureFile(path, engine = FRB.TexEngine.Pillow)
        tf.info["gamma"] = 2.2
        tf.save(img = Image.new("RGBA", (2, 2), (100, 100, 100, 128)))

        result = FRB.TextureFile(path, engine = FRB.TexEngine.Pillow)
        result.open()
        px = result.read()[0, 0]
        self.assertNotEqual(px[0], 100)
        self.assertEqual(px[3], 128)

    def test_pillowEngine_missingFile_hasImageFalseWidthHeightZero(self):
        tf = FRB.TextureFile(self._tmpPath("texturefile_engine_pillow_missing.dds"), engine = FRB.TexEngine.Pillow)
        tf.open()
        self.assertFalse(tf.hasImage)
        self.assertEqual((tf.width, tf.height), (0, 0))

    def test_hasImageWidthHeight_derivedFromImg_notCppBuffer(self):
        # Regression test: these three properties are overridden on TextureFile to derive from
        # .img directly (see TextureFile.py's own note on why) -- setting .img manually (bypassing
        # open()/save() entirely) must still report correctly, proving they don't just proxy the
        # inherited CppTextureFile state.
        tf = FRB.TextureFile(self._tmpPath("texturefile_hasimage_derived.dds"))
        self.assertFalse(tf.hasImage)
        tf.img = Image.new("RGBA", (7, 3))
        self.assertTrue(tf.hasImage)
        self.assertEqual((tf.width, tf.height), (7, 3))

    # ================================================
    # ================ readPillowImg ===================

    def test_readPillowImg_defaultsToFalse(self):
        tf = FRB.TextureFile(self._tmpPath("texturefile_readpillow_default.dds"))
        self.assertFalse(tf.readPillowImg)

    def test_readPillowImg_settableViaConstructor(self):
        tf = FRB.TextureFile(self._tmpPath("texturefile_readpillow_ctor.dds"), readPillowImg = True)
        self.assertTrue(tf.readPillowImg)

    def test_readPillowImgFalse_open_leavesImgNone(self):
        path = self._makeSavedTexture("texturefile_readpillow_false_open.dds")
        tf = FRB.TextureFile(path)  # readPillowImg=False (default)
        result = tf.open()
        self.assertIsNone(result)
        self.assertIsNone(tf.img)

    def test_readPillowImgFalse_open_hasImageWidthHeightStillCorrect(self):
        # even with .img left unset, hasImage/width/height fall back to the native Compressonator
        # buffer state -- callers that only need to know "did this succeed, and what size is it"
        # (eg. Mod.py) never actually need readPillowImg=True
        path = self._makeSavedTexture("texturefile_readpillow_false_fallback.dds", size = (5, 3))
        tf = FRB.TextureFile(path)
        tf.open()
        self.assertTrue(tf.hasImage)
        self.assertEqual((tf.width, tf.height), (5, 3))

    def test_readPillowImgFalse_save_leavesImgNone(self):
        path = self._tmpPath("texturefile_readpillow_false_save.dds")
        tf = FRB.TextureFile(path)  # readPillowImg=False (default)
        tf.save(img = Image.new("RGBA", (2, 2), (10, 20, 30, 255)))
        self.assertIsNone(tf.img)
        self.assertTrue(tf.hasImage)
        self.assertEqual((tf.width, tf.height), (2, 2))

    def test_readPillowImgTrue_open_populatesImg(self):
        path = self._makeSavedTexture("texturefile_readpillow_true_open.dds")
        tf = FRB.TextureFile(path, readPillowImg = True)
        tf.open()
        self.assertIsNotNone(tf.img)

    def test_readPillowImgTrue_save_populatesImg(self):
        path = self._tmpPath("texturefile_readpillow_true_save.dds")
        tf = FRB.TextureFile(path, readPillowImg = True)
        tf.save(img = Image.new("RGBA", (2, 2), (10, 20, 30, 255)))
        self.assertIsNotNone(tf.img)

    # ================================================
    # ================ saveAs ==========================

    def test_saveAs_png_opensOnDemandWhenNothingLoaded(self):
        # the one-liner case this method exists for: TextureFile(x).saveAs(y), no open() first
        path = self._makeSavedTexture("texturefile_saveas_ondemand.dds", size = (4, 4), colour = (10, 20, 30, 255))
        pngPath = self._tmpPath("texturefile_saveas_ondemand.png")

        self.assertTrue(FRB.TextureFile(path).saveAs(pngPath))
        self.assertTrue(os.path.isfile(pngPath))

        # 'with' (here and below): an un-closed Pillow handle keeps the file locked on Windows, and
        # tearDown's os.remove would then fail
        with Image.open(pngPath) as written:
            self.assertEqual(written.size, (4, 4))

    def test_saveAs_png_keepsSrcPointingAtTheDds(self):
        path = self._makeSavedTexture("texturefile_saveas_src.dds")
        pngPath = self._tmpPath("texturefile_saveas_src.png")

        tf = FRB.TextureFile(path)
        tf.saveAs(pngPath)
        self.assertEqual(tf.src, path)

    def test_saveAs_missingFile_returnsFalse(self):
        tf = FRB.TextureFile(self._tmpPath("texturefile_saveas_missing.dds"))
        pngPath = self._tmpPath("texturefile_saveas_missing.png")

        self.assertFalse(tf.saveAs(pngPath))
        self.assertFalse(os.path.isfile(pngPath))

    def test_saveAs_png_writesTheEditedPixelsNotTheOnesOnDisk(self):
        path = self._makeSavedTexture("texturefile_saveas_edited.dds", size = (2, 2), colour = (10, 20, 30, 255))
        pngPath = self._tmpPath("texturefile_saveas_edited.png")

        tf = FRB.TextureFile(path)
        tf.open()
        tf.setPixels(bytes([200, 100, 50, 255] * (2 * 2)), 2, 2)
        self.assertTrue(tf.saveAs(pngPath))

        with Image.open(pngPath) as written:
            self.assertEqual(written.convert("RGBA").getpixel((0, 0)), (200, 100, 50, 255))

    def test_saveAs_png_syncsFromImgWhenItIsBeingMaintained(self):
        path = self._makeSavedTexture("texturefile_saveas_fromimg.dds", size = (2, 2), colour = (10, 20, 30, 255))
        pngPath = self._tmpPath("texturefile_saveas_fromimg.png")

        tf = FRB.TextureFile(path, readPillowImg = True)
        tf.open()
        tf.img.putpixel((0, 0), (7, 8, 9, 255))
        self.assertTrue(tf.saveAs(pngPath))

        with Image.open(pngPath) as written:
            self.assertEqual(written.convert("RGBA").getpixel((0, 0)), (7, 8, 9, 255))

    def test_saveAs_doesNotApplyInfoGamma(self):
        # save() gamma-corrects; saveAs() deliberately does not -- it's meant to show the texture's
        # actual decoded pixels
        path = self._makeSavedTexture("texturefile_saveas_gamma.dds", size = (2, 2), colour = (100, 100, 100, 255))
        pngPath = self._tmpPath("texturefile_saveas_gamma.png")

        tf = FRB.TextureFile(path)
        tf.open()
        tf.info["gamma"] = 2.2
        self.assertTrue(tf.saveAs(pngPath))

        with Image.open(pngPath) as written:
            px = written.convert("RGBA").getpixel((0, 0))
        self.assertTrue(abs(px[0] - 100) <= 8, f"expected an uncorrected ~100 red channel, got {px}")

    def test_saveAs_pillowEngine_alsoWorks(self):
        path = self._tmpPath("texturefile_saveas_pillow.dds")
        src = FRB.TextureFile(path, engine = FRB.TexEngine.Pillow)
        src.save(img = Image.new("RGBA", (3, 3), (11, 22, 33, 255)))

        pngPath = self._tmpPath("texturefile_saveas_pillow.png")
        self.assertTrue(FRB.TextureFile(path, engine = FRB.TexEngine.Pillow).saveAs(pngPath))

        with Image.open(pngPath) as written:
            self.assertEqual(written.size, (3, 3))
            self.assertEqual(written.convert("RGBA").getpixel((0, 0)), (11, 22, 33, 255))

    def test_readPillowImgFalse_read_stillBuildsImgOnDemand(self):
        # read() is the on-demand escape hatch: regardless of readPillowImg, it must build .img
        # when a caller actually asks for pixel data through it
        path = self._makeSavedTexture("texturefile_readpillow_false_read.dds", colour = (1, 2, 3, 4))
        tf = FRB.TextureFile(path)  # readPillowImg=False (default)
        self.assertIsNone(tf.img)
        px = tf.read()[0, 0]
        self.assertIsNotNone(tf.img)
        self.assertEqual(px, (1, 2, 3, 4))

import os
import sys
import tempfile
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class TexCreatorTest(BaseUnitTest):
    """
    Tests for :class:`TexCreator` -- reimplements ``fix`` in Python on top of
    :class:`CppTexCreator` so the new texture is written via :class:`TextureFile`'s own
    :meth:`~TextureFile.save`, matching every other texture edit in this codebase (see
    test_CppTexCreator.py for the mostly-unused C++ base)
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

    def test_isSubclassOfCppTexCreator(self):
        self.assertTrue(issubclass(FRB.TexCreator, FRB.CppTexCreator))

    def test_noColourArgument_defaultsToOpaqueWhite(self):
        self.assertEqual(FRB.TexCreator(4, 4).colour.getTuple(), (255, 255, 255, 255))

    def test_fix_createsNewFileWithRequestedSize(self):
        outPath = self._tmpPath("texcreator_new.dds")
        tf = FRB.TextureFile(outPath)
        FRB.TexCreator(8, 4, FRB.Colour(1, 2, 3, 4)).fix(tf, outPath)

        self.assertTrue(os.path.isfile(outPath))
        result = FRB.TextureFile(outPath)
        result.open()
        # readPillowImg defaults to False, so width/height come from the buffer fallback, not .img
        self.assertEqual((result.width, result.height), (8, 4))

    def test_fix_fillsWithRequestedColour(self):
        outPath = self._tmpPath("texcreator_colour.dds")
        tf = FRB.TextureFile(outPath)
        FRB.TexCreator(2, 2, FRB.Colour(10, 20, 30, 255)).fix(tf, outPath)

        result = FRB.TextureFile(outPath)
        result.open()
        px = result.read()[0, 0]
        self.assertTrue(all(abs(a - b) <= 8 for a, b in zip(px, (10, 20, 30, 255))))

    def test_fix_existingFile_isNoOp(self):
        outPath = self._tmpPath("texcreator_existing.dds")
        tf = FRB.TextureFile(outPath)
        FRB.TexCreator(2, 2, FRB.Colour(10, 20, 30, 255)).fix(tf, outPath)

        # a second TexCreator, aimed at the same now-existing file, must not overwrite it
        tf2 = FRB.TextureFile(outPath)
        FRB.TexCreator(999, 999, FRB.Colour(0, 0, 0, 0)).fix(tf2, outPath)

        result = FRB.TextureFile(outPath)
        result.open()
        # readPillowImg defaults to False, so width/height come from the buffer fallback, not .img
        self.assertEqual((result.width, result.height), (2, 2))

    def test_fix_setsSrcToFixedTexFile(self):
        outPath = self._tmpPath("texcreator_setssrc.dds")
        tf = FRB.TextureFile("some_other_starting_path.dds")
        FRB.TexCreator(2, 2).fix(tf, outPath)
        self.assertEqual(tf.src, outPath)

    # ================================================
    # ================ engine ==========================

    def test_engine_defaultsToCompressonator(self):
        self.assertEqual(FRB.TexCreator(2, 2).engine, FRB.TexEngine.Compressonator)

    def test_fix_overridesTexFilesOwnEngine(self):
        outPath = self._tmpPath("texcreator_engine_override.dds")
        tf = FRB.TextureFile(outPath, engine = FRB.TexEngine.Compressonator)
        FRB.TexCreator(3, 3, FRB.Colour(9, 9, 9, 9), engine = FRB.TexEngine.Pillow).fix(tf, outPath)

        self.assertEqual(tf.engine, FRB.TexEngine.Pillow)
        result = FRB.TextureFile(outPath, engine = FRB.TexEngine.Pillow)
        result.open()
        self.assertEqual(result.img.size, (3, 3))
        self.assertEqual(result.read()[0, 0], (9, 9, 9, 9))

    # ================================================
    # ================ readPillowImg ===================

    def test_readPillowImg_defaultsToFalse(self):
        self.assertFalse(FRB.TexCreator(2, 2).readPillowImg)

    def test_readPillowImg_settableViaConstructor(self):
        self.assertTrue(FRB.TexCreator(2, 2, readPillowImg = True).readPillowImg)

    def test_fix_readPillowImgFalse_leavesImgNoneButHasImageTrue(self):
        outPath = self._tmpPath("texcreator_readpillow_false.dds")
        tf = FRB.TextureFile(outPath)  # readPillowImg=False (default)
        FRB.TexCreator(3, 3, FRB.Colour(9, 9, 9, 9)).fix(tf, outPath)

        self.assertIsNone(tf.img)
        self.assertTrue(tf.hasImage)
        self.assertEqual((tf.width, tf.height), (3, 3))

    def test_fix_readPillowImgTrue_populatesImg(self):
        outPath = self._tmpPath("texcreator_readpillow_true.dds")
        tf = FRB.TextureFile(outPath)
        FRB.TexCreator(3, 3, FRB.Colour(9, 9, 9, 9), readPillowImg = True).fix(tf, outPath)

        self.assertIsNotNone(tf.img)
        self.assertEqual(tf.img.size, (3, 3))

import os
import sys
import tempfile
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB
from PIL import Image


class TexEditorTest(BaseUnitTest):
    """
    Tests for :class:`TexEditor` -- entirely reimplemented in Python on top of
    :class:`CppTexEditor` so that ``filters`` can hold arbitrary Python callables, not just
    something a ``std::function`` can accept (see test_CppTexEditor.py for the mostly-unused C++
    base)
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

    def _makeSourceTexture(self, name, colour = (100, 100, 100, 255), size = (2, 2)):
        path = self._tmpPath(name)
        tf = FRB.TextureFile(path)
        tf.save(img = Image.new("RGBA", size, colour))
        return path

    # ================================================
    # ================ constructor ===================

    def test_isSubclassOfCppTexEditor(self):
        self.assertTrue(issubclass(FRB.TexEditor, FRB.CppTexEditor))

    def test_noFiltersArgument_defaultsToEmptyList(self):
        self.assertEqual(FRB.TexEditor().filters, [])

    # ================================================
    # ================ fix ============================

    def test_fix_noFilters_isNoOp(self):
        srcPath = self._makeSourceTexture("texeditor_nofilters.dds")
        outPath = self._tmpPath("texeditor_nofilters_out.dds")

        tf = FRB.TextureFile(srcPath)
        FRB.TexEditor().fix(tf, outPath)

        self.assertFalse(os.path.isfile(outPath))

    def test_fix_missingSourceFile_isNoOp(self):
        outPath = self._tmpPath("texeditor_missing_out.dds")
        tf = FRB.TextureFile(self._tmpPath("texeditor_missing_src.dds"))

        called = []
        FRB.TexEditor(filters = [lambda texFile: called.append(texFile)]).fix(tf, outPath)

        self.assertEqual(called, [])
        self.assertFalse(os.path.isfile(outPath))

    def test_fix_appliesFilterAndWritesOutput(self):
        srcPath = self._makeSourceTexture("texeditor_apply_src.dds", colour = (100, 100, 100, 255))
        outPath = self._tmpPath("texeditor_apply_out.dds")

        tf = FRB.TextureFile(srcPath)
        # TintTransform is a per-pixel BasePixelTransform, not a whole-image BaseTexFilter -- it
        # needs wrapping in a PixelFilter to be usable as a TexEditor.filters entry (their call
        # signatures differ: filter(texFile) vs. transform(pixel, x, y)).
        FRB.TexEditor(filters = [FRB.PixelFilter(transforms = [FRB.TintTransform(20)])]).fix(tf, outPath)

        self.assertTrue(os.path.isfile(outPath))
        result = FRB.TextureFile(outPath)
        result.open()
        px = result.read()[0, 0]
        self.assertTrue(abs(px[1] - 120) <= 8, f"expected green channel near 120 (BC7-lossy tolerance), got {px}")

    def test_fix_multipleFilters_appliedInOrder(self):
        srcPath = self._makeSourceTexture("texeditor_order_src.dds", colour = (100, 100, 100, 255))
        outPath = self._tmpPath("texeditor_order_out.dds")

        order = []

        def recordA(texFile):
            order.append("A")

        def recordB(texFile):
            order.append("B")

        tf = FRB.TextureFile(srcPath)
        FRB.TexEditor(filters = [recordA, recordB]).fix(tf, outPath)

        self.assertEqual(order, ["A", "B"])

    def test_fix_mixedNativeAndPythonCallableFilters(self):
        srcPath = self._makeSourceTexture("texeditor_mixed_src.dds", colour = (100, 100, 100, 200))
        outPath = self._tmpPath("texeditor_mixed_out.dds")

        calls = []

        def recordCall(texFile):
            calls.append(texFile.img.getpixel((0, 0)))

        # recordCall reaches into .img directly, so this mix needs readPillowImg=True (the default,
        # False, keeps .img unset -- see the readPillowImg tests further down)
        tf = FRB.TextureFile(srcPath)
        FRB.TexEditor(filters = [FRB.InvertAlphaFilter(), recordCall], readPillowImg = True).fix(tf, outPath)

        self.assertEqual(len(calls), 1)
        self.assertEqual(calls[0][3], 255 - 200)  # InvertAlphaFilter ran before recordCall

    def test_fix_setsSrcToFixedTexFile(self):
        srcPath = self._makeSourceTexture("texeditor_setssrc_src.dds")
        outPath = self._tmpPath("texeditor_setssrc_out.dds")

        tf = FRB.TextureFile(srcPath)
        FRB.TexEditor(filters = [FRB.InvertAlphaFilter()]).fix(tf, outPath)

        self.assertEqual(tf.src, outPath)

    # ================================================
    # ================ engine ==========================

    def test_engine_defaultsToCompressonator(self):
        self.assertEqual(FRB.TexEditor().engine, FRB.TexEngine.Compressonator)

    def test_fix_overridesTexFilesOwnEngine(self):
        srcPath = self._makeSourceTexture("texeditor_engine_src.dds")
        outPath = self._tmpPath("texeditor_engine_out.dds")

        tf = FRB.TextureFile(srcPath, engine = FRB.TexEngine.Compressonator)
        FRB.TexEditor(filters = [FRB.InvertAlphaFilter()], engine = FRB.TexEngine.Pillow).fix(tf, outPath)

        self.assertEqual(tf.engine, FRB.TexEngine.Pillow)
        self.assertTrue(os.path.isfile(outPath))

    def test_fix_pillowEngine_producesReadableOutput(self):
        srcPath = self._makeSourceTexture("texeditor_engine_pillow_src.dds", colour = (10, 20, 30, 200))
        outPath = self._tmpPath("texeditor_engine_pillow_out.dds")

        # source itself is written with the default (Compressonator) engine, but the editor forces
        # Pillow for the fix() call -- confirms the two engines can be mixed across the lifetime of
        # a single TextureFile without corrupting anything.
        tf = FRB.TextureFile(srcPath)
        FRB.TexEditor(filters = [FRB.InvertAlphaFilter()], engine = FRB.TexEngine.Pillow).fix(tf, outPath)

        result = FRB.TextureFile(outPath, engine = FRB.TexEngine.Pillow)
        result.open()
        px = result.read()[0, 0]
        self.assertEqual(px[3], 255 - 200)

        self.assertEqual(tf.src, outPath)

    # ================================================
    # ================ readPillowImg ===================

    def test_readPillowImg_defaultsToFalse(self):
        self.assertFalse(FRB.TexEditor().readPillowImg)

    def test_readPillowImg_settableViaConstructor(self):
        self.assertTrue(FRB.TexEditor(readPillowImg = True).readPillowImg)

    def test_fix_readPillowImgFalse_nativeFilterStaysBufferNative(self):
        # a filter with a genuine C++ transform() override (see PixelFilter's classification logic)
        # must keep working purely off the native buffer when readPillowImg=False -- .img is never
        # populated at all across the whole fix() call
        srcPath = self._makeSourceTexture("texeditor_readpillow_false_native.dds", colour = (100, 100, 100, 255))
        outPath = self._tmpPath("texeditor_readpillow_false_native_out.dds")

        tf = FRB.TextureFile(srcPath)  # readPillowImg=False (default)
        FRB.TexEditor(filters = [FRB.PixelFilter(transforms = [FRB.TintTransform(20)])]).fix(tf, outPath)

        self.assertTrue(os.path.isfile(outPath))
        self.assertIsNone(tf.img)

        result = FRB.TextureFile(outPath)
        result.open()
        px = result.read()[0, 0]
        self.assertTrue(abs(px[1] - 120) <= 8, f"expected green channel near 120 (BC7-lossy tolerance), got {px}")

    def test_fix_readPillowImgFalse_hasImageTrueOnSuccess(self):
        # mirrors the Mod.py-style success check: hasImage must fall back to the native buffer
        # state even though .img is never populated
        srcPath = self._makeSourceTexture("texeditor_readpillow_false_hasimage.dds")
        outPath = self._tmpPath("texeditor_readpillow_false_hasimage_out.dds")

        tf = FRB.TextureFile(srcPath)
        FRB.TexEditor(filters = [FRB.InvertAlphaFilter()]).fix(tf, outPath)
        self.assertTrue(tf.hasImage)

    def test_fix_readPillowImgTrue_pillowTouchingCallableStillWorks(self):
        # a plain Python callable that reaches into .img directly needs readPillowImg=True
        srcPath = self._makeSourceTexture("texeditor_readpillow_true_callable.dds", colour = (50, 60, 70, 200))
        outPath = self._tmpPath("texeditor_readpillow_true_callable_out.dds")

        def invertAlphaViaImg(texFile):
            alphaImg = texFile.img.getchannel("A")
            texFile.img.putalpha(alphaImg.point(lambda a: 255 - a))

        tf = FRB.TextureFile(srcPath)
        FRB.TexEditor(filters = [invertAlphaViaImg], readPillowImg = True).fix(tf, outPath)

        result = FRB.TextureFile(outPath, readPillowImg = True)
        result.open()
        px = result.read()[0, 0]
        self.assertTrue(abs(px[3] - (255 - 200)) <= 8)

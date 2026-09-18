import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB
from PIL import Image


class CppPixelFilterTest(BaseUnitTest):
    """
    Tests for :class:`CppPixelFilter`'s ``transform`` dispatch logic (the native-vs-Python-callable
    fast-path classification, applied fresh to whatever the current ``transforms`` list holds).

    .. note::
        A bare :class:`CppPixelFilter` instance has no ``.transforms`` attribute of its own -- only
        a real Python subclass (:class:`PixelFilter`) gets a ``__dict__`` to hold one (a plain
        pybind11-bound instance isn't registered with ``py::dynamic_attr()``). These tests construct
        via :class:`PixelFilter` for that reason, even though the actual dispatch logic under test
        lives entirely on :class:`CppPixelFilter` -- see test_PixelFilter.py for
        :class:`PixelFilter`'s own added ``__init__``/list-mutation behaviour
    """

    def _texFileWithImg(self, size, fill = (0, 0, 0, 0)):
        tf = FRB.TextureFile("never_touched.dds")
        tf.img = Image.new("RGBA", size, fill)
        return tf

    def test_emptyTransforms_isNoOp(self):
        tf = self._texFileWithImg((1, 1), (1, 2, 3, 4))
        f = FRB.PixelFilter()
        f.transform(tf)
        self.assertEqual(tf.img.getpixel((0, 0)), (1, 2, 3, 4))

    def test_nativeTransform_runsAtCppSpeed_noPerPixelPythonCall(self):
        tf = self._texFileWithImg((2, 2), (100, 100, 100, 255))
        f = FRB.PixelFilter(transforms = [FRB.TintTransform(20)])
        f.transform(tf)
        for xy in [(0, 0), (1, 0), (0, 1), (1, 1)]:
            self.assertEqual(tf.img.getpixel(xy), (100, 120, 100, 255))

    def test_pythonCallable_actuallyInvokedPerPixel(self):
        calls = []

        def markVisited(pixel, x, y):
            calls.append((x, y))
            pixel.red = 9

        tf = self._texFileWithImg((2, 2), (1, 1, 1, 1))
        f = FRB.PixelFilter(transforms = [markVisited])
        f.transform(tf)

        self.assertEqual(set(calls), {(0, 0), (1, 0), (0, 1), (1, 1)})
        for xy in [(0, 0), (1, 0), (0, 1), (1, 1)]:
            self.assertEqual(tf.img.getpixel(xy)[0], 9)

    def test_mixedNativeAndPythonTransforms_appliedInOrder(self):
        tf = self._texFileWithImg((1, 1), (100, 100, 100, 100))

        def addOneToGreen(pixel, x, y):
            pixel.green += 1

        f = FRB.PixelFilter(transforms = [FRB.TintTransform(10), addOneToGreen])
        f.transform(tf)

        self.assertEqual(tf.img.getpixel((0, 0)), (100, 111, 100, 100))

    def test_pixelTransformInstance_ofBasePixelTransformSubclass_takesFastPath(self):
        # Regression test: a pure-Python BasePixelTransform subclass overriding only 'transform'
        # has no C++ vtable entry for that override (no trampoline) -- classifying it as "native"
        # by isinstance() alone and calling straight through the C++ pointer would silently run the
        # no-op base implementation instead. The real discriminator is whether the instance's own
        # class actually resolves 'transform' to something other than the inherited C++ descriptor.
        class DoubleRed(FRB.BasePixelTransform):
            def transform(self, pixel, x, y):
                pixel.red = min(255, pixel.red * 2)

        tf = self._texFileWithImg((1, 1), (10, 1, 1, 1))
        f = FRB.PixelFilter(transforms = [DoubleRed()])
        f.transform(tf)
        self.assertEqual(tf.img.getpixel((0, 0))[0], 20)

    def test_nativePixelTransformSubclass_stillTakesFastPath(self):
        # The flip side of the regression above: a genuine C++-native transform (no Python
        # override at all) must still be classified as native and dispatched directly.
        tf = self._texFileWithImg((1, 1), (100, 100, 100, 255))
        f = FRB.PixelFilter(transforms = [FRB.CorrectGamma(2.2)])
        f.transform(tf)
        self.assertNotEqual(tf.img.getpixel((0, 0))[0], 100)

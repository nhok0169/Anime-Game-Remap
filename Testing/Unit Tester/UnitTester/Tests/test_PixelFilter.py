import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB
from PIL import Image


class PixelFilterTest(BaseUnitTest):
    """
    Tests for :class:`PixelFilter` -- adds the ``transforms = None`` default-to-empty-list
    constructor behaviour on top of :class:`CppPixelFilter` (see test_CppPixelFilter.py for the
    actual per-pixel dispatch logic)
    """

    def test_isSubclassOfCppPixelFilter(self):
        self.assertTrue(issubclass(FRB.PixelFilter, FRB.CppPixelFilter))

    def test_noTransformsArgument_defaultsToEmptyList(self):
        f = FRB.PixelFilter()
        self.assertEqual(f.transforms, [])

    def test_transformsArgument_stored(self):
        transform = FRB.TintTransform(5)
        f = FRB.PixelFilter(transforms = [transform])
        self.assertEqual(f.transforms, [transform])

    def test_mutatingTransformsInPlace_affectsNextTransformCall(self):
        tf = FRB.TextureFile("never_touched.dds")
        tf.img = Image.new("RGBA", (1, 1), (100, 100, 100, 255))

        f = FRB.PixelFilter()
        f.transform(tf)
        self.assertEqual(tf.img.getpixel((0, 0)), (100, 100, 100, 255))  # no-op, empty list

        f.transforms.append(FRB.TintTransform(10))
        f.transform(tf)
        self.assertEqual(tf.img.getpixel((0, 0)), (100, 110, 100, 255))

    def test_realFilterChain_viaTexEditor(self):
        tf = FRB.TextureFile("never_touched.dds")
        tf.img = Image.new("RGBA", (1, 1), (100, 100, 100, 255))

        editor = FRB.TexEditor(filters = [FRB.PixelFilter(transforms = [FRB.TintTransform(20)])])
        # exercise transform() the way TexEditor.fix() would call it -- via __call__
        pf = editor.filters[0]
        pf(tf)
        self.assertEqual(tf.img.getpixel((0, 0)), (100, 120, 100, 255))

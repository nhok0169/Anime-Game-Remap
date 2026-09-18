import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppBasePixelTransformTest(BaseUnitTest):
    """
    Tests for :class:`CppBasePixelTransform` and its pass-through pure-Python subclass
    :class:`BasePixelTransform`
    """

    def test_transform_isNoOp(self):
        t = FRB.CppBasePixelTransform()
        pixel = FRB.Colour(1, 2, 3, 4)
        t.transform(pixel, 0, 0)
        self.assertEqual(pixel.getTuple(), (1, 2, 3, 4))

    def test_call_dispatchesToTransform_evenForPurePythonOverride(self):
        calls = []

        class RecordingTransform(FRB.BasePixelTransform):
            def transform(self, pixel, x, y):
                calls.append((pixel.getTuple(), x, y))

        pixel = FRB.Colour(9, 9, 9, 9)
        RecordingTransform()(pixel, 3, 4)
        self.assertEqual(calls, [((9, 9, 9, 9), 3, 4)])

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.BasePixelTransform, FRB.CppBasePixelTransform))
        FRB.BasePixelTransform().transform(FRB.Colour(), 0, 0)

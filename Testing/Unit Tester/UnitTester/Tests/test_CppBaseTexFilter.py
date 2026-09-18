import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppBaseTexFilterTest(BaseUnitTest):
    """
    Tests for :class:`CppBaseTexFilter` and its pass-through pure-Python subclass :class:`BaseTexFilter`
    """

    def test_transform_isNoOp(self):
        filter = FRB.CppBaseTexFilter()
        tf = FRB.CppTextureFile("never_touched.dds")
        filter.transform(tf)  # should not raise

    def test_call_dispatchesToTransform_evenForPurePythonOverride(self):
        # Regression test: __call__ must resolve 'transform' via Python attribute lookup, not a
        # direct C++ virtual call -- otherwise a pure-Python subclass overriding only 'transform'
        # (not '__call__') would silently never actually run when invoked as filter(texFile).
        calls = []

        class RecordingFilter(FRB.BaseTexFilter):
            def transform(self, texFile):
                calls.append(texFile)

        f = RecordingFilter()
        tf = FRB.CppTextureFile("never_touched.dds")
        f(tf)
        self.assertEqual(calls, [tf])

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.BaseTexFilter, FRB.CppBaseTexFilter))
        filter = FRB.BaseTexFilter()
        filter.transform(FRB.CppTextureFile("never_touched.dds"))

import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppTexEditorTest(BaseUnitTest):
    """
    Tests for :class:`CppTexEditor` -- the pure-C++-SDK-facing engine behind :class:`TexEditor`.

    .. note::
        The Python-visible :class:`TexEditor` overrides ``fix`` entirely in Python (so its
        ``filters`` list can hold arbitrary Python callables), so :class:`CppTexEditor`'s own
        inherited ``fix`` is never actually reached through the Python-facing class -- see
        test_TexEditor.py for the real, exercised behaviour. This just confirms the C++ class
        itself constructs and inherits correctly
    """

    def test_construct_noArgs(self):
        editor = FRB.CppTexEditor()
        self.assertIsInstance(editor, FRB.CppBaseTexEditor)

    def test_isSubclassOfBaseTexEditor(self):
        self.assertTrue(issubclass(FRB.CppTexEditor, FRB.CppBaseTexEditor))

    def test_inheritedFix_isNoOp_whenCalledDirectly(self):
        # CppTexEditor holds no filters of its own from the Python side (TexEditor.py never passes
        # any into super().__init__()), so its inherited fix() is always a no-op if ever called.
        editor = FRB.CppTexEditor()
        tf = FRB.CppTextureFile("never_touched.dds")
        editor.fix(tf, "never_written.dds")

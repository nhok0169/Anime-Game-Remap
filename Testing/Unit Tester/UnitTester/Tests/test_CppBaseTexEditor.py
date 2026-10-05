import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppBaseTexEditorTest(BaseUnitTest):
    """
    Tests for :class:`CppBaseTexEditor` and its pass-through pure-Python subclass :class:`BaseTexEditor`
    """

    def test_fix_isNoOp(self):
        editor = FRB.CppBaseTexEditor()
        tf = FRB.CppTextureFile("this_file_should_never_be_touched.dds")
        editor.fix(tf, "also_never_written.dds")  # should not raise, should not touch tf

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.BaseTexEditor, FRB.CppBaseTexEditor))
        editor = FRB.BaseTexEditor()
        editor.fix(FRB.CppTextureFile("never_touched.dds"), "never_written.dds")

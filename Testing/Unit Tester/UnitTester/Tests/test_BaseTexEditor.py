import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class BaseTexEditorTest(BaseUnitTest):
    """
    Tests for :class:`BaseTexEditor` -- unlike most of the other pass-through wrappers ported in
    this codebase, this one now has real added behaviour of its own: ``engine`` and
    ``readPillowImg`` constructor flags (see test_CppBaseTexEditor.py for the mostly-unused, no-op
    ``fix`` it inherits)
    """

    def test_isSubclassOfCppBaseTexEditor(self):
        self.assertTrue(issubclass(FRB.BaseTexEditor, FRB.CppBaseTexEditor))

    def test_engine_defaultsToCompressonator(self):
        self.assertEqual(FRB.BaseTexEditor().engine, FRB.TexEngine.Compressonator)

    def test_engine_settableViaConstructor(self):
        self.assertEqual(FRB.BaseTexEditor(engine = FRB.TexEngine.Pillow).engine, FRB.TexEngine.Pillow)

    def test_readPillowImg_defaultsToFalse(self):
        self.assertFalse(FRB.BaseTexEditor().readPillowImg)

    def test_readPillowImg_settableViaConstructor(self):
        self.assertTrue(FRB.BaseTexEditor(readPillowImg = True).readPillowImg)

import struct
import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def _makePositionElements():
    return [FRB.BufElementType("POSITION", "R32G32B32_FLOAT", [FRB.BufFloat(), FRB.BufFloat(), FRB.BufFloat()])]


class BaseBufEditorTest(BaseUnitTest):
    """
    Tests for :class:`BaseBufEditor` -- the base class for editing some ``.buf`` file
    """

    def test_fix_noOpDefault(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line, _makePositionElements())
        editor = FRB.BaseBufEditor()

        result = editor.fix(bufFile, None)
        self.assertEqual(result, bytearray())

    def test_pythonSubclass_overridesFix_reachedThroughVirtualDispatch(self):
        # BaseBufEditor is a real pybind11 trampoline base -- a pure-Python subclass overriding
        # fix() must still be reached when called through the bound class, not silently ignored
        class CustomEditor(FRB.BaseBufEditor):
            def fix(self, bufFile, fixedBufFile = None):
                return "custom result"

        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line, _makePositionElements())
        editor = CustomEditor()

        self.assertEqual(editor.fix(bufFile, None), "custom result")

    def test_pythonSubclass_returningBytearray_convertedCorrectly(self):
        # The trampoline converts a Python override's return value back into the C++ FixResult by
        # hand (see PyBaseBufEditor.cpp) -- confirm the bytearray branch survives the round-trip
        class CustomEditor(FRB.BaseBufEditor):
            def fix(self, bufFile, fixedBufFile = None):
                return bytearray(b"\x01\x02\x03")

        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line, _makePositionElements())
        editor = CustomEditor()

        self.assertEqual(editor.fix(bufFile, None), bytearray(b"\x01\x02\x03"))

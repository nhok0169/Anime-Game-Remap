import os
import struct
import sys
import tempfile

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def _makePositionElements():
    return [FRB.BufElementType("POSITION", "R32G32B32_FLOAT", [FRB.BufFloat(), FRB.BufFloat(), FRB.BufFloat()])]


def _addOne(data, startInd, lineInd, lineSize):
    data["POSITION"][0] += 1.0
    return data


def _addTwo(data, startInd, lineInd, lineSize):
    data["POSITION"][0] += 2.0
    return data


class BufEditorTest(BaseUnitTest):
    """
    Tests for :class:`BufEditor` -- edits some ``.buf`` file by running a fixed sequence of
    filters over it (see test_BaseBufEditor.py for tests of the inherited base behaviour)
    """

    def test_isSubclassOfBaseBufEditor(self):
        self.assertTrue(issubclass(FRB.BufEditor, FRB.BaseBufEditor))

    def test_noFilters_matchesBufFileFixWithNoFilters(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)

        bufFile = FRB.CppBufFile(line, _makePositionElements())
        editor = FRB.BufEditor()
        result = editor.fix(bufFile, None)

        expectedBufFile = FRB.CppBufFile(line, _makePositionElements())
        expected = expectedBufFile.fix(fixedFile = None)

        self.assertEqual(result, expected)

    def test_filtersApplied_matchesBufFileFixWithSameFilters(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)

        bufFile = FRB.CppBufFile(line, _makePositionElements())
        editor = FRB.BufEditor(filters = [_addOne])
        result = editor.fix(bufFile, None)

        expectedBufFile = FRB.CppBufFile(line, _makePositionElements())
        expected = expectedBufFile.fix(fixedFile = None, filters = [_addOne])

        self.assertEqual(result, expected)

    def test_filters_defaultsToEmptyList(self):
        editor = FRB.BufEditor()
        self.assertEqual(editor.filters, [])

    def test_filters_propertyRoundTripsOriginalCallables(self):
        editor = FRB.BufEditor(filters = [_addOne])
        self.assertEqual(editor.filters, [_addOne])

        editor.filters = [_addTwo]
        self.assertEqual(editor.filters, [_addTwo])

        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line, _makePositionElements())
        result = editor.fix(bufFile, None)

        expectedBufFile = FRB.CppBufFile(line, _makePositionElements())
        expected = expectedBufFile.fix(fixedFile = None, filters = [_addTwo])
        self.assertEqual(result, expected)

    def test_fix_withRealFixedBufFilePath_writesFileAndReturnsPath(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line, _makePositionElements())
        editor = FRB.BufEditor(filters = [_addOne])

        with tempfile.TemporaryDirectory() as folder:
            outPath = os.path.join(folder, "out.buf")
            result = editor.fix(bufFile, outPath)

            self.assertEqual(result, outPath)
            self.assertTrue(os.path.exists(outPath))

    def test_pythonSubclass_overridesFix_reachedThroughVirtualDispatch(self):
        # Same trampoline concern as BaseBufEditorTest's own test, one level down the hierarchy --
        # a pure-Python subclass of BufEditor overriding fix() (rather than only setting .filters)
        # must still be reached, not silently fall back to the base's filter-running behaviour
        class CustomEditor(FRB.BufEditor):
            def fix(self, bufFile, fixedBufFile = None):
                return "custom result"

        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line, _makePositionElements())
        editor = CustomEditor(filters = [_addOne])

        self.assertEqual(editor.fix(bufFile, None), "custom result")

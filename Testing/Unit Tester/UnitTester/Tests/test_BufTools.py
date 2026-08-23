import sys
import struct

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def _makePositionElements():
    return [FRB.BufElementType("POSITION", "R32G32B32_FLOAT", [FRB.BufFloat(), FRB.BufFloat(), FRB.BufFloat()])]


class BufToolsTest(BaseUnitTest):
    """
    Tests for :class:`BufTools`
    """

    # ================================================
    # =================== toDataFrame ================

    def test_toDataFrame_correctShapeAndValues(self):
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)
        bufFile = FRB.BufFile(line1 + line2, _makePositionElements())

        df = FRB.BufTools.toDataFrame(bufFile)

        self.assertEqual(len(df), 2)
        self.assertEqual(list(df.columns), [("POSITION", 0), ("POSITION", 1), ("POSITION", 2)])
        self.assertEqual(list(df.iloc[0]), [1.0, 2.0, 3.0])
        self.assertEqual(list(df.iloc[1]), [4.0, 5.0, 6.0])

    def test_toDataFrame_emptyData_emptyDataFrame(self):
        bufFile = FRB.BufFile(b"", _makePositionElements())
        df = FRB.BufTools.toDataFrame(bufFile)
        self.assertEqual(len(df), 0)

    def test_toDataFrame_mixedElements_correctColumns(self):
        line = struct.pack("<fi", 1.5, -5)
        bufFile = FRB.BufFile(line, [FRB.BufElementType("WEIGHT", "fmt", [FRB.BufFloat()]), FRB.BufElementType("INDEX", "fmt", [FRB.BufSignedInt()])])

        df = FRB.BufTools.toDataFrame(bufFile)

        self.assertEqual(list(df.columns), [("INDEX", 0), ("WEIGHT", 0)])
        self.assertAlmostEqual(df.iloc[0][("WEIGHT", 0)], 1.5, places = 4)
        self.assertEqual(df.iloc[0][("INDEX", 0)], -5)

    # ================================================
    # ============ BufFile.toDataFrame ===============

    def test_bufFileToDataFrame_matchesBufTools(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.BufFile(line, _makePositionElements())

        self.assertTrue(bufFile.toDataFrame().equals(FRB.BufTools.toDataFrame(bufFile)))

    # ================================================
    # ============ works for CppBufFile too ==========

    def test_toDataFrame_cppBufFile_matchesBufFileResult(self):
        # BufFile is a thin subclass of CppBufFile -- confirms the shared free function produces
        # identical results whether it's handed the subclass or the impl base directly (both now
        # take the exact same, bare-named BufElementType/BufFloat, unlike before the
        # BufDataType/BufElementType family was fully replaced -- see the migration this test
        # module survived).
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)

        pyDf = FRB.BufTools.toDataFrame(FRB.BufFile(line1 + line2, _makePositionElements()))
        cppDf = FRB.BufTools.toDataFrame(FRB.CppBufFile(line1 + line2, _makePositionElements()))

        self.assertTrue(pyDf.equals(cppDf))

    # ================================================

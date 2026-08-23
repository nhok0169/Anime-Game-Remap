import struct
import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def _makePositionElements():
    return [FRB.BufElementType("POSITION", "R32G32B32_FLOAT", [FRB.BufFloat(), FRB.BufFloat(), FRB.BufFloat()])]


class BufFileTest(BaseUnitTest):
    """
    Tests for :class:`BufFile` -- the thin pure-Python subclass of :class:`CppBufFile` (see
    test_CppBufFile.py for tests of the inherited behaviour itself)
    """

    def test_isSubclassOfCppBufFile(self):
        self.assertTrue(issubclass(FRB.BufFile, FRB.CppBufFile))

    def test_constructorMatchesCppBufFileSignature(self):
        # BufFile defines no __init__ of its own -- CppBufFile's constructor is inherited
        # directly, so this is really confirming that inheritance chain resolves correctly
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.BufFile(line, _makePositionElements(), fileType = "Position.buf")
        self.assertEqual(bufFile.fileType, "Position.buf")
        self.assertIsInstance(bufFile, FRB.CppBufFile)

    def test_inheritedMethods_work(self):
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)
        bufFile = FRB.BufFile(line1 + line2, _makePositionElements())

        self.assertEqual(bufFile.bytesPerLine, 12)
        self.assertTrue(bufFile.isValid())

        decoded = bufFile.decodeLine(line1)
        self.assertEqual(bufFile.encodeLine(decoded), line1)

        fixed = bufFile.fix()
        self.assertEqual(bytes(fixed), line1 + line2)

    # ================================================
    # =================== toDataFrame ================

    def test_toDataFrame_matchesBufTools(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.BufFile(line, _makePositionElements())

        self.assertTrue(bufFile.toDataFrame().equals(FRB.BufTools.toDataFrame(bufFile)))

    # ================================================

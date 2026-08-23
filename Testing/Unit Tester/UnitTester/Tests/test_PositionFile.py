import struct
import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class PositionFileTest(BaseUnitTest):
    """
    Tests for :class:`PositionFile` -- the thin pure-Python subclass of :class:`CppPositionFile`
    (see test_CppPositionFile.py for tests of the inherited behaviour itself)
    """

    def _makeLine(self):
        return struct.pack("<10f", *range(10))

    def test_isSubclassOfCppPositionFile(self):
        self.assertTrue(issubclass(FRB.PositionFile, FRB.CppPositionFile))
        self.assertTrue(issubclass(FRB.PositionFile, FRB.CppBufFile))

    def test_defaultElements_correctLayout(self):
        posFile = FRB.PositionFile(self._makeLine())

        self.assertEqual(posFile.bytesPerLine, 40)
        self.assertEqual(posFile.fileType, "Position.buf")
        self.assertEqual([element.name for element in posFile.elements], ["POSITION", "NORMAL", "TANGENT"])

    def test_inheritedDecodeEncodeLine_work(self):
        line = self._makeLine()
        posFile = FRB.PositionFile(line)

        decoded = posFile.decodeLine(line)
        self.assertEqual(set(decoded.keys()), {"POSITION", "NORMAL", "TANGENT"})
        self.assertEqual(posFile.encodeLine(decoded), line)

import struct
import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class PositionFileTest(BaseUnitTest):
    """
    Tests for :class:`PositionFile` -- used for handling ``Position.buf`` files. Bound directly as
    a real pybind11 class (previously a thin pure-Python subclass of a separate ``CppPositionFile``
    -- the two were merged into this one class, per the "outcome 1 collapsing into outcome 2" case
    in Architecture/CLAUDE.md, since the pure-Python side added no behaviour of its own)
    """

    def _makeLine(self):
        # 3 (position) + 3 (normal) + 4 (tangent) = 10 floats/40 bytes
        return struct.pack("<10f", *range(10))

    # ================================================
    # ================= elements ======================

    def test_defaultElements_correctLayout(self):
        posFile = FRB.PositionFile(self._makeLine())

        self.assertEqual(posFile.bytesPerLine, 40)
        self.assertEqual(posFile.fileType, "Position.buf")

        elementNames = [element.name for element in posFile.elements]
        self.assertEqual(elementNames, ["POSITION", "NORMAL", "TANGENT"])

        elementSizes = [element.size for element in posFile.elements]
        self.assertEqual(elementSizes, [12, 12, 16])

    def test_isInstanceOfBufFile(self):
        self.assertIsInstance(FRB.PositionFile(self._makeLine()), FRB.CppBufFile)

    # ================================================
    # ============= decodeLine/encodeLine ============

    def test_decodeLine_correctValues(self):
        line = self._makeLine()
        posFile = FRB.PositionFile(line)

        decoded = posFile.decodeLine(line)
        self.assertEqual(set(decoded.keys()), {"POSITION", "NORMAL", "TANGENT"})

        for result, expected in zip(decoded["POSITION"], [0.0, 1.0, 2.0]):
            self.assertAlmostEqual(result, expected, places = 4)
        for result, expected in zip(decoded["NORMAL"], [3.0, 4.0, 5.0]):
            self.assertAlmostEqual(result, expected, places = 4)
        for result, expected in zip(decoded["TANGENT"], [6.0, 7.0, 8.0, 9.0]):
            self.assertAlmostEqual(result, expected, places = 4)

    def test_encodeLine_roundTrip(self):
        line = self._makeLine()
        posFile = FRB.PositionFile(line)

        decoded = posFile.decodeLine(line)
        self.assertEqual(posFile.encodeLine(decoded), line)

import os
import sys
import struct
import tempfile

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def _makePositionElements():
    return [FRB.BufElementType("POSITION", "R32G32B32_FLOAT", [FRB.BufFloat(), FRB.BufFloat(), FRB.BufFloat()])]


class CppBufFileTest(BaseUnitTest):
    """
    Tests for :class:`CppBufFile` -- the C++-backed impl base behind the pure-Python
    :class:`BufFile` (see test_BufFile.py for tests specific to that subclass)
    """

    # ================================================
    # ================ elements ======================

    def test_bytesPerLine_sumOfElementSizes(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line, _makePositionElements())
        self.assertEqual(bufFile.bytesPerLine, 12)

    def test_duplicateElementNames_suffixedKeys(self):
        # mirrors BufFile's own elementsInd/elementsDict naming scheme: the first occurrence of a
        # name keeps it bare, later occurrences get a numeric suffix
        line = struct.pack("<2f", 1.0, 2.0)
        elements = [FRB.BufElementType("X", "fmt", [FRB.BufFloat()]), FRB.BufElementType("X", "fmt", [FRB.BufFloat()])]
        bufFile = FRB.CppBufFile(line, elements)

        decoded = bufFile.decodeLine(line)
        self.assertEqual(set(decoded.keys()), {"X", "X1"})
        self.assertAlmostEqual(decoded["X"][0], 1.0, places = 4)
        self.assertAlmostEqual(decoded["X1"][0], 2.0, places = 4)

    def test_elements_matchConstructedTypes(self):
        bufFile = FRB.CppBufFile(struct.pack("<3f", 1.0, 2.0, 3.0), _makePositionElements())
        elements = bufFile.elements
        self.assertEqual(len(elements), 1)
        self.assertIsInstance(elements[0], FRB.BufElementType)
        self.assertEqual(elements[0].name, "POSITION")

    def test_elementsCloned_originalRemainsUsable(self):
        # Same clone-not-disown contract as BufElementType's own dataTypes -- see
        # test_BufElementType.py's identical test one hierarchy level down for why.
        element = FRB.BufElementType("X", "fmt", [FRB.BufFloat()])
        FRB.CppBufFile(b"\x00\x00\x00\x00", [element])

        self.assertEqual(element.size, 4)

    def test_fileType_defaultAndCustom(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        self.assertEqual(FRB.CppBufFile(line, _makePositionElements()).fileType, "Buffer")
        self.assertEqual(FRB.CppBufFile(line, _makePositionElements(), fileType = "Position.buf").fileType, "Position.buf")

    # ================================================
    # =================== isValid ====================

    def test_dataDivisibleByBytesPerLine_isValid(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line + line, _makePositionElements())
        self.assertTrue(bufFile.isValid())

    # ================================================
    # ============= decodeLine/encodeLine ============

    def test_decodeLine_correctValues(self):
        line = struct.pack("<3f", 1.0, -2.0, 3.5)
        bufFile = FRB.CppBufFile(line, _makePositionElements())

        decoded = bufFile.decodeLine(line)
        self.assertEqual(list(decoded.keys()), ["POSITION"])
        for result, expected in zip(decoded["POSITION"], [1.0, -2.0, 3.5]):
            self.assertAlmostEqual(result, expected, places = 4)

    def test_encodeLine_roundTrip(self):
        line = struct.pack("<3f", 1.0, -2.0, 3.5)
        bufFile = FRB.CppBufFile(line, _makePositionElements())

        decoded = bufFile.decodeLine(line)
        self.assertEqual(bufFile.encodeLine(decoded), line)

    # ================================================
    # ==================== fix =======================

    def test_fixNoFilters_returnsUnchangedData(self):
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)
        bufFile = FRB.CppBufFile(line1 + line2, _makePositionElements())

        result = bufFile.fix()
        self.assertIsInstance(result, bytearray)
        self.assertEqual(bytes(result), line1 + line2)

    def test_fixWithFilter_transformApplied(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)

        def doubleFirst(data, startInd, lineInd, lineSize):
            data["POSITION"][0] *= 2
            return data

        bufFile = FRB.CppBufFile(line, _makePositionElements())
        result = bufFile.fix(filters = [doubleFirst])
        self.assertEqual(struct.unpack("<3f", bytes(result)), (2.0, 2.0, 3.0))

    def test_fixMultipleFilters_appliedInOrder(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)

        def addOne(data, startInd, lineInd, lineSize):
            data["POSITION"][0] += 1
            return data

        def timesTen(data, startInd, lineInd, lineSize):
            data["POSITION"][0] *= 10
            return data

        bufFile = FRB.CppBufFile(line, _makePositionElements())
        result = bufFile.fix(filters = [addOne, timesTen])
        self.assertAlmostEqual(struct.unpack("<3f", bytes(result))[0], 20.0, places = 4)

    def test_fixWithFixedFile_writesFileAndReturnsPath(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line, _makePositionElements())

        outPath = os.path.join(tempfile.gettempdir(), "test_CppBufFile_fixOutput.buf")
        try:
            result = bufFile.fix(fixedFile = outPath)
            self.assertEqual(result, outPath)

            with open(outPath, "rb") as f:
                self.assertEqual(f.read(), line)
        finally:
            if (os.path.isfile(outPath)):
                os.remove(outPath)

    def test_fixLineIndArgument_isFloatTrueDivision(self):
        # preserves the pure-Python original's own quirk: 'lineInd' is i / bytesPerLine (a float),
        # not an integer index
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        seenLineInds = []

        def recordLineInd(data, startInd, lineInd, lineSize):
            seenLineInds.append(lineInd)
            return data

        bufFile = FRB.CppBufFile(line + line, _makePositionElements())
        bufFile.fix(filters = [recordLineInd])

        self.assertEqual(seenLineInds, [0.0, 1.0])
        for lineInd in seenLineInds:
            self.assertIsInstance(lineInd, float)

    # ================================================
    # ================ exceptions ====================

    def test_badBufData_bytesSrcWrongSize_raisesRealBadBufDataClass(self):
        with self.assertRaises(FRB.BadBufData) as ctx:
            FRB.CppBufFile(b"\x00\x00\x00", _makePositionElements())
        self.assertEqual(type(ctx.exception), FRB.BadBufData)

    def test_bufFileNotRecognized_fileWrongSize_raisesRealClass(self):
        with tempfile.NamedTemporaryFile(delete = False) as f:
            f.write(b"\x00" * 5)
            path = f.name

        try:
            with self.assertRaises(FRB.BufFileNotRecognized) as ctx:
                FRB.CppBufFile(path, _makePositionElements())
            self.assertEqual(type(ctx.exception), FRB.BufFileNotRecognized)
        finally:
            os.remove(path)

    def test_missingFile_raisesRuntimeErrorNotBufFileNotRecognized(self):
        # A genuinely missing file surfaces the same way open() would in the pure-Python original
        # (a plain file-not-found style error) -- BufFileNotRecognized is reserved for a file that
        # opened fine but has the wrong byte length.
        with self.assertRaises(RuntimeError) as ctx:
            FRB.CppBufFile("this_buf_file_should_never_exist_54321.buf", _makePositionElements())
        self.assertNotIsInstance(ctx.exception, FRB.BufFileNotRecognized)

    # ================================================

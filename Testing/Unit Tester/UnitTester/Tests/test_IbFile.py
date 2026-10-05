import struct
import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def _makeIbData(*triangles):
    return b"".join(struct.pack("<3I", *triangle) for triangle in triangles)


class IbFileTest(BaseUnitTest):
    """
    Tests for :class:`IbFile` -- a .ib (index buffer) file, and its dumped *ib.txt* text
    """

    # ================================================
    # =================== elements ===================

    def test_defaultElements_oneTriangleElementOf3UInts(self):
        ibFile = FRB.IbFile(_makeIbData((0, 1, 2)))

        self.assertEqual(ibFile.fileType, "Ib")
        self.assertEqual(ibFile.bytesPerLine, 12)
        self.assertEqual([element.name for element in ibFile.elements], ["Triangle"])
        self.assertEqual(ibFile.elements[0].formatName, "R32G32B32_UINT")

    def test_isSubclassOfCppBufFile(self):
        # a real pybind11 class now, deriving from the C++ base the same way BlendFile/PositionFile
        # do -- so it no longer picks up the pure-Python BufFile's toDataFrame/fromDataFrame, which
        # are reachable through BufTools for any .buf file anyway
        self.assertTrue(issubclass(FRB.IbFile, FRB.CppBufFile))

    # ================================================
    # ============ triangle/index counts =============

    def test_getTriangleCount_oneLinePerFace(self):
        self.assertEqual(FRB.IbFile(_makeIbData((0, 1, 2), (3, 4, 5))).getTriangleCount(), 2)

    def test_getIndexCount_threePerTriangle(self):
        self.assertEqual(FRB.IbFile(_makeIbData((0, 1, 2), (3, 4, 5))).getIndexCount(), 6)

    def test_emptyFile_zeroCounts(self):
        ibFile = FRB.IbFile(b"")
        self.assertEqual(ibFile.getTriangleCount(), 0)
        self.assertEqual(ibFile.getIndexCount(), 0)

    # ================================================
    # ================= makeDumpHeader ===============

    def test_makeDumpHeader_matchesTheDumpFormat(self):
        ibFile = FRB.IbFile(_makeIbData((0, 1, 2), (3, 4, 5)))

        self.assertEqual(ibFile.makeDumpHeader(30), """byte offset: 0
first index: 30
index count: 6
topology: trianglelist
format: DXGI_FORMAT_R16_UINT
""")

    def test_makeDumpHeader_firstIndexDefaultsToZero(self):
        ibFile = FRB.IbFile(_makeIbData((0, 1, 2)))
        self.assertIn("first index: 0", ibFile.makeDumpHeader())

    # ================================================
    # =================== getDumpStr =================

    def test_getDumpStr_headerThenSpaceSeparatedFaces(self):
        ibFile = FRB.IbFile(_makeIbData((0, 1, 2), (3, 4, 5)))

        self.assertEqual(ibFile.getDumpStr(), """byte offset: 0
first index: 0
index count: 6
topology: trianglelist
format: DXGI_FORMAT_R16_UINT

0 1 2
3 4 5
""")

    def test_getDumpStr_firstIndexCarriedIntoTheHeader(self):
        # a mod's faces span several .ib files, which a dump numbers continuously
        ibFile = FRB.IbFile(_makeIbData((0, 1, 2)))
        self.assertIn("first index: 42", ibFile.getDumpStr(42))

    def test_getDumpStr_emptyFile_headerOnly(self):
        result = FRB.IbFile(b"").getDumpStr()

        self.assertIn("index count: 0", result)
        self.assertTrue(result.endswith("\n\n"))

    # ================================================
    # ================= readDumpStr ==================

    def test_readDumpStr_roundTripsGetDumpStr(self):
        data = _makeIbData((0, 1, 2), (3, 4, 5))
        source = FRB.IbFile(data)

        rebuilt = FRB.IbFile(b"")
        rebuilt.readDumpStr(source.getDumpStr())

        self.assertEqual(rebuilt.data, data)
        self.assertEqual(rebuilt.getTriangleCount(), 2)

    def test_readDumpStr_skipsTheHeader(self):
        rebuilt = FRB.IbFile(b"")
        rebuilt.readDumpStr("""byte offset: 0
first index: 30
index count: 6
topology: trianglelist
format: DXGI_FORMAT_R16_UINT

0 1 2
3 4 5
""")

        self.assertEqual(rebuilt.data, _makeIbData((0, 1, 2), (3, 4, 5)))

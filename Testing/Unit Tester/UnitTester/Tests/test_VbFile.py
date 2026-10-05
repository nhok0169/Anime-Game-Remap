import struct
import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def _makeElements():
    return [FRB.BufElementTypes.PositionFloatRGB.value,
            FRB.BufElementTypes.TextureCoordinateRG.value,
            FRB.BufElementTypes.TextureCoordinateRG.value]


def _makeVertex(position, texcoord1, texcoord2):
    return struct.pack("<3f", *position) + struct.pack("<2f", *texcoord1) + struct.pack("<2f", *texcoord2)


class VbFileTest(BaseUnitTest):
    """
    Tests for :class:`VbFile` -- a .vb (vertex buffer) file, and its dumped *vb.txt* text
    """

    def test_isSubclassOfCppBufFile(self):
        # a real pybind11 class now, deriving from the C++ base the same way BlendFile/PositionFile
        # do -- so it no longer picks up the pure-Python BufFile's toDataFrame/fromDataFrame, which
        # are reachable through BufTools for any .buf file anyway
        self.assertTrue(issubclass(FRB.VbFile, FRB.CppBufFile))

    def test_bufToolsStillWorkOnIt(self):
        vbFile = FRB.VbFile(_makeVertex((1.0, 2.0, 3.0), (0.25, 0.5), (0.75, 0.125)), _makeElements())

        self.assertEqual(len(FRB.BufTools.toDataFrame(vbFile)), 1)
        self.assertIn("vb0[0]+000 POSITION: 1, 2, 3", FRB.BufTools.getDumpStr(vbFile))

    def test_layout_fileTypeAndStride(self):
        vbFile = FRB.VbFile(_makeVertex((1.0, 2.0, 3.0), (0.25, 0.5), (0.75, 0.125)), _makeElements())

        self.assertEqual(vbFile.fileType, "Vb")
        self.assertEqual(vbFile.bytesPerLine, 28)   # POSITION (12) + TEXCOORD (8) + TEXCOORD (8)
        self.assertEqual(vbFile.getVertexCount(), 1)

    def test_getVertexCount_emptyFile_zero(self):
        self.assertEqual(FRB.VbFile(b"", _makeElements()).getVertexCount(), 0)

    # ================================================
    # ================= makeDumpHeader ===============

    def test_makeDumpHeader_matchesTheDumpFormat(self):
        vbFile = FRB.VbFile(_makeVertex((1.0, 2.0, 3.0), (0.25, 0.5), (0.75, 0.125)), _makeElements())

        self.assertEqual(vbFile.makeDumpHeader(), """stride: 28
first vertex: 0
vertex count: 1
topology: trianglelist
element[0]:
  SemanticName: POSITION
  SemanticIndex: 0
  Format: R32G32B32_FLOAT
  InputSlot: 0
  AlignedByteOffset: 0
  InputSlotClass: per-vertex
  InstanceDataStepRate: 0
element[1]:
  SemanticName: TEXCOORD
  SemanticIndex: 0
  Format: R32G32_FLOAT
  InputSlot: 0
  AlignedByteOffset: 12
  InputSlotClass: per-vertex
  InstanceDataStepRate: 0
element[2]:
  SemanticName: TEXCOORD
  SemanticIndex: 1
  Format: R32G32_FLOAT
  InputSlot: 0
  AlignedByteOffset: 20
  InputSlotClass: per-vertex
  InstanceDataStepRate: 0

vertex-data:

""")

    def test_makeDumpHeader_repeatedName_getsItsOwnSemanticIndex(self):
        # 3dmigoto tells several same-named elements apart by their SemanticIndex
        vbFile = FRB.VbFile(_makeVertex((1.0, 2.0, 3.0), (0.25, 0.5), (0.75, 0.125)), _makeElements())
        header = vbFile.makeDumpHeader()

        self.assertEqual(header.count("SemanticName: TEXCOORD"), 2)
        self.assertIn("SemanticIndex: 1", header)

    # ================================================
    # =================== getDumpStr =================

    def test_getDumpStr_isTheHeaderPlusTheParentsDataSection(self):
        vbFile = FRB.VbFile(_makeVertex((1.0, 2.0, 3.0), (0.25, 0.5), (0.75, 0.125)), _makeElements())

        self.assertEqual(vbFile.getDumpStr(), vbFile.makeDumpHeader() + FRB.BufFile.getDumpStr(vbFile))

    def test_getDumpStr_dataSection(self):
        vbFile = FRB.VbFile(_makeVertex((1.0, 2.0, 3.0), (0.25, 0.5), (0.75, 0.125)) +
                            _makeVertex((4.0, 5.0, 6.0), (0.5, 0.5), (0.25, 0.25)), _makeElements())

        result = vbFile.getDumpStr()
        data = result[result.index("vertex-data:\n\n") + len("vertex-data:\n\n"):]

        self.assertEqual(data, """vb0[0]+000 POSITION: 1, 2, 3
vb0[0]+012 TEXCOORD: 0.25, 0.5
vb0[0]+020 TEXCOORD1: 0.75, 0.125

vb0[1]+000 POSITION: 4, 5, 6
vb0[1]+012 TEXCOORD: 0.5, 0.5
vb0[1]+020 TEXCOORD1: 0.25, 0.25
""")

    def test_getDumpStr_prefixIsConfigurable(self):
        vbFile = FRB.VbFile(_makeVertex((1.0, 2.0, 3.0), (0.25, 0.5), (0.75, 0.125)), _makeElements())
        self.assertIn("vb1[0]+000 POSITION:", vbFile.getDumpStr(prefix = "vb1"))

    # ================================================
    # ============ built from a merge ================

    def test_builtByMerging_separateBufFiles(self):
        # the real use case: a GI character's .vb data is split across a Position.buf, a Blend.buf
        # and a Texcoord.buf, one line each per vertex
        positionFile = FRB.PositionFile(struct.pack("<10f", *range(10)) + struct.pack("<10f", *range(10, 20)))
        blendFile = FRB.BlendFile(struct.pack("<4f", 1.0, 0.0, 0.0, 0.0) + struct.pack("<4i", 5, 0, 0, 0) +
                                  struct.pack("<4f", 0.5, 0.5, 0.0, 0.0) + struct.pack("<4i", 6, 7, 0, 0))

        vbFile = FRB.VbFile(b"", [])
        vbFile.merge([positionFile, blendFile])

        self.assertEqual(vbFile.getVertexCount(), 2)
        self.assertEqual(vbFile.bytesPerLine, 40 + 32)
        self.assertEqual([element.name for element in vbFile.elements],
                         ["POSITION", "NORMAL", "TANGENT", "BLENDWEIGHT", "BLENDINDICES"])

    # ================================================
    # ================= readDumpStr ==================

    def test_readDumpStr_rebuildsElementsFromTheHeader(self):
        # a dump names each element and gives its DXGI format, so it can be read back without being
        # told what the layout was
        source = FRB.VbFile(_makeVertex((1.0, 2.0, 3.0), (0.25, 0.5), (0.75, 0.125)) +
                            _makeVertex((4.0, 5.0, 6.0), (0.5, 0.5), (0.25, 0.25)), _makeElements())
        text = source.getDumpStr()

        rebuilt = FRB.VbFile(b"", [])
        rebuilt.readDumpStr(text)

        self.assertEqual([element.name for element in rebuilt.elements], ["POSITION", "TEXCOORD", "TEXCOORD"])
        self.assertEqual([element.formatName for element in rebuilt.elements],
                         ["R32G32B32_FLOAT", "R32G32_FLOAT", "R32G32_FLOAT"])
        self.assertEqual(rebuilt.bytesPerLine, 28)
        self.assertEqual(rebuilt.data, source.data)
        self.assertEqual(rebuilt.getDumpStr(), text)

    def test_parseFormatName_channelsAndSuffix(self):
        self.assertEqual(len(FRB.VbFile.parseFormatName("R32G32B32_FLOAT")), 3)
        self.assertEqual(len(FRB.VbFile.parseFormatName("R32G32B32A32_SINT")), 4)
        self.assertEqual(len(FRB.VbFile.parseFormatName("R8G8B8A8_UNORM")), 4)
        self.assertEqual(len(FRB.VbFile.parseFormatName("R32G32_FLOAT")), 2)

        self.assertEqual([dataType.size for dataType in FRB.VbFile.parseFormatName("R8G8B8A8_UNORM")], [1, 1, 1, 1])
        self.assertEqual([dataType.size for dataType in FRB.VbFile.parseFormatName("R32G32B32_FLOAT")], [4, 4, 4])

    def test_parseFormatName_unknownFormat_empty(self):
        self.assertEqual(FRB.VbFile.parseFormatName("SOMETHING_ELSE"), [])

    def test_parseDumpHeader_noHeader_none(self):
        self.assertIsNone(FRB.VbFile.parseDumpHeader("vb0[0]+000 POSITION: 1, 2, 3\n"))

    def test_readDumpStr_headerlessText_usesTheCurrentElements(self):
        vbFile = FRB.VbFile(b"", _makeElements())
        vbFile.readDumpStr("""vb0[0]+000 POSITION: 1, 2, 3
vb0[0]+012 TEXCOORD: 0.25, 0.5
vb0[0]+020 TEXCOORD1: 0.75, 0.125
""")

        self.assertEqual(vbFile.getVertexCount(), 1)
        self.assertEqual(vbFile.data, _makeVertex((1.0, 2.0, 3.0), (0.25, 0.5), (0.75, 0.125)))

    def test_builtByMerging_dumpsTheMergedLayout(self):
        positionFile = FRB.PositionFile(struct.pack("<10f", *range(10)))
        blendFile = FRB.BlendFile(struct.pack("<4f", 1.0, 0.0, 0.0, 0.0) + struct.pack("<4i", 5, 0, 0, 0))

        vbFile = FRB.VbFile(b"", [])
        vbFile.merge([positionFile, blendFile])

        result = vbFile.getDumpStr()

        self.assertIn("stride: 72", result)
        self.assertIn("vertex count: 1", result)
        self.assertIn("vb0[0]+000 POSITION: 0, 1, 2", result)
        self.assertIn("vb0[0]+040 BLENDWEIGHT: 1, 0, 0, 0", result)
        self.assertIn("vb0[0]+056 BLENDINDICES: 5, 0, 0, 0", result)

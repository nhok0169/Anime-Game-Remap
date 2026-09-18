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


def _makeTexcoordElement():
    return FRB.BufElementType("TEXCOORD", "R32G32_FLOAT", [FRB.BufFloat(), FRB.BufFloat()])


def _makeFloatElement(name):
    return FRB.BufElementType(name, "R32_FLOAT", [FRB.BufFloat()])


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
    # ============== decodeAll/encodeAll =============

    def test_decodeAll_oneArrayPerColumn_keyedByElementAndIndex(self):
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)
        bufFile = FRB.CppBufFile(line1 + line2, _makePositionElements())

        columns = bufFile.decodeAll()

        self.assertEqual(set(columns.keys()), {("POSITION", 0), ("POSITION", 1), ("POSITION", 2)})
        self.assertEqual(list(columns[("POSITION", 0)]), [1.0, 4.0])
        self.assertEqual(list(columns[("POSITION", 1)]), [2.0, 5.0])
        self.assertEqual(list(columns[("POSITION", 2)]), [3.0, 6.0])

    def test_decodeAll_matchesDecodeLinePerLine(self):
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)
        bufFile = FRB.CppBufFile(line1 + line2, _makePositionElements())

        columns = bufFile.decodeAll()

        for lineInd, line in enumerate([line1, line2]):
            decoded = bufFile.decodeLine(line)
            for valueInd, value in enumerate(decoded["POSITION"]):
                self.assertAlmostEqual(columns[("POSITION", valueInd)][lineInd], value, places = 4)

    def test_decodeAll_dtypeFollowsTheDataType(self):
        # an integer data type must come back integral rather than widened to a float
        line = struct.pack("<fiI", 1.5, -5, 7)
        bufFile = FRB.CppBufFile(line, [FRB.BufElementType("F", "fmt", [FRB.BufFloat()]),
                                        FRB.BufElementType("I", "fmt", [FRB.BufSignedInt()]),
                                        FRB.BufElementType("U", "fmt", [FRB.BufUnSignedInt()])])

        columns = bufFile.decodeAll()

        self.assertEqual(columns[("F", 0)].dtype.kind, "f")
        self.assertEqual(columns[("I", 0)].dtype.kind, "i")
        self.assertEqual(columns[("U", 0)].dtype.kind, "u")

    def test_decodeAll_emptyData_emptyColumns(self):
        bufFile = FRB.CppBufFile(b"", _makePositionElements())

        columns = bufFile.decodeAll()

        self.assertEqual(set(columns.keys()), {("POSITION", 0), ("POSITION", 1), ("POSITION", 2)})
        self.assertEqual(len(columns[("POSITION", 0)]), 0)

    def test_encodeAll_roundTripsDecodeAll(self):
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)
        bufFile = FRB.CppBufFile(line1 + line2, _makePositionElements())

        bufFile.encodeAll(bufFile.decodeAll())

        self.assertEqual(bufFile.data, line1 + line2)

    def test_encodeAll_columnOrderDoesNotMatter(self):
        # the columns are matched by their (elementKey, indexWithinElement) key, not by position
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line, _makePositionElements())

        columns = bufFile.decodeAll()
        bufFile.encodeAll({key: columns[key] for key in reversed(list(columns.keys()))})

        self.assertEqual(bufFile.data, line)

    def test_encodeAll_missingColumn_encodesAsZero(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line, _makePositionElements())

        columns = bufFile.decodeAll()
        del columns[("POSITION", 1)]
        bufFile.encodeAll(columns)

        self.assertEqual(bufFile.decodeLine(bufFile.data)["POSITION"], [1.0, 0.0, 3.0])

    def test_encodeAll_noColumns_emptiesData(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line, _makePositionElements())

        bufFile.encodeAll({})

        self.assertEqual(bufFile.data, b"")

    # ================================================
    # ==================== merge =====================

    def test_merge_stitchesLineByLineAndConcatenatesElements(self):
        posLine1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        posLine2 = struct.pack("<3f", 4.0, 5.0, 6.0)
        texLine1 = struct.pack("<2f", 0.25, 0.5)
        texLine2 = struct.pack("<2f", 0.5, 0.5)

        posFile = FRB.CppBufFile(posLine1 + posLine2, [FRB.BufElementTypes.PositionFloatRGB.value])
        texFile = FRB.CppBufFile(texLine1 + texLine2, [FRB.BufElementTypes.TextureCoordinateRG.value])

        merged = FRB.CppBufFile(b"", [])
        merged.merge([posFile, texFile])

        self.assertEqual(merged.data, posLine1 + texLine1 + posLine2 + texLine2)
        self.assertEqual(merged.bytesPerLine, 20)
        self.assertEqual([element.name for element in merged.elements], ["POSITION", "TEXCOORD"])

    def test_merge_leavesItsSourcesUsable(self):
        # a source's elements are deep-copied in, matching BufElementType's shareable-value contract
        posLine = struct.pack("<3f", 1.0, 2.0, 3.0)
        posFile = FRB.CppBufFile(posLine, [FRB.BufElementTypes.PositionFloatRGB.value])

        FRB.CppBufFile(b"", []).merge([posFile, posFile])

        self.assertEqual(posFile.data, posLine)
        self.assertEqual(posFile.bytesPerLine, 12)
        self.assertEqual([element.name for element in posFile.elements], ["POSITION"])

    def test_merge_raggedSources_truncatesToTheShortest(self):
        posFile = FRB.CppBufFile(struct.pack("<3f", 1.0, 2.0, 3.0) + struct.pack("<3f", 4.0, 5.0, 6.0),
                                 [FRB.BufElementTypes.PositionFloatRGB.value])
        texFile = FRB.CppBufFile(struct.pack("<2f", 0.25, 0.5), [FRB.BufElementTypes.TextureCoordinateRG.value])

        merged = FRB.CppBufFile(b"", [])
        merged.merge([posFile, texFile])

        self.assertEqual(len(merged.data), merged.bytesPerLine)

    def test_merge_nothing_emptiesTheFile(self):
        bufFile = FRB.CppBufFile(struct.pack("<3f", 1.0, 2.0, 3.0), _makePositionElements())
        bufFile.merge([])

        self.assertEqual(bufFile.data, b"")
        self.assertEqual(bufFile.elements, [])

    # ================================================
    # =================== append =====================

    def test_append_addsEveryLineOfEachSourceInOrder(self):
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)
        line3 = struct.pack("<3f", 7.0, 8.0, 9.0)

        bufFile = FRB.CppBufFile(line1, _makePositionElements())
        srcA = FRB.CppBufFile(line2, _makePositionElements())
        srcB = FRB.CppBufFile(line3, _makePositionElements())

        bufFile.append([srcA, srcB])

        self.assertEqual(bytes(bufFile.data), line1 + line2 + line3)
        self.assertEqual(bytes(srcA.data), line2)
        self.assertEqual(bytes(srcB.data), line3)

    def test_append_supersetSource_dropsTheExtraElements(self):
        # the whole point of the superset rule: a full .vb can be appended onto a Position.buf and
        # contributes only its positions
        position = struct.pack("<3f", 1.0, 2.0, 3.0)
        texcoord = struct.pack("<2f", 0.25, 0.5)

        bufFile = FRB.CppBufFile(b"", _makePositionElements())
        source = FRB.CppBufFile(position + texcoord, _makePositionElements() + [_makeTexcoordElement()])

        bufFile.append([source])

        self.assertEqual(bytes(bufFile.data), position)

    def test_append_supersetInADifferentOrder_rebuiltInThisFilesOrder(self):
        # the source carries this file's elements in the opposite order and with an extra one
        # wedged between them -- each appended line still comes out in *this* file's order
        bufFile = FRB.CppBufFile(b"", [_makeFloatElement("A"), _makeFloatElement("B")])
        source = FRB.CppBufFile(struct.pack("<3f", 20.0, 99.0, 10.0),
                                [_makeFloatElement("B"), _makeFloatElement("X"), _makeFloatElement("A")])

        bufFile.append([source])

        self.assertEqual(struct.unpack("<2f", bytes(bufFile.data)), (10.0, 20.0))

    def test_append_formatNameMayDiffer(self):
        # formatName is 3dmigoto's label for a layout, not the layout -- only the data types decide
        # whether two elements hold the same bytes
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(b"", _makePositionElements())
        source = FRB.CppBufFile(line, [FRB.BufElementType("POSITION", "SOME_OTHER_NAME",
                                                          [FRB.BufFloat(), FRB.BufFloat(), FRB.BufFloat()])])

        bufFile.append([source])

        self.assertEqual(bytes(bufFile.data), line)

    def test_append_missingElement_raisesValueError(self):
        bufFile = FRB.CppBufFile(b"", _makePositionElements() + [_makeTexcoordElement()])
        source = FRB.CppBufFile(struct.pack("<3f", 1.0, 2.0, 3.0), _makePositionElements())

        with self.assertRaises(ValueError) as ctx:
            bufFile.append([source])
        self.assertIn("TEXCOORD", str(ctx.exception))

    def test_append_sameNameDifferentDataTypes_raisesValueError(self):
        # same element name AND the same byte size (3 x 4 bytes), so only comparing sizes would let
        # this through and silently reinterpret every float as an unsigned integer
        bufFile = FRB.CppBufFile(b"", _makePositionElements())
        source = FRB.CppBufFile(struct.pack("<3I", 1, 2, 3),
                                [FRB.BufElementType("POSITION", "R32G32B32_UINT",
                                                    [FRB.BufUnSignedInt(), FRB.BufUnSignedInt(), FRB.BufUnSignedInt()])])

        with self.assertRaises(ValueError) as ctx:
            bufFile.append([source])
        self.assertIn("POSITION", str(ctx.exception))

    def test_append_oneBadSource_leavesTheFileUntouched(self):
        # the first source is perfectly appendable -- every source is checked before any byte is
        # copied, so this file must not end up holding it
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line, _makePositionElements())
        good = FRB.CppBufFile(struct.pack("<3f", 4.0, 5.0, 6.0), _makePositionElements())
        bad = FRB.CppBufFile(struct.pack("<2f", 0.25, 0.5), [_makeTexcoordElement()])

        with self.assertRaises(ValueError):
            bufFile.append([good, bad])

        self.assertEqual(bytes(bufFile.data), line)

    def test_append_self_doublesTheFile(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line, _makePositionElements())

        bufFile.append([bufFile])

        self.assertEqual(bytes(bufFile.data), line + line)

    def test_append_nothing_leavesTheFileUnchanged(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line, _makePositionElements())

        bufFile.append([])

        self.assertEqual(bytes(bufFile.data), line)

    def test_append_emptySource_addsNothing(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line, _makePositionElements())

        bufFile.append([FRB.CppBufFile(b"", _makePositionElements())])

        self.assertEqual(bytes(bufFile.data), line)

    def test_append_noElements_doesNothing(self):
        # nothing to take out of a source line, and every source is trivially a superset of nothing
        bufFile = FRB.CppBufFile(b"", [])
        bufFile.append([FRB.CppBufFile(struct.pack("<3f", 1.0, 2.0, 3.0), _makePositionElements())])

        self.assertEqual(bytes(bufFile.data), b"")

    def test_append_duplicateElementNames_matchedByTheSuffixedKey(self):
        # X/X1 are matched by their occurrence-suffixed keys, so the source's second X lands in this
        # file's second X even with an unrelated element between them
        bufFile = FRB.CppBufFile(b"", [_makeFloatElement("X"), _makeFloatElement("X")])
        source = FRB.CppBufFile(struct.pack("<3f", 1.0, 99.0, 2.0),
                                [_makeFloatElement("X"), _makeFloatElement("Y"), _makeFloatElement("X")])

        bufFile.append([source])

        self.assertEqual(struct.unpack("<2f", bytes(bufFile.data)), (1.0, 2.0))

    def test_append_fileSrc_becomesBytesAndLeavesTheFileOnDisk(self):
        # same 'data is read-only, the write path is src + read()' shape as merge/filter
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)

        with tempfile.NamedTemporaryFile(delete = False, suffix = ".buf") as f:
            f.write(line1)
            path = f.name

        try:
            bufFile = FRB.CppBufFile(path, _makePositionElements())
            bufFile.append([FRB.CppBufFile(line2, _makePositionElements())])

            self.assertEqual(bytes(bufFile.data), line1 + line2)
            self.assertNotIsInstance(bufFile.src, str)

            with open(path, "rb") as f:
                self.assertEqual(f.read(), line1)
        finally:
            os.remove(path)

    # ================================================
    # ========= getDumpStr/getFlatDumpStr ============

    def test_getDumpStr_oneEntryPerElementPerLine(self):
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0) + struct.pack("<2f", 0.25, 0.5)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0) + struct.pack("<2f", 0.5, 0.5)
        bufFile = FRB.CppBufFile(line1 + line2, [FRB.BufElementTypes.PositionFloatRGB.value,
                                                 FRB.BufElementTypes.TextureCoordinateRG.value])

        self.assertEqual(bufFile.getDumpStr(), """vb0[0]+000 POSITION: 1, 2, 3
vb0[0]+012 TEXCOORD: 0.25, 0.5

vb0[1]+000 POSITION: 4, 5, 6
vb0[1]+012 TEXCOORD: 0.5, 0.5
""")

    def test_getDumpStr_numbersFormatLike3dmigoto(self):
        # 3dmigoto writes its dumps with C's "%.9g" -- 9 significant digits, trailing zeros
        # dropped, and no forced ".0" -- which is NOT Python's str/repr. Verified against a real
        # frame analysis dump (GI-Model-Importer-Assets' own HuTao files).
        values = [1.0, -1.0, 0.0, 0.5, 0.25, 2.0, 1234.5]
        expected = ["1", "-1", "0", "0.5", "0.25", "2", "1234.5"]

        bufFile = FRB.CppBufFile(b"".join(struct.pack("<f", value) for value in values),
                                 [FRB.BufElementType("F", "fmt", [FRB.BufFloat()])])

        dumped = [line.split(": ", 1)[1] for line in bufFile.getDumpStr().split("\n") if line]
        self.assertEqual(dumped, expected)

    def test_getDumpStr_unormRoundedThroughFloat32Like3dmigoto(self):
        # 3dmigoto holds a decoded UNORM as a 32 bit float, so 128/255 prints as 0.501960814 there
        # rather than the 0.501960784 a double would give
        bufFile = FRB.CppBufFile(bytes([128]), [FRB.BufElementType("COLOR", "R8_UNORM", [FRB.BufUnorm("UNORM8", 1)])])

        self.assertEqual(bufFile.getDumpStr(), "vb0[0]+000 COLOR: 0.501960814\n")

    def test_getDumpStr_prefixIsConfigurable(self):
        bufFile = FRB.CppBufFile(struct.pack("<3f", 1.0, 2.0, 3.0), _makePositionElements())
        self.assertTrue(bufFile.getDumpStr(prefix = "vb2").startswith("vb2[0]+000 POSITION:"))

    def test_getFlatDumpStr_everyValueOnOneLine(self):
        data = struct.pack("<3I", 0, 1, 2) + struct.pack("<3I", 3, 4, 5)
        bufFile = FRB.CppBufFile(data, [FRB.BufElementType("Triangle", "R32G32B32_UINT",
                                                            [FRB.BufDataTypes.UInt32.value] * 3)])

        self.assertEqual(bufFile.getFlatDumpStr(), "0 1 2\n3 4 5\n")
        self.assertEqual(bufFile.getFlatDumpStr(valueSep = ", "), "0, 1, 2\n3, 4, 5\n")

    def test_dumpStrs_emptyData_emptyString(self):
        bufFile = FRB.CppBufFile(b"", _makePositionElements())

        self.assertEqual(bufFile.getDumpStr(), "")
        self.assertEqual(bufFile.getFlatDumpStr(), "")

    # ================================================
    # ======== readDumpStr/readFlatDumpStr ===========

    def test_readDumpStr_roundTripsGetDumpStr(self):
        line1 = struct.pack("<3f", 1.25, 2.5, 3.75) + struct.pack("<2f", 0.25, 0.5)
        line2 = struct.pack("<3f", -4.5, 5.25, 6.125) + struct.pack("<2f", 0.75, 0.125)
        elements = [FRB.BufElementTypes.PositionFloatRGB.value, FRB.BufElementTypes.TextureCoordinateRG.value]

        bufFile = FRB.CppBufFile(line1 + line2, elements)
        text = bufFile.getDumpStr()

        emptied = FRB.CppBufFile(b"", elements)
        emptied.readDumpStr(text)

        self.assertEqual(emptied.data, line1 + line2)

    def test_readDumpStr_integerElements_roundTrip(self):
        line = struct.pack("<4i", 5, -7, 0, 12)
        elements = [FRB.BufElementTypes.BlendIndicesIntRGBA.value]

        bufFile = FRB.CppBufFile(line, elements)
        emptied = FRB.CppBufFile(b"", elements)
        emptied.readDumpStr(bufFile.getDumpStr())

        self.assertEqual(emptied.data, line)

    def test_readDumpStr_skipsAHeader(self):
        # a whole dump file can be handed in, not just the data section
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(b"", _makePositionElements())

        bufFile.readDumpStr("""stride: 12
first vertex: 0
vertex count: 1
topology: trianglelist

vertex-data:

vb0[0]+000 POSITION: 1, 2, 3
""")

        self.assertEqual(bufFile.data, line)

    def test_readDumpStr_shortBlockIsZeroFilled(self):
        # a block missing values still contributes a whole line, so the stride never breaks
        bufFile = FRB.CppBufFile(b"", _makePositionElements())
        bufFile.readDumpStr("vb0[0]+000 POSITION: 1\n")

        self.assertEqual(bufFile.data, struct.pack("<3f", 1.0, 0.0, 0.0))

    def test_readDumpStr_emptyText_emptiesTheFile(self):
        bufFile = FRB.CppBufFile(struct.pack("<3f", 1.0, 2.0, 3.0), _makePositionElements())
        bufFile.readDumpStr("")

        self.assertEqual(bufFile.data, b"")

    def test_readFlatDumpStr_roundTripsGetFlatDumpStr(self):
        data = struct.pack("<3I", 0, 1, 2) + struct.pack("<3I", 3, 4, 5)
        elements = [FRB.BufElementType("Triangle", "R32G32B32_UINT", [FRB.BufDataTypes.UInt32.value] * 3)]

        bufFile = FRB.CppBufFile(data, elements)
        emptied = FRB.CppBufFile(b"", elements)
        emptied.readFlatDumpStr(bufFile.getFlatDumpStr())

        self.assertEqual(emptied.data, data)

    def test_readFlatDumpStr_skipsAHeader(self):
        # every line of a .ib dump's header carries a ':' and none of its data lines do
        elements = [FRB.BufElementType("Triangle", "R32G32B32_UINT", [FRB.BufDataTypes.UInt32.value] * 3)]
        bufFile = FRB.CppBufFile(b"", elements)

        bufFile.readFlatDumpStr("""byte offset: 0
first index: 0
index count: 3
topology: trianglelist
format: DXGI_FORMAT_R16_UINT

0 1 2
""")

        self.assertEqual(bufFile.data, struct.pack("<3I", 0, 1, 2))

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
    # =================== filter =====================

    def test_filter_keepsSatisfyingLinesAndReturnsTheirIndices(self):
        lines = [struct.pack("<3f", float(i), 0.0, 0.0) for i in range(5)]
        bufFile = FRB.CppBufFile(b"".join(lines), _makePositionElements())

        kept = bufFile.filter(lambda frame: frame["POSITION"][0] % 2 == 0)

        self.assertEqual(kept, [0, 2, 4])
        self.assertEqual(bytes(bufFile.data), lines[0] + lines[2] + lines[4])

    def test_filter_predicateSeesTheSameDataAsDecodeLine(self):
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)
        bufFile = FRB.CppBufFile(line1 + line2, _makePositionElements())

        seen = []
        bufFile.filter(lambda frame: seen.append(frame) or True)

        self.assertEqual(seen, [bufFile.decodeLine(line1), bufFile.decodeLine(line2)])

    def test_filter_keepsNothing_emptiesTheFile(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.CppBufFile(line + line, _makePositionElements())

        self.assertEqual(bufFile.filter(lambda frame: False), [])
        self.assertEqual(bytes(bufFile.data), b"")

    def test_filter_keepsEverything_dataUnchanged(self):
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)
        bufFile = FRB.CppBufFile(line1 + line2, _makePositionElements())

        self.assertEqual(bufFile.filter(lambda frame: True), [0, 1])
        self.assertEqual(bytes(bufFile.data), line1 + line2)

    def test_filter_mutatingTheFrame_doesNotChangeTheKeptBytes(self):
        # filter *selects* lines, it never rewrites them -- the opposite of fix's filters, whose
        # whole job is to hand back modified data. A kept line's original bytes are copied through
        # rather than re-encoded from whatever the predicate was handed.
        line = struct.pack("<3f", 1.0, 2.0, 3.0)

        def scramble(frame):
            frame["POSITION"][0] = 999.0
            return True

        bufFile = FRB.CppBufFile(line, _makePositionElements())
        bufFile.filter(scramble)

        self.assertEqual(bytes(bufFile.data), line)

    def test_filter_predicateRaises_leavesTheFileUntouched(self):
        # the first line is rejected and the second raises -- so a filter that wrote as it went
        # would leave this file holding no lines at all
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)

        def boom(frame):
            if (frame["POSITION"][0] > 1.5):
                raise ValueError("nope")
            return False

        bufFile = FRB.CppBufFile(line1 + line2, _makePositionElements())
        with self.assertRaises(ValueError):
            bufFile.filter(boom)

        self.assertEqual(bytes(bufFile.data), line1 + line2)

    def test_filter_truthyReturnValue_notJustBool(self):
        # the predicate's result is read the way an 'if' would read it, so a 0/1, a numpy bool or
        # an empty container all behave sensibly instead of failing a strict bool cast
        lines = [struct.pack("<3f", float(i), 0.0, 0.0) for i in range(3)]
        bufFile = FRB.CppBufFile(b"".join(lines), _makePositionElements())

        kept = bufFile.filter(lambda frame: [] if (frame["POSITION"][0] == 1.0) else [1])

        self.assertEqual(kept, [0, 2])
        self.assertEqual(bytes(bufFile.data), lines[0] + lines[2])

    def test_filter_noElements_returnsEmptyAndNeverCallsThePredicate(self):
        calls = []
        bufFile = FRB.CppBufFile(b"", [])

        self.assertEqual(bufFile.filter(lambda frame: calls.append(frame) or True), [])
        self.assertEqual(calls, [])

    def test_filter_fileSrc_becomesBytesAndLeavesTheFileOnDisk(self):
        # same 'data is read-only, the write path is src + read()' shape as merge/encodeAll
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)

        with tempfile.NamedTemporaryFile(delete = False, suffix = ".buf") as f:
            f.write(line1 + line2)
            path = f.name

        try:
            bufFile = FRB.CppBufFile(path, _makePositionElements())
            self.assertEqual(bufFile.filter(lambda frame: frame["POSITION"][0] > 2), [1])

            self.assertEqual(bytes(bufFile.data), line2)
            self.assertNotIsInstance(bufFile.src, str)

            with open(path, "rb") as f:
                self.assertEqual(f.read(), line1 + line2)
        finally:
            os.remove(path)

    def test_filter_duplicateElementNames_predicateGetsTheSuffixedKeys(self):
        # the predicate is keyed the same way decodeLine is, so a repeated element name is reached
        # by its occurrence suffix rather than being invisible
        elements = [FRB.BufElementType("X", "fmt", [FRB.BufFloat()]), FRB.BufElementType("X", "fmt", [FRB.BufFloat()])]
        line1 = struct.pack("<2f", 1.0, 2.0)
        line2 = struct.pack("<2f", 3.0, 4.0)

        bufFile = FRB.CppBufFile(line1 + line2, elements)
        kept = bufFile.filter(lambda frame: frame["X1"][0] > 3.0)

        self.assertEqual(kept, [1])
        self.assertEqual(bytes(bufFile.data), line2)

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

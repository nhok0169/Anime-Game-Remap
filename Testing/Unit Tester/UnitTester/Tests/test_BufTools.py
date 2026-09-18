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
    # ================= fromDataFrame ================

    def test_fromDataFrame_roundTrip_byteIdentical(self):
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)
        bufFile = FRB.BufFile(line1 + line2, _makePositionElements())

        FRB.BufTools.fromDataFrame(bufFile, FRB.BufTools.toDataFrame(bufFile))

        self.assertEqual(bufFile.data, line1 + line2)

    def test_fromDataFrame_editedValues_storedBack(self):
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)
        bufFile = FRB.BufFile(line1 + line2, _makePositionElements())

        df = FRB.BufTools.toDataFrame(bufFile)
        df[("POSITION", 1)] = df[("POSITION", 1)] + 10.0
        FRB.BufTools.fromDataFrame(bufFile, df)

        self.assertEqual(bufFile.decodeLine(bufFile.data[:12])["POSITION"], [1.0, 12.0, 3.0])
        self.assertEqual(bufFile.decodeLine(bufFile.data[12:])["POSITION"], [4.0, 15.0, 6.0])

    def test_fromDataFrame_mixedDtypes_integerElementStaysAnInteger(self):
        # A frame mixing float and integer columns must not be flattened through one common dtype
        # on the way back -- doing that (eg. a whole-frame 'to_numpy') would hand every integer
        # element a float to encode, silently corrupting a Blend.buf's blend indices
        weights = struct.pack("<4f", 1.0, 0.5, 0.0, 0.0)
        indices = struct.pack("<4i", 5, 7, 0, 0)
        blend = FRB.BlendFile(weights + indices)

        df = FRB.BufTools.toDataFrame(blend)
        df[("BLENDINDICES", 0)] = 42
        FRB.BufTools.fromDataFrame(blend, df)

        self.assertEqual(blend.data[16:20], struct.pack("<i", 42))

        decoded = blend.decodeLine(blend.data)
        self.assertEqual(decoded["BLENDINDICES"], [42, 7, 0, 0])
        self.assertAlmostEqual(decoded["BLENDWEIGHT"][1], 0.5, places = 4)

    def test_fromDataFrame_fewerRows_dataShrinks(self):
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)
        bufFile = FRB.BufFile(line1 + line2, _makePositionElements())

        FRB.BufTools.fromDataFrame(bufFile, FRB.BufTools.toDataFrame(bufFile).iloc[1:])

        self.assertEqual(bufFile.data, line2)

    def test_fromDataFrame_reorderedColumns_encodedByTheirKeys(self):
        # the columns are matched by their (elementKey, indexWithinElement) keys, not by position
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.BufFile(line, _makePositionElements())

        df = FRB.BufTools.toDataFrame(bufFile)
        FRB.BufTools.fromDataFrame(bufFile, df[list(reversed(df.columns))])

        self.assertEqual(bufFile.data, line)

    def test_fromDataFrame_emptyDataFrame_emptiesData(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.BufFile(line, _makePositionElements())

        FRB.BufTools.fromDataFrame(bufFile, FRB.BufTools.toDataFrame(bufFile).iloc[0:0])

        self.assertEqual(bufFile.data, b"")

    def test_fromDataFrame_changedElements_encodesWithTheNewElements(self):
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)
        bufFile = FRB.BufFile(line1 + line2, _makePositionElements())

        bufFile.elements = [FRB.BufElementType("POSITION", "R32G32_FLOAT", [FRB.BufFloat(), FRB.BufFloat()])]
        df = FRB.BufTools.toDataFrame(bufFile)
        FRB.BufTools.fromDataFrame(bufFile, df)

        self.assertEqual(bufFile.bytesPerLine, 8)
        self.assertEqual(len(bufFile.data), 8 * len(df.index))

    def test_fromDataFrame_newDataVisibleToFix(self):
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)
        bufFile = FRB.BufFile(line1 + line2, _makePositionElements())

        FRB.BufTools.fromDataFrame(bufFile, FRB.BufTools.toDataFrame(bufFile).iloc[:1])

        self.assertEqual(bytes(bufFile.fix()), line1)

    # ================================================
    # ============ BufFile.fromDataFrame =============

    def test_bufFileFromDataFrame_matchesBufTools(self):
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)

        viaMethod = FRB.BufFile(line1 + line2, _makePositionElements())
        viaTools = FRB.BufFile(line1 + line2, _makePositionElements())

        df = viaMethod.toDataFrame()
        df[("POSITION", 0)] = df[("POSITION", 0)] * 2

        viaMethod.fromDataFrame(df)
        FRB.BufTools.fromDataFrame(viaTools, df)

        self.assertEqual(viaMethod.data, viaTools.data)

    # ================================================
    # ================== getDumpStr ==================

    def test_getDumpStr_dataOnly_noHeader(self):
        # the numbers are formatted the way 3dmigoto's own "%.9g" does -- so "1", not "1.0"
        line1 = struct.pack("<3f", 1.0, 2.0, 3.0)
        line2 = struct.pack("<3f", 4.0, 5.0, 6.0)
        bufFile = FRB.BufFile(line1 + line2, _makePositionElements())

        self.assertEqual(FRB.BufTools.getDumpStr(bufFile), """vb0[0]+000 POSITION: 1, 2, 3

vb0[1]+000 POSITION: 4, 5, 6
""")

    def test_getDumpStr_byteOffsetsFollowTheElementSizes(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0) + struct.pack("<2f", 0.25, 0.5)
        bufFile = FRB.BufFile(line, [FRB.BufElementTypes.PositionFloatRGB.value,
                                     FRB.BufElementTypes.TextureCoordinateRG.value])

        self.assertEqual(FRB.BufTools.getDumpStr(bufFile), """vb0[0]+000 POSITION: 1, 2, 3
vb0[0]+012 TEXCOORD: 0.25, 0.5
""")

    def test_getDumpStr_blankLineBetweenLinesButNotAfterTheLast(self):
        # a real 3dmigoto dump ends right after its final entry
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        one = FRB.BufFile(line, _makePositionElements())
        two = FRB.BufFile(line + line, _makePositionElements())

        self.assertEqual(FRB.BufTools.getDumpStr(one).count("\n\n"), 0)
        self.assertEqual(FRB.BufTools.getDumpStr(two).count("\n\n"), 1)
        self.assertFalse(FRB.BufTools.getDumpStr(two).endswith("\n\n"))

    def test_getDumpStr_repeatedElementName_suffixedInTheEntryName(self):
        # matches both decodeLine's own keys and what 3dmigoto writes for a second TEXCOORD
        line = struct.pack("<2f", 0.25, 0.5) + struct.pack("<2f", 0.75, 0.125)
        bufFile = FRB.BufFile(line, [FRB.BufElementTypes.TextureCoordinateRG.value,
                                     FRB.BufElementTypes.TextureCoordinateRG.value])

        result = FRB.BufTools.getDumpStr(bufFile)

        self.assertIn("vb0[0]+000 TEXCOORD: 0.25, 0.5", result)
        self.assertIn("vb0[0]+008 TEXCOORD1: 0.75, 0.125", result)

    # ================================================
    # ================= readDumpStr ==================

    def test_readDumpStr_roundTripsGetDumpStr(self):
        line1 = struct.pack("<3f", 1.25, 2.5, 3.75)
        line2 = struct.pack("<3f", -4.5, 5.25, 6.125)
        bufFile = FRB.BufFile(line1 + line2, _makePositionElements())

        text = FRB.BufTools.getDumpStr(bufFile)
        emptied = FRB.BufFile(b"", _makePositionElements())
        FRB.BufTools.readDumpStr(emptied, text)

        self.assertEqual(emptied.data, line1 + line2)

    def test_readDumpStr_mixedDtypes_roundTrip(self):
        weights = struct.pack("<4f", 1.0, 0.5, 0.0, 0.0)
        indices = struct.pack("<4i", 5, 7, 0, 0)
        blend = FRB.BlendFile(weights + indices)

        text = FRB.BufTools.getDumpStr(blend)
        emptied = FRB.BlendFile(b"")
        FRB.BufTools.readDumpStr(emptied, text)

        self.assertEqual(emptied.data, weights + indices)

    def test_getDumpStr_elementsInDeclaredOrder(self):
        # decodeLine is backed by an unordered_map, so the order has to come from the elements
        # themselves -- a dump whose entries are shuffled per line would not import
        line = struct.pack("<3f", 1.0, 2.0, 3.0) + struct.pack("<2f", 0.25, 0.5)
        bufFile = FRB.BufFile(line, [FRB.BufElementTypes.PositionFloatRGB.value,
                                     FRB.BufElementTypes.TextureCoordinateRG.value])

        result = FRB.BufTools.getDumpStr(bufFile)

        self.assertLess(result.index("POSITION"), result.index("TEXCOORD"))

    def test_getDumpStr_integerElement_staysIntegral(self):
        blend = FRB.BlendFile(struct.pack("<4f", 1.0, 0.0, 0.0, 0.0) + struct.pack("<4i", 5, 0, 0, 0))

        self.assertIn("vb0[0]+016 BLENDINDICES: 5, 0, 0, 0", FRB.BufTools.getDumpStr(blend))

    def test_getDumpStr_prefixIsConfigurable(self):
        bufFile = FRB.BufFile(struct.pack("<3f", 1.0, 2.0, 3.0), _makePositionElements())

        self.assertTrue(FRB.BufTools.getDumpStr(bufFile, prefix = "vb2").startswith("vb2[0]+000 POSITION:"))

    def test_getDumpStr_emptyData_emptyString(self):
        self.assertEqual(FRB.BufTools.getDumpStr(FRB.BufFile(b"", _makePositionElements())), "")

    def test_bufFileGetDumpStr_matchesBufTools(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        bufFile = FRB.BufFile(line, _makePositionElements())

        self.assertEqual(bufFile.getDumpStr(), FRB.BufTools.getDumpStr(bufFile))

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

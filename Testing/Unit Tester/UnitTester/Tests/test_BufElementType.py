import sys
import struct

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class BufElementTypeTest(BaseUnitTest):
    """
    Tests for :class:`BufElementType`
    """

    # ================================================
    # =================== size =======================

    def test_size_sumOfDataTypeSizes(self):
        element = FRB.BufElementType("POSITION", "R32G32B32_FLOAT", [FRB.BufFloat(), FRB.BufFloat(), FRB.BufFloat()])
        self.assertEqual(element.size, 12)

    def test_mixedDataTypes_sizeIsSum(self):
        element = FRB.BufElementType("MIXED", "fmt", [FRB.BufFloat(), FRB.BufUnorm("U", 1), FRB.BufSignedInt()])
        self.assertEqual(element.size, 4 + 1 + 4)

    def test_setDataTypes_sizeRecomputed(self):
        element = FRB.BufElementType("X", "fmt", [FRB.BufFloat()])
        self.assertEqual(element.size, 4)

        element.dataTypes = [FRB.BufFloat(), FRB.BufFloat()]
        self.assertEqual(element.size, 8)

    # ================================================
    # =========== name / formatName ==================

    def test_constructorArgs_setCorrectly(self):
        element = FRB.BufElementType("POSITION", "R32G32B32_FLOAT", [FRB.BufFloat()])
        self.assertEqual(element.name, "POSITION")
        self.assertEqual(element.formatName, "R32G32B32_FLOAT")

    def test_setFormatName_updated(self):
        element = FRB.BufElementType("X", "fmt", [FRB.BufFloat()])
        element.formatName = "newFmt"
        self.assertEqual(element.formatName, "newFmt")

    # ================================================
    # =================== dataTypes ==================

    def test_dataTypes_matchConstructedTypes(self):
        element = FRB.BufElementType("X", "fmt", [FRB.BufFloat(), FRB.BufSignedInt()])
        dataTypes = element.dataTypes

        self.assertEqual(len(dataTypes), 2)
        self.assertIsInstance(dataTypes[0], FRB.BufFloat)
        self.assertIsInstance(dataTypes[1], FRB.BufSignedInt)

    def test_dataTypesCloned_originalRemainsUsableAndIndependent(self):
        # Unlike IfTemplate's 'parts' (ownership-transfer, disowns the original), a BufDataType is
        # cloned into the element -- the original instance must stay fully usable afterward, and
        # the very same instance must be safely reusable across more than one BufElementType (this
        # is exactly how constants/BufDataTypes.py's DeferredEnum-cached values are actually used
        # in production: eg. 'BufDataTypes.Float32.value' is shared by several BufElementTypes
        # entries).
        dataType = FRB.BufFloat()
        element1 = FRB.BufElementType("X", "fmt", [dataType])
        element2 = FRB.BufElementType("Y", "fmt", [dataType, dataType, dataType])

        self.assertEqual(dataType.size, 4)
        self.assertEqual(element1.size, 4)
        self.assertEqual(element2.size, 12)

        # mutating the original afterward must not affect the already-cloned copies
        dataType.name = "Renamed"
        self.assertEqual(element1.dataTypes[0].name, "Float32")

    def test_inlineConstruction_noNamedIntermediateVariables(self):
        # Matches Testing/CLAUDE.md's lifetime-safety convention: construct every argument 100%
        # inline, with no separate Python variable holding a reference to any of it.
        element = FRB.BufElementType("POSITION", "R32G32B32_FLOAT", [FRB.BufFloat(), FRB.BufFloat(), FRB.BufFloat()])
        self.assertEqual(element.size, 12)
        self.assertEqual(len(element.dataTypes), 3)

    # ================================================
    # ================ decode/encode =================

    def test_decodeEncode_roundTrip(self):
        element = FRB.BufElementType("POSITION", "R32G32B32_FLOAT", [FRB.BufFloat(), FRB.BufFloat(), FRB.BufFloat()])
        values = [1.5, -2.5, 3.0]

        encoded = element.encode(values)
        self.assertIsInstance(encoded, bytes)
        self.assertEqual(encoded, struct.pack("<3f", *values))

        decoded = element.decode(encoded)
        for result, expected in zip(decoded, values):
            self.assertAlmostEqual(result, expected, places = 4)

    def test_decode_mixedTypes_correctValues(self):
        element = FRB.BufElementType("MIXED", "fmt", [FRB.BufSignedInt(), FRB.BufFloat()])
        src = struct.pack("<if", -5, 2.5)

        decoded = element.decode(src)
        self.assertEqual(decoded[0], -5)
        self.assertAlmostEqual(decoded[1], 2.5, places = 4)

    def test_encode_fewerValuesThanDataTypes_onlyEncodesLeading(self):
        # mirrors the pure-Python original's min(len(dataTypes), len(src)) guard
        element = FRB.BufElementType("X", "fmt", [FRB.BufFloat(), FRB.BufFloat(), FRB.BufFloat()])
        encoded = element.encode([1.0])
        self.assertEqual(len(encoded), 4)
        self.assertEqual(struct.unpack("<f", encoded)[0], 1.0)

    # ================================================

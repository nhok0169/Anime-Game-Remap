import sys
import struct

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class BufDataTypeTest(BaseUnitTest):
    """
    Tests for :class:`BufType`/:class:`BufDataType` and its concrete subclasses
    (:class:`BufBaseInt`/:class:`BufSignedInt`/:class:`BufUnSignedInt`,
    :class:`BufBaseFloat`/:class:`BufFloat`/:class:`BufFloat16`, :class:`BufUnorm`)
    """

    # =============== inheritance/isinstance ==================

    def test_bufDataTypeHierarchy_isinstanceChain(self):
        self.assertIsInstance(FRB.BufSignedInt(), FRB.BufBaseInt)
        self.assertIsInstance(FRB.BufSignedInt(), FRB.BufDataType)
        self.assertIsInstance(FRB.BufSignedInt(), FRB.BufType)

        self.assertIsInstance(FRB.BufUnSignedInt(), FRB.BufBaseInt)
        self.assertIsInstance(FRB.BufFloat(), FRB.BufBaseFloat)
        self.assertIsInstance(FRB.BufFloat(), FRB.BufDataType)
        self.assertIsInstance(FRB.BufFloat16(), FRB.BufBaseFloat)
        self.assertIsInstance(FRB.BufUnorm("U", 1), FRB.BufBaseInt)

    # ================================================
    # =================== name =======================

    def test_setName_nameUpdated(self):
        dataType = FRB.BufFloat()
        dataType.name = "MyFloat"
        self.assertEqual(dataType.name, "MyFloat")

    # ================================================
    # =================== size =======================

    def test_defaultConstructorArgs_correctNameAndSize(self):
        tests = [[FRB.BufSignedInt(), "SignedInt32", 4],
                 [FRB.BufUnSignedInt(), "UnsignedInt32", 4],
                 [FRB.BufFloat(), "Float32", 4],
                 [FRB.BufFloat16(), "Float16", 2]]

        for dataType, expectedName, expectedSize in tests:
            self.assertEqual(dataType.name, expectedName)
            self.assertEqual(dataType.size, expectedSize)

    def test_sizeOutOfRange_raisesValueError(self):
        tests = [0, 9, 100]
        for size in tests:
            with self.assertRaises(ValueError):
                FRB.BufSignedInt("X", size)

    def test_setSizeOutOfRange_raisesValueError(self):
        dataType = FRB.BufSignedInt()
        with self.assertRaises(ValueError):
            dataType.size = 0

        with self.assertRaises(ValueError):
            dataType.size = 9

    # ================================================
    # ================ isBigEndian ===================

    def test_isBigEndianDefault_false(self):
        self.assertFalse(FRB.BufSignedInt().isBigEndian)

    def test_setIsBigEndian_updated(self):
        dataType = FRB.BufSignedInt()
        dataType.isBigEndian = True
        self.assertTrue(dataType.isBigEndian)

    # ================================================
    # ================= isSigned =====================

    def test_isSigned_matchesConstructedType(self):
        self.assertTrue(FRB.BufSignedInt().isSigned)
        self.assertFalse(FRB.BufUnSignedInt().isSigned)
        self.assertFalse(FRB.BufUnorm("U", 1).isSigned)

    # ================================================
    # ============ BufSignedInt/BufUnSignedInt =======

    def test_signedInt_decodeEncode_roundTrip(self):
        dataType = FRB.BufSignedInt()
        tests = [0, 1, -1, 2147483647, -2147483648, 12345]
        for value in tests:
            encoded = dataType.encode(value)
            self.assertIsInstance(encoded, bytes)
            self.assertEqual(len(encoded), 4)
            self.assertEqual(dataType.decode(encoded), value)

    def test_signedInt_matchesStructPacking_littleEndian(self):
        dataType = FRB.BufSignedInt()
        tests = [0, 1, -1, 2147483647, -2147483648]
        for value in tests:
            self.assertEqual(dataType.encode(value), struct.pack("<i", value))

    def test_signedInt_bigEndian_matchesStructPacking(self):
        dataType = FRB.BufSignedInt(isBigEndian = True)
        tests = [0, 1, -1, 2147483647, -2147483648]
        for value in tests:
            encoded = dataType.encode(value)
            self.assertEqual(encoded, struct.pack(">i", value))
            self.assertEqual(dataType.decode(encoded), value)

    def test_unsignedInt_decodeEncode_roundTrip(self):
        dataType = FRB.BufUnSignedInt()
        tests = [0, 1, 4294967295, 200000]
        for value in tests:
            encoded = dataType.encode(value)
            self.assertEqual(len(encoded), 4)
            self.assertEqual(dataType.decode(encoded), value)

    def test_unsignedInt_matchesStructPacking(self):
        dataType = FRB.BufUnSignedInt()
        tests = [0, 1, 4294967295]
        for value in tests:
            self.assertEqual(dataType.encode(value), struct.pack("<I", value))

    def test_smallIntSizes_roundTrip(self):
        # a 1-byte signed int, exercising this port's byte-size flexibility (not just the
        # 4-byte default every real BufDataTypes entry in this codebase uses)
        dataType = FRB.BufSignedInt("Int8", 1)
        tests = [0, 1, -1, 127, -128]
        for value in tests:
            encoded = dataType.encode(value)
            self.assertEqual(len(encoded), 1)
            self.assertEqual(dataType.decode(encoded), value)

    # ================================================
    # ============ BufFloat/BufFloat16 ===============

    def test_float32_decodeEncode_roundTrip(self):
        dataType = FRB.BufFloat()
        tests = [0.0, 1.5, -2.5, 3.0, -100.25]
        for value in tests:
            encoded = dataType.encode(value)
            self.assertIsInstance(encoded, bytes)
            self.assertEqual(len(encoded), 4)
            self.assertAlmostEqual(dataType.decode(encoded), value, places = 4)

    def test_float32_matchesStructPacking(self):
        dataType = FRB.BufFloat()
        tests = [0.0, 1.5, -2.5, 3.0]
        for value in tests:
            self.assertEqual(dataType.encode(value), struct.pack("<f", value))

    def test_float32_bigEndian_matchesStructPacking(self):
        dataType = FRB.BufFloat(isBigEndian = True)
        value = 3.5
        encoded = dataType.encode(value)
        self.assertEqual(encoded, struct.pack(">f", value))
        self.assertAlmostEqual(dataType.decode(encoded), value, places = 4)

    def test_float16_decodeEncode_roundTrip(self):
        dataType = FRB.BufFloat16()
        tests = [0.0, 1.5, -2.5, 0.5, -1.0]
        for value in tests:
            encoded = dataType.encode(value)
            self.assertEqual(len(encoded), 2)
            self.assertAlmostEqual(dataType.decode(encoded), value, places = 3)

    def test_float16_matchesStructPacking(self):
        dataType = FRB.BufFloat16()
        tests = [0.0, 1.5, -2.5, 0.5]
        for value in tests:
            self.assertEqual(dataType.encode(value), struct.pack("<e", value))

    # ================================================
    # =================== BufUnorm ===================

    def test_unorm_decodeEncode_roundTrip(self):
        dataType = FRB.BufUnorm("UNORM8", 1)
        tests = [0.0, 1.0, 0.5]
        for value in tests:
            encoded = dataType.encode(value)
            self.assertEqual(len(encoded), 1)
            self.assertAlmostEqual(dataType.decode(encoded), value, places = 2)

    def test_unorm_zeroBytesDecodesToZero(self):
        dataType = FRB.BufUnorm("UNORM8", 1)
        self.assertEqual(dataType.decode(b"\x00"), 0.0)

    def test_unorm_maxBytesDecodesToOne(self):
        dataType = FRB.BufUnorm("UNORM8", 1)
        self.assertEqual(dataType.decode(b"\xff"), 1.0)

    # ================================================

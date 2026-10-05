import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class Hash128Test(BaseUnitTest):

    # =============== __init__ =======================

    def test_defaultInit_bothHalvesAreZero(self):
        hashVal = FRB.Hash128()
        self.assertEqual(hashVal.high, 0)
        self.assertEqual(hashVal.low, 0)

    def test_initFromHalves_halvesPreserved(self):
        tests = [(0, 0), (1, 2), (2 ** 63, 2 ** 64 - 1), (2 ** 64 - 1, 0)]
        for high, low in tests:
            hashVal = FRB.Hash128(high, low)
            self.assertEqual(hashVal.high, high)
            self.assertEqual(hashVal.low, low)

    # ================================================
    # =================== hash =======================

    def test_hashStr_isDeterministic(self):
        tests = ["", "hello world", "Raiden Shogun", "a" * 1000]
        for text in tests:
            hash1 = FRB.Hash128.hash(text)
            hash2 = FRB.Hash128.hash(text)
            self.assertEqual(hash1, hash2)
            self.assertEqual(hash1.high, hash2.high)
            self.assertEqual(hash1.low, hash2.low)

    def test_hashStrAndBytes_sameUtf8Content_sameHash(self):
        tests = ["", "hello world", "Raiden Shogun", "😆😄"]
        for text in tests:
            self.assertEqual(FRB.Hash128.hash(text), FRB.Hash128.hash(text.encode("utf-8")))

    def test_hashDifferentInputs_differentHashes(self):
        tests = ["Raiden Shogun", "Raiden Ei", "Ei", "raiden shogun", "Raiden Shogun "]
        hashes = [FRB.Hash128.hash(text) for text in tests]
        self.assertEqual(len(set(hashes)), len(tests))

    # ================================================
    # ================ toHexString ====================

    def test_toHexString_fixedLengthAndMatchesHalves(self):
        tests = ["", "hello world", "Raiden Shogun"]
        for text in tests:
            hashVal = FRB.Hash128.hash(text)
            hexStr = hashVal.toHexString()

            self.assertEqual(len(hexStr), 32)
            self.assertEqual(hexStr, hexStr.lower())
            self.assertEqual(int(hexStr[:16], 16), hashVal.high)
            self.assertEqual(int(hexStr[16:], 16), hashVal.low)

    # ================================================
    # ================= toBase64 ======================

    def test_toBase64_fixedLengthAndDeterministic(self):
        tests = ["", "hello world", "Raiden Shogun"]
        for text in tests:
            hashVal = FRB.Hash128.hash(text)
            self.assertEqual(len(hashVal.toBase64()), 22)
            self.assertEqual(hashVal.toBase64(), FRB.Hash128.hash(text).toBase64())

    # ================================================
    # ============== operations =======================

    def test_equality(self):
        hash1 = FRB.Hash128.hash("Raiden Shogun")
        hash2 = FRB.Hash128.hash("Raiden Shogun")
        hash3 = FRB.Hash128.hash("Raiden Ei")

        self.assertEqual(hash1, hash2)
        self.assertFalse(hash1 != hash2)

        self.assertNotEqual(hash1, hash3)
        self.assertTrue(hash1 != hash3)

    def test_ordering_matchesHighThenLowOrdering(self):
        tests = [(0, 0), (0, 1), (1, 0), (1, 1), (2, 0)]
        hashes = [FRB.Hash128(high, low) for high, low in tests]

        for a in hashes:
            for b in hashes:
                expected = (a.high, a.low) < (b.high, b.low)
                self.assertEqual(a < b, expected)

    def test_usableAsDictAndSetKey(self):
        tests = ["Raiden Shogun", "Raiden Ei", "Ei"]
        hashes = [FRB.Hash128.hash(text) for text in tests]

        seen = set(hashes)
        seen.add(FRB.Hash128.hash("Raiden Shogun"))
        self.assertEqual(len(seen), len(tests))

        mapping = {h: text for h, text in zip(hashes, tests)}
        self.assertEqual(mapping[FRB.Hash128.hash("Raiden Ei")], "Raiden Ei")

    def test_str_equalsToHexString(self):
        hashVal = FRB.Hash128.hash("Raiden Shogun")
        self.assertEqual(str(hashVal), hashVal.toHexString())

    # ================================================

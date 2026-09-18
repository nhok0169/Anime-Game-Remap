import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class Hash64Test(BaseUnitTest):

    # =============== __init__ =======================

    def test_defaultInit_valueIsZero(self):
        self.assertEqual(FRB.Hash64().value, 0)

    def test_initFromValue_valuePreserved(self):
        tests = [0, 1, 12345, 2 ** 63, 2 ** 64 - 1]
        for value in tests:
            self.assertEqual(FRB.Hash64(value).value, value)

    # ================================================
    # =================== hash =======================

    def test_hashStr_isDeterministic(self):
        tests = ["", "hello world", "Raiden Shogun", "a" * 1000]
        for text in tests:
            hash1 = FRB.Hash64.hash(text)
            hash2 = FRB.Hash64.hash(text)
            self.assertEqual(hash1, hash2)
            self.assertEqual(hash1.value, hash2.value)

    def test_hashStrAndBytes_sameUtf8Content_sameHash(self):
        tests = ["", "hello world", "Raiden Shogun", "😆😄"]
        for text in tests:
            self.assertEqual(FRB.Hash64.hash(text), FRB.Hash64.hash(text.encode("utf-8")))

    def test_hashDifferentInputs_differentHashes(self):
        tests = ["Raiden Shogun", "Raiden Ei", "Ei", "raiden shogun", "Raiden Shogun "]
        hashes = [FRB.Hash64.hash(text) for text in tests]
        self.assertEqual(len(set(hashes)), len(tests))

    # ================================================
    # ================ toHexString ====================

    def test_toHexString_fixedLengthAndRoundTrips(self):
        tests = ["", "hello world", "Raiden Shogun"]
        for text in tests:
            hashVal = FRB.Hash64.hash(text)
            hexStr = hashVal.toHexString()

            self.assertEqual(len(hexStr), 16)
            self.assertEqual(hexStr, hexStr.lower())
            self.assertEqual(int(hexStr, 16), hashVal.value)

    # ================================================
    # ================= toBase64 ======================

    def test_toBase64_fixedLengthAndDeterministic(self):
        tests = ["", "hello world", "Raiden Shogun"]
        for text in tests:
            hashVal = FRB.Hash64.hash(text)
            self.assertEqual(len(hashVal.toBase64()), 11)
            self.assertEqual(hashVal.toBase64(), FRB.Hash64.hash(text).toBase64())

    # ================================================
    # ============== operations =======================

    def test_equality(self):
        hash1 = FRB.Hash64.hash("Raiden Shogun")
        hash2 = FRB.Hash64.hash("Raiden Shogun")
        hash3 = FRB.Hash64.hash("Raiden Ei")

        self.assertEqual(hash1, hash2)
        self.assertFalse(hash1 != hash2)

        self.assertNotEqual(hash1, hash3)
        self.assertTrue(hash1 != hash3)

    def test_ordering_matchesValueOrdering(self):
        tests = ["Raiden Shogun", "Raiden Ei", "Ei", "Yae Miko", "Furina"]
        hashes = [FRB.Hash64.hash(text) for text in tests]

        for a in hashes:
            for b in hashes:
                self.assertEqual(a < b, a.value < b.value)

    def test_usableAsDictAndSetKey(self):
        tests = ["Raiden Shogun", "Raiden Ei", "Ei"]
        hashes = [FRB.Hash64.hash(text) for text in tests]

        seen = set(hashes)
        seen.add(FRB.Hash64.hash("Raiden Shogun"))
        self.assertEqual(len(seen), len(tests))

        mapping = {h: text for h, text in zip(hashes, tests)}
        self.assertEqual(mapping[FRB.Hash64.hash("Raiden Ei")], "Raiden Ei")

    def test_str_equalsToHexString(self):
        hashVal = FRB.Hash64.hash("Raiden Shogun")
        self.assertEqual(str(hashVal), hashVal.toHexString())

    # ================================================

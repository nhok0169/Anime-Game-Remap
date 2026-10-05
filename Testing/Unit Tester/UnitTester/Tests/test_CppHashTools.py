import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppHashToolsTest(BaseUnitTest):

    def setUp(self):
        super().setUp()
        FRB.CppHashTools.clear()

    # =========== getDeterministicHash ================

    def test_getDeterministicHash_matchesHash128(self):
        tests = ["", "hello world", "Raiden Shogun"]
        for text in tests:
            self.assertEqual(FRB.CppHashTools.getDeterministicHash(text), FRB.Hash128.hash(text))

    def test_getDeterministicHash_isDeterministic(self):
        tests = ["", "hello world", "Raiden Shogun"]
        for text in tests:
            hash1 = FRB.CppHashTools.getDeterministicHash(text)
            hash2 = FRB.CppHashTools.getDeterministicHash(text)
            self.assertEqual(hash1, hash2)

    def test_getDeterministicHash_strAndBytes_sameUtf8Content_sameHash(self):
        tests = ["", "hello world", "Raiden Shogun", "😆😄"]
        for text in tests:
            self.assertEqual(FRB.CppHashTools.getDeterministicHash(text), FRB.CppHashTools.getDeterministicHash(text.encode("utf-8")))

    def test_getDeterministicHash_differentInputs_differentHashes(self):
        tests = ["Raiden Shogun", "Raiden Ei", "Ei", "raiden shogun", "Raiden Shogun "]
        hashes = [FRB.CppHashTools.getDeterministicHash(text) for text in tests]
        self.assertEqual(len(set(hashes)), len(tests))

    # ================================================
    # ========= getDeterministicHashStr ===============

    def test_getDeterministicHashStr_matchesHashToBase64(self):
        tests = ["", "hello world", "Raiden Shogun"]
        for text in tests:
            self.assertEqual(FRB.CppHashTools.getDeterministicHashStr(text), FRB.CppHashTools.getDeterministicHash(text).toBase64())

    def test_getDeterministicHashStr_isDeterministic(self):
        tests = ["", "hello world", "Raiden Shogun"]
        for text in tests:
            self.assertEqual(FRB.CppHashTools.getDeterministicHashStr(text), FRB.CppHashTools.getDeterministicHashStr(text))

    def test_getDeterministicHashStr_strAndBytes_sameUtf8Content_sameHash(self):
        tests = ["", "hello world", "Raiden Shogun", "😆😄"]
        for text in tests:
            self.assertEqual(FRB.CppHashTools.getDeterministicHashStr(text), FRB.CppHashTools.getDeterministicHashStr(text.encode("utf-8")))

    # ================================================
    # ======= getShortDeterministicHashStr ============

    def test_getShortDeterministicHashStr_isDeterministic_whenNoCollision(self):
        text = "Raiden Shogun"

        FRB.CppHashTools.clear()
        result1 = FRB.CppHashTools.getShortDeterministicHashStr(text)

        FRB.CppHashTools.clear()
        result2 = FRB.CppHashTools.getShortDeterministicHashStr(text)

        self.assertEqual(result1, result2)

    def test_getShortDeterministicHashStr_repeatedInput_appendsFrequencySuffix(self):
        FRB.CppHashTools.clear()
        text = "Raiden Shogun"

        first = FRB.CppHashTools.getShortDeterministicHashStr(text)
        second = FRB.CppHashTools.getShortDeterministicHashStr(text)
        third = FRB.CppHashTools.getShortDeterministicHashStr(text)

        # Note: the base64 digit '_' can legitimately appear as part of 'first' itself (it's a
        # valid digit in this project's base64 alphabet), so we can't assert its absence -- only
        # that every repeat exactly appends the expected '_<frequency>' suffix onto 'first'
        self.assertEqual(second, f"{first}_{FRB.IntTools.toBase64(1)}")
        self.assertEqual(third, f"{first}_{FRB.IntTools.toBase64(2)}")

    def test_getShortDeterministicHashStr_strAndBytes_sameUtf8Content_sameShortHash(self):
        tests = ["", "hello world", "Raiden Shogun", "😆😄"]
        for text in tests:
            FRB.CppHashTools.clear()
            resultStr = FRB.CppHashTools.getShortDeterministicHashStr(text)

            FRB.CppHashTools.clear()
            resultBytes = FRB.CppHashTools.getShortDeterministicHashStr(text.encode("utf-8"))

            self.assertEqual(resultStr, resultBytes)

    # ================================================
    # ==================== clear ======================

    def test_clear_resetsFrequencyCounts(self):
        FRB.CppHashTools.clear()
        text = "Raiden Shogun"

        first = FRB.CppHashTools.getShortDeterministicHashStr(text)
        second = FRB.CppHashTools.getShortDeterministicHashStr(text)
        self.assertNotEqual(first, second)

        FRB.CppHashTools.clear()
        third = FRB.CppHashTools.getShortDeterministicHashStr(text)
        self.assertEqual(third, first)

    # ================================================

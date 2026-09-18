import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


##########
# Note:
#    This unit test is created using AI (Github Copilot).
##########
class IntToolsTest(BaseUnitTest):

    # =========== toBase =====================================
    # ========================================================

    def test_toBase_positive(self):
        self.assertEqual(FRB.IntTools.toBase(10, 2), ([1, 0, 1, 0], False))
        self.assertEqual(FRB.IntTools.toBase(255, 16), ([15, 15], False))
        self.assertEqual(FRB.IntTools.toBase(0, 10), ([0], False))
        self.assertEqual(FRB.IntTools.toBase(123, 10), ([1, 2, 3], False))

    def test_toBase_negative(self):
        self.assertEqual(FRB.IntTools.toBase(-10, 2), ([1, 0, 1, 0], True))
        self.assertEqual(FRB.IntTools.toBase(-255, 16), ([15, 15], True))
        self.assertEqual(FRB.IntTools.toBase(-123, 10), ([1, 2, 3], True))

    def test_toBase_edge_cases(self):
        # base 1 is not supported, should raise ZeroDivisionError
        with self.assertRaises(TypeError):
            FRB.IntTools.toBase(5, 0)
        with self.assertRaises(TypeError):
            FRB.IntTools.toBase(5, -2)

    # ========================================================
    # =========== toStrBase ==================================

    def test_toStrBase_with_list_digits(self):
        digits = ["a", "b", "c", "d"]
        self.assertEqual(FRB.IntTools.toStrBase(3, 4, digits, "!"), "d")
        self.assertEqual(FRB.IntTools.toStrBase(10, 4, digits, "!"), "cc")
        self.assertEqual(FRB.IntTools.toStrBase(-2, 4, digits, "!"), "!c")

    # ========================================================
    # ============ toBase64 ==================================

    def test_toBase64_default(self):
        # 64^2 = 4096, so 4095 should be "++"
        self.assertEqual(FRB.IntTools.toBase64(4095), "__")
        self.assertEqual(FRB.IntTools.toBase64(0), "A")
        self.assertEqual(FRB.IntTools.toBase64(-1), "-B")
        self.assertEqual(FRB.IntTools.toBase64(63), "_")
        self.assertEqual(FRB.IntTools.toBase64(-63), "-_")

    def test_toBase64_custom_digits(self):
        digits = list("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_")
        self.assertEqual(FRB.IntTools.toBase64(63, digits), "_")
        self.assertEqual(FRB.IntTools.toBase64(62, digits), "-")
        self.assertEqual(FRB.IntTools.toBase64(0, digits), "0")
        self.assertEqual(FRB.IntTools.toBase64(-5, digits, "!"), "!5")

    # ========================================================

    def test_foo(self):
        digits = list("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_")
        FRB.IntTools.toBase64(0, digits)
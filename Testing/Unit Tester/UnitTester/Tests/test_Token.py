import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class TokenTest(BaseUnitTest):

    # =============== __init__ =======================

    def test_init_fieldsPreserved(self):
        token = FRB.Token("ID", "$swapvar$", 8, 1)
        self.assertEqual(token.type, "ID")
        self.assertEqual(token.val, "$swapvar$")
        self.assertEqual(token.lineNo, 8)
        self.assertEqual(token.charNo, 1)

    def test_init_noneType_preserved(self):
        token = FRB.Token(None, "+", 3, 5)
        self.assertIsNone(token.type)
        self.assertEqual(token.val, "+")

    def test_init_zeroLineAndCharNo_preserved(self):
        # 'val' is documented/typed as a plain str (not Optional), unlike 'type' -- a non-terminal
        # placeholder token (see BaseSLR1Parser's reduction bookkeeping) uses "" here, not None
        token = FRB.Token("SPRIME", "", 0, 0)
        self.assertEqual(token.lineNo, 0)
        self.assertEqual(token.charNo, 0)

    # ================================================
    # ================ attributes =====================

    def test_mutateFields_valuesUpdated(self):
        token = FRB.Token("ID", "$x$", 1, 1)

        token.type = "EQ"
        token.val = "=="
        token.lineNo = 2
        token.charNo = 3

        self.assertEqual(token.type, "EQ")
        self.assertEqual(token.val, "==")
        self.assertEqual(token.lineNo, 2)
        self.assertEqual(token.charNo, 3)

    def test_mutateType_toNone(self):
        token = FRB.Token("ID", "$x$", 1, 1)
        token.type = None
        self.assertIsNone(token.type)

    # ================================================
    # ================ independence ===================

    def test_separateInstances_areIndependent(self):
        token1 = FRB.Token("ID", "$x$", 1, 1)
        token2 = FRB.Token("ID", "$x$", 1, 1)

        token1.val = "$y$"
        self.assertEqual(token1.val, "$y$")
        self.assertEqual(token2.val, "$x$")

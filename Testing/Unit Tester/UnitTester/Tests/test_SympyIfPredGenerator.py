import sys
from sympy import Eq, Symbol, Ge, Lt, And, Or, Not, Ne, simplify_logic
from sympy.logic.boolalg import Boolean

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class SympyIfPredGeneratorTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._tokenizer = FRB.SympyTokenizer()
        cls._tokenizer.setup()

    def setUp(self):
        super().setUp()

        # See IfPredLogicGeneratorTest's identically-shaped setUp for why this is a fresh parser
        # per test with no id-generator mocking.
        self._parser = FRB.SympyParser()

    # ================== generate ====================

    def test_differentIfPreds_queriesGenerated(self):
        file = "MyFile.agr"
        startLineNo = 8

        tests = [
                 [" ", "0"],
                 ["$swapvar$ == 5", "$swapvar == 5"],
                 ["$x$ >= 5", "$x >= 5"],
                 ["Ge($x$, 5)", "$x >= 5"],
                 ["($\\swapvar$ < 7)", "($\\swapvar < 7)"],
                 ["~(($y$ | Ne($x$, $y$)) & (($x$ >= $y$) | ($x$ <= $y$)) & Eq($x$, $y$*$z$ - $y$ + $z$/3))", "!(($y || $x != $y) && (($x >= $y) || ($x <= $y)) && $x == $y * $z - $y + $z / 3)"],
                 ["Eq($x$, And($y$, True, Gt($z$, False), Or(True, False, ~$a$ > 8)))", "$x == ($y && 1 && $z > 0 && (1 || 0 || !$a > 8))"],
                 ["(($x$ >= 5 & $\\swapvar$ < (7 + 3.456 * -0.0)) | ~($z[]%d$ >= 1 & $z[]%d$ < -9 & $123$ == True)) \t & $\\~$ + -234 -        -0 / -9.6", "(($x >= 5 && $\swapvar < (7 + 3.456 * -0.0)) || !($z[]%d >= 1 && $z[]%d < -9 && $123 == 1)) && $\~ + -234 - -0 / -9.6"],
                 ["123", "123"],
                 ["((0))", "((0))"],
                 ["($x$ == 2) >= ($swapvar$ < 7.0)", "($x == 2) >= ($swapvar < 7.0)"],
                 ["($x$ == 2) == ($swapvar$ < 7.0)", "($x == 2) == ($swapvar < 7.0)"],
                 ["~~(~$a$ + 8 & $b$ > ~8)", "!!(!$a + 8 && $b > !8)"]
                 ]

        for test in tests:
            inputText = test[0]
            expectedQuery = test[1]
            ctx = FRB.ParseContext(inputText, file = file, startLineNo = startLineNo)

            tokens = self._tokenizer.simplifiedMaximalMunch(ctx)
            parseTree = self._parser.parse(tokens, ctx = ctx)

            error = None

            resultQuery = FRB.SympyIfPredGenerator.generate(parseTree)

            if (error is not None):
                self.assertEqual(type(error), type(expectedQuery))
            else:
                self.assertEqual(expectedQuery, resultQuery)

    # ================================================
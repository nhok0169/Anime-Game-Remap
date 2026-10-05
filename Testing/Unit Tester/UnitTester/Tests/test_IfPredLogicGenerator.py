import sys
from sympy import Eq, Symbol, Ge, Lt, And, Or, Not, Ne, Gt, simplify_logic
from sympy.logic.boolalg import Boolean

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class IfPredLogicGeneratorTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._tokenizer = FRB.IfPredTokenizer()
        cls._tokenizer.setup()

    def setUp(self):
        super().setUp()

        # A fresh parser per test -- see SLR1ParserTest's identically-shaped setUp for why (id
        # generation is no longer mockable on the compiled class). Harmless here specifically
        # since this test's own assertion (compareQuery) never looks at node/state ids at all --
        # it only cares about the final generated sympy query, which is structural, not id-based.
        self._parser = FRB.IfPredParser()

    # ================== generate ====================

    def test_differentIfPreds_queriesGenerated(self):
        vars = {"swapvar": Symbol("$swapvar$"),
                "x": Symbol("$x$"),
                "\\swapvar": Symbol("$\\swapvar$"),
                "z[]%d": Symbol("$z[]%d$"),
                "123": Symbol("$123$"),
                "\\~": Symbol("$\\~$"),
                "a": Symbol("$a$"),
                "b": Symbol("$b$")}

        file = "MyFile.agr"
        startLineNo = 8

        tests = [
                 [" ", False],
                 ["$swapvar == 5", Eq(vars["swapvar"], 5)],
                 ["$x >= 5", Ge(vars["x"], 5)],
                 ["($\\swapvar < 7)", Lt(vars["\\swapvar"], 7)],
                 ["(($x >= 5 && $\\swapvar < (7 + 3.456 * -0.0)) || !($z[]%d >= 1 && $z[]%d < -9 && $123 == null)) \t && $\\~ + -234 -        -0 / -9.6",
                  Ne(vars["\\~"], 234)],
                 ["123", True],
                 ["((0))", False],
                 ["($x == 2) >= ($swapvar < 7.0)", TypeError("Can only compare inequalities with Expr")],
                 ["($x == 2) == ($swapvar < 7.0)", Eq(Eq(vars["x"], 2), Lt(vars["swapvar"], 7.0))],
                 ["$b > !8", Gt(vars["b"], 0)]
                 ]

        for test in tests:
            inputText = test[0]
            expectedQuery = test[1]
            ctx = FRB.ParseContext(inputText, file = file, startLineNo = startLineNo)

            tokens = self._tokenizer.simplifiedMaximalMunch(ctx)
            parseTree = self._parser.parse(tokens, ctx = ctx)

            error = None
            try:
                resultQuery = FRB.IfPredLogicGenerator.generate(parseTree, vars, simplify = True)
            except Exception as e:
                error = e

            if (error is not None):
                self.assertEqual(type(error), type(expectedQuery))
            else:
                if (isinstance(expectedQuery, Boolean)):
                    expectedQuery = simplify_logic(expectedQuery)

                self.compareQuery(resultQuery, expectedQuery)

    # ================================================
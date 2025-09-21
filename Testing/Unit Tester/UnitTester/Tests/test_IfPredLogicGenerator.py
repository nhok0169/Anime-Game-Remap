import sys
from sympy import Equivalent, Eq, Symbol, Ge, Lt, And, Or, Not, Ne
from sympy.logic.boolalg import Boolean, BooleanFalse
from typing import Union

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class IfPredLogicGeneratorTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._tokenizer = FRB.IfPredTokenizer()
        cls._parser = FRB.IfPredParser(setup = False)

        cls._prodId = 0
        cls._stateId = 0

        cls._tokenizer.setup()

    def setUp(self):
        super().setUp()

        self._prodId = 0
        self._stateId = 0
        self._nodeId = 0

        self.patch("src.FixRaidenBoss2.BaseSLR1Parser._generateStateId", side_effect = self._generateStateId)
        self.patch("src.FixRaidenBoss2.BaseSLR1Parser._generateProductionId", side_effect = self._generateProductionId)
        self.patch("src.FixRaidenBoss2.BaseSLR1Parser._generateParserNodeId", side_effect = self._generateNodeId)

        self._parser.setup()

    def _generateStateId(self) -> int:
        self._prodId += 1
        return self._prodId
    
    def _generateProductionId(self) -> int:
        self._stateId -= 1
        return self._stateId
    
    def _generateNodeId(self) -> int:
        self._nodeId += 1
        return self._nodeId
    
    def compareQuery(self, query1: Union[int, float, bool, Boolean], query2: Union[int, float, bool, Boolean]):
        self.assertEqual(type(query1), type(query2))

        if (isinstance(query1, int) or isinstance(query1, bool) or isinstance(query1, float)):
            self.assertEqual(query1, query2)

        self.assertEqual(Equivalent(query1, query2), True)
    
    # ================== generate ====================

    def test_differentIfPreds_queriesGenerated(self):
        vars = {"swapvar": Symbol("swapvar"),
                "x": Symbol("x"),
                "/swapvar": Symbol("/swapvar"),
                "z[]%d": Symbol("z[]%d"),
                "123": Symbol("123"),
                "*!": Symbol("*!")}

        file = "MyFile.agr"
        startLineNo = 8

        tests = [[" ", False],
                 ["$swapvar == 5", Eq(vars["swapvar"], 5)],
                 ["$x >= 5", Ge(vars["x"], 5)],
                 ["($/swapvar < 7)", Lt(vars["/swapvar"], 7)],
                 ["(($x >= 5 && $/swapvar < (7 + 3.456 * -0.0)) || !($z[]%d >= 1 && $z[]%d < -9 && $123 == null)) \t && $*! + -234 -        -0 / -9.6",
                  And(Or(And(Ge(vars["x"], 5), Lt(vars["/swapvar"], 7 + 3.456 * -0.0)), 
                         Not(And(Ge(vars["z[]%d"], 1), Lt(vars["z[]%d"], -9), Eq(vars["123"], 0)))),
                      Ne(vars["*!"] + -234 - -0 / -9.6, 0))],
                 ["123", True],
                 ["((0))", False],
                 ["($x == 2) >= ($swapvar < 7.0)", TypeError("Can only compare inequalities with Expr")],
                 ["($x == 2) == ($swapvar < 7.0)", Eq(Eq(vars["x"], 2), Lt(vars["swapvar"], 7.0))]]

        for test in tests:
            self._prodId = 0
            self._stateId = 0
            self._nodeId = 0

            inputText = test[0]
            expectedQuery = test[1]
            ctx = FRB.ParseContext(inputText, file = file, startLineNo = startLineNo)

            tokens = self._tokenizer.simplifiedMaximalMunch(ctx)
            parseTree = self._parser.parse(tokens, ctx = ctx)

            error = None
            try:
                resultQuery = FRB.IfPredLogicGenerator.generate(parseTree, vars)
            except Exception as e:
                error = e

            if (error is not None):
                self.assertEqual(type(error), type(expectedQuery))
            else:
                self.compareQuery(resultQuery, expectedQuery)

    # ================================================
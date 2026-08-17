import sys
import unittest.mock as mock
from typing import List, Tuple, Dict, Union, Hashable

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class SLR1ParserTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls.startToken = "TESTSTARTTOKEN"
        cls.endToken = "TESTENDTOKEN"
        cls.nullToken = "TESTNULLTOKEN"

        cls._parser = FRB.BaseSLR1Parser([("SPRIME", [cls.startToken, "a", cls.endToken]),
                                          ("S", ["S", "R", "S"]),
                                          ("S", ["a"]),
                                          ("S", ["b"]),
                                          ("R", ["+"]),
                                          ("R", ["-"])], "SPRIME", 
                                         startToken = cls.startToken, endToken = cls.endToken, nullToken = cls.nullToken,
                                         setup = False)
        
        cls._prodId = 0
        cls._stateId = 0
        cls._nodeId = 0

    def setUp(self):
        super().setUp()

        self._parser.clear()
        self._prodId = 0
        self._stateId = 0
        self._nodeId = 0

        self.patch("src.py.FixRaidenBoss2.BaseSLR1Parser._generateStateId", side_effect = self._generateStateId)
        self.patch("src.py.FixRaidenBoss2.BaseSLR1Parser._generateProductionId", side_effect = self._generateProductionId)
        self.patch("src.py.FixRaidenBoss2.BaseSLR1Parser._generateParserNodeId", side_effect = self._generateNodeId)

    def compareProduction(self, production1: Tuple[str, List[str]], production2: Tuple[str, List[str]]):
        self.assertEqual(production1[0], production2[0])
        self.compareList(production1[1], production2[1])

    def compareProductions(self, productions1: List[Tuple[str, List[str]]], productions2: List[Tuple[str, List[str]]]):
        self.compareList(productions1, productions2, lambda prod1, prod2: self.compareProduction(prod1, prod2))

    def compareReductions(self, reductions1: Dict[Hashable, Union[int, Dict[str, int]]], reductions2: Dict[Hashable, Union[int, Dict[str, int]]]):
        self.compareSet(set(reductions1.keys()), set(reductions2.keys()))
        for stateId in reductions1:
            reduction1 = reductions1[stateId]
            reduction2 = reductions2[stateId]

            self.assertEqual(type(reduction1), type(reduction2))
            if (not isinstance(reduction1, dict)):
                self.assertEqual(reduction1, reduction2)
                continue

            self.compareDict(reduction1, reduction2)

    def _generateStateId(self) -> int:
        self._prodId += 1
        return self._prodId
    
    def _generateProductionId(self) -> int:
        self._stateId -= 1
        return self._stateId
    
    def _generateNodeId(self) -> int:
        self._nodeId += 1
        return self._nodeId

    # ================================================
    # ============ productions.setter ================

    def test_newProductions_productionsValidated(self):
        tests = [[[], None],
                 [[("SPRIME", [self.startToken, "a", self.endToken]),
                   ("S", [self.nullToken]),
                   ("S", ["a", "S", "b"])], None],
                 [[(self.startToken, [self.nullToken]),
                   (self.startToken, ["a", "S", "b"])], KeyError]]
        
        for test in tests:
            newProductions = test[0]
            expectedErrorType = test[1]

            resultError = None
            try:
                self._parser.productions = newProductions
            except Exception as e:
                resultError = e

            if (expectedErrorType is None):
                self.assertIsNone(resultError)
                self.compareProductions(self._parser.productions, newProductions)
            else:
                self.assertIsInstance(resultError, expectedErrorType)

    # ================================================
    # ============ startSymbol.setter ================

    def test_newStartSymbol_startSymbolIsNonTerminal(self):
        tests = [["S", None],
                 ["someTerminalSymbol", KeyError]]

        for test in tests:
            newStartSymbol = test[0]
            expectedErrorType = test[1]

            resultError = None
            try:
                self._parser.startSymbol = newStartSymbol
            except Exception as e:
                resultError = e

            if (expectedErrorType is None):
                self.assertIsNone(resultError)
                self.assertEqual(self._parser.startSymbol, newStartSymbol)
            else:
                self.assertIsInstance(resultError, expectedErrorType)

    # ================================================
    # ================== clear =======================

    @mock.patch("src.py.FixRaidenBoss2.DFA.clear")
    def test_clearParser_parserCleared(self, m_clear):
        self._parser.setup()
        self._parser.clear()

        self.compareDict(self._parser.nullable, {})
        self.compareDict(self._parser.first, {})
        self.compareDict(self._parser.follow, {})

        self.assertEqual(m_clear.call_count, 3)

    # ================================================
    # ============ getNonTermSymbols =================

    def test_differentProductions_nonTerminalSymbolsFound(self):
        tests = [[[], set()],
                 [[("SPRIME", [self.startToken, "a", self.endToken]),
                   ("S", ["S", "R", "S"]),
                   ("S", ["a"]),
                   ("S", ["b"]),
                   ("R", ["+"]),
                   ("R", ["-"])], {"SPRIME", "S", "R"}]]
        
        for test in tests:
            self._parser.productions = test[0]
            result = self._parser.getNonTermSymbols()
            self.compareSet(result, test[1])

    # ================================================
    # ============= getNullableSet ===================

    def test_differentCFG_getNullableSet(self):
        tests = [[[("SPRIME", [self.startToken, "S", self.endToken]),
                   ("S", ["c"]),
                   ("S", ["Q", "R", "S"]),
                   ("Q", ["R"]),
                   ("Q", ["d"]),
                   ("R", [self.nullToken]),
                   ("R", ["b"])], "SPRIME", {"SPRIME": False, "S": False, "Q": True, "R": True}]]
        
        for test in tests:
            self._parser.clear()
            self._parser.productions = test[0]
            self._parser.startSymbol = test[1]

            expected = test[2]
            result = self._parser.getNullableSet()

            self.compareDict(result, expected)

    # ================================================
    # ============== getFirstSet =====================

    def test_differentCFG_getFirstSet(self):
        tests = [[[("SPRIME", [self.startToken, "S", self.endToken]),
                   ("S", ["c"]),
                   ("S", ["Q", "R", "S"]),
                   ("Q", ["R"]),
                   ("Q", ["d"]),
                   ("R", [self.nullToken]),
                   ("R", ["b"])], "SPRIME", {"SPRIME": {self.startToken}, "S": {"b", "c", "d"}, "Q": {"b", "d"}, "R": {"b"}}]]
        
        for test in tests:
            self._parser.clear()
            self._parser.productions = test[0]
            self._parser.startSymbol = test[1]

            expected = test[2]
            result = self._parser.getFirstSet()

            self.compareDict(result, expected, lambda resKeys, expectedKeys: self.compareSet(resKeys, expectedKeys))

    # ================================================
    # ============== getFollowSet ====================

    def test_differentCFG_getFollowSet(self):
        tests = [[[("SPRIME", [self.startToken, "S", self.endToken]),
                   ("S", ["c"]),
                   ("S", ["Q", "R", "S"]),
                   ("Q", ["R"]),
                   ("Q", ["d"]),
                   ("R", [self.nullToken]),
                   ("R", ["b"])], "SPRIME", {"S": {self.endToken}, "Q": {"b", "c", "d"}, "R": {"b", "c", "d"}}]]
        
        for test in tests:
            self._parser.clear()
            self._parser.productions = test[0]
            self._parser.startSymbol = test[1]

            expected = test[2]
            result = self._parser.getFollowSet()

            self.compareDict(result, expected, lambda resKeys, expectedKeys: self.compareSet(resKeys, expectedKeys))

    # ================================================
    # ============== constructDFA ====================

    def test_differentCFG_slr1DFAConstructed(self):
        tests = [[[("SPRIME", [self.startToken, "S", self.endToken]), # LR(0)
                   ("S", ["S", "+", "T"]),
                   ("S", ["T"]),
                   ("T", ["d"])], "SPRIME",
                   {1: [(-1, 0)], 2: [(-2, 0), (-3, 1), (-4, 2), (-5, 3)], 3: [(-6, 0), (-7, 1)], 4: [(-8, 2)], 5: [(-9, 3)], 6: [(-10, 0)], 7: [(-11, 1), (-12, 3)], 8: [(-13, 1)]},
                   {1: {self.startToken: 2}, 2: {'S': 3, 'T': 4, 'd': 5}, 3: {self.endToken: 6, '+': 7}, 7: {'T': 8, 'd': 5}},
                   {5: 3, 4: 2, 8: 1, 6: 0},
                   {8, 4, 5, 6}],

                 [[("SPRIME", [self.startToken, "S", self.endToken]), # SLR(1)
                   ("S", ["T", "+", "S"]),
                   ("S", ["T"]),
                   ("T", ["d"])], "SPRIME",
                   {1: [(-1, 0)], 2: [(-2, 0), (-3, 1), (-4, 2), (-5, 3)], 3: [(-6, 0)], 4: [(-7, 1), (-8, 2)], 5: [(-9, 3)], 6: [(-10, 1), (-11, 1), (-12, 2), (-13, 3)], 7: [(-14, 1)], 8: [(-18, 0)]},
                   {1: {self.startToken: 2}, 2: {'S': 3, 'T': 4, 'd': 5}, 4: {'+': 6}, 6: {'S': 7, 'T': 4, 'd': 5}, 3: {self.endToken: 8}},
                   {5: 3, 4: {self.endToken: 2}, 7: 1, 8: 0},
                   {8, 4, 5, 7}]]
        
        for test in tests:
            self._parser.clear()
            self._prodId = 0
            self._stateId = 0

            self._parser.productions = test[0]
            self._parser.startSymbol = test[1]

            expectedStates = test[2]
            expectedTransitions = test[3]
            expectedReductions = test[4]
            expectedAcceptStates = test[5]

            resultStates = self._parser.constructDFA()

            self.compareDictList(resultStates, expectedStates, lambda resProd, expectedProd: self.compareList(resProd, expectedProd))
            self.compareReductions(self._parser._reductions, expectedReductions)
            self.assertEqual(self._parser._dfa.acceptLen(), len(expectedAcceptStates))

    # ================================================
    # ================== setup =======================

    @mock.patch("src.py.FixRaidenBoss2.BaseSLR1Parser.constructDFA")
    def test_differentCFG_parserSetup(self, m_constructDFA):
        tests = [[[("SPRIME", [self.startToken, "S", self.endToken]),
                   ("S", ["S", "+", "T"]),
                   ("S", ["T"]),
                   ("T", ["d"])], "SPRIME",
                   {'T': False, 'SPRIME': False, 'S': False},
                   {'SPRIME': {self.startToken}, 'S': {'d'}, 'T': {'d'}},
                   {'S': {self.endToken, '+'}, 'T': {self.endToken, '+'}}]]


        for test in tests:
            self._parser.productions = test[0]
            self._parser.startSymbol = test[1]

            self._parser.setup()

            m_constructDFA.assert_called_with(updateNullable = False, updateFirst = False, updateFollow = False)
            self.compareDict(test[2], self._parser.nullable)
            self.compareDict(test[3], self._parser.first, lambda resKeys, expectedKeys: self.compareSet(resKeys, expectedKeys))
            self.compareDict(test[4], self._parser.follow, lambda resKeys, expectedKeys: self.compareSet(resKeys, expectedKeys))

    # ================================================
    # ================== parse =======================

    def test_differentCFGAndText_parseTreeGenerated(self):
        file = "MyFile.agr"
        startLineNo = 8

        tests = [[[("SPRIME", [self.startToken, "S", self.endToken]),
                   ("S", ["A", "c", "B"]),
                   ("A", ["a", "b"]),
                   ("A", ["f", "f"]),
                   ("B", ["d", "e", "f"]),
                   ("B", ["e", "f"])], "SPRIME", "abcdef",
                   FRB.ParseTree({
                        1: FRB.ParseNode(1, token = FRB.Token(self.startToken, self.startToken, 8, 0)),
                        2: FRB.ParseNode(2, token = FRB.Token("a", "a", 8, 1)),
                        3: FRB.ParseNode(3, token = FRB.Token("b", "b", 8, 2)),
                        4: FRB.ParseNode(4, prodId=2),
                        5: FRB.ParseNode(5, token = FRB.Token("c", "c", 8, 3)),
                        6: FRB.ParseNode(6, token = FRB.Token("d", "d", 8, 4)),
                        7: FRB.ParseNode(7, token = FRB.Token("e", "e", 8, 5)),
                        8: FRB.ParseNode(8, token = FRB.Token("f", "f", 8, 6)),
                        9: FRB.ParseNode(9, prodId=4),
                        10: FRB.ParseNode(10, prodId=1),
                        11: FRB.ParseNode(11, token = FRB.Token(self.endToken, self.endToken, 9, 0)),
                        12: FRB.ParseNode(12, prodId=0)
                   },
                   {4: [2, 3], 9: [6, 7, 8], 10: [4, 5, 9], 12: [1, 10, 11]},
                   12)],
                   
                [[("SPRIME", [self.startToken, "S", self.endToken]), # LR(0)
                  ("S", ["S", "+", "T"]),
                  ("S", ["T"]),
                  ("T", ["d"])], "SPRIME", "d+d+d-d+d+d",
                  FRB.Token("-", "-", 8, 6)]]
        
        for test in tests:
            self._parser.productions = test[0]
            self._parser.startSymbol = test[1]
            self._parser.setup()

            inputText = test[2]
            expected = test[3]

            ctxSrc = inputText if (isinstance(inputText, str)) else map(lambda token: token.val, inputText)
            ctx = FRB.ParseContext(ctxSrc, file = file, startLineNo = startLineNo)

            error = None
            try:
                result = self._parser.parse(inputText, ctx = ctx)
            except FRB.SyntaxErr as e:
                error = e

            if (error is not None):
                expected = FRB.SyntaxErr(ctx, expected, process = "parsing")
                self.compareSyntaxErr(error, expected)
            else:
                self.compareParseTree(result, expected)

    # ================================================
import sys
import unittest.mock as mock

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class IfPredTokenizerTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._tokenizer = FRB.IfPredTokenizer()

    def setUp(self):
        super().setUp()
        self._tokenizer.clear()
        self._tokenizer.addStartState()

    # =============== clear ==========================

    @mock.patch("src.FixRaidenBoss2.DFA.clear")
    def test_clearTokenizer_dfaReset(self, m_clear):
        self._tokenizer.clear()
        m_clear.assert_called_once()

    # ================================================
    # =============== addStartState ==================

    def test_addStartState_startStateAdded(self):
        startId = self._tokenizer.addStartState()
        self.assertEqual(startId, "")
        self.assertEqual(len(self._tokenizer._dfa._states), 1)

    # ================================================
    # =============== addKeyword =====================

    def test_addKeywords_statesAndTransitionsAdded(self):
        tests = [["apple", 5, 5],
                 ["app", 0, 0],
                 ["api test", 6, 6],
                 ["tshirt", 6, 6],
                 ["", 0, 0]]

        statesLen = 1
        transititionLen = 0

        for test in tests:
            keyword = test[0]
            statesLen += test[1]
            transititionLen += test[2]

            self._tokenizer.addKeyword(keyword)
            self.assertEqual(len(self._tokenizer._dfa._states), statesLen)

            resultTransitionsLen = 0
            for keyNeighbours in self._tokenizer._dfa._neighbours.values():
                resultTransitionsLen += len(keyNeighbours)

            self.assertEqual(resultTransitionsLen, transititionLen)

    # ================================================
    # ======== addASCIIRangeTransitions ==============

    def test_addASCIIRangeTransitions_transitionsAdded(self):
        tests = [["", "a", "z", "state1", 26],
                 ["state1", "0", "0", "state2", 1],
                 ["state2", "9", "0", "stat3", 0],
                 ["", "A", "Z", "state1", 26],
                 ["", "M", "k", "state1", 6]]
        
        transititionLen = 0
        for test in tests:
            srcState = test[0]
            rangeStart = test[1]
            rangeEnd = test[2]
            dstState = test[3]
            transititionLen += test[4]

            self._tokenizer.addASCIIRangeTransitions(srcState, rangeStart, rangeEnd, dstState)

            resultTransitionsLen = 0
            for keyNeighbours in self._tokenizer._dfa._neighbours.values():
                resultTransitionsLen += len(keyNeighbours)

            self.assertEqual(resultTransitionsLen, transititionLen)

    # ================================================
    # ======== simplifiedMaximalMunch ================

    def test_differentPredicates_predicatesTokenized(self):
        self._tokenizer.setup()

        tests = [[" ", False, [], True],
                 ["\t", True, [("TAB", "\t")], True],
                 ["$swapvar == 5", False, [("ID", "$swapvar"), ("EQ", "=="), ("INT", "5")], True],
                 ['(($x >= 5 && $/swapvar < (7 + 3.456 * -0.0)) || !($z[]%d >= "hello world" && $z[]%d < \'goodbye world\' && $123 == null)) \t && $*! % -234 -        -0 / -9.6', False,
                  [('LPAREN', '('), ('LPAREN', '('), ('ID', '$x'), ('GE', '>='), ('INT', '5'), ('AND', '&&'), ('ID', '$/swapvar'), 
                   ('LT', '<'), ('LPAREN', '('), ('INT', '7'), ('PLUS', '+'), ('FLOAT', '3.456'), ('STAR', '*'), ('FLOAT', '-0.0'), 
                   ('RPAREN', ')'), ('RPAREN', ')'), ('OR', '||'), ('NOT', '!'), ('LPAREN', '('), ('ID', '$z[]%d'), ('GE', '>='), 
                   ('STRING', '"hello world"'), ('AND', '&&'), ('ID', '$z[]%d'), ('LT', '<'), ('STRING', "'goodbye world'"), 
                   ('AND', '&&'), ('ID', '$123'), ('EQ', '=='), ('NULL', 'null'), ('RPAREN', ')'), ('RPAREN', ')'), 
                   ('AND', '&&'), ('ID', '$*!'), ('PCT', '%'), ('INT', '-234'), ('MINUS', '-'), ('INT', '-0'), ('SLASH', '/'), ('FLOAT', '-9.6')], True],

                 ['(($x >= 5 && $/swapvar < (7 + 3.456 * -0.0)) || !($z[]%d >= "hello world" && $z[]%d < \'goodbye world\' && $123 == null)) \t && $*! % -234 -        -0 / -9.6s', False,
                  [('LPAREN', '('), ('LPAREN', '('), ('ID', '$x'), ('GE', '>='), ('INT', '5'), ('AND', '&&'), ('ID', '$/swapvar'), 
                   ('LT', '<'), ('LPAREN', '('), ('INT', '7'), ('PLUS', '+'), ('FLOAT', '3.456'), ('STAR', '*'), ('FLOAT', '-0.0'), 
                   ('RPAREN', ')'), ('RPAREN', ')'), ('OR', '||'), ('NOT', '!'), ('LPAREN', '('), ('ID', '$z[]%d'), ('GE', '>='), 
                   ('STRING', '"hello world"'), ('AND', '&&'), ('ID', '$z[]%d'), ('LT', '<'), ('STRING', "'goodbye world'"), 
                   ('AND', '&&'), ('ID', '$123'), ('EQ', '=='), ('NULL', 'null'), ('RPAREN', ')'), ('RPAREN', ')'), 
                   ('AND', '&&'), ('ID', '$*!'), ('PCT', '%'), ('INT', '-234'), ('MINUS', '-'), ('INT', '-0'), ('SLASH', '/'), ('FLOAT', '-9.6')], False],
                   
                 ['(($x >= 5 && $$swapvar < (7 + 3.456 * -0.0)) || !($z[]%d >= "hello world" && $z[]%d < \'goodbye world\' && $123 == null)) \t && $*! % -234 -        -0 / -9.6', False,
                  [('LPAREN', '('), ('LPAREN', '('), ('ID', '$x'), ('GE', '>='), ('INT', '5'), ('AND', '&&')], False]]
        
        for test in tests:
            src = test[0]
            whitespaces = test[1]
            expectedTokens = test[2]
            expectedIsParsed = test[3]

            resultTokens, resultIsParsed = self._tokenizer.simplifiedMaximalMunch(src, whitespaces = whitespaces)

            self.compareList(resultTokens, expectedTokens, compareValues = lambda resTokenData, expectedTokenData: self.compareList(resTokenData, expectedTokenData))
            self.assertEqual(resultIsParsed, expectedIsParsed)

    # ================================================
import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class IfPredTokenizerTest(BaseUnitTest):
    """
    Black-box tests against the C++-backed IfPredTokenizer's public API -- unlike the pure-Python
    predecessor (see test_IfPredTokenizerOld.py), this class's internal DFA isn't exposed to
    Python at all, so there's no equivalent of the old white-box '._dfa.stateLen()'-style tests
    here; behavior is verified purely through simplifiedMaximalMunch()'s actual output.
    """

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._tokenizer = FRB.IfPredTokenizer()

    def test_isFilteredTokenizer(self):
        self.assertIsInstance(self._tokenizer, FRB.FilteredTokenizer)
        self.assertIsInstance(self._tokenizer, FRB.BaseTokenizer)

    # ======== simplifiedMaximalMunch ================

    def test_differentPredicates_predicatesTokenized(self):
        tests = [# A genuinely empty predicate (as opposed to " ", below) used to crash the C++
                 # tokenizer: 'lines' ends up empty (0 lines, not 1 empty line -- see ParseContext),
                 # so simplifiedMaximalMunch() never enters its main loop and 'isAccept' stays at its
                 # initial 'false', which the post-loop check couldn't distinguish from "a real
                 # in-progress token got rejected" -- raising a SyntaxErr for a token that was never
                 # actually fed to the DFA. Should just tokenize to no tokens instead.
                 ["", False, [], True],
                 [" ", False, [], True],
                 ["\t", True, [FRB.Token("TAB", "\t", 1, 1)], True],
                 ["$swapvar == 5", False, [FRB.Token('ID', '$swapvar', 1, 1), FRB.Token('EQ', '==', 1, 10), FRB.Token('INT', '5', 1, 13)], True],
                 [FRB.ParseContext('(($x >= 5 && $\swapvar < (7 + 3.456 * -0.0)) || !($z[]%d >= 1 && $z[]%d < -9 && $123 == null)) \t && $\\~ + -234 -        -0 / -9.6',
                                   file = "poopy.ini",
                                   startLineNo = 5), False,
                  [FRB.Token('LPAREN', '(', 5, 1), FRB.Token('LPAREN', '(', 5, 2), FRB.Token('ID', '$x', 5, 3), FRB.Token('GE', '>=', 5, 6), FRB.Token('INT', '5', 5, 9),
                   FRB.Token('AND', '&&', 5, 11), FRB.Token('ID', '$\swapvar', 5, 14), FRB.Token('LT', '<', 5, 24), FRB.Token('LPAREN', '(', 5, 26),
                   FRB.Token('INT', '7', 5, 27), FRB.Token('PLUS', '+', 5, 29), FRB.Token('FLOAT', '3.456', 5, 31), FRB.Token('STAR', '*', 5, 37),
                   FRB.Token('FLOAT', '-0.0', 5, 39), FRB.Token('RPAREN', ')', 5, 43), FRB.Token('RPAREN', ')', 5, 44), FRB.Token('OR', '||', 5, 46),
                   FRB.Token('NOT', '!', 5, 49), FRB.Token('LPAREN', '(', 5, 50), FRB.Token('ID', '$z[]%d', 5, 51), FRB.Token('GE', '>=', 5, 58),
                   FRB.Token('INT', '1', 5, 61), FRB.Token('AND', '&&', 5, 63), FRB.Token('ID', '$z[]%d', 5, 66), FRB.Token('LT', '<', 5, 73),
                   FRB.Token('INT', '-9', 5, 75), FRB.Token('AND', '&&', 5, 78), FRB.Token('ID', '$123', 5, 81), FRB.Token('EQ', '==', 5, 86),
                   FRB.Token('NULL', 'null', 5, 89), FRB.Token('RPAREN', ')', 5, 93), FRB.Token('RPAREN', ')', 5, 94), FRB.Token('AND', '&&', 5, 98),
                   FRB.Token('ID', '$\\~', 5, 101), FRB.Token('PLUS', '+', 5, 105), FRB.Token('INT', '-234', 5, 107), FRB.Token('MINUS', '-', 5, 112),
                   FRB.Token('INT', '-0', 5, 121), FRB.Token('SLASH', '/', 5, 124), FRB.Token('FLOAT', '-9.6', 5, 126)], True],

                 [FRB.ParseContext('(($x >= 5 && $\swapvar < (7 + 3.456 * -0.0)) || !($z[]%d >= 1 && $z[]%d < -9 && $123 == null)) \t && $\\~ + -234 -        -0 / -9.6s',
                                   file = "poopy.ini",
                                   startLineNo = 5), False,
                  FRB.Token(None, "s", 5, 130), False],

                 ['(($x >= 5 && $$swapvar < (7 + 3.456 * -0.0)) || !($z[]%d >= 1 && $z[]%d < -9 && $123 == null)) \t && $*! + -234 -        -0 / -9.6', False,
                  FRB.Token(None, "$$", 1, 14), False],

                 ['0.', False, FRB.Token(None, '0.', 1, 1), False]]

        for test in tests:
            src = test[0]
            whitespaces = test[1]
            expectedTokens = test[2]
            expectedIsParsed = test[3]

            error = None
            try:
                resultTokens = self._tokenizer.simplifiedMaximalMunch(src, includeFiltered = whitespaces)
            except FRB.SyntaxErr as e:
                error = e

            if (not expectedIsParsed):
                ctx = src if (isinstance(src, FRB.ParseContext)) else FRB.ParseContext(src)
                expectedErr = FRB.SyntaxErr(ctx, expectedTokens, process = "tokenization")
                self.assertIsInstance(error, FRB.SyntaxErr)
                self.compareSyntaxErr(error, expectedErr)
            else:
                self.compareList(resultTokens, expectedTokens, lambda resToken, expectedToken: self.compareToken(resToken, expectedToken))

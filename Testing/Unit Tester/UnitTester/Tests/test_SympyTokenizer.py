import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class SympyTokenizerTest(BaseUnitTest):
    """
    Black-box tests against the C++-backed SympyTokenizer's public API -- see
    test_IfPredTokenizer.py's own docstring for why there's no white-box '._dfa' equivalent here
    (that coverage stays with the pure-Python predecessor, test_SympyTokenizerOld.py).
    """

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._tokenizer = FRB.SympyTokenizer()

    def test_isFilteredTokenizer(self):
        self.assertIsInstance(self._tokenizer, FRB.FilteredTokenizer)
        self.assertIsInstance(self._tokenizer, FRB.BaseTokenizer)

    # ======== simplifiedMaximalMunch ================

    def test_differentPredicates_predicatesTokenized(self):
        tests = [[" ", False, [], True],
                 ["\t", True, [FRB.Token("TAB", "\t", 1, 1)], True],
                 ["$swapvar$ == 5", False, [FRB.Token('ID', '$swapvar$', 1, 1), FRB.Token('EQ', '==', 1, 11), FRB.Token('INT', '5', 1, 14)], True],
                 ["Eq($swapvar$, 5)", False, [FRB.Token("EQFUNC", "Eq", 1, 1), FRB.Token("LPAREN", "(", 1, 3), FRB.Token("ID", "$swapvar$", 1, 4),
                                              FRB.Token("COMMA", ",", 1, 13), FRB.Token("INT", "5", 1, 15), FRB.Token("RPAREN", ")", 1, 16)], True],
                 [FRB.ParseContext('(($x$ >= 5 && $\swapvar$ < (7 + 3.456 * -0.0)) || ~($z[]%d$ >= 1 && $z[]%d$ < -9 && $123$ == 0)) \t && $\\~$ + -234 -        -0 / -9.6',
                                   file = "poopy.ini",
                                   startLineNo = 5), False,
                 [FRB.Token("LPAREN", "(", 5, 1), FRB.Token("LPAREN", "(", 5, 2), FRB.Token("ID", "$x$", 5, 3), FRB.Token("GE", ">=", 5, 7), FRB.Token("INT", "5", 5, 10),
                  FRB.Token("AND", "&", 5, 12), FRB.Token("AND", "&", 5, 13), FRB.Token("ID", "$\swapvar$", 5, 15), FRB.Token("LT", "<", 5, 26), FRB.Token("LPAREN", "(", 5, 28),
                  FRB.Token("INT", "7", 5, 29), FRB.Token("PLUS", "+", 5, 31), FRB.Token("FLOAT", "3.456", 5, 33), FRB.Token("STAR", "*", 5, 39), FRB.Token("FLOAT", "-0.0", 5, 41),
                  FRB.Token("RPAREN", ")", 5, 45), FRB.Token("RPAREN", ")", 5, 46), FRB.Token("OR", "|", 5, 48), FRB.Token("OR", "|", 5, 49), FRB.Token("NOT", "~", 5, 51),
                  FRB.Token("LPAREN", "(", 5, 52), FRB.Token("ID", "$z[]%d$", 5, 53), FRB.Token("GE", ">=", 5, 61), FRB.Token("INT", "1", 5, 64), FRB.Token("AND", "&", 5, 66),
                  FRB.Token("AND", "&", 5, 67), FRB.Token("ID", "$z[]%d$", 5, 69), FRB.Token("LT", "<", 5, 77), FRB.Token("INT", "-9", 5, 79), FRB.Token("AND", "&", 5, 82),
                  FRB.Token("AND", "&", 5, 83), FRB.Token("ID", "$123$", 5, 85), FRB.Token("EQ", "==", 5, 91), FRB.Token("INT", "0", 5, 94), FRB.Token("RPAREN", ")", 5, 95),
                  FRB.Token("RPAREN", ")", 5, 96), FRB.Token("AND", "&", 5, 100), FRB.Token("AND", "&", 5, 101), FRB.Token("ID", "$\~$", 5, 103), FRB.Token("PLUS", "+", 5, 108),
                  FRB.Token("INT", "-234", 5, 110), FRB.Token("MINUS", "-", 5, 115), FRB.Token("INT", "-0", 5, 124), FRB.Token("SLASH", "/", 5, 127), FRB.Token("FLOAT", "-9.6", 5, 129)], True],

                 [FRB.ParseContext('(Or((And(Ge($x$, 5), $\swapvar$ < (7 + 3.456 * -0.0))), ~And(($z[]%d$ >= 1, $z[]%d$ < -9, Ne($123$, 0))))) \t & $\\~$ + -234 -        -0 / -9.6',
                                   file = "poopy.ini",
                                   startLineNo = 5), False,
                  [FRB.Token("LPAREN", "(", 5, 1), FRB.Token("ORFUNC", "Or", 5, 2), FRB.Token("LPAREN", "(", 5, 4), FRB.Token("LPAREN", "(", 5, 5), FRB.Token("ANDFUNC", "And", 5, 6), FRB.Token("LPAREN", "(", 5, 9),
                   FRB.Token("GEFUNC", "Ge", 5, 10), FRB.Token("LPAREN", "(", 5, 12), FRB.Token("ID", "$x$", 5, 13), FRB.Token("COMMA", ",", 5, 16), FRB.Token("INT", "5", 5, 18), FRB.Token("RPAREN", ")", 5, 19),
                   FRB.Token("COMMA", ",", 5, 20), FRB.Token("ID", "$\swapvar$", 5, 22), FRB.Token("LT", "<", 5, 33), FRB.Token("LPAREN", "(", 5, 35), FRB.Token("INT", "7", 5, 36), FRB.Token("PLUS", "+", 5, 38),
                   FRB.Token("FLOAT", "3.456", 5, 40), FRB.Token("STAR", "*", 5, 46), FRB.Token("FLOAT", "-0.0", 5, 48), FRB.Token("RPAREN", ")", 5, 52), FRB.Token("RPAREN", ")", 5, 53), FRB.Token("RPAREN", ")", 5, 54),
                   FRB.Token("COMMA", ",", 5, 55), FRB.Token("NOT", "~", 5, 57), FRB.Token("ANDFUNC", "And", 5, 58), FRB.Token("LPAREN", "(", 5, 61), FRB.Token("LPAREN", "(", 5, 62), FRB.Token("ID", "$z[]%d$", 5, 63),
                   FRB.Token("GE", ">=", 5, 71), FRB.Token("INT", "1", 5, 74), FRB.Token("COMMA", ",", 5, 75), FRB.Token("ID", "$z[]%d$", 5, 77), FRB.Token("LT", "<", 5, 85), FRB.Token("INT", "-9", 5, 87),
                   FRB.Token("COMMA", ",", 5, 89), FRB.Token("NEFUNC", "Ne", 5, 91), FRB.Token("LPAREN", "(", 5, 93), FRB.Token("ID", "$123$", 5, 94), FRB.Token("COMMA", ",", 5, 99), FRB.Token("INT", "0", 5, 101),
                   FRB.Token("RPAREN", ")", 5, 102), FRB.Token("RPAREN", ")", 5, 103), FRB.Token("RPAREN", ")", 5, 104), FRB.Token("RPAREN", ")", 5, 105), FRB.Token("RPAREN", ")", 5, 106), FRB.Token("AND", "&", 5, 110),
                   FRB.Token("ID", "$\~$", 5, 112), FRB.Token("PLUS", "+", 5, 117), FRB.Token("INT", "-234", 5, 119), FRB.Token("MINUS", "-", 5, 124), FRB.Token("INT", "-0", 5, 133), FRB.Token("SLASH", "/", 5, 136),
                   FRB.Token("FLOAT", "-9.6", 5, 138)], True],

                 [FRB.ParseContext('(($x$ >= 5 && $\swapvar$ < (7 + 3.456 * -0.0)) || ~($z[]%d$ >= 1 && $z[]%d$ < -9 && $123$ == 0)) \t && $\\~$ + -234 -        -0 / -9.6s',
                                   file = "poopy.ini",
                                   startLineNo = 5), False,
                  FRB.Token(None, "s", 5, 133), False],

                 ['(($x$ >= 5 && $$swapvar < (7 + 3.456 * -0.0)) || !($z[]%d >= 1 && $z[]%d < -9 && $123 == null)) \t && $*! + -234 -        -0 / -9.6', False,
                  FRB.Token(None, "$$", 1, 15), False],

                 ['0.', False, FRB.Token(None, '0.', 1, 1), False]
                 ]

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

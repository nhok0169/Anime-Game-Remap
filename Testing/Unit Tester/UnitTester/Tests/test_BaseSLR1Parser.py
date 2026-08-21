import sys
from typing import List

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

    def setUp(self):
        super().setUp()

        # A fresh parser per test, rather than one shared/mutated class-level instance. Id
        # generation (state/item/node ids) is now real random UUIDs from the pybind11 binding's
        # own default generator -- there is no longer a patchable _generateStateId/
        # _generateProductionId/_generateParserNodeId Python attribute on the compiled class the
        # way the pure-Python original had, so there's no shared determinism to preserve across
        # tests anyway; see compareParseTreeShape's own note for the full rationale.
        self._parser = self._makeParser(setup = False)

    def _makeParser(self, setup: bool = False) -> FRB.BaseSLR1Parser:
        return FRB.BaseSLR1Parser({0: ("SPRIME", [self.startToken, "a", self.endToken]),
                                    1: ("S", ["S", "R", "S"]),
                                    2: ("S", ["a"]),
                                    3: ("S", ["b"]),
                                    4: ("R", ["+"]),
                                    5: ("R", ["-"])}, "SPRIME",
                                   startToken = self.startToken, endToken = self.endToken, nullToken = self.nullToken,
                                   setup = setup)

    def tokenizeChars(self, text: str, file: str, startLineNo: int) -> List[FRB.Token]:
        """
        Test-only helper: tokenizes 'text' one character per token, the same way the pure-Python
        original's own parse() did internally for a raw str input. The C++-backed parse() only
        accepts an already-tokenized List[Token] -- no real call site ever used the raw-str
        convenience form, so it wasn't ported (see BaseSLR1Parser.h's own note) -- so tests that
        want that shape now build the token list themselves.
        """

        tokens = []
        lines = text.splitlines(keepends = True)
        for i, line in enumerate(lines):
            for j, letter in enumerate(line):
                tokens.append(FRB.Token(letter, letter, startLineNo + i, j + 1))
        return tokens

    # ================================================
    # ========= constructor/productions validation ====

    def test_newProductions_productionsValidated(self):
        tests = [[{}, "SPRIME", KeyError],
                 [{0: (self.startToken, [self.nullToken]),
                   1: (self.startToken, ["a", "S", "b"])}, self.startToken, KeyError]]

        for productions, startSymbol, expectedErrorType in tests:
            resultError = None
            try:
                FRB.BaseSLR1Parser(productions, startSymbol, startToken = self.startToken, endToken = self.endToken, nullToken = self.nullToken, setup = False)
            except Exception as e:
                resultError = e

            self.assertIsInstance(resultError, expectedErrorType)

        # a valid, non-empty production set constructs without error and round-trips through 'productions'
        validProductions = {0: ("SPRIME", [self.startToken, "a", self.endToken]),
                             1: ("S", [self.nullToken]),
                             2: ("S", ["a", "S", "b"])}
        parser = FRB.BaseSLR1Parser(validProductions, "SPRIME", startToken = self.startToken, endToken = self.endToken, nullToken = self.nullToken, setup = False)
        self.compareDict(parser.productions, validProductions)

    # ================================================
    # ============ startSymbol.setter ================

    def test_newStartSymbol_startSymbolIsNonTerminal(self):
        tests = [["S", None],
                 ["someTerminalSymbol", KeyError]]

        for newStartSymbol, expectedErrorType in tests:
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

    def test_clearParser_parserCleared(self):
        self._parser.setup()
        self.assertTrue(len(self._parser.nullable) > 0, "setup() should have populated nullable")

        self._parser.clear()

        self.compareDict(self._parser.nullable, {})
        self.compareDict(self._parser.first, {})
        self.compareDict(self._parser.follow, {})

    # ================================================
    # ============ getNonTermSymbols =================

    def test_differentProductions_nonTerminalSymbolsFound(self):
        # Unlike the pure-Python original, there's no case for a completely empty production set
        # here -- the C++-backed constructor bundles productions and startSymbol together (no
        # separate productions-only setter, see test_newProductions_productionsValidated's own
        # note), and an empty production set has no nonterminal for any startSymbol to validate
        # against, so it can no longer be constructed at all.
        tests = [[{0: ("SPRIME", [self.startToken, "a", self.endToken]),
                   1: ("S", ["S", "R", "S"]),
                   2: ("S", ["a"]),
                   3: ("S", ["b"]),
                   4: ("R", ["+"]),
                   5: ("R", ["-"])}, {"SPRIME", "S", "R"}]]

        for productions, expected in tests:
            parser = FRB.BaseSLR1Parser(productions, "SPRIME", startToken = self.startToken, endToken = self.endToken, nullToken = self.nullToken, setup = False)
            result = parser.getNonTermSymbols()
            self.compareSet(result, expected)

    # ================================================
    # ============= getNullableSet ===================

    def test_differentCFG_getNullableSet(self):
        tests = [[{0: ("SPRIME", [self.startToken, "S", self.endToken]),
                   1: ("S", ["c"]),
                   2: ("S", ["Q", "R", "S"]),
                   3: ("Q", ["R"]),
                   4: ("Q", ["d"]),
                   5: ("R", [self.nullToken]),
                   6: ("R", ["b"])}, "SPRIME", {"SPRIME": False, "S": False, "Q": True, "R": True}]]

        for productions, startSymbol, expected in tests:
            parser = FRB.BaseSLR1Parser(productions, startSymbol, startToken = self.startToken, endToken = self.endToken, nullToken = self.nullToken, setup = False)
            result = parser.getNullableSet()
            self.compareDict(result, expected)

    # ================================================
    # ============== getFirstSet =====================

    def test_differentCFG_getFirstSet(self):
        tests = [[{0: ("SPRIME", [self.startToken, "S", self.endToken]),
                   1: ("S", ["c"]),
                   2: ("S", ["Q", "R", "S"]),
                   3: ("Q", ["R"]),
                   4: ("Q", ["d"]),
                   5: ("R", [self.nullToken]),
                   6: ("R", ["b"])}, "SPRIME", {"SPRIME": {self.startToken}, "S": {"b", "c", "d"}, "Q": {"b", "d"}, "R": {"b"}}]]

        for productions, startSymbol, expected in tests:
            parser = FRB.BaseSLR1Parser(productions, startSymbol, startToken = self.startToken, endToken = self.endToken, nullToken = self.nullToken, setup = False)
            result = parser.getFirstSet()
            self.compareDict(result, expected, lambda resKeys, expectedKeys: self.compareSet(resKeys, expectedKeys))

    # ================================================
    # ============== getFollowSet ====================

    def test_differentCFG_getFollowSet(self):
        tests = [[{0: ("SPRIME", [self.startToken, "S", self.endToken]),
                   1: ("S", ["c"]),
                   2: ("S", ["Q", "R", "S"]),
                   3: ("Q", ["R"]),
                   4: ("Q", ["d"]),
                   5: ("R", [self.nullToken]),
                   6: ("R", ["b"])}, "SPRIME", {"S": {self.endToken}, "Q": {"b", "c", "d"}, "R": {"b", "c", "d"}}]]

        for productions, startSymbol, expected in tests:
            parser = FRB.BaseSLR1Parser(productions, startSymbol, startToken = self.startToken, endToken = self.endToken, nullToken = self.nullToken, setup = False)
            result = parser.getFollowSet()
            self.compareDict(result, expected, lambda resKeys, expectedKeys: self.compareSet(resKeys, expectedKeys))

    # ================================================
    # ============== constructDFA ====================
    #
    # The pure-Python original's own version of this test constructed the DFA directly and
    # inspected its exact internal states/reductions dicts (mocked to deterministic sequential
    # ids). None of that -- constructDFA() itself, ._reductions, ._dfa -- is part of the new
    # C++-backed class's Python-visible surface (it was never part of the *documented* public
    # contract real callers rely on; only setup()/parse() are). So this is black-box now: build
    # the parser (which calls constructDFA() via setup()) and confirm it shift/reduces correctly
    # by actually parsing valid and invalid input -- the same "does the built DFA behave
    # correctly" property, just observed through parse() instead of through internal state.

    def test_differentCFG_slr1DFAConstructed(self):
        tests = [
            # LR(0): S -> S + T | T, T -> d
            [{0: ("SPRIME", [self.startToken, "S", self.endToken]),
              1: ("S", ["S", "+", "T"]),
              2: ("S", ["T"]),
              3: ("T", ["d"])}, "SPRIME", "d+d+d", None],
            [{0: ("SPRIME", [self.startToken, "S", self.endToken]),
              1: ("S", ["S", "+", "T"]),
              2: ("S", ["T"]),
              3: ("T", ["d"])}, "SPRIME", "d+d+d-d", FRB.Token("-", "-", 8, 6)],

            # SLR(1): S -> T + S | T, T -> d (needs FOLLOW-set lookahead disambiguation)
            [{0: ("SPRIME", [self.startToken, "S", self.endToken]),
              1: ("S", ["T", "+", "S"]),
              2: ("S", ["T"]),
              3: ("T", ["d"])}, "SPRIME", "d+d+d", None],
        ]

        for productions, startSymbol, inputText, expectedErrToken in tests:
            parser = FRB.BaseSLR1Parser(productions, startSymbol, startToken = self.startToken, endToken = self.endToken, nullToken = self.nullToken, setup = True)

            file = "MyFile.agr"
            startLineNo = 8
            tokens = self.tokenizeChars(inputText, file, startLineNo)
            ctx = FRB.ParseContext(inputText, file = file, startLineNo = startLineNo)

            error = None
            try:
                parser.parse(tokens, ctx = ctx)
            except FRB.SyntaxErr as e:
                error = e

            if (expectedErrToken is None):
                self.assertIsNone(error, self.getDataFailMsg(error, None, f"Unexpected SyntaxErr while parsing '{inputText}'"))
            else:
                self.assertIsNotNone(error, self.getDataFailMsg(None, expectedErrToken, f"Expected a SyntaxErr while parsing '{inputText}'"))
                expected = FRB.SyntaxErr(ctx, expectedErrToken, process = "parsing")
                self.compareSyntaxErr(error, expected)

    # ================================================
    # ================== setup =======================

    def test_differentCFG_parserSetup(self):
        productions = {0: ("SPRIME", [self.startToken, "S", self.endToken]),
                        1: ("S", ["S", "+", "T"]),
                        2: ("S", ["T"]),
                        3: ("T", ["d"])}
        parser = FRB.BaseSLR1Parser(productions, "SPRIME", startToken = self.startToken, endToken = self.endToken, nullToken = self.nullToken, setup = False)

        parser.setup()

        expectedNullable = {'T': False, 'SPRIME': False, 'S': False}
        expectedFirst = {'SPRIME': {self.startToken}, 'S': {'d'}, 'T': {'d'}}
        expectedFollow = {'S': {self.endToken, '+'}, 'T': {self.endToken, '+'}}

        self.compareDict(expectedNullable, parser.nullable)
        self.compareDict(expectedFirst, parser.first, lambda resKeys, expectedKeys: self.compareSet(resKeys, expectedKeys))
        self.compareDict(expectedFollow, parser.follow, lambda resKeys, expectedKeys: self.compareSet(resKeys, expectedKeys))

    # ================================================
    # ================== parse =======================

    def test_differentCFGAndText_parseTreeGenerated(self):
        file = "MyFile.agr"
        startLineNo = 8

        tests = [[{0: ("SPRIME", [self.startToken, "S", self.endToken]),
                   1: ("S", ["A", "c", "B"]),
                   2: ("A", ["a", "b"]),
                   3: ("A", ["f", "f"]),
                   4: ("B", ["d", "e", "f"]),
                   5: ("B", ["e", "f"])}, "SPRIME", "abcdef",
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

                  [{0: ("SPRIME", [self.startToken, "S", self.endToken]), # LR(0)
                    1: ("S", ["S", "+", "T"]),
                    2: ("S", ["T"]),
                    3: ("T", ["d"])}, "SPRIME", "d+d+d-d+d+d",
                    FRB.Token("-", "-", 8, 6)]]

        for productions, startSymbol, inputText, expected in tests:
            parser = FRB.BaseSLR1Parser(productions, startSymbol, startToken = self.startToken, endToken = self.endToken, nullToken = self.nullToken, setup = True)

            tokens = self.tokenizeChars(inputText, file, startLineNo)
            ctx = FRB.ParseContext(inputText, file = file, startLineNo = startLineNo)

            error = None
            result = None
            try:
                result = parser.parse(tokens, ctx = ctx)
            except FRB.SyntaxErr as e:
                error = e

            if (error is not None):
                expectedErr = FRB.SyntaxErr(ctx, expected, process = "parsing")
                self.compareSyntaxErr(error, expectedErr)
            else:
                self.compareParseTreeShape(result, expected)

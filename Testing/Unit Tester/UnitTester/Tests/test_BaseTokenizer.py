import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def buildTokenizer(setup = True):
    """
    A small tokenizer built entirely off the public API (addKeyword/addASCIIRangeTransitions --
    BaseTokenizer's internal DFA isn't exposed to Python, unlike the pure-Python original's
    self._dfa, so every state/transition here has to come from those two methods).

    Recognizes:
    * "+"/"++" (maximal-munch: "++" wins over two separate "+" tokens)
    * "\\n" as its own NEWLINE token (so line/char tracking can be exercised without relying on
      any implicit newline handling)
    * " " as its own SPACE token
    * runs of ASCII digits as a single NUM token (0-9, 1+ digits)
    """

    tokens = {
        "+": "PLUS",
        "++": "PLUSPLUS",
        "\n": "NEWLINE",
        " ": "SPACE",
        "0": "NUM",
    }

    tokenizer = FRB.BaseTokenizer(tokens, setup = setup)

    tokenizer.addKeyword("+")
    tokenizer.addKeyword("++")
    tokenizer.addKeyword("\n")
    tokenizer.addKeyword(" ")

    # digits: addKeyword("0") registers state "0" as accepting (tokens["0"] == "NUM") and wires
    # startState -[[0]]-> "0"; the range calls below add every other digit into/out of that same
    # state (self-loop) so multi-digit runs ("123") stay in the one accepting state.
    tokenizer.addKeyword("0")
    tokenizer.addASCIIRangeTransitions(tokenizer.startStateId, "0", "9", "0")
    tokenizer.addASCIIRangeTransitions("0", "0", "9", "0")

    return tokenizer


class BaseTokenizerTest(BaseUnitTest):

    def setUp(self):
        super().setUp()
        self._tokenizer = buildTokenizer()

    # =============== __init__ =======================

    def test_init_tokensPreserved(self):
        tokens = {"+": "PLUS"}
        tokenizer = FRB.BaseTokenizer(tokens)
        self.compareDict(tokenizer.tokens, tokens)

    def test_init_startStateIdIsEmptyStr(self):
        tokenizer = FRB.BaseTokenizer({})
        self.assertEqual(tokenizer.startStateId, "")

    def test_init_setupFalse_noAutoSetup(self):
        # setup=False skips the trivial base setup() (clear + register the start state) -- but
        # addKeyword() itself always calls addStartState() first, so building purely off
        # addKeyword still works fine without it
        tokenizer = FRB.BaseTokenizer({"+": "PLUS"}, setup = False)
        tokenizer.addKeyword("+")
        result = tokenizer.simplifiedMaximalMunch("+")
        self.assertEqual(len(result), 1)
        self.compareToken(result[0], FRB.Token("PLUS", "+", 1, 1))

    # ================================================
    # ================= addKeyword ====================

    def test_addKeyword_returnsAccumulatedStateId(self):
        tokenizer = FRB.BaseTokenizer({})
        self.assertEqual(tokenizer.addKeyword("abc"), "abc")

    # ================================================
    # ============ simplifiedMaximalMunch =============

    def test_singleToken(self):
        result = self._tokenizer.simplifiedMaximalMunch("+")
        self.assertEqual(len(result), 1)
        self.compareToken(result[0], FRB.Token("PLUS", "+", 1, 1))

    def test_maximalMunch_prefersLongerKeyword(self):
        result = self._tokenizer.simplifiedMaximalMunch("++")
        self.assertEqual(len(result), 1)
        self.compareToken(result[0], FRB.Token("PLUSPLUS", "++", 1, 1))

    def test_multipleTokens_separatedBySpace(self):
        result = self._tokenizer.simplifiedMaximalMunch("+ +")
        self.assertEqual(len(result), 3)
        self.compareToken(result[0], FRB.Token("PLUS", "+", 1, 1))
        self.compareToken(result[1], FRB.Token("SPACE", " ", 1, 2))
        self.compareToken(result[2], FRB.Token("PLUS", "+", 1, 3))

    def test_multiDigitNum_singleToken(self):
        result = self._tokenizer.simplifiedMaximalMunch("0123")
        self.assertEqual(len(result), 1)
        self.compareToken(result[0], FRB.Token("NUM", "0123", 1, 1))

    def test_multiLine_lineAndCharNoTracked(self):
        result = self._tokenizer.simplifiedMaximalMunch("+\n+")
        self.assertEqual(len(result), 3)
        self.compareToken(result[0], FRB.Token("PLUS", "+", 1, 1))
        self.compareToken(result[1], FRB.Token("NEWLINE", "\n", 1, 2))
        self.compareToken(result[2], FRB.Token("PLUS", "+", 2, 1))

    def test_emptySrc_raisesSyntaxErr(self):
        # Not a graceful []: the loop's 'isAccept' starts hardcoded False (never actually queried
        # from the start state), so an empty source can never "accept" either -- verified this
        # matches the pure-Python BaseTokenizerOld's own behavior for empty input, an existing
        # quirk being preserved here, not a gap introduced by the port.
        with self.assertRaises(FRB.SyntaxErr):
            self._tokenizer.simplifiedMaximalMunch("")

    def test_acceptsParseContext_directly(self):
        ctx = FRB.ParseContext("+ 0", startLineNo = 8)
        result = self._tokenizer.simplifiedMaximalMunch(ctx)
        self.assertEqual(len(result), 3)
        self.compareToken(result[0], FRB.Token("PLUS", "+", 8, 1))
        self.compareToken(result[1], FRB.Token("SPACE", " ", 8, 2))
        self.compareToken(result[2], FRB.Token("NUM", "0", 8, 3))

    def test_acceptsParseContext_startLineNoRespected(self):
        ctx = FRB.ParseContext("+\n+", startLineNo = 5)
        result = self._tokenizer.simplifiedMaximalMunch(ctx)
        self.compareToken(result[2], FRB.Token("PLUS", "+", 6, 1))

    # ================================================
    # =============== SyntaxErr =======================

    def test_unrecognizedChar_raisesSyntaxErr(self):
        with self.assertRaises(FRB.SyntaxErr) as ctxManager:
            self._tokenizer.simplifiedMaximalMunch("@")

        err = ctxManager.exception
        self.assertIsNone(err.token.type)
        self.assertEqual(err.token.val, "@")
        self.assertEqual(err.token.lineNo, 1)
        self.assertEqual(err.token.charNo, 1)

    def test_unrecognizedChar_afterValidTokens_correctPosition(self):
        with self.assertRaises(FRB.SyntaxErr) as ctxManager:
            self._tokenizer.simplifiedMaximalMunch("+ @")

        err = ctxManager.exception
        self.assertEqual(err.token.val, "@")
        self.assertEqual(err.token.charNo, 3)

    def test_syntaxErr_ctxPropagated(self):
        ctx = FRB.ParseContext("@", file = "test.ini", startLineNo = 3)

        with self.assertRaises(FRB.SyntaxErr) as ctxManager:
            self._tokenizer.simplifiedMaximalMunch(ctx)

        err = ctxManager.exception
        self.assertEqual(err.ctx.file, "test.ini")
        self.assertEqual(err.ctx.startLineNo, 3)
        self.assertEqual(err.token.lineNo, 3)

    def test_incompleteMultiCharKeyword_atEOF_raisesSyntaxErr(self):
        # a lone "+" is itself already a valid, accepting token -- use a grammar-invalid
        # trailing char instead to force the final post-loop flush down the error path
        with self.assertRaises(FRB.SyntaxErr):
            self._tokenizer.simplifiedMaximalMunch("+@")

    # ================================================
    # ================ clear/reset =====================

    def test_clear_removesStates(self):
        tokenizer = FRB.BaseTokenizer({"+": "PLUS"})
        tokenizer.addKeyword("+")
        tokenizer.clear()

        # after clear(), even the start state is gone -- calling simplifiedMaximalMunch() on a
        # fully-empty DFA raises a raw IndexError ("Value not found."), not SyntaxErr, since it
        # tries to resolve the (unset) current state back to a value. Verified this matches the
        # pure-Python BaseTokenizerOld's own behavior for the exact same sequence (clear(), then
        # tokenize) -- an existing quirk being preserved here, not a gap introduced by the port.
        with self.assertRaises(IndexError):
            tokenizer.simplifiedMaximalMunch("+")

    def test_reset_doesNotRemoveStates(self):
        tokenizer = FRB.BaseTokenizer({"+": "PLUS"})
        tokenizer.addKeyword("+")
        tokenizer.reset()

        result = tokenizer.simplifiedMaximalMunch("+")
        self.assertEqual(len(result), 1)

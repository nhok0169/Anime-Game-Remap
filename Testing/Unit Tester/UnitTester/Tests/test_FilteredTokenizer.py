import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def buildTokenizer(setup = True):
    """
    Unlike BaseTokenizer (which needs addKeyword() called manually after construction),
    FilteredTokenizer's own setup() adds every id in keywordTokenIds automatically -- so
    "+"/" "/"\\n" are all wired up for free here, with " "/"\\n" excluded from the default
    (includeFiltered=False) result.
    """

    tokens = {
        "+": "PLUS",
        " ": "SPACE",
        "\n": "NEWLINE",
    }
    keywordTokenIds = {"+", " ", "\n"}
    filteredTokenIds = {" ", "\n"}

    return FRB.FilteredTokenizer(tokens, keywordTokenIds, filteredTokenIds, setup = setup)


class FilteredTokenizerTest(BaseUnitTest):

    def setUp(self):
        super().setUp()
        self._tokenizer = buildTokenizer()

    # =============== __init__ =======================

    def test_init_tokensPreserved(self):
        tokens = {"+": "PLUS"}
        tokenizer = FRB.FilteredTokenizer(tokens, {"+"}, set())
        self.compareDict(tokenizer.tokens, tokens)

    def test_init_keywordTokenIdsPreserved(self):
        self.compareSet(self._tokenizer.keywordTokenIds, {"+", " ", "\n"})

    def test_init_filteredTokenIdsPreserved(self):
        self.compareSet(self._tokenizer.filteredTokenIds, {" ", "\n"})

    def test_init_isBaseTokenizer(self):
        self.assertIsInstance(self._tokenizer, FRB.BaseTokenizer)

    def test_init_setupAutomaticallyAddsKeywords(self):
        # no manual addKeyword() call needed -- setup() (called from __init__ since setup=True)
        # already added every id in keywordTokenIds
        result = self._tokenizer.simplifiedMaximalMunch("+")
        self.assertEqual(len(result), 1)
        self.compareToken(result[0], FRB.Token("PLUS", "+", 1, 1))

    def test_init_setupFalse_noKeywordsAdded(self):
        tokenizer = buildTokenizer(setup = False)
        with self.assertRaises(IndexError):
            tokenizer.simplifiedMaximalMunch("+")

    # ================================================
    # ============ simplifiedMaximalMunch =============

    def test_defaultExcludesFilteredTokens(self):
        result = self._tokenizer.simplifiedMaximalMunch("+ +")
        self.assertEqual(len(result), 2)
        self.compareToken(result[0], FRB.Token("PLUS", "+", 1, 1))
        self.compareToken(result[1], FRB.Token("PLUS", "+", 1, 3))

    def test_includeFilteredTrue_filteredTokensIncluded(self):
        result = self._tokenizer.simplifiedMaximalMunch("+ +", includeFiltered = True)
        self.assertEqual(len(result), 3)
        self.compareToken(result[0], FRB.Token("PLUS", "+", 1, 1))
        self.compareToken(result[1], FRB.Token("SPACE", " ", 1, 2))
        self.compareToken(result[2], FRB.Token("PLUS", "+", 1, 3))

    def test_filteredToken_stillConsumedNotJustDropped(self):
        # a filtered token is recognized (and resets the DFA/advances position) even though it's
        # excluded from the result -- not merely a post-hoc filter over an unfiltered result
        result = self._tokenizer.simplifiedMaximalMunch("\n+")
        self.assertEqual(len(result), 1)
        self.compareToken(result[0], FRB.Token("PLUS", "+", 2, 1))

    def test_unrecognizedChar_raisesSyntaxErr(self):
        with self.assertRaises(FRB.SyntaxErr) as ctxManager:
            self._tokenizer.simplifiedMaximalMunch("@")

        err = ctxManager.exception
        self.assertEqual(err.token.val, "@")

    def test_acceptsParseContext_directly(self):
        ctx = FRB.ParseContext("+", startLineNo = 5)
        result = self._tokenizer.simplifiedMaximalMunch(ctx)
        self.compareToken(result[0], FRB.Token("PLUS", "+", 5, 1))

    # ================================================
    # ========= inherited BaseTokenizer API ============

    def test_addKeyword_extendsBeyondKeywordTokenIds(self):
        # extra states can still be added the same way as BaseTokenizer's own public API, on top
        # of whatever setup() already wired up from keywordTokenIds -- 'tokens' has to be given
        # in full up front though (it's a read-only property: every access copies the underlying
        # map out to a fresh Python dict, so 'tokenizer.tokens["0"] = "NUM"' would only mutate a
        # throwaway copy), matching how the real tokenizer subclasses actually build their token
        # maps (IfPredTokenizer/SympyTokenizer both finish building the whole dict before ever
        # calling the base constructor, never mutate .tokens afterward)
        tokens = {"+": "PLUS", " ": "SPACE", "\n": "NEWLINE", "0": "NUM"}
        tokenizer = FRB.FilteredTokenizer(tokens, {"+", " ", "\n"}, {" ", "\n"})

        tokenizer.addKeyword("0")
        tokenizer.addASCIIRangeTransitions(tokenizer.startStateId, "0", "9", "0")
        tokenizer.addASCIIRangeTransitions("0", "0", "9", "0")

        result = tokenizer.simplifiedMaximalMunch("12")
        self.assertEqual(len(result), 1)
        self.compareToken(result[0], FRB.Token("NUM", "12", 1, 1))

    def test_clear_removesStates(self):
        self._tokenizer.clear()
        with self.assertRaises(IndexError):
            self._tokenizer.simplifiedMaximalMunch("+")

    def test_reset_doesNotRemoveStates(self):
        self._tokenizer.reset()
        result = self._tokenizer.simplifiedMaximalMunch("+")
        self.assertEqual(len(result), 1)

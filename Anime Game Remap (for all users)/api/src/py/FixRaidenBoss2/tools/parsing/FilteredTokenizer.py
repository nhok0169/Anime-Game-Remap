 ##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
from typing import List, Tuple, Union
##### EndExtImports

##### LocalImports
from ...tools.parsing.BaseTokenizer import BaseTokenizer
from ...tools.parsing.ParseContext import ParseContext
from ...tools.parsing.Token import Token
from typing import Dict, Set
##### EndLocalImports


##### Script
class FilteredTokenizer(BaseTokenizer):
    """
    This class inherits from :class:`BaseTokenizer`

    A tokenizer that that still accepts all tokens, but does not include certain tokens into the tokenized result


    Parameters
    ----------
    tokens: Dict[:class:`str`, :class:`str`]
        The tokens used for tokenization :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens

    keywordTokenIds: Set[:class:`str`]
        The ids of the accepting states in the `DFA`_ such that their corresponding tokens are simply keyword names

    filteredTokenIds: Set[:class:`str`]
        The ids of the accepting states in the `DFA`_ to not include their corresponding tokens into the tokenized result

    setup: :class:`bool`
        Whether to initialize all the setup for the tokenizer automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    Attributes
    ----------
    keywordTokenIds: Set[:class:`str`]
        The ids of the accepting states in the `DFA`_ such that their corresponding tokens are simply keyword names

    filteredTokenIds: Set[:class:`str`]
        The ids of the accepting states in the `DFA`_ to not include their corresponding tokens into the tokenized result
    """


    def __init__(self, tokens: Dict[str, str], keywordTokenIds: Set[str], filteredTokenIds: Set[str], setup: bool = True):
        self.keywordTokenIds = keywordTokenIds
        self.filteredTokenIds = filteredTokenIds
        super().__init__(tokens, setup = setup)

    def setup(self):
        self.clear()
        for keywordId in self.keywordTokenIds:
            self.addKeyword(keywordId)

        self._addStates()
        self._addTransitions()

    def _acceptToken(self, token: str, stateId: str, isAccept: bool, result, lineNo: int, charNo: int, includeFiltered: bool = False):
        if (isAccept and stateId in self.tokens):
            if (includeFiltered or stateId not in self.filteredTokenIds):
                result.append(Token(self.tokens[stateId], token, lineNo, charNo))

            self._dfa.reset()
            return True
        return False

    def simplifiedMaximalMunch(self, src: Union[str, ParseContext], includeFiltered: bool = False) -> Tuple[List[Tuple[str, str]], bool]:
        """
        Tokenizes the source text into tokens using the `Simplified Maximal Munch`_ algorithm

        Parameters
        ----------
        src: :class:`str`
            The source text to be tokenized

        includeFiltered: :class:`bool`
            Whether to include the filtered tokens specified at :attr:`filteredTokenIds` into the result :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        Tuple[List[:class:`str`], :class:`bool`]
            Includes:

            #. The tokens to the source text
            #. Whether the source text has been completly tokenized without errors
        """

        if (isinstance(src, ParseContext)):
            ctx = src
            src = "\n".join(ctx.lines)
        else:
            src = "\n".join(src.splitlines())
            ctx = ParseContext(src)

        result = []
        self._dfa.reset()
        stateId = self._dfa.currentStateId
        isAccept = False
        srcLen = len(src)
        currentToken = ""
        i = 0

        lineNo = ctx.startLineNo
        charNo = 1
        tokenLineNo = lineNo
        tokenCharNo = charNo

        while (i < srcLen):
            letter = src[i]
            stateId, isAccept, transitionTaken = self._dfa.transition(letter)

            if (transitionTaken):
                i += 1
                currentToken += letter

                if (letter == "\n"):
                    lineNo += 1
                    charNo = 1
                else:
                    charNo += 1
                continue

            accepted = self._acceptToken(currentToken, stateId, isAccept, result, tokenLineNo, tokenCharNo, includeFiltered = includeFiltered)
            if (not accepted):
                self._raiseSyntaxErr(ctx, f"{currentToken}{letter}", lineNo, charNo + 1)
            
            currentToken = ""
            tokenLineNo = lineNo
            tokenCharNo = charNo
            
        accepted = self._acceptToken(currentToken, stateId, isAccept, result, tokenLineNo, tokenCharNo, includeFiltered = includeFiltered)
        if (not accepted):
            self._raiseSyntaxErr(ctx, currentToken, lineNo, charNo)

        return result
##### EndScript
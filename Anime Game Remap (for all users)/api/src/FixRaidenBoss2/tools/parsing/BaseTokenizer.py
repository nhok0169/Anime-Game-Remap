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
from typing import Dict, List, Tuple, Union
##### EndExtImports

##### LocalImports
from ..DFA import DFA
from .Token import Token
from .ParseContext import ParseContext
from ...exceptions.SyntaxErr import SyntaxErr
##### EndLocalImports


##### Script
class BaseTokenizer():
    """
    The base class used for tokenizing text

    Parameters
    ----------
    tokens: Dict[:class:`str`, :class:`str`]
        The tokens used for tokenization :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens

    Attributes
    ----------
    _dfa: :class:`DFA`_
        The internal `DFA`_

    tokens: Dict[:class:`str`, :class:`str`]
        The tokens used for tokenization :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens

    startStateId: :class:`str`
        The id of the starting state of the `DFA`_

    setup: :class:`bool`
        Whether to initialize all the setup for the tokenizer automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``
    """

    def __init__(self, tokens: Dict[str, str], setup: bool = True):
        self._dfa = DFA()
        self.tokens = tokens
        self.startStateId = ""

        if (setup):
            self.setup()

    def clear(self):
        """
        Clears the `DFA`_ of the tokenizer
        """
        
        self._dfa.clear()

    def reset(self):
        """
        Resets the state of the `DFA`_ for the tokenzier
        """

        self._dfa.reset()

    def _addStates(self):
        """
        Adds all the necessary states into `DFA`_ of the tokenizer
        """

        self.addStartState()

    def addStartState(self) -> str:
        """
        Adds the start state representing an empty string

        Returns
        -------
        :class:`str`
            The id of the start state
        """

        self._dfa.addState(self.startStateId, isStart = True)
        return self.startStateId

    def _addTransitions(self):
        """
        Adds all the necessary state transitions to the `DFA`_ of the tokenizer
        """

        pass

    def addKeyword(self, keyword: str) -> str:
        """
        Adds a keyword into the `DFA`_ of the tokenizer

        Parameters
        ----------
        keyword: :class:`str`
            The keyword to add

        Returns
        -------
        :class:`str`
            The id of the accepting nodein the `DFA`_
        """

        self.addStartState()
        prevStateId = self._dfa.startId
        stateId = ""
        keywordLen = len(keyword)

        for i in range(keywordLen):
            letter = keyword[i]
            stateId += letter

            isAccept = None
            if (i == keywordLen - 1):
                isAccept = True

            self._dfa.addState(stateId, isAccept = isAccept)
            self._dfa.addTransition(prevStateId, letter, stateId)
            prevStateId = stateId

        return stateId
    
    def addASCIIRangeTransitions(self, srcId: str, startChar: str, endChar: str, destId: str):
        """
        Adds a group of transitions from one state to another according to a range of `ASCII`_ characters

        Parameters
        ----------
        srcId: :class:`str`
            The id of the source state for the transition

        startChar: :class:`str`
            The starting character within the ASCII range to add a transition for

        endChar: :class:`str`
            The ending character within the ASCII range to add a transition for

        destId: :class:`str`
            The id of the destionation state for the transition
        """
        
        for i in range(ord(startChar), ord(endChar) + 1):
            self._dfa.addTransition(srcId, chr(i), destId)

    def setup(self):
        """
        Performs any necessary setup to the tokenizer
        """

        self.clear()
        self._addStates()
        self._addTransitions()

    def _acceptToken(self, token: str, stateId: str, isAccept: bool, result: List[Tuple[str, str]], lineNo: int, charNo: int) -> bool:
        if (isAccept and stateId in self.tokens):
            result.append(Token(self.tokens[stateId], token, lineNo, charNo))
            self._dfa.reset()
            return True
        return False
    
    def _raiseSyntaxErr(self, ctx: ParseContext, currentToken: str, lineNo: int, charNo: int):
        token = Token(None, currentToken, lineNo, charNo - len(currentToken))
        raise SyntaxErr(ctx, token, process = "tokenization")

    def simplifiedMaximalMunch(self, src: Union[str, ParseContext]) -> List[Token]:
        """
        Tokenizes the source text into tokens using the `Simplified Maximal Munch`_ algorithm

        Parameters
        ----------
        src: Union[:class:`str`, :class:`ParseContext`]
            The source text to be tokenized

        Raises
        ------
        :class:`SyntaxErr`
            The provided source text cannot be correctly tokenized

        Returns
        -------
        List[:class:`Token`]
            The list of tokens to the source text
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

            accepted = self._acceptToken(currentToken, stateId, isAccept, result, tokenLineNo, tokenCharNo)
            if (not accepted):
                self._raiseSyntaxErr(ctx, f"{currentToken}{letter}", lineNo, charNo + 1)
            
            currentToken = ""
            tokenLineNo = lineNo
            tokenCharNo = charNo
            
        accepted = self._acceptToken(currentToken, stateId, isAccept, result, tokenLineNo, tokenCharNo)
        if (not accepted):
            self._raiseSyntaxErr(ctx, currentToken, lineNo, charNo)

        return result
##### EndScript
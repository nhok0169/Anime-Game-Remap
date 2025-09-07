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
from typing import Dict, List, Tuple
##### EndExtImports

##### LocalImports
from .DFA import DFA
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

    _tokens: Dict[:class:`str`, :class:`str`]
        The tokens used for tokenization :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens
    """

    def __init__(self, tokens: Dict[str, str]):
        self._dfa = DFA()
        self._tokens = tokens
        self._startStateId = ""

    @property
    def startStateId(self):
        """
        The id of the starting state of the `DFA`_

        :getter: Returns the id
        :type: :class:`str`
        """

        return self._startStateId

    def clear(self):
        """
        Clears the `DFA`_ of the tokenizer
        """
        
        self._dfa.clear()

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

        self._dfa.addState(self._startStateId, isStart = True)
        return self._startStateId

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

    def _acceptToken(self, token: str, stateId: str, isAccept: bool, result: List[Tuple[str, str]]) -> bool:
        if (isAccept and stateId in self._tokens):
            result.append((self._tokens[stateId], token))
            self._dfa.reset()
            return True
        return False

    def simplifiedMaximalMunch(self, src: str) -> Tuple[List[Tuple[str, str]], bool]:
        """
        Tokenizes the source text into tokens using the `Simplified Maximal Munch`_ algorithm

        Parameters
        ----------
        src: :class:`str`
            The source text to be tokenized

        Returns
        -------
        Tuple[List[Tuple[:class:`str`, :class:`str`]], :class:`bool`]
            Includes:

            #. The list of tokens to the source text, where each tuple contains the token type and the parsed token
            #. Whether the source text has been completly tokenized without errors
        """

        result = []
        self._dfa.reset()
        stateId = self._dfa.currentStateId
        isAccept = False
        srcLen = len(src)
        currentToken = ""
        i = 0

        while (i < srcLen):
            letter = src[i]
            stateId, isAccept, transitionTaken = self._dfa.transition(letter)

            if (transitionTaken):
                i += 1
                currentToken += letter
                continue

            accepted = self._acceptToken(currentToken, stateId, isAccept, result)
            currentToken = ""

            if (not accepted):
                return (result, False)
            
        accepted = self._acceptToken(currentToken, stateId, isAccept, result)
        return (result, accepted)
##### EndScript
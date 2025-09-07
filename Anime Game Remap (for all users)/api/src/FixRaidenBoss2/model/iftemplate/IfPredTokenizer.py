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
from typing import List, Tuple
##### EndExtImports

##### LocalImports
from ...tools.DictTools import DictTools
from ...tools.BaseTokenizer import BaseTokenizer
##### EndLocalImports


##### Script
class IfPredTokenizer(BaseTokenizer):
    """
    This class inherits from :class:`BaseTokenizer`

    The tokenizer used for conditional predicates within a .ini file

    eg.

    .. code-block:: ini
        :linenos:
        :emphasize-lines: 1,3

        if pred1
            ...
        else if pred2
            ...
        endif
    """


    def __init__(self):
        self._keywordTokens = {
            "null": "NULL",
            "+": "PLUS",
            "-": "MINUS",
            "*": "STAR",
            "/": "SLASH",
            "%": "PCT",
            "(": "LPAREN",
            ")": "RPAREN",
            "==": "EQ",
            "!=": "NE",
            "<": "LT",
            ">": "GT",
            "<=": "LE",
            ">=": "GE",
            "&&": "AND",
            "||": "OR",
            "!": "NOT",
            " ": "SPACE",
            "\t": "TAB"
        }

        self._whitespaces = {" ", "\t"}

        tokens = {
            "doubleQuoteStringClose": "STRING",
            "singleQuoteStringClose": "STRING",
            "id": "ID",
            "integer": "INT",
            "float": "FLOAT"
        }

        DictTools.update(tokens, self._keywordTokens)
        super().__init__(tokens)

    def _addStates(self):
        super()._addStates()

        # id variables
        self._dfa.addState("idStart")
        self._dfa.addState("id", isAccept = True)

        # strings
        self._dfa.addState("doubleQuoteStringOpen")
        self._dfa.addState("singleQuoteStringOpen")
        self._dfa.addState("doubleQuotestringContent")
        self._dfa.addState("singleQuoteStringContent")
        self._dfa.addState("doubleQuoteStringClose", isAccept = True)
        self._dfa.addState("singleQuoteStringClose", isAccept = True)

        # numbers
        self._dfa.addState("integer", isAccept = True)
        self._dfa.addState("decimalPoint")
        self._dfa.addState("float", isAccept = True)

    def _addTransitions(self):
        startId = ""

        # id variables
        varStartId = "idStart"
        varAcceptId = "id"
        varSymbols = ["!", "%", "&", "(", ")", "*", "+", ",", "-", ".", "/", ":", "<", "=", ">", "?", "@", "[", "\\", "]", "^", "_", "`", "{", "}", "|", "~"]

        self._dfa.addTransition(startId, "$", varStartId)
        self.addASCIIRangeTransitions(varStartId, "a", "z", varAcceptId)
        self.addASCIIRangeTransitions(varStartId, "A", "Z", varAcceptId)
        self.addASCIIRangeTransitions(varStartId, "0", "9", varAcceptId)
        self._dfa.addTransitions(varStartId, varSymbols, varAcceptId)

        self.addASCIIRangeTransitions(varAcceptId, "a", "z", varAcceptId)
        self.addASCIIRangeTransitions(varAcceptId, "A", "Z", varAcceptId)
        self.addASCIIRangeTransitions(varAcceptId, "0", "9", varAcceptId)
        self._dfa.addTransitions(varAcceptId, varSymbols, varAcceptId)

        # strings
        self._dfa.addTransition(startId, '"', "doubleQuoteStringOpen")
        self._dfa.addTransition("doubleQuoteStringOpen", '"', "doubleQuoteStringClose")
        self._dfa.addTransition("doubleQuoteStringOpen", lambda keyword: keyword != '"', "doubleQuotestringContent")
        self._dfa.addTransition("doubleQuotestringContent", lambda keyword: keyword != '"', "doubleQuotestringContent")
        self._dfa.addTransition("doubleQuotestringContent", '"', "doubleQuoteStringClose")

        self._dfa.addTransition(startId, "'", "singleQuoteStringOpen")
        self._dfa.addTransition("singleQuoteStringOpen", "'", "singleQuoteStringClose")
        self._dfa.addTransition("singleQuoteStringOpen", lambda keyword: keyword != "'", "singleQuoteStringContent")
        self._dfa.addTransition("singleQuoteStringContent", lambda keyword: keyword != "'", "singleQuoteStringContent")
        self._dfa.addTransition("singleQuoteStringContent", "'", "singleQuoteStringClose")

        # numbers
        self.addASCIIRangeTransitions("-", "0", "9", "integer")
        self.addASCIIRangeTransitions(startId, "0", "9", "integer")
        self.addASCIIRangeTransitions("integer", "0", "9", "integer")
        self._dfa.addTransition("integer", ".", "decimalPoint")
        self.addASCIIRangeTransitions("decimalPoint", "0", "9", "float")
        self.addASCIIRangeTransitions("float", "0", "9", "float")

    def setup(self):
        self.clear()
        for keyword in self._keywordTokens:
            self.addKeyword(keyword)

        self._addStates()
        self._addTransitions()

    def _acceptToken(self, token: str, stateId: str, isAccept: bool, result, whitespaces: bool = False):
        if (isAccept and stateId in self._tokens):
            if (whitespaces or stateId not in self._whitespaces):
                result.append((self._tokens[stateId], token))

            self._dfa.reset()
            return True
        return False

    def simplifiedMaximalMunch(self, src: str, whitespaces: bool = False) -> Tuple[List[Tuple[str, str]], bool]:
        """
        Tokenizes the source text into tokens using the `Simplified Maximal Munch`_ algorithm

        Parameters
        ----------
        src: :class:`str`
            The source text to be tokenized

        whitespaces: :class:`bool`
            Whether to include whitespace tokens in the result :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        Tuple[List[:class:`str`], :class:`bool`]
            Includes:

            #. The tokens to the source text
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

            accepted = self._acceptToken(currentToken, stateId, isAccept, result, whitespaces = whitespaces)
            currentToken = ""

            if (not accepted):
                return (result, False)
            
        accepted = self._acceptToken(currentToken, stateId, isAccept, result, whitespaces = whitespaces)
        return (result, accepted)
##### EndScript
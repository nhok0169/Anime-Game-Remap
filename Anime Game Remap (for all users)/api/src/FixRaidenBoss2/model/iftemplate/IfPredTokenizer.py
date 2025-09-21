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
from ...tools.DictTools import DictTools
from ...tools.parsing.BaseTokenizer import BaseTokenizer
from ...tools.parsing.ParseContext import ParseContext
from ...tools.parsing.Token import Token
from ...exceptions.SyntaxErr import SyntaxErr
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

    Parameters
    ----------
    setup: :class:`bool`
        Whether to initialize all the setup for the tokenizer automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``
    """


    def __init__(self, setup: bool = True):
        self._keywordTokens = {
            "null": "NULL",
            "+": "PLUS",
            "-": "MINUS",
            "*": "STAR",
            "/": "SLASH",
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
            "id": "ID",
            "integer": "INT",
            "float": "FLOAT"
        }

        DictTools.update(tokens, self._keywordTokens)
        super().__init__(tokens, setup = setup)

    def _addStates(self):
        super()._addStates()

        # id variables
        self._dfa.addState("idStart")
        self._dfa.addState("id", isAccept = True)

        # numbers
        self._dfa.addState("integer", isAccept = True)
        self._dfa.addState("decimalPoint")
        self._dfa.addState("float", isAccept = True)

    def _addTransitions(self):
        startId = self.startStateId

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

    def _acceptToken(self, token: str, stateId: str, isAccept: bool, result, lineNo: int, charNo: int, whitespaces: bool = False):
        if (isAccept and stateId in self.tokens):
            if (whitespaces or stateId not in self._whitespaces):
                result.append(Token(self.tokens[stateId], token, lineNo, charNo))

            self._dfa.reset()
            return True
        return False

    def simplifiedMaximalMunch(self, src: Union[str, ParseContext], whitespaces: bool = False) -> Tuple[List[Tuple[str, str]], bool]:
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

            accepted = self._acceptToken(currentToken, stateId, isAccept, result, tokenLineNo, tokenCharNo, whitespaces = whitespaces)
            if (not accepted):
                self._raiseSyntaxErr(ctx, f"{currentToken}{letter}", lineNo, charNo + 1)
            
            currentToken = ""
            tokenLineNo = lineNo
            tokenCharNo = charNo
            
        accepted = self._acceptToken(currentToken, stateId, isAccept, result, tokenLineNo, tokenCharNo, whitespaces = whitespaces)
        if (not accepted):
            self._raiseSyntaxErr(ctx, currentToken, lineNo, charNo)

        return result
##### EndScript
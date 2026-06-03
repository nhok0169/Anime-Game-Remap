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

##### LocalImports
from ...tools.DictTools import DictTools
from ...tools.parsing.FilteredTokenizer import FilteredTokenizer
##### EndLocalImports


##### Script
class SympyTokenizer(FilteredTokenizer):
    """
    This class inherits from :class:`FilteredTokenizer`

    The tokenizer used for a subset of the string representation of a `sympy logic query`_

    eg.

    .. code-block:: 
        :linenos:
        
        ~(($y$ | Ne($x$, $y$)) & (($x$ >= $y$) | ($x$ <= $y$)) & Eq($x$, $y$*$z$ - $y$ + $z$/3))

    Parameters
    ----------
    setup: :class:`bool`
        Whether to initialize all the setup for the tokenizer automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``
    """


    def __init__(self, setup: bool = True):
        keywordTokens = {
            "+": "PLUS",
            "-": "MINUS",
            "*": "STAR",
            "/": "SLASH",
            "(": "LPAREN",
            ")": "RPAREN",
            ",": "COMMA",

            "==": "EQ",
            "!=": "NE",
            "<": "LT",
            ">": "GT",
            "<=": "LE",
            ">=": "GE",
            "&": "AND",
            "|": "OR",
            "~": "NOT",

            "Eq": "EQFUNC",
            "Ne": "NEFUNC",
            "Lt": "LTFUNC",
            "Gt": "GTFUNC",
            "Le": "LEFUNC",
            "Ge": "GEFUNC",
            "And": "ANDFUNC",
            "Or": "ORFUNC",
            "Not": "NOTFUNC",

            "True": "TRUE",
            "False": "FALSE",

            " ": "SPACE",
            "\t": "TAB"
        }

        whitespaces = {" ", "\t"}

        tokens = {
            "id": "ID",
            "integer": "INT",
            "float": "FLOAT"
        }

        DictTools.update(tokens, keywordTokens)
        keywordTokenIds = set(keywordTokens.keys())

        super().__init__(tokens, keywordTokenIds, whitespaces, setup = setup)

    def _addStates(self):
        super()._addStates()

        # id variables
        self._dfa.addState("idStart")
        self._dfa.addState("idParsing")
        self._dfa.addState("id", isAccept = True)

        # numbers
        self._dfa.addState("integer", isAccept = True)
        self._dfa.addState("decimalPoint")
        self._dfa.addState("float", isAccept = True)

    def _addTransitions(self):
        startId = self.startStateId

        # id variables
        varStartId = "idStart"
        varPendingId = "idParsing"
        varAcceptId = "id"
        varSymbols = ["%", ",", ".", ":", "?", "@", "[", "\\", "]", "^", "_", "`", "{", "}", "~"]

        self._dfa.addTransition(startId, "$", varStartId)
        self.addASCIIRangeTransitions(varStartId, "a", "z", varPendingId)
        self.addASCIIRangeTransitions(varStartId, "A", "Z", varPendingId)
        self.addASCIIRangeTransitions(varStartId, "0", "9", varPendingId)
        self._dfa.addTransitions(varStartId, varSymbols, varPendingId)

        self.addASCIIRangeTransitions(varPendingId, "a", "z", varPendingId)
        self.addASCIIRangeTransitions(varPendingId, "A", "Z", varPendingId)
        self.addASCIIRangeTransitions(varPendingId, "0", "9", varPendingId)
        self._dfa.addTransitions(varPendingId, varSymbols, varPendingId)

        self._dfa.addTransition(varPendingId, "$", varAcceptId)

        # numbers
        self.addASCIIRangeTransitions("-", "0", "9", "integer")
        self.addASCIIRangeTransitions(startId, "0", "9", "integer")
        self.addASCIIRangeTransitions("integer", "0", "9", "integer") 
        self._dfa.addTransition("integer", ".", "decimalPoint")
        self.addASCIIRangeTransitions("decimalPoint", "0", "9", "float")
        self.addASCIIRangeTransitions("float", "0", "9", "float")
##### EndScript
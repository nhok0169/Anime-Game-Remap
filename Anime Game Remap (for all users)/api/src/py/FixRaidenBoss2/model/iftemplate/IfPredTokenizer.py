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
class IfPredTokenizer(FilteredTokenizer):
    """
    This class inherits from :class:`FilteredTokenizer`

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
        keywordTokens = {
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
        varSymbols = ["%", ",", ".", ":", "?", "@", "[", "\\", "]", "^", "_", "`", "{", "}", "~"]

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
##### EndScript
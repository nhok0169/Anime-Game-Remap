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
from ...tools.parsing.BaseSLR1Parser import BaseSLR1Parser
##### EndLocalImports


##### Script
class IfPredParser(BaseSLR1Parser):
    """
    This class inherits from :class:`BaseSLR1Parser`

    The context-free parser used for conditional predicates within a .ini file

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
    -----------
    startSymbol: :class:`str`
        The starting non-terminal symbol

    startToken: :class:`str`
        The name of the starting token for an input string :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``STARTTOKEN``

    endToken: :class:`str`
        The name of the ending token for an input string :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``ENDTOKEN``

    nullToken: :class:`str`
        The name for the empty token :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``EPSILON``

    setup: :class:`bool`
        Whether to initialize all the setup for the parser automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``
    """

    def __init__(self, startToken = "STARTTOKEN", endToken = "ENDTOKEN", nullToken = "EPSILON", setup = True):
        startSymbol = "pred_prime"
        productions = {"start":             (startSymbol, [startToken, "pred", endToken]),
                       "start empty":       (startSymbol, [startToken, endToken]),
                       "pred reduce":       ("pred", ["ntest"]),
                       "and":               ("pred", ["pred", "AND", "ntest"]),
                       "or":                ("pred", ["pred", "OR", "ntest"]),
                       "ntest reduce":      ("ntest", ["test"]),
                       "not":               ("ntest", ["NOT", "test"]),
                       "test reduce":       ("test", ["keyexpr"]),
                       "eq":                ("test", ["keyexpr", "EQ", "keyexpr"]),
                       "ne":                ("test", ["keyexpr", "NE", "keyexpr"]),
                       "gt":                ("test", ["keyexpr", "GT", "keyexpr"]),
                       "ge":                ("test", ["keyexpr", "GE", "keyexpr"]),
                       "lt":                ("test", ["keyexpr", "LT", "keyexpr"]),
                       "le":                ("test", ["keyexpr", "LE", "keyexpr"]),
                       "keyexpr reduce":    ("keyexpr", ["addexpr"]),
                       "null":              ("keyexpr", ["NULL"]),
                       "addexpr reduce":    ("addexpr", ["multexpr"]),
                       "add":               ("addexpr", ["addexpr", "PLUS", "multexpr"]),
                       "subtract":          ("addexpr", ["addexpr", "MINUS", "multexpr"]),
                       "multexpr reduce":    ("multexpr", ["term"]),
                       "multiply":          ("multexpr", ["multexpr", "STAR", "term"]),
                       "divide":            ("multexpr", ["multexpr", "SLASH", "term"]),
                       "variable":          ("term", ["ID"]),
                       "int":               ("term", ["INT"]),
                       "float":             ("term", ["FLOAT"]),
                       "bracket loop":      ("term", ["LPAREN", "pred", "RPAREN"])}

        super().__init__(productions, startSymbol, startToken, endToken, nullToken, setup)
##### EndScript
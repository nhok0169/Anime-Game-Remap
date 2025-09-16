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
                       "gt":                ("test", ["expr", "GT", "expr"]),
                       "ge":                ("test", ["expr", "GE", "expr"]),
                       "lt":                ("test", ["expr", "LT", "expr"]),
                       "le":                ("test", ["expr", "LE", "expr"]),
                       "keyexpr reduce":    ("keyexpr", ["expr"]),
                       "null":              ("keyexpr", ["NULL"]),
                       "expr reduce":       ("expr", ["term"]),
                       "add":               ("expr", ["expr", "PLUS", "term"]),
                       "subtract":          ("expr", ["expr", "MINUS", "term"]),
                       "multiply":          ("expr", ["expr", "STAR", "term"]),
                       "divide":            ("expr", ["expr", "SLASH", "term"]),
                       "modulus":           ("expr", ["expr", "PCT", "term"]),
                       "variable":          ("term", ["ID"]),
                       "int":               ("term", ["INT"]),
                       "float":             ("term", ["FLOAT"]),
                       "string":            ("term", ["STRING"]),
                       "bracket loop":      ("term", ["LPAREN", "pred", "RPAREN"])}

        super().__init__(productions, startSymbol, startToken, endToken, nullToken, setup)
##### EndScript
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

##### CppLocalImports
from ..core import IfPredTokenizer
from ..core import SympyTokenizer
##### EndCppLocalImports

##### LocalImports
from ..tools.enums.DeferredEnum import DeferredEnum
from ..model.iftemplate.IfPredParser import IfPredParser
from ..model.iftemplate.SympyParser import SympyParser
##### EndLocalImports


##### Script
class GlobalCompilerParts(DeferredEnum):
    """
    This class inherits from :class:`DeferredEnum`

    Global modules used by the sofware to help parse different languages

    Attributes
    ----------
    IfPredTokenizer: :class:`IfPredTokenizer`
        The tokenizer to tokenize the conditional predicates in a .ini file

    IfPredParser: :class:`IfPredParser`
        The context-free parser to parse the syntax structure of the conditional predicates in a .ini file

    SympyTokenizer: :class:`SympyTokenizer`
        The tokenizer to tokenize the logical queries used by `sympy`_

    SympyParser: :class:`SympyParser`
        The context-free parser to parse the syntax stucture of the logical queries used by `sympy`_
    """

    IfPredTokenizer = (lambda: IfPredTokenizer(), )
    IfPredParser = (lambda: IfPredParser(), )
    SympyTokenizer = (lambda: SympyTokenizer(), )
    SympyParser = (lambda: SympyParser(), )
##### EndScript

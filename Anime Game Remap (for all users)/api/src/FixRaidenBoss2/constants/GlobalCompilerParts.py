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
from ..tools.enums.DeferredEnum import DeferredEnum
from ..model.iftemplate.IfPredTokenizer import IfPredTokenizer
from ..model.iftemplate.IfPredParser import IfPredParser
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
    """

    IfPredTokenizer = (lambda: IfPredTokenizer(), )
    IfPredParser = (lambda: IfPredParser(), )
##### EndScript
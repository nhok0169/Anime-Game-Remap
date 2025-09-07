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
import re
##### EndExtImports

##### LocalImports
from .IfTemplatePart import IfTemplatePart
from ...constants.IfPredPartType import IfPredPartType
##### EndLocalImports


##### Script
class IfPredPart(IfTemplatePart):
    """
    This class inherits from :class:`IfTemplatePart`

    Class for defining the predicate part of an :class:`IfTemplate`

    .. note::
        see :class:`IfTemplate` for more details

    Parameters
    ----------
    src: :class:`str`
        The original string within the :class:`IfTemplate`

    type: :class:`IfPredPartType`
        The type of predicate encountered

    Attributes
    ----------
    src: :class:`str`
        The original string within the :class:`IfTemplate` 

    type: :class:`IfPredPartType`
        The type of predicate encountered
    """

    def __init__(self, src: str, type: IfPredPartType):
        self.src = src
        self.type = type

    def getTestStr(self) -> str:
        if (not self.type == IfPredPartType.Elif):
            return re.sub(self.type.value, "", self.src, flags=re.IGNORECASE, count = 1)
        
        cleanedSrc = self.src.lstrip().lower()
        if (cleanedSrc.startswith(IfPredPartType.Else)):
            result = re.sub(IfPredPartType.Else.value, "", self.src, flags=re.IGNORECASE, count = 1)
            return re.sub(IfPredPartType.If.value, "", result, flags=re.IGNORECASE, count = 1)
        
        return re.sub(IfPredPartType.Elif.value, "", result, flags=re.IGNORECASE, count = 1)

    def toStr(self) -> str:
        return f"{self.src}"
##### EndScript
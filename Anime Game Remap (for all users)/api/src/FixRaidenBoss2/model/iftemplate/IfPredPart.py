##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits


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
    pred: :class:`str`
        The predicate string within the :class:`IfTemplate`

    type: :class:`IfPredPartType`
        The type of predicate encountered

    Attributes
    ----------
    pred: :class:`str`
        The predicate string within the :class:`IfTemplate`

    type: :class:`IfPredPartType`
        The type of predicate encountered
    """

    def __init__(self, pred: str, type: IfPredPartType):
        self.pred = pred
        self.type = type


    def toStr(self) -> str:
        return f"{self.pred}"
##### EndScript
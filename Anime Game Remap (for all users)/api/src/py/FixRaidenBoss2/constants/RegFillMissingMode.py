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
from enum import Enum
##### EndExtImports


##### Script
class RegFillMissingMode(Enum):
    """
    Different modes for handling :class:`IfContentPart`s with missing registers
    """

    FillMissing = "fillMissing"
    """
    Finds all :class:`IfContentPart` missing the desired register and fills those parts with the register
    """

    TopdownCover = "topdownCover"
    """
    Determines whether the caller/callee graph (:class:`IniSectionGraph`) contains a :class:`IfContentPart` missing the desired register,
    then adds the register to the roots of the graph to cover for the missing registers
    """

    BottomCover = "bottomCover"
    """
    Like :attr:`TopdownCover`, but the register is added at the **bottom** of each root (a fresh last :class:`IfContentPart`),
    so it runs after everything the root sets up -- the mode for a draw call. :attr:`FillMissing` fills the FIRST content part
    of a section that lacks the register, which is the wrong end once something (a :class:`ResGroupCollect` splicing a
    collected register into an ``if`` block) has split the section
    """
##### EndScript
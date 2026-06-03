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
from typing import Optional
##### EndExtImports


##### Script
class Token():
    """
    Class for a token when parsing some language

    Parameters
    ----------
    type: Optional[:class:`str`]
        The name for the type of token, if available

    val: :class:`str`
        The value of the token

    lineNo: :class:`int`
        The line number the token belongs to

    charNo: :class:`int`
        The character number the token belongs to within some line
    """

    def __init__(self, type: Optional[str], val: str, lineNo: int, charNo: int):
        self.type = type
        self.val = val
        self.lineNo = lineNo
        self.charNo = charNo
##### EndScript
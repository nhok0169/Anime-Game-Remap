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
from typing import Optional, Union, List
##### EndExtImports


##### Script
class ParseContext():
    """
    Context for parsing some text

    Parameters
    ----------
    src: Union[:class:`str`, List[:class:`str`]]
        The source text to parse :raw-html:`<br />` :raw-html:`<br />`

        If this argument is a list, then assumes that the lines of the source text is given

    file: Optional[:class:`str`]
        The file path to the source text :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    startLineNo: :class:`int`
        The starting line of the source text :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``1``

    Attributes
    ----------
    lines: List[:class:`str`]
        The lines of the source text

    file: Optional[:class:`str`]
        The file path to the source text

    startLineNo: :class:`int`
        The starting line of the source text
    """

    def __init__(self, src: Union[str, List[str]], file: Optional[str] = None, startLineNo: int = 1):
        self.lines = src.splitlines() if (isinstance(src, str)) else src
        self.file = file
        self.startLineNo = startLineNo

    def getEndLineNo(self) -> int:
        """
        Retrieves the line number after the last line

        Returns
        -------
        :class:`int`
            The ending line number after the last line
        """

        return self.startLineNo + len(self.lines)
##### EndScript
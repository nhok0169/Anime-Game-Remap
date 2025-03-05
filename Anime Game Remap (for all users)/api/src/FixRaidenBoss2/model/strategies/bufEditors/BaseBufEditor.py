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
from typing import Optional, Union
##### EndExtImports

##### LocalImports
from ...files.BufFile import BufFile
##### EndLocalImports


##### Script
class BaseBufEditor():
    """
    Base class to edit some .buf file
    """

    def fix(self, bufFile: BufFile, fixedBufFile: str) -> Union[Optional[str], bytearray]:
        """
        Edits the binary file

        Parameters
        ----------
        bufFile: :class:`BufFile`
            The binary .buf file to be modified

        fixedBufFile: :class:`str`
            The name of the fixed .buf file

        Returns
        -------
        Union[Optional[:class:`str`], :class:`bytearray`]
            If the argument ``fixedBufFile`` is ``None``, then will return an array of bytes for the fixed .buf file :raw-html:`<br />` :raw-html:`<br />`
            Otherwise will return the filename to the fixed .buf file if the provided .buf file got corrected
        """
        pass
##### EndScript

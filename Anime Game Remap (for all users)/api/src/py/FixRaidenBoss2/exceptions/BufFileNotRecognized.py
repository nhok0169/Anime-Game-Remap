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
import os
##### EndExtImports

##### LocalImports
from .FileException import FileException
##### EndLocalImports


##### Script
class BufFileNotRecognized(FileException):
    """
    This Class inherits from :class:`FileException`

    Exception when a Blend.buf file cannot be read

    Parameters
    ----------
    filePath: :class:`str`
        The file path to the .buf file

    fileType: :class:`str`
        The name for the type of .buf file :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``Buffer``
    """
    def __init__(self, filePath: str, fileType: str = "Buffer"):
        super().__init__(f"{fileType} file format not recognized for {os.path.basename(filePath)}", path = os.path.dirname(filePath))
##### EndScript
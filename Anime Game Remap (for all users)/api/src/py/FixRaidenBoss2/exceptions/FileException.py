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

##### LocalImports
from .Error import Error
from ..constants.FilePathConsts import FilePathConsts
##### EndLocalImports


##### Script
class FileException(Error):
    """
    This Class inherits from :class:`Error`

    Exceptions relating to files

    Parameters
    ----------
    message: :class:`str`
        The error message to print out

    path: Optional[:class:`str`]
        The path where the error for the file occured. If this value is ``None``, then the path
        will be the current directory where this module is loaded :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, message: str, path: Optional[str] = None):
        path = FilePathConsts.getPath(path)

        if (path != FilePathConsts.DefaultPath):
            message += f" at {path}"

        super().__init__(message)
##### EndScript
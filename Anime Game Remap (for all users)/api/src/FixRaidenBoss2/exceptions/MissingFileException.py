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


##### ExtImports
from typing import Optional
##### EndExtImports

##### LocalImports
from .FileException import FileException
from ..constants.FileTypes import FileTypes
from ..constants.FilePathConsts import FilePathConsts
##### EndLocalImports


##### Script
class MissingFileException(FileException):
    """
    This Class inherits from :class:`FileException`

    Exception when a certain type of file is missing from a folder

    Parameters
    ----------
    fileType: :class:`str`
        The type of file searching in the folder :raw-html:`<br />` :raw-html:`<br />`

        **Default**: "file"

    path: :class:`str`
        The path to the folder that is being searched. If this value is ``None``, then the path
        will be the current directory where this module is loaded :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    fileType: :class:`str`
        The type of file searching in the folder
    """
    def __init__(self, fileType: str = FileTypes.Default.value, path: Optional[str] = None):
        path = FilePathConsts.getPath(path)
        message = f"Unable to find {fileType}. Ensure it is in the folder"
        self.fileType = fileType
        super().__init__(message, path = path)
##### EndScript
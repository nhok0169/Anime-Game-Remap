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
from typing import Optional, List
##### EndExtImports

##### LocalImports
from .FileException import FileException
from ..constants.FileTypes import FileTypes
from ..constants.FilePathConsts import FilePathConsts
##### EndLocalImports


##### Script
class DuplicateFileException(FileException):
    """
    This Class inherits from :class:`FileException`

    Exception when there are multiple files of the same type in a folder

    Parameters
    ----------
    files: List[:class:`str`]
        The files that triggered the exception

    fileType: :class:`str`
        The name for the type of files :raw-html:`<br />` :raw-html:`<br />`

        **Default**: "file"

    path: Optional[:class:`str`]
        The path to the folder where the files are located If this value is ``None``, then the path
        will be the current directory where this module is loaded :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    files: List[:class:`str`]
        The files that triggered the exception

    fileType: :class:`str`
        The name for the type of files

        **Default**: ``None``
    """

    def __init__(self, files: List[str], fileType: str = FileTypes.Default.value, path: Optional[str] = None):
        path = FilePathConsts.getPath(path)
        self.files = files
        self.fileType = fileType
        message = f"Ensure only one {fileType} exists"
        super().__init__(message, path = path)
##### EndScript

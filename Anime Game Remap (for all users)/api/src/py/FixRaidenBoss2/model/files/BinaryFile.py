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
from typing import Union
##### EndExtImports

##### LocalImports
from ...tools.files.FileService import FileService
from .File import File
##### EndLocalImports


##### Script
class BinaryFile(File):
    """
    This class inherits from :class:`File`

    A class to handle binary files

    Parameters
    ----------
    src: Union[:class:`str`, :class:`bytes`]
        The source file or bytes for the .buf file

    Attributes
    ----------
    src: Union[:class:`str`, :class:`bytes`]
        The source file or bytes for the .buf file
    """

    def __init__(self, src: Union[str, bytes]):
        self.src = src
        self._data = b""

    @property
    def data(self):
        """
        The bytes read in from the source

        :getter: Returns the bytes that were read
        :type: :class:`bytes`
        """

        return self._data

    def read(self) -> bytes:
        self._data = FileService.readBinary(self.src)
        return self._data
##### EndScript
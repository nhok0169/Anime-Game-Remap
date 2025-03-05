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

##### LocalImports
from .BufType import BufType
##### EndLocalImports


##### Script
class BufDataType(BufType):
    """
    This class inherits from :class:`BufType`

    The type definition for an elementary data type within a .buf file

    Parameters
    ----------
    name: :class:`str`
        The name of the element

    size: :class:`int`
        The byte size for the data type

    isBigEndian: :class:`bool`
        Whether the type is in big endian mode :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    Attributes
    ----------
    size: :class:`int`
        The byte size for the data type
    """

    def __init__(self, name: str, size: int, isBigEndian: bool = False):
        super().__init__(name)
        self.size = size
        self._endianSymbol = ""
        self.isBigEndian = isBigEndian

    @property
    def isBigEndian(self) -> bool:
        """
        The `endianness`_ for the data type

        :getter: Retrieves whether the data type is in big endian mode
        :setter: Sets the new `endianness`_ for the data type
        :type: :class:`bool`
        """

        return self._isBigEndian
    
    @isBigEndian.setter
    def isBigEndian(self, newIsBigEndian: bool):
        self._isBigEndian = newIsBigEndian
        self._endianSymbol = ">" if (self._isBigEndian) else "<"
##### EndScript
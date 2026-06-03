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
import struct
##### EndExtImports

##### LocalImports
from ...constants.BufTypeNames import BufDataTypeNames
from ...constants.ByteSize import ByteSize
from .BufDataType import BufDataType
##### EndLocalImports


##### Script
class BufBaseInt(BufDataType):
    """
    This class inherits from :class:`BufDataType`

    The type definition for some generic integer type within a .buf file

    Parameters
    ----------
    name: :class:`str`
        The name of the element

    size: :class:`int`
        The byte size for the data type

    isBigEndian: :class:`bool`
        Whether the type is in big endian mode :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    isSigned: :class:`bool`
        Whether the type is signed :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``
    """

    def __init__(self, name: str, size: int, isBigEndian: bool = False, isSigned: bool = True):
        super().__init__(name, size, isBigEndian = isBigEndian)
        self._endianSymbolLong = "big" if (isBigEndian) else "little"
        self._isSigned = isSigned

    @property
    def isSigned(self) -> bool:
        """
        Whether the data type is signed

        :getter: Retrieves whether the data type is signed
        :type: :class:`bool`
        """

        return self._isSigned

    def decode(self, src: bytes) -> int:
        """
        Decode the raw bytes to an integer

        .. warning::
            Please make sure the number of bytes passed into 'src' matches the size of the type

        Parameters
        ----------
        src: :class:`bytes`
            The raw bytes to decode

        Returns 
        -------
        :class:`int`
            The decoded signed integer
        """

        return int.from_bytes(src, byteorder = self._endianSymbolLong, signed = self._isSigned)

    def encode(self, src: int) -> bytes:
        """
        Encodes an integer back to raw bytes

        .. warning::
            Please make sure 'src' is within the acceptable range for the type

        Parameters
        ----------
        src: :class:`int`
            The integer to encode

        Returns 
        -------
        :class:`bytes`
            The encoded raw bytes
        """

        return (src).to_bytes(self.size, byteorder = self._endianSymbolLong, signed = self._isSigned)
    

class BufSignedInt(BufBaseInt):
    """
    This class inherits from :class:`BufBaseInt`

    The type definition for some signed integer type within a .buf file

    Parameters
    ----------
    name: :class:`str`
        The name of the element :raw-html:`<br />` :raw-html:`<br />`

        **Default**: :attr:`BufDataTypeNames.Int32`.value

    size: :class:`int`
        The byte size for the data type :raw-html:`<br />` :raw-html:`<br />`

        **Default**: :attr:`ByteSize.Int32`.value

    isBigEndian: :class:`bool`
        Whether the type is in big endian mode :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``
    """

    def __init__(self, name: str = BufDataTypeNames.Int32.value, size: int = ByteSize.Int32.value, isBigEndian: bool  = False):
        super().__init__(name, size, isBigEndian = isBigEndian)


class BufUnSignedInt(BufBaseInt):
    """
    This class inherits from :class:`BufBaseInt`

    The type definition for some signed integer type within a .buf file

    Parameters
    ----------
    name: :class:`str`
        The name of the element :raw-html:`<br />` :raw-html:`<br />`

        **Default**: :attr:`BufDataTypeNames.UInt32`.value

    size: :class:`int`
        The byte size for the data type :raw-html:`<br />` :raw-html:`<br />`

        **Default**: :attr:`ByteSize.UInt32`.value

    isBigEndian: :class:`bool`
        Whether the type is in big endian mode :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``
    """

    def __init__(self, name: str = BufDataTypeNames.UInt32.value, size: int = ByteSize.UInt32.value, isBigEndian: bool = False):
        super().__init__(name, size, isBigEndian = isBigEndian, isSigned = False)
##### EndScript
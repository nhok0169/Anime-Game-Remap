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
from typing import Any
##### EndExtImports

##### LocalImports
from ...constants.BufTypeNames import BufDataTypeNames
from ...constants.ByteSize import ByteSize
from .BufDataType import BufDataType
##### EndLocalImports


##### Script
class BufBaseFloat(BufDataType):
    """
    This class inherits from :class:`BufDataType`

    The type definition for a generic `floating point`_ number within a .buf file

    Parameters
    ----------
    name: :class:`str`
        The name of the element

    size: :class:`int`
        The byte size for the data type

    isBigEndian: :class:`bool`
        Whether the type is in big endian mode :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``
    """

    def __init__(self, name: str, size: int, isBigEndian: bool  = False):
        super().__init__(name, size, isBigEndian = isBigEndian)

    def decode(self, src: bytes) -> int:
        """
        Decode the raw bytes to a `floating point`_ number

        .. warning::
            Please make sure the number of bytes passed into 'src' matches the size of the type

        Parameters
        ----------
        src: :class:`bytes`
            The raw bytes to decode

        Returns 
        -------
        :class:`float`
            The decoded `floating point`_
        """

        return struct.unpack(f"{self._endianSymbol}f", src)[0]

    def encode(self, src: Any) -> bytes:
        """
        Encodes the `floating point`_ back to raw bytes

        .. warning::
            Please make sure 'src' is within the acceptable range for the type

        Parameters
        ----------
        src: :class:`float`
            The `floating point`_ to encode

        Returns 
        -------
        :class:`bytes`
            The encoded raw bytes
        """

        return struct.pack(f"{self._endianSymbol}f", src)
    

class BufFloat(BufBaseFloat):
    """
    This class inherits from :class:`BufBaseFloat`

    The type definition for a 32-bit `floating point`_ number within a .buf file

    Parameters
    ----------
    isBigEndian: :class:`bool`
        Whether the type is in big endian mode :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``
    """

    def __init__(self, isBigEndian: bool  = False):
        super().__init__(BufDataTypeNames.Float32.value, ByteSize.Float32.value, isBigEndian = isBigEndian)


class BufFloat16(BufBaseFloat):
    """
    This class inherits from :class:`BufBaseFloat`

    The type definition for a 16-bit `half precision floating point`_ number within a .buf file

    Parameters
    ----------
    isBigEndian: :class:`bool`
        Whether the type is in big endian mode :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``
    """

    def __init__(self, isBigEndian: bool  = False):
        super().__init__(BufDataTypeNames.Float16.value, ByteSize.Float16.value, isBigEndian = isBigEndian)

    def decode(self, src: bytes) -> int:
        return struct.unpack(f"{self._endianSymbol}e", src)[0]
    
    def encode(self, src: Any) -> bytes:
        return struct.pack(f"{self._endianSymbol}e", src)
##### EndScript
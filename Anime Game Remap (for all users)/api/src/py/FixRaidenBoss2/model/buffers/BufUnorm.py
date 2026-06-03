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
from .BufInt import BufBaseInt
##### EndLocalImports


##### Script
class BufUnorm(BufBaseInt):
    """
    This class inherits from :class:`BufBaseInt`

    The type definition for an `unsigned normalized integer`_ number within a .buf file

    Parameters
    ----------
    name: :class:`str`
        The name of the element

    size: :class:`int`
        The byte size for the data type

    isBigEndian: :class:`bool`
        Whether the type is in big endian mode
    """

    def __init__(self, name: str, size: int, isBigEndian: bool  = False):
        super().__init__(name, size, isBigEndian = isBigEndian, isSigned = False)
        self._maxValue = pow(2, size * 8) - 1

    def decode(self, src: bytes) -> float:
        """
        Decode the raw bytes to the `floating point`_ value for the `unsigned normalized integer`_
 
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

        numerator = super().decode(src)
        return numerator / self._maxValue

    def encode(self, src: float) -> bytes:
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

        result = int(src * self._maxValue)
        return super().encode(result)
##### EndScript
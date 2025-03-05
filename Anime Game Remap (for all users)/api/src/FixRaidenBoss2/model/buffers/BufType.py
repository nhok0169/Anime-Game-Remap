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
from typing import Any
##### EndExtImports


##### Script
class BufType():
    """
    The base class for a type in a .buf file

    Parameters
    ----------
    name: :class:`str`
        The name of the element

    Attributes
    ----------
    name: :class:`str`
        The name of the element
    """

    def __init__(self, name: str):
        self.name = name

    def decode(self, src: bytes) -> Any:
        """
        Decode the raw bytes to the required format for the type

        .. warning::
            Please make sure the number of bytes passed into 'src' matches the size of the type

        Parameters
        ----------
        src: :class:`bytes`
            The raw bytes to decode

        Returns 
        -------
        Any
            The decoded format for the type
        """

        pass

    def encode(self, src: Any) -> bytes:
        """
        Encodes the format of the type back to raw bytes

        .. warning::
            Please make sure 'src' is within the acceptable range for the type

        Parameters
        ----------
        src: Any
            The decoded format for the type

        Returns 
        -------
        :class:`bytes`
            The encoded raw bytes
        """

        pass
##### EndScript
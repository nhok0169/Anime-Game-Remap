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
from typing import List, Tuple, Optional, Callable, Union
##### EndExtImports

##### CppLocalImports
from ..core import CppIntTools
##### EndCppLocalImports


##### Script
class IntTools():
    """
    Tools for handling integers
    """

    @classmethod
    def toBase(cls, num: int, base: int) -> Tuple[List[int], bool]:
        """
        Converts a base 10 number to an arbitrary base number

        .. note::
            This function is a convenience for calling :meth:`CppIntTools.toBase`

        Parameters
        ----------
        num: :class:`int`
            The base 10 number to convert

        base: :class:`int`
            The base to convert to

        Raises
        ------
        :class:`TypeError`
            The base is smaller or equal to 1

        Returns
        -------
        Tuple[List[:class:`int`], :class:`bool`]
            Retrieves the following data in the tuple:

            #. The digits in the converted number
            #. Whether the number is negative
        """

        return CppIntTools.toBase(num, base)
    
    @classmethod
    def toStrBase(cls, num: int, base: int, getDigit: List[str], negativeChar: str) -> str:
        """
        Converts a base 10 number to an arbitrary base number, such that the characters in this arbitrary based number
        are all characters

        .. note::
            This function is a convenience for calling :meth:`CppIntTools.toStrBase`

        Parameters
        ----------
        num: :class:`int`
            The base 10 number to convert

        base: :class:`int`
            The base to convert to

        getDigit: List[:class:`str`]
            The string representations of each digit. Each element is the string representation
            of the digit at the particular index of the list.

        negativeChar: :class:`str`
            The character representation for the negative symbol

        Returns
        -------
        :class:`str`
            The converted string representation of the arbitrary base number

        Returns
        -------
        :class:`str`
            The converted string representation of the arbitrary base number
        """

        return CppIntTools.toStrBase(num, base, getDigit, negativeChar)
    
    @classmethod
    def toBase64(cls, num: int, getDigit: Optional[Union[str, List[str], Callable[[int], str]]] = None, negativeChar: str = "-") -> str:
        """
        Converts a base 10 number to a base 64 number

        .. note::
            This function is a convenience for calling :meth:`CppIntTools.toBase64`

        Parameters
        ----------
        num: :class:`int`
            The base 10 number to convert

        getDigit: List[:class:`str`]
            how to get the string representation of a digit. :raw-html:`<br />` :raw-html:`<br />`

            * If this argument is a list, each element is the string representation of the digit at the particular index of the string/list.
            * If this argument is ``None``, then will use the following string for each digit:

              ``ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_``

              This is the same digit representation as the `standard base 64`_ except that the 63rd digit (``/``) is replaced with the ``_`` character :raw-html:`<br />` :raw-html:`<br />`

              **Default**: ``None``

        negativeChar: :class:`str`
            The character representation for the negative symbol :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``"-"``

        Returns
        -------
        :class:`str`
            The converted string representation of the arbitrary base 64 number
        """

        return CppIntTools.toBase64(num, getDigit, negativeChar)
##### EndScript
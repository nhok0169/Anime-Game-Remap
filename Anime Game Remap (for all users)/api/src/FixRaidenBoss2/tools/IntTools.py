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


##### Script
class IntTools():
    """
    Tools for handling integers
    """

    @classmethod
    def toBase(cls, num: int, base: int) -> Tuple[List[int], bool]:
        """
        Converts a base 10 number to an arbitrary base number

        Parameters
        ----------
        num: :class:`int`
            The base 10 number to convert

        base: :class:`int`
            The base to convert to

        Raises
        ------
        :class:`ZeroDivisionError`
            The base is smaller or equal to 1

        Returns
        -------
        Tuple[List[:class:`int`], :class:`bool`]
            Retrieves the following data in the tuple:

            #. The digits in the converted number
            #. Whether the number is negative
        """

        if (base <= 1):
            raise ZeroDivisionError("Base must be greater than 1")

        if num == 0:
            return ([0], False)

        digits = []
        isNegative = num < 0

        if (isNegative):
            num *= -1

        while num:
            digits.append(int(num % base))
            num //= base

        return (digits[::-1], isNegative)
    
    @classmethod
    def toStrBase(cls, num: int, base: int, getDigit: Union[str, List[str], Callable[[int], str]], negativeChar: str) -> str:
        """
        Converts a base 10 number to an arbitrary base number, such that the characters in this arbitrary based number
        are all characters

        Parameters
        ----------
        num: :class:`int`
            The base 10 number to convert

        base: :class:`int`
            The base to convert to

        getDigit: Union[:class:`str`, List[:class:`str`], Callable[[:class:`int`], :class:`str`]]
            how to get the string representation of a digit. :raw-html:`<br />` :raw-html:`<br />`

            If this argument is a string or a list, each element is the string representation of the digit at the particular index of the string/list.

        negativeChar: :class:`str`
            The character representation for the negative symbol

        Returns
        -------
        :class:`str`
            The converted string representation of the arbitrary base number
        """

        digits, isNegative = cls.toBase(num, base)

        tempChars = getDigit
        if (not callable(getDigit)):
            getDigit = lambda digit: tempChars[digit]

        result = "".join(list(map(getDigit, digits)))
        if (isNegative):
            return negativeChar + result
        return result
    
    @classmethod
    def toBase64(cls, num: int, getDigit: Optional[Union[str, List[str], Callable[[int], str]]] = None, negativeChar: str = "-") -> str:
        """
        Converts a base 10 number to a base 64 number

        Parameters
        ----------
        num: :class:`int`
            The base 10 number to convert

        getDigit: Optional[Union[:class:`str`, List[:class:`str`], Callable[[:class:`int`], :class:`str`]]]
            how to get the string representation of a digit. :raw-html:`<br />` :raw-html:`<br />`

            * If this argument is a string or a list, each element is the string representation of the digit at the particular index of the string/list.
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

        if (getDigit is None):
            getDigit = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_"

        return cls.toStrBase(num, 64, getDigit, negativeChar)
##### EndScript
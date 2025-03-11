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
from typing import Tuple
##### EndExtImports

##### LocalImports
from ...constants.ColourConsts import ColourConsts
##### EndLocalImports


##### Script
class Colour():
    """
    Class to store data for a colour

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: hash(x)

            Retrieves the hash id for the colour based off :meth:`Colour.getId`

    :raw-html:`<br />`

    Parameters
    ----------
    red: :class:`int`
        The red channel for the colour :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``255``

    green: :class:`int`
        The green channel for the colour :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``255``

    blue: :class:`int`
        The blue channel for the colour :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``255``

    alpha: :class:`int`
        The transparency (alpha) channel for the colour with a range from 0-255. 0 = transparent, 255 = opaque :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``255``

    Attributes
    ----------
    red: :class:`int`
        The red channel for the colour

    green: :class:`int`
        The green channel for the colour

    blue: :class:`int`
        The blue channel for the colour

    alpha: :class:`int`
        The transparency (alpha) channel for the colour with a range from 0-255. 0 = transparent, 255 = opaque
    """

    def __init__(self, red: int = ColourConsts.MaxColourValue.value, green: int = ColourConsts.MaxColourValue.value, blue: int = ColourConsts.MaxColourValue.value, alpha: int = ColourConsts.MaxColourValue.value):
        self.red = self.boundColourChannel(red)
        self.green = self.boundColourChannel(green)
        self.blue = self.boundColourChannel(blue)
        self.alpha = self.boundColourChannel(alpha)

    @classmethod
    def boundColourChannel(self, val: int, min: int = ColourConsts.MinColourValue.value, max: int = ColourConsts.MaxColourValue.value):
        """
        Makes a colour channel to be in between the minimum and maximum value

        Parameters
        ----------
        val: :class:`int`
            The value of the channel

        min: :class:`int`
            The minimum bound for the colour channel :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``0``

        max: :class:`int`
            The maximum bound for the colour channel :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``255``
        """

        if (val > max):
            val = max
        elif (val < min):
            val = min
        return val
    
    @classmethod
    def boolToColourChannel(self, val: bool, min: int = ColourConsts.MinColourValue.value, max: int = ColourConsts.MaxColourValue.value) -> int:
        """
        Converts a boolean value to a value for a colour channel

        Parameters
        ----------
        val: :class:`bool`
            The boolean value to convert

        min: :class:`int`
            The minimum bound for the colour channel :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``0``

        max: :class:`int`
            The maximum bound for the colour channel :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``255``
        Returns
        -------
        :class:`int`
            The corresponding value for the colour channel based off the boolean
        """

        return max if (val) else min
    
    def __hash__(self) -> int:
        return hash(self.getId())
    
    def fromTuple(self, colourTuple: Tuple[int, int, int, int]):
        """
        Updates the colour based off 'colourTuple'

        Parameters
        ----------
        colourTuple: Tuple[:class:`int`, :class:`int`, :class:`int`, :class:`int`]
            The raw values for the colour in RGBA format
        """

        self.red = colourTuple[0]
        self.green = colourTuple[1]
        self.blue = colourTuple[2]
        self.alpha = colourTuple[3]
    
    def getTuple(self) -> Tuple[int, int, int, int]:
        """
        Retrieves the tuple representation of the colour in RGBA format

        Returns
        -------
        Tuple[:class:`int`, :class:`int`, :class:`int`, :class:`int`]
            The colour tuple containing the following colour channel values indicated by the order below: :raw-html:`<br />` :raw-html:`<br />`

            #. Red
            #. Green
            #. Blue
            #. Alpha            
        """

        return (self.red, self.green, self.blue, self.alpha)
    
    def getId(self) -> str:
        """
        Retrieves a unique id for the colour

        .. note::
            The id generated will not correspond to any id generated from :meth:`ColourRange.getId`

        Returns
        -------
        :class:`str`
            The id for the colour        
        """

        return f"{self.red}{self.green}{self.blue}{self.alpha}"

    def copy(self, colour, withAlpha: bool = True):
        """
        Copies the colour value from 'colour'

        Parameters
        ----------
        colour: :class:`Colour`
            The colour to copy from

        withAlpha: :class:`bool`
            Whether to also copy the alpha channel :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``
        """

        self.red = colour.red
        self.green = colour.green
        self.blue = colour.blue

        if (withAlpha):
            self.alpha = colour.alpha
    
    def match(self, colour):
        """
        Whether 'colour' matches this colour

        Parameters
        ----------
        colour: :class:`Colour`
            The colour to check

        Returns
        -------
        :class:`bool`
            Whether the colour matches this colour
        """

        return (colour.red == self.red and colour.green == self.green and
                colour.blue == self.blue and colour.alpha == self.alpha)
##### EndScript
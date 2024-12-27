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

##### LocalImports
from .Colour import Colour
##### EndLocalImports


##### Script
class ColourRange():
    """
    Class to store data for a colour

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: hash(x)

            Retrieves the hash id for the colour range based off :meth:`ColourRange.getId`

    :raw-html:`<br />`

    Parameters
    ----------
    min: :class:`Colour`
        The minimum range for the RGBA values

    max: :class:`Colour`
        The maximum range for the RGBA values
    """

    def __init__(self, min: Colour, max: Colour):
        self.min = min
        self.max = max

    def __hash__(self) -> int:
        return hash(self.getId())

    def getId(self) -> str:
        """
        Retrieves a unique id for the colour range

        .. note::
            The id generated will not correspond to any id generated from :meth:`Colour.getId`

        Returns
        -------
        :class:`str`
            The id for the colour range
        """

        return f"{self.min.getId()}{self.max.getId()}"
    
    def match(self, colour: Colour) -> bool:
        """
        Whether 'colour' is within the colour range

        Parameters
        ----------
        colour: :class:`Colour`
            The colour to check

        Returns
        -------
        :class:`bool`
            Whether the colour is within the colour range
        """
        
        return (self.min.red <= colour.red and colour.red <= self.max.red and
                self.min.green <= colour.green and colour.green <= self.max.green and
                self.min.blue <= colour.blue and colour.blue <= self.max.blue and
                self.min.alpha <= colour.alpha and colour.alpha <= self.max.alpha)
##### EndScript
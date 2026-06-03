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
from typing import Optional, Union, Set
##### EndExtImports

##### LocalImports
from ....textures.Colour import Colour
from ....textures.ColourRange import ColourRange
from .BasePixelTransform import BasePixelTransform
##### EndLocalImports


##### Script
class ColourReplace(BasePixelTransform):
    """
    This class inherits from :class:`BasePixelTransform`

    Replaces a coloured pixel

    Paramaters
    ----------
    replaceColour: :class:`Colour`
        The colour to fill in

    coloursToReplace: Optional[Set[Union[:class:`Colour`, :class:`ColourRange`]]]
        The colours to find to be replaced. If this value is ``None``, then will always replace the colour of the pixel :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    replaceAlpha: :class:`bool`
        Whether to also replace the alpha channel of the original colour :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    Attributes
    ----------
    replaceColour: :class:`Colour`
        The colour to fill in

    coloursToReplace: Optional[Set[Union[:class:`Colour`, :class:`ColourRange`]]]
        The colour to find to be replaced. If this value is ``None``, then will always replace the colour of the pixel

    replaceAlpha: :class:`bool`
        Whether to also replace the alpha channel of the original colour
    """

    def __init__(self, replaceColour: Colour, coloursToReplace: Optional[Set[Union[Colour, ColourRange]]] = None, replaceAlpha: bool = True):
        self.coloursToReplace = coloursToReplace
        self.replaceColour = replaceColour
        self.replaceAlpha = replaceAlpha

    def transform(self, pixel: Colour, x: int, y: int):
        if (self.coloursToReplace is None):
            pixel.copy(self.replaceColour, withAlpha = self.replaceAlpha)
            return
        
        for colour in self.coloursToReplace:
            if (colour.match(pixel)):
                pixel.copy(self.replaceColour, withAlpha = self.replaceAlpha)
                return
##### EndScript
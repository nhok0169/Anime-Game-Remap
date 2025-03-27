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
from typing import TYPE_CHECKING
##### EndExtImports

##### LocalImports
from ....textures.Colour import Colour
from .BaseTexFilter import BaseTexFilter

if (TYPE_CHECKING):
    from ....files.TextureFile import TextureFile
##### EndLocalImports


##### Script
class TransparencyAdjustFilter(BaseTexFilter):
    """
    This class inherits from :class:`BaseTexFilter`

    Adjust the trasparency (alpha channel) for an image

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: x(texFile)

            Calls :meth:`transform` for the filter, ``x``

    Parameters
    ----------
    alphaChange: :class:`int`
        How much to adjust the alpha channel of each pixel. Range from -255 to 255

        .. note::
            The alpha channel for an image is inclusively bounded from 0 to 255

    Attributes
    ----------
    alphaChange: :class:`int`
        How much to adjust the alpha channel of each pixel. Range from -255 to 255
    """

    def __init__(self, alphaChange: int):
        self.alphaChange = alphaChange

    def transform(self, texFile: "TextureFile"):
        alphaImg = texFile.img.getchannel('A')
        alphaImg = alphaImg.point(lambda alphaPixel: Colour.boundColourChannel(alphaPixel + self.alphaChange))
        texFile.img.putalpha(alphaImg)
##### EndScript
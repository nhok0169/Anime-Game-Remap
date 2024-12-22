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
from ....textures.Colour import Colour
from .BasePixelTransform import BasePixelTransform
##### EndLocalImports


##### Script
class Transparency(BasePixelTransform):
    """
    This class inherits from :class:`BasePixelTransform`

    Adjust the trasparency (alpha channel) for an image

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

    def transform(self, pixel: Colour, x: int, y: int):
        pixel.alpha = pixel.boundColourChannel(pixel.alpha + self.alphaChange)
##### EndScript
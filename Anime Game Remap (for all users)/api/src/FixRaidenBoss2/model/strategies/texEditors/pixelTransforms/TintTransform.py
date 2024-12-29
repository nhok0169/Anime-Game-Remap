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
class TintTransform(BasePixelTransform):
    """
    This class inherits from :class:`BasePixelTransform`

    Controls the tint of a texture file using the `Simple Image Temperature/Tint Adjust Algorithm`_

    Parameters
    ----------
    tint: :class:`int`
        The tint to set the image. Range from -100 to 100 :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``0``

    Attributes
    ----------
    tint: :class:`float`
        The tint to set the image. Range from -100 to 100
    """
    def __init__(self, tint: int = 0):
        self.tint = tint

    def transform(self, pixel: Colour, x: int, y: int):
        pixel.green = pixel.boundColourChannel(pixel.green + self.tint)
##### EndScript
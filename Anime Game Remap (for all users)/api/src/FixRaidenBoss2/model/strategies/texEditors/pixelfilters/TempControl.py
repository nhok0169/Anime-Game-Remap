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
from .....constants.ColourConsts import ColourConsts
from ....textures.Colour import Colour
from .BasePixelFilter import BasePixelFilter
##### EndLocalImports


##### Script
class TempControl(BasePixelFilter):
    """
    This class inherits from :class:`BasePixelFilter`

    Controls the temperature of a texture file

    Parameters
    ----------
    temp: :class:`int`
        The temperature to set the image. Range from -100 to 100 :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """
    def __init__(self, temp: int = 0):
        self.temp = temp

    def transform(self, pixel: Colour):
        pixel.red = pixel.boundColourChannel(pixel.red + self.temp)
        pixel.blue = pixel.boundColourChannel(pixel.blue - self.temp)
##### EndScript
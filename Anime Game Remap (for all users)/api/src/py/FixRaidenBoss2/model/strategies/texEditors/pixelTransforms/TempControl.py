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

##### LocalImports
from .....constants.ColourConsts import ColourConsts
from ....textures.Colour import Colour
from .BasePixelTransform import BasePixelTransform
##### EndLocalImports


##### Script
class TempControl(BasePixelTransform):
    """
    This class inherits from :class:`BasePixelTransform`

    Controls the temperature of a texture file using a modified version of the `Simple Image Temperature/Tint Adjust Algorithm`_ such that
    the colour channels increase/decrease linearly with respect to their corresponding pixel value and the user selected temperature

    Parameters
    ----------
    temp: :class:`float`
        The temperature to set the image. Range from -1 to 1 :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``0``

    Attributes
    ----------
    temp: :class:`float`
        The temperature to set the image. Range from -1 to 1

    _redFactor: :class:`float`
        The rate for how fast the red channel will change

    _blueFactor: :class:`float`
        The rate for how fast the blue channel will change
    """
    def __init__(self, temp: float = 0):
        self.temp = temp
        self._redFactor = ColourConsts.PaintTempIncRedFactor.value if (temp >= 0) else ColourConsts.PaintTempDecRedFactor.value
        self._blueFactor = ColourConsts.PaintTempIncBlueFactor.value if (temp >= 0) else ColourConsts.PaintTempDecBlueFactor.value

    def transform(self, pixel: Colour, x: int, y: int):
        pixel.red = pixel.boundColourChannel(round(pixel.red + self.temp * self._redFactor * pixel.red))
        pixel.blue = pixel.boundColourChannel(round(pixel.blue - self.temp * self._blueFactor * pixel.blue))
##### EndScript
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
from .BasePixelTransform import BasePixelTransform
##### EndLocalImports


##### Script
class TempControl(BasePixelTransform):
    """
    This class inherits from :class:`BasePixelTransform`

    Controls the temperature of a texture file using a modified version of the `Simple Image Temperature/Tint Adjust Algorithm`_ such that
    the rate the colour channel increases/decreases with respect to their corresponding pixel value is linear
    (So by integration, the colour channels change quadratically)

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

        .. note::
            Assume the rate the red channel changes is linear with respect to the pixel of the red channel
            (So by integration, the red channel changes quadratically)

    _blueFactor: :class:`float`
        The rate for how fast the blue channel will change

        .. note::
            Assume the rate the blue channel changes lienar with respect to the pixel of the blue channel
            (So by integration, the blue channel changes quadratically)
    """
    def __init__(self, temp: float = 0):
        self.temp = temp
        self._redFactor = ColourConsts.PaintTempIncRedFactor.value if (temp >= 0) else ColourConsts.PaintTempDecRedFactor.value
        self._blueFactor = ColourConsts.PaintTempIncBlueFactor.value if (temp >= 0) else ColourConsts.PaintTempDecBlueFactor.value

    def transform(self, pixel: Colour, x: int, y: int):
        pixel.red = pixel.boundColourChannel(pixel.red + self.temp * self._redFactor)
        pixel.blue = pixel.boundColourChannel(pixel.blue - self.temp * self._blueFactor)
##### EndScript
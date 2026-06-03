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
import math
##### EndExtImports

##### LocalImports
from .....constants.ColourConsts import ColourConsts
from ....textures.Colour import Colour
from .BasePixelTransform import BasePixelTransform
##### EndLocalImports


##### Script
class HighlightShadow(BasePixelTransform):
    """
    This class inherits from :class:`BasePixelTransform`

    A filter that approximates the adjustment of the shadow/hightlight of an image

    .. note::
        Reference: `Highlight Shadow Approximation Reference`_

    Parameters
    ----------
    highlight: :class:`float`
        The amount of highlight to apply to the pixel. Range from -1 to 1, and 0 = no change :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``0``

    shadow: :class:`float`
        The amount of shadow to apply to the pixel. Range from -1 to 1, and 0 = no change :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``0``

    Attributes
    ----------
    highlight: :class:`float`
        The amount of shadow to apply to the pixel. Range from -1 to 1, and 0 = no change

    shadow: :class:`float`
        The amount of shadow to apply to the pixel. Range from -1 to 1, and 0 = no change
    """
    def __init__(self, highlight: float = 0, shadow: float = 0):
        self.highlight = highlight
        self.shadow = shadow

    def transform(self, pixel: Colour, x: int, y: int):
        lumR = 0.299
        lumG = 0.587
        lumB = 0.114

        normRed = pixel.red / ColourConsts.MaxColourValue.value
        normGreen = pixel.green / ColourConsts.MaxColourValue.value
        normBlue = pixel.blue / ColourConsts.MaxColourValue.value

        # we have to find luminance of the pixel
        # here 0.0 <= source.r/source.g/source.b <= 1.0 
        # and 0.0 <= luminance <= 1.0

        luminance = math.sqrt(lumR * pow(normRed, 2.0) + lumG * pow(normGreen, 2.0) + lumB * pow(normBlue, 2.0))

        # here highlights and and shadows are our desired filter amounts
        # highlights/shadows should be >= -1.0 and <= +1.0
        #  highlights = shadows = 0.0 by default
        # you can change 0.05 and 8.0 according to your needs but okay for me

        h = self.highlight * 0.07 * ( pow(18.0, luminance) - 1.0 )
        s = self.shadow * 0.07 * ( pow(18.0, 1.0 - luminance) - 1.0 )

        pixel.red = Colour.boundColourChannel(round((normRed + h + s) * ColourConsts.MaxColourValue.value))
        pixel.green = Colour.boundColourChannel(round((normGreen + h + s) * ColourConsts.MaxColourValue.value))
        pixel.blue = Colour.boundColourChannel(round((normBlue + h + s) * ColourConsts.MaxColourValue.value))
##### EndScript
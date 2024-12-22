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
class CorrectGamma(BasePixelTransform):
    """
    This class inherits from :class:`BasePixelTransform`

    Performs a `Gamma Correction`_ on an individual pixel using the following simple `Gamma Correction Algorithm`_

    Parameters
    ----------
    gamma: :class:`float`
        The luminance parameter for how bright humans perceive the image. Based off the following Power Law Relationship`_:

        .. code-block::

            V_out = V_in ^ gamma

        :raw-html:`<br />`

        Where ``V_out`` is the perceived brightness by human eyes while ``V_in`` is the actual brightness of the image

        :raw-html:`<br />`

        .. note::
            higher gamma values make the image look brighter and less saturated while lower gamma values make the image look darker and more saturated.

    Attributes
    ----------
    gamma: :class:`float`
        The luminance parameter for how bright humans perceive the image.
    """
    def __init__(self, gamma: float):
        self.gamma = gamma

    @classmethod
    def correctGamma(cls, pixelValue: int, gamma: float) -> int:
        """
        The equation for the gamma correction done at every colour channel pixel

        Parameters
        ----------
        pixelValue: :class:`int`
            The value of the pixel for some colour channel

        gamma: :class:`float`
            The luminance parameter for how bright humans perceive the image.

        Returns
        -------
        :class:`int`
            The gamma corrected pixel values 
        """

        return round(Colour.boundColourChannel(pow(pixelValue / ColourConsts.MaxColourValue.value, (1 / gamma)) * ColourConsts.MaxColourValue.value))

    def transform(self, pixel: Colour, x: int, y: int):
        pixel.red = self.correctGamma(pixel.red, self.gamma)
        pixel.green = self.correctGamma(pixel.green, self.gamma)
        pixel.blue = self.correctGamma(pixel.blue, self.gamma)
##### EndScript
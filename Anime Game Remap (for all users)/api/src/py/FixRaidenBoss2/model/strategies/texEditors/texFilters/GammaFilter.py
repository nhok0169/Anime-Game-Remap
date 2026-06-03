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
from .....constants.ImgFormats import ImgFormats
from ..pixelTransforms.CorrectGamma import CorrectGamma
from .BaseTexFilter import BaseTexFilter

if (TYPE_CHECKING):
    from ....files.TextureFile import TextureFile
##### EndLocalImports


##### Script
class GammaFilter(BaseTexFilter):
    """
    This class inherits from :class:`BaseTexFilter`

    Performs a `Gamma Correction`_ on the texture file. See :class:`CorrectGamma` for more details

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: x(texFile)

            Calls :meth:`transform` for the filter, ``x``

    Parameters
    ----------
    gamma: :class:`float`
        The luminance parameter for how bright humans perceive the image.

    Attributes
    ----------
    gamma: :class:`float`
        The luminance parameter for how bright humans perceive the image.
    """

    def __init__(self, gamma: float):
        self.gamma = gamma

    def transform(self, texFile: "TextureFile"):
        alphaImg = texFile.img.getchannel('A')

        texFile.img = texFile.img.convert(ImgFormats.RGB.value)
        texFile.img = texFile.img.point(lambda pixel: CorrectGamma.correctGamma(pixel, self.gamma))

        texFile.img = texFile.img.convert(ImgFormats.RGBA.value)
        texFile.img.putalpha(alphaImg)
##### EndScript
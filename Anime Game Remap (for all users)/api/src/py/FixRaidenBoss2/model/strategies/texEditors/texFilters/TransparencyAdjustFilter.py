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
from typing import TYPE_CHECKING, Optional, Set, Union
##### EndExtImports

##### LocalImports
from ....textures.Colour import Colour
from ....textures.ColourRange import ColourRange
from .BaseTexFilter import BaseTexFilter
from .....constants.Packages import PackageModules
from .....constants.GlobalPackageManager import GlobalPackageManager
from .....constants.ImgFormats import ImgFormats

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

    coloursToFilter: Optional[Set[Union[:class:`Colour`, :class:`ColourRange`]]]
        The specific colours to have their transparency adjusted. If this value is ``None``, then will adjust the transparency for the entire image`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    alphaChange: :class:`int`
        How much to adjust the alpha channel of each pixel. Range from -255 to 255
    """

    def __init__(self, alphaChange: int, coloursToFilter: Optional[Set[Union[Colour, ColourRange]]] = None):
        self.alphaChange = alphaChange
        self.coloursToFilter = coloursToFilter

    def adjustTransparency(self, texFile: "TextureFile"):
        """
        Adjusts the transparency for the entire image

        Parameters
        ----------
        texFile: :class:`TextureFile`
            The texture to be editted
        """

        alphaImg = texFile.img.getchannel('A')
        alphaImg = alphaImg.point(lambda alphaPixel: Colour.boundColourChannel(alphaPixel + self.alphaChange))
        texFile.img.putalpha(alphaImg)


    def transform(self, texFile: "TextureFile"):
        if (self.coloursToFilter is None):
            self.adjustTransparency(texFile)
            return
        
        imgSize = texFile.img.size
        imgBox = (0, 0, imgSize[0], imgSize[1])
        ImageChops = GlobalPackageManager.get(PackageModules.PIL_ImageChops.value)
        
        redImg, greenImg, blueImg, alphaImg = texFile.img.split()

        newAlpha = alphaImg.copy()
        adjustedAlphaImg = alphaImg.point(lambda alphaPixel: Colour.boundColourChannel(alphaPixel + self.alphaChange))

        i = 0
        mask = None
        
        for colour in self.coloursToFilter:
            if (isinstance(colour, Colour)):
                redMatch = redImg.point(lambda redPixel: Colour.boolToColourChannel(redPixel == colour.red)).convert(ImgFormats.Bit.value)
                greenMatch = greenImg.point(lambda greenPixel: Colour.boolToColourChannel(greenPixel == colour.green)).convert(ImgFormats.Bit.value)
                blueMatch = blueImg.point(lambda bluePixel: Colour.boolToColourChannel(bluePixel == colour.blue)).convert(ImgFormats.Bit.value)
                alphaMatch = alphaImg.point(lambda alphaPixel: Colour.boolToColourChannel(alphaPixel == colour.alpha)).convert(ImgFormats.Bit.value)
            else:
                redMatch = redImg.point(lambda redPixel: Colour.boolToColourChannel(redPixel >= colour.min.red and redPixel <= colour.max.red)).convert(ImgFormats.Bit.value)
                greenMatch = greenImg.point(lambda greenPixel: Colour.boolToColourChannel(greenPixel >= colour.min.green and greenPixel <= colour.max.green)).convert(ImgFormats.Bit.value)
                blueMatch = blueImg.point(lambda bluePixel: Colour.boolToColourChannel(bluePixel >= colour.min.blue and bluePixel <= colour.max.blue)).convert(ImgFormats.Bit.value)
                alphaMatch = alphaImg.point(lambda alphaPixel: Colour.boolToColourChannel(alphaPixel >= colour.min.alpha and alphaPixel <= colour.max.alpha)).convert(ImgFormats.Bit.value)

            if (i > 0):
                mask = ImageChops.invert(mask)
                mask = ImageChops.logical_and(mask, redMatch)
            else:
                mask = redMatch

            mask = ImageChops.logical_and(mask, greenMatch)
            mask = ImageChops.logical_and(mask, blueMatch)
            mask = ImageChops.logical_and(mask, alphaMatch)

            newAlpha.paste(adjustedAlphaImg, box = imgBox, mask = mask)

        texFile.img.putalpha(newAlpha)
##### EndScript
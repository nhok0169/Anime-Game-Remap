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
from .....constants.ImgFormats import ImgFormats
from .....constants.Packages import PackageModules
from .....constants.GlobalPackageManager import GlobalPackageManager
from ....textures.Colour import Colour
from ....textures.ColourRange import ColourRange
from .BaseTexFilter import BaseTexFilter

if (TYPE_CHECKING):
    from ....files.TextureFile import TextureFile
##### EndLocalImports


##### Script
class ColourReplaceFilter(BaseTexFilter):
    """
    This class inherits from :class:`BaseTexFilter`

    Replaces specific colours in the image

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: x(texFile)

            Calls :meth:`transform` for the filter, ``x``

    Paramaters
    ----------
    replaceColour: :class:`Colour`
        The colour to fill in

    coloursToReplace: Optional[Set[Union[:class:`Colour`, :class:`ColourRange`]]]
        The colours to find to be replaced. If this value is ``None``, then will always replace the colour of the pixel :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    replaceAlpha: :class:`bool`
        Whether to also replace the alpha channel of the original colour :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    Attributes
    ----------
    replaceColour: :class:`Colour`
        The colour to fill in

    coloursToReplace: Optional[Set[Union[:class:`Colour`, :class:`ColourRange`]]]
        The colour to find to be replaced. If this value is ``None``, then will always replace the colour of the pixel

    replaceAlpha: :class:`bool`
        Whether to also replace the alpha channel of the original colour
    """

    def __init__(self, replaceColour: Colour, coloursToReplace: Optional[Set[Union[Colour, ColourRange]]] = None, replaceAlpha: bool = True):
        self.coloursToReplace = coloursToReplace
        self.replaceColour = replaceColour
        self.replaceAlpha = replaceAlpha

    def transform(self, texFile: "TextureFile"):
        imgSize = texFile.img.size
        imgBox = (0, 0, imgSize[0], imgSize[1])

        replaceAllColours = self.coloursToReplace is None

        # replace all colours
        if (replaceAllColours and self.replaceAlpha):
            texFile.img.paste(self.replaceColour.getTuple(), box = imgBox)
            return
        
        ImageChops = GlobalPackageManager.get(PackageModules.PIL_ImageChops.value)
        Image = GlobalPackageManager.get(PackageModules.PIL_Image.value)

        redImg, greenImg, blueImg, alphaImg = texFile.img.split()
        
        # replace all colours, but don't touch alpha
        if (replaceAllColours):
            redImg.paste(self.replaceColour.red, box = imgBox)
            greenImg.paste(self.replaceColour.green, box = imgBox)
            blueImg.paste(self.replaceColour.blue, box = imgBox)
            texFile.img = Image.merge(ImgFormats.RGBA.value, (redImg, greenImg, blueImg, alphaImg))
            return

        replaceColourTuple = self.replaceColour.getTuple()
        newRedImg = newGreenImg = newBlueImg = None

        if (not self.replaceAlpha):
            newRedImg = redImg.copy()
            newGreenImg = greenImg.copy()
            newBlueImg = blueImg.copy()

        i = 0
        mask = None
        
        for colour in self.coloursToReplace:
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

            mask = ImageChops.logical_and(mask, redMatch) if (i > 0) else redMatch
            mask = ImageChops.logical_and(mask, greenMatch)
            mask = ImageChops.logical_and(mask, blueMatch)
            mask = ImageChops.logical_and(mask, alphaMatch)

            if (self.replaceAlpha):
                texFile.img.paste(replaceColourTuple, mask = mask)
                i += 1
                continue

            newRedImg.paste(self.replaceColour.red, box = imgBox, mask = mask)
            newGreenImg.paste(self.replaceColour.green, box = imgBox, mask = mask)
            newBlueImg.paste(self.replaceColour.blue, box = imgBox, mask = mask)

            i += 1

        if (not self.replaceAlpha):
            texFile.img = Image.merge(ImgFormats.RGBA.value, (newRedImg, newGreenImg, newBlueImg, alphaImg))
##### EndScript
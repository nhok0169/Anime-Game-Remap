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

##### ExtImports
from typing import TYPE_CHECKING
##### EndExtImports

##### LocalImports
from .....constants.Packages import PackageModules
from .....constants.ColourConsts import ColourConsts
from .....constants.ImgFormats import ImgFormats
from .....tools.PackageManager import Packager
from .BaseTexFilter import BaseTexFilter

if (TYPE_CHECKING):
    from ....files.TextureFile import TextureFile
##### EndLocalImports


##### Script
class HueAdjust(BaseTexFilter):
    """
    This class inherits from :class:`BaseTexFilter`

    Adjusts the hue of a texture file

    Parameters
    ----------
    hue: :class:`int`
        The hue to adjust the image. Value is from -180 to 180
    """

    def __init__(self, hue: int):
        self.hue = hue

    def _adjustHue(self, hue: int) -> int:
        """
        Adjusts the hue

        Parameters
        ----------
        hue: :class:`int`
            The current hue that has not been adjust yet

        Returns
        -------
        :class:`int`
            The adjusted hue
        """

        result = hue + self.hue
        if (result > ColourConsts.MaxColourDegree.value):
            result = ColourConsts.MaxColourDegree.value - result
        elif (result < ColourConsts.MinColourDegree.value):
            result += ColourConsts.MaxColourValue.value

        return result
        

    def transform(self, texFile: "TextureFile"):
        Image = Packager.get(PackageModules.PIL_Image.value)

        alphaImg = texFile.img.getchannel('A')

        texFile.img = texFile.img.convert(ImgFormats.HSV.value)
        hImg, sImg, vImg = texFile.img.split()

        hImg = hImg.point(lambda hueVal: self._adjustHue(hueVal))

        texFile.img = Image.merge(ImgFormats.HSV.value, (hImg, sImg, vImg))
        texFile.img = texFile.img.convert(ImgFormats.RGBA.value)
        texFile.img.putalpha(alphaImg)
##### EndScript
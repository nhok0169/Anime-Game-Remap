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
from .....constants.ColourConsts import ColourConsts
from .BaseTexFilter import BaseTexFilter

if (TYPE_CHECKING):
    from ....files.TextureFile import TextureFile
##### EndLocalImports


##### Script
class InvertAlphaFilter(BaseTexFilter):
    """
    This class inherits from :class:`BaseTexFilter`

    Inverts the alpha channel of an image.
    """

    def transform(self, texFile: "TextureFile"):
        alphaImg = texFile.img.getchannel('A')
        alphaImg = alphaImg.point(lambda pixel: ColourConsts.MaxColourValue.value - pixel)
        texFile.img.putalpha(alphaImg)
##### EndScript
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

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: x(texFile)

            Calls :meth:`transform` for the filter, ``x``
    """

    def transform(self, texFile: "TextureFile"):
        alphaImg = texFile.img.getchannel('A')
        alphaImg = alphaImg.point(lambda pixel: ColourConsts.MaxColourValue.value - pixel)
        texFile.img.putalpha(alphaImg)
##### EndScript
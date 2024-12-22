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
if (TYPE_CHECKING):
    from ....files.TextureFile import TextureFile
##### EndLocalImports


##### Script
class BaseTexFilter():
    """
    Base class for transforming a texture file

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: x(texFile)

            Calls :meth:`BaseTexFilter.transform` for the :class:`BaseTexFilter`, ``x``
    """

    def __call__(self, texFile: "TextureFile"):
        self.transform(texFile)

    def transform(self, texFile: "TextureFile"):
        """
        Applies a Transformation to 'texFile'

        Parameters
        ----------
        texFile: :class:`TextureFile`
            The texture to be editteds
        """

        pass
##### EndScript
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
if (TYPE_CHECKING):
    from ...files.TextureFile import TextureFile
##### EndLocalImports


##### Script
class BaseTexEditor():
    """
    Base class to edit some .dds file
    """

    def fix(self, texFile: "TextureFile", fixedTexFile: str):
        """
        Edits the texture file

        Parameters
        ----------
        texFile: :class:`TextureFile`
            The texture .dds file to be modified

        fixedTexFile: :class:`str`
            The name of the fixed texture file
        """
        pass
##### EndScript
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
import os
from typing import Optional
##### EndExtImports

##### LocalImports
from ....constants.ImgFormats import ImgFormats
from ....constants.Colours import Colours
from ....constants.Packages import PackageModules
from ....tools.PackageManager import Packager
from ...textures.Colour import Colour
from .BaseTexEditor import BaseTexEditor
from ...files.TextureFile import TextureFile
##### EndLocalImports


##### Script
class TexCreator(BaseTexEditor):
    """
    This class inherits from :class:`BaseTexEditor`

    Creates a brand new .dds file if the file doe not exist
    """

    def __init__(self, width: int, height: int, colour: Optional[Colour] = None):
        self.width = width
        self.height = height
        self.colour = Colours.White.value if (colour is None) else colour

    def fix(self, texFile: "TextureFile", fixedTexFile: str):
        if (os.path.isfile(texFile.src)):
            return
        
        Image = Packager.get(PackageModules.PIL_Image.value)

        img = Image.new(mode = ImgFormats.RGBA.value, size=(self.width, self.height), color = self.colour.getTuple())
        texFile.src = fixedTexFile
        texFile.save(img = img)
##### EndScript
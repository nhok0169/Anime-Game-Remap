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
from typing import List, Union, Callable, Any, Optional
##### EndExtImports

##### LocalImports
from ....constants.Packages import PackageModules
from ....tools.PackageManager import Packager
from ...files.TextureFile import TextureFile
from .BaseTexEditor import BaseTexEditor
from .texFilters.BaseTexFilter import BaseTexFilter
##### EndLocalImportss


##### Script
class TexEditor(BaseTexEditor):
    """
    This class inherits from :class:`BaseTexEditor`

    Class for editing a texture file

    Parameters
    ----------
    filters: Optional[List[Union[:class:`BaseTexFilter`, Callable[[:class:`TextureFile`], Any]]]]
        The filters for editting the image :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    filters: List[Unsion[:class:`BaseTexFilter`, Callable[[:class:`TextureFile`], Any]]]
        The filters for editting the image :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, filters: Optional[List[Union[BaseTexFilter, Callable[[TextureFile], Any]]]] = None):
        super().__init__()
        self.filters = [] if (filters is None) else filters

    def fix(self, texFile: TextureFile, fixedTexFile: str):
        if (not self.filters):
            return

        texFile.open()
        if (texFile.img is None):
            return
        
        for filter in self.filters:
            filter(texFile)

        texFile.src = fixedTexFile
        texFile.save()

    @classmethod
    def adjustBrightness(self, texFile: TextureFile, brightness: float):
        """
        Adjust the brightness of the texture

        Parameters
        ----------
        texFile: :class:`TextureFile`
            The texture file to be editted

        brightness: :class:`float`
            The brightness to adjust the texture. :raw-html:`<br />` :raw-html:`<br />`

            0 => make the image black
            1 => original brightness of the image
            >1 => make the image brighter
        """

        ImageEnhance = Packager.get(PackageModules.PIL_ImageEnhance.value)
        
        enhancer = ImageEnhance.Brightness(texFile.img)
        texFile.img = enhancer.enhance(brightness)

    @classmethod
    def setTransparency(self, texFile: TextureFile, alpha: int):
        """
        Sets the transparency of the texture

        Parameters
        ----------
        texFile: :class:`TextureFile`
            The texture file to be editted

        alpha: :class:`int`
            The value for the alpha (transparency) channel of each pixel. Range from 0 - 255. :raw-html:`<br />` :raw-html:`<br />`

            0 => Transparent
            255 => Opaque
        """

        texFile.img.putalpha(alpha)

    @classmethod
    def adjustSaturation(self, texFile: TextureFile, saturation: float):
        """
        Adjust the saturation of the texture

        Parameters
        ----------
        texFile: :class:`TextureFile`
            The texture file to be editted

        brightness: :class:`float`
            The brightness to adjust the texture. :raw-html:`<br />` :raw-html:`<br />`

            0 => make the image black and white
            1 => original saturation of the image
            >1 => make the image really saturated like a TV
        """

        ImageEnhance = Packager.get(PackageModules.PIL_ImageEnhance.value)

        enhancer = ImageEnhance.Color(texFile.img)
        texFile.img = enhancer.enhance(saturation)
##### EndScript
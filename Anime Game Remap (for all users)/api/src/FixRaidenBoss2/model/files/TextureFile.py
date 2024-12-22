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
from typing import Optional, List, Tuple
##### EndExtImports

##### LocalImports
from ...constants.ImgFormats import ImgFormats
from ...constants.GenericTypes import Image
from ...constants.TexConsts import TexMetadataNames
from ...tools.PackageManager import Packager
from .File import File
from ...constants.GenericTypes import Image
from ...constants.Packages import PackageModules
from ..strategies.texEditors.texFilters.GammaFilter import GammaFilter
##### EndLocalImports


##### Script
class TextureFile(File):
    """
    This Class inherits from :class:`File`

    Used for handling .dds files

    Attributes
    ----------
    img: Optional[`PIL.Image`_]
        The associated image file for the texture
    """

    def __init__(self, src: str):
        self.src = src
        self.img = None

    def open(self, format: str = ImgFormats.RGBA.value) -> Image:
        """
        Opens the texture file

        Parameters
        ----------
        format: :class:`str`
            What format the image of the texture file should be opened as :raw-html:`<br />` :raw-html:`<br />`

            **Default**: "RGBA"

        Returns
        -------
        `PIL.Image`
            The image for the texture file
        """

        if (not os.path.exists(self.src)):
            self.img = None
            return None

        Image = Packager.get(PackageModules.PIL_Image.value)

        self.img = Image.open(self.src)
        self.img = self.img.convert(format)
        return self.img

    def read(self, format: str = ImgFormats.RGBA.value, flush: bool = False) -> Optional[List[List[Tuple[int, int, int, int]]]]:
        """
        Reads the pixels of the texture .dds file, if the file exists

        Parameters
        ----------
        format: :class:`str`
            What format to open the texture file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: "RGBA"

        flush: :class:`bool`
            Whether to reopen the texture file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        Optional[`PIL.PixelAccess`_]
            The pixels for the texture file with RGBA channels
        """

        if (flush or self.img is None):
            self.open(format = format)

        if (self.img is None):
            return None

        return self.img.load()
    
    def save(self, img: Optional[Image] = None):
        """
        Saves the pixels defined at 'img' to the texture .dds file

        Parameters
        ----------
        img: Optional[`PIL.Image`]
            the new image to set for the texture file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        if (img is not None):
            self.img = img

        gamma = None
        try:
            gamma = self.img.info[TexMetadataNames.Gamma.value]
        except KeyError:
            pass
        else:
            filter = GammaFilter(gamma)
            filter(self)

        self.img.save(self.src, 'DDS')
##### EndScript
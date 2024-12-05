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
from ....tools.PackageManager import Packager
from ...files.TextureFile import TextureFile
from ...textures.Colour import Colour
from .pixelfilters.BasePixelFilter import BasePixelFilter
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
    pixelFilters: Optional[List[Union[:class:`BasePixelFilter`, Callable[[:class:`Colour`], Any]]]]
        The filters to edit a single pixel in the texture file :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    preProcessor: Optional[List[Union[:class:`BaseTexFilter`, Callable[[:class:`TextureFile`], Any]]]]
        The pre-processors that transform the loaded image before the individual pixels are editted by :attr:`TexEditor.pixelFilters` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    postProcessor: Optional[List[Union[:class:`BaseTexFilter`, Callable[[:class:`TextureFile`], Any]]]]
        The post-processors that transform the loaded image after the individual pixels are editted by :attr:`TexEditor.pixelFilters` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    pixelFilters: List[Union[:class:`BasePixelFilter`, Callable[[:class:`Colour`], :class:`Colour`]]]
        The filters to edit a single pixel in the texture file

    preProcessors: List[Union[:class:`BaseTexFilter`, Callable[[:class:`TextureFile`], Any]]]
        The pre-processors that transform the loaded image before the individual pixels are editted by :attr:`TexEditor.pixelFilters`

    postProcessors: List[Union[:class:`BaseTexFilter`, Callable[[:class:`TextureFile`], Any]]]
        The post-processors that transform the loaded image after the individual pixels are editted by :attr:`TexEditor.pixelFilters`
    """

    def __init__(self, pixelFilters: Optional[List[Union[BasePixelFilter, Callable[[Colour], Colour]]]] = None,
                 preProcessors: Optional[List[Union[BaseTexFilter, Callable[[TextureFile], Any]]]] = None,
                 postProcessors: Optional[List[Union[BaseTexFilter, Callable[[TextureFile], Any]]]] = None):
        super().__init__()
        self.pixelFilters = [] if (pixelFilters is None) else pixelFilters
        self.preProcessors = [] if (preProcessors is None) else preProcessors
        self.postProcessors = [] if (postProcessors is None) else postProcessors

    def fix(self, texFile: TextureFile, fixedTexFile: str):
        texFile.open()
        if (texFile.img is None):
            return

        if (self.preProcessors):
            for preProcessor in self.preProcessors:
                preProcessor(texFile)

        if (self.pixelFilters):
            pixels = texFile.read()
            pixelColour = Colour()

            for y in range(texFile.img.size[1]):
                for x in range(texFile.img.size[0]):
                    pixel = pixels[x, y]
                    pixelColour.fromTuple(pixel)

                    for filter in self.pixelFilters:
                        if (isinstance(filter, BasePixelFilter)):
                            filter.transform(pixelColour)
                        else:
                            filter(pixelColour)

                    pixels[x, y] = pixelColour.getTuple()

        if (self.postProcessors):
            for postProcessor in self.postProcessors:
                postProcessor(texFile)

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

        ImageEnhance = Packager.get("PIL.ImageEnhance", "pillow")
        
        enhancer = ImageEnhance.Brightness(texFile.img)
        texFile.img = enhancer.enhance(brightness)

    @classmethod
    def adjustTranparency(self, texFile: TextureFile, alpha: int):
        """
        Adjust the transparency of the texture

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

        ImageEnhance = Packager.get("PIL.ImageEnhance", "pillow")

        enhancer = ImageEnhance.Color(texFile.img)
        texFile.img = enhancer.enhance(saturation)
##### EndScript
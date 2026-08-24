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
from typing import List, Union, Callable, Any, Optional
##### EndExtImports

##### CppLocalImports
from ....core import CppTexEditor
##### EndCppLocalImports

##### LocalImports
from ....constants.Packages import PackageModules
from ....constants.GlobalPackageManager import GlobalPackageManager
from ....constants.TexEngine import TexEngine
from ...files.TextureFile import TextureFile
from .BaseTexEditor import BaseTexEditor
from .texFilters.BaseTexFilter import BaseTexFilter
##### EndLocalImports


##### Script
class TexEditor(CppTexEditor):
    """
    This class inherits from :class:`CppTexEditor`

    Class for editing a texture file

    .. note::
        :meth:`fix` is entirely reimplemented in Python here (rather than using
        :class:`CppTexEditor`'s own C++ filter list) so that :attr:`filters` can hold arbitrary
        Python callables -- a plain function, a bound classmethod, or a :class:`BaseTexFilter`
        instance -- exactly like the original pure-Python implementation

    Parameters
    ----------
    filters: Optional[List[Union[:class:`BaseTexFilter`, Callable[[:class:`TextureFile`], Any]]]]
        The filters for editting the image :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    engine: :class:`TexEngine`
        Which engine to use to read/write the texture file being edited -- overrides whatever
        engine the ``texFile`` passed into :meth:`fix` was itself constructed with, for the
        duration of that :meth:`fix` call :raw-html:`<br />` :raw-html:`<br />`

        **Default**: :attr:`TexEngine.Compressonator`

    readPillowImg: :class:`bool`
        Whether to maintain :attr:`TextureFile.img` when :attr:`engine` is
        :attr:`TexEngine.Compressonator` -- also overrides ``texFile``'s own value for the
        duration of :meth:`fix`. See :class:`TextureFile`'s own notes on this flag :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    Attributes
    ----------
    filters: List[Union[:class:`BaseTexFilter`, Callable[[:class:`TextureFile`], Any]]]
        The filters for editting the image :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    engine: :class:`TexEngine`
        Which engine to use to read/write the texture file being edited

    readPillowImg: :class:`bool`
        Whether to maintain :attr:`TextureFile.img` when :attr:`engine` is
        :attr:`TexEngine.Compressonator`
    """

    def __init__(self, filters: Optional[List[Union[BaseTexFilter, Callable[[TextureFile], Any]]]] = None, engine: TexEngine = TexEngine.Compressonator, readPillowImg: bool = False):
        super().__init__()
        self.filters = [] if (filters is None) else filters
        self.engine = engine
        self.readPillowImg = readPillowImg

    def fix(self, texFile: TextureFile, fixedTexFile: str):
        if (not self.filters):
            return

        texFile.engine = self.engine
        texFile.readPillowImg = self.readPillowImg
        texFile.open()
        if (not texFile.hasImage):
            return

        for filter in self.filters:
            filter(texFile)

        texFile.src = fixedTexFile
        texFile.save()

    @classmethod
    def adjustBrightness(cls, texFile: TextureFile, brightness: float):
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

        ImageEnhance = GlobalPackageManager.get(PackageModules.PIL_ImageEnhance.value)

        enhancer = ImageEnhance.Brightness(texFile.img)
        texFile.img = enhancer.enhance(brightness)

    @classmethod
    def setTransparency(cls, texFile: TextureFile, alpha: int):
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
    def adjustSaturation(cls, texFile: TextureFile, saturation: float):
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

        ImageEnhance = GlobalPackageManager.get(PackageModules.PIL_ImageEnhance.value)

        enhancer = ImageEnhance.Color(texFile.img)
        texFile.img = enhancer.enhance(saturation)
##### EndScript

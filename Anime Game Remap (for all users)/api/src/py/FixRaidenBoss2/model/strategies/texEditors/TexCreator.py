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
import os
from typing import Optional
##### EndExtImports

##### CppLocalImports
from ....core import CppTexCreator
##### EndCppLocalImports

##### LocalImports
from ....constants.ImgFormats import ImgFormats
from ....constants.Packages import PackageModules
from ....constants.GlobalPackageManager import GlobalPackageManager
from ....constants.TexEngine import TexEngine
from ...textures.Colour import Colour
from .BaseTexEditor import BaseTexEditor
from ...files.TextureFile import TextureFile
##### EndLocalImports


##### Script
class TexCreator(CppTexCreator):
    """
    This class inherits from :class:`CppTexCreator`

    Creates a brand new .dds file if the file does not exist

    .. note::
        :meth:`fix` is reimplemented in Python here (rather than using :class:`CppTexCreator`'s own
        C++ implementation) so that the newly-created texture goes through :class:`TextureFile`'s
        own :meth:`~TextureFile.save`, matching every other texture edit in this codebase

    Parameters
    ----------
    width: :class:`int`
        The width, in pixels, of the texture to create

    height: :class:`int`
        The height, in pixels, of the texture to create

    colour: Optional[:class:`Colour`]
        The fill colour of the texture to create :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None`` (opaque white)

    engine: :class:`TexEngine`
        Which engine to use to read/write the texture file being created -- overrides whatever
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
    engine: :class:`TexEngine`
        Which engine to use to read/write the texture file being created

    readPillowImg: :class:`bool`
        Whether to maintain :attr:`TextureFile.img` when :attr:`engine` is
        :attr:`TexEngine.Compressonator`
    """

    def __init__(self, width: int, height: int, colour: Optional[Colour] = None, engine: TexEngine = TexEngine.Compressonator, readPillowImg: bool = False):
        if (colour is None):
            super().__init__(width, height)
        else:
            super().__init__(width, height, colour)
        self.engine = engine
        self.readPillowImg = readPillowImg

    def fix(self, texFile: "TextureFile", fixedTexFile: str):
        if (os.path.isfile(texFile.src)):
            return

        Image = GlobalPackageManager.get(PackageModules.PIL_Image.value)

        img = Image.new(mode = ImgFormats.RGBA.value, size = (self.width, self.height), color = self.colour.getTuple())
        texFile.engine = self.engine
        texFile.readPillowImg = self.readPillowImg
        texFile.src = fixedTexFile
        texFile.save(img = img)
##### EndScript

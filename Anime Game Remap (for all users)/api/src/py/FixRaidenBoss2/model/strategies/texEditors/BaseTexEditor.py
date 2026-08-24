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

##### CppLocalImports
from ....core import CppBaseTexEditor
##### EndCppLocalImports

##### LocalImports
from ....constants.TexEngine import TexEngine
##### EndLocalImports


##### Script
class BaseTexEditor(CppBaseTexEditor):
    """
    This class inherits from :class:`CppBaseTexEditor`

    Base class to edit some .dds file

    Parameters
    ----------
    engine: :class:`TexEngine`
        Which engine subclasses should use to read/write the texture file being edited :raw-html:`<br />` :raw-html:`<br />`

        **Default**: :attr:`TexEngine.Compressonator`

    readPillowImg: :class:`bool`
        Whether subclasses should maintain :attr:`TextureFile.img` when :attr:`engine` is
        :attr:`TexEngine.Compressonator` -- ignored when :attr:`engine` is
        :attr:`TexEngine.Pillow`. See :class:`TextureFile`'s own notes on this flag :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    Attributes
    ----------
    engine: :class:`TexEngine`
        Which engine subclasses should use to read/write the texture file being edited

    readPillowImg: :class:`bool`
        Whether subclasses should maintain :attr:`TextureFile.img` when :attr:`engine` is
        :attr:`TexEngine.Compressonator`
    """

    def __init__(self, engine: TexEngine = TexEngine.Compressonator, readPillowImg: bool = False):
        super().__init__()
        self.engine = engine
        self.readPillowImg = readPillowImg
##### EndScript

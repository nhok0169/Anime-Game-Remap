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
from typing import Optional, List, Tuple, Any, Dict
##### EndExtImports

##### CppLocalImports
from ...core import CppTextureFile
##### EndCppLocalImports

##### LocalImports
from ...constants.ImgFormats import ImgFormats
from ...constants.TexConsts import TexMetadataNames
from ...constants.TexEngine import TexEngine
from ...constants.GenericTypes import Image
from ...constants.Packages import PackageModules
from ...constants.GlobalPackageManager import GlobalPackageManager
from ..strategies.texEditors.texFilters.GammaFilter import GammaFilter
##### EndLocalImports


##### Script
class TextureFile(CppTextureFile):
    """
    This class inherits from :class:`CppTextureFile`

    Used for handling .dds files

    .. note::
        When :attr:`engine` is :attr:`TexEngine.Compressonator`, reading/writing pixels themselves
        (:meth:`CppTextureFile.open`/:meth:`CppTextureFile.save`) is done via `Compressonator`_ --
        when it's :attr:`TexEngine.Pillow`, the ``.dds`` file itself is read/written directly via
        `Pillow`_ instead, bypassing `Compressonator`_ entirely (gamma correction still runs through
        the same :class:`GammaFilter` either way, since that only ever touches the in-memory pixel
        buffer, never the file itself)

    .. note::
        :attr:`img` (a real `Pillow`_ `PIL.Image`_) is only needed for compatibility with a handful
        of things in this codebase that still work directly against `Pillow`_ (a few filters, and a
        couple of per-character custom edits in ``data/IniParseBuilderData.py``). When
        :attr:`engine` is :attr:`TexEngine.Compressonator` and :attr:`readPillowImg` is ``False``
        (the default), :meth:`open`/:meth:`save` skip maintaining :attr:`img` entirely and every
        ported filter (see ``PyTexFilterCommon.h``) operates directly on the native `Compressonator`_
        buffer instead, for real C++ speed with zero `Pillow`_ overhead. Set :attr:`readPillowImg`
        to ``True`` (or use :meth:`read`, which always builds :attr:`img` on demand regardless of
        the flag) whenever something in the filter chain genuinely needs :attr:`img` to be real.
        :attr:`readPillowImg` is ignored entirely when :attr:`engine` is :attr:`TexEngine.Pillow`,
        since :attr:`img` **is** the only pixel representation in that mode

    Parameters
    ----------
    src: :class:`str`
        The source file for the texture file

    engine: :class:`TexEngine`
        Which engine to use to read/write the texture file :raw-html:`<br />` :raw-html:`<br />`

        **Default**: :attr:`TexEngine.Compressonator`

    readPillowImg: :class:`bool`
        Whether :meth:`open`/:meth:`save` should maintain :attr:`img` when :attr:`engine` is
        :attr:`TexEngine.Compressonator`. Ignored when :attr:`engine` is :attr:`TexEngine.Pillow`
        (always treated as ``True`` in that case) :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    Attributes
    ----------
    img: Optional[`PIL.Image`_]
        The associated image file for the texture

    info: Dict[:class:`str`, Any]
        Metadata about the texture. Currently only the ``"gamma"`` key (see
        :class:`TexMetadataNames`) has any effect, applied on the next :meth:`save`

    engine: :class:`TexEngine`
        Which engine to use to read/write the texture file

    readPillowImg: :class:`bool`
        Whether :meth:`open`/:meth:`save` should maintain :attr:`img` when :attr:`engine` is
        :attr:`TexEngine.Compressonator`
    """

    def __init__(self, src: str, engine: TexEngine = TexEngine.Compressonator, readPillowImg: bool = False):
        super().__init__(src)
        self.img: Optional[Image] = None
        self.info: Dict[str, Any] = {}
        self.engine = engine
        self.readPillowImg = readPillowImg

    @property
    def hasImage(self) -> bool:
        """
        :class:`bool`: Whether a texture is currently loaded :raw-html:`<br />` :raw-html:`<br />`

        .. note::
            Overrides :attr:`CppTextureFile.hasImage` to derive from :attr:`img` when it's being
            maintained, falling back to `Compressonator`_'s own internal buffer state otherwise --
            :attr:`img` is never populated when :attr:`engine` is :attr:`TexEngine.Compressonator`
            and :attr:`readPillowImg` is ``False``
        """

        if (self.img is not None):
            return True
        return CppTextureFile.hasImage.fget(self)

    @property
    def width(self) -> int:
        """
        :class:`int`: The width, in pixels, of the currently loaded texture (0 if :attr:`hasImage`
        is ``False``) -- see :attr:`hasImage`'s own note for why this overrides
        :attr:`CppTextureFile.width`
        """

        if (self.img is not None):
            return self.img.width
        return CppTextureFile.width.fget(self)

    @property
    def height(self) -> int:
        """
        :class:`int`: The height, in pixels, of the currently loaded texture (0 if :attr:`hasImage`
        is ``False``) -- see :attr:`hasImage`'s own note for why this overrides
        :attr:`CppTextureFile.height`
        """

        if (self.img is not None):
            return self.img.height
        return CppTextureFile.height.fget(self)

    def open(self, format: str = ImgFormats.RGBA.value) -> Optional[Image]:
        """
        Opens the texture file

        Parameters
        ----------
        format: :class:`str`
            What format the image of the texture file should be opened as :raw-html:`<br />` :raw-html:`<br />`

            **Default**: "RGBA"

        Returns
        -------
        Optional[`PIL.Image`_]
            The image for the texture file, or ``None`` if either the file doesn't exist, or
            :attr:`img` isn't being maintained (see :attr:`readPillowImg`) -- check :attr:`hasImage`
            instead of this return value to see whether the texture actually loaded
        """

        Image = GlobalPackageManager.get(PackageModules.PIL_Image.value)

        if (self.engine == TexEngine.Pillow):
            if (not os.path.isfile(self.src)):
                self.img = None
                return None

            self.img = Image.open(self.src).convert(ImgFormats.RGBA.value)
            self.setPixels(self.img.tobytes(), self.img.width, self.img.height)
            return self.img

        super().open()
        if (not CppTextureFile.hasImage.fget(self)):
            self.img = None
            return None

        if (self.readPillowImg):
            self.img = Image.frombytes(ImgFormats.RGBA.value, (CppTextureFile.width.fget(self), CppTextureFile.height.fget(self)), self.getPixels())
        else:
            self.img = None

        return self.img

    def read(self, format: str = ImgFormats.RGBA.value, flush: bool = False) -> Optional[List[List[Tuple[int, int, int, int]]]]:
        """
        Reads the pixels of the texture .dds file, if the file exists :raw-html:`<br />` :raw-html:`<br />`

        Unlike :meth:`open`/:meth:`save`, this always builds :attr:`img` on demand if needed,
        regardless of :attr:`readPillowImg` -- this method's whole contract is `Pillow`_-shaped
        pixel access, so there's no buffer-native equivalent to fall back to here

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

        if (flush or not self.hasImage):
            self.open(format = format)

        if (not self.hasImage):
            return None

        if (self.img is None):
            Image = GlobalPackageManager.get(PackageModules.PIL_Image.value)
            self.img = Image.frombytes(ImgFormats.RGBA.value, (CppTextureFile.width.fget(self), CppTextureFile.height.fget(self)), self.getPixels())

        return self.img.load()

    def save(self, img: Optional[Image] = None):
        """
        Saves the pixels defined at 'img' to the texture .dds file

        Parameters
        ----------
        img: Optional[`PIL.Image`_]
            the new image to set for the texture file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        Image = GlobalPackageManager.get(PackageModules.PIL_Image.value)

        if (img is not None):
            self.img = img

        if (self.img is not None and self.img.mode != ImgFormats.RGBA.value):
            self.img = self.img.convert(ImgFormats.RGBA.value)

        gamma = self.info.get(TexMetadataNames.Gamma.value, None)

        if (self.engine == TexEngine.Pillow):
            # Pillow engine always needs a real self.img -- there is no other pixel representation
            # before this point
            if (gamma is not None):
                # GammaFilter only ever touches the in-memory pixel buffer (getPixels/setPixels,
                # both inherited from CppTextureFile), never the file itself -- reusable regardless
                # of which engine actually reads/writes the .dds file
                self.setPixels(self.img.tobytes(), self.img.width, self.img.height)
                GammaFilter(gamma).transform(self)
                self.img = Image.frombytes(ImgFormats.RGBA.value, self.img.size, self.getPixels())

            self.img.save(self.src, 'DDS')
            self.setPixels(self.img.tobytes(), self.img.width, self.img.height)
            return

        # Compressonator engine: if self.img is set (an explicit 'img' argument, or a filter chain
        # that needed it), push it into the native buffer -- otherwise the buffer already holds
        # whatever a buffer-native filter (or the last open()) left it as
        if (self.img is not None):
            self.setPixels(self.img.tobytes(), self.img.width, self.img.height)

        self.gamma = gamma
        super().save()

        if (self.readPillowImg):
            self.img = Image.frombytes(ImgFormats.RGBA.value, (CppTextureFile.width.fget(self), CppTextureFile.height.fget(self)), self.getPixels())
        else:
            self.img = None
##### EndScript

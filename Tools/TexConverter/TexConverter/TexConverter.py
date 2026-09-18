import os
import sys
from typing import List, Optional, Tuple

from .constants.Paths import APISrcPath


class TexConverter():
    """
    Converts ``.dds`` textures into an ordinary image format (``.png`` by default) so they can
    actually be looked at -- most image viewers, browsers and AI agents can't open a BCn-compressed
    ``.dds`` at all

    Parameters
    ----------
    src: :class:`str`
        The ``.dds`` file, or the folder of ``.dds`` files, to convert

    dest: :class:`str`
        Where to write the converted image(s). For a single source file this may be either the
        output file itself or a folder to put it in; for a source folder this is always a folder,
        and the source's subfolder structure is mirrored underneath it

    format: :class:`str`
        The image format to convert to, as a bare file extension (``"png"``, ``"bmp"``, ``"jpg"``,
        or ``"dds"`` to just re-encode)

    alpha: :class:`str`
        How to treat the texture's alpha channel :raw-html:`<br />` :raw-html:`<br />`

        - ``"drop"`` -- force every pixel opaque, so the RGB artwork is always visible
        - ``"keep"`` -- write all 4 channels exactly as decoded
        - ``"only"`` -- write the alpha channel on its own, as a greyscale image

        ``"drop"`` is the default **on purpose**: these textures routinely use alpha as a *mask*
        channel rather than as transparency (a GIMI face diffuse packs its blush mask in there),
        so an ordinary image viewer honouring it renders the texture blank and invites the wrong
        conclusion that it's empty. Use ``"keep"`` when you actually need a faithful copy, and
        ``"only"`` to look at what the alpha channel is really storing

    Attributes
    ----------
    src: :class:`str`
        The ``.dds`` file/folder to convert

    dest: :class:`str`
        Where to write the converted image(s)

    format: :class:`str`
        The image format to convert to, as a bare file extension

    alpha: :class:`str`
        How to treat the texture's alpha channel
    """

    DDSExt = ".dds"
    ImgExts = {".png", ".bmp", ".jpg", ".jpeg", ".dds"}

    AlphaDrop = "drop"
    AlphaKeep = "keep"
    AlphaOnly = "only"
    AlphaModes = (AlphaDrop, AlphaKeep, AlphaOnly)

    def __init__(self, src: str, dest: str, format: str = "png", alpha: str = AlphaDrop):
        if (alpha not in self.AlphaModes):
            raise ValueError(f"Unknown alpha mode '{alpha}' -- expected one of {', '.join(self.AlphaModes)}")

        self.src = src
        self.dest = dest
        self.format = format.lower().lstrip(".")
        self.alpha = alpha

    def _getTextureFileCls(self):
        """
        Imports the API's :class:`TextureFile`, with a readable error if the API isn't built yet

        Returns
        -------
        The ``TextureFile`` class
        """

        if (APISrcPath not in sys.path):
            sys.path.insert(1, APISrcPath)

        try:
            import FixRaidenBoss2 as FRB
        except ImportError as e:
            raise ImportError(
                f"Could not import the AG Remap API from '{APISrcPath}'.\n"
                "The API's native extensions probably aren't built yet -- see "
                "'AI Agent Help/Setup/CLAUDE.md' and 'AI Agent Help/Building/CLAUDE.md'.\n"
                f"Original error: {e}"
            ) from e

        return FRB.TextureFile

    def _getConversions(self) -> List[Tuple[str, str]]:
        """
        Works out every (source .dds, destination image) pair this conversion covers

        Returns
        -------
        List[Tuple[:class:`str`, :class:`str`]]
            The (source, destination) file pairs
        """

        if (os.path.isfile(self.src)):
            return [(self.src, self._getFileDest(self.src))]

        if (not os.path.isdir(self.src)):
            raise FileNotFoundError(f"No such file or folder: '{self.src}'")

        result = []
        for root, _dirs, files in os.walk(self.src):
            for file in files:
                if (not file.lower().endswith(self.DDSExt)):
                    continue

                srcFile = os.path.join(root, file)

                # mirror the source's own subfolder structure under 'dest', rather than flattening
                # everything into one folder -- mods routinely reuse the same texture filenames
                # across subfolders, and flattening would silently overwrite them
                relFolder = os.path.relpath(root, self.src)
                destFolder = self.dest if (relFolder == ".") else os.path.join(self.dest, relFolder)
                result.append((srcFile, os.path.join(destFolder, self._changeExt(file))))

        return result

    def _changeExt(self, file: str) -> str:
        """
        Swaps 'file's extension for :attr:`format`

        Parameters
        ----------
        file: :class:`str`
            The file name to change

        Returns
        -------
        :class:`str`
            The file name, with its extension replaced
        """

        return f"{os.path.splitext(file)[0]}.{self.format}"

    def _getFileDest(self, srcFile: str) -> str:
        """
        Works out the destination file for a single source file :raw-html:`<br />` :raw-html:`<br />`

        :attr:`dest` may be either the output file itself, or a folder to put it in

        Parameters
        ----------
        srcFile: :class:`str`
            The source file being converted

        Returns
        -------
        :class:`str`
            The destination file
        """

        destIsFolder = os.path.isdir(self.dest) or os.path.splitext(self.dest)[1].lower() not in self.ImgExts
        if (destIsFolder):
            return os.path.join(self.dest, self._changeExt(os.path.basename(srcFile)))

        return self.dest

    def _applyAlphaMode(self, pixels: bytes) -> bytes:
        """
        Rewrites a flat RGBA8 pixel buffer according to :attr:`alpha` :raw-html:`<br />` :raw-html:`<br />`

        Done with whole-buffer slice assignment rather than a per-pixel loop -- these are 1024x1024
        textures (4MB of bytes each), where a Python-level loop is very noticeably slower

        Parameters
        ----------
        pixels: :class:`bytes`
            The flat RGBA8 pixel buffer to rewrite

        Returns
        -------
        :class:`bytes`
            The rewritten buffer
        """

        result = bytearray(pixels)
        opaque = b"\xff" * (len(pixels) // 4)

        if (self.alpha == self.AlphaOnly):
            alphas = pixels[3::4]
            result[0::4] = alphas
            result[1::4] = alphas
            result[2::4] = alphas

        result[3::4] = opaque
        return bytes(result)

    def convert(self, silent: bool = False) -> List[str]:
        """
        Runs the conversion

        Parameters
        ----------
        silent: :class:`bool`
            Whether to skip printing progress :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        List[:class:`str`]
            The image files that were actually written
        """

        TextureFile = self._getTextureFileCls()
        conversions = self._getConversions()
        written = []

        for srcFile, destFile in conversions:
            destFolder = os.path.dirname(destFile)
            if (destFolder):
                os.makedirs(destFolder, exist_ok = True)

            texFile = TextureFile(srcFile)
            texFile.open()

            if (texFile.hasImage and self.alpha != self.AlphaKeep):
                texFile.setPixels(self._applyAlphaMode(texFile.getPixels()), texFile.width, texFile.height)

            if (texFile.saveAs(destFile)):
                written.append(destFile)
                if (not silent):
                    print(f"[ok]     {srcFile}\n     ->  {destFile}")
            elif (not silent):
                # saveAs returns False either because the source wasn't a readable texture, or
                # because the requested output format isn't one anything downstream can write
                print(f"[FAILED] {srcFile}  (unreadable texture, or unsupported output format '{self.format}')")

        if (not silent):
            print(f"\nConverted {len(written)}/{len(conversions)} texture(s)")

        return written

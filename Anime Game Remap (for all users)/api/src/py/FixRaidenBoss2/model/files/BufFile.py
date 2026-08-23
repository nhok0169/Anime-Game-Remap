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
from ...core import CppBufFile
##### EndCppLocalImports

##### LocalImports
from ...constants.GenericTypes import PdDataFrame
from ...tools.BufTools import BufTools
##### EndLocalImports


##### Script
class BufFile(CppBufFile):
    """
    This class inherits from :class:`CppBufFile`

    A class to handle .buf files

    A ``.buf`` file is a binary file made up of a sequence of same-sized "lines" (one line per
    vertex), each one composed of the same sequence of :class:`BufElementType`\\s -- there is no
    header or footer, just the lines themselves back-to-back :raw-html:`<br />` :raw-html:`<br />`

    .. note::
        This class itself only adds :meth:`toDataFrame` on top of what :class:`CppBufFile` already
        provides -- see that class for :attr:`~CppBufFile.elements`/:attr:`~CppBufFile.bytesPerLine`/
        :meth:`~CppBufFile.decodeLine`/:meth:`~CppBufFile.encodeLine`/:meth:`~CppBufFile.fix`/etc.

    Parameters
    ----------
    src: Union[:class:`str`, :class:`bytes`]
        The source file or bytes for the .buf file

    elements: List[:class:`BufElementType`]
        The sequence of elements within the .buf file

    fileType: :class:`str`
        The name for the type of .buf file :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``Buffer``
    """

    def toDataFrame(self) -> PdDataFrame:
        """
        Transforms the frame (line/vertex) data of this .buf file into a `pandas DataFrame`_

        .. note::
            This is a convenience for calling :meth:`BufTools.toDataFrame`

        Returns
        -------
        `pandas.DataFrame`_
            One row per line (vertex); one column per scalar value within an element
        """

        return BufTools.toDataFrame(self)
##### EndScript

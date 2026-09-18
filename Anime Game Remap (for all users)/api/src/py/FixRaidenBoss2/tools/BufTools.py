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
from ..core import CppBufFile
##### EndCppLocalImports

##### LocalImports
from ..constants.GenericTypes import PdDataFrame
from ..constants.GlobalPackageManager import GlobalPackageManager
from ..constants.Packages import PackageModules
##### EndLocalImports


##### Script
class BufTools():
    """
    Tools for working with the decoded frame (line/vertex) data of a .buf file
    """

    @classmethod
    def toDataFrame(cls, bufFile: CppBufFile) -> PdDataFrame:
        """
        Transforms the frame (line/vertex) data of a .buf file into a `pandas DataFrame`_

        .. note::
            'pandas' is lazily imported the same way as :meth:`DictTools.nestedDictToDataFrame`
            (via :meth:`GlobalPackageManager.get`) -- it stays an optional dependency, only
            actually imported the first time this function is called

        .. note::
            The columns are keyed by ``(elementKey, indexWithinElement)`` and ordered by the
            *sorted* order of the element keys, not the elements' declared order. Sorting keeps the
            resulting columns reproducible no matter what order the underlying decode reports them
            in

        .. note::
            The decoding itself is done in one bulk call to :meth:`CppBufFile.decodeAll`, which
            decodes every line in C++ and returns one `NumPy`_ array per column. Going line by line
            through :meth:`CppBufFile.decodeLine` instead would cost one crossing into C++ (plus a
            dict, a list per element and a boxed ``Python`` number per value) *per line*, which
            measured ~20x slower on a 50,000 line ``Blend.buf``

        Parameters
        ----------
        bufFile: :class:`CppBufFile`
            The .buf file to convert. Only needs :attr:`~CppBufFile.data`, :attr:`~CppBufFile.bytesPerLine`,
            and :meth:`~CppBufFile.decodeLine` -- :class:`BufFile` (the pure-Python subclass) and
            :class:`BlendFile`/:class:`PositionFile` (both real subclasses of :class:`CppBufFile`
            in their own right) all work here

        Returns
        -------
        `pandas.DataFrame`_
            One row per line (vertex); one column per scalar value within an element
        """

        pd = GlobalPackageManager.get(PackageModules.Pandas.value)

        data = bufFile.data
        bytesPerLine = bufFile.bytesPerLine

        if (not data or not bytesPerLine):
            return pd.DataFrame()

        columns = bufFile.decodeAll()
        columnKeys = sorted(columns.keys())

        # Built positionally then relabelled, rather than handing pandas the {(key, ind): array}
        # dict straight up, so the columns are guaranteed to come out as a real MultiIndex in
        # exactly 'columnKeys' order instead of depending on how pandas chooses to read tuple keys
        result = pd.DataFrame({colInd: columns[columnKey] for colInd, columnKey in enumerate(columnKeys)})
        result.columns = pd.MultiIndex.from_tuples(columnKeys)

        return result

    @classmethod
    def fromDataFrame(cls, bufFile: CppBufFile, df: PdDataFrame):
        """
        Stores the frame (line/vertex) data of a `pandas DataFrame`_ back into a .buf file --
        the inverse of :meth:`toDataFrame` :raw-html:`<br />` :raw-html:`<br />`

        Each row of ``df`` is encoded into one line (vertex) using the .buf file's *current*
        :attr:`~CppBufFile.elements`, and the resulting bytes replace the file's
        :attr:`~CppBufFile.data`

        .. note::
            The columns are read by their ``(elementKey, indexWithinElement)`` keys, not by
            position, so a ``df`` whose columns were reordered by some `pandas`_ operation still
            encodes correctly. The values within one element are ordered by
            ``indexWithinElement`` :raw-html:`<br />` :raw-html:`<br />`

            This assumes ``df``'s columns match the elements the .buf file currently has -- if
            you changed :attr:`~CppBufFile.elements` first, pass a ``df`` matching the *new*
            elements, since those are what the encoding uses

        .. note::
            :attr:`~CppBufFile.data` cannot be assigned directly, so this sets
            :attr:`~CppBufFile.src` to the newly encoded bytes and re-reads from it -- the same
            approach :meth:`TexEditor.fix` uses for a texture file. **A .buf file originally
            constructed from a file path therefore ends up with raw bytes as its**
            :attr:`~CppBufFile.src`; the file on disk is untouched (write it out with
            :meth:`~CppBufFile.fix` if you want it saved)

        Parameters
        ----------
        bufFile: :class:`CppBufFile`
            The .buf file to store the data into. Only needs :attr:`~CppBufFile.src`,
            :meth:`~CppBufFile.encodeLine` and :meth:`~CppBufFile.read` -- :class:`BufFile`,
            :class:`BlendFile` and :class:`PositionFile` all work here

        df: `pandas.DataFrame`_
            The frame data to store, shaped the same way :meth:`toDataFrame` produces it -- one
            row per line (vertex), one column per scalar value within an element

        Raises
        ------
        :class:`BadBufData`
            If the encoded bytes do not divide evenly into lines for the .buf file's current
            :attr:`~CppBufFile.elements`
        """

        # Each column is handed over on its own (instead of a whole-frame 'to_numpy'), so every one
        # keeps its own dtype -- a frame mixing float weights with integer indices (eg. a
        # Blend.buf's BLENDWEIGHT/BLENDINDICES) would otherwise be upcast to one common dtype,
        # silently handing every integer element a float to encode
        columns = {}
        for colInd, column in enumerate(df.columns):
            elementKey, valueInd = column
            columns[(elementKey, valueInd)] = df.iloc[:, colInd].to_numpy()

        bufFile.encodeAll(columns)

    @classmethod
    def getDumpStr(cls, bufFile: CppBufFile, prefix: str = "vb0") -> str:
        """
        Retrieves the *data* section of the dump text for a .buf file -- the text a 3dmigoto frame
        analysis would produce, which `Blender`_ can then import :raw-html:`<br />` :raw-html:`<br />`

        One line per element per vertex, in the elements' declared order, shaped as
        ``prefix[vertexInd]+byteOffset ElementKey: value, value, ...``, with a blank line between
        vertices

        .. note::
            **This is deliberately only the data.** A real dump file also needs a header, and that
            header differs by the kind of buffer being dumped -- see :class:`VbFile` and
            :class:`IbFile`, which add one each

        .. note::
            The work itself is done by :meth:`CppBufFile.getDumpStr` in C++ -- this is the tools
            entry point for it, kept alongside :meth:`toDataFrame`/:meth:`fromDataFrame`. It always
            returns the *data* section, even for a :class:`VbFile`, whose own
            :meth:`~VbFile.getDumpStr` would prepend a header

        Parameters
        ----------
        bufFile: :class:`CppBufFile`
            The .buf file to dump. :class:`BufFile`, :class:`BlendFile` and :class:`PositionFile`
            all work here

        prefix: :class:`str`
            The buffer name each entry is prefixed with :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``"vb0"``

        Returns
        -------
        :class:`str`
            The data section of the dump text. Empty when the file has no lines
        """

        # Deliberately reaches CppBufFile's own implementation rather than 'bufFile.getDumpStr',
        # so this always means "the data section" -- a VbFile overrides getDumpStr to prepend its
        # header, and going through the instance would quietly return that instead
        return CppBufFile.getDumpStr(bufFile, prefix)

    @classmethod
    def readDumpStr(cls, bufFile: CppBufFile, text: str):
        """
        Reads dump text back into a .buf file's bytes -- the inverse of :meth:`getDumpStr`

        .. note::
            The work itself is done by :meth:`CppBufFile.readDumpStr` in C++ -- this is the tools
            entry point for it. The values are encoded with the file's **current**
            :attr:`~CppBufFile.elements`, and a whole dump file is accepted, not just the data
            section (its header is skipped)

        .. note::
            Like :meth:`getDumpStr`, this always goes through :class:`CppBufFile`'s own
            implementation, so a :class:`VbFile` is read the same way any other .buf file is --
            call :meth:`VbFile.readDumpStr` directly if you want its header parsed for the elements
            too

        Parameters
        ----------
        bufFile: :class:`CppBufFile`
            The .buf file to read the dump text into

        text: :class:`str`
            The dump text to read

        Raises
        ------
        :class:`BadBufData`
            If the parsed bytes do not divide evenly into lines
        """

        CppBufFile.readDumpStr(bufFile, text)
##### EndScript

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
            *sorted* order of the decoded line's element keys, not the elements' declared order.
            This is deliberate: :meth:`CppBufFile.decodeLine` builds the decoded line from a
            ``std::unordered_map``, whose iteration order is not guaranteed to match the elements'
            declaration order (or even to stay identical between builds). Sorting keeps the
            resulting columns reproducible regardless of that

        Parameters
        ----------
        bufFile: :class:`CppBufFile`
            The .buf file to convert. Only needs :attr:`~CppBufFile.data`, :attr:`~CppBufFile.bytesPerLine`,
            and :meth:`~CppBufFile.decodeLine` -- :class:`BufFile`/:class:`BlendFile`/:class:`PositionFile`
            and their C++-backed counterparts (:class:`CppBufFile`/:class:`CppBlendFile`/
            :class:`CppPositionFile`) are all real subclasses of :class:`CppBufFile`, so any of them
            work here

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

        rows = []
        columns = None

        for startInd in range(0, len(data), bytesPerLine):
            decoded = bufFile.decodeLine(data[startInd: startInd + bytesPerLine])
            elementKeys = sorted(decoded.keys())

            if (columns is None):
                columns = pd.MultiIndex.from_tuples([(elementKey, i) for elementKey in elementKeys for i in range(len(decoded[elementKey]))])

            row = []
            for elementKey in elementKeys:
                row.extend(decoded[elementKey])
            rows.append(row)

        return pd.DataFrame(rows, columns = columns)
##### EndScript

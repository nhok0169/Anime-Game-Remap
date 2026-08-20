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
from typing import Generic, Optional, Dict, Any, List, Union, Hashable, Set
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import T
from ...core import CppModAssets
from .BaseModAssets import BaseModAssets
##### EndLocalImports


##### Script
class ModAssets(CppModAssets, Generic[T]):
    """
    This class inherits from :class:`CppModAssets`

    Class to handle assets of any type for a mod where retrieval is based on some keys where 1 or more of the keys refer to some versioning  :raw-html:`<br />` :raw-html:`<br />`

    .. tip::
        If the assets have more than 1 column that refers to some version, use this data structure. Otherwise if your asset has only 1 column that refers to some version,
        it recommended to use :class:`ModDictAssets` instead since it uses a hash based access instead of a linear scan

    .. note::
        Unlike the pure-Python original this class used to be, the underlying lookup itself is
        now the C++-backed :class:`CppModAssets` -- this class only adds back the flexible,
        name-keyed argument shapes (a bare value, a list, or a dict keyed by index name) real
        callers still use (e.g. :meth:`ModType.getVGRemap`), converting them into the plain
        positional lists :class:`CppModAssets` expects

    Parameters
    ----------
    repo: Union[List[Tuple[List[`Hashable`_], Any]], Dict[`Hashable`_, Any]]
        The original source for the assets -- either an already-flattened list of
        ``(indexVals, value)`` tuples, or a nested dictionary

    indices: Optional[List[:class:`str`]]
        The names of the index columns to query to retrive the main content of the asset :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will set 2 index column by the names "version" and "name" by default :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    versionIndices: Optional[Set[:class:`str`]]
        The names of the index columns that refer to some version :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will set an index to the name "version" by default :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    valueCol: Optional[:class:`str`]
        Unused by the new C++-backed lookup (rows already carry their own value, not selected by
        column name) -- kept only for constructor-signature backward compatibility

        **Default**: ``None``
    """

    VersionKey = "version"
    NameKey = "name"
    ValueKey = "value"

    def __init__(self, repo: Union[List[Any], Dict[Hashable, Any]], indices: Optional[List[str]] = None, versionIndices: Optional[Set[str]] = None, valueCol: Optional[str] = None, **kwargs):
        self._indices = [self.VersionKey, self.NameKey] if (indices is None) else indices

        uniqueIndices = set(self._indices)
        if (len(uniqueIndices) != len(self._indices)):
            raise KeyError("Index names must be unique")

        newVersionIndices = {self.VersionKey} if (versionIndices is None) else versionIndices
        self._versionIndices = newVersionIndices & uniqueIndices
        self.valueCol = self.ValueKey if (valueCol is None) else valueCol

        self._nonVersionIndexNames = list(filter(lambda ind: ind not in self._versionIndices, self._indices))
        self._versionIndexNames = list(filter(lambda ind: ind in self._versionIndices, self._indices))
        isVersionColumn = [(ind in self._versionIndices) for ind in self._indices]

        super().__init__(isVersionColumn, repo)

    @property
    def indices(self) -> List[str]:
        """
        The names of the index columns to query to retrive the main content of an asset

        :getter: Returns names of the index columns
        :type: List[:class:`str`]
        """

        return self._indices

    @property
    def versionIndices(self) -> Set[str]:
        """
        The names of the index columns that refer to some version

        :getter: Returns names of the index columns that refer to some version
        :type: Set[:class:`str`]
        """

        return self._versionIndices

    def get(self, nonVersionVals: Union[Hashable, List[Hashable], Dict[str, Hashable]],
            versionVals: Optional[Union[Hashable, List[Hashable], Dict[str, Hashable]]] = None,
            errorOnNotFound: bool = True, default: Any = None) -> T:
        """
        Retrieves the corresponding asset

        Parameters
        ----------
        nonVersionVals: Union[`Hashable`_, List[`Hashable`_], Dict[:class:`str`, `Hashable`_]]
            The values of the index columns that do not reference a version to query the specific asset

        versionVals: Optional[Union[`Hashable`_, List[`Hashable`_], Dict[:class:`str`, `Hashable`_]]]
            The values of the index columns that reference a version to query the specific asset :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                If the value for a particular version column is ``None``, then will get the latest version for that column

            **Default**: ``None``

        errorOnNotFound: :class:`bool`
            If no assets are found, whether to raise an exception :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if no assets are found :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Raises
        ------
        :class:`KeyError`
            If the corresponding asset based on the search parameters is not found and 'errorOnNotFound' is set to ``True``

        Returns
        -------
        T
            Either:

            * The found asset OR
            * The value specified from 'default' if 'errorOnNotFound' is set to ``False``
        """

        if (versionVals is None):
            versionVals = {}

        nv = BaseModAssets.toWildcardList(nonVersionVals, self._nonVersionIndexNames)
        vv = BaseModAssets.toWildcardList(versionVals, self._versionIndexNames)

        try:
            return super().get(nv, vv, errorOnNotFound = True)
        except KeyError as e:
            if (errorOnNotFound):
                raise e
            return default
##### EndScript

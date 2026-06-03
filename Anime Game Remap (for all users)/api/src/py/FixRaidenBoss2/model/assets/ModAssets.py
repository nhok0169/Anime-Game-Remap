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
from functools import lru_cache
from typing import Optional, Dict, Any, List, Union, Hashable, Set, Tuple
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import T, VersionType, PdDataFrame
from ...constants.Packages import PackageModules
from ...constants.GlobalPackageManager import GlobalPackageManager
from ...tools.DictTools import DictTools, UnHashableNone
from .BaseModAssets import BaseModAssets
from ..Version import Version
##### EndLocalImports


##### Script
class ModAssets(BaseModAssets[T]):
    """
    This class inherits from :class:`BaseModAssets`

    Class to handle assets of any type for a mod where retrieval is based on some keys where 1 or more of the keys refer to some versioning  :raw-html:`<br />` :raw-html:`<br />`

    .. tip::
        If the assets have more than 1 column that refers to some version, use this data structure. Otherwise if your asset has only 1 column that refers to some version, 
        it recommended to use :class:`ModDictAssets` instead since it uses a hash based access instead of looping through a `pandas DataFrame`_

    Parameters
    ----------
    repo: Union[`pd.DataFrame`_, Dict[`Hashable`_, Any]]
        The original source for the assets that is either a `pandas DataFrame`_ or a nested dictionary

        .. warning::
            See the warning at :attr:`repo`

    indices: Optional[List[:class:`str`]]
        The names of the index columns to query to retrive the main content of the asset :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will set 2 index column by the names "version" and "name" by default :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    versionIndices: Optional[Set[:class:`str`]]
        The names of the index columns that refer to some version :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will set an index to the name "version" by default :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    valueCol: Optional[:class:`str`]
        The name of column for the main content of the asset :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will set the name of the column to be "value"

        .. warning::
            Do not set the name for this argument to be the as some index name from :attr:`indices` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    valueCol: Optional[:class:`str`]
        The name of column for the main content of the asset :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will set the name of the column to be "value"

        .. warning::
            Do not set the name for this argument to be the as some index name from :attr:`indices`
    """

    def __init__(self, repo: Union[PdDataFrame, Dict[Hashable, Any]], indices: Optional[List[str]] = None, versionIndices: Optional[Set[str]] = None, valueCol: Optional[str] = None, **kwargs):
        self._versionIndices = set()
        self._nonVersionIndices = []
        self.valueCol = self.ValueKey if (valueCol is None) else valueCol
        self.indices = [self.VersionKey, self.NameKey] if (indices is None) else indices
        self.versionIndices = {self.VersionKey} if (versionIndices is None) else versionIndices

        self.repo = repo

    @property
    def repo(self) -> PdDataFrame:
        """
        The source data for the assets

        .. warning::
            If the argument entered in the setter is a `pandas DataFrame`_, then this attribute is a pointer to the argument (attribute uses the same memory as the argument)
            Otherwise, if the argument entered in the setter is a nested dictionary, then this attribute creates a brand new, converted data for the argument (attribute duplicates the memory of the argument) :raw-html:`<br />` :raw-html:`<br />`

            To save on space, consider :meth:`DicTools.nestedDictToDataFrame` to convert the nested dictionary into a `pandas DataFrame`_ first

        :getter: Returns the source for the data
        :setter: Sets the new source for the data
        :type: `pd.DataFrame`
        """

        return self._repo
    
    @repo.setter
    def repo(self, newRepo: Union[PdDataFrame, Dict[Hashable, Any]]):
        self._repo = DictTools.nestedDictToDataFrame(newRepo, self._indices + [self.valueCol]) if (isinstance(newRepo, dict)) else newRepo

        np = GlobalPackageManager.get(PackageModules.Numpy.value)
        convertVersion = np.vectorize(lambda version: Version.getVersion(version))
        
        for versionCol in self._versionIndices:
            self._repo[versionCol] = self._repo[versionCol].astype(object)
            self._repo[versionCol] = convertVersion(self._repo[versionCol])

    @property
    def indices(self) -> List[str]:
        """
        The names of the index columns to query to retrive the main content of an asset

        :getter: Returns names of the index columns
        :setter: Sets the names for the index columns
        :type: List[:class:`str`]
        """

        return self._indices

    def _setNonVersionIndices(self):
        self._nonVersionIndices = list(filter(lambda ind: ind not in self._versionIndices, self._indices))
    
    @indices.setter
    def indices(self, newIndices: List[str]):
        uniqueIndices = set(newIndices)
        if (len(uniqueIndices) != len(newIndices)):
            raise KeyError("Index names must be unique")
        
        self._indices = newIndices
        self._versionIndices &= uniqueIndices
        self._setNonVersionIndices()

    @property
    def versionIndices(self) -> Set[str]:
        """
        The names of the index columns that refer to some version

        :getter: Returns names of the index columns that refer to some version
        :setter: Sets the names of the index columns that refer to some version
        :type: Set[:class:`str`]
        """

        return self._versionIndices
    
    @versionIndices.setter
    def versionIndices(self, newVersionIndices: Set[str]) -> Set[str]:
        self._versionIndices = newVersionIndices & set(self.indices)
        self._setNonVersionIndices()

    def clearCache(self):
        self._get.cache_clear()
    
    def _convertNonVersionVals(self, indexVals: Union[Hashable, List[Hashable], Dict[str, Hashable]]) -> Dict[str, Hashable]:
        return self._convertIndexVals(indexVals, self._nonVersionIndices)
    
    def _convertVersionVals(self, indexVals: Union[float, str, VersionType, List[Union[float, str, VersionType]], Dict[str, Union[float, str, VersionType]]]):
        result = {}

        versionIndices = list(filter(lambda ind: ind in self.versionIndices, self._indices))
        result = self._convertIndexVals(indexVals, versionIndices)

        for key in result:
            version = result[key]
            result[key] = Version.getVersion(version) if (version != UnHashableNone) else version

        return result
    
    def _convertIndsToTuple(self, nonVersionVals: Union[Hashable, List[Hashable], Dict[str, Hashable]], versionVals: Union[float, str, VersionType, List[Union[float, str, VersionType]], Dict[str, Union[float, str, VersionType]]]) -> Tuple[Tuple[Hashable], Tuple[Union[float, str, VersionType]]]:
        nonVersionVals = self._convertNonVersionVals(nonVersionVals)
        versionVals = self._convertVersionVals(versionVals)
        return (tuple(nonVersionVals.values()), tuple(versionVals.values()))
    
    @lru_cache(maxsize = 1024)
    def _get(self, nonVersionVals: Tuple[Hashable], versionVals: Tuple[VersionType]) -> T:
        result = self._repo
        nonVersionIndsLen = len(self._nonVersionIndices)

        for i in range(nonVersionIndsLen):
            val = nonVersionVals[i]

            if (val == UnHashableNone):
                continue

            col = self._nonVersionIndices[i]
            result = result[result[col] == val]

            if (result.empty):
                raise KeyError(f"No results found for the following search --> nonVersionVals: {nonVersionVals}")
        
        versionIndices = list(filter(lambda ind: ind in self.versionIndices, self._indices))
        versionIndsLen = len(versionIndices)

        for i in range(versionIndsLen):
            col = versionIndices[i]
            val = versionVals[i]

            versions = result[col]
            version = None

            if (val != UnHashableNone and val != None):
                newVersions = versions[versions <= val]

                if (newVersions.empty):
                    version = versions[versions > val].min()
                else:
                    version = newVersions.max()

            else:
                version = versions.max()

            result = result[result[col] == version]

            if (result.empty):
                raise KeyError(f"No results found for the following search --> nonVersionVals: {nonVersionVals}, nonVersionVals: {nonVersionVals}")

        return result.iloc[0][self.valueCol]
        
    def get(self, nonVersionVals: Union[Hashable, List[Hashable], Dict[str, Hashable]], 
            versionVals: Union[Optional[Union[float, str, VersionType]], List[Optional[Union[float, str, VersionType]]], Dict[str, Optional[Union[float, str, VersionType]]]] = None, 
            errorOnNotFound: bool = True, default: Any = None) -> T:
        """
        Retrieves the corresponding asset

        Parameters
        ----------
        nonVersionVals: Union[Hashable, List[Hashable], Dict[:class:`str`, Hashable]]
            The values of the index columns that do not reference a version to query the specific asset

        versionVals: Union[Optional[Union[:class:`float`, :class:`str`, `packaging.version.Version`_]], List[Optional[Union[:class:`float`, :class:`str`, `packaging.version.Version`_]]], Dict[str, Optional[Union[:class:`float`, :class:`str`, `packaging.version.Version`_]]]]
            The values of the index columns that reference a version to query the specific asset :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                If the value for a particular version column is ``None``, then will get the latest version of the for that column

                eg. 
                if :attr:`indices` = `["gameVersion", "name", "charVersion"]` and :attr:`versionIndices` = `{"version"}`, then for the following argument in this parameter will get the latest character version for game version 4.2:
                
                ``[4.2, None]``

            :raw-html:`<br />` :raw-html:`<br />`

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

        nonVersionVals, versionVals = self._convertIndsToTuple(nonVersionVals, versionVals)

        try:
            result = self._get(nonVersionVals, versionVals)
        except KeyError as e:
            if (errorOnNotFound):
                raise e
            
            return default

        return result
##### EndScript
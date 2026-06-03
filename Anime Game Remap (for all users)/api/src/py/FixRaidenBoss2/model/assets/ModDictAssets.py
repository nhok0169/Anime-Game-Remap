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
from ...tools.DictTools import DictTools
from .BaseModAssets import BaseModAssets
from ..Version import Version
##### EndLocalImports


##### Script
class ModDictAssets(BaseModAssets):
    """
    This class inherits from :class:`BaseModAssets`

    Class to handle assets of any type for a mod where retrieval is based on some keys where only one of the keys refer to some versioning  :raw-html:`<br />` :raw-html:`<br />`

    Parameters
    ----------
    repo: Dict[`Hashable`_, T]
        The original nested dictionary source for the assets :raw-html:`<br />` :raw-html:`<br />`

        ``T`` is either:

        * A dictionary of type ``Dict[Hashable, T]``
        * A type for the content of the asset

    indices: Optional[List[:class:`str`]]
        The names of the index columns to query to retrive the main content of the asset :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will set 2 index column by the names "version" and "name" by default :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    versionIndex: Optional[:class:`str`]
        The name of the index column that refer to some version :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will set the index to the name "version" by default :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    valueCol: Optional[:class:`str`]
        The name of the column for the main content of the asset :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will set the name of the column to be "value"

        .. warning::
            Do not set the name for this argument to be the same as some index name from :attr:`indices` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    valueCol: Optional[:class:`str`]
        The name of column for the main content of the asset :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will set the name of the column to be "value"

        .. warning::
            Do not set the name for this argument to be the as some index name from :attr:`indices`
    """

    TempVersionCol = 0

    def __init__(self, repo: Dict[Hashable, T], indices: Optional[List[str]] = None, versionIndex: Optional[str] = None, valueCol: Optional[str] = None, **kwargs):
        self._repoIndices: PdDataFrame = None
        self._versionIndex = self.VersionKey if (versionIndex is None) else versionIndex
        self.indices = [self.VersionKey, self.NameKey] if (indices is None) else indices
        self.valueCol = self.ValueKey if (valueCol is None) else valueCol
        self._versionIndexPos = self._indices.index(self._versionIndex)

        self.repo = repo

    def clearCache(self):
        self._getVersion.cache_clear()
        self._get.cache_clear()

    def makeIndices(self):
        self._repoIndices = DictTools.nestedDictToDataFrame(self._repo, self._indices + [self.valueCol])
        print(self._repoIndices)

        self._repoIndices = self._repoIndices.drop(self.valueCol, axis = 1)

        np = GlobalPackageManager.get(PackageModules.Numpy.value)
        convertVersion = np.vectorize(lambda version: Version.getVersion(version))

        self._repoIndices[self.TempVersionCol] = self._repoIndices[self._versionIndex].copy()
        self._repoIndices[self._versionIndex] = self._repoIndices[self._versionIndex].astype(object)
        self._repoIndices[self._versionIndex] = convertVersion(self._repoIndices[self._versionIndex])
        self._repoIndices = self._repoIndices.sort_values(by = self._versionIndex)

    @property
    def repo(self) -> PdDataFrame:
        """
        The source data for the assets :raw-html:`<br />` :raw-html:`<br />`

        ``T`` is either:

        * A dictionary of type ``Dict[Hashable, T]``
        * A type for the content of the asset

        :getter: Returns the source for the data
        :setter: Sets the new source for the data
        :type: Dict[`Hashable`_, T]
        """

        return self._repo
    
    @repo.setter
    def repo(self, newRepo: Dict[Hashable, T]):
        self._repo = newRepo
        self.makeIndices()

    def _checkIndicesHaveVersionIndex(self, indices: Set[str], versionIndex: str):
        if (versionIndex not in indices):
            raise KeyError(f"The following version index, {versionIndex} is not in the list of indices: {indices}")

    @property
    def indices(self) -> List[str]:
        """
        The names of the index columns to query to retrive the main content of an asset

        :getter: Returns names of the index columns
        :setter: Sets the names for the index columns
        :type: List[:class:`str`]
        """

        return self._indices
    
    @indices.setter
    def indices(self, newIndices: List[str]):
        uniqueIndices = set(newIndices)
        if (len(uniqueIndices) != len(newIndices)):
            raise KeyError("Index names must be unique")
        
        self._checkIndicesHaveVersionIndex(uniqueIndices, self._versionIndex)
        self._indices = newIndices

    @property
    def versionIndex(self) -> Set[str]:
        """
        The name of the index column that refer to some version

        :getter: Returns the name of the index column that refer to some version
        :setter: Sets the new name of the index column that refer to some version
        :type: :class:`str`
        """

        return self._versionIndex
    
    @versionIndex.setter
    def versionIndex(self, newVersionIndex: Optional[str]) -> Set[str]:
        if (newVersionIndex is None):
            newVersionIndex = self.VersionKey

        uniqueIndices = set(self._indices)
        self._checkIndicesHaveVersionIndex(uniqueIndices, newVersionIndex)
        self._versionIndex = newVersionIndex
        self._versionIndexPos = self._indices.index(self._versionIndex)

    @property
    def versionIndexPos(self):
        """
        The index of where the version index is located in :attr:`indices`

        :getter: Retrieves the index for the version index
        :type: :class:`int`
        """

        return self._versionIndexPos

    @lru_cache(maxsize = 1024)
    def _getVersion(self, nonVersionVals: Tuple[Hashable], version: Optional[VersionType]) -> Union[str, float, VersionType]:
        filterDataFrame = self._repoIndices

        nonVersionValsLen = len(nonVersionVals)
        indicesLen = len(self._indices)
        nonVersionPos = 0
        indicesPos = 0

        while (nonVersionPos < nonVersionValsLen and indicesPos < indicesLen):
            if (indicesPos == self._versionIndexPos):
                indicesPos += 1
                continue
            
            indexCol = self._indices[indicesPos]
            indexVal = nonVersionVals[nonVersionPos]
            filterDataFrame = filterDataFrame[filterDataFrame[indexCol] == indexVal]

            if (filterDataFrame.empty):
                raise KeyError(f"No results found for the following search --> nonVersionVals: {nonVersionVals}")
            
            indicesPos += 1
            nonVersionPos += 1

        versions = filterDataFrame[self._versionIndex]
        if (version is None):
            maxVersionInd = versions.idxmax()
            return filterDataFrame.loc[maxVersionInd][self.TempVersionCol]

        versionPos = versions.searchsorted(version)
        if (versionPos > 0):
            versionPos -= 1

        return filterDataFrame.iloc[versionPos][self.TempVersionCol]
    
    def _updateAssetContent(self, srcAsset: T, newAsset: T) -> T:
        """
        Combines the content of 2 assets

        Parameters
        ----------
        srcAsset: T
            The content of the asset to be updates

        newAsset: T
            The new asset to update into ``srcAsset`` 

        Returns
        -------
        T
            The updated asset
        """

        return newAsset
    
    def _updateDupAssets(self, depth: int, srcAsset: Dict[str, Any], newAsset: Dict[str, Any]):
        if (depth >= len(self.indices)):
            return self._updateAssetContent(srcAsset, newAsset)

        return DictTools.update(srcAsset, newAsset, combineDuplicate = lambda assetId, srcVal, newVal: self._updateDupAssets(depth + 1, srcVal, newVal))

    def addRepo(self, newRepo: Dict[Hashable, T]) -> Dict[Hashable, T]:
        """
        Adds new data into the existing source data at :attr:`repo`

        Parameters
        ----------
        newRepo: Dict[`Hashable`_, T]
            The new data to add

        Returns
        -------
        Dict[`Hashable`_, T]
            The new source data with the newly added data
        """

        DictTools.update(self._repo, newRepo, combineDuplicate = lambda version, srcVal, newVal: self._updateDupAssets(1, srcVal, newVal))
        self.makeIndices()
        return self._repo
    
    @lru_cache(maxsize = 128)
    def _get(self, nonVersionVals: Tuple[Hashable], version: Optional[Union[str, float, VersionType]]) -> T:
        nonVersionValsLen = len(nonVersionVals)
        indicesLen = len(self._indices)
        nonVersionPos = 0
        indicesPos = 0
        result = self._repo

        while (nonVersionPos < nonVersionValsLen and indicesPos < indicesLen):
            isVersionInd = indicesPos == self._versionIndexPos
            indexVal = nonVersionVals[nonVersionPos] if (not isVersionInd) else version
            result = result[indexVal]

            indicesPos += 1
            if (not isVersionInd):
                nonVersionPos += 1

        return result

    def get(self, nonVersionVals: Union[Hashable, List[Hashable]], version: Optional[Union[str, float, VersionType]] = None, errorOnNotFound: bool = True, default: Any = None) -> T:
        """
        Retrieves the corresponding asset

        Parameters
        ----------
        nonVersionVals: Union[Hashable, List[Hashable], Dict[:class:`str`, Hashable]]
            The values of the index columns that do not reference a version to query the specific asset

        version: Union[:class:`float`, :class:`str`, `packaging.version.Version`_]
            The specific version to query the asset :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                If the value for the version is ``None``, then will latest version for the asset

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

        result = None
        nonVersionValsLen = len(nonVersionVals)
        nonVersionVals = tuple(nonVersionVals)

        if (version is not None):
            version = Version.getVersion(version)

        try:
            version = self._getVersion(nonVersionVals, version) if (nonVersionValsLen >= self._versionIndexPos) else None
            result = self._get(nonVersionVals, version)
        except Exception as e:
            if (errorOnNotFound):
                raise e
            
            return default
        
        return result
##### EndScript
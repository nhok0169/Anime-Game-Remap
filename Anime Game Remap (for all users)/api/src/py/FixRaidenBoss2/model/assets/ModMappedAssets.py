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
import copy
from functools import lru_cache
from collections import deque, defaultdict
from typing import Dict, Optional, Set, Tuple, List, Hashable, Union, Any
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import T, OrderedSetType, VersionType
from ...constants.GlobalPackageManager import GlobalPackageManager
from ...constants.Packages import PackageModules
from .ModDictAssets import ModDictAssets
from ..Version import Version
from .BaseModAssets import BaseModAssets
from ...tools.DictTools import DictTools, UnHashableNone
##### EndLocalImports


##### Script
class ModMappedAssets(BaseModAssets[T]):
    """
    This class inherits from :class:`BaseModAssets`

    Class to handle assets of any type where asset retrieval is based on a mapping :raw-html:`<br />` :raw-html:`<br />`

    .. note::
        This is a `bipartite graph`_ that maps assets to fix from to assets to fix to

    Parameters
    ----------
    repo: :class:`ModDictAssets`
        The original source for any preset assets :raw-html:`<br />` :raw-html:`<br />`

        .. warning::
            We require this argument to have the following requirements:

            * :attr:`ModDictAssets.indices` must have a size of at least 2, with the first non-version index being called ``name`` and the version index being called ``version``
            * The main content of the assets must be `Hashable`_

    map: Optional[Dict[`Hashable`_, `OrderedSet`_[`Hashable`_]]]
        The `adjacency list`_  that maps the assets to fix from to the assets to fix to using the predefined mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    VersionKey = "version"
    NameKey = "name"

    def __init__(self, repo: ModDictAssets, map: Optional[Dict[str, OrderedSetType[str]]] = None, **kwargs):
        self.repo = repo

        self._fixFrom: Set[str] = set()
        self._fixTo: Set[str] = set()

        self._keys: Dict[Hashable, Dict[float, List[Dict[str, Hashable]]]] = {}
        self._keyVersions: Dict[Hashable, Version] = {}

        self.map = {} if (map is None) else map

    @property
    def fixFrom(self) -> Set[str]:
        """
        The names of the assets to fix from using the predefined mods

        :getter: Retrieves the names of assets used to fix from
        :type: Set[:class:`str`]
        """
        
        return self._fixFrom

    @property
    def fixTo(self) -> Set[str]:
        """
        The names of the assets to fix to using the predefined mods

        :getter: Retrives the names of assets to fix to
        :type: Set[:class:`str`]
        """

        return self._fixTo
    
    def updateKeys(self):
        self._keys = defaultdict(lambda: defaultdict(lambda: []))
        self._keyVersions = {}

        addState = 0
        removeState = 1
        repo = self.repo.repo
        stack = deque()

        for key in repo:
            stack.appendleft((key, repo[key], 0, addState))

        indexNames = self.repo.indices
        maxDepth = len(self.repo.indices) - 1
        versionIndexPos = self.repo.versionIndexPos
        key = {}
        version = None

        while (stack):
            index, val, depth, state = stack.pop()
            indexName = indexNames[depth]

            if (state == addState):
                keyAdded = True

                if (depth == versionIndexPos):
                    version = index
                elif (key or index in self._map):
                    key[indexName] = index
                else:
                    keyAdded = False

                if (keyAdded):
                    stack.append((index, val, depth, removeState))

                if (depth < maxDepth):
                    for childKey in val:
                        stack.append((childKey, val[childKey], depth + 1, addState))

                    continue

                self._keys[val][version].append(copy.deepcopy(key))

                versioning = self._keyVersions.get(val)
                if (versioning is None):
                    versioning = Version()
                    self._keyVersions[val] = versioning

                versioning.add(version)
            else:
                if (depth == versionIndexPos):
                    version = None
                else:
                    key.pop(indexName, None)

        pd = GlobalPackageManager.get(PackageModules.Pandas.value)
        for asset in self._keys:
            assetData = self._keys[asset]

            for version in assetData:
                assetData[version] = pd.DataFrame(assetData[version])

            self._keys[asset] = dict(assetData)

        self._keys = dict(self._keys)

    def clearCache(self):
        self.repo.clearCache()
        self.getKey.cache_clear()

    def _convertNonVersionVals(self, indexVals: Union[Hashable, List[Hashable], Dict[str, Hashable]]) -> Tuple[Hashable]:
        nonVersionIndices = list(filter(lambda index: index != self.repo.versionIndex, self.repo.indices))
        result = self._convertIndexVals(indexVals, nonVersionIndices)
        return tuple(result.values())
    
    @property
    def map(self) -> Dict[str, Set[str]]:
        """
        The `adjacency list`_ used to map assets to fix from to assets to fix to

        :getter: Retrieves the `adjacency list`_
        :setter: Sets a new `adjacency list`_
        :type: Dict[`Hashable`_, `OrderedSet`_[`Hashable`_]]
        """

        return self._map
    
    @map.setter
    def map(self, newMap: Dict[str, Set[str]]):
        self._map = newMap
        self.updateKeys()

    @property
    def fromAssets(self) -> List[Hashable]:
        """
        The assets to map from

        :getter: Retrives the main content of all the assets to map from
        :type: List[`Hashable`_]
        """

        return list(self._keys.keys())
    
    def addRepo(self, newRepo: Dict[Hashable, T]) -> Dict[Hashable, T]:
        """
        Adds new data into the existing source data of :attr:`repo`

        Parameters
        ----------
        newRepo: Dict[`Hashable`_, T]
            The new data to add

        Returns
        -------
        Dict[`Hashable`_, T]
            The new source data with the newly added data
        """

        return self.repo.addRepo(newRepo)
    
    def addMap(self, assetMap: Dict[Hashable, OrderedSetType[Hashable]], assets: Optional[Dict[Hashable, T]] = None):
        """
        Adds a new map to the existing map on how to retrieve the assets

        Parameters
        ----------
        assetMap: Dict[`Hashable`_, `OrderedSet`_[`Hashable`_]]
            The new `adjacency list`_ to add that maps assets to fix from to assets to fix to

        assets: Optional[Dict[`Hashable`_, T]]
            Any new assets that needs to be added/updated to the existing assets to support the given map :raw-html:`<br />` :raw-html:`<br />`s

            **Default**: ``None``
        """

        if (assets is not None):
            self.addRepo(assets)

        DictTools.update(self._map, assetMap, lambda key, srcFixTo, destFixTo: srcFixTo.union(destFixTo))
        self.updateKeys()
    
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

        return self.repo.get(nonVersionVals, version = version, errorOnNotFound = errorOnNotFound, default = default)
    
    def _getByVersion(self, dct: Dict[Union[str, float, VersionType], T], version: VersionType) -> T:
        if (version in dct):
            return dct[version]
        
        version = str(version)
        if (version in dct):
            return dct[version]
        
        try:
            version = float(version)
        except Exception as e:
            raise KeyError(f"{version}") from e
        else:
            return dct[version]
    
    @lru_cache(maxsize = 1024)
    def getKey(self, asset: Hashable, fromVersion: Optional[Union[str, float, VersionType]], fromNonVersionVals: Tuple[Hashable]) -> Tuple[Hashable]:
        fromVersioning = self._keyVersions.get(asset)
        if (fromVersioning is None):
            raise KeyError(f"No keys found for the asset, {asset}")
        
        fromVersion = fromVersioning.findClosest(fromVersion)

        keys = self._keys[asset]
        keys = self._getByVersion(keys, fromVersion)

        indices = list(filter(lambda index: index != self.repo.versionIndex, self.repo.indices))
        indicesLen = len(indices)

        for i in range(indicesLen):
            key = indices[i]
            val = fromNonVersionVals[i]

            if (val == UnHashableNone):
                continue

            keys = keys[keys[key] == val]
            if (keys.empty):
                raise KeyError(f"No keys found for the asset, {asset} and the following filter indices, {fromNonVersionVals}")
            
        return tuple(keys.iloc[0])
    
    def _getToAssetNames(self, fromAsset: Hashable, toAssets: Optional[Union[Hashable, List[Hashable], Set[Hashable], OrderedSetType[Hashable]]] = None) -> OrderedSetType:
        mappedToAssets = self._map.get(fromAsset)
        if (mappedToAssets is None):
            raise KeyError("Asset to map from not found")
        
        if (toAssets is None):
            return mappedToAssets
        
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        if (isinstance(toAssets, list)):
            return OrderedSet(filter(lambda lstItem: lstItem in mappedToAssets, toAssets))
        elif (isinstance(toAssets, set) or isinstance(toAssets, OrderedSet)):
            return mappedToAssets & toAssets

        return mappedToAssets & {toAssets}

    def replace(self, asset: Hashable, fromVersion: Optional[Union[str, float, VersionType]] = None, fromNonVersionVals: Optional[Union[Hashable, List[Hashable], Dict[str, Hashable]]] = None, 
                toVersion: Optional[float] = None, toAssetNames: Optional[Union[Hashable, List[Hashable], Set[Hashable], OrderedSetType[Hashable]]] = None, 
                errorOnNotFound: bool = True, default: Any = None) -> Union[Optional[Hashable], Dict[Hashable, Hashable], Any]:
        """
        Retrieves the corresponding asset to replace 'asset'

        Parameters
        ----------
        asset: `Hashable`_
            The asset to be replaced

        fromVersion: Optional[Union[:class:`float`, :class:`str`, `packaging.version.Version`_]]
            The version we want the asset to come from :raw-html:`<br />` :raw-html:`<br />`

            If This value is ``None``, then will assume to replace the latest version of the argument ``asset``. :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        fromNonVersionVals: Optional[Union[`Hashable`_, List[`Hashable`_], Dict[:class:`str`, `Hashable`_]]]
            The values to the non-version indices used to help filter for a particular instance of the asset to replace. :raw-html:`<br />` :raw-html:`<br />`

            This parameter is helpful if the data in :attr:`repo` contains many different instances of assets that have the same value

        toVersion: Optional[Union[:class:`float`, :class:`str`, `packaging.version.Version`_]]
            The version we want the replacement asset to come from :raw-html:`<br />` :raw-html:`<br />`

            If This value is ``None``, then will assume we want the latest version of the corresponding replacement. :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        toAssetNames: Optional[Union[`Hashable`_, List[`Hashable`_], Set[`Hashable`_], `OrderedSet`_[`Hashable`_]]]
            The names of the assets to map to for replacement :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        errorOnNotFound: :class:`bool`  
            If no assets are found, whether to raise an exception :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if no assets are found :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Union[Optional[`Hashable`_], Dict[`Hashable`_, `Hashable`_]]
            The corresponding assets for the fix to replace, if available :raw-html:`<br />` :raw-html:`<br />`

            The result contains the main content of the asset if the passed in parameter 'toAssetNames' is a `Hashable`_ that has the same type as
            the other assets :raw-html:`<br />` :raw-html:`<br />`
            
            Otherwise, the result is a dictionary such that: :raw-html:`<br />` :raw-html:`<br />`

            * The keys are the names of the assets to map to
            * The values are the corresponding assets used for replacement

            :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        fromNonVersionVals = self._convertNonVersionVals(fromNonVersionVals)

        key = None
        try:
            key = self.getKey(asset, fromVersion, fromNonVersionVals)
        except KeyError as e:
            if (errorOnNotFound):
                raise e
            return default
        
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet
        isMultiResult = (isinstance(toAssetNames, list) or isinstance(toAssetNames, set) or isinstance(toAssetNames, OrderedSet) or toAssetNames is None)

        fromAsset = key[0]
        toAssetNames = self._getToAssetNames(fromAsset, toAssets = toAssetNames)

        if (not toAssetNames):
            return {} if (isMultiResult) else None
        
        # get all the mapped assets that use the same key
        result = {}
        currentKey = list(key)

        for toAsset in toAssetNames:
            currentKey[0] = toAsset
            currentResult = self.repo.get(currentKey, version = toVersion, errorOnNotFound = False, default = [])

            if (isinstance(currentResult, list)):
                continue

            result[toAsset] = currentResult

        if (not result):
            return {} if (isMultiResult) else None
        
        if (isMultiResult):
            return result
        return DictTools.getFirstValue(result)
##### EndScript
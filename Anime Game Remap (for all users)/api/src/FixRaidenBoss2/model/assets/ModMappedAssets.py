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
from typing import Dict, Optional, Set, Tuple, List, Hashable, Union, Any
from collections import deque
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import T, OrderedSetType, VersionType
from ...constants.GlobalPackageManager import GlobalPackageManager
from ...constants.Packages import PackageModules
from ..Version import Version
from .ModDictAssets import ModDictAssets
from ...tools.DictTools import DictTools
##### EndLocalImports


##### Script
class ModMappedAssets(ModDictAssets[T]):
    """
    This class inherits from :class:`ModDictAssets`

    Class to handle assets of any type where asset retrieval is based on a mapping

    .. note::
        This is a `bipartite graph`_ that maps assets to fix from to assets to fix to

    Parameters
    ----------
    repo: Dict[:class:`float`, Dict[`Hashable`_, T]]
        The original source for any preset assets :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version number for the assets
        * The inner key is the name of the asset
        * The inner value is the content for the asset :raw-html:`<br />` :raw-html:`<br />`

        .. note::
            The type ``T`` may be either a:

            * Nested dictionary of the form ``Dict[Hashable, T]``
            * A `Hashable`_ type representing the main content of the asset


    indices: Optional[List[:class:`str`]]
        The names of additional index columns to query to retrive the main content of the asset apart from the name of the asset :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    map: Optional[Dict[`Hashable`_, `OrderedSet`_[`Hashable`_]]]
        The `adjacency list`_  that maps the assets to fix from to the assets to fix to using the predefined mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, repo: Dict[float, Dict[Hashable, T]], indices: Optional[List[str]] = None, map: Optional[Dict[str, OrderedSetType[str]]] = None, **kwargs):
        if (indices is None):
            indices = []

        super().__init__(repo, indices = [self.NameKey] + indices, setVersions = False, **kwargs)

        self._fixFrom: Set[str] = set()
        self._fixTo: Set[str] = set()
        self._map = {} if (map is None) else map

        self._keys: Dict[Hashable, Dict[float, List[Dict[str, Hashable]]]] = {}
        self._fromVersions: Dict[Hashable, Version] = {}

        self.load()

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
        self.clear(flush = True, clearMap = True)
        self.addMap(newMap)

    @property
    def fromAssets(self) -> List[Hashable]:
        """
        The assets to map from

        :getter: Retrives the main content of all the assets to map from
        :type: List[`Hashable`_]
        """

        return list(self._keys.keys())

    def clear(self, flush: bool = True, clearMap: bool = False):
        """
        Clears all the assets

        Parameters
        ----------
        flush: :class:`bool`
            Whether to flush out (reload) any cached data
            
            **Default**: ``False``

        clearMap: :class:`bool`
            Whether to clear out the mapping for the assets 

            **Default**: ``False``
        """

        if (flush):
            self.clearCache()

        if (clearMap):
            self._fixFrom = set()
            self._fixTo = set()
            self._map = {}

    def load(self):
        """
        Reinitializes any necessary setup
        """

        map = self._map
        self.clear(flush = True, clearMap = True)
        self.addMap(map, assets = self._repo)

    @classmethod
    def updateMap(cls, srcMap: Dict[Hashable, OrderedSetType[Hashable]], newMap: Dict[Hashable, OrderedSetType[Hashable]]) -> Dict[Hashable, OrderedSetType[Hashable]]:
        """
        Combines 2 maps together

        Parameters
        ----------
        srcMap: Dict[`Hashable`_, `OrderedSet`_[`Hashable`_]]
            The map to be updates

        newMap: Dict[`Hashable`_, `OrderedSet`_[`Hashable`_]]
            The new map to update ``srcMap``

        Returns
        -------
        Dict[`Hashable`_, `OrderedSet`_[`Hashable`_]]
            The updated map
        """

        return DictTools.update(srcMap, newMap, combineDuplicate = lambda assetId, oldToAssets, newToAssets: oldToAssets.union(newToAssets))
        
    def _partition(self, map: Dict[Hashable, OrderedSetType[str]], assets: Dict[float, Dict[Hashable, T]]) -> Tuple[Dict[Hashable, OrderedSetType[Hashable]], Set[Hashable], Set[Hashable]]:
        """
        * Creates the `bipartition`_ for the assets to fix from vs the assets to fix to
        * Filters the mapping such that all the asset names in the new mapping exist in `assets`

        Parameters
        ----------
        map: Dict[`Hashable`_, `OrderedSet`_[:class:`str`]]
            The desired mapping for the assets for fixing

        assets: Dict[:class:`float`, Dict[`Hashable`_, T]]
            The source for all the assets :raw-html:`<br />` :raw-html:`<br />`

            * The outer key is the game version number for the assets
            * The inner key is the name of the asset
            * The inner value is the content for the asset

            .. note::
                The type ``T`` may be either a:

                * Nested dictionary of the form ``Dict[Hashable, T]``
                * A `Hashable`_ type representing the main content of the asset

        Returns
        -------
        Tuple[Dict[`Hashable`_, `OrderedSet`_[`Hashable`_]], Set[`Hashable`_], Set[`Hashable`_]]
            The following output is in the same order as listed below: :raw-html:`<br />` :raw-html:`<br />`

            #. The new mapping with all asset names being in `assets`
            #. The names of the assets to fix from
            #. The names of the assets to fix to
        """

        newMap = {}
        fixFrom = set()
        fixTo = set()

        vertices = set()
        visited = {}

        # retrieve all the vertices in the map
        for fromAsset in map:
            vertices.add(fromAsset)
            currentToAssets = map[fromAsset]
            vertices = vertices.union(currentToAssets)

        visited = {}
        for vertex in vertices:
            visited[vertex] = False

        # get all the vertices in the map that are visited in the assets repo
        for version in assets:
            versionAssets = assets[version]

            for assetName in versionAssets:
                if (assetName in vertices and not visited[assetName]):
                    visited[assetName] = True

        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        # creates the new sub-map and bipartition with vertices definitely being in the assets repo
        for fromAsset in map:
            if (not visited[fromAsset]):
                continue
            
            currentToAssets = map[fromAsset]
            newCurrentToAssets = OrderedSet(filter(lambda toAsset: visited[toAsset], currentToAssets))

            if (not newCurrentToAssets):
                continue

            newMap[fromAsset] = newCurrentToAssets
            fixFrom.add(fromAsset)
            fixTo = fixTo.union(newCurrentToAssets)

        return (newMap, fixFrom, fixTo)
    
    def _addFromVersion(self, asset: Hashable, version: Union[str, float, VersionType]):
        fromVersion = self._fromVersions.get(asset, [])

        if (isinstance(fromVersion, list)):
            fromVersion = Version()
            self._fromVersions[asset] = fromVersion

        fromVersion.add(version)
    
    def _updateVersions(self, assets: Dict[float, Dict[str, T]]):
        indicesLen = len(self.indices)
        
        for version in assets:

            # update the versions to fix to
            stack = deque([([], assets[version])])
            while (stack):
                indexVals, currentAssets = stack.pop()
                indexValsLen = len(indexVals)

                if (indexValsLen >= indicesLen):
                    self._addVersion(indexVals, version)
                    continue

                for indexVal in currentAssets:
                    if (indexValsLen > 0 or (indexValsLen == 0 and indexVal in self._fixTo)):
                        stack.append((indexVals + [indexVal], currentAssets[indexVal]))

            # update the versions to fix from
            stack = deque([([], assets[version])])
            while (stack):
                indexVals, currentAssets = stack.pop()
                indexValsLen = len(indexVals)

                if (indexValsLen >= indicesLen):
                    self._addFromVersion(currentAssets, version)
                    continue

                for indexVal in currentAssets:
                    if (indexValsLen > 0 or (indexValsLen == 0 and indexVal in self._fixFrom)):
                        stack.append((indexVals + [indexVal], currentAssets[indexVal]))
    
    def _updateKey(self, key: Hashable, indexVals: List[Hashable], version: Union[str, float, VersionType]):
        initialIndexVals = [key, version]
        initialIndexValsLen = len(initialIndexVals)
        versionKeys = None
        prevVersionKeys = self._keys

        for i in range(initialIndexValsLen):
            indexVal = initialIndexVals[i]
            versionKeys = prevVersionKeys.get(indexVal)

            if (versionKeys is None):
                versionKeys = {} if (i < initialIndexValsLen - 1) else []
                prevVersionKeys[indexVal] = versionKeys

            prevVersionKeys = versionKeys

        indexVals = self._convertIndexVals(indexVals)
        versionKeys.append(indexVals)
    
    def _updateKeys(self, assets: Dict[float, Dict[str, T]]):
        indicesLen = len(self.indices)

        for version in assets:
            stack = deque([([], assets[version])])

            while (stack):
                indexVals, currentAssets = stack.pop()
                indexValsLen = len(indexVals)

                if (indexValsLen >= indicesLen):
                    self._updateKey(currentAssets, indexVals, version)
                    continue

                for indexVal in currentAssets:
                    if (indexValsLen > 0 or (indexValsLen == 0 and indexVal in self._fixFrom)):
                        stack.append((indexVals + [indexVal], currentAssets[indexVal]))
    
    def addMap(self, assetMap: Dict[Hashable, OrderedSetType[Hashable]], assets: Optional[Dict[float, Dict[Hashable, T]]] = None):
        """
        Adds a new map to the existing map on how to retrieve the assets

        Parameters
        ----------
        assetMap: Dict[`Hashable`_, `OrderedSet`_[`Hashable`_]]
            The new `adjacency list`_ used to map assets to fix from to assets to fix to

        assets: Optional[Dict[:class:`float`, Dict[`Hashable`_, T]]]
            Any new assets that needs to be added/updated to the existing assets to support the given map :raw-html:`<br />` :raw-html:`<br />`

            * The outer key is the game version number for the assets
            * The inner key is the name of the asset
            * The inner value is the content for the asset

            .. note::
                The type ``T`` may be either a:

                * Nested dictionary of the form ``Dict[Hashable, T]``
                * A `Hashable`_ type representing the main content of the asset

            :raw-html:`<br />`

            **Default**: ``None``
        """

        if (assets is None):
            assets = {}

        self._repo = self.updateRepo(self._repo, assets, updateVersions = False)
        newAddMap, addFixFrom, addFixTo = self._partition(assetMap, self._repo)

        if (not addFixFrom or not addFixTo):
            return

        self._map = self.updateMap(self._map, newAddMap)
        self._fixFrom = self._fixFrom.union(addFixFrom)
        self._fixTo = self._fixTo.union(addFixTo)

        # update the versions and keys
        self._updateVersions(assets)
        self._updateKeys(assets)

    def findClosestFromVersion(self, asset: Hashable, version: Optional[Union[str, float, VersionType]] = None, fromCache: bool = True) -> VersionType:
        """
        Finds the closest available game version for a particular asset that belongs to the assets to map from

        Parameters
        ----------
        asset: `Hashable`_
            The asset to seach the version for

        version: Optional[Union[:class:`str`, :class:`float`, `packaging.version.Version`_]]
            The game version to be searched :raw-html:`<br />` :raw-html:`<br />`

            If This value is ``None``, then will assume we want the latest version :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        fromCache: :class:`bool`
            Whether to use the result from the cache

            **Default**: ``None``

        Raises
        ------
        :class:`KeyError`
            The name for the particular asset is not found

        Returns
        -------
        `packaging.version.Version`_
            The latest game version from the assets that corresponds to the desired version 
        """

        versionCache = self._fromVersions.get(asset)
        if (versionCache is None):
            raise KeyError("Asset to map from not found")

        result = versionCache.findClosest(version, fromCache = fromCache)
        if (result is None):
            KeyError("No available versions for the asset to map from")

        return result
    
    def _keyInFilter(self, key: Dict[str, Hashable], filterIndices: Optional[Dict[str, Hashable]] = None):
        if (filterIndices is None):
            return True
        
        for index in filterIndices:
            if (index not in key):
                return False
            
            filterVal = filterIndices[index]
            keyVal = key[index]

            if (filterVal != keyVal):
                return False

        return True
    
    def _getToAssets(self, fromAsset: Hashable, toAssets: Optional[Union[Hashable, List[Hashable], Set[Hashable], OrderedSetType[Hashable]]] = None) -> OrderedSetType:
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

    def replace(self, asset: Hashable, version: Optional[float] = None, filterIndices: Optional[Union[Hashable, List[Hashable], Dict[str, Hashable]]] = None, 
                toAssets: Optional[Union[Hashable, List[Hashable], Set[Hashable], OrderedSetType[Hashable]]] = None, 
                errorOnNotFound: bool = True, default: Any = None) -> Union[Optional[Hashable], Dict[Hashable, Hashable], Any]:
        """
        Retrieves the corresponding asset to replace 'asset'

        Parameters
        ----------
        asset: `Hashable`_
            The asset to be replaced

        version: Optional[:class:`float`]
            The game version we want the asset to come from :raw-html:`<br />` :raw-html:`<br />`

            If This value is ``None``, then will retrieve the asset of the latest version. :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        filterIndices: Optional[Union[`Hashable`_, List[`Hashable`_], Dict[:class:`str`, `Hashable`_]]]
            The index values used to help filter for a particular instance of an asset. :raw-html:`<br />` :raw-html:`<br />`

            This parameter is helpful if the data in :attr:`repo` contains many different instances of assets that have the same value

        toAssets: Optional[Union[`Hashable`_, List[`Hashable`_], Set[`Hashable`_], `OrderedSet`_[`Hashable`_]]]
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

            The result contains the main content of the asset if the passed in parameter 'toAssets' is a `Hashable`_ that has the same type as
            the other assets :raw-html:`<br />` :raw-html:`<br />`
            
            Otherwise, the result is a dictionary such that: :raw-html:`<br />` :raw-html:`<br />`

            * The keys are the names of the assets
            * The values are the corresponding assets used for replacement

            :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        if (filterIndices is not None):
            filterIndices = self._convertIndexVals(filterIndices)

        # retrieve the corresponding key for the asset
        closestVersion = None
        try:
            closestVersion = self.findClosestFromVersion(asset, version = version)
        except KeyError as e:
            if (errorOnNotFound):
                raise e
            return default
        
        keys = self._getVersionAssets(closestVersion, self._keys[asset])
        keys = list(filter(lambda key: self._keyInFilter(key, filterIndices = filterIndices), keys))

        if (not keys and errorOnNotFound):
            raise KeyError("Asset to map from not found after filter")
        elif (not keys):
            return default
        
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet
        isMultiResult = (isinstance(toAssets, list) or isinstance(toAssets, set) or isinstance(toAssets, OrderedSet) or toAssets is None)

        key = keys[0]
        fromAsset = key[self.NameKey]
        toAssets = self._getToAssets(fromAsset, toAssets = toAssets)

        if (not toAssets):
            return {} if (isMultiResult) else None
        
        # get all the mapped assets that use the same key
        result = {}
        currentKey = copy.deepcopy(key)

        for toAsset in toAssets:
            currentKey[self.NameKey] = toAsset
            currentResult = self.get(currentKey, version = version, errorOnNotFound = False, default = [])
            if (isinstance(currentResult, list)):
                continue

            result[toAsset] = currentResult

        if (not result):
            return {} if (isMultiResult) else None
        
        if (isMultiResult):
            return result
        return DictTools.getFirstValue(result)
##### EndScript
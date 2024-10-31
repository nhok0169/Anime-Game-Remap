##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits


##### ExtImports
from typing import Generic, Dict, Optional, Set, Any, Tuple
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import T
from ..Version import Version
from ...tools.DictTools import DictTools
##### EndLocalImports


##### Script
class ModAssets(Generic[T]):
    """
    Class to handle assets of any type for a mod

    .. note::
        This is a `bipartite graph`_ that maps assets to fix from to assets to fix to

    Parameters
    ----------
    repo: Dict[:class:`str`, Dict[:class:`str`, T]]
        The original source for any preset assets :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version number for the assets
        * The inner key is the name of the asset
        * The inner value is the content for the asset

    map: Optional[Dict[:class:`str`, Set[:class:`str`]]]
        The `adjacency list`_  that maps the assets to fix from to the assets to fix to using the predefined mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    repo: Dict[:class:`float`, Dict[:class:`str`, T]]
        The original source for any preset assets :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version number for the assets
        * The inner key is the name of the asset
        * The inner value is the content for the asset
    """

    def __init__(self, repo: Dict[float, Dict[str, T]], map: Optional[Dict[str, Set[str]]] = None):
        self._repo = repo
        self._fixFrom: Set[str] = set()
        self._fixTo: Set[str] = set()
        self._map = map

        if (self._map is None):
            self._map = {}  

        if (self._fixFrom is None):
            self._fixFrom = set()

        if (self._fixTo is None):
            self._fixTo = set()

        self._versions: Dict[str, Version] = {}

    @property
    def versions(self) -> Dict[str, Version]:
        """
        The game versions available for the assets :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the names of the assets
        * The values are versions for the asset

        :getter: Returns all the available game versions for the assets
        :type: Dict[:class:`str`, :class:`Version`]
        """

        return self._versions
    
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
        :type: Dict[:class:`str`, Set[:class:`str`]]
        """

        return self._map
    
    @map.setter
    def map(self, newMap: Dict[str, Set[str]]):
        self.clear(flush = True, clearMap = True)
        self.addMap(newMap)

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
            self._versions.clear()

        if (clearMap):
            self._fixFrom = set()
            self._fixTo = set()
            self._map = {}

    def loadFromPreset(self):
        """
        Reinitializes to load the predefined mods
        """

        map = self._map
        self.clear(clearMap = True)
        self.map = map

    @classmethod
    def updateMap(cls, srcMap: Dict[str, Set[str]], newMap: Dict[str, Set[str]]) -> Dict[str, Set[str]]:
        """
        Combines 2 maps together

        Parameters
        ----------
        srcMap: Dict[:class:`str`, Set[:class:`str`]]
            The map to be updates

        newMap: Dict[:class:`str`, Set[:class:`str`]]
            The new map to update ``srcMap``

        Returns
        -------
        Dict[:class:`str`, Set[:class:`str`]]
            The updated map
        """

        return DictTools.update(srcMap, newMap, combineDuplicate = lambda oldToAssets, newToAssets: oldToAssets.union(newToAssets))
    

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

        pass

    def _updateDupAssets(self, srcAsset: Dict[str, Any], newAsset: Dict[str, Any]):
        return DictTools.update(srcAsset, newAsset, combineDuplicate = self._updateAssetContent)
    
    def updateRepo(self, srcRepo: Dict[float, Dict[str, Any]], newRepo: Dict[float, Dict[str, Any]]) -> Dict[float, Dict[str, Any]]:
        """
        Updates the values in ``srcRepo``

        Parameters
        ----------
        srcRepo: Dict[:class:`float`, Dict[:class:`str`, Any]]
            The first repo to be combined

        newRepo: Dict[:class:`float`, Dict[:class:`str`, Any]]
            The second repo to be combined

        Returns
        -------
        Dict[:class:`float`, Dict[:class:`str`, Any]]
            The combined repo
        """

        result = DictTools.update(srcRepo, newRepo, combineDuplicate = self._updateDupAssets)
        return result

    def _addVersion(self, name: str, version: float):
        """
        Adds a new version for a particular asset

        Parameters
        ----------
        name: :class:`str`
            The name of the asset

        version: :class:`float`
            The game version
        """
        try:
            self._versions[name]
        except KeyError:
            self._versions[name] = Version()

        self._versions[name].add(version)

    def findClosestVersion(self, name: str, version: Optional[float] = None, fromCache: bool = True) -> float:
        """
        Finds the closest available game version from :attr:`ModStrAssets._toAssets` for a particular asset

        Parameters
        ----------
        name: :class:`str`
            The name of the asset to search

        version: Optional[:class:`float`]
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
        :class:`float`
            The latest game version from the assets that corresponds to the desired version 
        """

        try:
            self._versions[name]
        except KeyError as e:
            raise KeyError(f"Asset name, '{name}', not found in the available versions") from e

        result = self._versions[name].findClosest(version, fromCache = fromCache)
        if (result is None):
            raise KeyError(f"No available versions for the asset by the name, '{name}'")

        return result
        
    def _partition(self, map: Dict[str, Set[str]], assets: Dict[float, Dict[str, Any]]) -> Tuple[Dict[str, Set[str]], Set[str], Set[str]]:
        """
        * Creates the `bipartition`_ for the assets to fix from vs the assets to fix to
        * Filters the mapping such that all the asset names in the new mapping exist in `assets`

        Parameters
        ----------
        map: Dict[:class:`str`, Set[:class:`str`]]
            The desired mapping for the assets for fixing

        assets: Dict[:class:`float`, Dict[:class:`str`, Any]]
            The source for all the assets :raw-html:`<br />` :raw-html:`<br />`

            * The outer key is the game version number for the assets
            * The first inner key is the name of the asset
            * The second inner key is the type of asset
            * The most inner value is the id for the asset (must be unique)

        Returns
        -------
        Tuple[Dict[:class:`str`, Set[:class:`str`]], Set[:class:`str`], Set[:class:`str`]]
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

        # creates the new sub-map and bipartition with vertices definitely being in the assets repo
        for fromAsset in map:
            if (not visited[fromAsset]):
                continue
            
            currentToAssets = map[fromAsset]
            newCurrentToAssets = set(filter(lambda toAsset: visited[toAsset], currentToAssets))

            if (not newCurrentToAssets):
                continue

            newMap[fromAsset] = newCurrentToAssets
            fixFrom.add(fromAsset)
            fixTo = fixTo.union(newCurrentToAssets)

        return (newMap, fixFrom, fixTo)
    
    def _updateVersions(self, assets: Dict[float, Dict[str, T]]):
        """
        Updates the versioning of the assets

        Parameters
        ----------
        assets: T
            The assets for checking the versioning
        """
        pass
    
    def addMap(self, assetMap: Dict[str, Set[str]], assets: Optional[Dict[float, Dict[str, T]]] = None):
        """
        Adds a new map to the existing map on how to retrieve the assets

        Parameters
        ----------
        assetMap: Dict[:class:`str`, Set[:class:`str`]]
            The new `adjacency list`_ used to map assets to fix from to assets to fix to

        assets: Optional[T]
            Any new assets that needs to be added/updated to the existing assets to support the given map

            **Default**: ``None``
        """

        if (assets is None):
            assets = {}

        self._repo = self.updateRepo(self._repo, assets)
        newAddMap, addFixFrom, addFixTo = self._partition(assetMap, self._repo)

        self._repo = self._repo
        if (not addFixFrom or not addFixTo):
            return

        self._map = self.updateMap(self._map, newAddMap)
        self._fixFrom = self._fixFrom.union(addFixFrom)
        self._fixTo = self._fixTo.union(addFixTo)

        # update the versions
        self._updateVersions(assets)


    def addMapping(self, fromAsset: str, toAssets: Set[str], assets: Any):
        """
        Adds a new mapping of how to fix the assets

        Parameters
        ----------
        fromAsset: :class:`str`
            The name of the asset to fix from

        toAssets: Set[:class:`str`]
            The names of the assets to fix to

        assets: Any
            Any new assets that needs to be added/updated to the existing assets to support the new mapping
        """

        map = {fromAsset: toAssets}
        self.addMap(map, assets)
##### EndScript
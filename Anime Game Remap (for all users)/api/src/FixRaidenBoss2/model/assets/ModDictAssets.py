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
from typing import Optional, Dict, Any, List, Union, Hashable
from collections import deque
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import T, VersionType
from .ModAssets import ModAssets
from ...tools.DictTools import DictTools
from ..Version import Version
##### EndLocalImports


##### Script
class ModDictAssets(ModAssets[T]):
    """
    This class inherits from :class:`ModAssets`

    Class to handle assets of any type for a mod where retrieval is based on some key

    .. note::
        This is a dictionary that retrieves a certain asset for some game version

    Parameters
    ----------
    repo: Dict[:class:`float`, Dict[Hashable, T]]
        The original source for any preset assets :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version number for the assets
        * The inner key is the name of the asset
        * The inner value is the content for the asset :raw-html:`<br />` :raw-html:`<br />`

        .. note::
            The type ``T`` may be either a:

            * Nested dictionary of the form ``Dict[Hashable, T]``
            * The type of the main content of the asset

    indices: Optional[List[:class:`str`]]
        The names of the index columns to query to retrive the main content of the asset :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will set 1 index column by the name "name" by default :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    setVersions: :class:`bool`
        Whether to initialize the version caches :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    Attributes
    ----------
    indices: List[:class:`str`]
        The names of the index columns to query to retrive the main content of an asset
    """

    def __init__(self, repo:  Dict[float, Dict[str, T]], indices: Optional[List[str]] = None, setVersions: bool = True, **kwargs):
        super().__init__(repo, **kwargs)
        self.indices = ["name"] if (indices is None) else indices

        if (setVersions):
            self._updateVersions(repo)

    def _convertIndexVals(self, indexVals: Union[Hashable, List[Hashable], Dict[str, Hashable]]) -> Dict[str, Hashable]:
        if (isinstance(indexVals, list)):
            newIndexVals = {}

            indexKeysLen = len(self.indices)
            indexValsLen = len(indexVals)
            minIndexLen = min(indexKeysLen, indexValsLen)

            for i in range(minIndexLen):
                newIndexVals[self.indices[i]] = indexVals[i]

            indexVals = newIndexVals

        elif (not isinstance(indexVals, dict)):
            indexVals = {self.indices[0]: indexVals}

        return indexVals

    def _addVersion(self, indexVals: Union[Hashable, List[Hashable], Dict[str, Hashable]], version: Union[str, float, VersionType]):
        """
        Adds a new version for a particular asset

        Parameters
        ----------
        indexVals: Union[Hashable, List[Hashable], Dict[:class:`str`, Hashable]]
            The values of the index columns for the particular asset

        version: :class:`float`
            The game version
        """

        indicesLen = len(self.indices)
        indexVals = self._convertIndexVals(indexVals)
        versions = None
        prevVersions = self._versions

        for i in range(indicesLen):
            index = self.indices[i]
            val = indexVals[index]
            versions = prevVersions.get(val)

            if (versions is None):
                versions = (Version(), {})
                prevVersions[val] = versions

            versions[0].add(version)
            prevVersions = versions[1]

        versions[0].add(version)

    def _updateVersions(self, assets: Dict[float, Dict[str, T]]):
        indicesLen = len(self.indices)
        
        for version in assets:
            stack = deque([([], assets[version])])

            while (stack):
                indexVals, currentAssets = stack.pop()
                if (len(indexVals) >= indicesLen):
                    self._addVersion(indexVals, version)
                    continue

                for indexVal in currentAssets:
                    stack.append((indexVals + [indexVal], currentAssets[indexVal]))

    def _updateDupAssets(self, depth: int, srcAsset: Dict[str, Any], newAsset: Dict[str, Any]):
        if (depth > len(self.indices)):
            return self._updateAssetContent(srcAsset, newAsset)

        return DictTools.update(srcAsset, newAsset, combineDuplicate = lambda assetId, srcVal, newVal: self._updateDupAssets(depth + 1, srcVal, newVal))

    def updateRepo(self, srcRepo: Dict[float, Dict[str, Any]], newRepo: Dict[float, Dict[str, Any]]) -> Dict[float, Dict[str, Any]]:
        result =  DictTools.update(srcRepo, newRepo, combineDuplicate = lambda version, srcVal, newVal: self._updateDupAssets(1, srcVal, newVal))

        self._versions.clear()
        self._updateVersions(result)
        return result
    
    def findClosestVersion(self, indexVals: Union[Hashable, List[Hashable], Dict[str, Hashable]], version: Optional[Union[str, float, VersionType]] = None, fromCache: bool = True) -> VersionType:
        """
        Finds the closest available game version from :attr:`ModStrAssets._toAssets` for a particular asset

        Parameters
        ----------
        indexVals: Union[Hashable, List[Hashable], Dict[:class:`str`, Hashable]]
            The values of the index columns to query the specific asset

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

        versions = self._versions
        indexVals = self._convertIndexVals(indexVals)

        minIndexLen = min(len(self.indices), len(indexVals))
        for i in range(minIndexLen):
            index = self.indices[i]
            val = indexVals[index]

            try:
                versions = versions[val]
            except KeyError as e:
                raise KeyError(f"Asset mapping using query indices, {indexVals}, not found in the available versions")
            
            if (i < minIndexLen - 1):
                versions = versions[1]

        result = versions[0].findClosest(version, fromCache = fromCache)
        if (result is None):
            KeyError("No available versions for the asset mapping")

        return result
        
    def get(self, indexVals: Union[Hashable, List[Hashable], Dict[str, Hashable]], version: Optional[float] = None, errorOnNotFound: bool = True, default: Any = None) -> T:
        """
        Retrieves the corresponding asset

        Parameters
        ----------
        indexVals: Union[Hashable, List[Hashable], Dict[:class:`str`, Hashable]]
            The values of the index columns to query the specific asset

        version: Optional[:class:`float`]
            The game version we want the asset to come from :raw-html:`<br />` :raw-html:`<br />`

            If This value is ``None``, then will retrieve the asset of the latest version. :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        errorOnNotFound: :class:`bool`  
            If no keywords are found, whether to raise an exception :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if no keywords are found :raw-html:`<br />` :raw-html:`<br />`

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

        try:
            closestVersion = self.findClosestVersion(indexVals, version = version)
        except KeyError as e:
            if (errorOnNotFound):
                raise e
            return default
        
        versionAssets = self._getVersionAssets(closestVersion, self._repo)

        indexVals = self._convertIndexVals(indexVals)
        result = versionAssets
        
        minIndexLen = min(len(self.indices), len(indexVals))
        for i in range(minIndexLen):
            index = self.indices[i]
            val = indexVals[index]
            result = result[val]

        return result
##### EndScript
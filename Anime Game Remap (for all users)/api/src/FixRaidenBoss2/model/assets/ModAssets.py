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

    Parameters
    ----------
    repo: Dict[:class:`float`, Dict[:class:`str`, T]]
        The original source for any preset assets :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version number for the assets
        * The inner key is the name of the asset
        * The inner value is the content for the asset

    Attributes
    ----------
    repo: Dict[:class:`float`, Dict[:class:`str`, T]]
        The original source for any preset assets :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version number for the assets
        * The inner key is the name of the asset
        * The inner value is the content for the asset
    """

    def __init__(self, repo: Dict[float, Dict[str, T]]):
        self._repo = repo
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

    def clearCache(self):
        """
        Clears out any cached data
        """

        self._versions.clear()

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

    def _updateDupAssets(self, srcAsset: Dict[str, Any], newAsset: Dict[str, Any]):
        return DictTools.update(srcAsset, newAsset, combineDuplicate = lambda assetId, srcAsset, newAsset: self._updateAssetContent(srcAsset, newAsset))
    
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

        result = DictTools.update(srcRepo, newRepo, combineDuplicate = lambda version, srcRepo, newRepo: self._updateDupAssets(srcRepo, newRepo))
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
##### EndScript
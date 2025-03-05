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
from typing import Dict, Optional, Set
##### EndExtImports

##### LocalImports
from .ModMappedAssets import ModMappedAssets
from ...constants.GenericTypes import T
from ..Version import Version
from ...tools.DictTools import DictTools
from ..VGRemap import VGRemap
##### EndLocalImports


##### Script
class ModDoubleDictAssets(ModMappedAssets[Dict[str, T]]):
    """
    This class inherits from :class:`ModMappedAssets`

    Class to handle retrieval of assets requiring 2 keys:

        * Assets to fix from
        * Assets to fix to

    .. note::
        This is a nested dictionary that retrieves a certain asset from:
        
        * The assets to fix from
        * The assets to fix to
        * The version of the game

    Parameters
    ----------
    repo: Dict[:class:`float`, Dict[:class:`str`, Dict[:class:`str`, T]]]
        The original source for any preset assets :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version number for the assets
        * The inner key is the name of the asset
        * The inner value is the content for the asset

    map: Optional[Dict[:class:`str`, Set[:class:`str`]]]
        The `adjacency list`_  that maps the assets to fix from to the assets to fix to using the predefined mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, repo: Dict[float, Dict[str, Dict[str, T]]], map: Optional[Dict[str, Set[str]]] = None):
        super().__init__(repo, map = map)

        self._versions: Dict[str, Dict[str, Version]] = {}
        self.loadFromPreset()

    @property
    def versions(self) -> Dict[str, Version]:
        """
        The game versions available for the assets :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the names of the assets to map from
        * The inner keys are the names of the assets to map to
        * The inner values are versions for the assets

        :getter: Returns all the available game versions for the assets
        :type: Dict[:class:`str`, Dict[:class:`str`, :class:`Version`]]
        """

        return self._versions

    def _updateAssetContent(self, asset1: Dict[str, T], asset2: Dict[str, T]) -> T:
        return DictTools.update(asset1, asset2)

    def loadFromPreset(self):
        super().loadFromPreset()
        self._updateVersions(self._repo)
    
    def _addVersion(self, fromAsset: str, toAsset: str, version: float):
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
            self._versions[fromAsset]
        except KeyError:
            self._versions[fromAsset] = {}

        try:
            self._versions[fromAsset][toAsset]
        except KeyError:
            self._versions[fromAsset][toAsset] = Version()

        self._versions[fromAsset][toAsset].add(version)

    def findClosestVersion(self, fromAsset: str, toAsset: str, version: Optional[float] = None, fromCache: bool = True) -> float:
        """
        Finds the closest available game version from :attr:`ModStrAssets._toAssets` for a particular asset

        Parameters
        ----------
        fromAsset: :class:`str`
            The name of the asset to map from

        toAsset: :class:`str`
            The name of the asset to map to

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
            self._versions[fromAsset][toAsset]
        except KeyError as e:
            raise KeyError(f"Asset mapping from '{fromAsset}' to '{toAsset}' not found in the available versions") from e

        result = self._versions[fromAsset][toAsset].findClosest(version, fromCache = fromCache)
        if (result is None):
            KeyError("No available versions for the asset mapping")

        return result
    
    def get(self, fromAsset: str, toAsset: str, version: Optional[float] = None) -> str:
        """
        Retrieves the corresponding vertex group remap

        Parameters
        ----------
        fromAsset: :class:`str`
            The name of the asset to map from

        toAsset: :class:`str`
            The name of the asset to map to

        version: Optional[:class:`float`]
            The game version we want the remap to come from :raw-html:`<br />` :raw-html:`<br />`

            If This value is ``None``, then will retrieve the asset of the latest version. :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Raises
        ------
        :class:`KeyError`
            If the corresponding asset based on the search parameters is not found
            
        Returns
        -------
        :class:`str`
            The found asset
        """

        closestVersion = self.findClosestVersion(fromAsset, toAsset, version = version)
        result = self._repo[closestVersion][fromAsset][toAsset]
        return result

    def _updateVersions(self, assets: Dict[float, Dict[str, Dict[str, VGRemap]]]):
        assetNamesToUpdate = self.fixFrom.union(self.fixTo)

        for version, versionAssets in assets.items():
            for fromAssetName in versionAssets:
                if (fromAssetName not in assetNamesToUpdate):
                    continue

                fromAssets = versionAssets[fromAssetName]
                for toAssetName in fromAssets:
                    if (toAssetName not in assetNamesToUpdate):
                        continue

                    self._addVersion(fromAssetName, toAssetName, version)
##### EndScript
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
from typing import Dict, List, Tuple, Set, Optional, Union
from collections import defaultdict
import copy
##### EndExtImports

##### LocalImports
from .ModAssets import ModAssets
from ...tools.DictTools import DictTools
##### EndLocalImports


##### Script
class ModIdAssets(ModAssets[Dict[str, str]]):
    """
    This class inherits from :class:`ModAssets`

    Class to handle hashes, indices, and other string id type assets for a mod

    Parameters
    ----------
    repo: Dict[:class:`float`, Dict[:class:`str`, Dict[:class:`str`, :class:`str`]]]
        The original source for any preset assets :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version of the assets
        * The first inner key is the name of the asset
        * The second inner key is the type of asset
        * The most inner value is the id for the asset

        .. note::
            The id value for each asset should be unique

    map: Optional[Dict[:class:`str`, Set[:class:`str`]]]
        The `adjacency list`_  that maps the assets to fix from to the assets to fix to using the predefined mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, repo: Dict[float, Dict[str, Dict[str, str]]], map: Optional[Dict[str, Set[str]]] = None):
        super().__init__(repo, map = map)

        self._fromAssets: Dict[str, List[Tuple[str, str]]] = {}
        self._toAssets: Dict[float, Dict[str, Dict[str, str]]] = {}
        self.loadFromPreset()

    @property
    def fromAssets(self) -> Dict[str, Tuple[Set[str], str]]:
        """
        The assets to fix from :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the ids for the asset
        * The values contains metadata about the assets to fix to where each tuple contains:

            # The names of the assets
            # The type of asset

        :getter: Returns the assets needed to be fixed
        :type: Dict[:class:`str`, Tuple[Set[:class:`str`], :class:`str`]]
        """

        return self._fromAssets
    
    @property
    def toAssets(self) -> Dict[float, Dict[str, Dict[str, str]]]:
        """
        The assets to fix to: :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version number for the assets
        * The first inner key is the name of the assets
        * The most inner key is the type of asset
        * The most inner value is the id for the asset

        :getter: Returns the new assets that will replace the old assets
        :type: Dict[:class:`float`, Dict[:class:`str`, Dict[:class:`str`, :class:`str`]]]
        """
        return self._toAssets

    def clear(self, flush: bool = True, clearMap: bool = False):
        self._fromAssets = {}
        self._toAssets = {}
        super().clear(flush = flush, clearMap = clearMap)

    def loadFromPreset(self):
        super().loadFromPreset()
        self._loadFromAssets()
        self._loadToAssets()

    def get(self, assetName: str, assetType: str, version: Optional[float] = None) -> str:
        """
        Retrieves the corresponding id asset from :attr:`ModStrAssets._toAssets`

        Parameters
        ----------
        assetName: :class:`str`
            The name of the assets we want

        assetType: :class:`str`
            The name of the type of asset we want.

        version: Optional[:class:`float`]
            The game version we want the asset to come from :raw-html:`<br />` :raw-html:`<br />`

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

        closestVersion = self.findClosestVersion(assetName, version = version)
        assets = self._toAssets[closestVersion]
        return assets[assetName][assetType]
    
    def replace(self, fromAsset: str, version: Optional[float] = None, toAssets: Optional[Union[str, Set[str]]] = None) -> Union[Optional[str], Dict[str, str]]:
        """
        Retrieves the corresponding asset to replace 'fromAsset'

        Parameters
        ----------
        fromAsset: :class:`str`
            The asset to be replaced

        version: Optional[:class:`float`]
            The game version we want the asset to come from :raw-html:`<br />` :raw-html:`<br />`

            If This value is ``None``, then will retrieve the asset of the latest version. :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        toAssets: Optional[Union[:class:`str`, Set[:class:`str`]]]
            The assets to used for replacement

        Returns
        -------
        Union[:class:`str`, Dict[:class:`str`, :class:`str`]]
            The corresponding assets for the fix to replace, if available :raw-html:`<br />` :raw-html:`<br />`

            The result is a string if the passed in parameter 'toAssets' is also a string :raw-html:`<br />` :raw-html:`<br />`
            
            Otherwise, the result is a dictionary such that: :raw-html:`<br />` :raw-html:`<br />`

            * The keys are the names of the assets
            * The values are the corresponding asset ids to replace
        """

        if (fromAsset not in self._fromAssets):
            if (isinstance(toAssets, str)):
                return None
            else:
                return {}

        toAssetMetadata = self._fromAssets[fromAsset]
        toAssetType = toAssetMetadata[1]
        toAssetNames = toAssetMetadata[0]

        resultAsStr = False
        if (toAssets is not None and isinstance(toAssets, str)):
            toAssetNames = {toAssets}
            resultAsStr = True
        elif (toAssets is not None and toAssets):
            toAssetNames = toAssetNames.intersection(toAssets)

        result = {}
        for toAssetName in toAssetNames:
            try:
                currentReplacement = self.get(toAssetName, toAssetType, version = version)
            except KeyError:
                continue
            else:
                result[toAssetName] = currentReplacement

        if (not resultAsStr):
            return result
        
        try:
            return result[toAssets]
        except KeyError:
            return None
    
    def _loadFromAssets(self):
        self._fromAssets = self._getFromAssets(self._map, self._repo)  

    def _loadToAssets(self):
        self._toAssets = self._getToAssets(self._fixTo, self._repo)
        
    def _updateAssetContent(self, srcAsset: Dict[str, str], newAsset: Dict[str, str]) -> Dict[str, str]:
        return DictTools.update(srcAsset, newAsset)

    def _getAssetChanges(self, oldAssets: Dict[float, Dict[str, Dict[str, str]]], newAssets: Dict[float, Dict[str, Dict[str, str]]]) -> Tuple[Dict[str, str], Dict[float, Dict[str, Dict[str, str]]], Dict[float, Dict[str, Dict[str, str]]]]:
        assetsToRemove = {}
        assetsToUpdate = {}
        changedIds = {}
        commonVersions = set(oldAssets.keys()).intersection(set(newAssets.keys()))
        
        for version in commonVersions:
            oldVersionAssets = oldAssets[version]
            newVersionAssets = newAssets[version]
            commonAssetNames = set(oldVersionAssets).intersection(set(newVersionAssets.keys()))

            for assetName in commonAssetNames:
                oldVersionNameAssets = oldVersionAssets[assetName]
                newVersionNameAssets = newVersionAssets[assetName]
                commonAssetTypes = set(oldVersionNameAssets.keys()).intersection(set(newVersionNameAssets.keys()))

                for assetType in commonAssetTypes:
                    oldAsset = oldVersionNameAssets[assetType]
                    newAsset = newVersionNameAssets[assetType]

                    if (oldAsset != newAsset):
                        assetsToRemove[version][assetName][assetType] = oldAsset
                        assetsToUpdate[version][assetName][assetType] = newAsset
                        changedIds[oldAsset] = newAsset

        return [changedIds, assetsToRemove, assetsToUpdate]

    @classmethod
    def _updateFromAssetsIds(self, fromAssets: Dict[str, Tuple[Set[str], str]], changedAssetIds: Dict[str, str]):
        for oldAssetId in changedAssetIds:
            newAssetId = changedAssetIds[oldAssetId]
            assetMetadata = fromAssets[oldAssetId]
            fromAssets.pop(oldAssetId)
            fromAssets[newAssetId] = assetMetadata

    @classmethod
    def _getFromAssets(cls, map: Dict[str, Set[str]], assets: Dict[float, Dict[str, Dict[str, str]]]) -> Dict[str, Tuple[Set[str], str]]:
        """
        Retrieves the assets to fix from

        Parameters
        ----------
        map: Dict[str, Set[str]]
            The mapping for fixing the assets

        assets: Dict[:class:`float`, Dict[:class:`str`, Dict[:class:`str`, :class:`str`]]]
            The source for all the assets :raw-html:`<br />` :raw-html:`<br />`

            * The outer key is the game version number for the assets
            * The first inner key is the name of the asset
            * The second inner key is the type of asset
            * The most inner value is the id for the asset (must be unique)

        Returns
        -------
        Dict[:class:`str`, Tuple[Set[:class:`str`], :class:`str`]]
            The assets to fix from :raw-html:`<br />` :raw-html:`<br />`

            * The keys are the ids for the asset
            * The values contains metadata about the assets to fix to where each tuple contains:

                # The names of the assets
                # The type of asset

        """

        result = {}
        if (not map):
            return result

        invertedAssets = defaultdict(lambda: {})
        toAssets = defaultdict(lambda: set())

        for version in assets:
            versionAssets = assets[version]

            # get all the available assets to fix from
            for name in map:
                try:
                    asset = versionAssets[name]
                except KeyError:
                    continue
                else:
                    asset = DictTools.invert(asset)
                    DictTools.update(invertedAssets[name], asset)

            # get the available assets to fix to
            for name in map:
                toAssetNames = map[name]
                for toAssetName in toAssetNames:
                    try:
                        asset = versionAssets[toAssetName]
                    except:
                        continue
                    else:
                        toAssets[toAssetName] = toAssets[toAssetName].union(set(asset.keys()))

        # organize the assets
        for fromAssetName in invertedAssets:
            asset = invertedAssets[fromAssetName]
            toAssetNames = map[fromAssetName]

            for assetId in asset:
                assetType = asset[assetId]
                toNames = set()

                for toAssetName in toAssetNames:
                    toAssetTypes = toAssets[toAssetName]
                    if (assetType in toAssetTypes):
                        toNames.add(toAssetName)

                metadata = (toNames, assetType)
                result[assetId] = metadata

        return result
    
    @classmethod
    def _removeToAssets(cls, toAssets: Dict[float, Dict[str, Dict[str, str]]], assetsToRemove: Dict[float, Dict[str, Dict[str, str]]]):
        for version in toAssets:
            versionAssets = toAssets[version]
            
            for name in versionAssets:
                currentAssets = versionAssets[name]

                for type in currentAssets:
                    try:
                        assetsToRemove[version][name][type]
                    except:
                        continue
                    else:
                        toAssets[version][name].pop(type)

                if (not toAssets[version][name]):
                    toAssets[version].pop(name)

            if (not toAssets[version]):
                toAssets.pop(version)
    
    def _getToAssets(self, assetNames: Set[str], assets: Dict[float, Dict[str, Dict[str, str]]]) -> Dict[float, Dict[str, Dict[str, str]]]:
        """
        Retrieves the assets to fix to

        Parameters
        ----------
        assetNames: Set[:class:`str`]
            The names of the assets to fix to

        assets: Dict[:class:`float`, Dict[:class:`str`, Dict[:class:`str`, :class:`str`]]]
            The source for all the assets :raw-html:`<br />` :raw-html:`<br />`

            * The outer key is the game version number for the assets
            * The first inner key is the name of the asset
            * The second inner key is the type of asset
            * The most inner value is the id for the asset (must be unique)

        Returns
        -------
        Dict[:class:`float`, Dict[:class:`str`, Dict[:class:`str`, :class:`str`]]]
            The assets to fix to  :raw-html:`<br />` :raw-html:`<br />`

            * The outer key is the game version number for the assets
            * The first inner key is the name of the asset
            * The second inner key is the type of asset
            * The most inner value is the id for the asset (must be unique)
        """

        result = {}
        if (not assetNames):
            return result
        
        prevToAssets = defaultdict(lambda: {})

        for version, versionAssets in assets.items():
            currentToAssets = {}

            for name in assetNames:
                try:
                    asset = versionAssets[name]
                except KeyError:
                    continue
                else:
                    prevAsset = prevToAssets[name]
                    DictTools.update(prevAsset, asset)
                    
                    if (prevAsset):
                        currentToAssets[name] = copy.deepcopy(prevAsset)
                        self._addVersion(name, version)

            if (currentToAssets):
                result[version] = currentToAssets

        return result


    def addMap(self, assetMap: Dict[str, Set[str]], assets: Optional[Dict[float, Dict[str, Dict[str, str]]]] = None):
        super().addMap(assetMap, assets = assets)
        if (assets is None):
            assets = {}

        changedIds, assetsIdsToRemove, assetsIdsToUpdate = self._getAssetChanges(self._repo, assets)
        self._repo = self.updateRepo(self._repo, assets)
        newAddMap, addFixFrom, addFixTo = self._partition(assetMap, self._repo)

        self._repo = self._repo
        if (not addFixFrom or not addFixTo):
            return

        self._map = self.updateMap(self._map, newAddMap)
        self._fixFrom = self._fixFrom.union(addFixFrom)
        self._fixTo = self._fixTo.union(addFixTo)

        # update the assets to fix from
        self._updateFromAssetsIds(self._fromAssets, changedIds)
        addFromAssets = self._getFromAssets(newAddMap, self._repo)
        DictTools.update(self._fromAssets, addFromAssets)

        # update the assets to fix to
        self._removeToAssets(self._toAssets, assetsIdsToRemove)

        addToAssetNames = set(map(lambda versionAssets: versionAssets.keys(), assetsIdsToUpdate.values()))
        addToAssetNames = addToAssetNames.union(addFixTo)
        addToAssets = self._getToAssets(addToAssetNames, self._repo)

        DictTools.update(self._toAssets, addToAssets, combineDuplicate = self._updateDupAssets)
##### EndScript
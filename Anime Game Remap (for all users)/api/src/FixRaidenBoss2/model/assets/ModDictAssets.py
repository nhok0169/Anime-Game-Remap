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
from typing import Optional, Dict, Any
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import T
from .ModAssets import ModAssets
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
    repo: Dict[:class:`float`, Dict[:class:`str`, T]]
        The original source for any preset assets :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version number for the assets
        * The inner key is the name of the asset
        * The inner value is the content for the asset
    """

    def __init__(self, repo:  Dict[float, Dict[str, T]]):
        super().__init__(repo)
        self._updateVersions(repo)

    def _updateVersions(self, assets: Dict[float, Dict[str, T]]):
        for version, versionAssets in assets.items():
            for assetName in versionAssets:
                self._addVersion(assetName, version)

    def updateRepo(self, srcRepo: Dict[float, Dict[str, Any]], newRepo: Dict[float, Dict[str, Any]]) -> Dict[float, Dict[str, Any]]:
        result = super().updateRepo(srcRepo, newRepo)
        self._updateVersions(newRepo)
        return result
        
    def get(self, assetName: str, version: Optional[float] = None) -> T:
        """
        Retrieves the corresponding asset

        Parameters
        ----------
        assetName: :class:`str`
            The name of the assets we want

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
        T
            The found asset
        """

        closestVersion = self.findClosestVersion(assetName, version = version)
        return self._repo[closestVersion][assetName]
##### EndScript
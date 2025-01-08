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
from typing import Optional, List
##### EndExtImports

##### LocalImports
from ..tools.caches.LRUCache import LruCache
from ..tools.Algo import Algo
##### EndLocalImports


##### Script
class Version():
    """
    Class for handling game versions

    Parameters
    ----------
    versions: Optional[List[float]]
        The versions available

        **Default**: ``None``

    Attributes
    ----------
    _versionCache: :class:`LruCache`
        Cache to store the closest available versions based off the versions that the user searches :raw-html:`<br />` :raw-html:`<br />`

        * The keys in the `LRU cache`_ are the versions the user searches
        * The values in the  `LRU cache`_ are the corresponding versions available to the versions the user searches
    """

    def __init__(self, versions: Optional[List[float]] = None):
        if (versions is None):
            versions = []

        self._latestVersion: Optional[float] = None
        self._versionCache = LruCache()
        self.versions = versions

    @property
    def versions(self):
        """
        The available versions

        :getter: The versions in sorted ascending order
        :setter: Sets the new versions
        :type: List[float]
        """

        return self._versions
    
    @versions.setter
    def versions(self, newVersions: List[float]) -> List[float]:
        self.clear()

        self._versions = list(set(newVersions))
        self._versions.sort()
        if (self._versions):
            self._latestVersion = self._versions[-1]

    @property
    def latestVersion(self) -> Optional[float]:
        """
        The latest version available

        :getter: The latest version
        :type: Optional[float]
        """

        return self._latestVersion

    def clear(self):
        """
        Clears all the version data
        """

        self._versions = []
        self._latestVersion = None
        self._versionCache.clear()
    
    def _updateLatestVersion(self, newVersion: float):
        """
        Updates the latest version

        Parameters
        ----------
        newVersion: :class:`float`
            The new available version
        """

        if (self._latestVersion is None):
            self._latestVersion = newVersion
            return
        
        self._latestVersion = max(self._latestVersion, newVersion)

    def _add(self, newVersion: float):
        if (not self._versions or newVersion > self._versions[-1]):
            self._versions.append(newVersion)
        elif (newVersion < self._versions[0]):
            self._versions.insert(0, newVersion)
        else:
            Algo.binaryInsert(self._versions, newVersion, lambda v1, v2: v1 - v2, optionalInsert = True)

    def add(self, newVersion: float):
        """
        Adds a new version

        Parameters
        ----------
        newVersion: :class:`float`
            The new version to add
        """

        self._add(newVersion)
        self._updateLatestVersion(newVersion)

    def findClosest(self, version: Optional[float], fromCache: float = True) -> Optional[float]:
        """
        Finds the closest version available

        Parameters
        ----------
        version: Optional[:class:`float`]
            The version to be searched :raw-html:`<br />` :raw-html:`<br />`

            If This value is ``None``, then will assume we want the latest version :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        fromCache: :class:`float`
            Whether we want the result to be accessed from the cache :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        Optional[:class:`float`]
            The closest version available or ``None`` if there are no versions available
        """

        if (self._latestVersion is None):
            return None

        if (version is None):
            return self._latestVersion

        if (fromCache):
            try:
                return self._versionCache[version]
            except KeyError:
                pass

        found, ind = Algo.binarySearch(self._versions, version, lambda v1, v2: v1 - v2)

        result = 0
        if (found):
            result = self._versions[ind]
        elif (ind > 0):
            result = self._versions[ind - 1]
        else:
            result = self._versions[0]

        self._versionCache[version] = result
        return result
##### EndScript

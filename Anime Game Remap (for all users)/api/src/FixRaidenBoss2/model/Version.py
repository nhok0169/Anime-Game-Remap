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
from typing import Optional, List, Union
##### EndExtImports

##### LocalImports
from ..constants.GenericTypes import VersionType
from ..constants.Packages import PackageModules
from ..constants.GlobalPackageManager import GlobalPackageManager
from ..tools.caches.LRUCache import LruCache
from ..tools.Algo import Algo
##### EndLocalImports


##### Script
class Version():
    """
    Class for handling game versions

    Parameters
    ----------
    versions: Optional[List[Union[:class:`float`, :class:`str`, `packaging.version.Version`_]]]
        The versions available

        **Default**: ``None``

    Attributes
    ----------
    _versionCache: :class:`LruCache`
        Cache to store the closest available versions based off the versions that the user searches :raw-html:`<br />` :raw-html:`<br />`

        * The keys in the `LRU cache`_ are the versions the user searches
        * The values in the  `LRU cache`_ are the corresponding versions available to the versions the user searches
    """

    def __init__(self, versions: Optional[List[Union[float, str, VersionType]]] = None):
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
        :type: List[`packaging.version.Version`_]
        """

        return self._versions
    
    @versions.setter
    def versions(self, newVersions: List[Union[float, VersionType]]) -> List[float]:
        self.clear()

        versionModule = GlobalPackageManager.get(PackageModules.Packaging_Version.value)
        versionCls = versionModule.Version

        newVersions = list(map(lambda version: versionCls(f"{version}") if (isinstance(version, float) or isinstance(version, int) or isinstance(version, str)) else version, newVersions))

        self._versions = list(set(newVersions))
        self._versions.sort()
        if (self._versions):
            self._latestVersion = self._versions[-1]

    @property
    def latestVersion(self) -> Optional[VersionType]:
        """
        The latest version available

        :getter: The latest version
        :type: Optional[`packaging.version.Version`_]
        """

        return self._latestVersion

    def clear(self):
        """
        Clears all the version data
        """

        self._versions = []
        self._latestVersion = None
        self._versionCache.clear()

    @classmethod
    def getVersion(cls, rawVersion: Union[float, str, VersionType]) -> Optional[VersionType]:
        """
        Retrieves the corresponding version

        Parameters
        ----------
        rawVersion: Union[:class:`float`, :class:`str`, `packaging.version.Version`_]
            The version to translate

        Returns
        -------
        Optional[`packaging.version.Version`_]
            The corresponding version, if possible to translate
        """

        versionModule = GlobalPackageManager.get(PackageModules.Packaging_Version.value)
        versionCls = versionModule.Version
        invalidVersionError = versionModule.InvalidVersion

        if (isinstance(rawVersion, versionCls)):
            return rawVersion
        
        try:
            return versionCls(f"{rawVersion}")
        except invalidVersionError as e:
            return None
        
    @classmethod
    def compareVersions(cls, version1: VersionType, version2: VersionType) -> int:
        """
        Compares two versions

        Parameters
        ----------
        version1: `packaging.version.Version`_
            The first version to compare

        version2: `packaging.version.Version`_
            The second version to compare

        Returns
        -------
        :class:`int`
            A negative number if `version1` is less than `version2`, a positive number if `version1` is greater than `version2`, and zero if they are equal
        """

        if (version1 == version2):
            return 0
        elif (version1 < version2):
            return -1
        return 1
    
    def _updateLatestVersion(self, newVersion: VersionType):
        """
        Updates the latest version

        Parameters
        ----------
        newVersion: `packaging.version.Version`_
            The new available version
        """

        if (self._latestVersion is None):
            self._latestVersion = newVersion
            return
        
        self._latestVersion = max(self._latestVersion, newVersion)

    def _add(self, newVersion: Union[VersionType]):
        if (not self._versions or newVersion > self._versions[-1]):
            self._versions.append(newVersion)
        elif (newVersion < self._versions[0]):
            self._versions.insert(0, newVersion)
        else:
            Algo.binaryInsert(self._versions, newVersion, lambda v1, v2: self.compareVersions(v1, v2), optionalInsert = True)

    def add(self, newVersion: Union[str, float, VersionType]):
        """
        Adds a new version

        Parameters
        ----------
        newVersion: Union[:class:`str`, :class:`float`, `packaging.version.Version`_]
            The new version to add
        """

        newVersion = self.getVersion(newVersion)
        if (newVersion is None):
            return

        self._add(newVersion)
        self._updateLatestVersion(newVersion)

    def findClosest(self, version: Optional[Union[str, float, VersionType]], fromCache: bool = True) -> Optional[float]:
        """
        Finds the closest version available

        Parameters
        ----------
        version: Optional[Union[:class:`str`, :class:`float`, `packaging.version.Version`_]]
            The version to be searched :raw-html:`<br />` :raw-html:`<br />`

            If This value is ``None``, then will assume we want the latest version :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        fromCache: :class:`bool`
            Whether we want the result to be accessed from the cache :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        Optional[`packaging.version.Version`_]
            The closest version available or ``None`` if there are no versions available
        """

        if (self._latestVersion is None):
            return None

        if (version is None):
            return self._latestVersion
        
        version = self.getVersion(version)

        if (fromCache):
            try:
                return self._versionCache[version]
            except KeyError:
                pass

        found, ind = Algo.binarySearch(self._versions, version, lambda v1, v2: self.compareVersions(v1, v2))

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

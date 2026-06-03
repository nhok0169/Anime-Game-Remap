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
import os
import shutil
from typing import Optional, Callable, Any, TYPE_CHECKING, Any, List
##### EndExtImports

##### LocalImports
from .IniResource import IniResource, IniFixResource, IniGroupedResource
from ..stats.CachedFileStats import CachedFileStats
from ..stats.RemapStats import RemapStats
from ...tools.files.FileDownload import FileDownload
from ..files.BlendFile import BlendFile
from ..buffers.BufElementType import BufElementType
from ..VGRemap import VGRemap


if (TYPE_CHECKING):
    from ..Mod import Mod
##### EndLocalImports


##### Script
class RemapIniResourceMixin():
    def srcEncounteredError(self, stats: RemapStats) -> bool:
        """
        Determines whether the resources have previously encountered an error

        Parameters
        ----------
        stats: :class:`RemapStats`
            The stats tracked by the remap process

        Returns
        -------
        :class:`bool`
            Whether the resource has encountered an error
        """

        pass

    def srcIsFixed(self, stats: RemapStats) -> bool:
        """
        Determines whether the resources were already fixed

        Parameters
        ----------
        stats: :class:`RemapStats`
            The stats tracked by the remap process

        Returns
        -------
        :class:`bool`
            Whether the resource was already fixed
        """

        pass

    def fixEncounteredError(self, stats: RemapStats) -> bool:
        """
        Determines whether the fixed resources have previously encountered an error

        Parameters
        ----------
        stats: :class:`RemapStats`
            The stats tracked by the remap process

        Returns
        -------
        :class:`bool`
            Whether the fixed resource has encountered an error
        """

        pass

    def fixIsFixed(self, stats: RemapStats) -> bool:
        """
        Determines whether the fixed resources were already fixed

        Parameters
        ----------
        stats: :class:`RemapStats`
            The stats tracked by the remap process

        Returns
        -------
        :class:`bool`
            Whether the fixed resource was already fixed
        """

        pass

    def fixExists(self, stats: RemapStats) -> bool:
        """
        Determines whether the fixed resources already exist in the OS

        Parameters
        ----------
        stats: :class:`RemapStats`
            The stats tracked by the remap process

        Returns
        -------
        :class:`bool`
            Whether the fixed resources already exist
        """

        pass

    def hasRequired(self) -> bool:
        """
        Determines whether all the necessary resources have been collected

        Returns
        -------
        :class:`bool`
            whether all the required resources are gathered
        """

        pass

    def remapFix(self, mod: "Mod", stats: RemapStats, *args, **kwargs) -> bool:
        """
        Fixes the resource for the overall remap process

        Parameters
        ----------
        mod: :class:`Mod`
            The mod to fix from

        stats: :class:`RemapStats`
            The stats tracked by the remap process

        \*args
            Any extra arguments to supply

        \*\*kwargs
            Any extra keyword arguments to supply

        Returns
        -------
        :class:`bool`
            Whether the resource was fixed
        """

        pass


class RemapIniResource(IniResource, RemapIniResourceMixin):
    """
    This class inherits from :class:`IniResource`

    Base class for some resource in a .ini file that is used by the overall remap process at :class:`RemapService`

    Parameters
    ----------
    type: :class:`str`
        The name for the type of resource

    iniFolderPath: :class:`str`
        The path to the folder of the .ini file

    srcPath: :class:`str`
        The file path to the resource

    fixFunc: Optional[Callable[[:class:`RemapIniResource`, ...], Any]]
        Custom function for fixing the resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """
    
    def hasRequired(self):
        return True
    
    def fixExists(self, stats: RemapStats):
        return self.srcIsFixed(stats)
    
    def remapFix(self, mod: "Mod", stats: RemapStats, *args, **kwargs) -> bool:
        return self.fix(*args, **kwargs)
    

class RemapIniFixResource(IniFixResource, RemapIniResourceMixin):
    """
    This class inherits from :class:`IniFixResource`

    Base class for some resource to fix in a .ini file that is used by the overall remap process at :class:`RemapService`

    Parameters
    ----------
    type: :class:`str`
        The name for the type of resource

    iniFolderPath: :class:`str`
        The path to the folder of the .ini file

    srcPath: :class:`str`
        The file path to the resource

    fixedPath: :class:`str`
        The file path to the fixed resource

    fixFunc: Optional[Callable[[:class:`RemapIniFixResource`, :class:`Mod`, :class:`RemapStats`, ...], Any]]
        Custom function for fixing the resource :raw-html:`<br />` :raw-html:`<br />`

        The function takes in:

        #. The resource to fix
        #. The mod where the resource comes from
        #. The file stats to track :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """
    
    def fixExists(self):
        return os.path.isfile(self.fixedPath)
    
    def hasRequired(self):
        return True
    
    def remapFix(self, mod: "Mod", stats: RemapStats, *args, **kwargs) -> bool:
        return self.fix(*args, **kwargs)
    

class RemapIniGroupedResource(IniGroupedResource, RemapIniResourceMixin):
    """
    This class inherits from :class:`IniGroupedResource`

    Base class for a group of resources to fix in a .ini file that is used by the overall remap process at :class:`RemapService`

    Parameters
    ----------
    name: :class:`str`
        The name of the group of resources

    resources: Optional[Dict[:class:`str`, Union[:class:`IniResource`, :class:`str`]]]
        The group of resources. 

        * The keys are the type of the resource
        * If :attr:`isBuilt` is set to ``True`` then all the values are expected to be of type :class:`IniResource`
          Otherwise, all the values are expected to be a string value representing the source file location for the resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    fixFunc: Optional[Callable[[:class:`IniGroupedResource`, ...], Any]]
        Custom function for fixing the resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    isBuilt: :class:`bool`
        Whether the grouped resource is ready to be fixed :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``
    """

    def remapFix(self, mod: "Mod", stats: RemapStats, *args, **kwargs) -> bool:
        return self.fix(*args, **kwargs)


class RemapIniDownload(RemapIniResource):
    """
    This class inherits from :class:`RemapIniResource`

    Class for some download resource in a .ini file that is used by the overall remap process at :class:`RemapService`

    Parameters
    ----------
    iniFolderPath: :class:`str`
        The path to the folder of the .ini file

    srcPath: :class:`str`
        The file path to the resource

    download: :class:`FileDownload`
        The downloader associated to the file

    type: :class:`str`
        The name for the type of resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``"download"``

    fixFunc: Optional[Callable[[:class:`RemapIniDownload`, :class:`Mod`, :class:`RemapStats`, ...], Any]]
        Custom function for fixing the resource :raw-html:`<br />` :raw-html:`<br />`

        The function takes in:

        #. The resource to fix
        #. The mod where the resource comes from
        #. The file stats to track :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, iniFolderPath: str, srcPath: str, download: FileDownload, type: str = "download", fixFunc = None):
        super().__init__(type, iniFolderPath, srcPath, fixFunc = fixFunc)
        self.download = download

    def srcEncounteredError(self, stats: RemapStats) -> bool:
        return self.srcPath in stats.download.skipped
    
    def srcIsFixed(self, stats: RemapStats) -> bool:
        return self.srcPath in stats.download.fixed
    
    def fixEncounteredError(self, stats: RemapStats):
        return self.srcEncounteredError(stats)
    
    def fixIsFixed(self, stats: RemapStats):
        return self.srcIsFixed(stats)
    
    def fixExists(self, stats: RemapStats):
        return self.srcIsFixed(stats)

    def _fix(self, downloadStats: CachedFileStats, *args, proxy: Optional[str] = None, 
            downloadHandler: Optional[Callable[[str], Any]] = None, 
            cacheHitHandler: Optional[Callable[[str], Any]] = None, **kwargs) -> bool:
        """
        Downloads the resource

        Parameters
        ----------
        downloadStats: :class:`CachedFileStats`
            The stats for the file download for the overall remap process at :class:`RemapService`

        proxy: Optional[:class:`str`]
            The link to the proxy server used for any internet network requests made :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        downloadHandler: Optional[Callable[[:class:`str`], Any]]
            The callback once the file has been downloaded. The function takes in the filepath to the download :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        cacheHitHandler: Optional[Callable[[:class:`str`], Any]]
            The callback once the file was copied from the cache instead of needing to be downloaded. The function takes in the filepath to the download :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        :class:`bool`
            Whether the resource has been downloaded
        """

        downloadFolder = os.path.dirname(self.srcPath)
        rawDownloadFullPath, downloaded, downloadExisted =  self.download.get(downloadFolder, proxy = proxy)

        if (self.srcPath != rawDownloadFullPath):
            shutil.move(rawDownloadFullPath, self.srcPath)

        if (downloaded):
            downloadStats.addFixed(self.srcPath)
            if (downloadHandler is not None):
                downloadHandler(self.srcPath)
            
        else:
            downloadStats.addHit(self.srcPath)
            if (cacheHitHandler is not None):
                cacheHitHandler(self.srcPath)
    
    def remapFix(self, mod: "Mod", stats: RemapStats, *args, proxy: Optional[str] = None, **kwargs):
        """
        Fixes the resource for the overall remap process

        Parameters
        ----------
        mod: :class:`Mod`
            The mod to fix from

        stats: :class:`RemapStats`
            The stats tracked by the remap process

        proxy: Optional[:class:`str`]
            The link to the proxy server used for any internet network requests made :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None`` 

        Returns
        -------
        :class:`bool`
            Whether the resource was fixed
        """

        return self.fix(stats.download, proxy = proxy, 
                        downloadHandler = lambda downloadPath: mod.print("log", f"Download successful at {downloadPath}"),
                        cacheHitHandler = lambda downloadPath: mod.print("log", f"Copied previous download to {downloadPath}"))
##### EndScript
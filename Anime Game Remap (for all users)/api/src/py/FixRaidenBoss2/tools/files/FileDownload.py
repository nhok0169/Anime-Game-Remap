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
from typing import Optional, Tuple
##### EndExtImports

##### LocalImports
from ...constants.GlobalPackageManager import GlobalPackageManager
from ...constants.Packages import PackageModules
from .FileService import FileService
##### EndLocalImports


##### Script
class FileDownload():
    """
    Class to handle file downloads from some server

    Parameters
    ----------
    url: :class:`str`
        The link to the file download

    filename: :class:`str`
        The base name of the file (with extension)

    cache: :class:`bool`
        Whether to copy the previous downloaded file if possible instead of
        downloading another copy of the file :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    Attributes
    ----------
    url: :class:`str`
        The link to the file download

    filename: :class:`str`
        The base name of the file (with extension)

    cache: :class:`bool`
        Whether to copy the previous downloaded file if possible instead of
        downloading another copy of the file

    _prevPath: Optional[:class:`str`]
        The previous full path to the downloaded file
    """

    def __init__(self, url: str, filename: str, cache: bool = True):
        self.url = url
        self.filename = filename
        self.cache = cache

        self._prevPath: Optional[str] = None

    def download(self, folder: str, proxy: Optional[str] = None) -> str:
        """
        Downloads the required file

        Parameters
        ----------
        folder: :class:`str`
            The folder to store the downloaded file

        proxy: Optional[:class:`str`]
            The link to the proxy server used for any internet network access :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        proxies = None if (proxy is None) else {"http": proxy, "https": proxy, "ftp": proxy}

        filename = os.path.join(folder, os.path.basename(self.filename))

        requests = GlobalPackageManager.Packager.get(PackageModules.Requests.value)
        fileRequest = requests.get(self.url, proxies = proxies)

        FileService.writeBinary(filename, fileRequest.content)
        return filename
    
    def get(self, folder: str, proxy: Optional[str] = None) -> Tuple[str, bool, bool]:
        """
        Retrieves the required file

        Parameters
        ----------
        folder: :class:`str`
            The folder to store the downloaded file

        proxy: Optional[:class:`str`]
            The link to the proxy server used for any internet network access :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns 
        -------
        Tuple[:class:`str`, :class:`bool`, :class:`bool`]
            A tuple that contains:

            #. The path to the downloaded file
            #. Whether a download occured
            #. Whether a previous download to the file existed
        """

        wasDownloaded = self._prevPath is None
        if (not self.cache or wasDownloaded):
            self._prevPath = self.download(folder, proxy = proxy)
            return (self._prevPath, True, wasDownloaded)

        filename = os.path.join(folder, os.path.basename(self.filename))
        downloadRequired = False

        if (self._prevPath == filename):
            return (filename, downloadRequired, wasDownloaded)

        try:
            shutil.copy(self._prevPath, filename)
        except Exception as e:
            self._prevPath = self.download(folder, proxy = proxy)
            downloadRequired = True
        
        return (filename, downloadRequired, wasDownloaded)
##### EndScript
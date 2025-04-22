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
import requests
import os
from typing import Optional
##### EndExtImports

##### LocalImports
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

    Attributes
    ----------
    url: :class:`str`
        The link to the file download

    filename: :class:`str`
        The base name of the file (with extension)
    """

    def __init__(self, url: str, filename: str):
        self.url = url
        self.filename = filename

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
        fileRequest = requests.get(self.url, proxies = proxies)

        FileService.writeBinary(filename, fileRequest.content)
        return filename
##### EndScript
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
from typing import Dict, List
##### EndExtImports

##### LocalImports
from ...tools.files.FileDownload import FileDownload
from .IniSrcResourceModel import IniSrcResourceModel
##### EndLocalImports


##### Script
class IniDownloadModel(IniSrcResourceModel):
    """
    This class inherits from: :class:`IniSrcResourceModel`

    Contains data about a particular resource to download in the original .ini file

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: for path, fullPath in x

            Iterates over all the paths to some resource within a :class:`IfContentPart`, ``x`` :raw-html:`<br />` :raw-html:`<br />`

            The tuples to iterate over are as follows:
            #. path: (:class:`str`) The path to the file
            #. fullPath: (:class:`str`) The full path to the file

    Parameters
    ----------
    iniFolderPath: :class:`str`
        The folder path to where the .ini file of the resource is located

    paths: Dict[:class:`int`, List[:class:`str`]]
        The file paths to the download files for the resource :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the indices to the :class:`IfContentPart` that the resource file appears in the :class:`IfTemplate` for some resource
        * The values are the file paths within the :class:`IfContentPart`

    downloads: Dict[:class:`int`, List[:class:`FileDownload`]]
        The downloader associated for each file :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the indices to the :class:`IfContentPart` that the resource file appears in the :class:`IfTemplate` for some resource
        * The values are the downloaders for the files within the :class:`IfContentPart`

    Attributes
    ----------
    downloads: Dict[:class:`int`, List[:class:`FileDownload`]]
        The downloader associated for each file :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the indices to the :class:`IfContentPart` that the resource file appears in the :class:`IfTemplate` for some resource
        * The values are the downloaders for the files within the :class:`IfContentPart`s
    """

    def __init__(self, iniFolderPath: str, paths: Dict[int, List[str]], downloads: Dict[int, List[FileDownload]]):
        super().__init__(iniFolderPath, paths)
        self.downloads = downloads
##### EndScript
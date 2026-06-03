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
from ...tools.files.FileService import FileService
from .IniResourceModel import IniResourceModel
##### EndLocalImports


##### Script
class IniSrcResourceModel(IniResourceModel):
    """
    This class inherits from :class:`IniResourceModel`

    Contains data for a particular resource in the original .ini file

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
        The file paths to the fixed files for the resource :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the indices to the :class:`IfContentPart` that the resource file appears in the :class:`IfTemplate` for some resource
        * The values are the file paths within the :class:`IfContentPart`

    Attributes
    ----------
    paths: Dict[:class:`int`, List[:class:`str`]]
        The file paths to the fixed files for the resource :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the indices to the :class:`IfContentPart` that the resource file appears in the :class:`IfTemplate` for some resource
        * The values are the file paths within the :class:`IfContentPart`

    fullPaths: Dict[:class:`int`, List[:class:`str`]]
        The absolute paths to the fixed resource files for the resource :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the indices to the :class:`IfContentPart` that the files appear in the :class:`IfTemplate` for some resource
        * The values are the file paths within the :class:`IfContentPart`
    """

    def __init__(self, iniFolderPath: str, paths: Dict[int, List[str]]):
        super().__init__(iniFolderPath)
        self.paths = paths

        # retrieve the absolute paths
        self.fullPaths = {}
        for partIndex, partPaths in self.paths.items():
            self.fullPaths[partIndex] = list(map(lambda path: FileService.absPathOfRelPath(path, iniFolderPath), partPaths))

    def __iter__(self):
        for ifTemplateInd in self.paths:
            partPaths = self.paths[ifTemplateInd]
            partPathsLen = len(partPaths)

            for i in range(partPathsLen):
                path = partPaths[i]
                fullPath = self.fullPaths[ifTemplateInd][i]

                yield (path, fullPath)
##### EndScript


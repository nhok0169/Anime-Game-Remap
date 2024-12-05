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
from typing import Dict, Optional, List
##### EndExtImports

##### LocalImports
from ...tools.files.FileService import FileService
##### EndLocalImports


##### Script
# Needed data model to inject into the .ini file
class IniResourceModel():
    """
    Contains data for fixing a particular resource in a .ini file

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: for fixedPath, fixedFullPath, origPath, origFullPath in x

            Iterates over all the fixed paths to some resource within a :class:`IfContentPart`, ``x`` :raw-html:`<br />` :raw-html:`<br />`

            The tuples to iterate over are as follows:
            #. fixedPath: (:class:`str`) The path name of the fixed file
            #. fixedFullPath: (:class:`str`) The full path name to the fixed file 
            #. origPath: (Optional[:class:`str`]) The path to the orignal file, if available
            #. origFullPath: (Optional[:class:`str`]) The full path name to the original file, if available

    Parameters
    ----------
    iniFolderPath: :class:`str`
        The folder path to where the .ini file of the resource is located

    fixedPaths: Dict[:class:`int`, Dict[:class:`str`, List[:class:`str`]]]
        The file paths to the fixed files for the resource :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the indices to the :class:`IfContentPart` that the Blend.buf file appears in the :class:`IfTemplate` for some resource
        * The inner keys are the names for the type of mod to fix to
        * The inner values are the file paths within the :class:`IfContentPart`

    origPaths: Optional[Dict[:class:`int`, List[:class:`str`]]]
        The file paths for the resource :raw-html:`<br />` :raw-html:`<br />`
        
        * The keys are the indices to the :class:`IfContentPart` that the Blend.buf file appears in the :class:`IfTemplate` for some resource
        * The values are the file paths within the :class:`IfContentPart`

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    iniFolderPath: :class:`str`
        The folder path to where the .ini file of the resource is located

    fixedPaths: Dict[:class:`int`, Dict[:class:`str`, List[:class:`str`]]]
        The file paths to the fixed files for the resource :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the indices to the :class:`IfContentPart` that the resource files appear in the :class:`IfTemplate` for some resource
        * The inner keys are the names for the type of mod to fix to
        * The inner values are the file paths within the :class:`IfContentPart`

    origPaths: Optional[Dict[:class:`int`, List[:class:`str`]]]
        The file paths to the files for the resource :raw-html:`<br />` :raw-html:`<br />`
        
        * The keys are the indices to the :class:`IfContentPart` that the files appear in the :class:`IfTemplate` for some resource
        * The values are the file paths within the :class:`IfContentPart`

    fullPaths: Dict[:class:`int`, Dict[:class:`str`, List[:class:`str`]]]
        The absolute paths to the fixed RemapBlend.buf files for the resource :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the indices to the :class:`IfContentPart` that the files appear in the :class:`IfTemplate` for some resource
        * The inner keys are the names for the type of mod to fix to
        * The inner values are the file paths within the :class:`IfContentPart`

    origFullPaths: Dict[:class:`int`, List[:class:`str`]]
        The absolute paths to the files for the resource :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the indices to the :class:`IfContentPart` that the resource files appear in the :class:`IfTemplate` for some resource
        * The values are the file paths within the :class:`IfContentPart`
    """

    def __init__(self, iniFolderPath: str, fixedPaths: Dict[int, Dict[str, List[str]]], origPaths: Optional[Dict[int, List[str]]] = None):
        self.fixedPaths = fixedPaths
        self.origPaths = origPaths
        self.iniFolderPath = iniFolderPath

        self.fullPaths = {}
        self.origFullPaths = {}

        # retrieve the absolute paths
        for partIndex, partPaths in self.fixedPaths.items():
            try:
                self.fullPaths[partIndex]
            except KeyError:
                self.fullPaths[partIndex] = {}

            for modName, paths in partPaths.items():
                self.fullPaths[partIndex][modName] = list(map(lambda path: FileService.absPathOfRelPath(path, iniFolderPath), paths))

        if (self.origPaths is not None):
            for partIndex in self.origPaths:
                paths = self.origPaths[partIndex]
                self.origFullPaths[partIndex] = list(map(lambda path: FileService.absPathOfRelPath(path, iniFolderPath), paths))


    def __iter__(self):
        for ifTemplateInd in self.fixedPaths:
            modPaths = self.fixedPaths[ifTemplateInd]

            for modName in modPaths:
                partPaths = modPaths[modName]
                partPathsLen = len(partPaths)

                for i in range(partPathsLen):
                    fixedPath = self.fixedPaths[ifTemplateInd][modName][i]
                    fullPath = self.fullPaths[ifTemplateInd][modName][i]
                    origPath = None
                    origFullPath = None

                    if (self.origPaths is not None):
                        try:
                            origPath = self.origPaths[ifTemplateInd][i]
                            origFullPath = self.origFullPaths[ifTemplateInd][i]
                        except KeyError:
                            pass

                    yield (fixedPath, fullPath, origPath, origFullPath)

    def clear(self):
        """
        Clears out all the path data stored
        """

        self.fixedPaths.clear()
        self.fullPaths.clear()
        self.origFullPaths.clear()

        if (self.origPaths is not None):
            self.origPaths.clear()
##### EndScript
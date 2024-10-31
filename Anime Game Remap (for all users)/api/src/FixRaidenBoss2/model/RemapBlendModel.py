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
from typing import Dict, Optional
##### EndExtImports

##### LocalImports
from ..tools.files.FileService import FileService
##### EndLocalImports


##### Script
# Needed data model to inject into the .ini file
class RemapBlendModel():
    """
    Contains data for fixing a particular resource in a .ini file

    Parameters
    ----------
    iniFolderPath: :class:`str`
        The folder path to where the .ini file of the resource is located

    fixedBlendPaths: Dict[:class:`int`, Dict[:class:`str`, :class:`str`]]
        The file paths to the fixed RemapBlend.buf files for the resource :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for some resource
        * The inner keys are the names for the type of mod to fix to
        * The inner values are the file paths

    origBlendPaths: Optional[Dict[:class:`int`, :class:`str`]]
        The file paths to the Blend.buf files for the resource
        :raw-html:`<br />` :raw-html:`<br />`
        The keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for some resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    iniFolderPath: :class:`str`
        The folder path to where the .ini file of the resource is located

    fixedBlendPaths: Dict[:class:`int`, Dict[:class:`str`, :class:`str`]]
        The file paths to the fixed RemapBlend.buf files for the resource :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for some resource
        * The inner keys are the names for the type of mod to fix to
        * The inner values are the file paths

    origBlendPaths: Optional[Dict[:class:`int`, :class:`str`]]
        The file paths to the Blend.buf files for the resource :raw-html:`<br />` :raw-html:`<br />`

        The keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for the resource

    fullPaths: Dict[:class:`int`, Dict[:class:`str`, :class:`str`]]
        The absolute paths to the fixed RemapBlend.buf files for the resource :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for some resource
        * The inner keys are the names for the type of mod to fix to
        * The inner values are the file paths

    origFullPaths: Dict[:class:`int`, :class:`str`]
        The absolute paths to the Blend.buf files for the resource :raw-html:`<br />` :raw-html:`<br />`

        The keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for the resource
    """

    def __init__(self, iniFolderPath: str, fixedBlendPaths: Dict[int, Dict[str, str]], origBlendPaths: Optional[Dict[int, str]] = None):
        self.fixedBlendPaths = fixedBlendPaths
        self.origBlendPaths = origBlendPaths
        self.iniFolderPath = iniFolderPath

        self.fullPaths = {}
        self.origFullPaths = {}

        # retrieve the absolute paths
        for partIndex, partPaths in self.fixedBlendPaths.items():
            try:
                self.fullPaths[partIndex]
            except KeyError:
                self.fullPaths[partIndex] = {}

            for modName, path in partPaths.items():
                self.fullPaths[partIndex][modName] = FileService.absPathOfRelPath(path, iniFolderPath)

        if (self.origBlendPaths is not None):
            for partIndex in self.origBlendPaths:
                path = self.origBlendPaths[partIndex]
                self.origFullPaths[partIndex] = FileService.absPathOfRelPath(path, iniFolderPath)
##### EndScript
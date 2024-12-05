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
from .IniResourceModel import IniResourceModel
from ..strategies.texEditors.BaseTexEditor import BaseTexEditor
##### EndLocalImports


##### Script
class IniTexModel(IniResourceModel):
    """
    This class inherits from :class:`IniResourceModel`

    Contains data for editting some texture files in a .ini file

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: for fixedPath, fixedFullPath, origPath, origFullPath in x

            Iterates over all the fixed paths to some texture within a :class:`IfContentPart`, ``x`` :raw-html:`<br />` :raw-html:`<br />`

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

        * The outer keys are the indices to the :class:`IfContentPart` that the .dds files appears in the :class:`IfTemplate` for some texture
        * The inner keys are the names for the type of mod to fix to
        * The inner values are the file paths within the :class:`IfContentPart`

    texEdits: Dict[:class:`int`, Dict[:class:`str`, List[:class:`BaseTexEditor`]]]
        The texture editors used to edit the texture :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the indices to the :class:`IfContentPart` that the .dds files appears in the :class:`IfTemplate` for some texture
        * The inner keys are the names for the type of mod to fix to
        * The inner values are the different texture editors used to the .dds files

    origPaths: Optional[Dict[:class:`int`, List[:class:`str`]]]
        The file paths for the resource :raw-html:`<br />` :raw-html:`<br />`
        
        * The keys are the indices to the :class:`IfContentPart` that the .dds files appears in the :class:`IfTemplate` for some texture
        * The values are the file paths within the :class:`IfContentPart`

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, iniFolderPath: str, fixedPaths: Dict[int, Dict[str, List[str]]], texEdits: Dict[int, Dict[str, List[BaseTexEditor]]], 
                 origPaths: Optional[Dict[int, List[str]]] = None):
        super().__init__(iniFolderPath, fixedPaths, origPaths = origPaths)
        self.texEdits = texEdits

    def clear(self):
        super().clear()
        self.texEdits.clear()
##### EndScript
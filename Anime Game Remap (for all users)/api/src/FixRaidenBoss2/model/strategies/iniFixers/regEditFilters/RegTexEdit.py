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
from typing import Optional, Dict, List, TYPE_CHECKING
##### EndExtImports

##### LocalImports
from .....tools.ListTools import ListTools
from .....tools.DictTools import DictTools
from .RegEditFilter import RegEditFilter
from ....iftemplate.IfContentPart import IfContentPart

if (TYPE_CHECKING):
    from ...ModType import ModType
    from ..GIMIObjReplaceFixer import GIMIObjReplaceFixer
##### EndLocalImports


##### Script
class RegTexEdit(RegEditFilter):
    """
    This class inherits from :class:`RegEditFilter`

    Class for editting texture .dds files to a :class:`IfContentPart`

    Parameters
    ----------
    textures: Optional[Dict[:class:`str`, List[:class:`str`]]]
        Texture .dds files to be editted from existing textures files :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the name of the type of texture files of the mod object
        * The values are the name of the registers to hold the editted textures

        eg. :raw-html:`<br />`
        ``{"NormalMap": ["ps-t1", "r13", "ps-t0"], "ShinyMetalMap": ["ps-t2"]}`` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    _regEditVals: Optional[Dict[:class:`str`, :class:`str`]]
        The texture edits to do on the current :class:`IfContentPart` being parsed :raw-html:`<br />` :raw-html:`<br />`

        The keys are the name of the registers and the values are the `section`_ names for the textures
    """

    def __init__(self, textures: Optional[Dict[str, List[str]]] = None):
        self.textures = {} if (textures is None) else textures
        self._regEditVals: Dict[str, str] = None

    @property
    def textures(self) -> Dict[str, List[str]]:
        """
        Texture .dds files to be editted from existing textures files :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the name of the type of texture files of the mod object
        * The values are the name of the registers to hold the editted textures

        eg. :raw-html:`<br />`
        ``{"NormalMap": ["ps-t1", "r13", "ps-t0"], "ShinyMetalMap": ["ps-t2"]}``

        :getter: Retrieves the texture .dds files to be editted by register
        :setter: Sets the textures to be editted
        :type: Dict[:class:`str`, List[:class:`str`]]
        """

        return self._textures
    
    @textures.setter
    def textures(self, newTextures: Dict[str, List[str]]):
        self._textures = {}

        for texName in newTextures:
            self._textures[texName] = ListTools.getDistinct(newTextures[texName], keepOrder = True)

    def clear(self):
        self._regEditVals = None

    # _addTexEditCalledResources(part, result, regTexEditResult, oldSection, objName, reg, texTypeName, oldModName, newModeName, fixer): 
    #   Adds in the new editted resources section name into 'result'
    def _addTexEditCalledResources(self, part: IfContentPart, result: Dict[str, str], objName: str, reg: str, texTypeName: str, 
                                   oldModName: str, newModName: str, fixer: "GIMIObjReplaceFixer"):
        if (reg not in part):
            return

        # get the new registers for the editted resource
        texNewRegs = None
        try:
            texNewRegs = self.textures[texTypeName]
        except KeyError:
            return
        
        # get the current referenced resource by the editted texture
        currentRegVals = ListTools.getDistinct(part.getVals(reg), keepOrder = True)
        if (not currentRegVals):
            return
        currentRegResource = currentRegVals[-1]
        
        # get the name for the editted texture resource section
        texRemapFixName = None
        try:
            fixer._texEditRemapNames[currentRegResource]
        except KeyError:
            fixer._texEditRemapNames[currentRegResource] = {}

        try:
            texRemapFixName = fixer._texEditRemapNames[currentRegResource][texTypeName]
        except KeyError:
            texRemapFixName = fixer.getTexResourceRemapFixName(texTypeName, oldModName, newModName, objName, addInd = True)
            fixer._texEditRemapNames[currentRegResource][texTypeName] = texRemapFixName

        for newReg in texNewRegs:
            result[newReg] = texRemapFixName
            fixer._currentRegTexEdits[newReg] = (texTypeName, currentRegResource)
    
    def _editReg(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer") -> IfContentPart:
        texEdits = None
        try:
            texEdits = fixer._parser.texEdits[obj]
        except KeyError:
            return part

        self._regEditVals = {}
        DictTools.forDict(texEdits, ["reg", "texName"], 
                          lambda keys, values: self._addTexEditCalledResources(part, self._regEditVals, obj, keys["reg"], keys["texName"], modType.name, fixModName, fixer))
        part.replaceVals(self._regEditVals, addNewKVPs = True)
        return part
    
    def handleTexAdd(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer"):
        return
    
    def handleTexEdit(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer"):
        if (self._regEditVals is not None):
            fixer._currentTexEditRegs.update(set(self._regEditVals.keys()))
##### EndScript

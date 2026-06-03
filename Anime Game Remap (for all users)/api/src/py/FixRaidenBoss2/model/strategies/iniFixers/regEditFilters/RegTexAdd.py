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
from typing import Optional, Dict, Tuple, TYPE_CHECKING
##### EndExtImports

##### LocalImports
from .RegEditFilter import RegEditFilter
from ....iftemplate.IfContentPart import IfContentPart
from ...texEditors.TexCreator import TexCreator

if (TYPE_CHECKING):
    from ...ModType import ModType
    from ..GIMIObjReplaceFixer import GIMIObjReplaceFixer
##### EndLocalImports


##### Script
class RegTexAdd(RegEditFilter):
    """
    This class inherits from :class:`RegEditFilter`

    Class for adding new texture .dds files to a :class:`IfContentPart`

    Parameters
    ----------
    textures: Optional[Dict[:class:`str`, Dict[:class:`str`, Tuple[:class:`str`, :class:`TexCreator`, :class:`bool`]]]]
        New texture .dds files to be created :raw-html:`<br />` :raw-html:`<br />`

        * The outer key are the names of the mod object
        * The inner keys are the names of the register
        * The inner values contanis:
            #. The name of the type of texture file
            #. The object that will create the new texture file

        eg. :raw-html:`<br />`
        ``{"head": {"ps-t1": ("EmptyNormalMap", :class:`TexCreator`(4096, 1024))}, "body": {"ps-t3": ("NewLightMap", :class:`TexCreator`(1024, 1024, :class:`Colour`(0, 128, 0, 255))), "ps-t0": ("DummyShadowRamp", :class:`Colour`())}}`` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    mustAdd: :class:`bool`
        Whether the texture files will still be created for a particular :class:`IfContentPart` even if the corresponding register for the texture file does not exist within that particular :class:`IfContentPart`

    Attributes
    ----------
    textures: Dict[:class:`str`, Dict[:class:`str`, Tuple[:class:`str`, :class:`TexCreator`]]]
        New texture .dds files to be created :raw-html:`<br />` :raw-html:`<br />`

        * The outer key are the names of the mod object
        * The inner keys are the names of the register
        * The inner values contanis:
            #. The name of the type of texture file
            #. The object that will create the new texture file

        eg. :raw-html:`<br />`
        ``{"head": {"ps-t1": ("EmptyNormalMap", :class:`TexCreator`(4096, 1024))}, "body": {"ps-t3": ("NewLightMap", :class:`TexCreator`(1024, 1024, :class:`Colour`(0, 128, 0, 255))), "ps-t0": ("DummySshadowRamp", :class:`Colour`())}}``

    mustAdd: :class:`bool`
        Whether the texture files will still be created for a particular :class:`IfContentPart` even if the corresponding register for the texture file does not exist within that particular :class:`IfContentPart`

    _regAddVals: Optional[Dict[:class:`str`, :class:`str`]]
        The texture additions to do on the current :class:`IfContentPart` being parsed :raw-html:`<br />` :raw-html:`<br />`

        The keys are the name of the registers and the values are the `section`_ names for the textures
    """

    def __init__(self, textures: Optional[Dict[str, Dict[str, Tuple[str, TexCreator]]]] = None, mustAdd: bool = True):
        self.textures = {} if (textures is None) else textures
        self.mustAdd = mustAdd
        self._regAddVals: Optional[Dict[str, str]] = None

    def clear(self):
        self._regAddVals = None
    
    def _editReg(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer") -> IfContentPart:
        texAdds = None
        try:
            texAdds = self.textures[obj]
        except KeyError:
            return part

        self._regAddVals = {}
        for reg in texAdds:
            texAddData = texAdds[reg]
            texTypeName = texAddData[0]

            name = None
            try:
                fixer._texAddRemapNames[texTypeName]
            except KeyError:
                fixer._texAddRemapNames[texTypeName] = {}

            try:
                name = fixer._texAddRemapNames[texTypeName][obj]
            except KeyError:
                name = fixer.getTexResourceRemapFixName(texTypeName, modType.name, fixModName, obj)
                fixer._texAddRemapNames[texTypeName][obj] = name

            self._regAddVals[reg] = name

        part.replaceVals(self._regAddVals, addNewKVPs = self.mustAdd)
        return part
    
    def handleTexAdd(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer"):
        if (self._regAddVals is not None):
            fixer._currentTexAddsRegs.update(set(self._regAddVals.keys()))
    
    def handleTexEdit(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer"):
        if (self._regAddVals is not None):
            fixer._currentTexEditRegs = fixer._currentTexEditRegs.difference(set(self._regAddVals.keys()))
##### EndScript

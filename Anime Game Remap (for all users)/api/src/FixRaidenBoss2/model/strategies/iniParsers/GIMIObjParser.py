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
import re
import copy
from typing import TYPE_CHECKING, Set, Dict, Optional, Union, Tuple, List, Callable, Any
##### EndExtImports

##### LocalImports
from ....constants.GenericTypes import Pattern
from ....tools.ListTools import ListTools
from ....tools.TextTools import TextTools
from .GIMIParser import GIMIParser
from ...IniSectionGraph import IniSectionGraph
from ..texEditors.BaseTexEditor import BaseTexEditor
from ....tools.DictTools import DictTools
from ...iftemplate.IfContentPart import IfContentPart
from ...iniresources.IniTexModel import IniTexModel

if (TYPE_CHECKING):
    from ...files.IniFile import IniFile
##### EndLocalImports


##### Script
class GIMIObjParser(GIMIParser):
    """
    This class inherits from :class:`GIMIParser`

    Parses a .ini file used by a GIMI related importer and parses section's related to a specific mod object (head, body, dress, etc...)

    .. note::
        For the specific names of the objects for a particular mod, please refer to `GIMI Assets`_

    Parameters
    ----------
    iniFile: :class:`IniFile`
        The .ini file to parse

    objs: Set[:class:`str`]
        The specific mod objects to keep track of

    texEdits: Optional[Dict[:class:`str`, Dict[:class:`str`, Dict[:class:`str`, :class:`BaseTexEditor`]]]]
        texture resource `sections`_ that require to be editted

        * The outer keys ares the name of the mod object the texture resource belongs in
        * The second outer keys are the name of the register the texture resource belongs in
        * The inner keys are the names of the type of texture files that are editted
        * The inner value is the editor for changing the texture files

        .. note::
            The new names of the texture files to be editted should be all unique

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    objGraphs: Dict[:class:`str`, :class:`IniSectionGraph`]
        The different `sections`_ related to each mod object :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the objects and the values are the graphs related to each object

    texGraphs: Dict[:class:`str`, Dict[:class:`str`, :class:`IniSectionGraph`]]
        The different `sections`_ related to the textures to be editted :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the name of the mod object
        * The inner keys are the name of the register within the mod object
        * The inner value are the graphs for each register specified

    _objSearchPatterns: Dict[:class:`str`, `Pattern`]
        The Regex patterns used to find the roots of the `sections`_ related to each mod object :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the objects and the values are the Regex patterns

    _objRootSections: Dict[:class:`str`, Set[:class:`str`]]
        The root `sections`_ for each mod object :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the objects and the values are the names of the `sections`_

    texEditRegs: Dict[:class:`str`, Tuple[:class:`str`, :class:`str`]]
        The corresponding register for a particular texture resource to be editted :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the names of the type of texture resource to edit
        * The values contains info about the corresponding register for the texture. The tuple contains:
            #. The name of the mod object the texture resource belongs to
            #. The name of the register that holds the texture
    """

    def __init__(self, iniFile: "IniFile", objs: Set[str], texEdits: Optional[Dict[str, Dict[str, Dict[str, BaseTexEditor]]]] = None):
        super().__init__(iniFile)
        self.objGraphs: Dict[str, IniSectionGraph] = {}
        self.texGraphs: Dict[str, Dict[str, IniSectionGraph]] = {}
        self._objSearchPatterns: Dict[str, Pattern] = {}
        self._objRootSections: Dict[str, Set[str]] = {}
        self.texEditRegs: Dict[str, Tuple[str, str]] = {}
        self._objs = objs
        self.texEdits = {} if texEdits is None else texEdits
        self._objs = copy.deepcopy(self._objs)

    @property
    def objs(self):
        """
        The specific mod objects to keep track of

        :getter: Returns the names of the mod objects
        :setter: Sets the new names for the mod objects to keep track of
        :type: Set[:class:`str`]
        """

        return self._objs
    
    @objs.setter
    def objs(self, newObjs: Set[str]):
        self._objs = copy.deepcopy(newObjs)
        self._objs = self._objs.union(set(self.texEdits.keys()))
        self.clear()

    @property
    def texEdits(self):
        """
        texture resource `sections`_ that require to be editted

        * The outer keys ares the name of the mod object the texture resource belongs in
        * The second outer keys are the name of the register the texture resource belongs in
        * The inner keys are the names of the type of texture files that are editted
        * The inner value is the editor for changing the texture files

        :getter: Returns the specific registers to have their textures editted
        :setter: Sets the new registers to have their textures editted
        :type: Dict[:class:`str`, Dict[:class:`str`, Dict[:class:`str`, :class:`BaseTexEditor`]]]
        """

        return self._texEdits

    def _getTexEditRegs(self, result: Dict[str, Tuple[str, str]], keys: Dict[str, str], values: Dict[str, Union[BaseTexEditor, Dict[str, BaseTexEditor], Dict[str, Dict[str, BaseTexEditor]]]]):
        result[keys["tex"]] = (keys["modObj"], keys["reg"])
    
    @texEdits.setter
    def texEdits(self, newTexEdits: Dict[str, Dict[str, Dict[str, BaseTexEditor]]]):
        self._texEdits = newTexEdits
        self.texEditRegs = {}
        DictTools.forDict(self._texEdits, ["modObj", "reg", "tex"], lambda keys, values: self._getTexEditRegs(self.texEditRegs, keys, values))

        self._objs = self._objs.union(set(self.texEdits.keys()))
        self.clear()

    def _makeTexModels(self, texName: str, texGraph: IniSectionGraph, texEditor: BaseTexEditor, getFixedFile: Optional[Callable[[str], str]] = None) -> Dict[str, Dict[str, IniTexModel]]:
        """
        Creates all the data needed for fixing the ``[Resource.*]`` `sections`_ related to texture files in the .ini file

        Parameters
        ----------
        texName: :class:`str`
            The name for the type of texture file to edit

        texGraph: :class:`IniSectionGraph`
            The graph of `sections`_ for the particular type of textures

        getFixedFile: Optional[Callable[[:class:`str`], :class:`str`]]
            The function for transforming the file path of a found from the texture .dds file into a .*RemapFix.dds file :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will use :meth:`IniFile.getFixedTexFile` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Dict[:class:`str`, Dict[:class:`str`, :class:`IniTexModel`]]
            The data for fixing the resource `sections`_

            * The outer keys are the names for the particular type of texture files
            * The inner keys are the names for the resrouce `sections`_
            * The values are the required data for fixing the `sections`_
        """

        if (getFixedFile is None):
            getFixedFile = self._iniFile.getFixedTexFile

        texComands = texGraph.sections
        for sectionName in texComands:
            texIfTemplate = texComands[sectionName]
            texModel =  self._iniFile.makeTexModel(texIfTemplate, self._modsToFix, texEditor, getFixedFile = getFixedFile) 

            try:
                self._iniFile.texEditModels[texName]
            except KeyError:
                self._iniFile.texEditModels[texName] = {}

            self._iniFile.texEditModels[texName][sectionName] = texModel

        return self._iniFile.texEditModels

    def clear(self):
        super().clear()

        # reset the search patterns
        self._objSearchPatterns.clear()
        for obj in self._objs:
            capitalizedObj = TextTools.capitalize(obj.lower())
            self._objSearchPatterns[obj] = re.compile(r"^TextureOverride.*" + capitalizedObj + "$")

        # reset the graphs for the objects
        self.objGraphs.clear()
        for obj in self._objs:
            self.objGraphs[obj] = IniSectionGraph(set(), {})

        # reset the roots of each section
        self._objRootSections.clear()
        for obj in self._objs:
            self._objRootSections[obj] = set()

        # reset the graphs for each texture resource
        self.texGraphs.clear()
        for obj in self._texEdits:
            objRegs = self._texEdits[obj]
            self.texGraphs[obj] = {}

            for reg in objRegs:
                self.texGraphs[obj][reg] = IniSectionGraph(set(), {})
    
    # _getCurrentObjResources(part, objRegNames): Retrieves the desired resources from the registers
    #   specified at 'objRegNames' from 'part'
    def _getCurrentObjResources(self, part: IfContentPart, objRegNames: Set[str]) -> Dict[str, str]:
        result = DictTools.filter(part.src, lambda partKey, partVal: partKey in objRegNames)
        for reg in result:
            result[reg] = set(map(lambda valData: valData[1], result[reg]))
        return result

    def parse(self):
        super().parse()

        # retrieve the roots for each object
        for section in self._iniFile.sectionIfTemplates:
            for objName in self._objSearchPatterns:
                pattern = self._objSearchPatterns[objName]
                if (pattern.match(section)):
                    self._objRootSections[objName].add(section)
                    break

        # get the sections for each object
        for objName in self.objGraphs:
            objGraph = self.objGraphs[objName]
            objGraph.build(newTargetSections = self._objRootSections[objName], newAllSections = self._iniFile.sectionIfTemplates)

        # get the sections for each texture to be editted
        for objName in self._texEdits:
            objRegNames = set(self._texEdits[objName].keys())
            objGraph = self.objGraphs[objName]
            objResources = {}

            self._iniFile.getResources(objGraph, lambda part: set(part.src.keys()).intersection(objRegNames), 
                                       lambda part: self._getCurrentObjResources(part, objRegNames),
                                       lambda resource, part: DictTools.update(objResources, resource, 
                                                                               combineDuplicate = lambda val1, val2: val1.union(val2)))
            
            # build the graphs for each register
            for reg in objResources:
                objResources[reg] = set(objResources[reg])
                texGraph = self.texGraphs[objName][reg]
                texGraph.build(newTargetSections = objResources[reg], newAllSections = self._iniFile.sectionIfTemplates)

                # build the models for each texture type
                texEditors = self._texEdits[objName][reg]
                for texName in texEditors:
                    self._makeTexModels(texName, texGraph, texEditors[texName])

    # _getTexItem(texName, texItems): Retrieves the corresponding item from 'texItems' based off 'texName'
    def _getTexItem(self, texName: str, texItems: Dict[str, Dict[str, Any]]) -> Optional[Any]:
        texKeys = None
        try:
            texKeys = self.texEditRegs[texName]
        except KeyError:
            return None
        
        try:
            return texItems[texKeys[0]][texKeys[1]]
        except KeyError:
            return None

    def getTexEditor(self, texName: str) -> Optional[BaseTexEditor]:
        """
        Retrieves the corresponding :class:`BaseTexEditor` based on 'texName'

        Parameters
        ----------
        texName: :class:`str`
            The name to the type of texture file to be editted

        Returns
        -------
        Optional[:class:`BaseTexEditor`]
            The found texture editor
        """

        texEditors = self._getTexItem(texName, self._texEdits)
        if (texEditors is None):
            return None
        
        try:
            return texEditors[texName]
        except KeyError:
            return None

    def getTexGraph(self, texName: str) -> Optional[IniSectionGraph]:
        """
        Retrieves the corresponding :class:`IniSectionGraph` based on 'texName'

        Parameters
        ----------
        texName: :class:`str`
            The name to the type of texture file to be editted

        Returns
        -------
        Optional[:class:`IniSectionGraph`]
            The found section graph
        """

        texGraphKeys = None
        try:
            texGraphKeys = self.texEditRegs[texName]
        except KeyError:
            return None
        
        try:
            return self.texGraphs[texGraphKeys[0]][texGraphKeys[1]]
        except KeyError:
            return None
        
    def getTexGraphs(self, texNames: List[str]) -> List[IniSectionGraph]:
        """
        Retrieves the corresponding `section`_ graphs based on 'texNames'

        Parameters
        ----------
        texNames: List[:class:`str`]
            The names to the type of texture files to be editted

        Returns
        -------
        List[:class:`IniSectionGraph`]
            The found `section`_ graphs
        """

        result = []
        texNames = ListTools.getDistinct(texNames, keepOrder = True)

        for texName in texNames:
            currentResult = self.getTexGraph(texName)
            if (currentResult is not None):
                result.append(currentResult)

        return result
##### EndScript
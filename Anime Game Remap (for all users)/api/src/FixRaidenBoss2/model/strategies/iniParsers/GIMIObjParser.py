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
from typing import TYPE_CHECKING, Set, Dict
##### EndExtImports

##### LocalImports
from ....constants.GenericTypes import Pattern
from ....tools.TextTools import TextTools
from .GIMIParser import GIMIParser
from ...IniSectionGraph import IniSectionGraph

if (TYPE_CHECKING):
    from ...IniFile import IniFile
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

    Attributes
    ----------
    objGraphs: Dict[:class:`str`, :class:`IniSectionGraph`]
        The different `sections`_ related to each mod object :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the objects and the values are the graphs related to each object

    _objSearchPatterns: Dict[:class:`str`, `Pattern`]
        The Regex patterns used to find the roots of the `sections`_ related to each mod object :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the objects and the values are the Regex patterns

    _objRootSections: Dict[:class:`str`, Set[:class:`str`]]
        The root `sections`_ for each mod object :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the objects and the values are the names of the `sections`_
    """

    def __init__(self, iniFile: "IniFile", objs: Set[str]):
        super().__init__(iniFile)
        self.objGraphs: Dict[str, IniSectionGraph] = {}
        self._objSearchPatterns: Dict[str, Pattern] = {}
        self._objRootSections: Dict[str, Set[str]] = {}
        self.objs = objs

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
        self._objs = set()
        for obj in newObjs:
            self._objs.add(obj.lower())

        self.clear()


    def clear(self):
        super().clear()

        # reset the search patterns
        self._objSearchPatterns.clear()
        for obj in self._objs:
            capitalizedObj = TextTools.capitalize(obj)
            self._objSearchPatterns[obj] = re.compile(r"^TextureOverride.*" + capitalizedObj + "$")

        # reset the graphs
        self.objGraphs.clear()
        for obj in self._objs:
            self.objGraphs[obj] = IniSectionGraph(set(), {})

        # reset the roots of each section
        self._objRootSections.clear()
        for obj in self._objs:
            self._objRootSections[obj] = set()

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
##### EndScript
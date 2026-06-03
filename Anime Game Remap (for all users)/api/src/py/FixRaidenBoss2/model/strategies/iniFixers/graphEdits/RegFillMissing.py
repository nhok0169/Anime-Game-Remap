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
from typing import Callable, Union, List, Tuple, TYPE_CHECKING, Any
##### EndExtImports

##### LocalImports
from .....constants.DownloadMode import DownloadMode
from .....constants.RegFillMissingMode import RegFillMissingMode
from ....iftemplate.IfContentPart import IfContentPart
from ....IniSectionGraph import IniSectionGraph
from .BaseIniGraphEdit import BaseIniGraphEdit

if (TYPE_CHECKING):
    from ....files.IniFile import IniFile
    from ...ModType import ModType
##### EndLocalImports


##### Script
class RegFillMissing(BaseIniGraphEdit):
    """
    This class inherits from :class:`BaseIniGraphEdit`

    Fills the :class:`IniContentPart`s of some caller/callee graph that is missing a particular register

    Parameters
    ----------
    reg: :class:`str`
        The register to search for

    fillMissing: Union[:class:`str`, List[Tuple[:class:`str`, :class:`str`]], Callable[[:class:`IfContentPart`], Any]]
        How to fill in :class:`IniContentPart`s with their corresponding values :raw-html:`<br />` :raw-html:`<br />`

        If this argument is a string, will add the following line to: ``reg = fillMissing``
        If this argument is a list of tuples, will add the `KVPs`_ specified by each tuple into the missing part
        Otherwise, will modify the missing part according to the specified function

    fillMode: Optional[:class:`RegFillMissingMode`]
        What mode used to search and fill the missing register

    dependOnDownload: :class:`bool`
        Whether the editting is dependent on :attr:`IniFile.downloadMode` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    Attributes
    ----------
    reg: :class:`str`
        The register to search for

    fillMissing: Union[:class:`str`,List[Tuple[:class:`str`, :class:`str`]], Callable[[:class:`IfContentPart`], Any]]
        How to fill in :class:`IniContentPart`s with their corresponding values :raw-html:`<br />` :raw-html:`<br />`

        If this argument is a string, will add the following line to: ``reg = fillMissing``
        If this argument is a list of tuples, will add the `KVPs`_ specified by each tuple into the missing part
        Otherwise, will modify the missing part according to the specified function

    fillMode: :class:`RegFillMissingMode`
        What mode used to search and fill the missing register :raw-html:`<br />` :raw-html:`<br />`

        **Default**: :attr:`RegFillMissingMode.FillMissing`

    dependOnDownload: :class:`bool`
        Whether the editting is dependent on :attr:`IniFile.downloadMode`
    """

    def __init__(self, reg: str, fillMissing: Union[str, List[Tuple[str, str]], Callable[[IfContentPart], IfContentPart]], 
                 fillMode: RegFillMissingMode = RegFillMissingMode.FillMissing, dependOnDownload: bool = False):

        self.reg = reg
        self.fillMode = fillMode
        self.fillMissing = fillMissing
        self.dependOnDownload = dependOnDownload

    def _getFillMissingFunc(self, newFillMissing: Union[str, List[Tuple[str, str]], Callable[[IfContentPart], Any]], toFront: bool = False) -> Callable[[IfContentPart], Any]:
        result = None

        if (isinstance(newFillMissing, str) and not toFront):
            result = lambda part: part.addKVP(self.reg, newFillMissing)
        elif (isinstance(newFillMissing, str)):
            result = lambda part: part.addKVPToFront(self.reg, newFillMissing)
        elif (isinstance(newFillMissing, list) and not toFront):
            result = lambda part: part.addKVPs(newFillMissing)
        elif (isinstance(newFillMissing, list)):
            result = lambda part: part.addKVPsToFront(newFillMissing)
        else:
            result = newFillMissing

        return result

    def editFromIni(self, graph: IniSectionGraph, ini: "IniFile", modType: "ModType", modName: str = ""):
        if (not self.dependOnDownload):
            return self.edit(graph)
        
        if (ini.downloadMode == DownloadMode.Disabled):
            return graph

        if (ini.downloadMode == DownloadMode.Always):
            graph.normalize()

        return self.edit(graph, modType, modName = modName)
    
    @classmethod
    def fillMissingGraph(cls, graph: IniSectionGraph, reg: str, fillMissing: Callable[[IfContentPart], Any]) -> IniSectionGraph:
        """
        Fills the :class:`IfContentPart`s from 'graph' that are missing 'reg'

        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to search

        reg: :class:`str`
            The register to search

        fillMissing: Callable[[:class:`IfContentPart`], Any]
            The function to modify the parts that are missing the desired registers

        Returns
        -------
        :class:`IniSectionGraph`
            The modified graph with filled parts
        """

        parts = graph.getKeyMissingParts(reg)
        partVisited = set()

        for sectionName in parts:
            for part in parts[sectionName]:
                if (part not in partVisited):
                    fillMissing(part)
                    partVisited.add(part)

    @classmethod
    def addCover(cls, graph: IniSectionGraph, reg: str, fillMissing: Callable[[IfContentPart], Any]) -> IniSectionGraph:
        """
        Fill the top :class:`IfContentPart`s at the roots of 'graph' if 'reg' is missing in some :class:`IfContentPart` of 'graph'

        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to search

        reg: :class:`str`
            The register to search

        fillMissing: Callable[[:class:`IfContentPart`], Any]
            The function to modify the parts that are missing the desired registers

        Returns
        -------
        :class:`IniSectionGraph`
            The modified graph with filled parts
        """

        covered = graph.rootsAreFullyCovered(reg)

        needCover = False
        for sectionName in covered:
            if (not covered[sectionName]):
                needCover = True
                break

        if (not needCover):
            return
        
        rootSections = graph.getRootSections()
        for section in rootSections:
            topPart = section.addTopContentPart()
            fillMissing(topPart)

    def edit(self, graph: IniSectionGraph, modType: "ModType", modName: str = "") -> IniSectionGraph:
        isCoverMode = self.fillMode == RegFillMissingMode.TopdownCover
        fillMissingFunc = self._getFillMissingFunc(self.fillMissing, toFront = isCoverMode)

        if (isCoverMode):
            self.addCover(graph, self.reg, fillMissingFunc)
        elif (self.fillMode == RegFillMissingMode.FillMissing):
            self.fillMissingGraph(graph, self.reg, fillMissingFunc)

        return graph
##### EndScript
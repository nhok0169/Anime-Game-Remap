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
from typing import List, TYPE_CHECKING, Tuple, Any, Union
##### EndExtImports

##### LocalImports
from ..BaseIniPartEdit import BaseIniPartEdit
from ....IniGraphGroup import IniGraphGroup
from ....IniSectionGraph import IniSectionGraph

if (TYPE_CHECKING):
    from ....files.IniFile import IniFile
    from ...ModType import ModType
##### EndLocalImports


##### Script
class BaseIniGraphGroupEdit(BaseIniPartEdit):
    """
    This class inherits from :class:`BaseIniPartEdit`

    Base class for a filter that edits a group of caller/callee graphs across many .ini files
    """

    def editFromIni(self, graphGroups: List[IniGraphGroup], ini: "IniFile", modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        """
        Edits a group of caller/callee graphs with state info from 'ini'

        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file

        ini: :class:`IniFile`
            The associated original .ini file

        modType: :class:`ModType`
            The type of mod to fix

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns 
        -------
        List[:class:`IniGraphGroup`]
            The resultant group of graphs that got editted
        """

        return self.edit(graphGroups, modType, modName = modName)

    def edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        """
        Edits a group of caller/callee graphs

        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file

        modType: :class:`ModType`
            The type of mod to fix

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns 
        -------
        List[:class:`IniGraphGroup`]
            The resultant group of graphs that got editted
        """

        pass

    @classmethod
    def getGraph(cls, graphGroups: List[IniGraphGroup], id: Tuple[int, str, str], errorOnNotFound: bool = True, default: Any = None) -> Union[IniSectionGraph, Any]:
        """
        Retrieves the corresponding graph from a group of graphs

        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file

        id: Tuple[:class:`int`, :class:`str`, :class:`str`]
            The id to retrieve the graph. The tuple contains: :raw-html:`<br />` :raw-html:`<br />`

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        errorOnNotFound: :class:`bool`  
            If no graphs are found, whether to raise an exception

        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if no graphs are found

        Raises
        ------
        :class:`KeyError`
            If no graphs are found

        Returns
        -------
        Union[:class:`IniSectionGraph`, Any]
            Either the found graph or the value specified at 'default', if no keywords were found and 'errorOnNotFound' is set to ``False``
        """

        iniInd, comp, obj = id
        result = None
        
        if (iniInd < len(graphGroups)):
            graphGroup = graphGroups[iniInd]
            result = graphGroup.graphs.get((comp, obj))

        if (result is None and errorOnNotFound):
            raise KeyError(f"No .ini graph found by the key: {id}")
        elif (result is None):
            return default
        
        return result

    @classmethod
    def addGraph(cls, graphGroups: List[IniGraphGroup], id: Tuple[int, str, str], graph: IniSectionGraph) -> bool:
        """
        Adds a graph to the group of graphs

        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file

        id: Tuple[:class:`int`, :class:`str`, :class:`str`]
            The id to retrieve the graph. The tuple contains: :raw-html:`<br />` :raw-html:`<br />`

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        graph: :class:`IniSectionGraph`
            the graph to add

        Returns
        -------
        :class:`bool`
            Whether the graph has been added
        """

        iniInd, comp, obj = id
        if (iniInd >= len(graphGroups)):
            return False

        graphGroup = graphGroups[iniInd]
        modObj = (comp, obj)
        graphGroup.addGraph(modObj, graph)
        return True
##### EndScript
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
from typing import Dict, Tuple, Optional, Any
##### EndExtImports

##### LocalImports
from .IniSectionGraph import IniSectionGraph
##### EndLocalImports


##### Script
class IniGraphGroup():
    """
    A class to represent a group of caller/callee graphs within a .ini file

    Parameters
    ----------
    graphs: Optional[Dict[Tuple[:class:`str`, :class:`str`], :class:`IniSectionGraph`]]
        The group of graphs :raw-html:`<br />` :raw-html:`<br />`

        * The keys contain the name of the component and the name of the mod object
        * The values are the associated graph

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    graphs: Optional[Dict[Tuple[:class:`str`, :class:`str`], :class:`IniSectionGraph`]]
        The group of graphs :raw-html:`<br />` :raw-html:`<br />`

        * The keys contain the name of the component and the name of the mod object
        * The values are the associated graph
    """

    def __init__(self, graphs: Optional[Dict[Tuple[str, str], IniSectionGraph]] = None):
        self.graphs = graphs if (graphs is not None) else {}

    def addGraph(self, modObj: Tuple[str, str], graph: IniSectionGraph):
        """
        Adds a new graph

        Parameters
        ----------
        modObj: Tuple[:class:`str`, :class:`str`]
            The associated component and mod object for the graph

        graph: :class:`IniSectionGraph`
            The new graph to add
        """

        self.graphs[modObj] = graph

    def removeGraph(self, modObj: Tuple[str, str]) -> Optional[IniSectionGraph]:
        """
        Removes a graph based on the specified component and mod object

        Parameters
        ----------
        modObj: Tuple[:class:`str`, :class:`str`]
            The name of the component and mod object

        Returns
        -------
        Optional[:class:`IniSectionGraph`]
            The associated graph, if removed
        """

        return self.graphs.pop(modObj, None)
    
    def toStr(self, autoindent: bool = True) -> str:
        """
        Converts all the `sections`_ in the  group of graphs to a string

        Parameters
        ----------
        autoIndent: :class:`bool`
            Whether to compute the proper tab indent for the `section`_ :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        :class:`str`
            The string representation 
        """

        result = []
        for modObj in self.graphs:
            currentResult = self.graphs[modObj].toStr(autoindent = autoindent)
            if (currentResult):
                result.append(currentResult)

        return "\n\n".join(result)
##### EndScript
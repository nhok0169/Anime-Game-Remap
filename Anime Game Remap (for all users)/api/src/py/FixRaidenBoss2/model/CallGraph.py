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
from typing import Any, Dict, List, Optional, Set
##### EndExtImports

##### CppLocalImports
from ..core import IfContentPart
##### EndCppLocalImports

##### LocalImports
from ..constants.IniConsts import IniKeywords
##### EndLocalImports


##### Script
class CallGraph():
    """
    The result of :meth:`IniSectionGraph.buildCallGraph` -- a `call graph`_ over the :class:`IfContentPart`\\s
    of an :class:`IniSectionGraph`, suitable for the `dataflow analysis`_ tools at :class:`GraphTools` :raw-html:`<br />` :raw-html:`<br />`

    Nodes are either the ``id()`` of a real :class:`IfContentPart`, or a virtual ``("exit", id(part))`` node
    (only present for a part that actually makes a ``run =`` call) representing the point control reaches once
    that call has *returned* -- see :meth:`exitNodeOf`

    Parameters
    ----------
    forwardEdges: Dict[Any, List[Any]]
        ``node -> list of the nodes that can run directly after it``

    backwardEdges: Dict[Any, List[Any]]
        The reverse of 'forwardEdges'

    partsById: Dict[:class:`int`, :class:`IfContentPart`]
        The ``id()`` of every reachable :class:`IfContentPart`, mapped to the part itself

    rootNodeIds: Set[:class:`int`]
        The ``id()`` of every part that's a genuine entry point of one of the graph's own target `section`_\\s
        (as opposed to only being reachable via a ``run =`` call) -- the boundary condition for
        :meth:`GraphTools.runForwardMustFixpoint`

    Attributes
    ----------
    forwardEdges: Dict[Any, List[Any]]
        ``node -> list of the nodes that can run directly after it``

    backwardEdges: Dict[Any, List[Any]]
        The reverse of :attr:`forwardEdges`

    partsById: Dict[:class:`int`, :class:`IfContentPart`]
        The ``id()`` of every reachable :class:`IfContentPart`, mapped to the part itself

    rootNodeIds: Set[:class:`int`]
        The ``id()`` of every part that's a genuine entry point of one of the graph's own target `section`_\\s
    """

    def __init__(self, forwardEdges: Dict[Any, List[Any]], backwardEdges: Dict[Any, List[Any]],
                 partsById: Dict[int, IfContentPart], rootNodeIds: Set[int]):
        self.forwardEdges = forwardEdges
        self.backwardEdges = backwardEdges
        self.partsById = partsById
        self.rootNodeIds = rootNodeIds

    def exitNodeOf(self, partId: int) -> Any:
        """
        Retrieves the node representing "once 'partId's own ``run =`` call (if it makes one) has returned" :raw-html:`<br />` :raw-html:`<br />`

        For a part that makes no call, this is just 'partId' itself -- there's no call to distinguish "before"
        from "after", so the part's own node already serves both purposes

        Parameters
        ----------
        partId: :class:`int`
            The ``id()`` of the :class:`IfContentPart` to look up (see :attr:`partsById`)

        Returns
        -------
        Any
            Either ``("exit", partId)`` (if the part makes a ``run =`` call) or 'partId' itself
        """

        part: Optional[IfContentPart] = self.partsById.get(partId)
        if (part is not None and part.getVals(IniKeywords.Run.value)):
            return ("exit", partId)
        return partId
##### EndScript

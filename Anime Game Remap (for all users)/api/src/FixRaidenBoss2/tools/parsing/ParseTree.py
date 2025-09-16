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
from typing import Dict, Hashable, List
##### EndExtImports

##### LocalImports
from ..nodes.ParseNode import ParseNode
##### EndLocalImports


##### Script
class ParseTree():
    """
    The generated parse tree after parsing some text

    Parameters
    ----------
    nodes: Dict[`Hashable`_, :class:`ParseNode`]
        The nodes in the tree :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids of the node and the values are the nodes

    children: Dict[`Hashable`_, List[`Hashable`]]
        The children relations of the nodes :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids of the parent nodes and the values are the ids of the children nodes

    rootId: `Hashable`_
        The id of the root node

    Attributes
    ----------
    nodes: Dict[`Hashable`_, :class:`ParseNode`]
        The nodes in the tree :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids of the node and the values are the nodes

    children: Dict[`Hashable`_, List[`Hashable`]]
        The children relations of the nodes :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids of the parent nodes and the values are the ids of the children nodes

    rootId: `Hashable`_
        The id of the root node
    """

    def __init__(self, nodes: Dict[Hashable, ParseNode], children: Dict[Hashable, List[Hashable]], rootId: Hashable):
        self.nodes = nodes
        self.children = children
        self.rootId = rootId

    @property
    def rootId(self) -> Hashable:
        """
        The id of the root node

        :getter: Retrives id of the root node
        :setter: Sets the new id of the root node
        :type: `Hashable`_
        """

        return self._rootId
    
    @rootId.setter
    def rootId(self, newRootId: Hashable):
        if (newRootId not in self.nodes):
            raise KeyError(f"The new root id, {newRootId}, does not reference an existing node in the parse tree")
        
        self._rootId = newRootId
##### EndScript

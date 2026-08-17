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
import uuid
from typing import Hashable, Dict, List, Union, Optional, Tuple, Set
##### EndExtImports

##### CppLocalImports
from ...core import IfContentPart
##### EndCppLocalImports

##### LocalImports
from ...tools.nodes.Node import Node
from .IfPredPart import IfPredPart
##### EndLocalImports


##### Script
class IfTemplateNode(Node):
    """
    This class inherits from :class:`Node`

    A node within the parse tree of the :class:`IfTemplate`. 
    This node contains a subset of the :class:`IfContentPart` from the original :class:`IfTemplate`

    .. note::
        For more details on the structure of the parse tree of an :class:`IfTemplate`, see :class:`IfTemplateTree`

    Parameters
    ----------
    id: Optional[Hashable]
        The id for the node :raw-html:`<br />` :raw-html:`<br />`

        If this argument is ``None``, then will generate the id for the node using :meth:`generateId`

    ifPredPart: Optional[:class:`IfPredPart`]
        The predicate part that is associated with this node

    Attributes
    ----------
    id: Hashable
        The id for the node

    children: Dict[Hashable, :class:`IfTemplateNode`]
        The children to this node :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids of the children nodes and the values are the corresponding nodes for the children

    parts: List[Union[:class:`IfContentPart`, :class:`IfTemplateNode`]]
        The parts of the :class:`IfTemplate` within the node
    """

    def __init__(self, id: Optional[Hashable] = None, ifPredPart: Optional[IfPredPart] = None):
        if (id is None):
            id = self.generateId()

        super().__init__(id)

        self.ifPredPart = ifPredPart
        self.parts: List[Union[IfContentPart, "IfTemplateNode"]] = []
        self.children: Dict[Hashable, "IfTemplateNode"] = {}

    @classmethod
    def generateId(self) -> Hashable:
        """
        Generates a new id for the node
        """
        return uuid.uuid4().int

    def addChild(self, node: "IfTemplateNode"):
        """
        Adds a child to the node

        Parameters
        ----------
        node: :class:`IfTemplateNode`
            The child to be added
        """

        self.parts.append(node)
        self.children[node.id] = node

    def addIfContentPart(self, part: IfContentPart):
        """
        Adds an :class:`IfContentPart` to the node

        Parameters
        ----------
        part: :class:`IfContentPart`
            The content part of the :class:`IfTemplate` to add to this node
        """

        self.parts.append(part)

    def hasKey(self, key: str) -> bool:
        """
        Purely checkes whether the key exists within the parts of the node without accounting for
        whether the key exists in other subcommands called by this node or other children nodes that have the key

        Paramters
        ---------
        key: :class:`str`
            The key to check

        Returns
        -------
        :class:`bool`
            Whether the key exists
        """

        result = False

        for part in self.parts:
            if (not isinstance(part, IfContentPart) or key not in part):
                continue

            if (key not in part):
                continue
            
            result = True
            break

        return result
    
    def getKeyPart(self, key: str) -> Optional[IfContentPart]:
        """
        Retrieves the latest :class:`IfContentPart` that contains 'key'

        Parameters
        ----------
        key: :class:`str`
            The key to find

        Returns
        -------
        Optional[:class:`IfContentPart`]
            The found part if available
        """

        partsLen = len(self.parts)

        for i in range(partsLen - 1, -1, -1):
            part = self.parts[i]

            if (not isinstance(part, IfContentPart)):
                continue
            
            if (key in part):
                return part
            
        return None
    
    def getKeyVal(self, key: str) -> Optional[str]:
        """
        Retrives the latest value that correponds to 'key'

        Parameters
        ----------
        key: :class:`str`
            The key to find

        Returns
        -------
        Optional[:class:`str`]
            The found value if available
        """

        part = self.getKeyPart(key)
        if (part is None):
            return part
        
        return part[key][-1][1]
    
    def getKeyValues(self, key: str) -> List[List[Tuple[int, str]]]:
        """
        Retrieves all the corresponding values to a certain key within the node

        Parameters
        ----------
        key: :class:`str`
            The key to find

        Returns
        -------
        List[List[Tuple[:class:`int`, :class:`str`]]]
            All the corresponding values to the key in the node :raw-html:`<br />` :raw-html:`<br />`

            * The outer elements in the list are the values for each part in the node
            * The inner elements of the list are the different instance of the `KVP`_ within each part
            * The tuple contains the order index an occurence of the `KVP`_ appears in the part and the corresponding value for the `KVP`_
        """

        result = []
        for part in self.parts:
            if (not isinstance(part, IfContentPart) or key not in part):
                continue

            result.append(part.getValsWithInds(key))

        return result
    
    def getKeyMissingPart(self, key: str) -> Tuple[Optional[IfContentPart], bool]:
        """
        Retrieves the first :class:`IfContentPart` if 'key' is not found in this node, without accounting for
        the key being in any other subcommands or other children nodes

        Parameters
        ----------
        key: :class:`str`
            The key to find

        Returns
        -------
        Tuple[Optional[:class:`IfContentPart`], :class:`bool`]
            A tuple containing:

            #. The first part found, if all the :class:`IfContent`s within the node does not contain the key
            #. Whether a :class:`IfContentPart` is found within the node
        """

        result = None
        hasContentPart = False

        for part in self.parts:
            if (not isinstance(part, IfContentPart)):
                continue

            if (not hasContentPart):
                hasContentPart = True

            if (key in part):
                result = None
                break
            
            if (result is None):
                result = part

        return (result, hasContentPart)
##### EndScript
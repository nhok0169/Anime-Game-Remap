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
from collections import deque
from typing import Optional, List
##### EndExtImports

##### LocalImports
from ...constants.IfPredPartType import IfPredPartType
from .ifTemplateNode import IfTemplateNode
from .IfTemplatePart import IfTemplatePart
from .IfContentPart import IfContentPart
##### EndLocalImports


##### Script
class IfTemplateTree():
    """
    The parse tree for some :class:`IfTemplate` :raw-html:`<br />`

    .. note::
        The parse tree for the :class:`IfTemplate` is structured such that:

        * A node conposes of :class:`IfContentPart` or other nodes
        * The children to the node occurs when the node enters a specific branching condition :raw-html:`<br />` :raw-html:`<br />`

        eg. *Suppose we have this branching structure*

        .. code-block:: ini
            :linenos:

            ...(does stuff)...
            if ...(bool)...
                if ...(bool)...
                    ...(does stuff)...
                else if ...(bool)...
                    ...(does stuff)...
                endif
            else ...(bool)...
                ...(does stuff)...
                if ...(bool)...
                    if ...(bool)...
                        ...(does stuff)...
                    endif
                    ...(does stuff)...
                endif
            endif
            ...(does stuff)...
        
        :raw-html:`<br />`

        Let `C` be some :class:`IfContentPart` (the parts that says `...(does stuff)...`)
        Let `B` be some branching point (the parts that say `if` or `else`)

        The parse tree generated for the above code would be:

        .. code-block::

                   C B B C
                     | |                       
                +----+ +----+
                |           | 
               B B         C B
               | |           |
            +--+ +--+       B C
            |       |       |
            C       C       C
    """

    def __init__(self):
        self._root: Optional[IfTemplateNode] = None

    @property
    def root(self):
        """
        The root node in the parse tree

        :getter: Retrieves the root node
        :type: :class:`IfTemplateNode`
        """

        return self._root

    def clear(self):
        """
        Clears the tree
        """

        self._root = None

    @classmethod
    def construct(cls, parts: List[IfTemplatePart]):
        """
        Constructs the parse tree

        Parameters
        ----------
        parts: List[:class:`IfTemplatePart`]
            The parts within the :class:`IfTemplate`
        """

        node = IfTemplateNode()
        root = node
        nodeStack = deque()
        partsLen = len(parts)

        for i in range(partsLen):
            part = parts[i]
            if (isinstance(part, IfContentPart)):
                node.addIfContentPart(part)
                continue

            predType = part.type

            if (predType == IfPredPartType.If):
                nodeStack.append(node)
                node = IfTemplateNode()
                continue

            isChild = bool(nodeStack)
            if (not isChild):
                continue

            parent = nodeStack[-1]
            parent.addChild(node)

            if (predType == IfPredPartType.EndIf):
                node = nodeStack.pop()
            elif (predType == IfPredPartType.Else):
                node = IfTemplateNode()

        result = cls()
        result._root = root
        return result
##### EndScript
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
import re
from typing import Optional, List
##### EndExtImports

##### LocalImports
from ...constants.IfPredPartType import IfPredPartType
from .IfTemplateNode import IfTemplateNode
from .IfTemplatePart import IfTemplatePart
from .IfContentPart import IfContentPart
from .IfPredPart import IfPredPart
##### EndLocalImports


##### Script
class IfTemplateTree():
    """
    The parse tree for some :class:`IfTemplate` :raw-html:`<br />`

    .. note::
        The parse tree for the :class:`IfTemplate` is structured such that:

        * A node conposes of :class:`IfContentPart` or other nodes
        * The children to the node occurs when the node enters a specific branching condition :raw-html:`<br />` :raw-html:`<br />`

        eg. Suppose we have this branching structure

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
                ...(does stuff)...
                if
                endif
            endif
            ...(does stuff)...
        
        :raw-html:`<br />`

        Let `C` be some :class:`IfContentPart` (the parts that says `...(does stuff)...`)
        Let `B` be some branching point (the parts that say `if` or `else`)
        Let `[...]` be some node
        Let `X` be a node without any parts

        The parse tree generated for the above code would be:

        .. code-block::

                   [C B B C]
                      | |                       
                 +----+ +----+
                 |           | 
               [B B]     [C B C B]
                | |         |   |
             +--+ +--+    [B C] X
             |       |     |
            [C]     [C]   [C]
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
                node = IfTemplateNode(ifPredPart = part)
                continue

            isChild = bool(nodeStack)
            if (not isChild):
                continue

            parent = nodeStack[-1]
            parent.addChild(node)

            if (predType == IfPredPartType.EndIf):
                node = nodeStack.pop()
            elif (predType == IfPredPartType.Else or predType == IfPredPartType.Elif):
                node = IfTemplateNode(ifPredPart = part)

        result = cls()
        result._root = root
        return result
    

class IfTemplateNonEmptyNodeTree(IfTemplateTree):
    """
    This class inherits from :class:`IfTemplateTree`

    A variation of :class:`IfTemplateTree` such that leaf nodes that do not have any parts (eg. empty conditions)
    will include a empty :class:`IfContentPart` placeholder.

    .. tip::
        See :class:`IfTemplateTree` on the basic structure of the parse tree for an :class:`IfTemplate`

    :raw-html:`<br />` :raw-html:`<br />`

    So conditions with forms of:

    .. code-block:: ini

        if
        endif

    that have the following parse subtree:

    .. code-block::

       [B]
        |
        X

    will now become:

    .. code-block:: ini

        if
            ...(does nothing)...
        endif

    with the following parse subtree:
    
    .. code-block::

       [B]
        |
       [C]

    :raw-html:`<br />` :raw-html:`<br />`

    .. note::
        eg. Suppose we have this branching structure (same structure from the example at :class:`IfTemplateTree`)

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
                ...(does stuff)...
                if
                endif
            endif
            ...(does stuff)...
        
        :raw-html:`<br />`

        Let `C` be some :class:`IfContentPart` (the parts that says `...(does stuff)...`)
        Let `B` be some branching point (the parts that say `if` or `else`)
        Let `[...]` be some node
        Let `X` be a node without any parts

        The parse tree generated for the above code would be:

        .. code-block::

                   [C B B C]
                      | |                       
                 +----+ +----+
                 |           | 
               [B B]     [C B C B]
                | |         |   |
             +--+ +--+    [B C] |
             |       |     |    |
            [C]     [C]   [C]  [C]
    """

    @classmethod
    def construct(cls, parts: List[IfTemplatePart]):
        """
        Constructs the parse tree

        .. note::
            The construction may change 'parts'

        Parameters
        ----------
        parts: List[:class:`IfTemplatePart`]
            The parts within the :class:`IfTemplate`
        """

        node = IfTemplateNode()
        root = node
        nodeStack = deque()
        partsLen = len(parts)
        depth = 0
        i = 0

        while (i < partsLen):
            part = parts[i]
            if (isinstance(part, IfContentPart)):
                node.addIfContentPart(part)
                i += 1
                continue

            predType = part.type

            if (predType == IfPredPartType.If):
                nodeStack.append(node)
                node = IfTemplateNode(ifPredPart = part)
                depth += 1
                i += 1
                continue

            isChild = bool(nodeStack)
            if (not isChild):
                i += 1
                continue

            parent = nodeStack[-1]
            parent.addChild(node)

            if (predType == IfPredPartType.EndIf):
                if (not node.parts):
                    ifContentPlaceholder = IfContentPart({}, depth)
                    parts.insert(i, ifContentPlaceholder)
                    node.addIfContentPart(ifContentPlaceholder)
                    i += 1
                    partsLen += 1

                node = nodeStack.pop()
                depth -= 1
            elif (predType == IfPredPartType.Else or predType == IfPredPartType.Elif):
                node = IfTemplateNode(ifPredPart = part)

            i += 1

        result = cls()
        result._root = root
        return result
    
class IfTemplateNormTree(IfTemplateNonEmptyNodeTree):
    """
    This class inherits from :class:`IfTemplateNonEmptyNodeTree`

    A variation of :class:`IfTemplateNonEmptyNodeTree` such that an empty ``else`` clause will be added for branches that do not end with a single ``else`` :raw-html:`<br />`

    .. tip::
        See :class:`IfTemplateTree` on the basic structure of the parse tree for an :class:`IfTemplate`

    :raw-html:`<br />` :raw-html:`<br />`

    So conditions with forms of:

    .. code-block:: ini

        if
            ...(does stuff)...
        else if
            ...(does stuff)...
        endif

    that have the following parse subtree:

    .. code-block::

          [B B]
           | |
         +-+ +-+
         |     |
        [C]   [C]

    will now become:

    .. code-block:: ini

        if
            ...(does stuff)...
        else if
            ...(does stuff)...
        else
            ...(does nothing)...
        endif

    with the following parse subtree:
    
    .. code-block::

        [B B B]
         | | |
       +-+ | +-+
       |  [C]  |
      [C]     [C]

    :raw-html:`<br />` :raw-html:`<br />`

    .. note::
        eg. Suppose we have this branching structure (same structure from the example at :class:`IfTemplateTree`)

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
                ...(does stuff)...
                if
                endif
            endif
            ...(does stuff)...
        
        :raw-html:`<br />`

        This class will turn this branching structure into:

        .. code-block:: ini
            :linenos:

            ...(does stuff)...
            if ...(bool)...
                if ...(bool)...
                    ...(does stuff)...
                else if ...(bool)...
                    ...(does stuff)...
                else
                    ...(does nothing)...
                endif
            else ...(bool)...
                ...(does stuff)...
                if ...(bool)...
                    if ...(bool)...
                        ...(does stuff)...
                    else
                        ...(does nothing)...
                    endif
                    ...(does stuff)...
                else
                    ...(does nothing)...
                endif
                ...(does stuff)...
                if
                    ...(does nothing)...
                else
                    ...(does nothing)...
                endif
            endif
            ...(does stuff)...

        Let `C` be some :class:`IfContentPart` (the parts that says `...(does stuff)...`)
        Let `B` be some branching point (the parts that say `if` or `else`)
        Let `[...]` be some node
        Let `X` be a node without any parts

        The parse tree generated for the above code would be:

        .. code-block::

                     [C B B C]
                        | |                       
                    +----+ +-------+
                    |              | 
               [B B B]         [C B B C B B]
                | | |             | |   | |
             +--+ | +-+         +-+ +-+ | +--+
             |    |   |         |     | |    |
            [C]  [C] [C]     [B B C]  | +-+  |
                              | |     |   | [C]
                            +-+ |    [C]  |
                            |   |        [C]
                           [C] [C]
    """

    @classmethod
    def construct(cls, parts: List[IfTemplatePart]):
        node = IfTemplateNode()
        root = node
        nodeStack = deque()
        partsLen = len(parts)
        elseEncountered = False
        elseEncounteredStack = deque() 
        depth = 0
        i = 0

        while (i < partsLen):
            part = parts[i]
            if (isinstance(part, IfContentPart)):
                node.addIfContentPart(part)
                i += 1
                continue

            predType = part.type

            if (predType == IfPredPartType.If):
                nodeStack.append(node)
                node = IfTemplateNode(ifPredPart = part)
                elseEncounteredStack.append(elseEncountered)
                elseEncountered = False
                depth += 1
                i += 1
                continue

            isChild = bool(nodeStack)
            if (not isChild):
                i += 1
                continue

            parent = nodeStack[-1]
            parent.addChild(node)

            if (predType == IfPredPartType.EndIf):
                node = nodeStack.pop()
                elseEncountered = elseEncounteredStack.pop()

                # construct the 'empty else' if an 'else' has not been encountered
                if (not elseEncountered):
                    linePrefix = re.match(r"^[( |\t)]*", part.pred)
                    if (linePrefix):
                        linePrefix = linePrefix.group(0)
                        linePrefixLen = len(linePrefix)
                        linePrefix = part.pred[:linePrefixLen]
                    else:
                        linePrefix = ""

                    emptyElse = IfPredPart(linePrefix + "else\n", IfPredPartType.Else)
                    emptyElseContent = IfContentPart({}, depth = depth)

                    emptyElseChild = IfTemplateNode(ifPredPart = emptyElse)
                    emptyElseChild.addIfContentPart(emptyElseContent)

                    parts.insert(i, emptyElseContent)
                    parts.insert(i, emptyElse)
                    node.addChild(emptyElseChild)

                    i += 2
                    partsLen += 2

                depth -= 1
                elseEncountered = False

            elif (predType == IfPredPartType.Else or predType == IfPredPartType.Elif):
                node = IfTemplateNode(ifPredPart = part)

                if (predType == IfPredPartType.Else):
                    elseEncounteredStack[-1] = True

            i += 1

        result = cls()
        result._root = root
        return result
##### EndScript
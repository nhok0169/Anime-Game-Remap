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
from typing import Hashable, Optional
##### EndExtImports

##### LocalImports
from ..nodes.Node import Node
from ..parsing.Token import Token
##### EndLocalImports


##### Script
class ParseNode(Node):
    """
    This class inherits from :class:`Node`

    A node within a parse tree, created from a parser that interprets some `CFG`_

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: hash(x)

            Retrieves the id of the node as the hash value

    Parameters
    ----------
    id: Hashable
        The id for the node

    prodId: Optional[`Hashable`_]
        The id for the chosen production from the `CFG`_

        .. note::
            Typically for parsers such as :class:`BaseSLR1Parser` , this id refers to the
            index of the chosen production from the given productions for the parser

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    token: Optional[:class:`Token`]
        The token that the node references :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    prodId: Optional[`Hashable`_]
        The id for the chosen production from the `CFG`_
        
    tokenType: Optional[:class:`str`]
        The type of token the node references

    token: Optional[:class:`str`]
        The token that the node references
    """

    def __init__(self, id: Hashable, prodId: Optional[Hashable] = None, token: Optional[Token] = None):
        super().__init__(id)
        self.prodId = prodId
        self.token = token
##### EndScript

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
from typing import Hashable
##### EndExtImports


##### Script
class Node():
    """
    Class for a node in a `graph`_

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: hash(x)

            Retrieves the id of the node as the hash value

    Parameters
    ----------
    id: Hashable
        The id for the node
    """

    def __init__(self, id: Hashable):
        self._id = id

    def __hash__(self):
        return self._id

    @property
    def id(self) -> Hashable:
        """
        The id of the node

        :getter: Returns the id for the node
        :type: Hashable
        """

        return self._id
##### EndScript
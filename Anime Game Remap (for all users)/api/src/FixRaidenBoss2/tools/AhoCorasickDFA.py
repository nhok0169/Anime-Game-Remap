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
from typing import Dict, Optional, Optional, List
##### EndExtImports

##### LocalImports
from ..constants.GenericTypes import T
from .Trie import Trie
##### EndLocalImports


##### Script
class AhoCorasickDFA(Trie):
    """
    This class inherits from :class:`Trie`

    The `DFA (Deterministic Finite Automaton)`_ used in the `Aho-Corasick`_ algorithm

    Parameters
    ----------
    data: Optional[Dict[:class:`str`, T]]
        Any initial data to put into the `DFA`_ :raw-html:`<br />` :raw-html:`<br />`

        The keys are the keywords to put into the `DFA`_ and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    _fail: Dict[:class:`int`, :class:`int`]
        The failure edges in the `DFA`_ :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids to the sources node of the edges and the values are the ids to the sink nodes of the edges

    _out: Dict[:class:`int`, List[:class:`int`]]
        The keywords to output at a given node :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids to the nodes and the values are the indices of the keywords in the `DFA`_ , determined from :attr:`AhoCorasickDFA._keywords` :raw-html:`<br />` :raw-html:`<br />`

        .. note::
            The list of indices for each node are sorted by the length of the keyword
    """

    def __init__(self, data: Optional[Dict[str, T]] = None):
        self._fail: Dict[int, int] = {}
        self._out: Dict[int, List[int]] = {}

        super().__init__(data = data)

    def clear(self):
        """
        Clears the `DFA`_
        """

        super().clear()
        self._fail = {}
        self._out = {}

    def build(self, data = None):
        """
        Rebuilds the `DFA`_
        """

        super().build(data)
##### EndScript

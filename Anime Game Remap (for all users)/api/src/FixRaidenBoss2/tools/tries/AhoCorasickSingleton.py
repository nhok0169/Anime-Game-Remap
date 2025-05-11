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
from typing import Dict
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import T
from .AhoCorasickBuilder import AhoCorasickBuilder
##### EndLocalImports


##### Script
class AhoCorasickSingleton():
    """
    Wrapper class to the :class:`BaseAhoCorasickDFA` that only setup the data in the `DFA`_ once
    at some point during runtime

    Parameters
    ----------
    builder: :class:`AhoCorasickBuilder`
        The builder that constructs the :class:`BaseAhoCorasickDFA`

    *args:
        Any extra arguments to provide into :meth:`AhoCorasickBuilder.build` during the initial construction of the :class:`BaseAhoCorasickDFA`

    **kwargs:
        Any extra keyword arguments to provide into :meth:`AhoCorasickBuilder.build` during the initial construction of the :class:`BaseAhoCorasickDFA`

    Attributes
    ----------
    dfa: :class:`BaseAhoCorasickDFA`
        The `DFA`_ used in the `Aho-Corasick`_ algorithm
    """

    def __init__(self, builder: AhoCorasickBuilder, *args, **kwargs):
        self.dfa = builder.build(*args, **kwargs)
        self._isSetup = False

    @property
    def isSetup(self):
        """
        Whether the data in the `DFA`_ has been setup

        :getter: Retrieves whether the data has been setup yet
        :type: :class:`bool`
        """

        return self._isSetup
    
    def reset(self):
        """
        Resets the state so that :attr:`dfa` can have its data updated
        """
        
        self._isSetup = False
    
    def setup(self, data: Dict[str, T]) -> bool:
        """
        Setup the data for the `DFA`_ , if the data has not been setup yet

        Parameters
        ----------
        data: Dict[:class:`str`, T]
            The data to pass into :meth:`BaseAhoCorasickDFA.build`

        Returns
        -------
        :class:`bool`
            Whether the data in :attr:`dfa` got updated
        """

        if (not self._isSetup):
            self._isSetup = True
            self.dfa.build(data = data)
            return True

        return False
##### EndScript
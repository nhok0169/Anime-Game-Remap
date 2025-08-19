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
from enum import Enum
from typing import Optional
##### EndExtImports

##### LocalImports
from ..tries.AhoCorasickSingleton import AhoCorasickSingleton
from ..tries.AhoCorasickBuilder import AhoCorasickBuilder
##### EndLocalImports


##### Script
EnumAhoCorasickDFAs = {}


class StrEnum(Enum):
    """
    This class inherits from: `Enum`_

    An enum that deals with faster string searching for larget sets of search values using the `Aho-Corasick`_ algorithm
    """

    def __str__(self) -> str:
        return self.value

    @classmethod
    def _buildAhocorasickDFA(cls) -> AhoCorasickSingleton:
        """
        Builds the `DFA`_ used in `Aho-Corasick`_
        """

        data = {}
        for strEnum in cls:
            data[strEnum.value] = strEnum

        result = AhoCorasickSingleton(AhoCorasickBuilder())
        result.setup(data)
        return result
    
    @classmethod
    def _setupAhocorasick(cls):
        """
        Performs any setup needed for `Aho-Corasick`_
        """

        dfa = cls._buildAhocorasickDFA()
        EnumAhoCorasickDFAs[cls] = dfa

    @classmethod
    def match(cls, name: str) -> Optional["StrEnum"]:
        """
        Searches for an exact match for a particular enum

        Paramaters
        ----------
        name: :class:`str`
            The text to match
        """

        ahoCorasickDFA = EnumAhoCorasickDFAs.get(cls)
        if (ahoCorasickDFA is None):
            cls._setupAhocorasick()

        ahoCorasickDFA = EnumAhoCorasickDFAs[cls]
        return ahoCorasickDFA.dfa.getKeyVal(name, errorOnNotFound = False)

    @classmethod
    def search(cls, txt: str) -> Optional["StrEnum"]:
        """
        Finds whether a particular enum contains the substring of 'txt'

        Parameters
        ----------
        txt: :class:`str`
            The text to find
        """

        ahoCorasickDFA = EnumAhoCorasickDFAs.get(cls)
        if (ahoCorasickDFA is None):
            cls._setupAhocorasick()

        ahoCorasickDFA = EnumAhoCorasickDFAs[cls]
        keyword, val = ahoCorasickDFA.dfa.get(txt, errorOnNotFound = False)
        return val
##### EndScript
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
from typing import Union, List, Optional
##### EndExtImports

##### LocalImports
from ..ModType import ModType
from ....tools.TextTools import TextTools
from ....tools.AhoCorasickDFA import AhoCorasickDFA
from ....tools.DFA import DFA
from .BaseIniClassifier import BaseIniClassifier
from .BaseIniClassifierBuilder import BaseIniClassifierBuilder
##### EndLocalImports


##### Script
class IniClassifier(BaseIniClassifier):
    """
    This class inherits from :class:`BaseIniClassifier`

    Class to help classify the type of mod given the mod's .ini files :raw-html:`<br />` :raw-html:`<br />`

    This classifier will read each line in the .ini file, and performs the following:

    * Keywords in a line are first quickly identified and filtered using `Aho-Corasick`_ . 
      The large majority of the lines in a .ini file will be identified through this method.
    * State information between different lines in a .ini file are stored in a `DFA`_
    * If there are any further ambiguity that keyword searching cannot solve, will perform any needed post-processing on the line (eg. regex matching). 
      Very little to no lines in a .ini file will need to resort to such method.

    Attributes
    ----------
    _keywordDFA: :class:`AhoCorasickDFA`
        The `DFA`_ that will use `Aho-Corasick`_ to quickly search/filter keywords in a line in the .ini file

    _stateDFA: :class:`DFA`
        The `DFA`_ that will store state information
    """

    def __init__(self):
        self._keywordDFA = AhoCorasickDFA()
        self._stateDFA = DFA()

    def build(self, builder: BaseIniClassifierBuilder):
        builder.build(self)

    def classify(self, iniTxt: Union[str, List[str]]):
        self._stateDFA.reset()
        if (isinstance(iniTxt, str)):
            iniTxt = TextTools.getTextLines(iniTxt)

        for line in iniTxt:
            modType = self.readLine(line)
            if (isinstance(modType, ModType)):
                return modType

    def readLine(self, line: str) -> Optional[ModType]:
        """
        Reads a single line in a .ini file

        Parameters
        ----------
        line: :class:`str`
            The line in the .ini file

        Returns
        -------
        Optional[ModType]
            The found mod type if determined from this single line
        """

        keyword, action = self._keywordDFA.getMaximal(line, errorOnNotFound = False)
        if (keyword is None):
            return None
        
        newStateId, isAccept, transitionMade = self._stateDFA.transition(keyword)
        
        isModType = isinstance(action, ModType)
        if (action is None or (isModType and not isAccept)):
            return None
        elif (isModType and isAccept):
            self._stateDFA.reset()
            return action

        result = action(line, keyword, newStateId, isAccept, transitionMade)
        if (isinstance(result, ModType)):
            self._stateDFA.reset()

        return result
##### EndScript
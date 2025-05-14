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
import re 
from typing import Union, List, Optional, Hashable, Optional, Callable, Any, Type
##### EndExtImports

##### LocalImports
from ....tools.tries.AhoCorasickBuilder import AhoCorasickBuilder
from ....tools.tries.BaseAhoCorasickDFA import BaseAhoCorasickDFA
from ....constants.IniConsts import IniKeywords
from ..ModType import ModType
from ....tools.TextTools import TextTools
from ....tools.DFA import DFA
from .BaseIniClassifier import BaseIniClassifier
from .BaseIniClassifierBuilder import BaseIniClassifierBuilder
from .states.IniClsActionArgs import IniClsActionArgs
from .states.IniClsAction import IniClsAction
from .IniClassifyStats import IniClassifyStats
from .states.IniClsTransitionVals import IniClsTransitionVals
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

    Parameters
    ----------
    builder: Optional[:class:`BaseIniClassifierBuilder`]
        The builder used to build the data within the classifier :raw-html:`<br />` :raw-html:`<br />`

        If this argument is ``None``, the constructor will not automatically build the data in the classifier and the
        user must call :meth:`build` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    ahoCorasickCls: Optional[Type[:class:BaseAhoCorasickDFA`]]
        The class implementation of `Aho-Corasick` to use :raw-html:`<br />` :raw-html:`<br />`

        If this parameter is ``None``, then will try to :class:`FastAhoCorasickDFA` if possible, otherwise
        will fall back to :class:`AhoCorasickDFA` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    builder: Optional[:class:`BaseIniClassifierBuilder`]
        The builder used to build the data within the classifier, if available

    _keywordDFA: :class:`BaseAhoCorasickDFA`
        The `DFA`_ that will use `Aho-Corasick`_ to quickly search/filter keywords in a line in the .ini file

    _stateDFA: :class:`DFA`
        The `DFA`_ that will store state information
    """

    IsFixedPattern = re.compile(r"\s*\[.*" + f"{IniKeywords.Remap.value}({IniKeywords.Blend.value}|{IniKeywords.Position.value}|{IniKeywords.Ib.value}|dl|tex|fix".lower() + r").*\]")
    IsModPattern = re.compile(r"\s*\[.*(" + f"{IniKeywords.Blend.value}|{IniKeywords.Position.value}".lower() + r").*\]")
    IsModOrIsFixedPattern = re.compile(r"(" + f"{IniKeywords.Blend.value}|{IniKeywords.Position.value}|{IniKeywords.Remap.value}(fix|tex|{IniKeywords.Ib.value})".lower() + r")")
    RemapFixSuffixPattern = re.compile(IniKeywords.RemapFix.value.lower() + ".*\]")

    IsFixedKeywords = {IniKeywords.RemapBlend.value.lower(), IniKeywords.RemapFix.value.lower(), IniKeywords.RemapPosition.value.lower(),
                       IniKeywords.RemapTex.value.lower(), IniKeywords.RemapDL.value.lower(), IniKeywords.RemapIb.value.lower(), IniKeywords.RemapTexcoord.value.lower()}
    IsModKeywords = {IniKeywords.Blend.value.lower(), IniKeywords.Position.value.lower()}

    def __init__(self, builder: Optional[BaseIniClassifierBuilder] = None, ahoCorasickCls: Optional[Type[BaseAhoCorasickDFA]] = None):
        self.builder = builder
        self._keywordDFA = AhoCorasickBuilder(buildCls = ahoCorasickCls, kwargs = {"handleDuplicate": self._handleDuplicate}).build()
        self._stateDFA = DFA()

        if (builder is not None):
            self.build(builder)

    # _handleDuplicate(keyword, oldVal, newVal): How to handle duplicate values within the keyword DFA
    def _handleDuplicate(self, keyword: str, oldVal: IniClsTransitionVals, newVal: IniClsTransitionVals) -> IniClsTransitionVals:
        oldVal.update(newVal)
        return oldVal
    
    def clear(self):
        """
        Clears all the saved data in the classifier
        """

        self._keywordDFA.clear()
        self._stateDFA.clear()

    def build(self, builder: BaseIniClassifierBuilder):
        """
        Rebuilds the classifier

        Parameters
        ----------
        builder: :class:`BaseIniClassifierBuilder`
            The builder to help build the classifier
        """

        self.clear()
        self.builder = builder
        builder.build(self)

    def reset(self):
        """
        Resets the state the classifier is at
        """

        self._stateDFA.reset()

    def classify(self, iniTxt: Union[str, List[str]], checkIsMod: bool = True, checkIsFixed: bool = True) -> IniClassifyStats:
        self._stateDFA.reset()
        if (isinstance(iniTxt, str)):
            iniTxt = TextTools.getTextLines(iniTxt)

        stats = IniClassifyStats()

        isMod = not checkIsMod
        isFixed = not checkIsFixed
        modFound = False

        for line in iniTxt:
            cleanedLine = line.replace(IniKeywords.HideOriginalComment.value, "").lower()

            if (not modFound):
                self.readLine(cleanedLine, stats)
            else:
                self.checkOnlyIsFixedOrisMod(cleanedLine, stats)

            if (not modFound and isinstance(stats.modType, ModType)):
                modFound = True

            if (not isMod and stats.isMod):
                isMod = True

            if (not isFixed and stats.isFixed):
                isFixed = True

            if (modFound and isFixed and isMod):
                return stats

        return stats

    @classmethod
    def getSectionName(cls, line: str) -> str:
        """
        Retrieves the name of a `section`_ from a line in the .ini file

        Parameters
        ----------
        line: :class:`str`
            The line from the .ini file to retrieve the section name from

        Returns
        -------
        :class:`str`
            The retrieved name
        """

        currentSectionName = line
        rightPos = currentSectionName.rfind("]")
        leftPos = currentSectionName.find("[")

        if (rightPos > -1 and leftPos > -1):
            currentSectionName = currentSectionName[leftPos + 1:rightPos]
        elif (rightPos > -1):
            currentSectionName = currentSectionName[:rightPos]
        elif (leftPos > -1):
            currentSectionName = currentSectionName[leftPos + 1:]

        return currentSectionName.strip()
    
    def checkOnlyIsFixedOrisMod(self, line: str, stats: IniClassifyStats):
        """
        Reads a line in the .ini file and checks whether the line contains
        keywords for:

        #. Whether the .ini file belongs to a mod OR
        #. Whether the .ini file is fixed

        Parameters
        ----------
        line: :class:`str`
            The line from the .ini file to read

        stats: :class:`IniClassifyStats`
            The resultant stats to store the classification result of the .ini file
        """

        if (not stats.isFixed and re.match(self.IsFixedPattern, line)):
            stats.isFixed = True

        if (not stats.isMod and re.match(self.IsModPattern, line)):
            stats.isMod = True
    
    def _addTransition(self, srcStateId: Hashable, transition: str, destStateId: Hashable, transitionVal: Union[Optional[ModType], IniClsAction, Callable[[IniClsActionArgs], Any]]):
        """
        Convenience function to add a transition to the classifier

        Parameters
        ----------
        srcStateId: `Hashable`_
            The id of the source state

        transition: :class:`str`
            The keyword to trigger the transition

        destStateId: `Hashable`_
            The id of the destionation state

            .. note::
                If this state is created from this function, the state will not be an accepting state

        transitionVal: Union[Optional[:class:`ModType`], :class:`IniClsAction`, Callable[[:class:`IniActionArgs`], Any]]
            The corresponding value to store at the transition
        """

        self._stateDFA.addTransition(srcStateId, transition, destStateId)
        self._keywordDFA.add(transition, IniClsTransitionVals({srcStateId: transitionVal}))

    def _transition(self, stats: IniClassifyStats, line: str, keyword: str, keywordInd: int = -1, keywordEndInd: int = -1, keywordVals: Optional[IniClsTransitionVals] = None):
        """
        Transitions the classifier to another state

        Parameters
        ----------
        stats: :class:`IniClassifyStats`
            The resultant stats to store the classification result of the .ini file

        line: :class:`str`
            The line in the .ini file that was read

        keyword: :class:`str`
            The keyword found from the line of the .ini file read

        keywordInd: :class:`int`
            The index where the keyword was found :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``-1``

        keywordEndInd: :class:`int`
            The ending index of where the keyword was found :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``-1``

        keywordVals: :class:`IniClsTransitionVals`
            The corresponding values for the keyword found :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        if (keywordVals is None):
            keywordVals = self._keywordDFA.getKeyVal(keyword, errorOnNotFound = False)

        currentStateId = self._stateDFA.currentStateId
        if (currentStateId not in keywordVals):
            self._stateDFA.reset()
            return

        action = keywordVals[currentStateId]

        isModType = isinstance(action, ModType)
        newStateId, isAccept, transitionMade = self._stateDFA.transition(keyword)

        if (action is None or (isModType and not isAccept)):
            return
        elif (isModType and isAccept):
            stats.modType = action
            stats.isMod = True
            self._stateDFA.reset()
            return

        actionArgs = IniClsActionArgs(self, stats, line, keyword, keywordInd, keywordEndInd, currentStateId, newStateId, isAccept, transitionMade)
        action(actionArgs)

        if (isinstance(stats.modType, ModType)):
            self._stateDFA.reset()

    def setIsFixed(self, keyword: str, stats: IniClassifyStats):
        """
        Marks the .ini file to be fixed, after checking 'keyword'

        Parameters
        ----------
        keyword: :class:`str`
            The keyword to trigger the .ini file to be considered as fixed

        stats: :class:`IniClassifyStats`
            The resultant stats to store the classification result of the .ini file
        """

        if (not stats.isFixed and keyword in self.IsFixedKeywords):
            stats.isFixed = True

    def setIsMod(self, keyword: str, stats: IniClassifyStats):
        """
        Marks the .ini file to belong to a mod, based off the 'keyword'

        Parameters
        ----------
        keyword: :class:`str`
            The keyword to trigger the .ini file to be a .ini file that belongs to some mod

        stats: :class:`IniClassifyStats`
            The resultant stats to store the classification result of the .ini file
        """

        if (not stats.isMod and keyword in self.IsModKeywords):
            stats.isMod = True

    def setIsFixedAndIsMod(self, keyword: str, stats: IniClassifyStats):
        """
        Marks the .ini file to belong to a mod and is fixed, based off the 'keyword's

        Parameters
        ----------
        keyword: :class:`str`
            The keyword to trigger the .ini file to be a .ini file that belongs to some mod and
            the .ini file to be fixed

        stats: :class:`IniClassifyStats`
            The resultant stats to store the classification result of the .ini file
        """

        self.setIsFixed(keyword, stats)
        self.setIsMod(keyword, stats)

    def readLine(self, line: str, stats: IniClassifyStats):
        """
        Reads a single line in a .ini file

        .. note::
            If you do not care about what type of mod is returned and only want to know
            whether the .ini file belongs to a mod or has already been fixed, then it is recommended
            to use the :meth:`checkOnlyIsFixedOrisMod` method instead for faster computation

        Parameters
        ----------
        line: :class:`str`
            The line in the .ini file

        stats: :class:`IniClassifyStats`
            The resultant stats to store the classification result of the .ini file
        """

        keyword, keywordInd = self._keywordDFA.findMaximal(line)
        if (keyword is None):
            return
        
        val = self._keywordDFA.getKeyVal(keyword)
        keywordEndInd = keywordInd + len(keyword)

        self._transition(stats, line, keyword, keywordInd, keywordEndInd = keywordEndInd, keywordVals = val)
##### EndScript
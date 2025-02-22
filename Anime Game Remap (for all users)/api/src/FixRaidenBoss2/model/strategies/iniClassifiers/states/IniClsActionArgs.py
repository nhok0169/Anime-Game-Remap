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
from typing import TYPE_CHECKING, Hashable
##### EndExtImports

##### LocalImports
from ..IniClassifyStats import IniClassifyStats
if (TYPE_CHECKING):
    from ..IniClassifier import IniClassifier
##### EndLocalImports


##### Script
class IniClsActionArgs():
    """
    Class to store the arguments for a :class:`IniClsAction`

    Parameters
    ----------
    classifier: :class:`IniClassifier`
        The classifier to identify a mod given a .ini file

    stats: :class:`IniClassiyStats`
        The resultant stats about the classification of the .ini file

    line: :class:`str`
        The current line being read from the .ini file

    keyword: :class:`str`
        The keyword found from the current line read from the .ini file

    keywordInd: :class:`int`
        The start index where the keyword was found

    keywordEndInd: :class:`int`
        The end index of the keyword

    prevStateId: `Hashable`_
        The id of the previous state the classifier was on

    currentStateId: `Hashable`_
        The id of the current state the classifier is on

    isAccept: :class:`bool` 
        Whether the current state is an accepting state

    transitionMade: :class:`bool`
        Whether a transition was made from the prevous state to the current state

    Attributes
    ----------
    classifier: :class:`IniClassifier`
        The classifier to identify a mod given a .ini file

    stats: :class:`IniClassiyStats`
        The resultant stats about the classification of the .ini file

    line: :class:`str`
        The current line being read from the .ini file

    keyword: :class:`str`
        The keyword found from the current line read from the .ini file

    keywordInd: :class:`int`
        The start index where the keyword was found

    keywordEndInd: :class:`int`
        The end index of the keyword

    prevStateId: `Hashable`_
        The id of the previous state the classifier was on

    currentStateId: `Hashable`_
        The id of the current state the classifier is on

    isAccept: :class:`bool` 
        Whether the current state is an accepting state

    transitionMade: :class:`bool`
        Whether a transition was made from the prevous state to the current state
    """

    def __init__(self, classifier: "IniClassifier", stats: IniClassifyStats, line: str, keyword: str, keywordInd: int, 
                 keywordEndInd: int, prevStateId: Hashable, currentStateId: Hashable, isAccept: bool, transitionMade: bool):
        self.classifier = classifier
        self.stats = stats
        self.line = line
        self.keyword = keyword
        self.keywordInd = keywordInd
        self.keywordEndInd = keywordEndInd
        self.prevStateId = prevStateId
        self.currentStateId = currentStateId
        self.isAccept = isAccept
        self.transitionMade = transitionMade
##### EndScript
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
from typing import TYPE_CHECKING, Dict, Union, Callable, Optional, Hashable, Any, List
##### EndExtImports

##### LocalImports
from ....constants.GenericTypes import Pattern
from ....constants.IniConsts import IniKeywords
from .states.IniClsCond import IniClsCond
from .states.IniClsActionArgs import IniClsActionArgs
from .states.IniClsAction import IniClsAction
from .IniClassifyStats import IniClassifyStats
from .BaseIniClassifierBuilder import BaseIniClassifierBuilder
from ....constants.ModTypes import ModTypes

if (TYPE_CHECKING):
    from .IniClassifier import IniClassifier
    from ..ModType import ModType
##### EndLocalImports


##### Script
class IniClassifierLambda():
    def __init__(self, *args):
        self.args = args

    def checkRegex(self, actionArgs: IniClsActionArgs) -> bool:
        return bool(re.search(self.args[0], actionArgs.line))
    
    def checkStr(self, actionArgs: IniClsActionArgs) -> bool:
        return actionArgs.line.find(self.args[0]) > -1
    
    def giAcceptLine(self, args: IniClsActionArgs):
        args.stats.modType = self.args[0]
        if (not args.stats.isMod):
            args.stats.isMod = True

        args.classifier.reset()

        txtSuffix = args.line[args.keywordEndInd:]
        suffixKey, suffixInd = args.classifier._keywordDFA.findMaximal(txtSuffix)

        if (suffixKey is None):
            return
        
        args.classifier.setIsFixedAndIsMod(suffixKey, args.stats)

    def giTransitionToCheck(self, args: IniClsActionArgs):
        args.classifier._transition(args.stats, args.line, self.args[0], keywordInd = args.keywordInd, keywordEndInd = args.keywordEndInd)
        

class IniClassifierBuilder(BaseIniClassifierBuilder):
    """
    This class inherits from :class:`BaseIniClassifierBuilder` :raw-html:`<br />` :raw-html:`<br />`

    Class to help build/customize a :class:`IniClassifier` used for this software

    Attributes
    ----------
    _startStateId: :class:`str`
        The id for the root state

    _textureOverrideId: :class:`str`
        The id for the ``TextureOverride`` state
    """

    def __init__(self):
        self._startStateId = "root"
        self._textureOverrideId = "textureOverride"
        self._sectionPatterns = {}

        sectionKeywords = {IniKeywords.RemapFix.value.lower(), IniKeywords.RemapTex.value.lower(),
                           IniKeywords.Blend.value.lower(), IniKeywords.RemapBlend.value.lower(),
                           IniKeywords.RemapPosition.value.lower()}

        for keyword in sectionKeywords:
            self._sectionPatterns[keyword] = re.compile(r"^\s*\[.*" + keyword + r".*\]")

    def _reset(self, args: IniClsActionArgs):
        args.classifier.reset()

    def _setIsFixed(self, args: IniClsActionArgs):
        args.stats.isFixed = True
        args.classifier.reset()

    def _setIsMod(self, args: IniClsActionArgs):
        args.stats.isMod = True
        args.classifier.reset()

    def _setIsFixedAndIsMod(self, args: IniClsActionArgs):
        args.stats.isFixed = True
        args.stats.isMod = True
        args.classifier.reset()

    def _handlePosition(self, args: IniClsActionArgs):
        args.classifier.reset()
        if (not args.stats.isMod):
            args.stats.isMod = True

        if (args.stats.isFixed):
            return
        
        if (re.search(args.classifier.RemapFixSuffixPattern, args.line[args.keywordEndInd:])):
            args.stats.isFixed = True

    def _transitionTextureOverride(self, args: IniClsActionArgs):
        keywordEndInd = args.keywordEndInd
        txtSuffix, txtSuffixInd = args.classifier._keywordDFA.findMaximal(args.line[keywordEndInd:])
        if (txtSuffix is None):
            return
        
        txtSuffixVals = args.classifier._keywordDFA.getKeyVal(txtSuffix)
        txtSuffixEndInd = txtSuffixInd + len(txtSuffix)
        args.classifier._transition(args.stats, args.line, txtSuffix, txtSuffixInd + keywordEndInd, txtSuffixEndInd + keywordEndInd, txtSuffixVals)

    def _checkSectionKeyword(self, actionArgs: IniClsActionArgs) -> bool:
        return bool(self._sectionPatterns[actionArgs.keyword].search(actionArgs.line))
    
    def _checkOnlyFixedSectionKeyword(self, actionArgs: IniClsActionArgs) -> bool:
        return not actionArgs.stats.isFixed and bool(self._sectionPatterns[actionArgs.keyword].search(actionArgs.line))
    
    def _checkOnlyIsModKeyword(self, actionArgs: IniClsActionArgs) -> bool:
        return not actionArgs.stats.isMod and bool(self._sectionPatterns[actionArgs.keyword].search(actionArgs.line))
    
    def _checkIsFixed(self, args: IniClsActionArgs) -> bool:
        return args.stats.isFixed
    
    def _checkIsMod(self, args: IniClsActionArgs) -> bool:
        return args.stats.isMod

    def build(self, classifier: "IniClassifier"):
        classifier._stateDFA.addState(self._startStateId)

        # Comments
        self._addKeywordGroup(classifier, [";", "#"], self._startStateId, "comment", self._reset)

        # texuture override keyword
        classifier._addTransition(self._startStateId, "textureoverride", self._textureOverrideId, self._transitionTextureOverride)

        # Keywords for whether the .ini file is only fixed
        onlyFixedKeywords = [IniKeywords.RemapFix.value.lower(), IniKeywords.RemapTex.value.lower()]
        onlyFixedCond = IniClsCond([self._checkOnlyFixedSectionKeyword], [self._setIsFixed], self._reset)
        self._addKeywordGroup(classifier, onlyFixedKeywords, self._startStateId, "onlyFixed", onlyFixedCond)
        self._addKeywordGroup(classifier, onlyFixedKeywords, self._textureOverrideId, "texOnlyFixed", onlyFixedCond)

        # Keywords for whether the .ini file is only a mod
        onlyIsModKeywords = [IniKeywords.Blend.value.lower()]
        onlyIsModCond = IniClsCond([self._checkOnlyIsModKeyword], [self._setIsMod], self._reset)
        self._addKeywordGroup(classifier, onlyIsModKeywords, self._startStateId, "onlyIsMod", onlyIsModCond)
        self._addKeywordGroup(classifier, onlyIsModKeywords, self._textureOverrideId, "texOnlyIsMod", onlyIsModCond)
        
        # Keywords for whether the .ini file is both fixed and is a mod
        fixedAndIsModKeywords = [IniKeywords.RemapBlend.value.lower(), IniKeywords.RemapPosition.value.lower()]
        fixedAndIsModCond = IniClsCond([self._checkSectionKeyword], [self._setIsFixedAndIsMod], self._reset)
        self._addKeywordGroup(classifier, fixedAndIsModKeywords, self._startStateId, "fixedAndIsModBlend", fixedAndIsModCond)
        self._addKeywordGroup(classifier, fixedAndIsModKeywords, self._textureOverrideId, "tFixedAndIsModBlend", fixedAndIsModCond)
        
        # Position and Position.*RemapFix keywords
        positionKeywords = [IniKeywords.Position.value.lower()]
        self._addKeywordGroup(classifier, positionKeywords, self._startStateId, "fixedOrIsModPos", self._handlePosition)
        self._addKeywordGroup(classifier, positionKeywords, self._textureOverrideId, "tFixedOrIsModPos", self._handlePosition)

        # ===== GI mods =====

        self.addGIModType(classifier, ModTypes.Amber.value, {"amber": re.compile(r"^\s*\[\s*textureoverride.*(amber)((?!cn).)*\]")})
        self.addGIModType(classifier, ModTypes.AmberCN.value, {"ambercn": re.compile(r"^\s*\[\s*textureoverride.*(ambercn).*\]")})
        self.addGIModType(classifier, ModTypes.Ayaka.value, {"ayaka": re.compile(r"^\s*\[\s*textureoverride.*(ayaka)((?!(springbloom)).)*\]")})
        self.addGIModType(classifier, ModTypes.AyakaSpringBloom.value, {"ayakaspringbloom": re.compile(r"^\s*\[\s*textureoverride.*(ayakaspringbloom).*\]")})
        self.addGIModType(classifier, ModTypes.Arlecchino.value, {"arlecchino": re.compile(r"^\s*\[\s*textureoverride.*(arlecchino).*\]")})
        self.addGIModType(classifier, ModTypes.Barbara.value, {"barbara": re.compile(r"^\s*\[\s*textureoverride.*(barbara)((?!summertime).)*\]")})
        self.addGIModType(classifier, ModTypes.BarbaraSummertime.value, {"barbarasummertime": re.compile(r"^\s*\[\s*textureoverride.*(barbarasummertime).*\]")})
        self.addGIModType(classifier, ModTypes.CherryHuTao.value, {"cherryhutao": re.compile(r"^\s*\[\s*textureoverride.*(cherryhutao).*\]"),
                                                                    "hutaocherry": re.compile(r"^\s*\[\s*textureoverride.*(hutaocherry).*\]")})
        self.addGIModType(classifier, ModTypes.Diluc.value, {"diluc": re.compile(r"^\s*\[\s*textureoverride.*(diluc)((?!flamme).)*\]")})
        self.addGIModType(classifier, ModTypes.DilucFlamme.value, {"dilucflamme": re.compile(r"^\s*\[\s*textureoverride.*(dilucflamme).*\]")})
        self.addGIModType(classifier, ModTypes.Fischl.value, {"fischl": re.compile(r"^\s*\[\s*textureoverride.*(fischl)((?!highness).)*\]")})
        self.addGIModType(classifier, ModTypes.FischlHighness.value, {"fischlhighness": re.compile(r"^\s*\[\s*textureoverride.*(fischlhighness).*\]")})
        self.addGIModType(classifier, ModTypes.Ganyu.value, {"ganyu": re.compile(r"^\s*\[\s*textureoverride.*(ganyu)((?!(twilight)).)*\]")})
        self.addGIModType(classifier, ModTypes.GanyuTwilight.value, {"ganyutwilight": re.compile(r"^\s*\[\s*textureoverride.*(ganyutwilight).*\]")})
        self.addGIModType(classifier, ModTypes.HuTao.value, {"hutao": re.compile(r"^\s*\[\s*textureoverride((?!cherry).)*(hutao)((?!cherry).)*\]")})
        self.addGIModType(classifier, ModTypes.Jean.value, {"jean": re.compile(r"^\s*\[\s*textureoverride.*(jean)((?!(cn|sea)).)*\]")})
        self.addGIModType(classifier, ModTypes.JeanCN.value, {"jeancn": re.compile(r"^\s*\[\s*textureoverride.*(jeancn)((?!sea).)*\]")})
        self.addGIModType(classifier, ModTypes.JeanSea.value, {"jeansea": re.compile(r"^\s*\[\s*textureoverride.*(jeansea)((?!cn).)*\]")})
        self.addGIModType(classifier, ModTypes.Kaeya.value, {"kaeya": re.compile(r"^\s*\[\s*textureoverride.*(kaeya)((?!(sailwind)).)*\]")})
        self.addGIModType(classifier, ModTypes.KaeyaSailwind.value, {"kaeyasailwind": re.compile(r"^\s*\[\s*textureoverride.*(kaeyasailwind).*\]")})
        self.addGIModType(classifier, ModTypes.Keqing.value, {"keqing": re.compile(r"^\s*\[\s*textureoverride.*(keqing)((?!(opulent)).)*\]")})
        self.addGIModType(classifier, ModTypes.KeqingOpulent.value, {"keqingopulent": re.compile(r"^\s*\[\s*textureoverride.*(keqingopulent).*\]")})
        self.addGIModType(classifier, ModTypes.Kirara.value, {"kirara": re.compile(r"^\s*\[\s*textureoverride.*(kirara)((?!boots).)*\]")})
        self.addGIModType(classifier, ModTypes.KiraraBoots.value, {"kiraraboots": re.compile(r"^\s*\[\s*textureoverride.*(kiraraboots).*\]")})
        self.addGIModType(classifier, ModTypes.Klee.value, {"klee": re.compile(r"^\s*\[\s*textureoverride.*(klee)((?!blossomingstarlight).)*\]")})
        self.addGIModType(classifier, ModTypes.KleeBlossomingStarlight.value, {"kleeblossomingstarlight": re.compile(r"^\s*\[\s*textureoverride.*(kleeblossomingstarlight).*\]")})
        self.addGIModType(classifier, ModTypes.Lisa.value, {"lisa": re.compile(r"^\s*\[\s*textureoverride.*(lisa)((?!student).)*\]")})
        self.addGIModType(classifier, ModTypes.LisaStudent.value, {"lisastudent": re.compile(r"^\s*\[\s*textureoverride.*(lisastudent).*\]")})
        self.addGIModType(classifier, ModTypes.Mona.value, {"mona": re.compile(r"^\s*\[\s*textureoverride.*(mona)((?!(cn)).)*\]")})
        self.addGIModType(classifier, ModTypes.MonaCN.value, {"monacn": re.compile(r"^\s*\[\s*textureoverride.*(monacn).*\]")})
        self.addGIModType(classifier, ModTypes.Nilou.value, {"nilou": re.compile(r"^\s*\[\s*textureoverride.*(nilou)((?!(breeze)).)*\]")})
        self.addGIModType(classifier, ModTypes.NilouBreeze.value, {"niloubreeze": re.compile(r"^\s*\[\s*textureoverride.*(niloubreeze).*\]")})
        self.addGIModType(classifier, ModTypes.Ningguang.value, {"ningguang": re.compile(r"^\s*\[\s*textureoverride.*(ningguang)((?!(orchid)).)*\]")})
        self.addGIModType(classifier, ModTypes.NingguangOrchid.value, {"ningguangorchid": re.compile(r"^\s*\[\s*textureoverride.*(ningguangorchid).*\]")})
        self.addGIModType(classifier, ModTypes.Raiden.value, {"raiden": re.compile(r"^\s*\[\s*textureoverride.*(raiden).*\]"),
                                                               "shogun": re.compile(r"^\s*\[\s*textureoverride.*(shogun).*\]")})
        self.addGIModType(classifier, ModTypes.Rosaria.value, {"rosaria": re.compile(r"^\s*\[\s*textureoverride.*(rosaria)((?!(cn)).)*\]")})
        self.addGIModType(classifier, ModTypes.RosariaCN.value, {"rosariacn": re.compile(r"^\s*\[\s*textureoverride.*(rosariacn).*\]")})
        self.addGIModType(classifier, ModTypes.Shenhe.value, {"shenhe": re.compile(r"^\s*\[\s*textureoverride.*(shenhe)((?!frostflower).)*\]")})
        self.addGIModType(classifier, ModTypes.ShenheFrostFlower.value, {"shenhefrostflower": re.compile(r"^\s*\[\s*textureoverride.*(shenhefrostflower).*\]")})
        self.addGIModType(classifier, ModTypes.Xiangling.value, {"xiangling": re.compile(r"^\s*\[\s*textureoverride.*(xiangling)((?!cheer|newyear).)*\]")})
        self.addGIModType(classifier, ModTypes.XianglingCheer.value, {"xianglingcheer": re.compile(r"^\s*\[\s*textureoverride.*(xianglingcheer).*\]"),
                                                                      "xianglingnewyear": re.compile(r"^\s*\[\s*textureoverride.*(xianglingnewyear).*\]")})
        self.addGIModType(classifier, ModTypes.Xingqiu.value, {"xingqiu": re.compile(r"^\s*\[\s*textureoverride.*(xingqiu)((?!bamboo).)*\]")})
        self.addGIModType(classifier, ModTypes.XingqiuBamboo.value, {"xingqiubamboo": re.compile(r"^\s*\[\s*textureoverride.*(xingqiubamboo).*\]")})

        # ===================

    def _addKeywordGroup(self, classifier: "IniClassifier", keywords: List[str], srcStateId: Hashable, keywordsStateId: Hashable, 
                         transitionVal: Union[Optional["ModType"], IniClsAction, Callable[["IniClassifier", IniClassifyStats, str, str, Hashable, Hashable, bool, bool], Any]]):
        """
        Convenience function to add many keywords that transition from the same source state to the same destionation state

        Parameters
        ----------
        classifier: :class:`IniClassifier`
            The classifier to identify mods from .ini files

        keywords: List[:class:`str`]
            The keywords to add

        srcStateId: `Hashable`_
            The id of the source state

        keywordsStateId: `Hashable`_
            The id of the destionation state

            .. note::
                If this function creates the destionation state, the destionation state will not be an accepting state

        transitionVal: Union[Optional[:class:`ModType`], :class:`IniClsAction`, Callable[[:class:`IniClassifier`, :class:`IniClassifyStats`, :class:`str`, :class:`str`, `Hashable`_, `Hashable`_, :class:`bool`, :class:`bool`], Any]]
            The corresponding value to store at the transition :raw-html:`<br />` :raw-html:`<br />`

            If this value is a function, refer to :meth:`IniClsAction.run` for the specifics of what paramters to pass to the function
        """

        for keyword in keywords:
            classifier._addTransition(srcStateId, keyword, keywordsStateId, transitionVal)

    def addGIModType(self, classifier: "IniClassifier", modType: "ModType", keywords: Dict[Optional[str], Union[Optional[str], Pattern, Callable[[IniClsActionArgs], bool]]]):
        """
        Convenience function to add a mod type from the game GI

        Parameters
        ----------
        classifier: :class:`IniClassifier`
            The classifier to identify mods from .ini files

        modType: :class:`ModType`
            The type of mod to register

        keywords: Dict[Optional[:class:`str`, Union[Optional[:class:`str`, `Pattern`_, Callable[[:class:`IniClsActionArgs`], :class:`bool`]]]]]
            The keywords used to identify the mod :raw-html:`<br />` :raw-html:`<br />`

            * The keys are the keywords to identify the type of mod when reading a line from the .ini file
            * The values are any further checks to verify the keyword :raw-html:`<br />` :raw-html:`<br />`

                #. If value is a string, then will check if a line in the .ini file equals to this value
                #. If value is a regex pattern, then will check if a line in the .ini file matches this regex pattern
                #. If this value is a function, then will check if a line in the .ini file will make the function for this value return `True`
        """

        name = modType.name
        acceptStateId = f"accept_{name}"
        middleStateId = f"check_{name}"
        classifier._stateDFA.addState(acceptStateId, isAccept = True)
        acceptFunc = IniClassifierLambda(modType).giAcceptLine

        keywordInd = 0
        for keyword in keywords:
            keywordAction = keywords[keyword]
            transitionKeyword = f"{name}{keywordInd}"
            action = None

            # no action --> directly go to accept state
            if (keywordAction is None):
                classifier._addTransition(self._textureOverrideId, keyword, acceptStateId, acceptFunc)
                keywordInd += 1
                continue
            
            # further checks to verify the keyword need to go through an
            #   intermediate state
            condActions = [IniClassifierLambda(transitionKeyword).giTransitionToCheck]
            defaultAction = self._reset

            if (isinstance(keywordAction, str)):
                action = IniClsCond([IniClassifierLambda(keywordAction).checkStr], condActions, default = defaultAction)
            elif (callable(keywordAction)):
                action = IniClsCond([keywordAction], condActions)
            else:
                action = IniClsCond([IniClassifierLambda(keywordAction).checkRegex], condActions, default = defaultAction)

            classifier._addTransition(self._textureOverrideId, keyword, middleStateId, action)
            classifier._addTransition(middleStateId, transitionKeyword, acceptStateId, acceptFunc)
            keywordInd += 1
##### EndScript
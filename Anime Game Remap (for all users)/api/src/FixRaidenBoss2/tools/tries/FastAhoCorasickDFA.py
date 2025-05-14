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
from functools import lru_cache 
from typing import Dict, Optional, Optional, List, Tuple, Union, Any, Callable
##### EndExtImports

##### LocalImports
from ...constants.GlobalPackageManager import GlobalPackageManager
from .BaseAhoCorasickDFA import BaseAhoCorasickDFA
from ..DictTools import DictTools
from ...constants.Packages import PackageModules
from ...constants.GenericTypes import T
from ..Node import Node
##### EndLocalImports


##### Script
class FastAhoCorasickDFA(BaseAhoCorasickDFA):
    """
    A wrapper class over `pyahocorasick.Automaton`_

    The `DFA (Deterministic Finite Automaton)`_ used in the `Aho-Corasick`_ algorithm, implemented at the C level

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: txt in x

            Determines if a keyword is found within 'txt'

        .. describe:: x[txt]

            Retrieves the following data:

            #. The found keyword
            #. The corresponding value to the found keyword

            .. note::
                See :meth:`getMaximal` for more details

        .. describe:: x[key] = val

            Sets the new `KVP`_

            .. caution::
                Please see the warning at :meth:`add`

    Parameters
    ----------
    data: Optional[Dict[:class:`str`, T]]
        Any initial data to put into the `DFA`_ :raw-html:`<br />` :raw-html:`<br />`

        The keys are the keywords to put into the `DFA`_ and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    handleDuplicate: Optional[Callable[[:class:`str`, T, T], T]]
        Function to handle the case where 2 `KVPs`_ inserted have the same key(word) :raw-html:`<br />` :raw-html:`<br />`

        The function takes in the following parameters:

        #. The duplicate keyword in both `KVPs`_
        #. The value of the existing `KVP`_
        #. The value of the new `KVP`_

        If this value is ``None``, will return the value of the new `KVP`_ by default :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    _dfa: `pyahocorasick.Automaton`_
        The internal `DFA`_

    _data: Dict[:class:`str`, T]
        The `KVP`_ data within the `DFA`_
    """

    def __init__(self, data: Optional[Dict[str, T]] = None, handleDuplicate: Optional[Callable[[str, T, T], T]] = None):
        ahocorasick = GlobalPackageManager.get(PackageModules.AhoCorasick.value)
        self._dfa = ahocorasick.Automaton()
        super().__init__(data, handleDuplicate = handleDuplicate)
        self.build(data)

    def clearCache(self):
        super().clearCache()
        self._findMaximalMultiple.cache_clear()
        self._findMaximalSingle.cache_clear()

    def clear(self):
        ahocorasick = GlobalPackageManager.get(PackageModules.AhoCorasick.value)
        self._dfa = ahocorasick.Automaton()
        super().clear()

    def add(self, keyword: str, value: T):
        self.clearCache()
        self._data[keyword] = self.handleDuplicate(keyword, self._data[keyword], value) if (keyword in self._data) else value

        self._dfa.add_word(keyword, keyword)
        self._dfa.make_automaton()

    def build(self, data: Optional[Dict[str, T]] = None, clear: bool = True):
        if (clear):
            self.clear()
        
        if (data is not None):
            self.clearCache()

        if (data is None):
            data = {}

        self._data = DictTools.update(self._data, data, combineDuplicate = self.handleDuplicate)

        for keyword in self._data:
            self._dfa.add_word(keyword, keyword)

        self._dfa.make_automaton()

    # _dfaOnlyHasEmptyStr(): Whether the internal AhoCorasick DFA only has the empty string
    def _dfaOnlyHasEmptyStr(self):
        return len(self._data) == 1 and "" in self._data

    def findAll(self, txt: str) -> Dict[str, List[Tuple[int, int]]]:
        result = {}
        if (not self._data):
            return result

        if (not self._dfaOnlyHasEmptyStr()):
            for endInd, keyword in self._dfa.iter(txt):
                keywordInds = result.get(keyword)
                if (keywordInds is None):
                    keywordInds = []
                    result[keyword] = keywordInds

                keywordInds.append((endInd - len(keyword) + 1, endInd + 1))

        if ("" not in self._data):
            return result

        # case where the empty string is a keyword
        emptyInds = []
        txtLen = len(txt)
        for i in range(txtLen + 1):
            emptyInds.append((i, i))

        result[""] = emptyInds
        return result
    
    def findFirstAll(self, txt: str) -> Dict[str, Tuple[int, int]]:
        result = {}
        if (not self._data):
            return result

        keywordsLen = len(self._data)

        if (not self._dfaOnlyHasEmptyStr()):
            for endInd, keyword in self._dfa.iter(txt):
                result[keyword] = (endInd - keywordsLen + 1, endInd + 1)
                if (len(result) >= keywordsLen):
                    break

        if ("" not in self._data):
            return result

        # case where the empty string is a keyword
        result[""] = [(0, 0)]
        return result
    
    @lru_cache(maxsize = 256)
    def find(self, txt: str) -> Tuple[Optional[str], int]:
        if ("" in self._data):
            return ("", 0)

        keyword = None
        keywordInd = -1

        if (not self._data):
            return (keyword, keywordInd)

        for endInd, foundKeyword in self._dfa.iter(txt):
            keyword = foundKeyword
            keywordInd = endInd - len(foundKeyword) + 1
            break

        return (keyword, keywordInd)

    # _findMaximalSingle(txt): Finds the first largest keyword in 'txt'
    @lru_cache(maxsize = 256)
    def _findMaximalSingle(self, txt: str) -> Tuple[Optional[str], int]:
        keyword = None
        keywordStartInd = -1

        if (not self._data):
            return (keyword, keywordStartInd)
        
        hasEmptyKeyword = "" in self._data
        if (hasEmptyKeyword):
            keyword = ""
            keywordStartInd = 0

        if (self._dfaOnlyHasEmptyStr()):
            return (keyword, keywordStartInd) 
        
        for endInd, foundKeyword in self._dfa.iter(txt):
            startInd = endInd - len(foundKeyword) + 1
            txtSuffix = txt[startInd:]
            longestKeywordPrefixLen = self._dfa.longest_prefix(txtSuffix)

            keywordStartInd = startInd
            keyword = foundKeyword

            if (longestKeywordPrefixLen <= endInd + 1 - startInd):
                break
            
            # found the longest search result, longer than the first result
            newKeyword = txtSuffix[:longestKeywordPrefixLen]
            if (newKeyword in self._data):
                keyword = newKeyword

            break

        return (keyword, keywordStartInd)
    
    # _findMaximalMultiple(txt, count): Finds the first few largest keywords in 'txt'
    @lru_cache(maxsize = 256)
    def _findMaximalMultiple(self, txt: str, count: int) -> Tuple[List[str], List[int]]:
        keywords = []
        keywordInds = []
        currentKeyword = None
        currentKeywordStartInd = -1
        numOfKeywordsToFind = count

        if (not self._data):
            return (keywords, keywordInds)
        
        hasEmptyKeyword = "" in self._data
        if (hasEmptyKeyword):
            currentKeyword = ""
            currentKeywordStartInd = 0

        if (self._dfaOnlyHasEmptyStr()):
            txtLen = len(txt)
            for i in range(0, min(txtLen + 1, count)):
                keywords.append("")
                keywordInds.append(i)

            return (keywords, keywordInds)
        
        currentTxtInd = 0
        txtLen = len(txt)

        while (numOfKeywordsToFind > 0 and currentTxtInd < txtLen):
            currentLongestFound = False

            # when the user requests multiple keywords returned and the empty string
            #   is a keyword
            if (hasEmptyKeyword):
                currentKeyword = ""
                currentKeywordStartInd = currentTxtInd

            for currentEndInd, foundKeyword in self._dfa.iter(txt[currentTxtInd:]):
                currentStartInd = currentEndInd - len(foundKeyword) + 1
                startInd = currentStartInd + currentTxtInd
                endInd = startInd + currentEndInd + 1

                # found keyword is not the next maximal keyword
                if (currentKeyword is not None and startInd > currentKeywordStartInd):
                    break

                txtSuffix = txt[startInd:]
                longestKeywordPrefixLen = self._dfa.longest_prefix(txtSuffix)

                currentKeywordStartInd = startInd
                currentKeyword = foundKeyword 

                if (longestKeywordPrefixLen <= currentEndInd + 1 - currentStartInd):
                    currentLongestFound = True
                
                # found the longest search result, longer than the first result
                if (not currentLongestFound):
                    newKeyword = txtSuffix[:longestKeywordPrefixLen]
                    currentLongestFound = True

                    if (newKeyword in self._data):
                        currentKeyword = newKeyword
                        endInd = startInd + longestKeywordPrefixLen

                currentTxtInd = endInd
                numOfKeywordsToFind -= 1

                # reset the keyword found
                keywords.append(currentKeyword)
                keywordInds.append(currentKeywordStartInd)
                currentKeyword = None
                currentKeywordStartInd = -1

                break

            # add the empty string as the current longest keyword
            if (currentKeyword is not None):
                numOfKeywordsToFind -= 1
                keywords.append(currentKeyword)
                keywordInds.append(currentKeywordStartInd)
                currentKeyword = None
                currentKeywordStartInd = -1
                currentTxtInd += 1
                currentLongestFound = True

            # no more keywords found
            if (not currentLongestFound):
                break

        # empty string at the very end of the text
        if (hasEmptyKeyword and numOfKeywordsToFind):
            keywords.append("")
            keywordInds.append(txtLen)

        return (keywords, keywordInds)

    @lru_cache(maxsize = 256)
    def findMaximal(self, txt: str, count: int = 1) -> Tuple[Union[Optional[str], List[str]], Union[int, List[int]]]:
        if (count <= 1):
            return self._findMaximalSingle(txt)

        return self._findMaximalMultiple(txt, count)
    
    @lru_cache(maxsize = 256) 
    def get(self, txt: str, errorOnNotFound: bool = True, default: Any = None) -> Tuple[Optional[str], Union[T, Any]]:
        keyword, _ = self.find(txt)

        keywordFound = keyword is not None
        if (not keywordFound and errorOnNotFound):
            raise KeyError(f"The text, '{txt}', does not contain any matching keywords")
        elif (not keywordFound):
            return (keyword, default)

        return (keyword, self._data[keyword])
    
    @lru_cache(maxsize = 256)
    def getMaximal(self, txt: str, errorOnNotFound: bool = True, default: Any = None, count: int = 1) -> Tuple[Union[Optional[str], List[str]], Union[T, Any, List[T]]]:
        keywords, _ = self.findMaximal(txt, count = count)
        findSingleKeyword = count <= 1

        keywordFound = keywords is not None and (findSingleKeyword or bool(keywords))
        if (not keywordFound and errorOnNotFound):
            raise KeyError(f"The text, '{txt}', does not contain any matching keywords")
        elif (not keywordFound and findSingleKeyword):
            return (keywords, default)
        elif (not keywordFound):
            return ([], [])

        if (findSingleKeyword):
            return (keywords, self._data[keywords])
        
        keywordVals = []
        for keyword in keywords:
            keywordVals.append(self._data[keyword])

        return (keywords, keywordVals)
    
    @lru_cache(maxsize = 256)
    def getKeyVal(self, txt: str, errorOnNotFound: bool = True, default: Any = None) -> Union[T, Any]:
        if (txt in self._data):
            return self._data[txt]
        
        if (errorOnNotFound):
            raise KeyError(f"The given key, '{txt}', is not found")
        
        return default

    def getAll(self, txt: str) -> Dict[str, T]:
        result = {}
        if (not self._data):
            return result
        
        keywordsLen = len(self._data)

        if (not self._dfaOnlyHasEmptyStr()):
            for endInd, keyword in self._dfa.iter(txt):
                result[keyword] = self._data[keyword]
                if (len(result) >= keywordsLen):
                    break
        
        if ("" in self._data):
            result[""] = self._data[""]
        return result
##### EndScript

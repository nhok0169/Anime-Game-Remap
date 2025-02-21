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
from collections import deque
from typing import Dict, Optional, Optional, List, Tuple, Union, Any, Type, Callable
##### EndExtImports

##### LocalImports
from .BaseAhoCorasickDFA import BaseAhoCorasickDFA
from ...constants.GenericTypes import T
from ..Node import Node
from .Trie import Trie
from ..Algo import Algo
##### EndLocalImports


##### Script
class AhoCorasickDFA(Trie, BaseAhoCorasickDFA):
    """
    This class inherits from :class:`Trie` and :class:`BaseAhoCorasickDFA`

    The `DFA (Deterministic Finite Automaton)`_ used in the `Aho-Corasick`_ algorithm, implemented using pure Python

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

    nodeCls: Type[:class:`Node`]
        The class used to construct a node in the `trie`_

    Attributes
    ----------
    _fail: Dict[:class:`int`, :class:`int`]
        The failure edges in the `DFA`_ :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids to the sources node of the edges and the values are the ids to the sink nodes of the edges
    """

    def __init__(self, data: Optional[Dict[str, T]] = None, handleDuplicate: Optional[Callable[[str, T, T], T]] = None, nodeCls: Type[Node] = Node):
        self._fail: Dict[int, int] = {}
        Trie.__init__(self, data = data, handleDuplicate = handleDuplicate, nodeCls = nodeCls)

    def __getitem__(self, txt: str) -> Tuple[Optional[str], T]:
        return self.getMaximal(txt)
    
    def __setitem__(self, keyword: int, value: T):
        self.add(keyword, value)

    def __contains__(self, txt: str) -> bool:
        keyword, ind = self.find(txt)
        return keyword is not None
    
    def clearCache(self):
        Trie.clearCache(self)
        BaseAhoCorasickDFA.clearCache(self)
        self._getNextState.cache_clear()
        self._findMaximalMultiple.cache_clear()
        self._findMaximalSingle.cache_clear()

    def clear(self):
        Trie.clear(self)
        self._fail = {}

    def add(self, keyword: str, value: T):
        data = {}
        for currentKeyword in self._keywordIds:
            keywordId = self._keywordIds[currentKeyword]
            val = self._vals[keywordId]
            data[currentKeyword] = val

        data[keyword] = self._handleDuplicate(keyword, data[keyword], value) if (keyword in data) else value
        self.build(data)

    def build(self, data: Dict[str, T] = None):
        self.clearCache()
        Trie.build(self, data)

        node = self._root
        rootId = node.id
        childrenIds = self._children.get(node.id)

        # no keywords added
        if (childrenIds is None):
            return

        # all depth 1 children in the trie have a failure
        #   function that returns to the root
        for letter in childrenIds:
            childId = childrenIds[letter]
            self._fail[childId] = node.id

        # BFS to complete the failure function and the output results
        visitedNodes = set()
        nodeQueue = deque()

        nodeQueue.append(node.id)
        visitedNodes.add(node.id)

        while (nodeQueue):
            nodeId = nodeQueue.popleft()

            childrenIds = self._children.get(nodeId)
            if (childrenIds is None):
                continue
            
            # should be able to get the failure of every node
            # except for the root node
            failureId = self._fail.get(nodeId)
            if (failureId is None and nodeId != self._root.id):
                continue

            for letter in childrenIds:
                childId = childrenIds[letter]
                if (childId in visitedNodes):
                    continue

                visitedNodes.add(childId)
                nodeQueue.append(childId)

                currentFailureId = failureId
                childrenFailure = self._children.get(currentFailureId)
                childFailureId = childrenFailure.get(letter) if (childrenFailure is not None) else None

                # Failure node is the node that forms the longest proper suffix
                #   with the current substring read
                # Note: Longest proper suffix is the prefix of some keyword
                while (currentFailureId is not None and currentFailureId != rootId and childFailureId is None):
                    currentFailureId = self._fail.get(currentFailureId)
                    childrenFailure = self._children.get(currentFailureId)
                    childFailureId = childrenFailure.get(letter) if (childrenFailure is not None) else None

                # default failure node if no other keyword has a proper prefix
                #   that matches the proper suffix of the current substring read
                if (childFailureId is None):
                    childFailureId = rootId

                self._fail[childId] = childFailureId
                
                childOut = self._out.get(childId, [])
                childFailureOut = self._out.get(childFailureId, [])
                self._out[childId] = Algo.merge([childOut, childFailureOut], self._compareKeywordIds)

    @lru_cache(maxsize = 512)
    def _getNextState(self, currentStateId: int, letter: str) -> Tuple[int, bool]:
        """
        Retrieves the next state for travel to in the `DFA`_

        Parameters
        ----------
        currentStateId: :class:`int`
            The id of the current state

        letter: :class:`str`
            The transition letter to go to the next state

        Returns
        -------
        Tuple[:class:`int`, :class:`bool`]
        The resultant node data that contains: :raw-html:`<br />` :raw-html:`<br />`
        
            #. The id of the node to the next state
            #. Whether the next state is from a failure transition
        """

        nextStateChildren = self._children.get(currentStateId)
        nextStateId = nextStateChildren.get(letter) if (nextStateChildren is not None) else None
        isFail = False
        rootId = self._root.id

        while (nextStateId is None and currentStateId != rootId):
            currentStateId = self._fail.get(currentStateId, rootId)
            nextStateChildren = self._children.get(currentStateId)
            nextStateId = nextStateChildren.get(letter) if (nextStateChildren is not None) else None

            if (not isFail):
                isFail = True
            
        if (nextStateId is None):
            nextStateId = rootId
            isFail = True

        return (nextStateId, isFail)

    def findAll(self, txt: str) -> Dict[str, List[Tuple[int, int]]]:
        result = {}
        stateId = self._root.id
        txtLen = len(txt)

        for i in range(-1, txtLen):
            letter = txt[i] if (i >= 0) else ""
            stateId, isFail = self._getNextState(stateId, letter)

            currentKeywords = self._out.get(stateId)
            if (currentKeywords is None):
                continue

            for keywordId in currentKeywords:
                keyword = self._keywords[keywordId]

                currentResult = result.get(keyword)
                if (currentResult is None):
                    currentResult = []
                    result[keyword] = currentResult
                
                currentResult.append((i - len(keyword) + 1, i + 1))

        return result
    
    def findFirstAll(self, txt: str) -> Dict[str, Tuple[int, int]]:
        result = {}
        stateId = self._root.id
        txtLen = len(txt)
        keywordsLen = len(self._keywords)

        for i in range(-1, txtLen):
            letter = txt[i] if (i >= 0) else ""
            stateId, isFail = self._getNextState(stateId, letter)

            currentKeywords = self._out.get(stateId)
            if (currentKeywords is None):
                continue

            for keywordId in currentKeywords:
                keyword = self._keywords[keywordId]
                if (keyword in result):
                    continue
                
                result[keyword] = (i - len(keyword) + 1, i + 1)

                if (len(result) == keywordsLen):
                    break

        return result
    
    @lru_cache(maxsize = 256)
    def find(self, txt: str) -> Tuple[Optional[str], int]:
        keyword = None
        keywordInd = -1
        stateId = self._root.id
        txtLen = len(txt)

        for i in range(-1, txtLen):
            letter = txt[i] if (i >= 0) else ""
            stateId, isFail = self._getNextState(stateId, letter)

            currentKeywords = self._out.get(stateId)
            if (currentKeywords is not None and currentKeywords):
                keyword = self._keywords[currentKeywords[0]]
                keywordInd = i - len(keyword) + 1
                break

        return (keyword, keywordInd)

    # _findMaximalSingle(txt): Finds the first largest keyword in 'txt'
    @lru_cache(maxsize = 512)
    def _findMaximalSingle(self, txt: str) -> Tuple[Optional[str], int]:
        keyword = None
        keywordInd = -1

        rootId = self._root.id
        stateId = rootId
        txtLen = len(txt)

        for i in range(-1, txtLen):
            letter = txt[i] if (i >= 0) else ""
            stateId, isFail = self._getNextState(stateId, letter)

            keywordFound = keyword is not None
            if (keywordFound and isFail):
                break

            stateIsAccept = stateId in self._accept
            if (keyword and not stateIsAccept):
                continue

            currentKeywords = self._out.get(stateId)
            if (currentKeywords is not None and currentKeywords):
                keyword = self._keywords[currentKeywords[0]]
                keywordInd = i - len(keyword) + 1

        return (keyword, keywordInd)
    
    @lru_cache(maxsize = 256)
    def _findMaximalMultiple(self, txt: str, count: int) -> Tuple[List[str], List[int]]:
        keywordLst = []
        keywordIndLst = []
        currentTxtInd = 0
        txtLen = len(txt)
        numOfFoundKeywords = count

        while (currentTxtInd < txtLen and numOfFoundKeywords > 0):
            keyword, keywordInd = self._findMaximalSingle(txt[currentTxtInd:])
            if (keyword is None):
                break

            keywordLst.append(keyword)
            keywordIndLst.append(currentTxtInd + keywordInd)
            currentTxtInd += keywordInd + len(keyword) if (keyword) else 1
            numOfFoundKeywords -= 1

        if ("" in self._keywordIds and numOfFoundKeywords):
            keywordLst.append("")
            keywordIndLst.append(txtLen)

        return (keywordLst, keywordIndLst)

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
        
        keywordId = self._keywordIds[keyword]
        return (keyword, self._vals[keywordId])
    
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
        
        if (count <= 1):
            keywordId = self._keywordIds[keywords]
            return (keywords, self._vals[keywordId])
        
        keywordVals = []
        for keyword in keywords:
            keywordId = self._keywordIds[keyword]
            keywordVals.append(self._vals[keywordId])

        return (keywords, keywordVals)

    
    @lru_cache(maxsize = 256)
    def getKeyVal(self, txt: str, errorOnNotFound: bool = True, default: Any = None) -> Union[T, Any]:
        if (txt in self._keywordIds):
            keywordId = self._keywordIds[txt]
            return self._vals[keywordId]
        
        if (errorOnNotFound):
            raise KeyError(f"The keyword, '{txt}', is not found")
        
        return default

    def getAll(self, txt: str) -> Dict[str, T]:
        result = {}
        stateId = self._root.id
        txtLen = len(txt)

        for i in range(-1, txtLen):
            letter = txt[i] if (i >= 0) else ""
            stateId, isFail = self._getNextState(stateId, letter)

            currentKeywords = self._out.get(stateId)
            if (currentKeywords is None):
                continue

            for keywordId in currentKeywords:
                keyword = self._keywords[keywordId]
                if (keyword in result):
                    continue

                result[keyword] = self._vals[keywordId]

        return result
##### EndScript
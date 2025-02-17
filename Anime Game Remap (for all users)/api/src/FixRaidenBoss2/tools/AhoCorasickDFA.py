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
import numba
from collections import deque
from typing import Dict, Optional, Optional, List, Tuple, Union, Any, Type
##### EndExtImports

##### LocalImports
from ..constants.GenericTypes import T
from .Node import Node
from .Trie import Trie
from .Algo import Algo
##### EndLocalImports


##### Script
class AhoCorasickDFA(Trie):
    """
    This class inherits from :class:`Trie`

    The `DFA (Deterministic Finite Automaton)`_ used in the `Aho-Corasick`_ algorithm

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

    nodeCls: Type[:class:`Node`]
        The class used to construct a node in the `trie`_

    Attributes
    ----------
    _fail: Dict[:class:`int`, :class:`int`]
        The failure edges in the `DFA`_ :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids to the sources node of the edges and the values are the ids to the sink nodes of the edges
    """

    def __init__(self, data: Optional[Dict[str, T]] = None, nodeCls: Type[Node] = Node):
        self._fail: Dict[int, int] = {}
        super().__init__(data = data, nodeCls = nodeCls)

    def __getitem__(self, txt: str) -> Tuple[Optional[str], T]:
        return self.getMaximal(txt)
    
    def __setitem__(self, keyword: int, value: T):
        self.add(keyword, value)

    def __contains__(self, txt: str) -> bool:
        keyword, ind = self.find(txt)
        return keyword is not None

    def clear(self):
        """
        Clears the `DFA`_
        """

        super().clear()
        self._fail = {}

    def add(self, keyword: str, value: T):
        """
        Adds a new keyword

        .. caution::
            Adding a new keyword will trigger the entire `DFA`_ to be rebuilt

        Parameters
        ----------
        keyword: :class:`str`
            The keyword to add

        value: T
            The value associated with the keyword
        """

        data = {}
        for currentKeyword in self._keywordIds:
            keywordId = self._keywordIds[currentKeyword]
            val = self._vals[keywordId]
            data[currentKeyword] = val

        data[keyword] = self._handleDuplicate(keyword, data[keyword], value) if (keyword in data) else value
        self.build(data)

    def build(self, data: Dict[str, T] = None):
        """
        Rebuilds the `DFA`_
        """

        super().build(data)

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

    def _getNextState(self, currentStateId: int, letter: str) -> Tuple[Node, bool]:
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
        Tuple[:class:`Node`, :class:`bool`]
        The resultant node data that contains: :raw-html:`<br />` :raw-html:`<br />`
        
            #. The node to the next state
            #. Whether the next state is from a failure transition
        """

        nextStateChildren = self._children.get(currentStateId)
        nextStateId = nextStateChildren.get(letter) if (nextStateChildren is not None) else None
        isFail = False

        while (nextStateId is None and currentStateId != self._root.id):
            currentStateId = self._fail.get(currentStateId, self._root.id)
            nextStateChildren = self._children.get(currentStateId)
            nextStateId = nextStateChildren.get(letter) if (nextStateChildren is not None) else None

            if (not isFail):
                isFail = True
            
        resultNode = self._nodes[nextStateId] if (nextStateId is not None) else self._root
        return (resultNode, isFail)

    def findAll(self, txt: str) -> Dict[str, List[Tuple[int, int]]]:
        """
        Finds all occurences of the keywords from the `DFA`_ in the given text

        Parameters
        ----------
        txt: :class:`str`
            The text to search for keywords

        Returns
        -------
        Dict[:class:`str`, List[Tuple[:class:`int`, :class:`int`]]]
            The indices for all the found keywords within the given text :raw-html:`<br />` :raw-html:`<br />`

            * The keys are the keywords found
            * The values are all instances of the keyword found
            * The tuple contains the starting index of the found instance and the ending index of the found instance
        """

        result = {}
        state = self._root
        txtLen = len(txt)

        for i in range(-1, txtLen):
            letter = txt[i] if (i >= 0) else ""
            state, isFail = self._getNextState(state.id, letter)

            currentKeywords = self._out.get(state.id)
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
        """
        Finds the first occurences of the keywords from the `DFA`_ in the given text

        Parameters
        ----------
        txt: :class:`str`
            The text to search for keywords

        Returns
        -------
        Dict[:class:`str`, Tuple[:class:`int`, :class:`int`]]
            The indices for all the found keywords within the given text :raw-html:`<br />` :raw-html:`<br />`

            * The keys are the keywords found
            * The tuple contains the starting index of the found instance and the ending index of the first found instance
        """

        result = {}
        state = self._root
        txtLen = len(txt)
        keywordsLen = len(self._keywords)

        for i in range(-1, txtLen):
            letter = txt[i] if (i >= 0) else ""
            state, isFail = self._getNextState(state.id, letter)

            currentKeywords = self._out.get(state.id)
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
    
    def find(self, txt: str) -> Tuple[Optional[str], int]:
        """
        Finds the first keyword within 'txt'

        Parameters
        ----------
        txt: :class:`str`
            The text to search for the keyword

        Returns
        -------
        Tuple[Optional[:class:`str`], :class:`int`]
            Data of the found keyword containing: :raw-html:`<br />` :raw-html:`<br />`

            #. The keyword found
            #. The starting index of where the keyword was found. If no keywords were found, this index is -1
        """

        keyword = None
        keywordInd = -1
        state = self._root
        txtLen = len(txt)

        for i in range(-1, txtLen):
            letter = txt[i] if (i >= 0) else ""
            state, isFail = self._getNextState(state.id, letter)

            currentKeywords = self._out.get(state.id)
            if (currentKeywords is not None and currentKeywords):
                keyword = self._keywords[currentKeywords[0]]
                keywordInd = i - len(keyword) + 1
                break

        return (keyword, keywordInd)
    
    def findMaximal(self, txt: str) -> Tuple[Optional[str], int]:
        """
        Finds the first largest keyword within 'txt'

        .. note::
            This function is a greedy version of :meth:`find` or `Maximal Munch`_ that consumes only 1 token

        Parameters
        ----------
        txt: :class:`str`
            The text to search for the keyword

        Returns
        -------
        Tuple[Optional[:class:`str`], :class:`int`]
            Data of the found keyword containing: :raw-html:`<br />` :raw-html:`<br />`

            #. The keyword found
            #. The starting index of where the keyword was found. If no keywords were found, this index is -1
        """

        keyword = None
        keywordInd = -1
        state = self._root
        txtLen = len(txt)

        for i in range(-1, txtLen):
            letter = txt[i] if (i >= 0) else ""
            state, isFail = self._getNextState(state.id, letter)

            keywordFound = keyword is not None
            if (keywordFound and isFail):
                break
            
            stateIsAccept = state.id in self._accept
            if (keywordFound and not stateIsAccept):
                continue

            currentKeywords = self._out.get(state.id)
            if (currentKeywords is not None and currentKeywords):
                keyword = self._keywords[currentKeywords[0]]
                keywordInd = i - len(keyword) + 1

        return (keyword, keywordInd)
    
    def get(self, txt: str, errorOnNotFound: bool = True, default: Any = None) -> Tuple[Optional[str], Union[T, Any]]:
        """
        Retrieves the corresponding value from the first keyword fround in 'txt'

        .. note::
            This function retrieves the corresponding value after running :meth:`find`

        Parameters
        ----------
        txt: :class:`str`
            The text to search for a keyword

        errorOnNotFound: :class:`bool`  
            If no keywords are found, whether to raise an exception

        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if no keywords are found

        Raises
        ------
        :class:`KeyError`
            If no keywords are found

        Returns
        -------
        Tuple[Optional[:class:`str`], Union[T, Any]]
            Retrieves the following resultant data:

            #. The first keyword found
            #. Either the found value for the first keyword found or the value specified at 'default', if no keywords were found and
               'errorOnNotFound' is set to ``False``
        """

        keyword, _ = self.find(txt)

        keywordFound = keyword is not None
        if (not keywordFound and errorOnNotFound):
            raise KeyError(f"The text, '{txt}', does not contain the keyword, '{keyword}'")
        elif (not keywordFound):
            return (keyword, default)
        
        keywordId = self._keywordIds[keyword]
        return (keyword, self._vals[keywordId])
    
    def getMaximal(self, txt: str, errorOnNotFound: bool = True, default: Any = None) -> Union[T, Any]:
        """
        Retrieves the corresponding value from the first largest keyword fround in 'txt'

        .. note::
            This function retrieves the corresponding value after running :meth:`findMaximal`

        Parameters
        ----------
        txt: :class:`str`
            The text to search for a keyword

        errorOnNotFound: :class:`bool`  
            If no keywords are found, whether to raise an exception

        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if no keywords are found

        Raises
        ------
        :class:`KeyError`
            If no keywords are found

        Returns
        -------
        Retrieves the following resultant data:

            #. The first largest keyword found
            #. Either the found value for the first largest keyword found or the value specified at 'default', if no keywords were found and
               'errorOnNotFound' is set to ``False``
        """

        keyword, _ = self.findMaximal(txt)

        keywordFound = keyword is not None
        if (not keywordFound and errorOnNotFound):
            raise KeyError(f"The text, '{txt}', does not contain the keyword, '{keyword}'")
        elif (not keywordFound):
            return (keyword, default)
        
        keywordId = self._keywordIds[keyword]
        return (keyword, self._vals[keywordId])

    def getAll(self, txt: str) -> Dict[str, T]:
        """
        Retrieves all the corresponding values to all the keywords found within 'txt'

        Parameters
        ----------
        txt: :class:`str`
            The text to search for keywords

        Returns
        -------
        Dict[:class:`str`, T]
            The corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`

            The keys are the keywords found and the values are the values to the keywords
        """

        result = {}
        state = self._root
        txtLen = len(txt)

        for i in range(-1, txtLen):
            letter = txt[i] if (i >= 0) else ""
            state, isFail = self._getNextState(state.id, letter)

            currentKeywords = self._out.get(state.id)
            if (currentKeywords is None):
                continue

            for keywordId in currentKeywords:
                keyword = self._keywords[keywordId]
                if (keyword in result):
                    continue

                result[keyword] = self._vals[keywordId]

        return result
##### EndScript

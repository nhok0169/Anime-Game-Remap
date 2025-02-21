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
import uuid
from typing import Dict, Optional, Generic, Optional, Tuple, Callable, List, Any, Union, Set, Type, Hashable
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import T
from ..Algo import Algo
from ..Node import Node
##### EndLocalImports


##### Script
class Trie(Generic[T]):
    """
    A class for a basic `trie`_

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: key in x

            Determines if 'key' is found

        .. describe:: x[key]

            Retrieves the corresponding value to 'key'

        .. describe:: x[key] = val

            Sets the new `KVP`_

    Parameters
    ----------
    data: Optional[Dict[:class:`str`, T]]
        Any initial data to insert :raw-html:`<br />` :raw-html:`<br />`

        The keys are the keywords to put into the `trie`_ and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`

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
    _nodes: Dict[:class:`str`, :class:`Node`]
        The nodes in the `trie`_ :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids for the node and the values are the physical node

    _children: Dict[:class:`int`, Dict[:class:`str`, :class:`int`]]
        The children nodes associated to a node :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the ids of the nodes
        * The inner keys are the string sequences of the edges between a node and its children
        * The inner values are the ids for the children

        .. note::
            This is the `adjacency list`_ for the trie

    _parent: Dict[:class:`int`, :class:`int`]
        The parent node associated to a node :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids of a node and the values are the ids of the parents

    _keywords: Dict[:class:`int`, :class:`str`]
        The keywords inside of the `trie`_ :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids for the keywords and the values are the text for the keywords

    _keywordIds: Dict[:class:`str`, :class:`int`]
        The inverse of :attr:`_keywords`

    _vals: Dict[:class:`int`, T]
        The corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids of the keywords and the values corresponding data values for the keyword

    _out: Dict[:class:`int`, List[:class:`int`]]
        The keywords found at a node :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids for the nodes and the values are the ids for the found keywords

    _accept: Set[:class:`int`]
        The ids to the nodes that are considered as accepting states

    _root: :class:`Node`
        The root node

    _nodeCls: Type[:class:`Node`]
        The class used to construct a node in the `trie`_
    """

    def __init__(self, data: Optional[Dict[str, T]] = None, handleDuplicate: Optional[Callable[[str, T, T], T]] = None, nodeCls: Type[Node] = Node):
        self._currentNodeId = uuid.uuid4().int
        self._currentKeywordId = uuid.uuid4().int

        self._nodeCls = nodeCls

        self._nodes: Dict[int, Node] = {}
        self._children: Dict[int, Dict[str, int]] = {}
        self._parent: Dict[int, int]
        self._vals: Dict[int, T] = {}
        self._out: Dict[int, List[int]] = {}
        self._accept: Set[int] = set()

        self._keywords: Dict[int, str] = {}
        self._keywordIds: Dict[str, int] = {}

        self.handleDuplicate = handleDuplicate
        self._root: Node = None

        self.build(data)

    def __getitem__(self, keyword: str) -> T:
        return self.get(keyword)
    
    def __setitem__(self, keyword: int, value: T):
        self.add(keyword, value)

    def __contains__(self, keyword: str) -> bool:
        try:
            self.get(keyword)
        except KeyError:
            return False

        return True

    @property
    def handleDuplicate(self) -> Callable[[str, T, T], T]:
        """
        Function to handle the case where 2 `KVPs`_ inserted have the same key(word) :raw-html:`<br />` :raw-html:`<br />`

        The function takes in the following parameters:

        #. The duplicate keyword in both `KVPs`_
        #. The value of the existing `KVP`_
        #. The value of the new `KVP`_

        :getter: Retrieves the function
        :setter: Sets the new function
        :type: Callable[[:class:`str`, T, T], T]
        """

        return self._handleDuplicate
    
    @handleDuplicate.setter
    def handleDuplicate(self, newHandleDuplicate: Optional[Callable[[T, T], T]]):
        self._handleDuplicate = newHandleDuplicate if (newHandleDuplicate is not None) else lambda key, oldVal, newVal: newVal

    @classmethod
    def _getNextNodeId(cls, currentId: int) -> int:
        return uuid.uuid4().int
    
    @classmethod
    def _getNextKeywordId(cls, currentId: int) -> int:
        return uuid.uuid4().int
    
    def _updateNextNodeId(self) -> int:
        self._currentNodeId = self._getNextNodeId(self._currentNodeId)
        return self._currentNodeId
    
    def _updateNextKeywordId(self) -> int:
        self._currentKeywordId = self._getNextKeywordId(self._currentKeywordId)
        return self._currentKeywordId
    
    def _resetNodeId(self) -> int:
        return self._updateNextNodeId()
    
    def _resetKeywordId(self) -> int:
        return self._updateNextKeywordId()
    
    def _constructNode(self, id: Hashable, *args, **kwargs) -> Node:
        """
        Constructs a a node used for the trie

        Parameters
        ----------
        id: Hashable
            The id for the node

        *args:
            Any extra arguments to pass to the node

        **kwargs:
            Any extra keyword arguments to pass to the node

        Returns
        -------
        :class:`Node`
            The constructed node
        """

        return self._nodeCls(id, *args, **kwargs)
    
    def clearCache(self):
        """
        Clears any cached search results
        """

        self.get.cache_clear()

    def clear(self):
        """
        Clears the data
        """

        self.clearCache()
        self._nodes = {}
        self._children = {}
        self._parent = {}
        self._vals = {}
        self._out = {}
        self._keywords = {}
        self._keywordIds = {}
        self._accept = set()

        self._resetNodeId()
        self._resetKeywordId()
        self._root = self._addNode()

    def _compareKeywordIds(self, keywordId1: int, keywordId2: int) -> int:
        """
        The `compare function`_ for the ids of the keywords :raw-html:`<br />` :raw-html:`<br />`

        The sorting order for keyword ids is as follows:

        #. ids to existing keywords go before ids that do not correspond to a keyword
        #. ids with longer length keywords go before ids with shorter length keywords
        #. keywords of ids are ordered in alphabetical order

        Paramters
        ---------
        keywordId1: :class:`int`
            The id for the first keyword

        keywordId2: :class:`int`
            The id for the second keyword

        Returns
        -------
        :class:`int`
            The comparison result of a `compare function`_
        """

        keyword1 = self._keywords.get(keywordId1)
        keyword2 = self._keywords.get(keywordId2)

        if (keyword1 is None and keyword2 is None):
            return 0
        elif (keyword1 is None):
            return 1
        elif (keyword2 is None):
            return -1
        
        keyword1Len = len(keyword1)
        keyword2Len = len(keyword2)
        if (keyword1Len > keyword2Len):
            return -1
        elif (keyword1Len < keyword2Len):
            return 1
        
        if (keyword1 > keyword2):
            return 1
        elif (keyword1 < keyword2):
            return -1
        
        return 0

    def build(self, data: Optional[Dict[str, T]] = None):
        """
        Rebuilds the `trie`_

        Parameters
        ----------
        data: Optional[Dict[:class:`str`, T]]
            Any initial data to put into the `trie`_ :raw-html:`<br />` :raw-html:`<br />`

            The keys are the keywords to put into the trie and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        self.clear()
        if (data is None):
            data = {}

        for keyword in data:
            self._addKeyword(keyword, data[keyword])

    def _addNode(self) -> Node:
        """
        Add a node into the `trie`_

        Returns
        -------
        :class:`TrieNode`
            The node added to the trie
        """

        node = self._constructNode(self._currentNodeId)
        self._nodes[self._currentNodeId] = node
        self._updateNextNodeId()
        return node
    
    def _addKVP(self, keyword: str, value: T) -> int:
        """
        Adds in a new `KVP`_

        .. warning::
            If 'keyword' already exists, then the new value for the `KVP`_ will be
            determined based off the :attr:`handleDuplicate` function

        Returns
        -------
        :class:`int`
            The id to the keyword
        """

        if (keyword in self._keywordIds):
            keywordId = self._keywordIds[keyword]
            self._vals[keywordId] = self.handleDuplicate(keyword, self._vals[keywordId], value)
            return keywordId

        result = self._currentKeywordId
        self._keywords[self._currentKeywordId] = keyword
        self._keywordIds[keyword] = self._currentKeywordId
        self._vals[self._currentKeywordId] = value

        self._updateNextKeywordId()
        return result
    
    def add(self, keyword: str, value: T) -> Tuple[Node, bool]:
        """
        Adds a new keyword

        Parameters
        ----------
        keyword: :class:`str`
            The keyword to add

        value: T
            The value associated with the keyword

        Returns
        -------
        Tuple[:class:`Node`, :class:`bool`]
            Retrieves the following data:

            #. The node that at the end of the keyword
            #. Whether the keyword has already been inserted
        """
        
        return self._addKeyword(keyword, value)

    def _addKeyword(self, keyword: str, value: T) -> Tuple[Node, bool]:
        """
        Adds a keyword to the `trie`_

        Parameters
        ----------
        keyword: :class:`str`
            The keyword to add

        value: T
            The value associated with the keyword

        Returns
        -------
        Tuple[:class:`Node`, :class:`bool`]
            Retrieves the following data:

            #. The node that at the end of the keyword
            #. Whether the keyword has not already been inserted into the `trie`_
        """

        prevNode = self._root
        newKeyword = False

        for letter in keyword:
            prevChildren = {}
            try:
                prevChildren = self._children[prevNode.id]
            except KeyError:
                self._children[prevNode.id] = prevChildren

            nodeId = prevChildren.get(letter)
            if (nodeId is not None):
                prevNode = self._nodes[nodeId]
                continue

            if (not newKeyword):
                newKeyword = True

            node = self._addNode()
            self._parent[node.id] = prevNode.id
            prevChildren[letter] = node.id
            prevNode = node

        # if the keyword to be inserted is a proper prefix of some keyword that
        #   already exists in the trie
        if (not newKeyword and self._keywordIds.get(keyword) is None):
            newKeyword = True

        # add the KVP
        if (newKeyword):
            keywordId = self._addKVP(keyword, value)
            foundKeywordIds = self._out.get(prevNode.id)

            if (foundKeywordIds is None):
                self._out[prevNode.id] = [keywordId]
                self._accept.add(prevNode.id)
            else:
                Algo.binaryInsert(foundKeywordIds, keywordId, self._compareKeywordIds, optionalInsert = True)
        else:
            keywordId = self._keywordIds[keyword]
            self._vals[keywordId] = self.handleDuplicate(keyword, self._vals[keywordId], value)

        return (prevNode, newKeyword)

    @lru_cache(maxsize = 256)
    def get(self, keyword: str, errorOnNotFound: bool = True, default: Any = None) -> Union[T, Any]:
        """
        Retrieves the corresponding value to 'keyword'

        Parameters
        ----------
        keyword: :class:`str`
            The keyword to get the corresponding value for

        errorOnNotFound: :class:`bool`  
            If the keyword is not found, whether to raise an exception

        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if 'keyword' is not found

        Raises
        ------
        :class:`KeyError`
            If 'keyword' is not found

        Returns
        -------
        Union[T, Any]
            Either the found value for the keyword or the value specified at 'default', if 'keyword' is not found and
            'errorOnNotFound' is set to ``False``
        """

        error = False
        prevNode = self._root

        for letter in keyword:
            if (prevNode.id not in self._children):
                error = True
                break

            nodeId = self._children[prevNode.id].get(letter)
            if (nodeId is None):
                error = True
                break
            
            node = self._nodes[nodeId]
            prevNode = node

        # when there is no output at the reached node
        if (self._out.get(prevNode.id) is None):
            error = True

        if (error and errorOnNotFound):
            raise KeyError(f"{type(self).__name__} does not contain the keyword, '{keyword}'")
        elif (error):
            return default
        
        keywordId = self._out[prevNode.id][0]
        return self._vals[keywordId]
##### EndScript
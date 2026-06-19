"""
C++ internal core of AGRemap
"""
from __future__ import annotations
import collections.abc
import typing
__all__: list[str] = ['BaseDFA', 'BiMap', 'CppIntTools', 'CppListTools', 'DFA', 'Trie']
class BaseDFA:
    def acceptLen(self) -> int:
        """
        Retrieves the number of accepting states in the `DFA`_
        
        Returns
        -------
        :class:`int`
            The number of accepting states in the `DFA`_
        """
    def addState(self, id: typing.Any, isAccept: bool | None = None, isStart: bool = False) -> bool:
        """
        Add a new state to the `DFA`
        
        Parameters
        ----------
        id: Hashable
            The id for the state
        
        isAccept: Optional[:class:`bool`]
            Whether the state is an accepting state :raw-html:`<br />` :raw-html:`<br />`
        
            * If this value is ``None`` and the state already exists, then will not change whether the existing state is accepting or not.
            * Otherwise, if this value is ``None`` and the state does not already exists, then will not set the state as accepting. :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        isStart: :class:`bool`
            Whether to set the state as the new starting state
        
            .. warning::
                A `DFA`_ can only have 1 start state
        
            .. warning::
                If the `DFA`_ is empty and you add a new state, will set this state as the start state
        
            :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        Returns
        -------
        :class:`bool`
            Whether the state was newly added
        """
    def clear(self) -> None:
        """
        Clears the `DFA`_
        """
    def isAccept(self, id: typing.Any) -> bool:
        """
        Determines whether some state is an accepting state
        
        Paramters
        ---------
        id: `Hashable`_
            The id of the state
        
        Returns
        -------
        :class:`bool`
            Whether the corresponding state is an accepting state
        """
    def isStart(self, id: typing.Any) -> bool:
        """
        Determines whether some state is a starting state
        
        Paramters
        ---------
        id: `Hashable`_
            The id of the state
        
        Returns
        -------
        :class:`bool`
            Whether the corresponding state is a starting state
        """
    def reset(self) -> None:
        """
        Resets the `DFA`_ to return back to its starting state
        """
    def stateExists(self, id: typing.Any) -> bool:
        """
        Determines whether some state exists in the `DFA`_
        
        Paramters
        ---------
        id: `Hashable`_
            The id of the state
        
        Returns
        -------
        :class:`bool`
            Whether the id corresponds to a state in the `DFA`_
        """
    def stateLen(self) -> int:
        """
        Retrieves the number of states in the `DFA`_
        
        Returns
        -------
        :class:`int`
            The number of states in the `DFA`_
        """
class BiMap:
    """
    
            A one-to-one dictionary
            
    """
    def __init__(self) -> None:
        ...
    def __len__(self) -> int:
        ...
    def add(self, key: typing.Any, val: typing.Any) -> bool:
        ...
    def clear(self) -> None:
        ...
    def empty(self) -> bool:
        ...
    def findKey(self, key: typing.Any) -> typing.Any | None:
        ...
    def findValue(self, val: typing.Any) -> typing.Any | None:
        ...
    def getKey(self, val: typing.Any) -> typing.Any:
        ...
    def getValue(self, key: typing.Any) -> typing.Any:
        ...
    def insert(self, key: typing.Any, val: typing.Any) -> None:
        ...
class CppIntTools:
    """
    C++ Tools for handling integers
    """
    @staticmethod
    def toBase(num: typing.SupportsInt | typing.SupportsIndex, base: typing.SupportsInt | typing.SupportsIndex) -> tuple[list[int], bool]:
        """
                                Converts a base 10 number to an arbitrary base number
        
                                Parameters
                                ----------
                                num: :class:`int`
                                    The base 10 number to convert
        
                                base: :class:`int`
                                    The base to convert to
        
                                Raises
                                ------
                                :class:`TypeError`
                                    The base is smaller or equal to 1
        
                                Returns
                                -------
                                Tuple[List[:class:`int`], :class:`bool`]
                                    Retrieves the following data in the tuple:
        
                                    #. The digits in the converted number
                                    #. Whether the number is negative
        """
    @staticmethod
    def toBase64(num: typing.SupportsInt | typing.SupportsIndex, getDigit: collections.abc.Sequence[str] | None = None, negativeChar: str = '-') -> str:
        """
                                Converts a base 10 number to a base 64 number
        
                                Parameters
                                ----------
                                num: :class:`int`
                                    The base 10 number to convert
        
                                getDigit: List[:class:`str`]
                                    how to get the string representation of a digit. :raw-html:`<br />` :raw-html:`<br />`
        
                                    * If this argument is a list, each element is the string representation of the digit at the particular index of the string/list.
                                    * If this argument is ``None``, then will use the following string for each digit:
        
                                    ``ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_``
        
                                    This is the same digit representation as the `standard base 64`_ except that the 63rd digit (``/``) is replaced with the ``_`` character :raw-html:`<br />` :raw-html:`<br />`
        
                                    **Default**: ``None``
        
                                negativeChar: :class:`str`
                                    The character representation for the negative symbol :raw-html:`<br />` :raw-html:`<br />`
        
                                    **Default**: ``"-"``
        
                                Returns
                                -------
                                :class:`str`
                                    The converted string representation of the arbitrary base 64 number
        """
    @staticmethod
    def toStrBase(num: typing.SupportsInt | typing.SupportsIndex, base: typing.SupportsInt | typing.SupportsIndex, getDigit: collections.abc.Sequence[str], negativeChar: str) -> str:
        """
                                Converts a base 10 number to an arbitrary base number, such that the characters in this arbitrary based number
                                are all characters
        
                                Parameters
                                ----------
                                num: :class:`int`
                                    The base 10 number to convert
        
                                base: :class:`int`
                                    The base to convert to
        
                                getDigit: List[:class:`str`]
                                    The string representations of each digit. Each element is the string representation
                                    of the digit at the particular index of the list.
        
                                negativeChar: :class:`str`
                                    The character representation for the negative symbol
        
                                Returns
                                -------
                                :class:`str`
                                    The converted string representation of the arbitrary base number
        """
class CppListTools:
    """
    C++ Tools for handling with Lists
    """
    @staticmethod
    def addLstsByInds(lst: list, subLsts: collections.abc.Mapping[typing.SupportsInt | typing.SupportsIndex, list]) -> list:
        """
                                Inserts multiple sublists into the main list by index
        
                                Parameters
                                ----------
                                lst: List[T]
                                    The main list to work with
                                
                                subLsts: Dict[:class:`int`, List[T]]
                                    The sublists to insert into the main list :raw-html:`<br />` :raw-html:`<br />`
        
                                    The keys are the indices to insert the sublists and the values are the sublists
        
                                Returns
                                -------
                                List[T]
                                    The resultant combined list
        """
    @staticmethod
    def getIndsAfterRemove(removedInds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], lstLen: typing.SupportsInt | typing.SupportsIndex) -> list[int]:
        """
                                Retrieve the index shifts in some data structure,
                                after the list got elements removed by indices
        
                                Parameters
                                ----------
                                removedInds: List[:class:`int`] 
                                    The indices to elements that got removed from the list :raw-html:`<br />` :raw-html:`<br />`
        
                                    Assume that the list in sorted order
        
                                lstLen: :class:`int`
                                    The length of the original list, before its elements got removed
        
                                Returns
                                -------
                                List[:class:`int`]
                                    A list containing how much each index is shifted
        """
    @staticmethod
    def removeByInds(lst: list, inds: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> list:
        """
                                Removes many indices from a list
        
                                Parameters
                                ----------
                                lst: List[T]
                                    The desired list to have its parts removed
        
                                inds: Set[:class:`int`]
                                    The indices to the elements in the list that needs to be removed :raw-html:`<br />` :raw-html:`<br />`
        
                                Returns
                                -------
                                List[T]
                                    The new list with elements specified by indices removed
        """
    @staticmethod
    def removeParts(lst: list, part_indices: list) -> list:
        """
                                Removes many indices from a list
        
                                Parameters
                                ----------
                                lst: List[T]
                                    The desired list to have its parts removed
        
                                inds: Set[:class:`int`]
                                    The indices to the elements in the list that needs to be removed
        
                                Returns
                                -------
                                List[T]
                                    The new list with elements specified by indices removed
        """
class DFA(BaseDFA):
    """
    
    Class for a `DFA (Deterministic Finite Automaton)`_
            
    """
    def __init__(self) -> None:
        ...
    def addFuncTransition(self, srcId: typing.Any, func: collections.abc.Callable, destId: typing.Any) -> None:
        """
        Adds a transition to the `DFA`_ such that the transition is based off a predicate function
        
        Parameters
        ----------
        srcId: `Hashable`_
            The id of the source state for the transition
        
            .. caution::
                The id to the source state must refer to an existing state to the `DFA`_
        
        func: Callable[[Hashable], :class:`bool`]
            The predicate function that will trigger a transition from the source state to the destination state :raw-html:`<br />` :raw-html:`<br />`
        
            The function will take in a keyword as an argument
        
            .. warning::
                If the source state already has such a transition, then will overwrite the destination state for this transition
        
        destId: `Hashable`_
            The id of the destionation state for the transition
        
            .. note::
                The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
                will create a new state in the `DFA`_
        """
    def addKeywordTransition(self, srcId: typing.Any, keyword: typing.Any, destId: typing.Any) -> None:
        """
        Adds a transition to the `DFA`_ such that the transition is based off a keyword
        
        Parameters
        ----------
        srcId: `Hashable`_
            The id of the source state for the transition
        
            .. caution::
                The id to the source state must refer to an existing state to the `DFA`_
        
        keyword: `Hashable`_
            The keyword or predicate function that will trigger a transition from the source state to the destination state
        
            .. warning::
                If the source state already has such a transition, then will overwrite the destination state for this transition
        
        destId: `Hashable`_
            The id of the destionation state for the transition
        
            .. note::
                The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
                will create a new state in the `DFA`_
        """
    def addTransition(self, srcId: typing.Any, keyword: typing.Any, destId: typing.Any) -> None:
        """
        Adds a transition to the `DFA`_
        
        Parameters
        ----------
        srcId: `Hashable`_
            The id of the source state for the transition
        
            .. caution::
                The id to the source state must refer to an existing state to the `DFA`_
        
        keyword: Union[`Hashable`_, Callable[[Hashable], :class:`bool`]]
            The keyword or predicate function that will trigger a transition from the source state to the destination state :raw-html:`<br />` :raw-html:`<br />`
        
            If keyword is a predicate function, the function will take in a keyword as an argument
        
            .. warning::
                If the source state already has such a transition, then will overwrite the destination state for this transition
        
        destId: `Hashable`_
            The id of the destionation state for the transition
        
            .. note::
                The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
                will create a new state in the `DFA`_
        """
    def addTransitions(self, srcId: typing.Any, keywords: typing.Any, destId: typing.Any) -> None:
        """
        Adds a group of transitions from one state to another state
        
        Parameters
        ----------
        srcId: `Hashable`_
            The id of the source state for the transition
        
            .. caution::
                The id to the source state must refer to an existing state to the `DFA`_
        
        keywords: Union[List[Union[`Hashable`_, Callable[[Hashable], :class:`bool`]]], `Hashable`_, Callable[[Hashable], :class:`bool`]]
            The keywords or predicate functions that will trigger a transition from the source state to the destination state :raw-html:`<br />` :raw-html:`<br />`
        
            For predicate functions, the function will take in a keyword as an argument
        
            .. warning::
                If the source state already has such a transition, then will overwrite the destination state for this transition
        
        destId: `Hashable`_
            The id of the destionation state for the transition
        
            .. note::
                The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
                will create a new state in the `DFA`_
        """
    def transition(self, keyword: typing.Any) -> tuple[typing.Any, bool, bool]:
        """
        Transitions to a new state
        
        Parameters
        ----------
        keyword: Hashable
            The keyword to trigger the transition to the new state
        
        Returns
        -------
        Tuple[Hashable, :class:`bool`, :class:`bool`]
            Resultant data regarding the new transitioned state, which includes:
        
            #. The id of the new state
            #. Whether the new state is an accepting state
            #. Whether a transition was taken 
        """
    @property
    def currentStateId(self) -> typing.Any | None:
        """
        The id of the state the `DFA`_ is currently at
        
        .. warning::
            The setter will not set the new id for the state if the newly current id does not correspond
            to any state within the `DFA`_
        
        :getter: Retrieves the id of the current state
        :setter: Sets the new id of the current state the `DFA`_ is on
        :type: Hashable
        """
    @currentStateId.setter
    def currentStateId(self, arg1: typing.Any) -> None:
        ...
    @property
    def startId(self) -> typing.Any | None:
        """
        The id to the start state
        
        .. warning::
            The setter will not set the new id for the state if the newly given start id does not correspond
            to any state within the `DFA`_
        
        :getter: Retrieves the start id
        :setter: Sets the new start id
        :type: Hashable
        """
    @startId.setter
    def startId(self, arg1: typing.Any) -> None:
        ...
class Trie:
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
            
    """
    def __contains__(self, key: str) -> bool:
        """
        Determines if 'key' is found
        """
    def __getitem__(self, key: str) -> typing.Any:
        """
        Retrieves the corresponding value to 'key'
        """
    def __init__(self, data: collections.abc.Mapping[str, typing.Any] | None = None, handleDuplicate: collections.abc.Callable[[str, typing.Any, typing.Any], typing.Any] | None = None) -> None:
        ...
    def __setitem__(self, key: str, val: typing.Any) -> bool:
        """
        Sets the new `KVP`_
        """
    def add(self, keyword: str, value: typing.Any) -> bool:
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
        :class:`bool`
            Whether the keyword has already been inserted
        """
    def build(self, data: collections.abc.Mapping[str, typing.Any] | None = None) -> None:
        """
        Rebuilds the `trie`_
        
        Parameters
        ----------
        data: Optional[Dict[:class:`str`, T]]
            Any initial data to put into the `trie`_ :raw-html:`<br />` :raw-html:`<br />`
        
            The keys are the keywords to put into the trie and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def clear(self) -> None:
        """
        Clears the data
        """
    def get(self, keyword: str, errorOnNotFound: bool = True, default: typing.Any = None) -> typing.Any:
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
    @property
    def handleDuplicate(self) -> collections.abc.Callable[[str, typing.Any, typing.Any], typing.Any]:
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
    @handleDuplicate.setter
    def handleDuplicate(self, arg1: collections.abc.Callable[[str, typing.Any, typing.Any], typing.Any]) -> None:
        ...

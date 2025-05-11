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
from ...constants.GenericTypes import T
##### EndLocalImports


##### Script
class BaseAhoCorasickDFA():
    """
    Base class for the `DFA (Deterministic Finite Automaton)`_ used in the `Aho-Corasick`_ algorithm

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
    """

    def __init__(self, data: Optional[Dict[str, T]] = None, handleDuplicate: Optional[Callable[[str, T, T], T]] = None):
        self.handleDuplicate = handleDuplicate
        self._data = {}

        self.build(data)

    def __getitem__(self, txt: str) -> Tuple[Optional[str], T]:
        return self.getMaximal(txt)
    
    def __setitem__(self, keyword: int, value: T):
        self.add(keyword, value)

    def __contains__(self, txt: str) -> bool:
        keyword, ind = self.find(txt)
        return keyword is not None

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

    def clearCache(self):
        """
        Clears any cached search results
        """

        self.find.cache_clear()
        self.findMaximal.cache_clear()
        self.get.cache_clear()
        self.getMaximal.cache_clear()
        self.getKeyVal.cache_clear()

    def clear(self):
        """
        Clears the `DFA`_
        """

        self.clearCache()
        self._data.clear()

    def add(self, keyword: str, value: T):
        """
        Adds a new keyword

        .. caution::
            Adding a new keyword may trigger the entire `DFA`_ to be rebuilt

        Parameters
        ----------
        keyword: :class:`str`
            The keyword to add

        value: T
            The value associated with the keyword
        """

        self.clearCache()
        self._data[keyword] = self.handleDuplicate(keyword, self._data[keyword], value) if (keyword in self._data) else value

    def build(self, data: Optional[Dict[str, T]] = None):
        """
        Rebuilds the `DFA`_

        Parameters
        ----------
        data: Dict[:class:`str`, T]
            The new data to add to the `DFA`_ :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        pass

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

        pass
    
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

        pass
    
    @lru_cache(maxsize = 256)
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

        pass
    
    @lru_cache(maxsize = 256)
    def findMaximal(self, txt: str, count: int = 1) -> Tuple[Union[Optional[str], List[str]], Union[int, List[int]]]:
        """
        Finds the first few largest keywords within 'txt'

        .. note::
            This function is a greedy version of :meth:`find` or `Maximal Munch`_ that consumes only a limited amount of tokens

        Parameters
        ----------
        txt: :class:`str`
            The text to search for the keyword

        count: :class:`int`
            The count of how many keywords to find in the search string :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``1``

        Returns
        -------
        Tuple[Union[Optional[:class:`str`], List[:class:`str`]], Union[:class:`int`, List[:class:`int`]]]
            Data of the found keyword: :raw-html:`<br />` :raw-html:`<br />`

            * If the 'count' argument is less than or equal to 1, then the data will contain:

                #. The keyword found
                #. The starting index of where the keyword was found. If no keywords were found, this index is -1

            * If the 'count' argument is greater than 1, then the data will contain:

                #. The list of keywords found
                #. The corresponding starting indices for where the keyword were found
        """

        pass
    
    @lru_cache(maxsize = 256) 
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

        pass
    
    @lru_cache(maxsize = 256)
    def getMaximal(self, txt: str, errorOnNotFound: bool = True, default: Any = None, count: int = 1) -> Tuple[Union[Optional[str], List[str]], Union[T, Any, List[T]]]:
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

        count: :class:`int`
            The count of how many keywords to find in the search string :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``1``

        Raises
        ------
        :class:`KeyError`
            If no keywords are found

        Returns
        -------
        Tuple[Union[Optional[:class:`str`], List[:class:`str`]], Union[T, Any, List[T]]]
            Retrieves the following resultant data: :raw-html:`<br />` :raw-html:`<br />`

            * If the 'count' argument is less than or equal to 1, then the data contains:

                #. The first largest keyword found
                #. Either the found value for the first largest keyword found or the value specified at 'default', if no keywords were found and
                'errorOnNotFound' is set to ``False``

            * If the 'count' argument is greater than 1, then the data contains:

                #. The list of keywords found
                #. The corresponding found values to the keywords
        """

        pass
    
    @lru_cache(maxsize = 256)
    def getKeyVal(self, txt: str, errorOnNotFound: bool = True, default: Any = None) -> Union[T, Any]:
        """
        Retrieves the corresponding value of the key given in 'txt'

        Parameters
        ----------
        txt: :class:`str`
            The text to search for a keyword

        errorOnNotFound: :class:`bool`  
            If no keywords are found, whether to raise an exception

        default: Any
            If 'errorsOnNotFound' is ``False``, then the default value to return if no keywords are found

        Raises
        ------
        :class:`KeyError`
            If the keyword is found

        Returns
        -------
        Union[T, Any]
            Either the found value for the first largest keyword found or the value specified at 'default', if no keywords were found and
            'errorOnNotFound' is set to ``False``
        """

        pass

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

        pass
##### EndScript
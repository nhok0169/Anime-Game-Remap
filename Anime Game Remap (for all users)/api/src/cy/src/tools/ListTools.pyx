# distutils: name = CyListTools
# cython: language_level=3, boundscheck=False, wraparound=False, nonecheck=False, cdivision=True


cdef class CyListTools():
    """
    Cython tools for handling with Lists
    """

    cpdef list interleave(self, list lst1, list lst2):
        """
        Interleaves 2 lists toghether

        Parameters
        ----------
        lst1: List[T]
            The first list to interleave. Items from this list will be used first in the alternating sequence

        lst2: List[T]
            The second list to interleave. Items from this list will be used second in the alternating sequence

        Returns
        -------
        List[T]
            A new list with the elements from both lists interleaved toghether
        """

        cdef:
            Py_ssize_t i
            Py_ssize_t len1 = len(lst1)
            Py_ssize_t len2 = len(lst2)
            Py_ssize_t m = min(len1, len2)

            list result = []

        for i in range(m):
            result.append(lst1[i])
            result.append(lst2[i])

        result.extend(lst1[m:])
        result.extend(lst2[m:])

        return result

    cpdef list filterInPlace(self, list lst, object predicate):
        """
        Filters a list, in place

        Equivalent to the `built-in filter`_ function, except 'lst' is mutated directly instead
        of a new list/iterator being returned

        Parameters
        ----------
        lst: List[T]
            The list to filter, in place

        predicate: Callable[[T], :class:`bool`]
            The predicate used for the filter :raw-html:`<br />` :raw-html:`<br />`

            Return ``True`` to keep an element, ``False`` to remove it

        Returns
        -------
        List[T]
            Reference to the filtered list (the same object as 'lst')
        """

        cdef Py_ssize_t readInd
        cdef Py_ssize_t writeInd = 0
        cdef Py_ssize_t n = len(lst)
        cdef object item

        for readInd in range(n):
            item = lst[readInd]
            if predicate(item):
                lst[writeInd] = item
                writeInd += 1

        del lst[writeInd:]
        return lst

    cpdef list updateMany(self, list srcList, list lstOfLists):
        """
        Appends the elements from a list of lists onto the end of a source list, in place

        Parameters
        ----------
        srcList: List[T]
            The list to append onto, in place

        lstOfLists: List[List[T]]
            The lists whose elements get appended onto ``srcList``, in the order provided

        Returns
        -------
        List[T]
            Reference to the updated list (the same object as ``srcList``)
        """

        cdef list lst

        for lst in lstOfLists:
            srcList.extend(lst)

        return srcList
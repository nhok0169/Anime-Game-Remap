# distutils: name = CyAlgo
# cython: language_level=3, boundscheck=False, wraparound=False, nonecheck=False, cdivision=True


cdef class CyAlgo():
    """
    Cython tools for handling with some basic algorithms
    """

    cpdef tuple binarySearch(self, list lst, object target, object compare):
        """
        Performs `binary search`_ to search for 'target' in 'lst'

        Parameters
        ----------
        lst: List[T]
            The sorted list we are searching from

        target: T
            The target element to search for in the list

        compare: Callable[[T, T], :class:`int`]
            The `compare function`_ for comparing elements in the list with the target element

        Returns
        -------
        Tuple[:class:`bool`, :class:`int`]
            * The first element is whether the target element is found in the list
            * The second element is the found index or the index that we expect the target element to be in the list
        """

        cdef Py_ssize_t n = len(lst)
        cdef Py_ssize_t left = 0
        cdef Py_ssize_t right
        cdef Py_ssize_t mid
        cdef int compResult
        cdef object midItem

        if n == 0:
            return (False, left)

        right = n - 1
        mid = left + (right - left) / 2

        while left <= right:
            midItem = lst[mid]
            compResult = compare(midItem, target)

            if compResult == 0:
                return (True, mid)
            elif compResult > 0:
                right = mid - 1
            else:
                left = mid + 1

            mid = left + (right - left) / 2

        return (False, left)

    cpdef bint binaryInsert(self, list lst, object target, object compare, bint optionalInsert = False):
        """
        Inserts 'target' into 'lst' using `binary search`_

        Parameters
        ----------
        lst: List[T]
            The sorted list we want to insert the target element

        target: T
            The target element to insert

        compare: Callable[[T, T], :class:`int`]
            The `compare function`_ for comparing elements in the list with the target element

        optionalInsert: :class:`bool`
            Whether to still insert the target element into the list if the element target element is found in the list :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`bool`
            Whether the target element has been inserted into the list
        """

        cdef bint found
        cdef Py_ssize_t insertInd
        cdef bint inserted = False

        found, insertInd = self.binarySearch(lst, target, compare)

        if not optionalInsert or not found:
            lst.insert(insertInd, target)
            inserted = True

        return inserted

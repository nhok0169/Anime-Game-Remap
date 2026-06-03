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
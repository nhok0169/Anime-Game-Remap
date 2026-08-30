#ifndef AGRemapPyBind_PyIndices_H
#define AGRemapPyBind_PyIndices_H

#include <pybind11/pybind11.h>

#include "PyModMappedAssets.h"


namespace py = pybind11;


/**
 * @brief
 @rst
 The `pybind11`_ bound ``Indices`` class -- the exact same pattern as :cpp:class:`PyHashes` (see
 its own class-level note): one *specific*, pre-populated instance of
 :cpp:class:`PyModMappedAssets`, with its index data (:cpp:func:`AGRemapCore::Data::getIndexDataRows`)
 and non-version index names (``name``, ``component``, ``type``) baked in at construction,
 matching the pure-Python ``Indices`` class's own constructor contract (``model/assets/Indices.py``,
 now removed entirely -- see git history) exactly: a bare, optional ``map`` is the only real
 constructor argument
 @endrst
 */
class PyIndices: public PyModMappedAssets {
    public:
        /**
         * @brief Constructs a new, fully-populated index lookup table
         *
         * @param map The initial `adjacency list`_ mapping indices to fix from to indices to fix
         *      to -- see :cpp:func:`AGRemapPyBind::convertMap`. ``py::none()`` for none
         */
        explicit PyIndices(const py::object &map = py::none());
};


void initCppIndices(pybind11::module_ &m);

#endif

#ifndef AGRemapPyBind_PyHashes_H
#define AGRemapPyBind_PyHashes_H

#include <pybind11/pybind11.h>

#include "PyModMappedAssets.h"


namespace py = pybind11;


/**
 * @brief
 @rst
 The `pybind11`_ bound ``Hashes`` class -- unlike :cpp:class:`PyModMappedAssets` (a generic,
 reusable engine), this is one *specific*, pre-populated instance of it: its hash data
 (:cpp:func:`AGRemapCore::Data::getHashDataRows`) and non-version index names (``name``,
 ``type``) are baked in at construction, matching the pure-Python ``Hashes`` class's own
 constructor contract (``model/assets/Hashes.py``, now removed entirely -- see git history)
 exactly: a bare, optional ``map`` is the only real constructor argument :raw-html:`<br />` :raw-html:`<br />`

 .. note::
    Bound directly as the bare ``Hashes`` -- there is no Python source file behind this class at
    all any more, not even a thin wrapper: :cpp:class:`PyModMappedAssets` already provides every
    method (:cpp:func:`toWildcardList`-backed argument normalization included, via its
    ``nonVersionIndexNames``); this class only supplies the data. See :cpp:class:`PyModDictAssets`'s
    own note on the "two outcomes for porting a class" convention this project follows -- this is
    a step further than either of those two outcomes (outcome 2, full replacement, but with no
    Python file involved at construction time either, since the data itself moved into C++ too)
 @endrst
 */
class PyHashes: public PyModMappedAssets {
    public:
        /**
         * @brief Constructs a new, fully-populated hash lookup table
         *
         * @param map The initial `adjacency list`_ mapping hashes to fix from to hashes to fix
         *      to -- see :cpp:func:`AGRemapPyBind::convertMap`. ``py::none()`` for none
         */
        explicit PyHashes(const py::object &map = py::none());
};


void initCppHashes(pybind11::module_ &m);

#endif

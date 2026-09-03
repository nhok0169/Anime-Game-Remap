#ifndef AGRemapPyBind_PyModAssets_H
#define AGRemapPyBind_PyModAssets_H

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/assets/ModAssets.h"
#include "../../tools/PyTools.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


// K and T are both bound as 'py::object' -- see PyModDictAssets.h's comment (this file's own
// precedent) for why. Unlike ModDictAssets/ModMappedAssets, this class does no hashing of K at
// all (a plain linear scan -- see AGRC::ModAssets's own class-level note for why), so only
// PyObjectEqual is needed, not PyObjectHash.
extern template class AGRC::ModAssets<std::string, py::object>;


/**
 * @brief
 @rst
 The `pybind11`_ bound version of ``ModAssets``, using ``py::object`` for both the index value
 and leaf content type -- adds nothing over :cpp:class:`AGRC::ModAssets` itself, the same
 "just a concrete name for pybind11 to register" pattern as :cpp:class:`PyModDictAssets`
 :raw-html:`<br />` :raw-html:`<br />`

 .. note::
    Bound as ``CppModAssets``, not the bare ``ModAssets`` -- the pure-Python ``ModAssets``
    (``model/assets/ModAssets.py``) already holds that name; see :cpp:class:`PyModDictAssets`'s
    own note on the same convention
 @endrst
 */
// The value-preserving twin of PyModDictAssets.h's convertRowsOrNestedDict. That one builds
// Row<std::string, std::string>, which is right for Hashes/Indices -- a hash IS a string. Every
// Python subclass of the bound ModAssets holds something else, though (VGRemaps[VGRemap],
// VertexCounts[int], PositionEditors[BaseBufEditor], and two BuilderArgs[Callable]), so this one
// keeps the leaf as whatever object the caller supplied.
std::vector<AGRC::Row<std::string, py::object>> convertObjRowsOrNestedDict(const py::object &rowsOrNestedDict,
                                                                           std::size_t totalIndices);


class PyModAssets: public AGRC::ModAssets<std::string, py::object> {
    public:
        using Base = AGRC::ModAssets<std::string, py::object>;
        using Base::Base;
};


void initCppModAssets(pybind11::module_ &m);

#endif

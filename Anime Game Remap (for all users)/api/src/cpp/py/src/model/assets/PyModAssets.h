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
 The `pybind11`_ bound version of ``ModAssets``, using ``std::string`` index values and
 ``py::object`` leaf content -- the generic, multi-version-column asset table
 :raw-html:`<br />` :raw-html:`<br />`

 .. note::
    Bound directly as the bare ``ModAssets``. It used to be ``CppModAssets``, wrapped by a
    pure-Python ``model/assets/ModAssets.py`` subclass that existed only to add back the
    name-keyed constructor/``get`` argument shapes; once that wrapper's last three subclasses
    (``PositionEditors`` and the two ``Ini*BuilderArgs``) were removed, the wrapper had nothing
    left of its own and was folded in here -- the same "outcome 1 collapsing into outcome 2"
    route :cpp:class:`IfContentPart` took. So this class carries a little pybind-layer-only
    metadata the Python-free core has no concept of (see #indices), exactly like
    :cpp:member:`PyModMappedAssets::nonVersionIndexNames`
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

        /**
         * @brief
         @rst
         Every index column's name, in column order -- pybind-layer-only metadata (the
         Python-free core is strictly positional and has no notion of a column *name*), set at
         construction time. Defaults to ``["version", "name"]``, matching the pure-Python
         ``ModAssets`` this replaced
         @endrst
         */
        std::vector<std::string> indices;

        /**
         * @brief The subset of #indices naming a version column, in column order -- what
         *      ``get``'s 'versionVals' argument is keyed by when a dict is passed
         */
        std::vector<std::string> versionIndexNames;

        /**
         * @brief The subset of #indices *not* naming a version column, in column order -- what
         *      ``get``'s 'nonVersionVals' argument is keyed by when a dict is passed
         */
        std::vector<std::string> nonVersionIndexNames;

        /**
         * @brief
         @rst
         Unused by the lookup itself (a row carries its own value rather than selecting one by
         column name) -- kept only because the pure-Python ``ModAssets``'s constructor accepted
         it, and dropping a parameter is a breaking change for no gain
         @endrst
         */
        std::string valueCol;
};


void initCppModAssets(pybind11::module_ &m);

#endif

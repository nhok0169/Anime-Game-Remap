#ifndef AGRemapPyBind_PyModMappedAssets_H
#define AGRemapPyBind_PyModMappedAssets_H

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/assets/ModMappedAssets.h"
#include "PyModDictAssets.h"
#include "../../tools/PyTools.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief The `adjacency list`_ shape :cpp:class:`AGRC::ModMappedAssets`'s ``map`` constructor
 *      argument takes, with both sides bound as ``py::object`` -- shared by
 *      :cpp:class:`PyModMappedAssets`'s own constructor and :cpp:class:`PyHashes`'s (which needs
 *      the exact same ``Optional[Dict[Any, List[Any]]]`` -> internal-map conversion for its own
 *      ``map`` argument)
 */
using PyObjectMap = std::unordered_map<std::string, std::vector<std::string>>;

/**
 * @brief Converts a Python ``Dict[Any, List[Any]]`` into #PyObjectMap
 */
PyObjectMap convertMap(const py::dict &mapDict);


// K, T, and the asset value hash/equal are all bound as 'py::object'/PyObjectHash/PyObjectEqual
// -- see PyModDictAssets.h's (and, further back, PyOrderedMultiMap.h's) comment for why.
extern template class AGRC::ModMappedAssets<std::string, std::string>;


/**
 * @brief
 @rst
 The `pybind11`_ bound version of ``ModMappedAssets``, using ``py::object`` throughout -- adds
 one thing over :cpp:class:`AGRC::ModMappedAssets` itself (see #nonVersionIndexNames), otherwise
 the same "just a concrete name for pybind11 to register" pattern as :cpp:class:`PyModDictAssets`
 :raw-html:`<br />` :raw-html:`<br />`

 .. note::
    Bound directly as the bare ``ModMappedAssets`` -- see :cpp:class:`PyModDictAssets`'s own note;
    the same full-replacement convention applies here. The old pure-Python implementation lives
    on as ``ModMappedAssetsOld`` (``model/assets/ModMappedAssetsOld.py``)
 @endrst
 */
class PyModMappedAssets: public AGRC::ModMappedAssets<std::string, std::string> {
    public:
        using Base = AGRC::ModMappedAssets<std::string, std::string>;
        using Base::Base;

        /**
         * @brief
         @rst
         The names of the non-version index columns, in position order -- pybind-layer-only
         metadata (there's no such concept in the Python-free core, which stays strictly
         positional), set at construction time :raw-html:`<br />` :raw-html:`<br />`

         When present, ``hasFrom``/``getKey``/``replace``/``replaceAll``/``_convertNonVersionVals``
         accept the same flexible bare-value/list/dict-keyed-by-name argument shape the pure-Python
         ``Hashes``/``Indices`` used to provide via their own (now removed)
         ``_convertNonVersionVals`` override, instead of requiring an already-positional list --
         see :cpp:func:`toWildcardList`. ``std::nullopt`` (the default) preserves this class's
         prior behaviour exactly: a strictly positional list (or ``None`` for "no filtering at
         all"), with no name-keyed or bare-value convenience -- the right default for any
         :cpp:class:`ModMappedAssets` use that isn't backed by named indices
         @endrst
         */
        std::optional<std::vector<std::string>> nonVersionIndexNames;
};


void initCppModMappedAssets(pybind11::module_ &m);

#endif

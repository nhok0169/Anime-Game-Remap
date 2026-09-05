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
 *      ``ModMappedAssets``'s own constructor and ``Hashes``'s (which needs
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
/**
 * @brief
 @rst
 The one ``ModMappedAssets`` instantiation this project binds :raw-html:`<br />`
 :raw-html:`<br />`

 There used to be a ``PyModMappedAssets`` subclass here, and that was the whole problem: it made
 the `Python`_-facing ``ModMappedAssets`` a **different C++ type** from
 :cpp:class:`AGRemapCore::Hashes`/:cpp:class:`AGRemapCore::Indices`, which derive from this same
 instantiation directly. They were siblings, not relatives, so a :cpp:member:`ModType::hashes`
 could not cross into `Python`_ at all -- which is why ``ModType`` could expose none of its four
 asset tables. The one thing that subclass added, ``nonVersionIndexNames``, moved into the core
 class, and the subclass is gone
 @endrst
 */
using CoreModMappedAssets = AGRC::ModMappedAssets<std::string, std::string>;


void initCppModMappedAssets(pybind11::module_ &m);

#endif

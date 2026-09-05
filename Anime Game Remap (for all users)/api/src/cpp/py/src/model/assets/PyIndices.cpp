#include "PyIndices.h"

#include <utility>

#include "PyModMappedAssets.h"

#include "AGRemapCore/model/assets/Indices.h"


void initCppIndices(pybind11::module_ &m) {
    py::class_<AGRC::Indices, CoreModMappedAssets, py::smart_holder>(m, "Indices", R"doc(
This class inherits from :class:`ModMappedAssets`

Class for managing indices for a mod, pre-populated with this project's real index data

:raw-html:`<br />`

.. note::
    Names of the available indices used for querying with the ``get``/``hasFrom``/``getKey``/
    ``replace``/``replaceAll`` methods (inherited from :class:`ModMappedAssets`) are:

    * version (version index)
    * name
    * component
    * type
    )doc")

        // See PyHashes.cpp's identical note: the core class owns the table and its index names.
        .def(py::init([](const py::object &map) {
            return std::make_unique<AGRC::Indices>(map.is_none() ? PyObjectMap{} : convertMap(map.cast<py::dict>()));
        }), py::arg("map") = py::none(), py::doc(R"doc(
Constructs a new, fully-populated index lookup table

Parameters
----------
map: Optional[Dict[Any, List[Any]]]
    The `adjacency list`_ that maps the indices to fix from to the indices to fix to using the
    predefined mods

    **Default**: ``None``
        )doc"));
}

#include "PyIndices.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/data/IndexData.h"
#include "../PyVersion.h"


namespace {

    AGRC::ModDictAssets<std::string, std::string> buildIndexRepo() {
        const auto &rawRows = AGRC::Data::getIndexDataRows();

        std::vector<AGRC::Row<std::string, std::string>> rows;
        rows.reserve(rawRows.size());
        for (const auto &rawRow : rawRows) {
            std::vector<std::string> indexVals;
            indexVals.reserve(rawRow.first.size());
            for (const std::string &v : rawRow.first) {
                indexVals.push_back(v);
            }
            rows.push_back(AGRC::Row<std::string, std::string>{std::move(indexVals), py::str(rawRow.second).cast<std::string>()});
        }

        // 4 total indices (version, name, component, type), version at position 0 -- matches the
        // pure-Python IndexData's own nesting depth/order exactly (see Indices.py's history, now
        // removed).
        return AGRC::ModDictAssets<std::string, std::string>(4, 0,
            // ModDictAssets::VersionParser is std::function<optional<Version>(const K&)>, and K
            // is std::string here -- parseVersionArg still speaks py::object, so it is adapted.
            [](const std::string &v) { return parseVersionArg(py::cast(v)); },
            std::move(rows));
    }

}


PyIndices::PyIndices(const py::object &map)
    : PyModMappedAssets(buildIndexRepo(), map.is_none() ? PyObjectMap{} : convertMap(map.cast<py::dict>())) {
    nonVersionIndexNames = std::vector<std::string>{"name", "component", "type"};
}


void initCppIndices(pybind11::module_ &m) {
    py::class_<PyIndices, PyModMappedAssets>(m, "Indices", R"doc(
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

        .def(py::init<const py::object&>(), py::arg("map") = py::none(), py::doc(R"doc(
Constructs a new, fully-populated index lookup table

Parameters
----------
map: Optional[Dict[Any, List[Any]]]
    The `adjacency list`_ that maps the indices to fix from to the indices to fix to using the
    predefined mods

    **Default**: ``None``
        )doc"));
}

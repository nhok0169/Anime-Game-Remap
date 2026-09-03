#include "PyHashes.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/data/HashData.h"
#include "../PyVersion.h"


namespace {

    AGRC::ModDictAssets<std::string, std::string> buildHashRepo() {
        const auto &rawRows = AGRC::Data::getHashDataRows();

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

        // 3 total indices (version, name, type), version at position 0 -- matches the pure-Python
        // HashData's own nesting depth/order exactly (see Hashes.py's history, now removed).
        return AGRC::ModDictAssets<std::string, std::string>(3, 0,
            // ModDictAssets::VersionParser is std::function<optional<Version>(const K&)>, and K
            // is std::string here -- parseVersionArg still speaks py::object, so it is adapted.
            [](const std::string &v) { return parseVersionArg(py::cast(v)); },
            std::move(rows));
    }

}


PyHashes::PyHashes(const py::object &map)
    : PyModMappedAssets(buildHashRepo(), map.is_none() ? PyObjectMap{} : convertMap(map.cast<py::dict>())) {
    nonVersionIndexNames = std::vector<std::string>{"name", "type"};
}


void initCppHashes(pybind11::module_ &m) {
    py::class_<PyHashes, PyModMappedAssets>(m, "Hashes", R"doc(
This class inherits from :class:`ModMappedAssets`

Class for managing hashes for a mod, pre-populated with this project's real hash data

:raw-html:`<br />`

.. note::
    Names of the available indices used for querying with the ``get``/``hasFrom``/``getKey``/
    ``replace``/``replaceAll`` methods (inherited from :class:`ModMappedAssets`) are:

    * version (version index)
    * name
    * type
    )doc")

        .def(py::init<const py::object&>(), py::arg("map") = py::none(), py::doc(R"doc(
Constructs a new, fully-populated hash lookup table

Parameters
----------
map: Optional[Dict[Any, List[Any]]]
    The `adjacency list`_ that maps the hashes to fix from to the hashes to fix to using the
    predefined mods

    **Default**: ``None``
        )doc"));
}

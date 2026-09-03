#include "PyModAssets.h"

#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../PyVersion.h"
#include "PyModDictAssets.h"


template class AGRC::ModAssets<std::string, py::object>;

namespace {

    // Matches the pure-Python ModAssets' own class constants, which its defaults are spelled in
    // terms of.
    const std::string VersionKey = "version";
    const std::string NameKey = "name";
    const std::string ValueKey = "value";

    // 'versionVals' arrives in the same flexible shapes as 'nonVersionVals' (a bare value, a list,
    // a name-keyed dict, or None), so it goes through toWildcardList first and is only parsed
    // after -- a column left unspecified stays std::nullopt, meaning "latest available here".
    std::vector<std::optional<AGRC::Version>> toVersionList(const py::object &raw,
                                                            const std::vector<std::string> &versionIndexNames) {
        std::vector<std::optional<std::string>> wildcards = toWildcardList(raw, versionIndexNames);

        std::vector<std::optional<AGRC::Version>> result;
        result.reserve(wildcards.size());
        for (const std::optional<std::string> &val : wildcards) {
            result.push_back(val.has_value() ? parseVersionArg(py::cast(*val)) : std::nullopt);
        }
        return result;
    }

    void flattenObjNestedDictNode(const py::object &node, std::vector<std::string> &path, std::size_t depth,
                                   std::size_t totalIndices, std::vector<AGRC::Row<std::string, py::object>> &rows) {
        if (depth == totalIndices) {
            rows.push_back(AGRC::Row<std::string, py::object>{path, py::reinterpret_borrow<py::object>(node)});
            return;
        }

        if (!py::isinstance<py::dict>(node)) {
            throw py::value_error("convertObjRowsOrNestedDict: expected a dict at depth " + std::to_string(depth)
                                  + " (totalIndices=" + std::to_string(totalIndices) + ")");
        }

        py::dict asDict = node.cast<py::dict>();
        for (auto item : asDict) {
            path.push_back(py::str(item.first).cast<std::string>());
            flattenObjNestedDictNode(py::reinterpret_borrow<py::object>(item.second), path, depth + 1, totalIndices, rows);
            path.pop_back();
        }
    }

}


std::vector<AGRC::Row<std::string, py::object>> convertObjRowsOrNestedDict(const py::object &rowsOrNestedDict,
                                                                           std::size_t totalIndices) {
    std::vector<AGRC::Row<std::string, py::object>> rows;

    if (py::isinstance<py::dict>(rowsOrNestedDict)) {
        std::vector<std::string> path;
        flattenObjNestedDictNode(rowsOrNestedDict, path, 0, totalIndices, rows);
        return rows;
    }

    // The index values are still keys, so they narrow to strings; only the leaf stays an object.
    auto flat = rowsOrNestedDict.cast<std::vector<std::pair<std::vector<std::string>, py::object>>>();
    rows.reserve(flat.size());
    for (auto &row : flat) {
        rows.push_back(AGRC::Row<std::string, py::object>{std::move(row.first), std::move(row.second)});
    }

    return rows;
}


void initCppModAssets(pybind11::module_ &m) {
    py::class_<PyModAssets> modAssets(m, "ModAssets", R"doc(
Class to handle assets of any type for a mod where retrieval is based on some keys where 1 or more
of the keys refer to some versioning

:raw-html:`<br />`

.. tip::
    If the assets have more than 1 column that refers to some version, use this data structure.
    Otherwise if your asset has only 1 column that refers to some version, it is recommended to use
    :class:`ModDictAssets` instead, since that uses a hash based access instead of a linear scan
    )doc");

    // The pure-Python ModAssets exposed these as class constants, and its own defaults are spelled
    // in terms of them, so they stay part of the public surface.
    modAssets.attr("VersionKey") = VersionKey;
    modAssets.attr("NameKey") = NameKey;
    modAssets.attr("ValueKey") = ValueKey;

    modAssets
        .def(py::init([](const py::object &repo, const py::object &indices, const py::object &versionIndices,
                         const py::object &valueCol, const py::kwargs &) {
            std::vector<std::string> indexNames = indices.is_none()
                ? std::vector<std::string>{VersionKey, NameKey}
                : indices.cast<std::vector<std::string>>();

            std::unordered_set<std::string> uniqueIndices(indexNames.begin(), indexNames.end());
            if (uniqueIndices.size() != indexNames.size()) {
                throw py::key_error("Index names must be unique");
            }

            std::unordered_set<std::string> versionNames = versionIndices.is_none()
                ? std::unordered_set<std::string>{VersionKey}
                : versionIndices.cast<std::unordered_set<std::string>>();

            std::vector<bool> isVersionColumn;
            std::vector<std::string> versionIndexNames;
            std::vector<std::string> nonVersionIndexNames;
            isVersionColumn.reserve(indexNames.size());

            for (const std::string &indexName : indexNames) {
                // Intersected with the real index names, matching the pure-Python original -- a
                // version index that isn't an index at all is simply dropped rather than an error.
                bool isVersion = versionNames.count(indexName) > 0;
                isVersionColumn.push_back(isVersion);
                (isVersion ? versionIndexNames : nonVersionIndexNames).push_back(indexName);
            }

            auto result = std::make_unique<PyModAssets>(isVersionColumn,
                // VersionParser speaks K, which is std::string here -- adapt parseVersionArg.
                [](const std::string &v) { return parseVersionArg(py::cast(v)); },
                convertObjRowsOrNestedDict(repo, indexNames.size()));

            result->indices = std::move(indexNames);
            result->versionIndexNames = std::move(versionIndexNames);
            result->nonVersionIndexNames = std::move(nonVersionIndexNames);
            result->valueCol = valueCol.is_none() ? ValueKey : py::str(valueCol).cast<std::string>();

            return result;
        }), py::arg("repo"), py::arg("indices") = py::none(), py::arg("versionIndices") = py::none(),
            py::arg("valueCol") = py::none(), py::doc(R"doc(
Constructs a new asset lookup table

:raw-html:`<br />`

.. note::
    Any extra keyword argument is accepted and ignored, matching the pure-Python original this
    replaced (whose own constructor ended in ``**kwargs``)

Parameters
----------
repo: Union[List[Tuple[List[Any], Any]], dict]
    The original source for the assets -- either an already-flattened list of ``(indexVals, value)``
    tuples, or a nested dict exactly ``len(indices)`` levels deep

indices: Optional[List[:class:`str`]]
    The names of the index columns to query to retrieve the main content of the asset
    :raw-html:`<br />` :raw-html:`<br />`

    If this value is ``None``, then will set 2 index columns by the names "version" and "name"

    **Default**: ``None``

versionIndices: Optional[Set[:class:`str`]]
    The names of the index columns that refer to some version -- any name not also in 'indices' is
    ignored :raw-html:`<br />` :raw-html:`<br />`

    If this value is ``None``, then will set an index to the name "version"

    **Default**: ``None``

valueCol: Optional[:class:`str`]
    Unused by the lookup (rows already carry their own value, rather than one being selected by
    column name) -- kept for constructor-signature backward compatibility

    **Default**: ``None``

Raises
------
:class:`KeyError`
    If 'indices' contains a duplicate name

:class:`ValueError`
    If 'repo' is a dict that isn't nested exactly ``len(indices)`` levels deep
        )doc"))

        .def("addRows", [](PyModAssets &self, const py::object &rows) {
            self.addRows(convertObjRowsOrNestedDict(rows, self.getTotalIndices()));
        }, py::arg("rows"), py::doc(R"doc(
Adds new rows to the table (an addition beyond the pure-Python original, which has no
incremental-add capability at all) -- overwrites the value of any row whose full key already exists

Parameters
----------
rows: Union[List[Tuple[List[Any], Any]], dict]
    The rows to add, in the same shape as the constructor's own 'repo' argument
        )doc"))

        .def("get", [](const PyModAssets &self, const py::object &nonVersionVals, const py::object &versionVals,
                       bool errorOnNotFound, const py::object &default_) -> py::object {
            std::optional<py::object> result = self.get(toWildcardList(nonVersionVals, self.nonVersionIndexNames),
                                                        toVersionList(versionVals, self.versionIndexNames),
                                                        false);
            if (!result.has_value()) {
                if (errorOnNotFound) {
                    throw py::key_error("No matching asset found for the given non-version/version values");
                }
                return default_;
            }

            // Already a py::object -- the whole point of this table's value type.
            return *result;
        }, py::arg("nonVersionVals"), py::arg("versionVals") = py::none(), py::arg("errorOnNotFound") = true,
           py::arg("default") = py::none(), py::doc(R"doc(
Retrieves the corresponding asset

Parameters
----------
nonVersionVals: Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]
    The values of the index columns that do not reference a version -- a bare value (taken as the
    first such column), a positional list, or a dict keyed by index name. A column left
    unspecified matches anything there

versionVals: Optional[Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]]
    The values of the index columns that reference a version, in the same accepted shapes
    :raw-html:`<br />` :raw-html:`<br />`

    .. note::
        If the value for a particular version column is ``None``, then will get the latest version
        for that column -- among the rows still matching everything resolved before it, since
        version columns are resolved sequentially in index order

    **Default**: ``None``

errorOnNotFound: :class:`bool`
    If no assets are found, whether to raise an exception :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

default: Any
    If 'errorOnNotFound' is ``False``, then the default value to return if no assets are found
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Raises
------
:class:`KeyError`
    If the corresponding asset based on the search parameters is not found and 'errorOnNotFound' is
    set to ``True``

Returns
-------
Any
    Either the found asset, or the value specified from 'default' if 'errorOnNotFound' is set to
    ``False``
        )doc"))

        .def_property_readonly("indices", [](const PyModAssets &self) { return self.indices; },
            py::doc(R"doc(List[:class:`str`]: The names of the index columns to query to retrieve the main content of an asset)doc"))

        .def_property_readonly("versionIndices", [](const PyModAssets &self) {
            py::set result;
            for (const std::string &name : self.versionIndexNames) {
                result.add(py::str(name));
            }
            return result;
        }, py::doc(R"doc(Set[:class:`str`]: The names of the index columns that refer to some version)doc"))

        .def_property_readonly("valueCol", [](const PyModAssets &self) { return self.valueCol; },
            py::doc(R"doc(:class:`str`: Unused by the lookup -- see the constructor's own note)doc"))

        .def_property_readonly("totalIndices", &PyModAssets::getTotalIndices,
            py::doc(R"doc(:class:`int`: The total number of index columns)doc"))

        .def_property_readonly("versionColumnCount", &PyModAssets::getVersionColumnCount,
            py::doc(R"doc(:class:`int`: The number of version columns)doc"))

        .def_property_readonly("nonVersionColumnCount", &PyModAssets::getNonVersionColumnCount,
            py::doc(R"doc(:class:`int`: The number of non-version columns)doc"))

        .def("__len__", &PyModAssets::size,
            py::doc(R"doc(The total number of rows currently in the table)doc"))

        // A fresh py::class_ has neither copy.copy() nor copy.deepcopy() until they're bound, and
        // these tables are mutable -- see Architecture's note on that gap.
        .def("clone", [](const PyModAssets &self) { return PyModAssets(self); }, py::doc(R"doc(
Creates an independent copy of this table

Returns
-------
:class:`ModAssets`
    The copied table
        )doc"))

        .def("__copy__", [](const PyModAssets &self) { return PyModAssets(self); })

        .def("__deepcopy__", [](const PyModAssets &self, const py::dict &) { return PyModAssets(self); },
            py::arg("memo"));
}

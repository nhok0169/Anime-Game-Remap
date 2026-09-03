#include "PyModDictAssets.h"

#include <utility>
#include <vector>

#include "../PyVersion.h"


template class AGRC::ModDictAssets<std::string, std::string>;

// Rows cross the pybind boundary as a plain list of (indexVals, value) tuples --
// Python: [(["1.0", "A", "x"], "hash1"), ...] -- an alternative to flattenNestedDict below for
// callers that already have (or want to build) the flat shape directly.
std::vector<AGRC::Row<std::string, std::string>> convertRows(const std::vector<std::pair<std::vector<std::string>, std::string>> &rows) {
    std::vector<AGRC::Row<std::string, std::string>> converted;
    converted.reserve(rows.size());
    for (const auto &row : rows) {
        converted.push_back(AGRC::Row<std::string, std::string>{row.first, row.second});
    }
    return converted;
}

namespace {

    void flattenNestedDictNode(const py::object &node, std::vector<std::string> &path, std::size_t depth, std::size_t totalIndices, std::vector<AGRC::Row<std::string, std::string>> &rows) {
        if (depth == totalIndices) {
            rows.push_back(AGRC::Row<std::string, std::string>{path, py::str(node).cast<std::string>()});
            return;
        }

        if (!py::isinstance<py::dict>(node)) {
            throw py::value_error("flattenNestedDict: expected a dict at depth " + std::to_string(depth) + " (totalIndices=" + std::to_string(totalIndices) + "), got " + py::str(node.get_type()).cast<std::string>());
        }

        py::dict asDict = node.cast<py::dict>();
        for (auto item : asDict) {
            path.push_back(py::str(item.first).cast<std::string>());
            flattenNestedDictNode(py::reinterpret_borrow<py::object>(item.second), path, depth + 1, totalIndices, rows);
            path.pop_back();
        }
    }

}

std::vector<AGRC::Row<std::string, std::string>> flattenNestedDict(const py::dict &repo, std::size_t totalIndices) {
    std::vector<AGRC::Row<std::string, std::string>> rows;
    std::vector<std::string> path;
    flattenNestedDictNode(repo, path, 0, totalIndices, rows);
    return rows;
}

std::vector<AGRC::Row<std::string, std::string>> convertRowsOrNestedDict(const py::object &rowsOrNestedDict, std::size_t totalIndices) {
    if (py::isinstance<py::dict>(rowsOrNestedDict)) {
        return flattenNestedDict(rowsOrNestedDict.cast<py::dict>(), totalIndices);
    }
    return convertRows(rowsOrNestedDict.cast<std::vector<std::pair<std::vector<std::string>, std::string>>>());
}

namespace {

    // 'raw' is UnHashableNone (FixRaidenBoss2.tools.DictTools.UnHashableNone), the pure-Python
    // codebase's own alternative "no value given" sentinel -- still the documented default for
    // GIMIParser.py's hashNonVersionVals/indexNonVersionVals and ModType.py's getHashRanges, both
    // of which flow straight into toWildcardList below. Resolved lazily (first real call, well
    // after FixRaidenBoss2/__init__.py has finished importing this very module -- no circular
    // import risk) and cached, rather than importing FixRaidenBoss2.tools.DictTools eagerly at
    // bindings.cpp's module-init time.
    bool isUnHashableNone(const py::object &raw) {
        static py::object unHashableNoneClass;
        static bool resolved = false;

        if (!resolved) {
            resolved = true;
            try {
                unHashableNoneClass = py::module_::import("FixRaidenBoss2.tools.DictTools").attr("UnHashableNone");
            } catch (const py::error_already_set &) {
                unHashableNoneClass = py::object();
            }
        }

        if (!unHashableNoneClass) {
            return false;
        }
        return raw.is(unHashableNoneClass) || py::isinstance(raw, unHashableNoneClass);
    }

}

std::vector<std::optional<std::string>> toWildcardList(const py::object &raw, const std::vector<std::string> &indexNames) {
    std::vector<std::optional<std::string>> result(indexNames.size(), std::nullopt);

    if (raw.is_none() || isUnHashableNone(raw)) {
        return result;
    }

    // Deliberately py::list only, not any generic iterable/tuple -- matches the pure-Python
    // original's own strict `isinstance(indexVals, list)` check exactly (see this function's
    // header doc comment for why that matters: a bare str/tuple falls through to the "bare
    // value" case below instead of being iterated element-by-element).
    if (py::isinstance<py::list>(raw)) {
        py::list asList = raw.cast<py::list>();
        std::size_t len = asList.size();
        for (std::size_t i = 0; i < result.size() && i < len; ++i) {
            py::object item = py::reinterpret_borrow<py::object>(asList[i]);
            if (!item.is_none()) {
                result[i] = py::str(item).cast<std::string>();
            }
        }
        return result;
    }

    if (py::isinstance<py::dict>(raw)) {
        py::dict asDict = raw.cast<py::dict>();
        for (std::size_t i = 0; i < result.size(); ++i) {
            py::str key(indexNames[i]);
            if (asDict.contains(key)) {
                py::object item = py::reinterpret_borrow<py::object>(asDict[key]);
                if (!item.is_none()) {
                    result[i] = py::str(item).cast<std::string>();
                }
            }
        }
        return result;
    }

    if (!result.empty()) {
        result[0] = py::str(raw).cast<std::string>();
    }
    return result;
}


void initCppModDictAssets(pybind11::module_ &m) {
    py::class_<PyModDictAssets>(m, "ModDictAssets", R"doc(
Handles assets of any type for a mod where retrieval is based on some keys where only one of the
keys refers to some versioning

:raw-html:`<br />`

Internally, the source data is never a nested dict -- rows are stored already-flattened, as
``(indexVals, value)`` tuples, where ``indexVals`` holds every index column's raw value in index
order (including the version index's own raw, not-yet-parsed value). The constructor takes rows
already in that shape; :meth:`fromNestedDict` builds an instance from a real nested dict instead
(the shape ``HashData``/``IndexData`` are written as), flattening it in C++ rather than Python
    )doc")

        .def(py::init([](std::size_t totalIndices, std::size_t versionIndexPos, const py::object &rows) {
            return std::make_unique<PyModDictAssets>(totalIndices, versionIndexPos,
                // VersionParser speaks K, which is std::string now -- adapt parseVersionArg.
                [](const std::string &v) { return parseVersionArg(py::cast(v)); },
                convertRowsOrNestedDict(rows, totalIndices));
        }), py::arg("totalIndices"), py::arg("versionIndexPos"), py::arg("rows") = py::list(), py::doc(R"doc(
Constructs a new asset lookup table

Parameters
----------
totalIndices: :class:`int`
    The total number of index columns (including the version index)

versionIndexPos: :class:`int`
    The position (0-based) of the version index within a row's index values

rows: Union[List[Tuple[List[Any], Any]], dict]
    The initial rows to populate the table with -- either a flat list of ``(indexVals, value)``
    tuples, or a real nested dict ('totalIndices' levels deep) -- see :meth:`addRows`

    **Default**: ``[]``
        )doc"))

        .def_static("fromNestedDict", [](std::size_t totalIndices, std::size_t versionIndexPos, const py::dict &repo) {
            return std::make_unique<PyModDictAssets>(totalIndices, versionIndexPos,
                [](const std::string &v) { return parseVersionArg(py::cast(v)); },
                flattenNestedDict(repo, totalIndices));
        }, py::arg("totalIndices"), py::arg("versionIndexPos"), py::arg("repo"), py::doc(R"doc(
Constructs a new asset lookup table from a real nested dict, flattening it first

Parameters
----------
totalIndices: :class:`int`
    The total number of index columns (including the version index)

versionIndexPos: :class:`int`
    The position (0-based) of the version index within a row's index values

repo: dict
    The nested dict to flatten, exactly 'totalIndices' levels deep (e.g. for
    ``totalIndices = 3``: ``{version: {name: {type: leafValue}}}``)

Raises
------
:class:`ValueError`
    If 'repo' is not nested exactly 'totalIndices' levels deep
        )doc"))

        .def("addRows", [](PyModDictAssets &self, const py::object &rows) {
            self.addRows(convertRowsOrNestedDict(rows, self.getTotalIndices()));
        }, py::arg("rows"), py::doc(R"doc(
Adds new rows to the table, overwriting the value of any row whose full key (every non-version
index value, plus its parsed version) already exists

Parameters
----------
rows: Union[List[Tuple[List[Any], Any]], dict]
    The rows to add, in the same shape as the constructor's own 'rows' argument (a flat list or a
    real nested dict)

Raises
------
:class:`ValueError`
    If any row's index values don't match :attr:`totalIndices` in length, or if a row's version
    index value fails to parse as a version
        )doc"))

        .def("get", [](const PyModDictAssets &self, const std::vector<std::string> &nonVersionVals, const py::object &version, bool errorOnNotFound) -> py::object {
            std::optional<AGRC::Version> parsedVersion = parseVersionArg(version);
            std::optional<std::string> result = self.get(nonVersionVals, parsedVersion, false);
            if (!result.has_value()) {
                if (errorOnNotFound) {
                    throw py::key_error("No matching asset found for the given non-version values");
                }
                return py::none();
            }
            return py::cast(*result);
        }, py::arg("nonVersionVals"), py::arg("version") = py::none(), py::arg("errorOnNotFound") = true, py::doc(R"doc(
Retrieves the corresponding asset

Parameters
----------
nonVersionVals: List[Any]
    The values of every index column that does not refer to a version, in index order (with the
    version column's position skipped)

version: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
    The specific version to query the asset -- the latest available version is used if this is
    ``None`` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

errorOnNotFound: :class:`bool`
    Whether to raise :class:`KeyError` if no matching asset is found

    **Default**: ``True``

Raises
------
:class:`ValueError`
    If 'nonVersionVals' doesn't have exactly :attr:`totalIndices` ``- 1`` elements, or if
    'version' doesn't parse as a valid version

:class:`KeyError`
    If no matching asset is found and 'errorOnNotFound' is ``True``

Returns
-------
Any
    The found asset, or ``None`` if none is found and 'errorOnNotFound' is ``False``
        )doc"))

        .def_property_readonly("totalIndices", &PyModDictAssets::getTotalIndices, py::doc(R"doc(:class:`int`: The total number of index columns (including the version index))doc"))

        .def_property_readonly("versionIndexPos", &PyModDictAssets::getVersionIndexPos, py::doc(R"doc(:class:`int`: The position (0-based) of the version index within a row's index values)doc"))

        .def("__len__", &PyModDictAssets::size, py::doc(R"doc(The total number of rows currently in the table, across every non-version index group)doc"))

        .def("toNestedDict", [](const PyModDictAssets &self) -> py::dict {
            py::dict result;
            std::size_t versionPos = self.getVersionIndexPos();

            self.forEachEntry([&](const std::vector<std::string> &nonVersionVals, const AGRC::Version &version, const std::string &value) {
                // Re-insert the version at its original column position, so the reconstructed
                // dict nests in the exact same order the source data was originally written in
                // (e.g. Hashes' {version: {name: {type: hash}}}).
                std::vector<std::string> fullIndexVals;
                fullIndexVals.reserve(nonVersionVals.size() + 1);
                std::size_t nv = 0;
                for (std::size_t i = 0; i < nonVersionVals.size() + 1; ++i) {
                    if (i == versionPos) {
                        // A normalized string, not the original raw key (a bare float in
                        // HashData.py, before that file was removed) -- ModData.Hashes's own
                        // documented type is Dict[Union[str, float], ...], so this stays
                        // contract-compliant; Version itself doesn't retain the original raw form
                        // it was parsed from.
                        fullIndexVals.push_back(py::str(version.toString()));
                    } else {
                        fullIndexVals.push_back(nonVersionVals[nv++]);
                    }
                }

                py::dict node = result;
                for (std::size_t i = 0; i + 1 < fullIndexVals.size(); ++i) {
                    py::str key(fullIndexVals[i]);
                    if (!node.contains(key) || !py::isinstance<py::dict>(node[key])) {
                        node[key] = py::dict();
                    }
                    node = node[key].cast<py::dict>();
                }
                node[py::str(fullIndexVals.back())] = value;
            });

            return result;
        }, py::doc(R"doc(
Rebuilds the original nested-dict form of this table's data (``{indexVal0: {indexVal1: {... :
value}}}``, in index-column order, the version column's original raw value replaced with its
normalized string form) -- the inverse of :meth:`fromNestedDict`/the constructor's own nested-dict
'rows' shape

Returns
-------
dict
    The reconstructed nested dict
        )doc"));
}

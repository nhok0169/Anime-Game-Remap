#include "PyModAssets.h"

#include <optional>
#include <utility>
#include <vector>

#include "../PyVersion.h"
#include "PyModDictAssets.h"


template class AGRC::ModAssets<py::object, py::object, PyObjectEqual>;

namespace {

    std::vector<std::optional<py::object>> toOptionalList(const std::vector<py::object> &raw) {
        std::vector<std::optional<py::object>> result;
        result.reserve(raw.size());
        for (const py::object &v : raw) {
            result.push_back(v.is_none() ? std::nullopt : std::optional<py::object>(v));
        }
        return result;
    }

    std::vector<std::optional<AGRC::Version>> toVersionList(const std::vector<py::object> &raw) {
        std::vector<std::optional<AGRC::Version>> result;
        result.reserve(raw.size());
        for (const py::object &v : raw) {
            result.push_back(parseVersionArg(v));
        }
        return result;
    }

}


void initCppModAssets(pybind11::module_ &m) {
    py::class_<PyModAssets>(m, "CppModAssets", R"doc(
Handles assets of any type for a mod where retrieval is based on some keys where one or more of
the keys refer to some versioning

:raw-html:`<br />`

If an asset has only one version column, :class:`CppModDictAssets` is the better fit (a real
hash-map lookup instead of this class's linear scan) -- this class exists specifically for the
multi-version-column case (e.g. this project's real ``VGRemaps``, which resolves a ``fromVersion``
and a ``toVersion`` independently and sequentially)

:raw-html:`<br />`

Like :class:`CppModDictAssets`, the source data is never a nested dict internally -- rows are
supplied already-flattened, as a list of ``(indexVals, value)`` tuples, or as a real nested dict
(flattened automatically -- see the constructor's 'rows' argument)
    )doc")

        .def(py::init([](const std::vector<bool> &isVersionColumn, const py::object &rows) {
            return std::make_unique<PyModAssets>(isVersionColumn, parseVersionArg, convertRowsOrNestedDict(rows, isVersionColumn.size()));
        }), py::arg("isVersionColumn"), py::arg("rows") = py::list(), py::doc(R"doc(
Constructs a new asset lookup table

Parameters
----------
isVersionColumn: List[:class:`bool`]
    One entry per index column, in index order -- ``True`` marks that column as a version column.
    Must have at least 1 element

rows: Union[List[Tuple[List[Any], Any]], dict]
    The initial rows to populate the table with -- either a flat list of ``(indexVals, value)``
    tuples, or a real nested dict ('len(isVersionColumn)' levels deep)

    **Default**: ``[]``
        )doc"))

        .def("addRows", [](PyModAssets &self, const py::object &rows) {
            self.addRows(convertRowsOrNestedDict(rows, self.getTotalIndices()));
        }, py::arg("rows"), py::doc(R"doc(
Adds new rows to the table (an addition beyond the pure-Python original, which has no
incremental-add capability at all) -- overwrites the value of any row whose full key already
exists

Parameters
----------
rows: Union[List[Tuple[List[Any], Any]], dict]
    The rows to add, in the same shape as the constructor's own 'rows' argument
        )doc"))

        .def("get", [](const PyModAssets &self, const std::vector<py::object> &nonVersionVals, const std::vector<py::object> &versionVals, bool errorOnNotFound) -> py::object {
            std::optional<py::object> result = self.get(toOptionalList(nonVersionVals), toVersionList(versionVals), false);
            if (!result.has_value()) {
                if (errorOnNotFound) {
                    throw py::key_error("No matching asset found for the given non-version/version values");
                }
                return py::none();
            }
            return *result;
        }, py::arg("nonVersionVals"), py::arg("versionVals"), py::arg("errorOnNotFound") = true, py::doc(R"doc(
Retrieves the corresponding asset

Parameters
----------
nonVersionVals: List[Optional[Any]]
    One entry per non-version column, in their relative index order -- ``None`` at a position
    means "match any value there". Must have exactly :attr:`nonVersionColumnCount` elements

versionVals: List[Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]]
    One entry per version column, in their relative index order -- ``None`` at a position means
    "use the latest available value for this column, among rows still matching everything
    resolved so far". Must have exactly :attr:`versionColumnCount` elements :raw-html:`<br />` :raw-html:`<br />`

    Version columns are resolved sequentially, in index order -- each one's floor-match narrows
    the candidate set before the next version column is resolved against it

errorOnNotFound: :class:`bool`
    Whether to raise :class:`KeyError` if no matching asset is found

    **Default**: ``True``

Raises
------
:class:`ValueError`
    If 'nonVersionVals'/'versionVals' don't have exactly :attr:`nonVersionColumnCount`/
    :attr:`versionColumnCount` elements respectively, or if a version value doesn't parse

:class:`KeyError`
    If no matching asset is found and 'errorOnNotFound' is ``True``

Returns
-------
Any
    The found asset, or ``None`` if none is found and 'errorOnNotFound' is ``False``
        )doc"))

        .def_property_readonly("totalIndices", &PyModAssets::getTotalIndices, py::doc(R"doc(:class:`int`: The total number of index columns)doc"))

        .def_property_readonly("versionColumnCount", &PyModAssets::getVersionColumnCount, py::doc(R"doc(:class:`int`: The number of version columns)doc"))

        .def_property_readonly("nonVersionColumnCount", &PyModAssets::getNonVersionColumnCount, py::doc(R"doc(:class:`int`: The number of non-version columns)doc"))

        .def("__len__", &PyModAssets::size, py::doc(R"doc(The total number of rows currently in the table)doc"));
}

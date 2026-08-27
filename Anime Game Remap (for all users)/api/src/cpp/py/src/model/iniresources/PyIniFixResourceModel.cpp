#include "PyIniFixResourceModel.h"

#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <tsl/ordered_map.h>

#include <pybind11/stl.h>

#include "AGRemapCore/model/iniresources/IniFixResourceModel.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

tsl::ordered_map<int, std::vector<std::string>> intVectorMapFromDict(const py::dict &d) {
    tsl::ordered_map<int, std::vector<std::string>> result;
    for (auto item : d) {
        result.emplace(item.first.cast<int>(), item.second.cast<std::vector<std::string>>());
    }
    return result;
}

py::dict intVectorMapToDict(const tsl::ordered_map<int, std::vector<std::string>> &m) {
    py::dict d;
    for (const auto &[key, val] : m) {
        d[py::int_(key)] = val;
    }
    return d;
}

tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::string>>> nestedMapFromDict(const py::dict &d) {
    tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::string>>> result;
    for (auto outer : d) {
        tsl::ordered_map<std::string, std::vector<std::string>> inner;
        for (auto item : py::cast<py::dict>(outer.second)) {
            inner.emplace(item.first.cast<std::string>(), item.second.cast<std::vector<std::string>>());
        }
        result.emplace(outer.first.cast<int>(), std::move(inner));
    }
    return result;
}

py::dict nestedMapToDict(const tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::string>>> &m) {
    py::dict d;
    for (const auto &[outerKey, inner] : m) {
        py::dict innerDict;
        for (const auto &[innerKey, val] : inner) {
            innerDict[py::str(innerKey)] = val;
        }
        d[py::int_(outerKey)] = innerDict;
    }
    return d;
}

using EntryTuple = std::tuple<std::string, std::string, std::optional<std::string>, std::optional<std::string>>;

std::vector<EntryTuple> entriesToTuples(const std::vector<AGRC::IniFixResourceModel::Entry> &entries) {
    std::vector<EntryTuple> result;
    result.reserve(entries.size());

    for (const auto &entry : entries) {
        result.emplace_back(entry.fixedPath, entry.fullPath, entry.origPath, entry.origFullPath);
    }

    return result;
}

}


void initCppIniFixResourceModel(pybind11::module_ &m) {
    py::class_<AGRC::IniFixResourceModel, AGRC::IniResourceModel>(m, "IniFixResourceModel", R"doc(
This class inherits from :class:`IniResourceModel`

Contains data for fixing a particular resource in a .ini file
    )doc")

        .def(py::init([](std::string iniFolderPath, const py::dict &fixedPaths, const std::optional<py::dict> &origPaths) {
            std::optional<tsl::ordered_map<int, std::vector<std::string>>> convertedOrigPaths;
            if (origPaths.has_value()) {
                convertedOrigPaths = intVectorMapFromDict(*origPaths);
            }
            return AGRC::IniFixResourceModel(std::move(iniFolderPath), nestedMapFromDict(fixedPaths), std::move(convertedOrigPaths));
        }), py::arg("iniFolderPath"), py::arg("fixedPaths"), py::arg("origPaths") = py::none(), py::doc(R"doc(
Constructs new data for fixing a resource in a .ini file

Parameters
----------
iniFolderPath: :class:`str`
    The folder path to where the .ini file of the resource is located

fixedPaths: Dict[:class:`int`, Dict[:class:`str`, List[:class:`str`]]]
    The file paths to the fixed files for the resource -- the outer keys are the indices to the
    :class:`IfContentPart` that the resource file appears in the :class:`IfTemplate` for some
    resource, the inner keys are the names for the type of mod to fix to, and the inner values are
    the file paths within that :class:`IfContentPart`

origPaths: Optional[Dict[:class:`int`, List[:class:`str`]]]
    The file paths for the (unfixed) resource. ``None`` if there's no original-file data at all

    **Default**: ``None``
        )doc"))

        .def_property("fixedPaths", [](const AGRC::IniFixResourceModel &self) { return nestedMapToDict(self.fixedPaths); },
            [](AGRC::IniFixResourceModel &self, const py::dict &fixedPaths) { self.fixedPaths = nestedMapFromDict(fixedPaths); },
    py::doc(R"doc(Dict[:class:`int`, Dict[:class:`str`, List[:class:`str`]]]: The file paths to the fixed files for the resource)doc"))

        .def_property("origPaths",
            [](const AGRC::IniFixResourceModel &self) -> py::object {
                if (!self.origPaths.has_value()) {
                    return py::none();
                }
                return intVectorMapToDict(*self.origPaths);
            },
            [](AGRC::IniFixResourceModel &self, const std::optional<py::dict> &origPaths) {
                if (!origPaths.has_value()) {
                    self.origPaths = std::nullopt;
                } else {
                    self.origPaths = intVectorMapFromDict(*origPaths);
                }
            },
    py::doc(R"doc(Optional[Dict[:class:`int`, List[:class:`str`]]]: The file paths for the (unfixed) resource, if any)doc"))

        .def_property("fullPaths", [](const AGRC::IniFixResourceModel &self) { return nestedMapToDict(self.fullPaths); },
            [](AGRC::IniFixResourceModel &self, const py::dict &fullPaths) { self.fullPaths = nestedMapFromDict(fullPaths); },
    py::doc(R"doc(Dict[:class:`int`, Dict[:class:`str`, List[:class:`str`]]]: The absolute paths to the fixed resource files, keyed the same way as 'fixedPaths')doc"))

        .def_property("origFullPaths", [](const AGRC::IniFixResourceModel &self) { return intVectorMapToDict(self.origFullPaths); },
            [](AGRC::IniFixResourceModel &self, const py::dict &origFullPaths) { self.origFullPaths = intVectorMapFromDict(origFullPaths); },
    py::doc(R"doc(Dict[:class:`int`, List[:class:`str`]]: The absolute paths to the (unfixed) resource files, keyed the same way as 'origPaths')doc"))

        .def("items", [](const AGRC::IniFixResourceModel &self) { return entriesToTuples(self.items()); }, py::doc(R"doc(
Every fixed/orig path combination across every :class:`IfContentPart` and mod type in 'fixedPaths',
in the same order 'fixedPaths' itself iterates -- the equivalent of iterating directly over the
pure-Python original (``for fixedPath, fullPath, origPath, origFullPath in x``)

Returns
-------
List[Tuple[:class:`str`, :class:`str`, Optional[:class:`str`], Optional[:class:`str`]]]
    The flattened ``(fixedPath, fullPath, origPath, origFullPath)`` tuples
        )doc"))

        .def("clear", &AGRC::IniFixResourceModel::clear, py::doc(R"doc(
Clears out all the path data stored
        )doc"));
}

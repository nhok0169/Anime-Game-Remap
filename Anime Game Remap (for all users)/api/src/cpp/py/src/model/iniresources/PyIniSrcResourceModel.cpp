#include "PyIniSrcResourceModel.h"

#include <string>
#include <vector>

#include <tsl/ordered_map.h>

#include <pybind11/stl.h>

#include "AGRemapCore/model/iniresources/IniSrcResourceModel.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

// tsl::ordered_map has no built-in pybind11 type_caster -- converted through a py::dict by hand,
// same pattern as PyIniClassifyStats.cpp's modTypeFromDict/modTypeToDict.
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

}


void initCppIniSrcResourceModel(pybind11::module_ &m) {
    py::class_<AGRC::IniSrcResourceModel, AGRC::IniResourceModel>(m, "IniSrcResourceModel", R"doc(
This class inherits from :class:`IniResourceModel`

Contains data for a particular resource in the original .ini file
    )doc")

        .def(py::init([](std::string iniFolderPath, const py::dict &paths) {
            return AGRC::IniSrcResourceModel(std::move(iniFolderPath), intVectorMapFromDict(paths));
        }), py::arg("iniFolderPath"), py::arg("paths"), py::doc(R"doc(
Constructs new data for a resource in the original .ini file

Parameters
----------
iniFolderPath: :class:`str`
    The folder path to where the .ini file of the resource is located

paths: Dict[:class:`int`, List[:class:`str`]]
    The file paths to the resource -- the keys are the indices to the :class:`IfContentPart` that
    the resource file appears in the :class:`IfTemplate` for some resource, and the values are the
    file paths within that :class:`IfContentPart`
        )doc"))

        .def_property("paths", [](const AGRC::IniSrcResourceModel &self) { return intVectorMapToDict(self.paths); },
            [](AGRC::IniSrcResourceModel &self, const py::dict &paths) { self.paths = intVectorMapFromDict(paths); },
    py::doc(R"doc(Dict[:class:`int`, List[:class:`str`]]: The file paths to the resource, keyed by :class:`IfContentPart` index)doc"))

        .def_property("fullPaths", [](const AGRC::IniSrcResourceModel &self) { return intVectorMapToDict(self.fullPaths); },
            [](AGRC::IniSrcResourceModel &self, const py::dict &fullPaths) { self.fullPaths = intVectorMapFromDict(fullPaths); },
    py::doc(R"doc(Dict[:class:`int`, List[:class:`str`]]: The absolute paths to the resource, keyed the same way as 'paths')doc"))

        .def("items", &AGRC::IniSrcResourceModel::items, py::doc(R"doc(
Every ``(path, fullPath)`` pair across every :class:`IfContentPart` in 'paths', in the same order
'paths' itself iterates -- the equivalent of iterating directly over the pure-Python original
(``for path, fullPath in x``)

Returns
-------
List[Tuple[:class:`str`, :class:`str`]]
    The flattened ``(path, fullPath)`` pairs
        )doc"));
}

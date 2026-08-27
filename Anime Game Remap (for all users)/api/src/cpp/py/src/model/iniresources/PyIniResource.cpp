#include "PyIniResource.h"

#include <string>

#include <pybind11/stl.h>

#include "AGRemapCore/model/iniresources/IniResource.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppIniResource(pybind11::module_ &m) {
    // py::smart_holder -- required so a unique_ptr<IniResource> (or a unique_ptr<IniResource>
    // pointing at a IniFixResource, via the real inheritance relationship registered below) can
    // be extracted from an existing Python object when building a IniGroupedResource's
    // 'resources' map.
    py::class_<AGRC::IniResource, py::smart_holder>(m, "IniResource", R"doc(
Base class for a resource in the .ini file
    )doc")

        .def(py::init<std::string, std::string, std::string>(), py::arg("type"), py::arg("iniFolderPath"), py::arg("srcPath"), py::doc(R"doc(
Constructs a new resource

Parameters
----------
type: :class:`str`
    The name for the type of resource

iniFolderPath: :class:`str`
    The path to the folder of the .ini file

srcPath: :class:`str`
    The file path to the resource (resolved to an absolute path against 'iniFolderPath')
        )doc"))

        .def_readwrite("type", &AGRC::IniResource::type, py::doc(R"doc(
:class:`str`: The name for the type of resource
        )doc"))

        .def_readwrite("srcPath", &AGRC::IniResource::srcPath, py::doc(R"doc(
:class:`str`: The full file path to the resource
        )doc"));
}

void initCppIniFixResource(pybind11::module_ &m) {
    py::class_<AGRC::IniFixResource, AGRC::IniResource, py::smart_holder>(m, "IniFixResource", R"doc(
This class inherits from :class:`IniResource`

Base class for a resource to be fixed in the .ini file
    )doc")

        .def(py::init<std::string, std::string, std::string, std::string>(), py::arg("type"), py::arg("iniFolderPath"),
             py::arg("srcPath"), py::arg("fixedPath"), py::doc(R"doc(
Constructs a new resource to be fixed

Parameters
----------
type: :class:`str`
    The name for the type of resource

iniFolderPath: :class:`str`
    The path to the folder of the .ini file

srcPath: :class:`str`
    The file path to the resource (resolved to an absolute path against 'iniFolderPath')

fixedPath: :class:`str`
    The file path to the fixed resource (resolved to an absolute path against 'iniFolderPath')
        )doc"))

        .def_readwrite("fixedPath", &AGRC::IniFixResource::fixedPath, py::doc(R"doc(
:class:`str`: The full file path to the fixed resource
        )doc"));
}

// initCppIniGroupedResource moved to PyIniGroupedResource.cpp -- it binds PyIniGroupedResource
// (a dedicated Python-facing subclass), not AGRC::IniGroupedResource directly. See that file's own
// header comment for why.

#include "PyRemapIniGroupedResource.h"

#include <functional>
#include <memory>
#include <string>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppRemapIniGroupedResource(pybind11::module_ &m) {
    // py::smart_holder -- both bases (IniGroupedResource, RemapIniResourceMixin) use it, and a
    // holder type must stay consistent throughout one inheritance chain. 'resources'/'name'/
    // 'fixFunc'/'isBuilt'/'fix'/'isMissing'/'addResource'/'__deepcopy__' are all inherited from
    // IniGroupedResource automatically (real Python multiple inheritance, no rebinding needed --
    // same established pattern as RemapIniResource inheriting IniResource's properties); the
    // 6 status-query methods are likewise inherited from RemapIniResourceMixin.
    py::class_<PyRemapIniGroupedResource, PyIniGroupedResource, AGRC::RemapIniResourceMixin, py::smart_holder>(m, "RemapIniGroupedResource", R"doc(
This class inherits from :class:`IniGroupedResource` and :class:`RemapIniResourceMixin`

Base class for a group of resources to fix in a .ini file that's used by the overall remap process
    )doc")

        // Same shared-mutable-default-argument fix as CppIniGroupedResource's own constructor
        // binding -- see that file's own comment on why a lambda is required here instead of a
        // plain 'py::arg("resources") = py::dict()' default.
        .def(py::init([](std::string name, py::object resources, std::function<bool(AGRC::IniGroupedResource&)> fixFunc, bool isBuilt) {
            py::dict resourcesDict = resources.is_none() ? py::dict() : resources.cast<py::dict>();
            return std::make_unique<PyRemapIniGroupedResource>(std::move(name), std::move(resourcesDict), std::move(fixFunc), isBuilt);
        }), py::arg("name"), py::arg("resources") = py::none(), py::arg("fixFunc") = py::none(), py::arg("isBuilt") = true, py::doc(R"doc(
Constructs a new group of resources to fix -- see :class:`IniGroupedResource`'s constructor for the parameters
        )doc"));
}

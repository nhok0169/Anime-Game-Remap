#include "PyIniGroupedResource.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

namespace py = pybind11;
namespace AGRC = AGRemapCore;


PyIniGroupedResource::PyIniGroupedResource(std::string name, py::dict resources, std::function<bool(AGRC::IniGroupedResource&)> fixFunc, bool isBuilt):
    // The base class's own 'resources' map is never populated from the Python-facing side (see this
    // class's own header comment) -- pass an explicitly-constructed empty map, not a bare '{}'
    // literal, per this codebase's documented MSVC move-only-container-default-argument quirk.
    AGRC::IniGroupedResource(std::move(name), std::unordered_map<std::string, std::unique_ptr<AGRC::IniResource>>{}, std::move(fixFunc), isBuilt),
    resources(std::move(resources)) {
}

bool PyIniGroupedResource::isMissing(const py::object& collected) const {
    for (auto item : collected) {
        if (PyDict_Contains(resources.ptr(), item.ptr()) != 1) {
            return true;
        }
    }

    return false;
}

void PyIniGroupedResource::addResource(py::object resType, py::object resource) {
    resources[resType] = resource;
}

py::object iniGroupedResourceDeepCopy(const PyIniGroupedResource& self, py::object memo) {
    py::object deepcopy = py::module_::import("copy").attr("deepcopy");
    py::object newResources = deepcopy(self.resources, memo);

    // Reconstruct via 'self.__class__', not a hardcoded C++ type -- dispatches correctly whether
    // 'self' is a PyIniGroupedResource or a Python-facing subclass (eg. PyRemapIniGroupedResource)
    // sharing this same __deepcopy__ binding.
    py::object selfObj = py::cast(self);
    py::object selfClass = selfObj.attr("__class__");
    return selfClass(self.name, newResources, self.fixFunc, self.isBuilt);
}


void initCppIniGroupedResource(pybind11::module_ &m) {
    // NOTE: AGRC::IniGroupedResource is NOT listed as a pybind base here, even though
    // PyIniGroupedResource really does inherit from it in C++ -- pybind11 requires a class listed as
    // a py::class_ base template argument to already be its own registered Python type (for the
    // automatic up/downcasting machinery), and AGRC::IniGroupedResource deliberately never gets its
    // own separate "IniGroupedResource"-named registration (there's nothing Python-facing that
    // needs to treat a PyIniGroupedResource as a distinct AGRC::IniGroupedResource object -- the
    // real C++ inheritance alone is enough for #fix/#name/#fixFunc/#isBuilt to work through the
    // normal vtable). Omitting it here previously produced a hard runtime ImportError ("referenced
    // unknown base type") at module-import time, not a compile error -- if this class ever needs a
    // second real pybind base in the future, don't reach for AGRC::IniGroupedResource itself.
    py::class_<PyIniGroupedResource, py::smart_holder>(m, "IniGroupedResource", R"doc(
Base class for a group of resources

.. note::
    'resources' is a real Python ``dict`` here (not a typed mapping to some resource class) --
    this class's one real caller (``ResGroupCollect``) uses it as general-purpose scratch storage
    keyed by arbitrary hashable values, not just resource type names; see this binding's own
    source comment for why
    )doc")

        // py::init(factory), NOT py::init<std::string, py::dict, ...>() with a plain
        // 'py::arg("resources") = py::dict()' default -- that form bakes in ONE Python dict
        // object at bind time (module load), shared by every single construction that omits
        // 'resources' from then on (the exact same "mutable default argument" footgun as Python's
        // own 'def f(x=[]):'). A lambda constructs a genuinely fresh py::dict() each time it
        // actually runs (call time, not bind time) when 'resources' is omitted -- confirmed this
        // was a real, live bug via a test that constructed two IniGroupedResource()s with no
        // 'resources' argument and saw the second one already polluted with the first one's data.
        .def(py::init([](std::string name, py::object resources, std::function<bool(AGRC::IniGroupedResource&)> fixFunc, bool isBuilt) {
            py::dict resourcesDict = resources.is_none() ? py::dict() : resources.cast<py::dict>();
            return std::make_unique<PyIniGroupedResource>(std::move(name), std::move(resourcesDict), std::move(fixFunc), isBuilt);
        }), py::arg("name"), py::arg("resources") = py::none(), py::arg("fixFunc") = py::none(), py::arg("isBuilt") = true, py::doc(R"doc(
Constructs a new group of resources

Parameters
----------
name: :class:`str`
    The name of the group of resources

resources: Optional[Dict[Any, Any]]
    The group of resources -- general-purpose scratch storage, keyed and valued by whatever the
    caller needs (see this class's own note above). If ``None``, a fresh empty ``dict`` is used

    **Default**: ``None``

fixFunc: Optional[Callable[[:class:`IniGroupedResource`], :class:`bool`]]
    Custom function for fixing the resource, overriding the default (no-op) behavior if given

    **Default**: ``None``

isBuilt: :class:`bool`
    Whether the grouped resource is ready to be fixed

    **Default**: ``True``
        )doc"))

        .def_readwrite("resources", &PyIniGroupedResource::resources, py::doc(R"doc(
Dict[Any, Any]: The group of resources -- general-purpose scratch storage (see the class's own note)
        )doc"))

        .def_readwrite("name", &AGRC::IniGroupedResource::name, py::doc(R"doc(
:class:`str`: The name of the group of resources
        )doc"))

        .def_readwrite("fixFunc", &AGRC::IniGroupedResource::fixFunc, py::doc(R"doc(
Optional[Callable[[:class:`IniGroupedResource`], :class:`bool`]]: Custom function for fixing the resource, overriding the default (no-op) behavior if set
        )doc"))

        .def_readwrite("isBuilt", &AGRC::IniGroupedResource::isBuilt, py::doc(R"doc(
:class:`bool`: Whether the grouped resource is ready to be fixed
        )doc"))

        .def("fix", &AGRC::IniGroupedResource::fix, py::doc(R"doc(
Fixes the resource -- calls 'fixFunc' if set, otherwise does nothing

Returns
-------
:class:`bool`
    Whether the resource was fixed
        )doc"))

        .def("isMissing", &PyIniGroupedResource::isMissing, py::arg("collected"), py::doc(R"doc(
Given a subset of the collected resource keys so far, is this grouped resource missing some
resource from the given subset

Parameters
----------
collected: Iterable[Any]
    The subset of the keys of the collected resources so far

Returns
-------
:class:`bool`
    Whether this grouped resource is missing some resource from the specified subset
        )doc"))

        .def("addResource", &PyIniGroupedResource::addResource, py::arg("resType"), py::arg("resource"), py::doc(R"doc(
Adds an individual resource to the resource group

Parameters
----------
resType: Any
    The key for the resource

resource: Any
    The resource to add
        )doc"))

        .def("__deepcopy__", &iniGroupedResourceDeepCopy, py::arg("memo"), py::doc(R"doc(
Supports ``copy.deepcopy()`` on this object
        )doc"));
}

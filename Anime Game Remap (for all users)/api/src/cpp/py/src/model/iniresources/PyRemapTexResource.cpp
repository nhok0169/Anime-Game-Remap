#include "PyRemapTexResource.h"

#include <functional>
#include <string>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/iniresources/RemapTexResource.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppRemapTexAddResource(pybind11::module_ &m) {
    // No stats-taking methods rebound here -- same reasoning as PyRemapBlendResource.cpp: every
    // RemapIniResourceMixin-declared method is inherited via real Python inheritance from
    // RemapIniResource, dispatching correctly to this class's own overrides through the vtable.
    // py::smart_holder -- its base (RemapIniResource) uses it, and a holder type must stay
    // consistent throughout one inheritance chain.
    py::class_<AGRC::RemapTexAddResource, AGRC::RemapIniResource, py::smart_holder>(m, "RemapTexAddResource", R"doc(
This class inherits from :class:`RemapIniResource`

Class for adding a brand new texture file used by the overall remap process
    )doc")

        .def(py::init<const std::string&, const std::string&, AGRC::TexCreator, std::string, std::function<bool(AGRC::RemapTexAddResource&)>>(),
             py::arg("iniFolderPath"), py::arg("srcPath"), py::arg("texCreator"), py::arg("type") = "resourceRemapTexAdd",
             py::arg("fixFunc") = py::none(), py::doc(R"doc(
Constructs a new texture-add resource

Parameters
----------
iniFolderPath: :class:`str`
    The path to the folder of the .ini file

srcPath: :class:`str`
    The file path to the resource

texCreator: :class:`CppTexCreator`
    The texture creator used to create the ``.dds`` file if it's missing

type: :class:`str`
    The name for the type of resource

    **Default**: ``"resourceRemapTexAdd"``

fixFunc: Optional[Callable[[:class:`RemapTexAddResource`], :class:`bool`]]
    Custom function for fixing the resource, overriding the default behavior if given

    **Default**: ``None``
        )doc"))

        .def_readwrite("texCreator", &AGRC::RemapTexAddResource::texCreator, py::doc(R"doc(
:class:`CppTexCreator`: The texture creator used to create the ``.dds`` file if it's missing
        )doc"))

        .def_readwrite("fixFunc", &AGRC::RemapTexAddResource::fixFunc, py::doc(R"doc(
Optional[Callable[[:class:`RemapTexAddResource`], :class:`bool`]]: Custom function for fixing the resource, overriding the default behavior if set
        )doc"))

        .def("fix", &AGRC::RemapTexAddResource::fix, py::doc(R"doc(
Fixes the resource -- calls the custom 'fixFunc' if set at construction, otherwise creates the
texture file at ``srcPath`` if it doesn't already exist

Returns
-------
:class:`bool`
    Whether the resource was fixed
        )doc"));
}

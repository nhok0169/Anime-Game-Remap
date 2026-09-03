#include "PyRemapTexResource.h"

#include <functional>
#include <string>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/iniresources/RemapTexResource.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"

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


void initCppRemapTexEditResource(pybind11::module_ &m) {
    // Same reasoning as initCppRemapTexAddResource above, which this deliberately mirrors.
    py::class_<AGRC::RemapTexEditResource, AGRC::RemapIniFixResource, py::smart_holder>(m, "RemapTexEditResource", R"doc(
This class inherits from :class:`RemapIniFixResource`

Class for editing a texture file used by the overall remap process

The texture counterpart to :class:`RemapBlendResource`, and shaped like it rather than like
:class:`RemapTexAddResource`: an **edit** reads one file and writes another, so it carries the
``srcPath``/``fixedPath`` pair. An *add* has only the one path
    )doc")

        .def(py::init<const std::string&, const std::string&, const std::string&, AGRC::TexEditor, std::string, std::function<bool(AGRC::RemapTexEditResource&)>>(),
             py::arg("iniFolderPath"), py::arg("srcPath"), py::arg("fixedPath"), py::arg("texEditor"),
             py::arg("type") = "resourceRemapTexEdit",
             py::arg("fixFunc") = py::none(), py::doc(R"doc(
Constructs a new texture-edit resource

Parameters
----------
iniFolderPath: :class:`str`
    The path to the folder of the .ini file

srcPath: :class:`str`
    The file path to the resource

fixedPath: :class:`str`
    The file path to the fixed resource

texEditor: :class:`CppTexEditor`
    The texture editor used to edit the ``.dds`` file

type: :class:`str`
    The name for the type of resource

    **Default**: ``"resourceRemapTexEdit"``

fixFunc: Optional[Callable[[:class:`RemapTexEditResource`], :class:`bool`]]
    Custom function for fixing the resource, overriding the default behavior if given

    **Default**: ``None``
        )doc"))

        .def_readwrite("texEditor", &AGRC::RemapTexEditResource::texEditor, py::doc(R"doc(
:class:`CppTexEditor`: The texture editor used to edit the ``.dds`` file
        )doc"))

        .def_readwrite("fixFunc", &AGRC::RemapTexEditResource::fixFunc, py::doc(R"doc(
Optional[Callable[[:class:`RemapTexEditResource`], :class:`bool`]]: Custom function for fixing the resource, overriding the default behavior if set
        )doc"))

        .def("fix", &AGRC::RemapTexEditResource::fix, py::doc(R"doc(
Fixes the resource -- calls the custom 'fixFunc' if set at construction, otherwise edits the
texture at ``srcPath`` and writes the result to ``fixedPath``

Returns
-------
:class:`bool`
    Whether the resource was fixed
        )doc"));
}

// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/iniresources/RemapFileResource.h"
#include "../../tools/PyRefFunction.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {
    std::vector<std::uint8_t> toByteVec(const py::bytes& bytes) {
        const std::string data = bytes;
        return std::vector<std::uint8_t>(data.begin(), data.end());
    }
}


void initCppRemapFileAddResource(pybind11::module_ &m) {
    // Mirrors initCppRemapTexAddResource: the stats-taking methods come from RemapIniResource through
    // real Python inheritance, and the holder matches that base's
    py::class_<AGRC::RemapFileAddResource, AGRC::RemapIniResource, py::smart_holder>(m, "RemapFileAddResource", R"doc(
This class inherits from :class:`RemapIniResource`

Class for adding a brand new file, of contents the fix already knows, used by the overall remap process
-- eg. the source of a ``CustomShader`` the fix draws a part with, or the constant buffer that shader reads

The non-texture counterpart to :class:`RemapTexAddResource`, counted in :attr:`RemapStats.fileAdd`
    )doc")

        .def(py::init([](const std::string &iniFolderPath, const std::string &srcPath, const py::bytes &bytes,
                         std::string type, const PyOptionalCallable<bool(AGRC::RemapFileAddResource&)> &fixFunc) {
                 return std::make_unique<AGRC::RemapFileAddResource>(iniFolderPath, srcPath, toByteVec(bytes), std::move(type),
                                                                     toPyRefFunction<bool(AGRC::RemapFileAddResource&)>(fixFunc));
             }),
             py::arg("iniFolderPath"), py::arg("srcPath"), py::arg("bytes"), py::arg("type") = "resourceRemapFileAdd",
             py::arg("fixFunc") = py::none(), py::doc(R"doc(
Constructs a new file-add resource

Parameters
----------
iniFolderPath: :class:`str`
    The path to the folder of the .ini file

srcPath: :class:`str`
    The path of the file to write

bytes: :class:`bytes`
    The file's contents

type: :class:`str`
    The name for the type of resource

    **Default**: ``"resourceRemapFileAdd"``

fixFunc: Optional[Callable[[:class:`RemapFileAddResource`], :class:`bool`]]
    Custom function for fixing the resource, overriding the default behavior if given

    **Default**: ``None``
        )doc"))

        .def_property("bytes",
            [](const AGRC::RemapFileAddResource &self) {
                return py::bytes(reinterpret_cast<const char*>(self.bytes.data()), self.bytes.size());
            },
            [](AGRC::RemapFileAddResource &self, const py::bytes &bytes) {
                self.bytes = toByteVec(bytes);
            }, py::doc(R"doc(
:class:`bytes`: The contents written to the file
        )doc"))

        .def_property("fixFunc",
            [](const AGRC::RemapFileAddResource &self) {
                return fromPyRefFunction(self.fixFunc);
            },
            [](AGRC::RemapFileAddResource &self, const PyOptionalCallable<bool(AGRC::RemapFileAddResource&)> &fixFunc) {
                self.fixFunc = toPyRefFunction<bool(AGRC::RemapFileAddResource&)>(fixFunc);
            }, py::doc(R"doc(
Optional[Callable[[:class:`RemapFileAddResource`], :class:`bool`]]: Custom function for fixing the resource, overriding the default behavior if set
        )doc"))

        .def("fix", &AGRC::RemapFileAddResource::fix, py::doc(R"doc(
Fixes the resource -- calls the custom 'fixFunc' if set at construction, otherwise writes :attr:`bytes` to
``srcPath``, creating its folder when it is missing

Returns
-------
:class:`bool`
    Whether the file was written
        )doc"));
}

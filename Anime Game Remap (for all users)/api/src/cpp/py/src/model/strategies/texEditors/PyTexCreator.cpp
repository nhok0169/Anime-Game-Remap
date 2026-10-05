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

#include "PyTexCreator.h"

#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/model/strategies/texEditors/BaseTexEditor.h"
#include "AGRemapCore/model/textures/Colour.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppTexCreator(pybind11::module_ &m) {
    py::class_<AGRC::TexCreator, AGRC::BaseTexEditor, py::smart_holder>(m, "CppTexCreator", R"doc(
This class inherits from :class:`CppBaseTexEditor`

Creates a brand new ``.dds`` file if the file does not already exist
    )doc")

        .def(py::init<int, int, AGRC::Colour, bool, bool>(), py::arg("width"), py::arg("height"),
             py::arg("colour") = AGRC::Colour(), py::arg("compress") = true, py::arg("mipmaps") = false, py::doc(R"doc(
Constructs a new texture creator

Parameters
----------
width: :class:`int`
    The width, in pixels, of the texture to create

height: :class:`int`
    The height, in pixels, of the texture to create

colour: :class:`CppColour`
    The fill colour of the texture to create. **Default**: opaque white

compress: :class:`bool`
    Whether the created texture is written compressed, or as a plain 32-bit uncompressed ``.dds``.
    **Default**: ``True``

mipmaps: :class:`bool`
    Whether the created texture is written with its full mip chain -- see :meth:`CppTextureFile.save`.
    **Default**: ``False``
        )doc"))

        .def_readwrite("width", &AGRC::TexCreator::width, py::doc(R"doc(
:class:`int`: The width, in pixels, of the texture to create
        )doc"))

        .def_readwrite("height", &AGRC::TexCreator::height, py::doc(R"doc(
:class:`int`: The height, in pixels, of the texture to create
        )doc"))

        .def_readwrite("colour", &AGRC::TexCreator::colour, py::doc(R"doc(
:class:`CppColour`: The fill colour of the texture to create
        )doc"))

        .def_readwrite("compress", &AGRC::TexCreator::compress, py::doc(R"doc(
:class:`bool`: Whether the created texture is written in a compressed format, or as a plain 32-bit
uncompressed ``.dds``

The :class:`CppTexCreator` counterpart of :attr:`CppTexEditor.compress`
        )doc"))

        .def_readwrite("mipmaps", &AGRC::TexCreator::mipmaps, py::doc(R"doc(
:class:`bool`: Whether the created texture is written with its full mip chain

The :class:`CppTexCreator` counterpart of :attr:`CppTexEditor.mipmaps`
        )doc"));
}

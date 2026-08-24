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

        .def(py::init<int, int, AGRC::Colour>(), py::arg("width"), py::arg("height"),
             py::arg("colour") = AGRC::Colour(), py::doc(R"doc(
Constructs a new texture creator

Parameters
----------
width: :class:`int`
    The width, in pixels, of the texture to create

height: :class:`int`
    The height, in pixels, of the texture to create

colour: :class:`CppColour`
    The fill colour of the texture to create. **Default**: opaque white
        )doc"))

        .def_readwrite("width", &AGRC::TexCreator::width, py::doc(R"doc(
:class:`int`: The width, in pixels, of the texture to create
        )doc"))

        .def_readwrite("height", &AGRC::TexCreator::height, py::doc(R"doc(
:class:`int`: The height, in pixels, of the texture to create
        )doc"))

        .def_readwrite("colour", &AGRC::TexCreator::colour, py::doc(R"doc(
:class:`CppColour`: The fill colour of the texture to create
        )doc"));
}

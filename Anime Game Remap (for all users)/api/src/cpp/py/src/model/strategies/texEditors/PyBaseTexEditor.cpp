#include "PyBaseTexEditor.h"

#include "AGRemapCore/model/strategies/texEditors/BaseTexEditor.h"
#include "AGRemapCore/model/files/TextureFile.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppBaseTexEditor(pybind11::module_ &m) {
    py::class_<AGRC::BaseTexEditor, py::smart_holder>(m, "CppBaseTexEditor", R"doc(
Base class to edit some ``.dds`` file
    )doc")

        .def(py::init<>())

        .def("fix", &AGRC::BaseTexEditor::fix, py::arg("texFile"), py::arg("fixedTexFile"), py::doc(R"doc(
Edits the texture file. No-op by default

Parameters
----------
texFile: :class:`CppTextureFile`
    The texture ``.dds`` file to be modified

fixedTexFile: :class:`str`
    The name of the fixed texture file
        )doc"));
}

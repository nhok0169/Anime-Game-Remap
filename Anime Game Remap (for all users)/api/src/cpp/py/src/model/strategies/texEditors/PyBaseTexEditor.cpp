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

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

#include "PyTexEditor.h"

#include <memory>
#include <vector>

#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
#include "AGRemapCore/model/strategies/texEditors/BaseTexEditor.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppTexEditor(pybind11::module_ &m) {
    py::class_<AGRC::TexEditor, AGRC::BaseTexEditor, py::smart_holder>(m, "CppTexEditor", R"doc(
This class inherits from :class:`CppBaseTexEditor`

The pure-C++-SDK-facing engine behind :class:`TexEditor` -- runs a fixed sequence of
:class:`CppBaseTexFilter`\s over a texture file. :meth:`~CppBaseTexEditor.fix` is a no-op unless a
filter list was passed to the constructor.

.. note::
    The Python-facing :class:`TexEditor` overrides :meth:`~CppBaseTexEditor.fix` itself instead of
    using this class's filter list, so that its own ``filters`` attribute can hold arbitrary Python
    callables (not just objects this constructor can accept) -- see that class for the
    Python-visible behavior
    )doc")

        .def(py::init([](bool compress, bool mipmaps) {
                 return std::make_unique<AGRC::TexEditor>(std::vector<AGRC::TexEditor::Filter>{}, compress, mipmaps);
             }),
             py::arg("compress") = true, py::arg("mipmaps") = false)

        .def_property("mipmaps", &AGRC::TexEditor::getMipmaps, &AGRC::TexEditor::setMipmaps, py::doc(R"doc(
Whether :meth:`~CppBaseTexEditor.fix` writes the edited texture back with its full mip chain --
handed straight to :meth:`CppTextureFile.save`, which says why a texture wants one

**Default**: ``False``
        )doc"))

        .def_property("compress", &AGRC::TexEditor::getCompress, &AGRC::TexEditor::setCompress, py::doc(R"doc(
Whether :meth:`~CppBaseTexEditor.fix` writes the edited texture back in its original compressed
format, or as a plain 32-bit uncompressed ``.dds``

Handed straight to :meth:`CppTextureFile.save`, which documents the trade. The short version: BCn
encoding is almost the entire cost of an edit, and turning it off is roughly eight times faster for
a file about four times larger.

**Default**: ``True``
        )doc"));
}

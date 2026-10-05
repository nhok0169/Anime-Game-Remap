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

#include "PyHighlightShadow.h"

#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/HighlightShadow.h"
#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/BasePixelTransform.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppHighlightShadow(pybind11::module_ &m) {
    py::class_<AGRC::HighlightShadow, AGRC::BasePixelTransform, py::smart_holder>(m, "CppHighlightShadow", R"doc(
This class inherits from :class:`CppBasePixelTransform`

A filter that approximates the adjustment of the shadow/highlight of an image

.. note::
    Reference: `Highlight Shadow Approximation Reference`_
    )doc")

        .def(py::init<double, double>(), py::arg("highlight") = 0, py::arg("shadow") = 0, py::doc(R"doc(
Constructs a new highlight/shadow pixel transform

Parameters
----------
highlight: :class:`float`
    The amount of highlight to apply to the pixel. Range from -1 to 1, and 0 = no change.
    **Default**: ``0``

shadow: :class:`float`
    The amount of shadow to apply to the pixel. Range from -1 to 1, and 0 = no change.
    **Default**: ``0``
        )doc"))

        .def_readwrite("highlight", &AGRC::HighlightShadow::highlight, py::doc(R"doc(
:class:`float`: The amount of highlight to apply to the pixel. Range from -1 to 1, and 0 = no change
        )doc"))

        .def_readwrite("shadow", &AGRC::HighlightShadow::shadow, py::doc(R"doc(
:class:`float`: The amount of shadow to apply to the pixel. Range from -1 to 1, and 0 = no change
        )doc"));
}

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

#include "PyTransparency.h"

#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/Transparency.h"
#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/BasePixelTransform.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppTransparency(pybind11::module_ &m) {
    py::class_<AGRC::Transparency, AGRC::BasePixelTransform, py::smart_holder>(m, "CppTransparency", R"doc(
This class inherits from :class:`CppBasePixelTransform`

Adjusts the transparency (alpha channel) of a pixel
    )doc")

        .def(py::init<int>(), py::arg("alphaChange"), py::doc(R"doc(
Constructs a new transparency pixel transform

Parameters
----------
alphaChange: :class:`int`
    How much to adjust the alpha channel of the pixel. Range from -255 to 255

    .. note::
        The alpha channel for an image is inclusively bounded from 0 to 255
        )doc"))

        .def_readwrite("alphaChange", &AGRC::Transparency::alphaChange, py::doc(R"doc(
:class:`int`: How much to adjust the alpha channel of the pixel. Range from -255 to 255
        )doc"));
}

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

#include "PyInvertAlpha.h"

#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/InvertAlpha.h"
#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/BasePixelTransform.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppInvertAlpha(pybind11::module_ &m) {
    py::class_<AGRC::InvertAlpha, AGRC::BasePixelTransform, py::smart_holder>(m, "CppInvertAlpha", R"doc(
This class inherits from :class:`CppBasePixelTransform`

Inverts the alpha channel of a pixel
    )doc")

        .def(py::init<>());
}
